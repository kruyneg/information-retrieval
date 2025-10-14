import argparse
import asyncio
import logging

from internal.logger import setup_logger
from internal.config import load_config, config
from internal.storage import StorageManager
from internal.loader import PageDownloader
from internal.fetchurl import UrlFetcher


def parse_program_args():
    parser = argparse.ArgumentParser(
        description="Загрузчик статей для сайтов с sitemap.xml")
    parser.add_argument("config", help="Путь к yaml файлу конфигурации")

    args = parser.parse_args()
    return args


async def main():
    args = parse_program_args()
    load_config(args.config)
    setup_logger()

    logger = logging.getLogger(__name__)
    logger.info("Crawler Started")

    fetchers = [UrlFetcher("https://www.geeksforgeeks.org"),
                UrlFetcher("https://habr.com")]
    for f in fetchers:
        await f.load()

    storage = StorageManager()

    loader = PageDownloader(fetchers, storage, workers=config.workers)
    await loader.run()


if __name__ == '__main__':
    asyncio.run(main())
