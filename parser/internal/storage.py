import asyncio
from motor.motor_asyncio import AsyncIOMotorClient
from dataclasses import asdict

from internal.documents import Document


class StorageManager:
    def __init__(self, mongo_url: str,
                 db_name: str = "parser_db", collection: str = "articles",
                 *, number_limit: int = None):
        self._numlim = number_limit


        self._client = AsyncIOMotorClient(mongo_url)
        self._col = self._client[db_name][collection]
        self._progress_col = self._client[db_name]["url_progress"]

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
            {"$set": {"last_url": url}}
        )
