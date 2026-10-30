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
import urllib.parse
import webbrowser
import winreg
import zipfile
from pathlib import Path

# ---------------------------------------------------------------------------
# Константы
# ---------------------------------------------------------------------------

REPO = "kuvbur/AddOn_SomeStuff"
SITE_URL = "https://kuvbur.org/go/archicad"
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
# Реестр: пользовательские пути аддонов (Add-On Manager\Include)
# ---------------------------------------------------------------------------

def _decode_lan_flat(value):
    """'lan.flat:///C:/Program%20Files/x.apx' → 'C:/Program Files/x.apx' (или None)."""
    if not isinstance(value, str) or not value:
        return None
    prefix = "lan.flat:///"
    if value.startswith(prefix):
        return urllib.parse.unquote(value[len(prefix):])
    return None  # незнакомая схема (lan.dev, lan.hts и т.п.) — не сетевой путь


def find_registry_custom_addons(ac_version):
    """Ищет SomeStuff*.apx в реестре HKCU: Add-On Manager (Include + Actual Path).

    Ключи вида HKCU\\Software\\GRAPHISOFT\\ARCHICAD\\ARCHICAD <N>... <язык>...;
    Include — значения '#N.' = lan.flat:///URL-encoded/путь.apx;
    'Actual Path' под Add-On Manager — папка, назначенная пользователем в
    диспетчере аддонов (аддоны из неё тоже грузятся).
    Возвращает список путей к найденным SomeStuff*.apx.
    """
    name_pat = re.compile(r"^ARCHICAD\s+%d(\D|$)" % ac_version)
    result = []
    key = _open_key(winreg.HKEY_CURRENT_USER, r"Software\GRAPHISOFT\ARCHICAD", wow64_64=False)
    if key is None:
        return result
    try:
        i = 0
        while True:
            try:
                subkey_name = winreg.EnumKey(key, i)
            except OSError:
                break
            i += 1
            if not name_pat.match(subkey_name):
                continue
            am_path = r"Software\GRAPHISOFT\ARCHICAD\%s\Add-On Manager" % subkey_name
            # 1. значения Include — пути к конкретным .apx
            inc = _open_key(winreg.HKEY_CURRENT_USER, am_path + r"\Include", wow64_64=False)
            if inc is not None:
                try:
                    j = 0
                    while True:
                        try:
                            name, value, _ = winreg.EnumValue(inc, j)
                        except OSError:
                            break
                        j += 1
                        if name.startswith("Include "):  # счётчики вида 'Include Number'
                            continue
                        decoded = _decode_lan_flat(value)
                        if not decoded:
                            continue
                        p = Path(decoded)
                        if p.is_file() and p.name.lower().startswith("somestuff") and p.suffix.lower() == ".apx":
                            result.append(p)
                finally:
                    winreg.CloseKey(inc)
            # 2. Actual Path — пользовательская папка аддонов
            am = _open_key(winreg.HKEY_CURRENT_USER, am_path, wow64_64=False)
            if am is not None:
                try:
                    actual, _ = winreg.QueryValueEx(am, "Actual Path")
                except (FileNotFoundError, OSError):
                    actual = None
                finally:
                    winreg.CloseKey(am)
                if actual:
                    d = Path(actual)
                    if d.is_dir():
                        result.extend(d.glob("SomeStuff*.apx"))
    finally:
        winreg.CloseKey(key)
    # уникальные, с сохранением порядка
    seen = set()
    unique = []
    for p in result:
        s = str(p).lower()
        if s not in seen:
            seen.add(s)
            unique.append(p)
    return unique


