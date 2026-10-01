"""Performance-baseline стенд для рефакторинга Spec (шаг P3 плана).

Что измеряется: серия одинаковых вызовов SomeStuffCommand.Spec на УЖЕ СОШЕДШЕМСЯ
состоянии модели, то есть сценарий no-op (0/0/0). Такой прогон безопасен для
повтора: он ничего не создаёт, не меняет и не удаляет.

Чего стенд НЕ измеряет и почему:
- create с нуля, update, delete_old требуют подготовки состояния модели. Работающего
  автоматического setter свойств нет (API.SetPropertyValuesOfElements отвечает
  success, но не пишет — зафиксировано в #226), а API.Undo/API.Redo в JSON-порту
  AC25 отсутствуют (проверено: "Command 'API.Undo' not found"). Поэтому эти сценарии
  остаются ручными и помечены not verified, а не подменяются no-op.
- Клиентский wall-clock включает сериализацию ответа, тогда как elapsedSeconds в JSON
  заканчивается на выходе из SpecAll. Сохраняются оба.

Граница достоверности (план §10.2): JSON-ответ характеризует подготовленную запись,
а не конечное состояние модели. Сравнение времени здесь измеряет работу алгоритма
на текущих данных, а не подтверждает корректность записанного.

Запуск:
  python Tools/spec_benchmark.py --runs 10 --mode both --tag baseline-a
  python Tools/spec_benchmark.py --runs 10 --mode false --tag baseline-nodump
"""

import argparse
import json
import os
import re
import socket
import statistics
import subprocess
import time
import urllib.error
import urllib.request

BASELINE_DIR = os.path.join("Reviews", "spec-refactor-baseline")
PORT_FIRST = 19723
PORT_LAST = 19743


def find_port():
    """Порт JSON API ArchiCAD. Основной диапазон, затем LISTENING-порты
    процесса Archicad (экземпляр может быть поднят из VS и занять иной порт)."""
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


def call_spec(port, include, timeout=900):
    """Один вызов SomeStuffCommand.Spec; возвращает (ответ, клиентский wall-clock)."""
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
    started = time.perf_counter()
    with urllib.request.urlopen(req, timeout=timeout) as resp:
        raw = json.loads(resp.read().decode("utf-8"))
    return raw, time.perf_counter() - started


def row_count(body, section):
    return len(((body.get(section) or {}).get("element") or []))


def rejected(body, run_index):
    """Почему ответ нельзя принять как измерение no-op серии.

    Проверяются ПЕРЕД записью замера: серия, названная noop-converged, обязана
    состоять из успешных ответов без единой операции. Иначе в timings попадает
    не то, что измеряли, а тишина отказа (status failed) или незамеченная
    мутация модели (ненулевой create).
    """
    status = body.get("status")
    if status != "completed":
        return "status=%r" % status
    code = body.get("resultCode")
    if isinstance(code, int) and code != 0:
        return "resultCode=%r" % code
    counters = (body.get("elementsToCreate"),
                body.get("elementsToModify"),
                body.get("elementsToDelete"))
    if any(isinstance(c, int) and c != 0 for c in counters):
        return "counters C/M/D=%s" % (counters,)
    for key in ("status", "resultCode", "elementsToCreate",
                "elementsToModify", "elementsToDelete"):
        if key not in body:
            return "missing %s" % key
    return None


def measure(port, runs, include):
    """Серия прогонов; порядок фиксирован, вариативность не оценивается как
    A/B — здесь только baseline одной конфигурации."""
    records = []
    for index in range(1, runs + 1):
        raw, wall = call_spec(port, include)
        if raw.get("succeeded") is not True:
            print("run %d FAILED: %s" % (index, json.dumps(raw, ensure_ascii=False)[:300]))
            return None
        body = raw.get("result", {}).get("addOnCommandResponse", {})
        why = rejected(body, index)
        if why is not None:
            # Серия обрывается: дальнейшие вызовы только повторят то же самое
            # и, при create, продолжили бы изменять модель.
            print("run %d REJECTED (%s): %s" % (index, why, json.dumps(body, ensure_ascii=False)[:300]))
            return None
        elapsed = body.get("elapsedSeconds")
        if not isinstance(elapsed, (int, float)) or elapsed < 0:
            # Нечисловое время не усредняем, но и не выбрасываем молча: прогон
            # учитывается в счётчиках, и series_complete становится ложным.
            print("run %d BAD TIMING: %r" % (index, elapsed))
            elapsed = None
        record = {
            "run": index,
            "includeParameters": include,
            "status": body.get("status"),
            "resultCode": body.get("resultCode"),
            "elementsToCreate": body.get("elementsToCreate"),
            "elementsToModify": body.get("elementsToModify"),
            "elementsToDelete": body.get("elementsToDelete"),
            "elapsedSeconds": elapsed,
            "clientWallSeconds": round(wall, 6),
            "createdRows": row_count(body, "created"),
            "modifiedRows": row_count(body, "modified"),
            "deletedRows": row_count(body, "deleted"),
        }
        records.append(record)
        print("  run %2d/%d  elapsed=%s  wall=%.4f  C=%s M=%s D=%s  %s"
              % (index, runs,
                 ("%.4f" % elapsed) if elapsed is not None else "n/a",
                 wall, record["elementsToCreate"],
                 record["elementsToModify"],
                 record["elementsToDelete"], record["status"]))
    return records


