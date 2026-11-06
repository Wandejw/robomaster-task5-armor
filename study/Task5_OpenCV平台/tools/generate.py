#!/usr/bin/env python3
"""从 ImageProcessing100Wen 生成离线学习平台数据。

脚本只读取原始题库，仅向 Task5_OpenCV平台/data 和 assets 写入。
当前平台只保留 C++ 答案，不读取 Python 答案。
"""

from __future__ import annotations

import json
import re
import shutil
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[1]
SOURCE_ROOT = PROJECT_ROOT / "ImageProcessing100Wen"
DATA_DIR = PROJECT_ROOT / "data"
IMAGE_DIR = PROJECT_ROOT / "assets" / "images"
CODE_DIR = PROJECT_ROOT / "assets" / "codes"

IMPLEMENT_QUESTIONS = {1, 2, 3, 5, 7, 9, 10, 11}
DEEP_READ_QUESTIONS = {15, 17, 43}
UNDERSTAND_QUESTIONS = {4, 16, 20, 21, 22, 23, 25, 26, 27, 28, 29, 30}
SELECTED_QUESTIONS = (
    IMPLEMENT_QUESTIONS | DEEP_READ_QUESTIONS | UNDERSTAND_QUESTIONS
)

QUESTION_DIR_PATTERN = re.compile(r"Question_(\d+)_(\d+)$")
QUESTION_HEADING_PATTERN = re.compile(r"^##\s+问题[^\n]+$", re.MULTILINE)
MARKDOWN_IMAGE_PATTERN = re.compile(r"!\[[^\]]*\]\((?:<)?([^\s)>]+)")
HTML_IMAGE_PATTERN = re.compile(
    r"<img\b[^>]*?\bsrc=[\"']([^\"']+)[\"']", re.IGNORECASE
)


def difficulty_for(question_id: int) -> str:
    if question_id in IMPLEMENT_QUESTIONS:
        return "implement"
    if question_id in DEEP_READ_QUESTIONS:
        return "deep-read"
    return "understand"


def strip_answer_block(markdown: str) -> str:
    """详情页有独立 C++ 入口，移除原 README 末尾的 Python/C++ 答案链接块。"""
    return re.sub(r"\n?>\s*(?:[-*]\s*)?答案[\s\S]*$", "", markdown).strip()


def clean_title(heading: str) -> str:
    """把 `## 问题二：灰度化` 转换为适合列表展示的标题。"""
    raw = re.sub(r"^##\s+问题", "", heading).strip()
    parts = re.split(r"[：:]", raw, maxsplit=1)
    title = parts[1] if len(parts) == 2 else raw
    title = re.sub(r"\[\^[^\]]+\]", "", title)
    title = re.sub(r"\\text\{([^{}]+)\}", r"\1", title)
    title = title.replace("$", "")
    title = title.lstrip("：: ")
    return re.sub(r"\s+", " ", title).strip()


def question_sections(readme_text: str) -> list[tuple[str, str]]:
    """依据二级题目标题切分 README，保留每题的 Markdown 原文。"""
    matches = list(QUESTION_HEADING_PATTERN.finditer(readme_text))
    sections: list[tuple[str, str]] = []

    for index, match in enumerate(matches):
        end = matches[index + 1].start() if index + 1 < len(matches) else len(readme_text)
        heading = match.group(0).strip()
        body = readme_text[match.end() : end].strip()
        sections.append((heading, body))

    return sections


def find_image_sources(markdown: str) -> list[str]:
    candidates = MARKDOWN_IMAGE_PATTERN.findall(markdown)
    candidates.extend(HTML_IMAGE_PATTERN.findall(markdown))
    unique: list[str] = []

    for candidate in candidates:
        candidate = candidate.strip()
        if candidate.startswith(("http://", "https://", "data:")):
            continue
        if candidate not in unique:
            unique.append(candidate)

    return unique


def copy_question_images(
    question_id: int, markdown: str, readme_dir: Path
) -> tuple[list[str], dict[str, str]]:
    copied: list[str] = []
    image_map: dict[str, str] = {}
    destination_dir = IMAGE_DIR / f"question_{question_id:03d}"

    for source_text in find_image_sources(markdown):
        source = (readme_dir / source_text).resolve()
        if not source.is_file():
            source = (SOURCE_ROOT / source_text).resolve()
        try:
            source.relative_to(SOURCE_ROOT.resolve())
        except ValueError:
            continue
        if not source.is_file():
            continue

        destination_dir.mkdir(parents=True, exist_ok=True)
        destination = destination_dir / source.name
        counter = 2
        while destination.exists() and destination.read_bytes() != source.read_bytes():
            destination = destination_dir / f"{source.stem}_{counter}{source.suffix}"
            counter += 1
        shutil.copy2(source, destination)
        relative_destination = destination.relative_to(PROJECT_ROOT).as_posix()
        copied.append(relative_destination)
        image_map[source_text] = relative_destination

    return copied, image_map


