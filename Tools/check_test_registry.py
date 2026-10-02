#!/usr/bin/env python3
#------------ kuvbur 2026 ------------
# Сверка реестра наборов с их определениями.
#
# Зачем: реестр наполняется вызовами TestKit::Register в tests/TestFunc.cpp, а
# наборы определены по файлам tests/*.cpp. Если набор забыли зарегистрировать,
# он молча не попадает в прогон - ни ошибки компиляции, ни строки в отчёте.
# Обратная ситуация не менее опасна: регистрация есть, а определения нет -
# такое возможно при удалении набора. Обе ошибки молчаливые, поэтому проверка
# выполняется скриптом, а не глазами.
#
# Проверяется:
#   1. каждый зарегистрированный набор определён ровно в одном tests/*.cpp;
#   2. каждый набор, определённый как void TestXxx () в tests/*.cpp,
#      зарегистрирован (иначе он не выполняется);
#   3. нет повторной регистрации одного имени;
#   4. объявление набора есть в tests/TestFunc.hpp.
#
# Возвращает 0, если расхождений нет, иначе 1.
import re
import sys
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TESTS = ROOT / "Sources" / "AddOn" / "tests"
REGISTRY = TESTS / "TestFunc.cpp"
HEADER = TESTS / "TestFunc.hpp"

# TestKit::Register ("Имя", Группа, Функция) - вызов может разбит на две строки.
RE_REGISTER = re.compile(
    r'TestKit::Register\s*\(\s*"([^"]+)"\s*,\s*Groups::\w+\s*,\s*(\w+)\s*\)',
    re.S,
)
# Определение набора: void TestXxx () { ... }. Тело открывается на той же
# строке - таково оформление всех наборов в tests/*.cpp.
RE_DEFINITION = re.compile(r'^\s{4}void\s+(Test\w+)\s*\(\s*\)\s*\{', re.M)
# Объявление в заголовке.
RE_DECLARATION = re.compile(r'^\s*void\s+(Test\w+)\s*\(\s*\)\s*;\s*$', re.M)

# Объявления служебных функций TestFunc: они не наборы, а помощники, и в
# реестре им не место. Список явный, а не по префиксу "Test", иначе под него
# попал бы любой будущий помощник с таким именем.
NOT_SUITES = {
    "Test",
    "TestGetTextLineLength",
    "DumpAllBuiltInProperties",
    "ResetSyncPropertyArray",
    "ResetSyncPropertyOne",
}


def collect_definitions():
    """Имя набора -> список файлов, где оно определено."""
    found = defaultdict(list)
    for path in sorted(TESTS.glob("*.cpp")):
        # TestFunc.cpp содержит реестр, а не определения наборов.
        if path.name == REGISTRY.name:
            continue
        text = path.read_text(encoding="utf-8")
        for name in RE_DEFINITION.findall(text):
            found[name].append(path.name)
    return found


def main():
    if not REGISTRY.is_file():
        print("registry: %s not found" % REGISTRY)
        return 1

    registry_text = REGISTRY.read_text(encoding="utf-8")
    header_text = HEADER.read_text(encoding="utf-8") if HEADER.is_file() else ""

    registered = RE_REGISTER.findall(registry_text)
    definitions = collect_definitions()
    declared = set(RE_DECLARATION.findall(header_text))

    problems = []

    # 3. Повторная регистрация: набор выполнится дважды, и его счётчики
    #    удвоятся - «END ... passed=43» станет «passed=86» без видимой причины.
    seen = {}
    for name, fn in registered:
        if name in seen:
            problems.append("duplicate registration: %s" % name)
        seen[name] = fn

    # 1. Зарегистрирован, но не определён.
    for name, fn in seen.items():
        sites = definitions.get(fn)
        if not sites:
            problems.append("registered but not defined: %s -> %s" % (name, fn))
        elif len(sites) > 1:
            problems.append("defined more than once: %s in %s" % (fn, ", ".join(sites)))

    # 2. Определён, но не зарегистрирован: набор молча выпадает из прогона.
    for fn, sites in sorted(definitions.items()):
        if fn in NOT_SUITES:
            continue
        if fn not in set(seen.values()):
            problems.append("defined but not registered: %s in %s" % (fn, ", ".join(sites)))

    # 4. Объявление в заголовке. Само по себе лишнее объявление не ломает
    #    сборку, но расхождение с определением - верный признак рассинхрона.
    for name, fn in sorted(seen.items()):
        if fn not in declared:
            problems.append("registered but not declared in TestFunc.hpp: %s" % fn)

    registered_names = set(seen)
    for fn in sorted(set(declared) - set(seen.values()) - NOT_SUITES):
        problems.append("declared but not registered: %s" % fn)

    print("registered=%d defined=%d declared=%d" % (len(registered_names), len(definitions), len(declared)))
    for line in problems:
        print("  PROBLEM: %s" % line)
    if problems:
        print("FAILED: %d problem(s)" % len(problems))
        return 1
    print("OK: registry matches definitions")
    return 0


if __name__ == "__main__":
    sys.exit(main())