def summarize(records, include):
    elapsed = [r["elapsedSeconds"] for r in records
               if isinstance(r["elapsedSeconds"], (int, float))]
    discarded = len(records) - len(elapsed)
    wall = [r["clientWallSeconds"] for r in records]
    counters = {(r["elementsToCreate"], r["elementsToModify"], r["elementsToDelete"])
                for r in records}
    statuses = {r["status"] for r in records}
    return {
        "scenario": "noop-converged",
        "includeParameters": include,
        "runs": len(records),
        # Число действительных измерений и число отброшенных показываются
        # явно: иначе серия из двух прогонов с одним временем выглядит как
        # «два замера, нулевой разброс».
        "validTimingSamples": len(elapsed),
        "discardedTimingSamples": discarded,
        "series_complete": discarded == 0,
        "statuses": sorted(statuses),
        "countersObserved": sorted(list(counters)),
        "elapsedSeconds": {
            "median": round(statistics.median(elapsed), 6) if elapsed else None,
            "min": round(min(elapsed), 6) if elapsed else None,
            "max": round(max(elapsed), 6) if elapsed else None,
            "stdev": round(statistics.stdev(elapsed), 6) if len(elapsed) > 1 else None,
            "spreadPct": (round(100.0 * (max(elapsed) - min(elapsed)) / statistics.median(elapsed), 2)
                          if elapsed and statistics.median(elapsed) else None),
        },
        "clientWallSeconds": {
            "median": round(statistics.median(wall), 6) if wall else None,
            "min": round(min(wall), 6) if wall else None,
            "max": round(max(wall), 6) if wall else None,
        },
        "note": ("JSON dump characterizes prepared write, not final model state; "
                 "this is not evidence that stored values are correct."),
    }


def write_csv(path, records):
    if not records:
        return
    columns = list(records[0].keys())
    with open(path, "w", encoding="utf-8", newline="") as fh:
        fh.write(",".join(columns) + "\n")
        for rec in records:
            fh.write(",".join(str(rec[c]) for c in columns) + "\n")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--runs", type=int, default=10)
    ap.add_argument("--mode", choices=("false", "true", "both"), default="both")
    ap.add_argument("--tag", default="baseline")
    args = ap.parse_args()

    port = find_port()
    if port is None:
        print("ArchiCAD JSON port not found")
        return 3
    print("port: %d | runs: %d | mode: %s" % (port, args.runs, args.mode))

    modes = [False, True] if args.mode == "both" else [args.mode == "true"]
    all_records, summaries = [], []
    for include in modes:
        print("series includeParameters=%s" % str(include).lower())
        records = measure(port, args.runs, include)
        if records is None:
            # Серия отвергнута: файлы НЕ пишутся, чтобы отвергнутый замер не
            # выглядел как готовая baseline-серия для следующего сравнения.
            print("series rejected - no baseline written")
            return 1
        all_records.extend(records)
        summaries.append(summarize(records, include))
    if any(not s["series_complete"] for s in summaries):
        print("series incomplete - timings are partial, treat as evidence of a defect")
        return 1

    os.makedirs(BASELINE_DIR, exist_ok=True)
    csv_path = os.path.join(BASELINE_DIR, "runs-%s.csv" % args.tag)
    json_path = os.path.join(BASELINE_DIR, "timing-%s.json" % args.tag)
    write_csv(csv_path, all_records)
    with open(json_path, "w", encoding="utf-8") as fh:
        json.dump({"tag": args.tag, "port": port, "summaries": summaries},
                  fh, ensure_ascii=False, indent=2, sort_keys=True)
        fh.write("\n")

    print("\nsaved:\n  %s\n  %s" % (csv_path, json_path))
    for s in summaries:
        e = s["elapsedSeconds"]
        print("includeParameters=%-5s median=%.4fs  min=%.4f  max=%.4f  spread=%s%%  counters=%s"
              % (str(s["includeParameters"]).lower(), e["median"], e["min"], e["max"],
                 e["spreadPct"], s["countersObserved"]))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