def read_cpp(question_dir: Path, question_id: int) -> tuple[str, str | None]:
    source = question_dir / "answers_cpp" / f"answer_{question_id}.cpp"
    if not source.is_file():
        return "", None

    code = source.read_text(encoding="utf-8", errors="replace")
    destination = CODE_DIR / source.name
    destination.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, destination)
    return code, destination.relative_to(PROJECT_ROOT).as_posix()


def build_questions() -> list[dict[str, object]]:
    if not SOURCE_ROOT.is_dir():
        raise FileNotFoundError(f"找不到题库目录: {SOURCE_ROOT}")

    question_dirs: list[tuple[int, int, Path]] = []
    for path in SOURCE_ROOT.iterdir():
        if not path.is_dir():
            continue
        match = QUESTION_DIR_PATTERN.fullmatch(path.name)
        if match:
            question_dirs.append((int(match.group(1)), int(match.group(2)), path))
    question_dirs.sort()

    questions: list[dict[str, object]] = []
    for start, end, question_dir in question_dirs:
        if not any(start <= question_id <= end for question_id in SELECTED_QUESTIONS):
            continue
        readme_path = question_dir / "README.md"
        if not readme_path.is_file():
            raise FileNotFoundError(f"题组缺少 README.md: {question_dir}")

        readme_text = readme_path.read_text(encoding="utf-8-sig", errors="replace")
        sections = question_sections(readme_text)
        expected_count = end - start + 1
        if len(sections) != expected_count:
            raise ValueError(
                f"{question_dir.name} 应有 {expected_count} 题，"
                f"实际解析到 {len(sections)} 题"
            )

        for offset, (heading, body) in enumerate(sections):
            question_id = start + offset
            if question_id not in SELECTED_QUESTIONS:
                continue
            body = strip_answer_block(body)
            cpp, cpp_file = read_cpp(question_dir, question_id)
            images, image_map = copy_question_images(question_id, body, question_dir)
            questions.append(
                {
                    "id": question_id,
                    "title": clean_title(heading),
                    "difficulty": difficulty_for(question_id),
                    "readme": body,
                    "cpp": cpp,
                    "cpp_file": cpp_file,
                    "images": images,
                    "image_map": image_map,
                    "source_readme": readme_path.relative_to(SOURCE_ROOT).as_posix(),
                }
            )

    actual_ids = [question["id"] for question in questions]
    expected_ids = sorted(SELECTED_QUESTIONS)
    if actual_ids != expected_ids:
        raise ValueError(f"题号与精选列表不一致: {actual_ids}")
    return questions


def remove_unselected_generated_assets() -> None:
    """只清理平台自己生成的非精选题目副本。"""
    for path in IMAGE_DIR.glob("question_[0-9][0-9][0-9]"):
        match = re.fullmatch(r"question_(\d{3})", path.name)
        if match and int(match.group(1)) not in SELECTED_QUESTIONS:
            shutil.rmtree(path)
    for path in CODE_DIR.glob("answer_*.cpp"):
        match = re.fullmatch(r"answer_(\d+)\.cpp", path.name)
        if match and int(match.group(1)) not in SELECTED_QUESTIONS:
            path.unlink()


def main() -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    IMAGE_DIR.mkdir(parents=True, exist_ok=True)
    CODE_DIR.mkdir(parents=True, exist_ok=True)
    remove_unselected_generated_assets()

    questions = build_questions()
    json_text = json.dumps(questions, ensure_ascii=False, indent=2)
    (DATA_DIR / "questions.json").write_text(json_text + "\n", encoding="utf-8")

    # file:// 下 fetch(JSON) 常被浏览器限制，同时输出 JS 数据以保证双击即用。
    (DATA_DIR / "questions.js").write_text(
        "window.OPENCV_QUESTIONS = " + json_text + ";\n", encoding="utf-8"
    )

    cpp_count = sum(bool(question["cpp"]) for question in questions)
    image_count = sum(len(question["images"]) for question in questions)
    print(f"生成完成: {len(questions)} 题")
    print(f"C++ 答案: {cpp_count} 题")
    print(f"题目图片引用: {image_count} 个")
    print(f"数据文件: {DATA_DIR / 'questions.json'}")


if __name__ == "__main__":
    main()
