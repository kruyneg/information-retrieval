from pymongo import MongoClient, UpdateOne
from typing import Iterable
from tqdm import tqdm

from internal.documents import Document, parse_document

MONGO_URI = "mongodb://user:password@localhost:27017/"
DB_NAME = "parser_db"
COLLECTION_NAME = "articles"

BATCH_SIZE = 100


def iter_articles(collection) -> Iterable[dict]:

    return collection.find(
        {
            "text": {"$exists": False},
            "html": {"$exists": True},
            "url": {"$exists": True},
        },
        projection={"url": 1, "html": 1},
        no_cursor_timeout=True,
    )


def main():
    client = MongoClient(MONGO_URI)
    db = client[DB_NAME]
    collection = db[COLLECTION_NAME]

    bulk_ops = []
    processed = 0

    cursor = iter_articles(collection)

    try:
        for article in tqdm(cursor, desc="Parsing documents"):
            url = article["url"]
            html = article["html"]

            try:
                parsed = parse_document(url, html)
                text = ''
                if parsed.title:
                    text += parsed.title + '\n'
                if parsed.text:
                    text += parsed.text
                text = text.strip()

            except Exception as e:
                print(f"[WARN] Failed to parse {url}: {e}")
                continue

            bulk_ops.append(
                UpdateOne(
                    {"_id": article["_id"]},
                    {"$set": {"text": text}},
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

    print(f"Done. Parsed {processed} documents.")


if __name__ == "__main__":
    main()
