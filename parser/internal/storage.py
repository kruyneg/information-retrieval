import asyncio
from motor.motor_asyncio import AsyncIOMotorClient
from dataclasses import asdict

from internal.config import config
from internal.documents import Document


class StorageManager:
    def __init__(self):
        self._client = AsyncIOMotorClient(config.mongo.url)
        self._col = self._client[config.mongo.db][config.mongo.collection]
        self._progress_col = \
            self._client[config.mongo.db][config.mongo.progress_collection]

    async def save(self, doc: Document):
        await self._col.update_one(
            {"url": doc.url},
            {"$set": asdict(doc)},
            upsert=True)

    async def check_connection(self):
        await asyncio.wait_for(self._client.admin.command("ping"),
                               timeout=2)

    async def get_last_url(self, base_url: str) -> str | None:
        doc = await self._progress_col.find_one({"_id": base_url})
        return doc["last_url"] if doc else None

    async def update_last_url(self, base_url: str, url: str):
        await self._progress_col.update_one(
            {"_id": base_url},
            {"$set": {"last_url": url}},
            upsert=True
        )
