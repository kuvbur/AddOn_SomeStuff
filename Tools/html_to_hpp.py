#!/usr/bin/env python3
"""
Конвертирует HTML файл в C++ заголовочный файл с HTML в виде C-строки.
Использование: python html_to_hpp.py [input_html] [output_hpp]

Почему не «по \n» и не raw string R"...": у MSVC лимит ~65535 байт на один
строковый литерал (C2026). Минифицированный JS часто одна строка > 200к.
Разбиваем бинарно по длине чанка (15000), не разрывая escape-пары
(\\ и \") между литералами — иначе C++ склеит невалидную escape-последовательность.
"""

import sys
import os

# макс. размер одного C++ строкового литерала в исходнике (запас < 65535 MSVC)
CHUNK_MAX = 15000


def _split_into_chunks(escaped_text, max_len):
    """
    Разбивает строку escaped_text на куски <= max_len,
    не разрывая escape-последовательности \\ и \" на границе чанка.

    Простой способ без посимвольного цикла: если справа от точки среза
    стоит '\\', убираем её из текущего чанка в начало следующего — пара
    'x' в одном литерале и '\\' в другом была бы битой (\ не помещается).
    """
    chunks = []
    text = escaped_text
    while text:
        if len(text) <= max_len:
            chunks.append(text)
            break
        part = text[:max_len]
        rest = text[max_len:]
        # Опасно только если чанк заканчивается на '\\'; переносим его в следующий.
        # Чётное число '\\' подряд в конце (например '\\\\') — это '\\' + '\\',
        # трогать не нужно; считаем через rstrip/rfind.
        if part.endswith('\\') and not part.endswith('\\\\'):
            # отделяем последний '\' — он бы разорвал '\\' или '\"' между литералами
            part, rest = part[:-1], '\\' + rest
        chunks.append(part)
        text = rest
    return chunks


def html_to_cpp_string(html_content):
    """
    Экранирует \\ и \" и режет на C++ литералы "..." по CHUNK_MAX.
    Пустой результат при любом входе не будет.
    """
    escaped = html_content.replace('\\', '\\\\').replace('"', '\\"')
    if not escaped:
        return '    ""'
    chunks = _split_into_chunks(escaped, CHUNK_MAX)
    # Пустых чанков быть не должно (split только на непустом тексте),
    # но на всякий случай фильтруем, чтобы не было `""` в потоке.
    chunks = [c for c in chunks if c] or [""]
    return '\n'.join(f'    "{c}"' for c in chunks)


def generate_hpp(html_var_name, html_content, guard_name):
    cpp_string = html_to_cpp_string(html_content)
    return (
        f"#ifndef {guard_name}_HPP\n"
        f"#define {guard_name}_HPP\n"
        f"\n"
        f"// Автогенерируемый файл — не редактировать вручную!\n"
        f"// Исходный HTML: index.html\n"
        f"// Сгенерировано скриптом: Tools/html_to_hpp.py\n"
        f"// Разбивка: чанки по {CHUNK_MAX} символов (лимит MSVC C2026).\n"
        f"\n"
        f"static const char *{html_var_name} =\n"
        f"{cpp_string};\n"
        f"\n"
        f"#endif // {guard_name}_HPP\n"
    )


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    project_dir = os.path.dirname(script_dir)

    input_html = os.path.join(project_dir, "Sources", "AddOn", "dialogs", "index.html")
    output_hpp = os.path.join(project_dir, "Sources", "AddOn", "dialogs", "HTML_Pages.hpp")

    if len(sys.argv) >= 2:
        input_html = sys.argv[1]
    if len(sys.argv) >= 3:
        output_hpp = sys.argv[2]

    if not os.path.exists(input_html):
        print(f"Ошибка: файл {input_html} не найден")
        sys.exit(1)

    try:
        # Читаем бинарно, чтобы '\n' из редактора (LF/CRLF) стабильно улетал в C++ как LF.
        with open(input_html, 'rb') as f:
            html_content = f.read().decode('utf-8')
    except Exception as e:
        print(f"Ошибка чтения {input_html}: {e}")
        sys.exit(1)

    base_name = os.path.splitext(os.path.basename(output_hpp))[0]
    html_var_name = f"{base_name}_html"
    guard_name = base_name.upper()

    hpp_content = generate_hpp(html_var_name, html_content, guard_name)

    try:
        # Контроль: ни один эмитированный литерал не должен превышать лимит MSVC.
        worst = 0
        for line in hpp_content.splitlines():
            if line.startswith('    "') and line.endswith('"'):
                body = line[5:-1]
                worst = max(worst, len(body))
        if worst > 65000:
            print(f"ВНИМАНИЕ: найден литерал длиной {worst} > 65000 — близко к лимиту C2026")

        with open(output_hpp, 'w', encoding='utf-8', newline='\n') as f:
            f.write(hpp_content)
        print(f"Успешно: {output_hpp} создан")
        print(f"  Переменная: {html_var_name}")
        print(f"  Размер HTML: {len(html_content)} байт")
        print(f"  Макс. длина литерала в файле: {worst} (должно быть < 65000)")
    except Exception as e:
        print(f"Ошибка записи {output_hpp}: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
