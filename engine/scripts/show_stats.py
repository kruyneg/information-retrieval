import struct
from collections import Counter
import matplotlib.pyplot as plt


INDEX_FILE = "data/index.bin"


def decode_vbyte_at(data, start_idx=0):
    n = 0
    shift = 0
    idx = start_idx
    while idx < len(data):
        byte = data[idx]
        n |= (byte & 0x7F) << shift
        idx += 1
        if byte & 0x80:
            break
        shift += 7
    return n, idx


def decode_vbyte(data):
    numbers = []
    n = 0
    shift = 0
    for byte in data:
        if byte & 0x80:
            n |= (byte & 0x7F) << shift
            numbers.append(n)
            n = 0
            shift = 0
        else:
            n |= (byte & 0x7F) << shift
            shift += 7
    return numbers


def read_index(filename):
    term_freqs = {}
    with open(filename, "rb") as f:
        terms_count = struct.unpack("Q", f.read(8))[0]
        for _ in range(terms_count):
            term_size = struct.unpack("Q", f.read(8))[0]
            term = f.read(term_size).decode("utf-8")

            list_size = struct.unpack("Q", f.read(8))[0]

            doc_buf_size = struct.unpack("Q", f.read(8))[0]
            doc_buffer = f.read(doc_buf_size)

            coord_buf_size = struct.unpack("Q", f.read(8))[0]
            coord_buffer = f.read(coord_buf_size)

            global_tf = 0
            for i, val in enumerate(decode_vbyte(doc_buffer)):
                if i % 2 == 1:
                    tf, _ = decode_vbyte_at(coord_buffer, val)
                    global_tf += tf
            term_freqs[term] = global_tf
            print("len(term_freqs)=", len(term_freqs), end='\r')
    return term_freqs


def compute_zipf(term_freqs):
    freq = Counter(term_freqs)

    sorted_terms = freq.most_common()

    ranks = list(range(1, len(sorted_terms) + 1))
    frequencies = [tf for _, tf in sorted_terms]

    return ranks, frequencies


def plot_zipf(ranks, freqs):
    plt.figure(figsize=(8, 6))
    plt.loglog(ranks, freqs, marker='.')
    plt.xlabel("Rank")
    plt.ylabel("Frequency")
    plt.title("Zipf's law for corpus")
    plt.grid(True, which="both", ls="--")
    plt.show()


if __name__ == "__main__":
    index = read_index(INDEX_FILE)
    ranks, freqs = compute_zipf(index)
    plot_zipf(ranks, freqs)
