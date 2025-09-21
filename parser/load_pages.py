import argparse
from pathlib import Path
import asyncio
import aiohttp
from bs4 import BeautifulSoup
from dataclasses import dataclass, asdict
import hashlib
import json
from tqdm import tqdm
import sys


@dataclass
class Document:
    url: str
    title: str
    text: str

    @staticmethod
    def parse(url: str, html_text: str) -> "Document":
        """
        Разбор HTML и создание объекта Document.
        Бросает ValueError, если данные неполные.
        """
        soup = BeautifulSoup(html_text, "lxml")

        title_tag = soup.find("h1")
        title = title_tag.get_text(strip=True) if title_tag else None

        body = soup.find("div", class_="article-formatted-body")
        text = body.get_text(separator="\n", strip=True) if body else None

        if not text:
            raise ValueError(f"не достаточно данных для text: {url}")

        return Document(url=url, title=title, text=text)


class StorageManager:
    def __init__(self, output: str, *, number_limit: int = None, memory_limit: int = None):
        if number_limit is None and memory_limit is None:
            raise ValueError("не указан лимит документов")
        if number_limit is not None and memory_limit is not None:
            raise ValueError("указано два лимита для документов")

        self._numlim = number_limit
        self._memlim = memory_limit

        self._pbar = None
        if self._numlim:
            self._pbar = tqdm(total=self._numlim,
                              desc="Saving docs", unit="doc")
        elif self._memlim:
            self._pbar = tqdm(
                total=self._memlim,
                desc="Saving docs",
                unit="B",
                unit_scale=True,
            )

        self.path = Path(output).expanduser().resolve()
        self.path.mkdir(exist_ok=True)

        self.on_limit_reached = None
        self._count = 0
        self._size = 0

    def save(self, doc: Document):
        if self._limit_is_reached():
            return
        # имя файла = md5(url).json
        filename = hashlib.md5(doc.url.encode("utf-8")).hexdigest() + ".json"
        filepath = self.path / filename

        with open(filepath, "w", encoding="utf-8") as f:
            json.dump(asdict(doc), f, ensure_ascii=False, indent=2)

        self._count += 1
        file_size = filepath.stat().st_size
        self._size += file_size

        if self._pbar:
            if self._numlim:
                self._pbar.update(1)
            elif self._memlim:
                self._pbar.update(file_size)
        self._update_limits()

    def _limit_is_reached(self) -> bool:
        return (self._numlim is not None and self._count >= self._numlim) or \
               (self._memlim is not None and self._size >= self._memlim)

    def _update_limits(self):
        if self._limit_is_reached():
            if self._pbar:
                self._pbar.close()
            if self.on_limit_reached:
                self.on_limit_reached()


class PageDownloader:
    def __init__(self, storage: "StorageManager", *, workers: int = 4):
        self.storage = storage
        self.workers = workers
        self._lock = asyncio.Lock()
        self._queue = asyncio.Queue()
        self._stopped = False

        self.storage.on_limit_reached = self.stop

    def stop(self):
        """Вызывается при достижении лимита, чтобы завершить всех воркеров"""
        self._stopped = True

        while not self._queue.empty():
            self._queue.get_nowait()
        for _ in range(self.workers):
            self._queue.put_nowait(None)  # sentinel для остановки воркеров

    async def fetch(self, session: aiohttp.ClientSession, url: str):
        try:
            async with session.get(url) as resp:
                if resp.status == 200:
                    html = await resp.text()
                    try:
                        doc = Document.parse(url, html)

                        async with self._lock:
                            self.storage.save(doc)
                            tqdm.write(f"[OK] {url}", end='\r')

                    except ValueError as e:
                        tqdm.write(
                            f"[PARSE ERROR] {url}: {e}", file=sys.stderr)

                elif resp.status == 404:
                    pass
                else:
                    tqdm.write(f"[HTTP {resp.status}] {url}", file=sys.stderr)

        except Exception as e:
            tqdm.write(f"[NETWORK ERROR] {url}: {e}", file=sys.stderr)

    async def worker(self, session: aiohttp.ClientSession):
        while True:
            url = await self._queue.get()
            if url is None:  # sentinel
                break
            await self.fetch(session, url)
            self._queue.task_done()

    async def run(self, start_id: int, end_id: int, *, reversed: bool = False, batch_size: int = 100):
        base = "https://habr.com/ru/articles/{}"

        async with aiohttp.ClientSession(headers={"User-Agent": "Mozilla/5.0"}) as session:
            tasks = [asyncio.create_task(self.worker(session))
                     for _ in range(self.workers)]

            if reversed:
                current_id = end_id
                while current_id >= start_id and not self._stopped:
                    if self._queue.qsize() < batch_size:
                        batch_start = max(
                            current_id - batch_size + 1, start_id)
                        for i in range(current_id, batch_start - 1, -1):
                            await self._queue.put(base.format(i))
                        current_id = batch_start - 1
                    await asyncio.sleep(0.1)
            else:
                current_id = start_id
                while current_id <= end_id and not self._stopped:
                    if self._queue.qsize() < batch_size:
                        batch_end = min(current_id + batch_size - 1, end_id)
                        for i in range(current_id, batch_end + 1):
                            await self._queue.put(base.format(i))
                        current_id = batch_end + 1
                    await asyncio.sleep(0.1)

            for _ in range(self.workers):
                await self._queue.put(None)

            await asyncio.gather(*tasks)


def parse_memory_val(mem: str):
    """ Переводит указанное значение памяти в байты """
    match mem[-2:]:
        case "kb":
            n = int(mem[:-2])
            n *= 1024
            return n
        case "mb":
            n = int(mem[:-2])
            n *= 1024 * 1024
            return n
        case "gb":
            n = int(mem[:-2])
            n *= 1024 * 1024 * 1024
            return n
        case _:
            return int(mem)


def parse_program_args():
    parser = argparse.ArgumentParser(description="Парсер статей habr.com")
    parser.add_argument("-w", "--workers",
                        type=int, default=1, help="Количество потоков")
    parser.add_argument("-o", "--output",
                        default=".", help="Папка для сохранения документов")

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("-m", "--memory",
                       help="Ограничения по размеру (X, Xkb, Xmb, Xgb)")
    group.add_argument("-n", "--num",
                       type=int, help="Точное количество статей")

    args = parser.parse_args()
    if args.memory is not None:
        args.memory = parse_memory_val(args.memory)
    return args


async def main():
    args = parse_program_args()

    loader = PageDownloader(StorageManager(
        args.output, number_limit=args.num, memory_limit=args.memory), workers=args.workers)
    await loader.run(1, 947506, reversed=True)


if __name__ == '__main__':
    asyncio.run(main())
