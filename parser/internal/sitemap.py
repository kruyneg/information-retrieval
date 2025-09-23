from urllib.robotparser import RobotFileParser
import aiohttp
from lxml import etree


IGNORE_LIST = [
    "https://www.geeksforgeeks.org/category/category-sitemap-1.xml",
    "https://www.geeksforgeeks.org/quizzes/quiz-sitemap-1.xml",
    "https://www.geeksforgeeks.org/quizzes/quiz-sitemap-2.xml",
    "https://www.geeksforgeeks.org/quizzes/quiz-sitemap-3.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-1.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-2.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-3.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-4.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-5.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-6.xml",
    "https://www.geeksforgeeks.org/videos/video-sitemap-7.xml",
]


class RobotManager:
    def __init__(self, url: str, user_agent: str):
        self.base_url = url.rstrip("/")
        self.user_agent = user_agent
        self._parser = RobotFileParser()
        self.crawl_delay = 1

    async def load(self):
        robots_url = self.base_url + "/robots.txt"
        async with aiohttp.ClientSession() as session:
            async with session.get(robots_url) as resp:
                if resp.status == 200:
                    text = await resp.text()
                    self._parser.parse(text.splitlines())
                    self.crawl_delay = self._parser.crawl_delay(
                        self.user_agent)
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


class SitemapFetcher:
    def __init__(self, url: str, user_agent: str = "SimpleCrawler"):
        self.base_url = url.rstrip("/")
        self._robot_manager = RobotManager(url, user_agent)
    
    def get_delay(self) -> int:
        return self._robot_manager.get_delay()

    async def urls(self):
        await self._robot_manager.load()

        sitemaps = self._robot_manager.get_sitemaps()
        for sm_url in sitemaps:
            async for url in self._parse_sitemap(sm_url):
                yield url

    async def _fetch(self, url):
        async with aiohttp.ClientSession() as session:
            async with session.get(url) as resp:
                if resp.status == 200:
                    text = await resp.text()
                    return text
                else:
                    raise RuntimeError("sitemap not found")

    async def _parse_sitemap(self, url):
        text = await self._fetch(url)
        
        root = etree.fromstring(text.encode())
        ns = {"sm": "http://www.sitemaps.org/schemas/sitemap/0.9"}
        
        for sitemap in root.xpath("//sm:sitemap/sm:loc/text()", namespaces=ns):
            if sitemap in IGNORE_LIST:
                continue
            async for u in self._parse_sitemap(sitemap.strip()):
                yield u
        
        for loc in root.xpath("//sm:url/sm:loc/text()", namespaces=ns):
            loc = loc.strip()
            if self._robot_manager.is_allowed(loc):
                yield loc
