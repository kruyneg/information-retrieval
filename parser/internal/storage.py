import asyncio
from motor.motor_asyncio import AsyncIOMotorClient
from dataclasses import asdict
from tqdm import tqdm

from internal.documents import Document


class StorageManager:
    def __init__(self, mongo_url: str,
                 db_name: str = "parser_db", collection: str = "articles",
                 *, number_limit: int = None):
        self._numlim = number_limit

        self._pbar = None
        if self._numlim:
            self._pbar = tqdm(total=self._numlim,
                              unit="doc", position=1)
            self._statbar = tqdm(total=0, bar_format="{desc}", position=0)

        self._client = AsyncIOMotorClient(mongo_url)
        self._col = self._client[db_name][collection]

        self.on_limit_reached = None
        self._count = 0

    async def save(self, doc: Document):
        if self._limit_is_reached():
            return

        if self._pbar:
            self._statbar.set_description(f"Saving {doc.url}")
            self._statbar.refresh()

        await self._col.update_one(
            {"url": doc.url},
            {"$set": asdict(doc)},
            upsert=True)
        self._count += 1

        if self._pbar:
            self._pbar.update(1)
        self._update_limits()

    async def check_connection(self):
        await asyncio.wait_for(self._client.admin.command("ping"),
                               timeout=2)

    def _limit_is_reached(self) -> bool:
        return self._numlim is not None and self._count >= self._numlim

    def _update_limits(self):
        if self._limit_is_reached():
            if self._pbar:
                self._pbar.close()
            if self.on_limit_reached:
                self.on_limit_reached()
