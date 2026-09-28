"""Эталонные JSON-данные для рефакторинга Spec (шаг P1 плана).

Два режима:

  capture  — прогнать Spec с includeParameters=true и сохранить ответ как эталон
  compare  — прогнать Spec снова и сравнить с эталоном

Что сравнивается: значения свойств и GDL-параметров по семантическому ключу
строки. GUID НЕ сравниваются: каждое создание даёт новые GUID, а при A/B
рефакторинге достаточно семантического соответствия (план §10.3, P1).

Граница достоверности (план §10.2): это эталон ПОДГОТОВЛЕННОЙ ЗАПИСИ, а не
снимок конечной модели. Совпадение не доказывает, что модель записана верно.

Запуск:
  python Tools/spec_baseline.py capture   <tag>
  python Tools/spec_baseline.py compare   <tag>
"""

import json
import os
import re
import sys

BASELINE_DIR = os.path.join("Reviews", "spec_baseline")
PORT_FIRST = 19723
PORT_LAST = 19743


def find_port():
    """Порт JSON API Archicad. Основной диапазон из документации; запасной
    путь - LISTENING-порты процесса Archicad (нужен, если экземпляр поднят
    из Visual Studio и занял произвольный порт)."""
    import socket
    import subprocess
    import urllib.error
    import urllib.request

    for port in range(PORT_FIRST, PORT_LAST + 1):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(0.4)
            if sock.connect_ex(("127.0.0.1", port)) == 0:
                return port

    try:
        netstat = subprocess.check_output(
            ["netstat", "-ano"], stderr=subprocess.DEVNULL
        ).decode("utf-8", "replace")
        pids_raw = subprocess.check_output(
            ["powershell", "-NoProfile", "-Command",
             "(Get-Process -Name ARCHICAD* -ErrorAction SilentlyContinue)"
             ".Id | ForEach-Object { $_.ToString() }"],
            stderr=subprocess.DEVNULL,
        ).decode("utf-8", "replace")
    except (OSError, subprocess.CalledProcessError):
        return None

    pids = {tok.strip() for tok in pids_raw.split() if tok.strip()}
    for line in netstat.splitlines():
        if "LISTENING" not in line.upper():
            continue
        fields = line.split()
        if len(fields) < 5 or fields[-1] not in pids:
            continue
        try:
            port = int(fields[1].rsplit(":", 1)[1])
        except (IndexError, ValueError):
            continue
        try:
            # urlopen не принимает headers - заголовки задаются через Request
            probe = urllib.request.Request(
                "http://127.0.0.1:%d/json" % port,
                data=b'{"command":"API.GetProjectInfo","parameters":{}}',
                headers={"Content-Type": "application/json"},
                method="POST",
            )
            urllib.request.urlopen(probe, timeout=20).read()
            return port
        except urllib.error.HTTPError:
            return port
        except (urllib.error.URLError, OSError):
            continue
    return None


