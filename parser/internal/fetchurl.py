from urllib.robotparser import RobotFileParser
import aiohttp
from lxml import etree

from internal.config import config
from internal.storage import StorageManager


class RobotManager:
    def __init__(self, url: str, user_agent: str):
        self.base_url = url.rstrip("/")
        self.user_agent = user_agent
        self._parser = RobotFileParser()
        self.crawl_delay = config.crawler.default_delay

    async def load(self):
        robots_url = self.base_url + "/robots.txt"
        async with aiohttp.ClientSession() as session:
            async with session.get(robots_url) as resp:
                if resp.status == 200:
                    text = await resp.text()
                    self._parser.parse(text.splitlines())
                    delay = self._parser.crawl_delay(
                        self.user_agent)
                    # if delay is not None:
                    #     self.crawl_delay = delay
                else:
                    self._parser = None

    def is_allowed(self, url: str) -> bool:
        if self._parser is None:
            return True
        return self._parser.can_fetch(self.user_agent, url)

    def get_sitemaps(self) -> list[str]:
        if self._parser is None or self._parser.site_maps() is None:
            return [self.base_url + "/sitemap.xml"]
        return self._parser.site_maps()

    def get_delay(self) -> int:
        return self.crawl_delay


class UrlFetcher:
    def __init__(self, url: str, storage: StorageManager | None = None,
                 user_agent: str = "SimpleCrawler"):
        self.base_url = url.rstrip("/")
        self._robot_manager = RobotManager(url, config.crawler.user_agent)
        self.mongo = storage
        
    async def load(self):
        await self._robot_manager.load()

    def get_delay(self) -> int:
        return self._robot_manager.get_delay()

    async def urls(self):
        last_url = None
        if self.mongo is not None:
            last_url = await self.mongo.get_last_url(self.base_url)

        sitemaps = self._robot_manager.get_sitemaps()
        for sm_url in sitemaps:
            async for url in self._parse_sitemap(sm_url, last_url):
                yield url

    async def _fetch(self, url):
        async with aiohttp.ClientSession() as session:
            async with session.get(url) as resp:
                if resp.status == 200:
                    text = await resp.text()
                    return text
                else:
                    raise RuntimeError("sitemap not found")

    async def _parse_sitemap(self, url, last_url=None):
        text = await self._fetch(url)
        root = etree.fromstring(text.encode())
        ns = {"sm": "http://www.sitemaps.org/schemas/sitemap/0.9"}

        skip = last_url is not None
        for sitemap in root.xpath("//sm:sitemap/sm:loc/text()", namespaces=ns):
            if sitemap in config.crawler.ignore:
                continue
            async for u in self._parse_sitemap(sitemap.strip()):
                if skip and u == last_url:
                    skip = False
                    continue
                if not skip:
                    yield u

        for loc in root.xpath("//sm:url/sm:loc/text()", namespaces=ns):
            loc = loc.strip()
            if skip:
                if loc == last_url:
                    skip = False
                continue
            if self._robot_manager.is_allowed(loc):
                await self.mongo.update_last_url(self.base_url, loc)
                yield loc