def find_installed_addon_any(inst, addons_dir):
    """Расширенный поиск установленного SomeStuff: стандартная папка → реестр.

    Возвращает (apx, target_dir, dupes):
      apx — найденный файл или None при конфликте; target_dir — куда ставить
      новый apx (родитель найденного файла либо стандартная папка аддонов);
      dupes — список конфликтующих файлов (если >1).
    """
    apx, dupes = find_installed_addon(addons_dir)
    if apx is not None:
        return apx, apx.parent, []
    if dupes:
        return None, addons_dir, dupes
    # стандартной папки нет/пуста — смотрим пути из реестра
    reg_found = find_registry_custom_addons(inst["version"])
    if not reg_found:
        return None, addons_dir, []
    if len(reg_found) == 1:
        return reg_found[0], reg_found[0].parent, []
    return None, reg_found[0].parent, reg_found


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
        backup_name = ADDON_NAME + "._old_version_%s" % (installed_stamp or "unknown")
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
    apx, target_dir, dupes = find_installed_addon_any(inst, addons)
    if dupes:
        print("Найдено несколько SomeStuff.apx (конфликт):")
        for d in dupes:
            print("  " + str(d))
        return 2
    if apx is None:
        print("SomeStuff не установлен (стандартная папка и реестр): %s" % addons)
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

# Палитра
C_BG = "#f4f5f8"
C_CARD = "#ffffff"
C_BORDER = "#e0e3ea"
C_TEXT = "#1f2430"
C_MUTED = "#6b7385"
C_ACCENT = "#2563eb"
C_ACCENT_DARK = "#1d4ed8"
# вид плашки статуса: (фон, текст)
BADGES = {
    "idle": ("#eceff4", "#6b7385"),
    "info": ("#dbeafe", "#1d4ed8"),
    "ok": ("#dcfce7", "#15803d"),
    "warn": ("#fef3c7", "#b45309"),
    "err": ("#fee2e2", "#b91c1c"),
}
FONT = "Segoe UI"


def fetch_remote(channel):
    """Последняя версия на канале → dict {err, tag, asset_name, commit, stamp, label, sub}."""
    if channel == "Test":
        asset_name, tag, commit, err = get_test_channel()
        if err:
            return {"err": err}
        stamp = parse_stamp_from_asset_name(asset_name)
        return {"err": None, "tag": tag, "asset_name": asset_name, "commit": commit,
                "stamp": stamp, "label": stamp or "—",
                "sub": ("коммит %s" % commit) if commit else ""}
    tag, err = get_release_channel()
    if err:
        return {"err": err}
    return {"err": None, "tag": tag, "asset_name": None, "commit": None, "stamp": None,
            "label": tag, "sub": "релиз"}


def remote_is_newer(channel, info, remote):
    if channel == "Test":
        return is_test_newer(info["date"], info["commit"], remote["stamp"], remote["commit"])
    return is_newer(info["version"], remote["tag"])


def find_asset(tag, asset_name, ac_ver):
    """Ищет ассет релиза под нужную версию Archicad. Возвращает (имя, url, ошибка)."""
    data, _, status = _http_get(GITHUB_API)
    if data is None:
        return None, None, "API недоступен: %s" % status
    try:
        releases = json.loads(data)
    except json.JSONDecodeError as e:
        return None, None, "Не удалось разобрать ответ API: %s" % e
    # тестовый канал: штамп общий для всех AC, подставляем свою версию
    needed = re.sub(r"AC\d+", "AC%d" % ac_ver, asset_name) if asset_name else None
    for rel in releases:
        if rel.get("tag_name") != tag:
            continue
        for a in rel.get("assets", []):
            name = a.get("name", "")
            if needed:
                ok = name == needed
            else:
                ok = name.startswith("SomeStuff.AC%d.Win." % ac_ver) and name.endswith(".zip")
            if ok and a.get("browser_download_url"):
                return name, a["browser_download_url"], None
    return None, None, "Сборка под AC%d не найдена в релизе %s" % (ac_ver, tag)


def download_file(url, dest, progress_cb=None):
    """Скачивание с прогрессом: progress_cb(скачано_байт, всего_байт)."""
    req = urllib.request.Request(url, headers={"User-Agent": UA})
    with urllib.request.urlopen(req, timeout=60) as r, open(dest, "wb") as f:
        total = int(r.headers.get("Content-Length") or 0)
        done = 0
        while True:
            chunk = r.read(64 * 1024)
            if not chunk:
                break
            f.write(chunk)
            done += len(chunk)
            if progress_cb:
                progress_cb(done, total)