def call_spec(port, include=True, timeout=900):
    """Один вызов SomeStuffCommand.Spec. Формат - по
    Sources/AddOn/json_commands/How JSON Commands work.md (AC25)."""
    import urllib.request

    payload = {
        "command": "API.ExecuteAddOnCommand",
        "parameters": {
            "addOnCommandId": {
                "commandNamespace": "SomeStuffCommand",
                "commandName": "Spec",
            },
            "addOnCommandParameters": {
                "placementPoint": {"x": 1000.0, "y": 1000.0},
                "includeParameters": include,
            },
        },
    }
    req = urllib.request.Request(
        "http://127.0.0.1:%d/json" % port,
        data=json.dumps(payload).encode("utf-8"),
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        return json.loads(resp.read().decode("utf-8"))


def is_guid(value):
    return bool(re.match(
        r"^\{?[0-9A-Fa-f]{8}-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{4}"
        r"-[0-9A-Fa-f]{4}-[0-9A-Fa-f]{12}\}?$", str(value or "").strip()))


def guid_list(value):
    """Список GUID из строки-свойства вида 'A;B;C' или из массива объектов."""
    out = []
    for item in value or []:
        if isinstance(item, dict):
            guid = item.get("guid")
            if guid:
                out.append(guid.upper())
    if out:
        return sorted(out)
    if isinstance(value, str):
        out = [p.strip().upper() for p in value.split(";") if is_guid(p.strip())]
    return sorted(out)


def row_key(favorite, values):
    """Семантический ключ строки спецификации.

    Из значений исключаются GUID-поля (связанные элементы) и «имя правила»:
    они меняются от прогона к прогону, но не характеризуют строку.
    Остальные значения и есть идентичность строки.
    """
    identity = []
    for item in values:
        name = item.get("name", "")
        low = name.lower()
        if "связанные элементы" in low or "имя правила" in low:
            continue
        identity.append("%s=%s" % (name, item.get("value", "")))
    return "%s|%s" % (favorite or "", ";".join(sorted(identity)))


def collect_rows(body, section):
    """Строки (create/modify) -> {семантический ключ: {свойства, gdl}}."""
    rows = {}
    elements = (body.get(section) or {}).get("element", [])
    for element in elements:
        props = element.get("property", [])
        gdl = element.get("gdlParameter", [])
        key = row_key(element.get("favoriteName"), props)
        if key in rows:
            rows[key]["sources"] += len(element.get("sourceElement", []))
            continue
        rows[key] = {
            "favorite": element.get("favoriteName", ""),
            "properties": {p["name"]: p.get("value", "") for p in props},
            "gdl": {g["name"]: g.get("value", "") for g in gdl},
            "sources": len(element.get("sourceElement", [])),
        }
    return rows


def summarize(body):
    return {
        "status": body.get("status"),
        "resultCode": body.get("resultCode"),
        "elementsToCreate": body.get("elementsToCreate"),
        "elementsToModify": body.get("elementsToModify"),
        "elementsToDelete": body.get("elementsToDelete"),
        "elapsedSeconds": body.get("elapsedSeconds"),
    }


def save(tag, raw, body, rows):
    os.makedirs(BASELINE_DIR, exist_ok=True)
    raw_path = os.path.join(BASELINE_DIR, "raw-%s.json" % tag)
    cmp_path = os.path.join(BASELINE_DIR, "compare-%s.json" % tag)
    with open(raw_path, "w", encoding="utf-8") as fh:
        json.dump(raw, fh, ensure_ascii=False, indent=2, sort_keys=True)
        fh.write("\n")
    payload = {"summary": summarize(body), "rows": rows}
    with open(cmp_path, "w", encoding="utf-8") as fh:
        json.dump(payload, fh, ensure_ascii=False, indent=2, sort_keys=True)
        fh.write("\n")
    return raw_path, cmp_path


def load(tag):
    path = os.path.join(BASELINE_DIR, "compare-%s.json" % tag)
    if not os.path.isfile(path):
        print("baseline not found: %s" % path)
        return None
    with open(path, encoding="utf-8") as fh:
        return json.load(fh)


def diff_rows(expected, actual):
    """Возвращает список расхождений по значениям и счётчикам."""
    problems = []
    exp_keys, act_keys = set(expected), set(actual)
    for key in sorted(exp_keys - act_keys):
        problems.append("MISSING row: %s" % key[:200])
    for key in sorted(act_keys - exp_keys):
        problems.append("EXTRA row:   %s" % key[:200])
    for key in sorted(exp_keys & act_keys):
        e, a = expected[key], actual[key]
        for field, label in (("properties", "property"), ("gdl", "gdlParameter")):
            exp_vals, act_vals = e[field], a[field]
            for name in sorted(set(exp_vals) - set(act_vals)):
                problems.append(
                    "row %s: %s lost [%s]: %s" % (key[:80], label, name, exp_vals[name]))
            for name in sorted(set(act_vals) - set(exp_vals)):
                problems.append(
                    "row %s: %s new [%s]: %s" % (key[:80], label, name, act_vals[name]))
            for name in sorted(set(exp_vals) & set(act_vals)):
                if exp_vals[name] != act_vals[name]:
                    problems.append(
                        "row %s: %s changed [%s]: %r -> %r"
                        % (key[:80], label, name, exp_vals[name], act_vals[name]))
        if e["sources"] != a["sources"]:
            problems.append(
                "row %s: sourceElement count %d -> %d"
                % (key[:80], e["sources"], a["sources"]))
    return problems


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    action, tag = sys.argv[1], sys.argv[2]
    if action not in ("capture", "compare"):
        print("unknown action: %s" % action)
        return 2

    port = find_port()
    if port is None:
        print("ArchiCAD JSON port not found (%d-%d or Archicad process ports)"
              % (PORT_FIRST, PORT_LAST))
        return 3
    print("port: %d" % port)

    raw = call_spec(port, include=True)
    if raw.get("succeeded") is not True:
        print("command failed: %s" % json.dumps(raw, ensure_ascii=False)[:800])
        return 1
    body = raw.get("result", {}).get("addOnCommandResponse", {})
    rows = {}
    for section in ("created", "modified"):
        rows.update(collect_rows(body, section))
    print("summary: %s" % json.dumps(summarize(body), ensure_ascii=False))
    print("rows: %d" % len(rows))

    if action == "capture":
        raw_path, cmp_path = save(tag, raw, body, rows)
        print("saved:\n  %s\n  %s" % (raw_path, cmp_path))
        return 0

    expected = load(tag)
    if expected is None:
        return 4
    exp_sum, act_sum = expected["summary"], summarize(body)
    for key in ("status", "resultCode", "elementsToCreate",
                "elementsToModify", "elementsToDelete"):
        if exp_sum.get(key) != act_sum.get(key):
            print("FAIL summary %s: %r -> %r" % (key, exp_sum.get(key), act_sum.get(key)))
        else:
            print("ok   summary %s = %r" % (key, act_sum.get(key)))
    print("     elapsedSeconds %r -> %r (not compared)"
          % (exp_sum.get("elapsedSeconds"), act_sum.get("elapsedSeconds")))

    problems = diff_rows(expected["rows"], rows)
    if problems:
        print("\nFAIL: %d difference(s)" % len(problems))
        for line in problems[:80]:
            print("  " + line)
        if len(problems) > 80:
            print("  ... and %d more" % (len(problems) - 80))
        return 1
    print("\nPASS: values of %d row(s) identical (elapsedSeconds not compared)" % len(rows))
    return 0


if __name__ == "__main__":
    sys.exit(main())
