import os
import bz2
import urllib.request
import xml.etree.ElementTree as ET
from nltk.corpus import wordnet as wn
import nltk


OPENCORPORA_URL = "http://opencorpora.org/files/export/dict/dict.opcorpora.xml.bz2"
LOCAL_RU_BZ2 = "dict.opcorpora.xml.bz2"
LOCAL_RU_XML = "dict.opcorpora.xml"
RU_OUTFILE = "ru_lemmas.txt"

EN_OUTFILE = "en_lemmas.txt"


def download_open_corpora():
    if not os.path.exists(LOCAL_RU_BZ2):
        print("Downloading OpenCorpora…")
        urllib.request.urlretrieve(OPENCORPORA_URL, LOCAL_RU_BZ2)
    else:
        print("OpenCorpora archive already downloaded.")

    if not os.path.exists(LOCAL_RU_XML):
        with bz2.open(LOCAL_RU_BZ2, "rb") as fr, open(LOCAL_RU_XML, "wb") as fw:
            print("Unpacking OpenCorpora XML…")
            fw.write(fr.read())
    else:
        print("OpenCorpora XML already exists.")


def build_russian_dict():
    print("Parsing OpenCorpora XML…")
    tree = ET.parse(LOCAL_RU_XML)
    root = tree.getroot()

    with open(RU_OUTFILE, "w", encoding="utf-8") as fout:
        for lemma_el in root.findall(".//lemma"):
            lemma_text = lemma_el.get("t") or lemma_el.find("l").get("t")
            for form in lemma_el.findall(".//f"):
                wf = form.get("t")
                fout.write(f"{wf}\t{lemma_text}\n")
    print(f"Russian dict saved to '{RU_OUTFILE}'")


def build_english_dict():
    print("Downloading WordNet (NLTK)…")
    nltk.download("wordnet")
    nltk.download("omw-1.4")

    with open(EN_OUTFILE, "w", encoding="utf-8") as fout:
        for synset in wn.all_synsets():
            for lemma in synset.lemmas():
                wordform = lemma.name().replace("_", " ")
                lemma_norm = lemma.key().split("%")[0].replace("_", " ")
                fout.write(f"{wordform}\t{lemma_norm}\n")

    print(f"English dict saved to '{EN_OUTFILE}'")


if __name__ == "__main__":
    os.makedirs("data", exist_ok=True)
    os.chdir("data")

    download_open_corpora()
    build_russian_dict()
    build_english_dict()
