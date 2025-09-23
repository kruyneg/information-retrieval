import argparse
import asyncio

from internal.storage import StorageManager
from internal.loader import PageDownloader
from internal.sitemap import SitemapFetcher


def parse_program_args():
    parser = argparse.ArgumentParser(
        description="Парсер статей для сайтов с sitemap.xml")
    parser.add_argument("-w", "--workers",
                        type=int, default=4, help="Количество потоков")
    parser.add_argument("-u", "--mongo-url",
                        default="mongodb://localhost:27017", help="URL для MongoDB")
    parser.add_argument("-n", "--num",
                        type=int, help="Ограничение на количество статей")

    args = parser.parse_args()
    return args


async def main():
    args = parse_program_args()

    sitemaps = [SitemapFetcher("https://www.geeksforgeeks.org"),
                SitemapFetcher("https://habr.com")]
    storage = StorageManager(args.mongo_url, number_limit=args.num)

    loader = PageDownloader(sitemaps, storage, workers=args.workers)
    await loader.run()


if __name__ == '__main__':
    asyncio.run(main())
