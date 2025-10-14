import asyncio
import aiohttp
import logging
import signal

from internal.config import config
from internal.documents import Document
from internal.fetchurl import UrlFetcher
from internal.storage import StorageManager


logger = logging.getLogger(__name__)

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
    def __init__(self, fetchers: list["UrlFetcher"], storage: "StorageManager",
                 *, workers: int = 4, batch_size: int = 5):
        self.url_fetchers = fetchers
        self.storage = storage
        self._workers_num = workers
        self._queue = asyncio.Queue(maxsize=batch_size)
        self._stopped = asyncio.Event()
        self._limiters: dict[str, RateLimiter] = {
            sm.base_url: RateLimiter(sm.get_delay()) for sm in self.url_fetchers
        }
        for fetcher in self.url_fetchers:
            fetcher.mongo = storage

    async def stop(self):
        """
        Вызывается при завершении программы,
        чтобы корректно завершить всех воркеров
        """
        logger.error("Graceful shutdown initiated (not a error)")
        self._stopped.set()

        await self._queue.join()

    async def run(self):
        loop = asyncio.get_running_loop()
        for s in (signal.SIGINT, signal.SIGTERM):
            loop.add_signal_handler(
                s, lambda: asyncio.create_task(self.stop()))

        await self.storage.check_connection()
        async with aiohttp.ClientSession(headers={"User-Agent": config.crawler.user_agent}) as session:
            tasks = [asyncio.create_task(self._worker(session))
                     for _ in range(self._workers_num)]

            producers = [asyncio.create_task(self._produce(sm))
                         for sm in self.url_fetchers]

            await asyncio.gather(*producers)

            for _ in range(self._workers_num):
                await self._queue.put(None)

            await asyncio.gather(*tasks)

    async def _produce(self, sitemap: UrlFetcher):
        base = sitemap.base_url
        async for url in sitemap.urls():
            await self._queue.put((url, base))
            if self._stopped.is_set():
                break

    async def _worker(self, session: aiohttp.ClientSession):
        while True:
            item = await self._queue.get()
            if item is None:
                break

            url, base = item
            limiter = self._limiters[base]
            await limiter.wait()

            await self._fetch(session, url)
            self._queue.task_done()

    async def _fetch(self, session: aiohttp.ClientSession, url: str):
        try:
            async with session.get(url) as resp:
                if resp.status == 200:
                    html = await resp.text()
                    try:
                        doc = Document(url, html)
                        logger.debug(f"Saving {url}")
                        await self.storage.save(doc)

                    except ValueError as e:
                        logger.error(
                            f"[PARSE ERROR] {url}: {e}")
                    except Exception as e:
                        logger.error(
                            f"{type(e)} {url}: {e}")
                elif resp.status == 404:
                    pass
                else:
                    logger.error(f"[HTTP {resp.status}] {url}")

        except Exception as e:
            logger.error(f"[NETWORK ERROR] {url}: {e}")
