from dataclasses import dataclass, field
from pathlib import Path
import yaml


@dataclass
class MongoConfig:
    url: str = "mongodb://localhost:27017"
    db: str = "parser_db"
    collection: str = "articles"
    progress_collection: str = "url_progress"


@dataclass
class CrawlerConfig:
    user_agent: str = "SimpleCrawler"
    default_delay: int = 5
    ignore: list[str] = field(default_factory=list)


@dataclass
class Config:
    log_level: str = "error"
    workers: int = 1

    mongo: MongoConfig = field(default_factory=MongoConfig)
    crawler: CrawlerConfig = field(default_factory=CrawlerConfig)


config = Config()


def load_config(config_path: Path | str = "config.yaml"):
    path = Path(config_path)
    if not path.exists():
        print(f"Config file not found, using defaults: {path}")
        return

    with open(path, 'r', encoding='utf-8') as f:
        data = yaml.safe_load(f) or {}

    mongo = {**MongoConfig().__dict__,  **data.get("mongo", {})}
    crawler = {**CrawlerConfig().__dict__, **data.get("crawler", {})}

    top_level_defaults = {
        "log_level": "error",
        "workers": 1,
    }
    top_level_data = {**top_level_defaults, **{
        k: v for k, v in data.items() if k in top_level_defaults
    }}

    global config
    config.mongo = MongoConfig(**mongo)
    config.crawler = CrawlerConfig(**crawler)
    config.log_level = top_level_data["log_level"]
    config.workers = top_level_data["workers"]
