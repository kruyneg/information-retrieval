from pymongo import MongoClient, UpdateOne
from typing import Iterable
from tqdm import tqdm

MONGO_URI = "mongodb://user:password@localhost:27017/"
DB_NAME = "parser_db"
COLLECTION_NAME = "articles"
BATCH_SIZE = 100

def iter_articles(collection) -> Iterable[dict]:
    return collection.find(
        {},
        projection={"_id": 1},
        no_cursor_timeout=True
    ).sort("_id", 1)

def main():
    client = MongoClient(MONGO_URI)
    db = client[DB_NAME]
    collection = db[COLLECTION_NAME]

    bulk_ops = []
    processed = 0
    cursor = iter_articles(collection)

    try:
        for doc_id, article in enumerate(tqdm(cursor, desc="Assigning doc_id")):
            bulk_ops.append(
                UpdateOne(
                    {"_id": article["_id"]},
                    {"$set": {"doc_id": doc_id}},
                )
            )

            if len(bulk_ops) >= BATCH_SIZE:
                collection.bulk_write(bulk_ops, ordered=False)
                processed += len(bulk_ops)
                bulk_ops.clear()

        if bulk_ops:
            collection.bulk_write(bulk_ops, ordered=False)
            processed += len(bulk_ops)

    finally:
        cursor.close()

    print(f"Done. Assigned doc_id to {processed} documents.")

if __name__ == "__main__":
    main()