def gui_main():
    import queue
    import threading
    import tkinter as tk
    from tkinter import ttk

    try:  # чёткий текст на мониторах с масштабом 125–150 %
        ctypes.windll.shcore.SetProcessDpiAwareness(1)
    except Exception:
        pass

    class UpdaterGUI:
        def __init__(self, root):
            self.root = root
            self.installs = find_archicad_installs()
            self.q = queue.Queue()          # события из рабочих потоков → GUI
            self.cancel_evt = threading.Event()
            self.phase = "idle"             # idle | check | wait | download | install
            self.can_update = False
            self._last_pct = -1

            root.title("SomeStuff Updater")
            root.configure(bg=C_BG)
            root.minsize(540, 1)
            root.resizable(True, False)
            self._setup_style()
            self._build()
            self._pump()
            root.after(150, self.start_check)   # проверяем сразу при запуске

        # ------------------------------------------------------------ стиль
        def _setup_style(self):
            s = ttk.Style(self.root)
            s.theme_use("clam")
            s.configure(".", background=C_BG, foreground=C_TEXT, font=(FONT, 10))
            s.configure("TFrame", background=C_BG)
            s.configure("TLabel", background=C_BG)
            s.configure("Card.TFrame", background=C_CARD)
            s.configure("Card.TLabel", background=C_CARD)
            s.configure("Muted.TLabel", foreground=C_MUTED)
            s.configure("Title.TLabel", font=(FONT, 17, "bold"))
            s.configure("Big.TLabel", font=(FONT, 22, "bold"), background=C_CARD)
            s.configure("Cap.TLabel", font=(FONT, 8, "bold"), foreground=C_MUTED, background=C_CARD)
            s.configure("Small.TLabel", font=(FONT, 9), foreground=C_MUTED, background=C_CARD)

            s.configure("TCombobox", fieldbackground=C_CARD, background=C_CARD,
                        bordercolor=C_BORDER, lightcolor=C_CARD, darkcolor=C_CARD,
                        arrowcolor=C_MUTED, padding=6)
            s.map("TCombobox", fieldbackground=[("readonly", C_CARD), ("disabled", C_BG)],
                  bordercolor=[("focus", C_ACCENT)], foreground=[("disabled", C_MUTED)])
            self.root.option_add("*TCombobox*Listbox.font", (FONT, 10))
            self.root.option_add("*TCombobox*Listbox.selectBackground", C_ACCENT)

            s.configure("Accent.TButton", background=C_ACCENT, foreground="white",
                        borderwidth=0, focusthickness=0, padding=(22, 9), font=(FONT, 10, "bold"))
            s.map("Accent.TButton",
                  background=[("disabled", "#b8c6ea"), ("pressed", C_ACCENT_DARK), ("active", C_ACCENT_DARK)],
                  foreground=[("disabled", "white")])
            s.configure("Ghost.TButton", background=C_CARD, foreground=C_TEXT,
                        bordercolor=C_BORDER, lightcolor=C_CARD, darkcolor=C_CARD,
                        focusthickness=0, padding=(18, 8))
            s.map("Ghost.TButton", background=[("active", "#eef1f6")],
                  foreground=[("disabled", C_MUTED)])

            # «сегментированный» переключатель канала
            s.configure("Seg.Toolbutton", background=C_BG, foreground=C_TEXT,
                        bordercolor=C_BORDER, lightcolor=C_BG, darkcolor=C_BG,
                        padding=(18, 6), focusthickness=0, focuscolor=C_BG, relief="flat")
            s.map("Seg.Toolbutton",
                  background=[("selected", C_ACCENT), ("active", "#e6eaf2")],
                  foreground=[("selected", "white"), ("disabled", C_MUTED)])

            s.configure("Horizontal.TProgressbar", troughcolor="#e3e7ef", background=C_ACCENT,
                        bordercolor="#e3e7ef", borderwidth=0, lightcolor=C_ACCENT, darkcolor=C_ACCENT, thickness=6)

        def _card(self, parent):
            outer = tk.Frame(parent, bg=C_BORDER)
            inner = ttk.Frame(outer, style="Card.TFrame", padding=(18, 16))
            inner.pack(fill="both", expand=True, padx=1, pady=1)
            return outer, inner

        # ---------------------------------------------------------- вёрстка
        def _build(self):
            main = ttk.Frame(self.root, padding=(22, 20, 22, 18))
            main.pack(fill="both", expand=True)

            # --- заголовок ---
            header = ttk.Frame(main)
            header.pack(fill="x")
            ttk.Label(header, text="SomeStuff Updater", style="Title.TLabel").pack(side="left")
            site_link = ttk.Label(header, text="Сайт ↗", style="Muted.TLabel", cursor="hand2")
            site_link.pack(side="right")
            site_link.bind("<Button-1>", lambda e: webbrowser.open(SITE_URL))
            ttk.Label(main, text="Обновление аддона SomeStuff для ARCHICAD",
                      style="Muted.TLabel").pack(anchor="w", pady=(0, 14))

            # --- карточка: куда ставим ---
            outer, card = self._card(main)
            outer.pack(fill="x")
            card.columnconfigure(1, weight=1)

            ttk.Label(card, text="ARCHICAD", style="Cap.TLabel").grid(row=0, column=0, sticky="w", padx=(0, 18))
            self.labels = ["Archicad %d  ·  %s" % (i["version"], i["lang"]) for i in self.installs]
            self.inst_var = tk.StringVar(value=self.labels[-1] if self.labels else "не найден")
            self.inst_box = ttk.Combobox(card, textvariable=self.inst_var, values=self.labels,
                                         state="readonly" if self.labels else "disabled")
            self.inst_box.grid(row=0, column=1, sticky="ew")
            self.inst_box.bind("<<ComboboxSelected>>", lambda e: self._on_changed())
            self.path_var = tk.StringVar()
            ttk.Label(card, textvariable=self.path_var, style="Small.TLabel").grid(
                row=1, column=1, sticky="w", pady=(4, 0))

            ttk.Label(card, text="КАНАЛ", style="Cap.TLabel").grid(
                row=2, column=0, sticky="w", pady=(16, 0), padx=(0, 18))
            seg = ttk.Frame(card, style="Card.TFrame")
            seg.grid(row=2, column=1, sticky="w", pady=(16, 0))
            self.channel_var = tk.StringVar(value="Release")
            self.seg_buttons = []
            for text, val in (("Релиз", "Release"), ("Тестовый", "Test")):
                b = ttk.Radiobutton(seg, text=text, value=val, variable=self.channel_var,
                                    style="Seg.Toolbutton", command=self._on_changed)
                b.pack(side="left")
                self.seg_buttons.append(b)

            # --- карточка: статус ---
            outer2, card2 = self._card(main)
            outer2.pack(fill="x", pady=(12, 0))
            card2.columnconfigure(0, weight=1, uniform="v")
            card2.columnconfigure(2, weight=1, uniform="v")

            self.inst_ver = tk.StringVar(value="—")
            self.inst_sub = tk.StringVar(value="")
            self.new_ver = tk.StringVar(value="—")
            self.new_sub = tk.StringVar(value="")
            for col, cap, v, sub in ((0, "УСТАНОВЛЕНА", self.inst_ver, self.inst_sub),
                                     (2, "ДОСТУПНА", self.new_ver, self.new_sub)):
                ttk.Label(card2, text=cap, style="Cap.TLabel").grid(row=0, column=col, sticky="w")
                ttk.Label(card2, textvariable=v, style="Big.TLabel").grid(row=1, column=col, sticky="w", pady=(2, 0))
                ttk.Label(card2, textvariable=sub, style="Small.TLabel").grid(row=2, column=col, sticky="w")
            ttk.Label(card2, text="→", style="Big.TLabel", foreground=C_BORDER).grid(
                row=1, column=1, padx=14)

            self.badge = tk.Label(card2, text="", font=(FONT, 9, "bold"), padx=10, pady=4,
                                  anchor="w", justify="left")
            self.badge.grid(row=3, column=0, columnspan=3, sticky="w", pady=(14, 0))
            self._set_badge("idle", "Проверка...")

            self.progress = ttk.Progressbar(card2, mode="determinate", maximum=100)
            self.progress.grid(row=4, column=0, columnspan=3, sticky="ew", pady=(14, 0))
            self.status_var = tk.StringVar(value="")
            ttk.Label(card2, textvariable=self.status_var, style="Small.TLabel").grid(
                row=5, column=0, columnspan=3, sticky="w", pady=(6, 0))

            # --- кнопки ---
            btns = ttk.Frame(main)
            btns.pack(fill="x", pady=(14, 0))
            self.update_btn = ttk.Button(btns, text="Обновить", style="Accent.TButton",
                                         command=self.on_action)
            self.update_btn.pack(side="right")
            self.check_btn = ttk.Button(btns, text="Проверить", style="Ghost.TButton",
                                        command=self.start_check)
            self.check_btn.pack(side="right", padx=(0, 8))

            # --- подробности (журнал) ---
            self.details_open = False
            self.toggle = ttk.Label(main, text="▸  Подробности", style="Muted.TLabel", cursor="hand2")
            self.toggle.pack(anchor="w", pady=(12, 0))
            self.toggle.bind("<Button-1>", lambda e: self._toggle_details())
            self.log_frame = tk.Frame(main, bg=C_BORDER)
            self.log = tk.Text(self.log_frame, height=10, wrap="word", relief="flat",
                               font=("Consolas", 9), bg=C_CARD, fg=C_TEXT, padx=10, pady=8,
                               state="disabled", borderwidth=0, highlightthickness=0)
            sb = ttk.Scrollbar(self.log_frame, command=self.log.yview)
            self.log.configure(yscrollcommand=sb.set)
            sb.pack(side="right", fill="y", padx=(0, 1), pady=1)
            self.log.pack(side="left", fill="both", expand=True, padx=(1, 0), pady=1)
            self.log.tag_configure("err", foreground="#b91c1c")
            self.log.tag_configure("ok", foreground="#15803d")
            self.log.tag_configure("dim", foreground=C_MUTED)

            if self.installs:
                self._log("Найдено установок Archicad: %d" % len(self.installs), "dim")
                for i in self.installs:
                    self._log("  AC%d %s — %s" % (i["version"], i["lang"], i["install_root"]), "dim")
                self._on_install_selected()
            else:
                self._set_badge("err", "Archicad не найден в реестре")
                self._log("Archicad в реестре не найден.", "err")
            self._set_phase("idle")

        # ---------------------------------------------------------- утилиты
        def _toggle_details(self, force=None):
            self.details_open = (not self.details_open) if force is None else force
            if self.details_open:
                self.toggle.configure(text="▾  Подробности")
                self.log_frame.pack(fill="x", pady=(6, 0))
            else:
                self.toggle.configure(text="▸  Подробности")
                self.log_frame.pack_forget()
            self.root.geometry("")  # подогнать высоту окна под содержимое

        def _log(self, msg, tag=None):
            self.log.configure(state="normal")
            self.log.insert("end", msg + "\n", tag or ())
            self.log.see("end")
            self.log.configure(state="disabled")

        def _set_badge(self, kind, text):
            bg, fg = BADGES[kind]
            self.badge.configure(text=text, bg=bg, fg=fg)

        def post(self, fn, *args):
            """Вызов функции в GUI-потоке из рабочего потока."""
            self.q.put(lambda: fn(*args))

        def log_t(self, msg, tag=None):
            self.post(self._log, msg, tag)

        def _pump(self):
            try:
                while True:
                    self.q.get_nowait()()
            except queue.Empty:
                pass
            except Exception as e:  # ошибка в обработчике не должна убить очередь
                self._log("Внутренняя ошибка интерфейса: %s" % e, "err")
            self.root.after(50, self._pump)

        def _selected_install(self):
            idx = self.inst_box.current()
            return self.installs[idx] if 0 <= idx < len(self.installs) else None

        def _on_install_selected(self):
            inst = self._selected_install()
            if inst is None and self.installs:
                self.inst_box.current(len(self.installs) - 1)
                inst = self._selected_install()
            self.path_var.set(inst["install_root"] if inst else "")

        def _on_changed(self):
            self._on_install_selected()
            self.start_check()

        def _set_phase(self, phase):
            self.phase = phase
            busy = phase != "idle"
            sel_state = "disabled" if busy or not self.installs else "readonly"
            self.inst_box.configure(state=sel_state)
            for b in self.seg_buttons:
                b.configure(state="disabled" if busy else "normal")
            self.check_btn.configure(state="disabled" if busy or not self.installs else "normal")
            if phase == "wait":
                self.update_btn.configure(text="Отмена", state="normal")
            else:
                self.update_btn.configure(text="Обновить",
                                          state="normal" if (self.can_update and not busy) else "disabled")
            if phase in ("check", "wait", "install"):
                self.progress.configure(mode="indeterminate")
                self.progress.start(14)
            else:
                self.progress.stop()
                self.progress.configure(mode="determinate")
                self.progress["value"] = 0
            if phase == "idle":
                self.status_var.set("")

        def _render(self, info, remote, kind, text, can_update):
            self.inst_ver.set((info or {}).get("version") or "—")
            sub = []
            if info and info.get("date"):
                sub.append(info["date"])
            if info and info.get("commit"):
                sub.append("#" + info["commit"])
            self.inst_sub.set("  ·  ".join(sub))
            self.new_ver.set((remote or {}).get("label") or "—")
            self.new_sub.set((remote or {}).get("sub") or "")
            self._set_badge(kind, text)
            self.can_update = can_update
            self._set_phase("idle")

        def _fail(self, text, info=None, remote=None):
            self._render(info, remote, "err", text, False)
            self._log(text, "err")
            self._toggle_details(True)

        # --------------------------------------------------------- проверка
        def start_check(self):
            if self.phase != "idle":
                return
            inst = self._selected_install()
            if inst is None:
                return
            self.can_update = False
            self._set_phase("check")
            self._set_badge("idle", "Проверяем...")
            self.status_var.set("Читаем установленную версию и запрашиваем GitHub")
            self._log("--- Проверка (%s) ---" % self.channel_var.get(), "dim")
            threading.Thread(target=self._check_worker, args=(inst, self.channel_var.get()),
                             daemon=True).start()

        def _check_worker(self, inst, channel):
            try:
                addons = find_addons_dir(inst["install_root"])
                if addons is None:
                    self.post(self._fail, "Папка аддонов не найдена в %s" % inst["install_root"])
                    return
                apx, target_dir, dupes = find_installed_addon_any(inst, addons)
                if dupes:
                    for d in dupes:
                        self.log_t("  " + str(d))
                    self.post(self._fail, "Найдено несколько SomeStuff.apx — оставьте один")
                    return
                if apx is None:
                    self.post(self._render, None, None, "warn",
                              "SomeStuff не установлен в эту версию Archicad", False)
                    self.log_t("SomeStuff не найден в %s (и в реестре Add-On Manager)" % addons)
                    return
                info = read_apx_version_info(apx)
                self.log_t("Установлено: %s (дата %s, коммит %s, PE %s)" % (
                    info["version"], info["date"], info["commit"], info["pe_version"]))
                remote = fetch_remote(channel)
                if remote.get("err"):
                    self.post(self._fail, "Не удалось получить версию с GitHub: %s" % remote["err"], info)
                    return
                self.log_t("На канале: %s %s" % (remote["label"], remote["sub"]))
                if remote_is_newer(channel, info, remote):
                    self.post(self._render, info, remote, "info", "Доступно обновление", True)
                else:
                    self.post(self._render, info, remote, "ok", "Установлена актуальная версия", False)
            except Exception as e:
                self.post(self._fail, "Ошибка проверки: %s" % e)

        # ------------------------------------------------------- обновление
        def on_action(self):
            if self.phase == "wait":       # кнопка «Отмена» во время ожидания закрытия Archicad
                self.cancel_evt.set()
                return
            if self.phase != "idle" or not self.can_update:
                return
            inst = self._selected_install()
            if inst is None:
                return
            self.cancel_evt.clear()
            self._set_phase("install")
            self._set_badge("idle", "Подготовка...")
            self._log("--- Обновление ---", "dim")
            threading.Thread(target=self._update_worker, args=(inst, self.channel_var.get()),
                             daemon=True).start()

        def _phase(self, phase, badge_kind, badge_text, status=""):
            self._set_phase(phase)
            self._set_badge(badge_kind, badge_text)
            self.status_var.set(status)

        def _on_progress(self, done, total):
            if total <= 0:
                self.status_var.set("Скачано %.1f МБ" % (done / 1048576.0))
                return
            pct = int(done * 100 / total)
            if pct == self._last_pct:
                return
            self._last_pct = pct
            self.progress["value"] = pct
            self.status_var.set("%d %%  ·  %.1f / %.1f МБ" % (pct, done / 1048576.0, total / 1048576.0))

        def _update_worker(self, inst, channel):
            try:
                addons = find_addons_dir(inst["install_root"])
                apx, target_dir, dupes = find_installed_addon_any(inst, addons)
                if dupes:
                    for d in dupes:
                        self.log_t("  " + str(d))
                    self.post(self._fail, "Найдено несколько SomeStuff.apx — оставьте один")
                    return
                if apx is None:
                    self.post(self._fail, "SomeStuff не найден (стандартная папка и реестр) — обновление невозможно")
                    return
                self.log_t("Установлено: %s" % apx)
                old = read_apx_version_info(apx)
                remote = fetch_remote(channel)
                if remote.get("err"):
                    self.post(self._fail, "Не удалось получить версию с GitHub: %s" % remote["err"], old)
                    return
                if not remote_is_newer(channel, old, remote):
                    self.post(self._render, old, remote, "ok", "Обновление не требуется", False)
                    return

                # 1. ждём закрытия Archicad
                if get_archicad_processes(inst["install_root"]):
                    self.post(self._phase, "wait", "warn",
                              "Закройте Archicad — обновление продолжится автоматически",
                              "Ожидание закрытия Archicad (до 5 минут)")
                    start = time.time()
                    while get_archicad_processes(inst["install_root"]):
                        if self.cancel_evt.is_set():
                            self.log_t("Отменено пользователем.")
                            self.post(self._render, old, remote, "idle", "Обновление отменено", True)
                            return
                        if time.time() - start > 300:
                            self.post(self._fail, "Archicad не закрылся за 5 минут — обновление отменено",
                                      old, remote)
                            return
                        time.sleep(1)
                    self.log_t("Archicad закрыт.")

                # 2. скачиваем
                name, url, err = find_asset(remote["tag"], remote["asset_name"], inst["version"])
                if err:
                    self.post(self._fail, err, old, remote)
                    return
                self._last_pct = -1
                self.post(self._phase, "download", "info", "Скачивание обновления", "Подключение...")
                self.log_t("Скачиваем %s" % url)
                dest = Path(tempfile.gettempdir()) / name
                try:
                    download_file(url, dest, lambda d, t: self.post(self._on_progress, d, t))
                except Exception as e:
                    self.post(self._fail, "Ошибка скачивания: %s" % e, old, remote)
                    return

                # 3. устанавливаем
                self.post(self._phase, "install", "info", "Установка...", "Копируем файл в папку аддонов")
                new_stamp, err = do_update_flow(target_dir, dest, old["version"], log_cb=self.log_t)
                if err:
                    self.post(self._fail, "Ошибка установки: %s" % err, old, remote)
                    return
                new_info = read_apx_version_info(apx)
                self.log_t("Обновление завершено: %s" % new_stamp, "ok")
                self.post(self._render, new_info, remote, "ok", "Обновление установлено", False)
            except Exception as e:
                self.post(self._fail, "Ошибка обновления: %s" % e)

    root = tk.Tk()
    UpdaterGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
