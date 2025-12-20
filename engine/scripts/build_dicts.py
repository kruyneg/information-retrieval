import os
import bz2
import urllib.request
import xml.etree.ElementTree as ET
import struct

OPENCORPORA_URL = "http://opencorpora.org/files/export/dict/dict.opcorpora.xml.bz2"
LOCAL_RU_BZ2 = "dict.opcorpora.xml.bz2"
LOCAL_RU_XML = "dict.opcorpora.xml"
RU_OUTFILE = "ru_lemmas.txt"
RU_BIN = "ru_lemmas.bin"

UNIMORPH_EN_URL = "https://raw.githubusercontent.com/unimorph/eng/master/eng"
EN_RAW = "eng.txt"
EN_OUTFILE = "en_lemmas.txt"
EN_BIN = "en_lemmas.bin"


class BinaryDictWriter:
    def __init__(self, filename):
        print(f"Opening binary writer: {filename}")
        self.f = open(filename, "wb")
        self.count = 0
        self.f.write(struct.pack("<I", 0))

    def write_pair(self, word, lemma):
        w = word.encode("utf-8")
        l = lemma.encode("utf-8")
        self.f.write(struct.pack("<H", len(w)))
        self.f.write(w)
        self.f.write(struct.pack("<H", len(l)))
        self.f.write(l)
        self.count += 1

    def close(self):
        print(f"Finalizing binary file, entries: {self.count}")
        self.f.seek(0)
        self.f.write(struct.pack("<I", self.count))
        self.f.close()


def unpack_bz2(src, dst, buf_size=1024 * 1024):
    print(f"Unpacking {src} → {dst}")
    with bz2.open(src, "rb") as fr, open(dst, "wb") as fw:
        while True:
            chunk = fr.read(buf_size)
            if not chunk:
                break
            fw.write(chunk)


def download_open_corpora():
    print("Checking OpenCorpora archive")
    if not os.path.exists(LOCAL_RU_BZ2):
        print("Downloading OpenCorpora bz2")
        urllib.request.urlretrieve(OPENCORPORA_URL, LOCAL_RU_BZ2)
    else:
        print("OpenCorpora bz2 already exists")

    if not os.path.exists(LOCAL_RU_XML):
        unpack_bz2(LOCAL_RU_BZ2, LOCAL_RU_XML)
    else:
        print("OpenCorpora XML already exists")


def build_russian_dict():
    print("Building Russian dictionary (streaming XML)")
    txt = open(RU_OUTFILE, "w", encoding="utf-8")
    binw = BinaryDictWriter(RU_BIN)

    context = ET.iterparse(LOCAL_RU_XML, events=("end",))
    lemma_count = 0

    for event, elem in context:
        if elem.tag != "lemma":
            continue

        lemma = elem.get("t") or elem.find("l").get("t")
        for form in elem.findall("f"):
            word = form.get("t")
            txt.write(f"{word}\t{lemma}\n")
            binw.write_pair(word, lemma)

        elem.clear()
        lemma_count += 1
        if lemma_count % 10000 == 0:
            print(f"Processed {lemma_count} lemmas")

    txt.close()
    binw.close()
    print("Russian dictionary completed")


def download_unimorph_en():
    print("Checking UniMorph English")
    if not os.path.exists(EN_RAW):
        print("Downloading UniMorph English")
        urllib.request.urlretrieve(UNIMORPH_EN_URL, EN_RAW)
    else:
        print("UniMorph English already exists")


def build_english_dict():
    print("Building English dictionary (UniMorph)")
    txt = open(EN_OUTFILE, "w", encoding="utf-8")
    binw = BinaryDictWriter(EN_BIN)

    line_count = 0
    with open(EN_RAW, "r", encoding="utf-8") as fin:
        for line in fin:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            parts = line.split("\t")
            if len(parts) < 2:
                continue

            lemma = parts[0].lower()
            wordform = parts[1].lower()

            txt.write(f"{wordform}\t{lemma}\n")
            binw.write_pair(wordform, lemma)

            line_count += 1
            if line_count % 100000 == 0:
                print(f"Processed {line_count} lines")

    txt.close()
    binw.close()
    print("English dictionary completed")


if __name__ == "__main__":
    print("Starting dictionary build pipeline")
    os.makedirs("data", exist_ok=True)
    os.chdir("data")

    download_open_corpora()
    build_russian_dict()
    download_unimorph_en()
    build_english_dict()

    print("All done")
