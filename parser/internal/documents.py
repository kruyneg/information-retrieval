from dataclasses import dataclass, field
from bs4 import BeautifulSoup
from time import time
from urllib.parse import urlparse


@dataclass
class Document:
    url: str
    html: str
    text: str | None = None
    created_at: float = field(default_factory=lambda: time())
    site: str = field(init=False)

    def __post_init__(self):
        self.site = urlparse(self.url).netloc
        

@dataclass
class HabrDocument:
    url: str
    title: str
    text: str

    @staticmethod
    def parse(url: str, html_text: str) -> "HabrDocument":
        """
        Разбор HTML и создание объекта Document.
        Бросает ValueError, если данные неполные.
        """
        soup = BeautifulSoup(html_text, "lxml")

        title_tag = soup.find("h1")
        title = title_tag.get_text(strip=True) if title_tag else None

        body = soup.find("div", class_="article-formatted-body")
        text = body.get_text(separator="\n", strip=True) if body else None

        if not text and not title:
            raise ValueError(f"не достаточно данных")

        return HabrDocument(url=url, title=title, text=text)


@dataclass
class GeeksDocument:
    url: str
    title: str
    text: str

    @staticmethod
    def parse(url: str, html_text: str) -> "HabrDocument":
        """
        Разбор HTML и создание объекта Document.
        Бросает ValueError, если данные неполные.
        """
        soup = BeautifulSoup(html_text, "lxml")

        title_tag = soup.find("h1")
        title = title_tag.get_text(strip=True) if title_tag else None

        text_div = soup.find('div', class_='text')
        article_text = text_div.get_text(strip=True) if text_div else None

        if not article_text and not title:
            raise ValueError(f"не достаточно данных")

        return GeeksDocument(url=url, title=title, text=article_text)


def parse_document(url: str, html: str) -> HabrDocument | GeeksDocument:
    if url.startswith("https://habr.com"):
        return HabrDocument.parse(url, html)
    elif url.startswith("https://www.geeksforgeeks.org"):
        return GeeksDocument.parse(url, html)
    else:
        raise ValueError(f"unknown site {url}")
