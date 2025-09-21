from pathlib import Path
import argparse
from statistics import mean, median
import json
import sys


class StatisticsCollector:
    def __init__(self, input: str, calc_text: bool = True):
        self._path = Path(input).expanduser().resolve()
        if not self._path.exists() or not self._path.is_dir():
            raise ValueError(
                f"путь {input} не существует или не является директорией")
        self._calc_text = calc_text
        self._collect()

    def _collect(self):
        self._doc_sizes = []
        self._file_sizes = []

        for file in self._path.iterdir():
            if self._calc_text:
                if file.is_file() and file.suffix == ".json":
                    self._file_sizes.append(file.stat().st_size)
                    try:
                        with open(file, "r", encoding="utf-8") as f:
                            data = json.load(f)
                            text = data.get("text", "")
                            self._doc_sizes.append(len(text))
                    except Exception as e:
                        print(f"Ошибка чтения {file}: {e}", file=sys.stderr)
            else:
                self._file_sizes.append(file.stat().st_size)

        if not self._file_sizes:
            raise ValueError("не найдено ни одного документа для статистики")

        self.total_docs = len(self._file_sizes)
        self.total_bytes = sum(self._file_sizes)
        if self._calc_text:
            self.avg_text_len = mean(self._doc_sizes)
            self.median_text_len = median(self._doc_sizes)
            self.min_text_len = min(self._doc_sizes)
            self.max_text_len = max(self._doc_sizes)
            self.p90_text_len = sorted(self._doc_sizes)[
                int(0.9 * self.total_docs) - 1]

        self.min_file_size = min(self._file_sizes)
        self.max_file_size = max(self._file_sizes)
        self.avg_file_size = mean(self._file_sizes)

    def __repr__(self):
        width = 40
        def line(char="+"): return char + "-" * (width - 1) + char
        def row(label, value): return f"| {label:<25} {value:>12} |"

        lines = [
            line(),
            row("Всего документов", self.total_docs),
            row("Общий размер (байт)", self.total_bytes),
            row("Общий размер (МБ)", f"{self.total_bytes / 1024**2:.2f}"),
            line(),
        ]
        if self._calc_text:
            lines += [
                row("Длина текста (символы)", ""),
                row("Среднее", f"{self.avg_text_len:.2f}"),
                row("Медиана", self.median_text_len),
                row("Минимум", self.min_text_len),
                row("Максимум", self.max_text_len),
                row("90-й перцентиль", self.p90_text_len),
                row("Всего символов", sum(self._doc_sizes)),
                line(),
            ]
        lines += [
            row("Размер файлов (байт)", ""),
            row("Средний", f"{self.avg_file_size:.2f}"),
            row("Минимум", self.min_file_size),
            row("Максимум", self.max_file_size),
            row("Средний (КБ)", f"{self.avg_file_size / 1024:.2f}"),
            row("Средний (МБ)", f"{self.avg_file_size / 1024**2:.2f}"),
            line()
        ]
        return "\n".join(lines)


def parse_program_args():
    parser = argparse.ArgumentParser(
        description="Показывает статистику по скачанным страницам")
    parser.add_argument("-i", "--input",
                        default=".", help="Путь к директории, где сохранены документы")
    parser.add_argument("--only-size", action="store_true", help="Анализировать только размер файлов (без текста)")

    args = parser.parse_args()
    return args


def main():
    args = parse_program_args()
    stats = StatisticsCollector(args.input, not args.only_size)
    print(stats)


if __name__ == '__main__':
    main()
