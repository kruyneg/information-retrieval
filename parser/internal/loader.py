import asyncio
import aiohttp
import sys
from tqdm import tqdm

from internal.documents import parse_document
from internal.sitemap import SitemapFetcher
from internal.storage import StorageManager


def log_err(msg: str):
    tqdm.write(f"{msg}", file=sys.stderr)


class RateLimiter:
    def __init__(self, delay: float):
        self.delay = delay
        self._lock = asyncio.Lock()
        self._last_call = 0.0

    async def wait(self):
        async with self._lock:
            now = asyncio.get_event_loop().time()
            wait_time = self._last_call + self.delay - now
            if wait_time > 0:
                await asyncio.sleep(wait_time)
            self._last_call = asyncio.get_event_loop().time()


class PageDownloader:
    def __init__(self, sitemaps: list["SitemapFetcher"], storage: "StorageManager",
                 *, workers: int = 4, batch_size: int = 100):
        self.sitemaps = sitemaps
        self.storage = storage
        self._workers_num = workers
        self._queue = asyncio.Queue(maxsize=batch_size)
        self._stopped = asyncio.Event()
        self._limiters: dict[str, RateLimiter] = {
            sm.base_url: RateLimiter(sm.get_delay()) for sm in self.sitemaps
        }

        self.storage.on_limit_reached = self.stop

    def stop(self):
        """ Вызывается при достижении лимита, чтобы завершить всех воркеров """
        self._stopped.set()

        while not self._queue.empty():
            self._queue.get_nowait()

    async def run(self):
        await self.storage.check_connection()
        async with aiohttp.ClientSession(headers={"User-Agent": "Mozilla/5.0"}) as session:
            tasks = [asyncio.create_task(self._worker(session))
                     for _ in range(self._workers_num)]

            producers = [asyncio.create_task(self._produce(sm))
                         for sm in self.sitemaps]

            await asyncio.gather(*producers)

            for _ in range(self._workers_num):
                await self._queue.put(None)

            await asyncio.gather(*tasks)

    async def _produce(self, sitemap: SitemapFetcher):
        base = sitemap.base_url
        async for url in sitemap.urls():
            if self._stopped.is_set():
                break
            await self._queue.put((url, base))

    async def _worker(self, session: aiohttp.ClientSession):
        while True:
            item = await self._queue.get()
            if item is None:  # sentinel
                break

            url, base = item
            limiter = self._limiters[base]
            await limiter.wait()
            if self._stopped.is_set():
                break

            await self._fetch(session, url)
            self._queue.task_done()

    async def _fetch(self, session: aiohttp.ClientSession, url: str):
        try:
            async with session.get(url) as resp:
                if resp.status == 200:
                    html = await resp.text()
                    try:
                        doc = parse_document(url, html)

                        await self.storage.save(doc)

                    except ValueError as e:
                        log_err(
                            f"[PARSE ERROR] {url}: {e}")
                    except Exception as e:
                        log_err(
                            f"[ERROR] {type(e)} {url}: {e}")
                elif resp.status == 404:
                    pass
                else:
                    log_err(f"[HTTP {resp.status}] {url}")

        except Exception as e:
            log_err(f"[NETWORK ERROR] {url}: {e}")
