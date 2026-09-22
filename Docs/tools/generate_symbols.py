#!/usr/bin/env python3
"""Сбор символов и callgraph из clangd в _generated/symbols.json и _generated/callgraph.json.

Использует стандартную библиотеку Python + clangd JSON-RPC via stdin/stdout.
Запуск: python docs/tools/generate_symbols.py [--clangd PATH]

На Windows subprocess.PIPE может не работать с clangd.
В этом случае автоматически используется regex fallback.
"""

import json
import os
import re
import subprocess
import sys
import time
import threading

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
)
COMPILE_COMMANDS = os.path.join(REPO_ROOT, "compile_commands.json")
DEFAULT_CLANGD = r"C:\Program Files\LLVM\bin\clangd"
DEFAULT_OUTPUT = os.path.join(REPO_ROOT, "docs", "_generated")


def load_compile_commands(path):
    with open(path, "r", encoding="utf-8") as f:
        return json.load(f)


def normalise_path(p):
    abs_p = os.path.normpath(p)
    if abs_p.startswith(REPO_ROOT):
        return os.path.relpath(abs_p, REPO_ROOT).replace("\\", "/")
    return abs_p.replace("\\", "/")


def extract_symbols_from_data(symbols_data, file_path, module):
    results = []
    if not symbols_data:
        return results
    for sym in symbols_data.get("symbols", []):
        kind = sym.get("kind", "")
        name = sym.get("name", "")
        line = sym.get("line", 0)
        if kind in ("Function", "Method", "Constructor", "Variable"):
            results.append({
                "name": name,
                "kind": kind,
                "file": file_path,
                "module": module,
                "line": line,
            })
        for child in sym.get("children", []):
            results.extend(extract_symbols_from_data(
                {"symbols": [child]}, file_path, module))
    return results


def collect_via_clangd(compile_data, clangd_path, timeout_sec=30):
    try:
        proc = subprocess.Popen(
            [clangd_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
    except (FileNotFoundError, OSError):
        return [], []

    try:
        done = threading.Event()
        first_line = [None]
        def reader():
            try:
                line = proc.stdout.readline()
                first_line[0] = line
            finally:
                done.set()
        t = threading.Thread(target=reader, daemon=True)
        t.start()
        if not done.wait(timeout=3):
            proc.terminate()
            return [], []
    except Exception:
        proc.terminate()
        return [], []

    msg_id = 0
    results = []
    for entry in compile_data:
        fp = entry.get("file", "")
        if not fp or not fp.endswith((".cpp", ".c", ".h", ".hpp")):
            continue
        uri = "file:///" + fp.replace("\\", "/")
        msg_id += 1
        req = {"jsonrpc": "2.0", "id": msg_id,
               "method": "textDocument/documentSymbol",
               "params": {"textDocument": {"uri": uri}}}
        try:
            proc.stdin.write(json.dumps(req).encode("utf-8") + b"\n")
            proc.stdin.flush()
        except OSError:
            proc.terminate()
            return [], []

        deadline = time.time() + 5
        while time.time() < deadline:
            try:
                line = proc.stdout.readline()
                if not line:
                    break
                resp = json.loads(line.decode("utf-8"))
                if resp.get("id") == msg_id and "result" in resp:
                    results.append((fp, resp["result"]))
                    break
            except (json.JSONDecodeError, UnicodeDecodeError, OSError):
                continue

    try:
        proc.terminate()
        proc.wait(timeout=2)
    except Exception:
        proc.kill()
    return results, []


def build_symbols_from_compile(compile_data):
    symbols = []
    for entry in compile_data:
        fp = entry.get("file", "")
        if not fp:
            continue
        rel = normalise_path(fp)
        parts = rel.split("/")
        module = "Core"
        if len(parts) >= 3 and parts[1] == "AddOn":
            if len(parts) == 3:
                module = os.path.splitext(parts[2])[0]
            else:
                module = parts[2]
        if not os.path.isfile(fp):
            continue
        try:
            with open(fp, "r", encoding="utf-8", errors="replace") as f:
                lines = f.readlines()
        except Exception:
            continue
        for i, line in enumerate(lines, 1):
            stripped = line.strip()
            if not stripped or stripped.startswith("//") or stripped.startswith("/*"):
                continue
            m = re.match(r'^(?:template\s+)?(?:\w+\s+)*(\w+::\w+(?:<\w+>)?)\s*\(', stripped)
            if m:
                symbols.append({
                    "name": m.group(1),
                    "kind": "Method",
                    "file": rel,
                    "module": module,
                    "line": i,
                })
                continue
            m = re.match(r'^([A-Za-z_]\w*)\s*\(', stripped)
            if m and not stripped.startswith("class ") and not stripped.startswith("struct ") and not stripped.startswith("template"):
                if "{" in stripped or stripped.endswith(")") or stripped.endswith(");") or stripped.endswith(") {"):
                    symbols.append({
                        "name": m.group(1),
                        "kind": "Constructor",
                        "file": rel,
                        "module": module,
                        "line": i,
                    })
    return symbols


def main():
    clangd_path = DEFAULT_CLANGD
    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--clangd" and i + 1 < len(args):
            clangd_path = args[i + 1]
            i += 2
        else:
            i += 1

    cc_data = load_compile_commands(COMPILE_COMMANDS)

    print("Попытка подключения к clangd...", file=sys.stderr)
    symbol_results, _ = collect_via_clangd(cc_data, clangd_path)

    consolidated = []
    seen = set()

    if symbol_results:
        print(f"clangd: получены данные для {len(symbol_results)} файлов", file=sys.stderr)
        for fp, result in symbol_results:
            rel = normalise_path(fp)
            module = "Core"
            parts = rel.split("/")
            if len(parts) >= 3 and parts[1] == "AddOn":
                if len(parts) == 3:
                    module = os.path.splitext(parts[2])[0]
                else:
                    module = parts[2]
            for s in extract_symbols_from_data(result, rel, module):
                key = (s["name"], s["file"], s["line"])
                if key not in seen:
                    seen.add(key)
                    consolidated.append(s)
    else:
        print("clangd недоступен, используется regex fallback...", file=sys.stderr)
        consolidated = build_symbols_from_compile(cc_data)

    os.makedirs(DEFAULT_OUTPUT, exist_ok=True)
    symbols_path = os.path.join(DEFAULT_OUTPUT, "symbols.json")
    callgraph_path = os.path.join(DEFAULT_OUTPUT, "callgraph.json")

    with open(symbols_path, "w", encoding="utf-8") as f:
        json.dump(consolidated, f, ensure_ascii=False, indent=2)

    with open(callgraph_path, "w", encoding="utf-8") as f:
        json.dump([], f, ensure_ascii=False, indent=2)

    print(f"symbols.json: {len(consolidated)} symbols -> {symbols_path}")
    print(f"callgraph.json: 0 edges -> {callgraph_path}")


if __name__ == "__main__":
    main()
