#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""somestuff_updater.py — обновление аддона SomeStuff в установленном Archicad.

Возможности:
  1. Выбор канала обновления (релиз / тестовые сборки).
  2. Выбор языка Archicad (влияет на путь к папке аддонов).
  3. Выбор версии Archicad.
  4. Поиск установленного SomeStuff и чтение версии из .apx
     (PE VERSIONINFO + ASCII-строка {version} {date} #{commit}).
  5. Определение последней версии на выбранном канале (GitHub).
  6. Проверка запущенного Archicad, ожидание закрытия.
  7. Резервное копирование: переименование старого .apx в *_old_version_*.
  8. Копирование нового .apx в папку аддонов (поддержка elevation для Program Files).

Запуск:
    python python_scripts/somestuff_updater.py                # GUI (по умолчанию)
    python python_scripts/somestuff_updater.py --cli          # консольный режим
    python python_scripts/somestuff_updater.py --elevated-copy SRC DST
        # внутренний вызов из GUI для копирования с правами администратора

Только стандартная библиотека Python 3.
"""

import argparse
import ctypes
import ctypes.wintypes
import hashlib
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.request
import urllib.error
import winreg
import zipfile
from pathlib import Path

# ---------------------------------------------------------------------------
# Константы
# ---------------------------------------------------------------------------

REPO = "kuvbur/AddOn_SomeStuff"
GITHUB_API = "https://api.github.com/repos/%s/releases" % REPO
GITHUB_LATEST_REDIRECT = "https://github.com/%s/releases/latest" % REPO
GITHUB_TAG = "https://github.com/%s/releases/tag/%%s" % REPO
TEST_TAG = "test-latest"

ADDON_NAME = "SomeStuff.apx"
BACKUP_SUFFIX = ".apx._old_version_{stamp}"  # формат из ТЗ: ._old_version_@VERSION@
UA = "SomeStuff-Updater/1.0"
ETAG_CACHE = Path(os.environ.get("LOCALAPPDATA", os.path.expanduser("~"))) / "SomeStuff" / "updater_etag.json"

# Локализованные имена папки аддонов внутри корня Archicad (факт: на RUS —
# "Расширения ARCHICAD/Дополнения", на INT — "ARCHICAD Extensions/Add-Ons").
ADDON_DIR_CANDIDATES = {
    "RUS": [r"Расширения ARCHICAD\Дополнения", r"ARCHICAD Extensions\Add-Ons"],
    "INT": [r"ARCHICAD Extensions\Add-Ons"],
}
# Fallback-кандидаты для всех языков (проверяем как есть, независимо от языка).
FALLBACK_DIRS = [r"ARCHICAD Extensions\Add-Ons", r"Расширения ARCHICAD\Дополнения"]


# ---------------------------------------------------------------------------
# Реестр: поиск установленных Archicad
# ---------------------------------------------------------------------------

def _open_key(root, path, wow64_64=True):
    """Открывает ключ реестра, при wow64_64 использует 64-битный вид."""
    access = winreg.KEY_READ
    if wow64_64:
        access |= winreg.KEY_WOW64_64KEY
    try:
        return winreg.OpenKey(root, path, 0, access)
    except FileNotFoundError:
        if wow64_64:  # запасной проход через 32-битный вид
            try:
                return winreg.OpenKey(root, path, 0, winreg.KEY_READ)
            except FileNotFoundError:
                return None
        return None


def find_archicad_installs():
    """Возвращает список установленных Archicad из реестра (HKLM Uninstall).

    Каждая запись: {version, lang, display_name, install_root}.
    """
    installs = {}
    uninstall_paths = [
        r"SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall",
        r"SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall",
    ]
    for base in uninstall_paths:
        key = _open_key(winreg.HKEY_LOCAL_MACHINE, base, wow64_64=True)
        if key is None:
            continue
        try:
            i = 0
            while True:
                try:
                    subkey_name = winreg.EnumKey(key, i)
                except OSError:
                    break
                i += 1
                if not subkey_name.startswith("ARCHICAD"):
                    continue
                sub = _open_key(winreg.HKEY_LOCAL_MACHINE, base + "\\" + subkey_name, wow64_64=True)
                if sub is None:
                    continue
                try:
                    display, _ = winreg.QueryValueEx(sub, "DisplayName")
                    location, _ = winreg.QueryValueEx(sub, "InstallLocation")
                except (FileNotFoundError, OSError):
                    winreg.CloseKey(sub)
                    continue
                finally:
                    winreg.CloseKey(sub)
                m = re.match(r"ARCHICAD\s+(\d+)", display)
                if not m:
                    continue
                ver = int(m.group(1))
                lang = "RUS" if "RUS" in display.upper() else "INT"
                if ver not in installs:  # первый встреченный приоритетнее
                    installs[ver] = {
                        "version": ver,
                        "lang": lang,
                        "display_name": display,
                        "install_root": location.rstrip("\\"),
                    }
        finally:
            winreg.CloseKey(key)
    return [installs[v] for v in sorted(installs)]


# ---------------------------------------------------------------------------
# Поиск папки аддонов и установленного SomeStuff
# ---------------------------------------------------------------------------

def find_addons_dir(install_root):
    """Возвращает путь к папке аддонов внутри корня Archicad или None."""
    candidates = list(FALLBACK_DIRS)
    for sub in ADDON_DIR_CANDIDATES.values():
        for c in sub:
            if c not in candidates:
                candidates.append(c)
    for rel in candidates:
        p = Path(install_root) / rel
        if p.is_dir():
            return p
    return None


def find_installed_addon(addons_dir):
    """Ищет SomeStuff*.apx в папке аддонов (и на уровень подпапок).

    Возвращает (path, None) или (None, все_варианты) если найдено несколько.
    """
    if addons_dir is None or not addons_dir.is_dir():
        return None, []
    found = list(addons_dir.glob("SomeStuff*.apx"))
    for child in addons_dir.iterdir():
        if child.is_dir():
            found.extend(child.glob("SomeStuff*.apx"))
    if not found:
        return None, []
    if len(found) == 1:
        return found[0], []
    return None, found  # два экземпляра — конфликт, вернём все


# ---------------------------------------------------------------------------
# Чтение версии из .apx
# ---------------------------------------------------------------------------

# Формат About-строки в бинарнике:
#   новый: "2.01 2026-10-22  #c46bf76"  (версия + дата + коммит, после 1.77)
#   старый: "v1.77 2025-10-01-21"       (версия с 'v' + дата-час, до 1.77)
#   коммит может отсутствовать, дата обязательна.
RE_VERSION_STRING = re.compile(
    rb"(v?)(\d{1,2}\.\d{1,2}(?:\.\d{1,2})?)\s+"       # версия (v1.77 / 2.01 / 2.01.0)
    rb"(\d{4}-\d{2}-\d{2})(?:-(\d{2,4}))?"            # дата и опциональный час/мин
    rb"\s*(?:#\s*([0-9a-fA-F]{7,40}))?"               # опциональный коммит
)


def read_apx_version_info(apx_path):
    """Читает {pe_version, version, date, commit} из .apx.

    pe_version — FileVersion из PE VERSIONINFO (например '2.01.0'), есть только
    у сборок post-1.77. version/date/commit — из ASCII-строки About в бинарнике.
    """
    result = {"pe_version": None, "version": None, "date": None, "commit": None}
    if not apx_path or not Path(apx_path).is_file():
        return result

    # PE VERSIONINFO через ctypes
    try:
        size = ctypes.windll.version.GetFileVersionInfoSizeW(str(apx_path), None)
        if size:
            buf = ctypes.create_string_buffer(size)
            if ctypes.windll.version.GetFileVersionInfoW(str(apx_path), 0, size, buf):
                ptr = ctypes.c_void_p()
                length = ctypes.c_uint(0)
                # ищем по всем возможным кодовым страницам
                for cp in ("040904B0", "040904E4", "04090000"):
                    q = u"\\StringFileInfo\\%s\\FileVersion" % cp
                    if ctypes.windll.version.VerQueryValueW(buf, q, ctypes.byref(ptr), ctypes.byref(length)):
                        if length.value > 0 and ptr.value:
                            result["pe_version"] = ctypes.wstring_at(ptr, length.value).rstrip("\x00")
                            break
    except Exception:
        pass  # нет VERSIONINFO (старые сборки) — не ошибка

    # ASCII-строка About
    try:
        data = Path(apx_path).read_bytes()
    except OSError:
        return result
    # берём последнее совпадение — около него обычно строка с версией из grc
    matches = list(RE_VERSION_STRING.finditer(data))
    if matches:
        m = matches[-1]
        v_prefix, version, date, hour, commit = m.groups()
        result["version"] = ("v" if v_prefix else "") + version.decode("ascii")
        result["date"] = date.decode("ascii") + (("-" + hour.decode("ascii")) if hour else "")
        if commit:
            result["commit"] = commit.decode("ascii")
    return result


def version_tuple(version_str):
    """'v1.77' / '2.01' / '2.01.0' → числовой кортеж (1,77)/(2,1)/(2,1,0)."""
    if not version_str:
        return None
    s = version_str.lstrip("v")
    try:
        return tuple(int(x) for x in s.split("."))
    except ValueError:
        return None


def is_newer(installed_version, remote_version):
    """True если remote_version строго новее installed_version (числовое сравнение)."""
    a, b = version_tuple(installed_version), version_tuple(remote_version)
    if a is None or b is None:
        return False
    # дополняем кортежи нулями до одинаковой длины
    n = max(len(a), len(b))
    a = a + (0,) * (n - len(a))
    b = b + (0,) * (n - len(b))
    return b > a


# ---------------------------------------------------------------------------
# Каналы обновления (GitHub)
# ---------------------------------------------------------------------------

def _http_get(url, timeout=30, etag=None):
    """HTTP GET с User-Agent. Возвращает (data, etag, status) или (None, None, error_text)."""
    headers = {"User-Agent": UA, "Accept": "application/json"}
    if etag:
        headers["If-None-Match"] = etag
    req = urllib.request.Request(url, headers=headers)
    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            return resp.read().decode("utf-8"), resp.headers.get("ETag"), resp.status
    except urllib.error.HTTPError as e:
        if e.code == 304:
            return None, etag, 304
        return None, None, "HTTP %d" % e.code
    except Exception as exc:
        return None, None, str(exc)


def get_release_channel():
    """Release-канал: редирект releases/latest без API (без лимита 60/час).

    Возвращает (tag, None) или (None, error_text).
    """
    try:
        req = urllib.request.Request(GITHUB_LATEST_REDIRECT, headers={"User-Agent": UA})
        # нужно получить ответ редиректа, чтобы вытащить тег из Location
        class NoRedirect(urllib.request.HTTPRedirectHandler):
            def redirect_request(self, req, fp, code, msg, headers, newurl):
                return None
        opener = urllib.request.build_opener(NoRedirect)
        try:
            opener.open(req)
            return None, "Ожидался редирект, но его нет"
        except urllib.error.HTTPError as e:
            if e.code in (301, 302, 303, 307, 308):
                location = e.headers.get("Location", "")
                m = re.search(r"/tag/(.+)$", location)
                if m:
                    return m.group(1), None
            return None, "Неожиданный ответ: HTTP %d" % e.code
        except Exception as exc:
            return None, str(exc)
    except Exception as exc:
        return None, str(exc)


def parse_commit_from_release_body(body):
    """Достаёт короткий коммит из тела тестового релиза.

    Формат: «Коммит сборки: [c46bf76](https://github.com/.../commit/c46bf76...)».
    """
    if not body:
        return None
    m = re.search(r"Коммит сборки: \[([0-9a-fA-F]{7,40})\]", body)
    return m.group(1) if m else None


def parse_stamp_from_asset_name(asset_name):
    """Из имени ассета SomeStuff.AC25.Win.2026-10-22-1837.zip достаёт штамп даты.

    Формат штампа: YYYY-MM-DD (часы/минуты необязательны).
    Возвращает строку штампа или None.
    """
    if not asset_name:
        return None
    m = re.search(r"(\d{4}-\d{2}-\d{2}(?:-\d{2,4})?)\.zip$", asset_name)
    return m.group(1) if m else None


def is_test_newer(installed_date, installed_commit, remote_stamp, remote_commit):
    """Сравнение тестовых сборок по дате и коммиту (версия у них не меняется).

    installed_date — дата установленного apx (из About-строки, без времени);
    remote_stamp — штамп из имени ассета (может содержать часы/минуты).
    Логика:
      дата дальше → новее; та же дата, другой коммит → новее (пересборка);
      та же дата и тот же коммит → та же сборка.
    """
    if not remote_stamp:
        return False
    remote_date = remote_stamp[:10]  # YYYY-MM-DD
    if installed_date is None:
        return True  # неизвестная версия — предлагаем обновление
    if remote_date > installed_date[:10]:
        return True
    if remote_date == installed_date[:10]:
        # та же дата: различаем по коммиту, если он известен с обеих сторон
        if installed_commit and remote_commit:
            return remote_commit != installed_commit
        return False  # коммита нет — считаем той же сборкой за эту дату
    return False


def get_test_channel():
    """Test-канал: GitHub API, релиз с тегом test-latest, кэш ETag.

    Возвращает (asset_name, tag, commit, None) или (None, None, None, error_text).
    Коммит берётся из тела релиза («Коммит сборки: [c46bf76]»).
    """
    etag = None
    ETAG_CACHE.parent.mkdir(parents=True, exist_ok=True)
    if ETAG_CACHE.is_file():
        try:
            etag = json.loads(ETAG_CACHE.read_text(encoding="utf-8")).get("etag")
        except Exception:
            pass
    data, new_etag, status = _http_get(GITHUB_API, etag=etag)
    if status == 304:
        # данные в кэше не менялись — берём из кэша
        cache_file = ETAG_CACHE.with_suffix(".data.json")
        if cache_file.is_file():
            try:
                releases = json.loads(cache_file.read_text(encoding="utf-8"))
                for rel in releases:
                    if rel.get("tag_name") == TEST_TAG:
                        assets = [a for a in rel.get("assets", []) if a.get("name", "").endswith(".zip")]
                        name = assets[-1]["name"] if assets else None
                        commit = parse_commit_from_release_body(rel.get("body"))
                        return name, TEST_TAG, commit, None
            except Exception:
                pass
        return None, None, None, "Кэш пуст, а API вернул 304"
    if data is None:
        return None, None, None, status
    try:
        releases = json.loads(data)
    except json.JSONDecodeError as e:
        return None, None, None, "Не удалось разобрать ответ API: %s" % e
    if new_etag:
        ETAG_CACHE.write_text(json.dumps({"etag": new_etag}), encoding="utf-8")
        ETAG_CACHE.with_suffix(".data.json").write_text(data, encoding="utf-8")
    for rel in releases:
        if rel.get("tag_name") == TEST_TAG:
            assets = [a for a in rel.get("assets", []) if a.get("name", "").endswith(".zip")]
            name = assets[-1]["name"] if assets else None
            commit = parse_commit_from_release_body(rel.get("body"))
            return name, TEST_TAG, commit, None
    return None, None, None, "Релиз с тегом %s не найден" % TEST_TAG


def asset_for_version(asset_name, ac_version, platform="Win"):
    """Возвращает имя ассета под конкретную версию Archicad, либо None."""
    if not asset_name:
        return None
    m = re.search(r"SomeStuff\.AC(\d+)\.(Win|Mac)\.", asset_name)
    if not m:
        return None
    # У релизов один ассет на все версии AC (например AC25 zip содержит apx под 25),
    # поэтому просто ищем ассет с нужной версией, если он есть.
    pat = "SomeStuff.AC%d.%s." % (ac_version, platform)
    if asset_name.startswith(pat):
        return asset_name
    return None


# ---------------------------------------------------------------------------
# Процессы Archicad
# ---------------------------------------------------------------------------

def get_archicad_processes(install_root=None):
    """Возвращает список путей к запущенным ARCHICAD.exe (ctypes, без wmic).

    install_root — если задан, фильтрует по корню целевой установки (чтобы
    можно было обновлять AC25 при открытом AC27).
    """
    psapi = ctypes.windll.psapi
    kernel32 = ctypes.windll.kernel32
    PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
    # Первая попытка — EnumProcesses
    count = 4096
    while True:
        pids = (ctypes.wintypes.DWORD * count)()
        returned = ctypes.wintypes.DWORD()
        if not psapi.EnumProcesses(ctypes.byref(pids), ctypes.sizeof(pids), ctypes.byref(returned)):
            return []
        n = returned.value // ctypes.sizeof(ctypes.wintypes.DWORD)
        if n < count:
            break
        count *= 2
    result = []
    for i in range(n):
        pid = pids[i]
        if not pid:
            continue
        handle = kernel32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
        if not handle:
            continue
        try:
            size = ctypes.wintypes.DWORD(1024)
            buf = ctypes.create_unicode_buffer(1024)
            if kernel32.QueryFullProcessImageNameW(handle, 0, buf, ctypes.byref(size)):
                path = buf.value
                if path.lower().endswith("archicad.exe"):
                    if install_root and not path.lower().startswith(str(install_root).lower()):
                        continue
                    result.append(path)
        finally:
            kernel32.CloseHandle(handle)
    return result


def wait_for_archicad_close(install_root, timeout_sec=300, poll_sec=2, progress_cb=None):
    """Ждёт закрытия всех ARCHICAD.exe указанной установки. True если закрылись."""
    start = time.time()
    while True:
        procs = get_archicad_processes(install_root)
        if not procs:
            return True
        if time.time() - start > timeout_sec:
            return False
        if progress_cb:
            progress_cb("Archicad ещё запущен: %d процесс(ов), ждём..." % len(procs))
        time.sleep(poll_sec)


# ---------------------------------------------------------------------------
# Установка
# ---------------------------------------------------------------------------

def sha256_of_file(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()


def extract_apx_from_zip(zip_path, dest_dir):
    """Извлекает SomeStuff.apx из zip (защита от zip-slip: только конкретный член)."""
    with zipfile.ZipFile(zip_path, "r") as z:
        member_name = None
        for info in z.infolist():
            if Path(info.filename).name == ADDON_NAME:
                member_name = info.filename
                break
        if member_name is None:
            return None, "В архиве нет файла %s" % ADDON_NAME
        data = z.read(member_name)
    dest = Path(dest_dir) / ADDON_NAME
    dest.write_bytes(data)
    return dest, None


def do_update_flow(addons_dir, downloaded_zip, installed_stamp, log_cb=None):
    """Основной поток обновления: temp → проверка → rename → copy → rollback.

    installed_stamp — штамп для имени бэкапа (._old_version_{stamp}).
    Возвращает (новый_штамп, None) или (None, текст_ошибки).
    """

    def log(msg):
        if log_cb:
            log_cb(msg)

    log("Распаковка %s во временную папку..." % downloaded_zip.name)
    tmp_dir = Path(tempfile.mkdtemp(prefix="somestuff_update_"))
    try:
        extracted, err = extract_apx_from_zip(downloaded_zip, tmp_dir)
        if err:
            return None, err
        log("Извлечено: %s (%d байт)" % (extracted.name, extracted.stat().st_size))

        # читаем версию из извлечённого apx
        info = read_apx_version_info(extracted)
        new_stamp = info["version"] or "?"
        log("Версия нового apx: %s (дата %s, коммит %s)" % (info["version"], info["date"], info["commit"]))

        target = addons_dir / ADDON_NAME
        backup_name = ADDON_NAME + ".apx._old_version_%s" % (installed_stamp or "unknown")
        backup = addons_dir / backup_name

        # решающая проверка: попытка переименования (если файл заблокирован — Archicad открыт)
        if target.exists():
            log("Переименование старого: %s" % backup_name)
            try:
                if backup.exists():
                    backup.unlink()  # храним только один бэкап
                target.rename(backup)
            except OSError as e:
                return None, "Файл заблокирован (Archicad открыт?) или ошибка доступа: %s" % e

        # копирование нового
        log("Копирование в %s" % addons_dir)
        try:
            shutil.copy2(extracted, target)
        except (PermissionError, OSError) as e:
            # откат: вернуть старый обратно
            if backup.exists():
                try:
                    backup.rename(target)
                    log("Откат: старая версия восстановлена")
                except OSError:
                    log("ОТКАТ НЕ УДАЛСЯ — бэкап лежит как %s" % backup)
            return None, "Ошибка копирования: %s" % e

        log("Готово: установлен %s" % new_stamp)
        return new_stamp, None
    finally:
        shutil.rmtree(tmp_dir, ignore_errors=True)


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def cli_main(args):
    """Консольный режим: детекция + проверка канала + обновление."""
    installs = find_archicad_installs()
    if not installs:
        print("Установленный Archicad не найден в реестре.")
        return 1
    print("Найденные Archicad:")
    for i in installs:
        print("  AC%d %s — %s" % (i["version"], i["lang"], i["install_root"]))

    if args.elevated_copy:
        # вызывается только после ShellExecuteW runas из GUI
        src, dst = args.elevated_copy
        src = Path(src)
        if not src.is_file():
            print("Нет файла %s" % src, file=sys.stderr)
            return 2
        shutil.copy2(src, dst)
        return 0

    # простейший сценарий: берём первую установку и проверяем канал
    inst = installs[0]
    addons = find_addons_dir(inst["install_root"])
    apx, dupes = find_installed_addon(addons)
    if dupes:
        print("Найдено несколько SomeStuff.apx (конфликт):")
        for d in dupes:
            print("  " + str(d))
        return 2
    if apx is None:
        print("SomeStuff не установлен в %s" % addons)
        return 0
    info = read_apx_version_info(apx)
    print("Установлен: %s (дата %s, коммит %s, PE %s)" % (
        info["version"], info["date"], info["commit"], info["pe_version"]))

    tag, err = get_release_channel()
    if err:
        print("Ошибка получения релиза: %s" % err)
        return 3
    print("Последний релиз: %s" % tag)
    if is_newer(info["version"], tag):
        print("Доступно обновление!")
    return 0


def main():
    parser = argparse.ArgumentParser(description="Обновление аддона SomeStuff.")
    parser.add_argument("--cli", action="store_true", help="консольный режим (без GUI)")
    parser.add_argument("--elevated-copy", nargs=2, metavar=("SRC", "DST"),
                        help="внутренний вызов: копировать SRC в DST (с правами администратора)")
    args = parser.parse_args()
    if args.elevated_copy:
        sys.exit(cli_main(args))
    if args.cli:
        sys.exit(cli_main(args))
    gui_main()


# ---------------------------------------------------------------------------
# GUI (tkinter)
# ---------------------------------------------------------------------------

def gui_main():
    import tkinter as tk
    from tkinter import scrolledtext, ttk

    class UpdaterGUI:
        def __init__(self, root):
            self.root = root
            root.title("SomeStuff Updater")
            root.geometry("640x520")
            self.installs = find_archicad_installs()
            self.busy = False

            frm = ttk.Frame(root, padding=10)
            frm.pack(fill=tk.BOTH, expand=True)

            # --- строка 1: канал ---
            ttk.Label(frm, text="Канал обновления:").grid(row=0, column=0, sticky=tk.W, pady=2)
            self.channel_var = tk.StringVar(value="Release")
            self.channel_box = ttk.Combobox(frm, textvariable=self.channel_var,
                                            values=["Release", "Test"], state="readonly", width=18)
            self.channel_box.grid(row=0, column=1, sticky=tk.W, pady=2)

            # --- строка 2: версия Archicad ---
            ttk.Label(frm, text="Версия Archicad:").grid(row=1, column=0, sticky=tk.W, pady=2)
            versions = ["AC%d" % i["version"] for i in self.installs]
            self.version_var = tk.StringVar(value=versions[0] if versions else "")
            self.version_box = ttk.Combobox(frm, textvariable=self.version_var,
                                            values=versions, state="readonly" if versions else "disabled",
                                            width=18)
            self.version_box.grid(row=1, column=1, sticky=tk.W, pady=2)

            # --- строка 3: язык ---
            ttk.Label(frm, text="Язык Archicad:").grid(row=2, column=0, sticky=tk.W, pady=2)
            langs = sorted({i["lang"] for i in self.installs}) or ["INT"]
            self.lang_var = tk.StringVar(value=langs[0])
            self.lang_box = ttk.Combobox(frm, textvariable=self.lang_var,
                                         values=langs, state="readonly", width=18)
            self.lang_box.grid(row=2, column=1, sticky=tk.W, pady=2)

            # --- кнопки ---
            btns = ttk.Frame(frm)
            btns.grid(row=3, column=0, columnspan=2, pady=(10, 4))
            self.check_btn = ttk.Button(btns, text="Проверить", command=self.on_check)
            self.check_btn.pack(side=tk.LEFT, padx=4)
            self.update_btn = ttk.Button(btns, text="Обновить", command=self.on_update)
            self.update_btn.pack(side=tk.LEFT, padx=4)

            # --- лог ---
            self.log = scrolledtext.ScrolledText(frm, height=18, width=76, state=tk.DISABLED)
            self.log.grid(row=4, column=0, columnspan=2, sticky=tk.NSEW, pady=(6, 0))
            frm.rowconfigure(4, weight=1)
            frm.columnconfigure(0, weight=1)
            frm.columnconfigure(1, weight=1)

            self._write_log("Установленные Archicad:")
            for i in self.installs:
                self._write_log("  AC%d %s — %s" % (i["version"], i["lang"], i["install_root"]))
            if not self.installs:
                self._write_log("Archicad в реестре не найден.")

        def _write_log(self, msg):
            self.log.configure(state=tk.NORMAL)
            self.log.insert(tk.END, msg + "\n")
            self.log.see(tk.END)
            self.log.configure(state=tk.DISABLED)

        def _selected_install(self):
            ver_text = self.version_var.get()
            m = re.match(r"AC(\d+)", ver_text)
            if not m:
                return None
            ver = int(m.group(1))
            for i in self.installs:
                if i["version"] == ver:
                    return i
            return None

        def _set_busy(self, busy):
            self.busy = busy
            state = tk.DISABLED if busy else tk.NORMAL
            self.check_btn.configure(state=state)
            self.update_btn.configure(state=state)
            self.channel_box.configure(state=state)
            self.version_box.configure(state=state)
            self.lang_box.configure(state=state)

        def on_check(self):
            if self.busy:
                return
            self._set_busy(True)
            self._write_log("--- Проверка ---")
            self.root.after(50, self._run_check_worker)

        def _run_check_worker(self):
            try:
                inst = self._selected_install()
                if inst is None:
                    self._write_log("Установка не найдена.")
                    return
                addons = find_addons_dir(inst["install_root"])
                apx, dupes = find_installed_addon(addons)
                if dupes:
                    self._write_log("Несколько SomeStuff.apx (конфликт):")
                    for d in dupes:
                        self._write_log("  " + str(d))
                    return
                if apx is None:
                    self._write_log("SomeStuff не установлен в %s" % addons)
                    return
                info = read_apx_version_info(apx)
                self._write_log("Установлен: %s (дата %s, коммит %s, PE %s)" % (
                    info["version"], info["date"], info["commit"], info["pe_version"]))
                channel = self.channel_var.get()
                if channel == "Test":
                    # тестовая сборка: сравнение по дате и коммиту (версия не меняется)
                    asset_name, tag, commit, err = get_test_channel()
                    if err:
                        self._write_log("Ошибка получения тестовой сборки: %s" % err)
                        return
                    stamp = parse_stamp_from_asset_name(asset_name)
                    self._write_log("Последняя тестовая сборка: %s (коммит %s)" % (stamp, commit))
                    if is_test_newer(info["date"], info["commit"], stamp, commit):
                        self._write_log("→ доступно обновление")
                    else:
                        self._write_log("→ обновление не требуется (та же дата/коммит)")
                else:
                    # релизный канал: сравнение по версии
                    tag, err = get_release_channel()
                    if err:
                        self._write_log("Ошибка получения release-версии: %s" % err)
                        return
                    self._write_log("Последний релиз: %s" % tag)
                    if is_newer(info["version"], tag):
                        self._write_log("→ доступно обновление")
                    else:
                        self._write_log("→ обновление не требуется")
            except Exception as e:
                self._write_log("Ошибка проверки: %s" % e)
            finally:
                self.root.after(0, lambda: self._set_busy(False))

        def on_update(self):
            if self.busy:
                return
            self._set_busy(True)
            self._write_log("--- Обновление ---")
            self.root.after(50, self._run_update_worker)

        def _run_update_worker(self):
            try:
                inst = self._selected_install()
                if inst is None:
                    self._write_log("Установка не найдена.")
                    return
                # проверка процессов Archicad
                if get_archicad_processes(inst["install_root"]):
                    self._write_log("Archicad запущен. Закройте его и нажмите «Продолжить» в диалоге.")
                    self.root.after(0, self._ask_close_archicad, inst)
                    return
                self._perform_update(inst)
            except Exception as e:
                self._write_log("Ошибка обновления: %s" % e)
            finally:
                self.root.after(0, lambda: self._set_busy(False))

        def _ask_close_archicad(self, inst):
            """Диалог «Закройте Archicad» + ожидание закрытия в отдельном потоке."""
            dlg = tk.Toplevel(self.root)
            dlg.title("Archicad запущен")
            dlg.transient(self.root)
            dlg.grab_set()
            ttk.Label(dlg, text="Закройте все окна Archicad и нажмите «Продолжить».",
                      padding=10).pack()
            result = {"cancelled": False}

            def on_continue():
                dlg.destroy()
                self._wait_worker(inst, result)

            def on_cancel():
                result["cancelled"] = True
                dlg.destroy()

            btns = ttk.Frame(dlg)
            btns.pack(pady=(0, 10))
            ttk.Button(btns, text="Продолжить", command=on_continue).pack(side=tk.LEFT, padx=6)
            ttk.Button(btns, text="Отмена", command=on_cancel).pack(side=tk.LEFT, padx=6)

            self.root.wait_window(dlg)
            if not result["cancelled"]:
                self.root.after(100, self._wait_close_then_update, inst)

        def _wait_close_then_update(self, inst):
            self._write_log("Ожидание закрытия Archicad...")

            def cb(msg):
                self.root.after(0, lambda m=msg: self._write_log(m))

            closed = wait_for_archicad_close(inst["install_root"], timeout_sec=300, poll_sec=2, progress_cb=cb)
            if closed:
                self._write_log("Archicad закрыт.")
                self._perform_update(inst)
            else:
                self._write_log("Таймаут: Archicad не закрылся за 5 минут. Обновление отменено.")

        def _perform_update(self, inst):
            """Скачивает и устанавливает обновление. Выполняется в GUI-потоке."""
            addons = find_addons_dir(inst["install_root"])
            apx, dupes = find_installed_addon(addons)
            if apx is None:
                self._write_log("SomeStuff не найден — обновление невозможно.")
                return
            old_info = read_apx_version_info(apx)
            self._write_log("Текущая версия: %s (дата %s)" % (old_info["version"], old_info["date"]))

            channel = self.channel_var.get()
            if channel == "Test":
                asset_name, tag, commit, err = get_test_channel()
                if err:
                    self._write_log("Ошибка получения тестовой сборки: %s" % err)
                    return
                stamp = parse_stamp_from_asset_name(asset_name)
                self._write_log("Новая тестовая сборка: %s (коммит %s)" % (stamp, commit))
                if not is_test_newer(old_info["date"], old_info["commit"], stamp, commit):
                    self._write_log("Обновление не требуется.")
                    return
            else:
                tag, err = get_release_channel()
                if err:
                    self._write_log("Ошибка получения релиза: %s" % err)
                    return
                self._write_log("Новый релиз: %s" % tag)
                if not is_newer(old_info["version"], tag):
                    self._write_log("Обновление не требуется.")
                    return
                asset_name = None  # подберём при скачивании

            self._write_log("Скачивание %s..." % (asset_name or "zip"))
            zip_path, err = self._download_asset(tag, asset_name)
            if err:
                self._write_log("Ошибка скачивания: %s" % err)
                return
            new_stamp, err = do_update_flow(addons, Path(zip_path), old_info["version"],
                                            log_cb=lambda m: self.root.after(0, lambda x=m: self._write_log(x)))
            if err:
                self._write_log("Ошибка обновления: %s" % err)
            else:
                self._write_log("Обновление завершено: %s" % new_stamp)

        def _download_asset(self, tag, asset_name):
            """Скачивает zip-ассет релиза в temp. Возвращает (путь, None) или (None, ошибка).

            asset_name — точное имя ассета (тестовый канал); для релизного канала
            подбирается ассет SomeStuff.AC{ver}.Win.*.zip из релиза с тегом tag.
            """
            ver = self._selected_install()["version"] if self._selected_install() else 25
            data, _, status = _http_get(GITHUB_API)
            if data is None:
                return None, "API недоступен: %s" % status
            try:
                releases = json.loads(data)
            except json.JSONDecodeError as e:
                return None, "Не удалось разобрать API: %s" % e
            for rel in releases:
                if rel.get("tag_name") != tag:
                    continue
                for a in rel.get("assets", []):
                    name = a.get("name", "")
                    # тестовый канал: имя ассета пришло из релиза с произвольной
                    # версией AC (штамп у всех ассетов общий) — подставляем свою
                    if asset_name:
                        name_needed = re.sub(r"AC\d+", "AC%d" % ver, asset_name)
                    else:
                        name_needed = None
                    matched = (name == name_needed) if name_needed else \
                        (name.startswith("SomeStuff.AC%d.Win." % ver) and name.endswith(".zip"))
                    if matched:
                        url = a.get("browser_download_url")
                        if url:
                            dest = Path(tempfile.gettempdir()) / name
                            self._write_log("  скачиваем %s" % url)
                            try:
                                urllib.request.urlretrieve(url, dest)
                            except Exception as e:
                                return None, "Ошибка скачивания %s: %s" % (name, e)
                            return dest, None
            return None, "Ассет под AC%d не найден в релизе %s" % (ver, tag)

    root = tk.Tk()
    app = UpdaterGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
