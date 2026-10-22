#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""roombook_setup.py — первичная проверка готовности проекта Archicad к функции отделки.

Проверяет через JSON-порт Archicad (порт 19723+):
  1. Доступность порта (автопоиск среди запущенных экземпляров).
  2. Загружен ли аддон SomeStuff (команда Health в namespace SomeStuffCommand).
  3. Наличие избранных: smstf wall / floor / ceil / window / reveal side / reveal up.
  4. Наличие свойств зон с маркерами some_stuff_fin_* в описаниях.
  5. Наличие классификационных классов some_stuff_fin_*.

Только стандартная библиотека Python. Режим только для чтения — скрипт ничего
не меняет в проекте (создание/настройка — отдельная команда, планируется).

Запуск:
    python python_scripts/roombook_setup.py                # GUI (по умолчанию)
    python python_scripts/roombook_setup.py --cli          # консоль, все экземпляры
    python python_scripts/roombook_setup.py --cli --port 19723
"""

import argparse
import concurrent.futures
import json
import sys
import threading
import urllib.request

import tkinter as tk
from tkinter import scrolledtext, ttk

# ---------------------------------------------------------------------------
# Константы (контракт из Sources/AddOn/roombook)
# ---------------------------------------------------------------------------

HOST = "127.0.0.1"
PORT_RANGE = range(19723, 19744)  # 19723..19743, штатный диапазон Archicad
TIMEOUT = 15

ADDON_NAMESPACE = "SomeStuffCommand"  # CommandBase.cpp:8

# Избранные, наличие которых проверяет Favorite_GetDict (RoombookData.cpp:7454-7476).
# required=True  — отсутствие приводит к использованию настроек инструмента по умолчанию.
# required=False — отсутствие допустимо, используется fallback ('smstf wall' для откосов).
REQUIRED_FAVORITES = [
    ("smstf wall", True),
    ("smstf floor", True),
    ("smstf ceil", True),
    ("smstf window", True),
    ("smstf reveal side", False),
    ("smstf reveal up", False),
]

# Маркеры в описаниях свойств зон (RoombookData.cpp:5370-5539).
# Поиск в аддоне: description.Contains(marker), исключая marker + "_" (защита
# '...result' от '...result_bytype'). В скрипте применяется та же логика.
REQUIRED_PROPERTY_MARKERS = [
    "some_stuff_fin_has_ceil",
    "some_stuff_fin_has_floor",
    "some_stuff_fin_floor_by_slab",
    "some_stuff_fin_ceil_by_slab",
    "some_stuff_fin_type",
    "some_stuff_fin_type_ceil",
    "some_stuff_fin_type_floor",
    "some_stuff_fin_main_material",
    "some_stuff_fin_main_height",
    "some_stuff_fin_up_material",
    "some_stuff_fin_down_material",
    "some_stuff_fin_down_height",
    "some_stuff_fin_column_material",
    "some_stuff_fin_ceil_material",
    "some_stuff_fin_ceil_result",
    "some_stuff_fin_main_result",
    "some_stuff_fin_up_result",
    "some_stuff_fin_down_result",
    "some_stuff_fin_reveal_result",
    "some_stuff_fin_ceil_result_bytype",
    "some_stuff_fin_main_result_bytype",
    "some_stuff_fin_up_result_bytype",
    "some_stuff_fin_down_result_bytype",
    "some_stuff_fin_reveal_result_bytype",
]

# Классы классификации (Roombook.hpp:323-332).
REQUIRED_CLASSES = [
    "some_stuff_fin_class",
    "some_stuff_fin_walls",
    "some_stuff_fin_down_walls",
    "some_stuff_fin_reveals",
    "some_stuff_fin_windows_reveals",
    "some_stuff_fin_doors_reveals",
    "some_stuff_fin_columns",
    "some_stuff_fin_floors",
    "some_stuff_fin_beams",
    "some_stuff_fin_ceils",
]

# Комбинация слоёв ищется по суффиксу (Roombook.cpp:69-83).
LAYER_COMBINATION_SUFFIX = "smstf_fin"


# ---------------------------------------------------------------------------
# Транспорт JSON-команд
# ---------------------------------------------------------------------------

def raw_post(port, command, parameters):
    """Отправляет команду на JSON-порт. Возвращает (dict_ответа, None) или (None, текст_ошибки)."""
    url = "http://%s:%d/" % (HOST, port)
    body = json.dumps({"command": command, "parameters": parameters}).encode("utf-8")
    req = urllib.request.Request(url, data=body, headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
            data = json.loads(resp.read().decode("utf-8"))
        return data, None
    except Exception as exc:  # noqa: BLE001 — любой сбой транспорта трактуем как недоступность
        return None, str(exc)


def addon_call(port, command_name, parameters=None):
    """Вызывает команду аддона через API.ExecuteAddOnCommand."""
    return raw_post(port, "API.ExecuteAddOnCommand", {
        "addOnCommandId": {"commandNamespace": ADDON_NAMESPACE, "commandName": command_name},
        "addOnCommandParameters": parameters or {},
    })


# ---------------------------------------------------------------------------
# Шаги проверки
# ---------------------------------------------------------------------------

def discover_ports(explicit_port=None):
    """Ищет живые порты. Живой = любой HTTP-ответ (даже JSON-ошибка 200).

    Проверка идёт параллельно с ограничением числа workers.
    Недоступные порты обрабатываются волнами, каждая ограничена таймаутом.
    """
    ports = [explicit_port] if explicit_port else list(PORT_RANGE)
    if len(ports) == 1:
        data, _err = raw_post(ports[0], "API.GetProductInfo", {})
        return [ports[0]] if data is not None else []
    workers = min(len(ports), 16)
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as pool:
        futures = {pool.submit(raw_post, p, "API.GetProductInfo", {}): p for p in ports}
        alive = []
        for fut in concurrent.futures.as_completed(futures):
            data, _err = fut.result()
            if data is not None:
                alive.append(futures[fut])
    return sorted(alive)


def get_instance_info(port):
    """Собирает сведения об одном экземпляре: порт, имя проекта, статус аддона.

    Имя проекта берётся из ответа Health (HealthCommand.cpp добавляет projectName
    через ACAPI_Environment(APIEnv_ProjectID)). Пока аддон не загружен (4010),
    имя недоступно — используется fallback «Archicad (порт N)».
    """
    info = {"port": port, "name": "Archicad (порт %d)" % port, "addon_loaded": False, "version": None}
    data, err = addon_call(port, "Health")
    if data is None:
        info["detail"] = "нет ответа: %s" % err
        return info
    if data.get("succeeded"):
        result = data.get("result", {}).get("addOnCommandResponse", {})
        name = result.get("projectName")
        if name:
            info["name"] = name
        info["addon_loaded"] = True
        info["version"] = result.get("version")
        info["detail"] = "аддон %s, версия %s" % (result.get("addon", "?"), result.get("version", "?"))
    else:
        code = data.get("error", {}).get("code")
        if code == 4010:
            info["detail"] = "аддон SomeStuff не загружен (4010)"
        elif code == 4001:
            info["detail"] = "Archicad занят модальным диалогом — закройте его и обновите список"
        else:
            info["detail"] = "неожиданный ответ: %s" % json.dumps(data, ensure_ascii=False)
    return info


def discover_instances(explicit_port=None):
    """Список экземпляров Archicad с именем проекта и портом (для GUI-выпадающего списка)."""
    ports = discover_ports(explicit_port)
    return [get_instance_info(port) for port in ports]


def check_addon(port):
    """Health-команда аддона: загружен ли SomeStuff и его JSON-команды."""
    data, err = addon_call(port, "Health")
    if data is None:
        return {"ok": False, "detail": "нет ответа: %s" % err}
    if data.get("succeeded"):
        result = data.get("result", {}).get("addOnCommandResponse", {})
        return {"ok": True, "detail": "аддон %s, версия %s" % (
            result.get("addon", "?"), result.get("version", "?"))}
    error = data.get("error", {})
    code = error.get("code")
    if code == 4010:
        return {"ok": False, "detail": "аддон SomeStuff не загружен (4010) — установите свежий .apx и переоткройте проект"}
    if code == 4001:
        return {"ok": None, "detail": "Archicad занят модальным диалогом («Параметры Выбранного Окна» и т.п.) — закройте его и повторите проверку"}
    return {"ok": False, "detail": "неожиданный ответ: %s" % json.dumps(data, ensure_ascii=False)}


def check_favorites(port):
    """Проверяет наличие рекомендуемых избранных.

    Штатный JSON API AC25 не имеет команды перечисления избранного (см. исследование:
    отсутствует и GetAllFavorites, и команда аддона). Проверяем через аддонную
    команду RoomBook-подобного доступа невозможно, поэтому честно сообщаем, что
    проверка требует аддонной команды (запланирована).
    """
    data, err = addon_call(port, "FavoritesList")
    if data is not None and data.get("succeeded"):
        names = [n.lower().strip() for n in data.get("result", {}).get("addOnCommandResponse", {}).get("favorites", [])]
        rows = []
        for name, required in REQUIRED_FAVORITES:
            rows.append({"name": name, "required": required, "found": name in names})
        return {"ok": all(not r["required"] or r["found"] for r in rows), "rows": rows, "detail": "проверено аддонной командой FavoritesList"}
    # Команды нет (2002/4010) — сообщаем о невозможности проверки штатными средствами.
    return {
        "ok": None,
        "rows": [],
        "detail": "штатный JSON API AC25 не умеет перечислять избранное; требуется аддонная команда (todo)",
    }


def marker_matches(description, marker):
    """Логика поиска маркера, идентичная аддонной: Contains(marker) и НЕ Contains(marker + '_')."""
    if marker in description:
        if (marker + "_") not in description:
            return True
    return False


def check_properties(port):
    """Проверяет наличие определений свойств по маркерам в описаниях.

    Контракты AC25 (проверено на живом порту 2026-xx):
      GetAllPropertyNames -> {properties: [{type, nonLocalizedName | localizedName:[группа, имя]}]}
      GetPropertyIds      -> по {type:"UserDefined", localizedName:[группа, имя]}
      GetDetailsOfProperties -> propertyDefinitions[].propertyDefinition.{group.name, name, description}
    Маркеры ищутся в description по аддонной логике Contains(marker) без marker+"_".
    """
    data, err = raw_post(port, "API.GetAllPropertyNames", {})
    if data is None:
        return {"ok": None, "rows": [], "detail": "нет ответа: %s" % err}
    if not data.get("succeeded"):
        code = data.get("error", {}).get("code")
        return {
            "ok": None,
            "rows": [],
            "detail": "перечисление свойств недоступно (код %s)" % code,
        }
    props = data.get("result", {}).get("properties") or []
    if not props:
        return {"ok": None, "rows": [], "detail": "перечисление вернуло пустой список"}

    found = set()
    # Проверяем только UserDefined — маркеры расположены в описаниях кастомных свойств.
    for prop in props:
        if prop.get("type") != "UserDefined":
            continue
        localizedName = prop.get("localizedName") or []
        if not localizedName:
            continue
        for desc in _resolve_descriptions(port, localizedName):
            for marker in REQUIRED_PROPERTY_MARKERS:
                if marker_matches(desc, marker):
                    found.add(marker)
    rows = [{"marker": m, "found": m in found} for m in REQUIRED_PROPERTY_MARKERS]
    missing = [r["marker"] for r in rows if not r["found"]]
    return {
        "ok": len(missing) == 0,
        "rows": rows,
        "detail": "маркеров найдено %d из %d (UserDefined всего %d)" % (
            len(found), len(REQUIRED_PROPERTY_MARKERS),
            sum(1 for p in props if p.get("type") == "UserDefined")),
    }


def _resolve_descriptions(port, localizedName):
    """Для одного свойства: GetPropertyIds -> GetDetailsOfProperties -> описание."""
    data, _ = raw_post(port, "API.GetPropertyIds", {
        "properties": [{"type": "UserDefined", "localizedName": localizedName}]})
    if data is None or not data.get("succeeded"):
        return []
    for p in data.get("result", {}).get("properties", []):
        guid = (p.get("propertyId") or {}).get("guid")
        if not guid:
            continue
        det, _ = raw_post(port, "API.GetDetailsOfProperties", {
            "properties": [{"propertyId": {"guid": guid}}]})
        if det and det.get("succeeded"):
            items = det.get("result", {}).get("propertyDefinitions", []) or []
            for item in items:
                defn = item.get("propertyDefinition") or item
                yield defn.get("description", "")


def check_classifications(port):
    """Проверяет наличие классов some_stuff_fin_* в системах классификации.

    Контракты AC25 (проверено на живом порту):
      GetAllClassificationSystems   -> {classificationSystems: [{classificationSystemId:{guid}, name, ...}]}
      GetAllClassificationsInSystem -> {classificationItems: [{classificationItem: {id, name, description, children:[...]}}]}
    Параметр второй команды: {"classificationSystemId": {"guid": ...}}.
    """
    data, err = raw_post(port, "API.GetAllClassificationSystems", {})
    if data is None:
        return {"ok": None, "rows": [], "detail": "нет ответа: %s" % err}
    if not data.get("succeeded"):
        code = data.get("error", {}).get("code")
        return {"ok": None, "rows": [], "detail": "команда недоступна (код %s)" % code}
    systems = data.get("result", {}).get("classificationSystems") or []
    if not systems:
        return {"ok": None, "rows": [], "detail": "в проекте нет систем классификации"}

    found = set()
    system_names = []
    for system in systems:
        guid = ((system.get("classificationSystemId") or {}).get("guid"))
        if not guid:
            continue
        system_names.append(system.get("name", "?"))
        items, _ = raw_post(port, "API.GetAllClassificationsInSystem", {
            "classificationSystemId": {"guid": guid}})
        if not (items and items.get("succeeded")):
            continue
        for root in items.get("result", {}).get("classificationItems", []) or []:
            _collect_classification_ids(root, found)
    rows = [{"cls": c, "found": c.lower() in found} for c in REQUIRED_CLASSES]
    missing = [r["cls"] for r in rows if not r["found"]]
    matched = sum(1 for r in rows if r["found"])
    return {
        "ok": len(missing) == 0,
        "rows": rows,
        "detail": "системы: %s; классов some_stuff_fin_* найдено %d из %d (элементов в дереве: %d)" % (
            ", ".join(system_names) or "—", matched, len(REQUIRED_CLASSES), len(found)),
    }


def _collect_classification_ids(node, found):
    """Рекурсивно собирает id/name/description из дерева классификации."""
    item = node.get("classificationItem") or node
    for key in ("id", "name", "description"):
        value = item.get(key)
        if value:
            found.add(value.lower())
    for child in item.get("children", []) or []:
        _collect_classification_ids(child, found)


def check_layer_combination(port):
    """Проверяет наличие комбинации слоёв с суффиксом smstf_fin.

    Штатный JSON API AC25 не отдаёт комбинации слоёв; проверка требует аддонной команды.
    """
    return {"ok": None, "rows": [], "detail": "штатный JSON API AC25 не отдаёт комбинации слоёв; требуется аддонная команда (todo)"}


# ---------------------------------------------------------------------------
# Отчёт
# ---------------------------------------------------------------------------

def report(port, sections):
    print("=" * 72)
    print("Archicad JSON-порт %s:%d" % (HOST, port))
    print("=" * 72)
    for title, section in sections.items():
        status = "?" if section["ok"] is None else ("OK" if section["ok"] else "FAIL")
        print("\n[%s] %s" % (status, title))
        print("    %s" % section["detail"])
        for row in section.get("rows", []):
            label = row.get("name") or row.get("marker") or row.get("cls")
            found = row.get("found")
            req = row.get("required")
            mark = "OK " if found else "MISS"
            extra = " (обяз.)" if req else " (опц.)"
            print("    %-6s %s%s" % (mark, label, extra))
    print()


def collect_report(port):
    """Собирает результаты всех проверок одного порта в dict (без печати/выхода).

    Используется и CLI, и GUI этого файла — единая логика проверок.
    """
    return {
        "Аддон SomeStuff": check_addon(port),
        "Избранные (favorites)": check_favorites(port),
        "Свойства зон (some_stuff_fin_*)": check_properties(port),
        "Классификационные классы": check_classifications(port),
        "Комбинация слоёв smstf_fin": check_layer_combination(port),
    }


# ---------------------------------------------------------------------------
# GUI (tkinter) — по образцу libpack2gsm; логика проверки — функции выше
# ---------------------------------------------------------------------------

APP_VERSION = "v1.0"

STRINGS = {
    "i18n": {
        "ru": {
            "window_title": "Проверка отделки (Roombook) " + APP_VERSION,
            "label_language": "Язык интерфейса:",
            "label_port": "Экземпляр Archicad:",
            "checkbutton_auto": "Проверить все найденные экземпляры",
            "btn_refresh": "Обновить порты",
            "btn_run": "Запустить проверку",
            "status_ready": "Готов к работе",
            "status_scanning": "Поиск Archicad...",
            "status_running": "Проверка выполняется...",
            "status_no_archicad": "Archicad не найден. Запустите Archicad и откройте проект.",
            "status_done_ok": "Проверка завершена: всё в порядке.",
            "status_done_fail": "Проверка завершена: есть проблемы.",
            "status_done_incomplete": "Проверка завершена не полностью: часть данных не проверена.",
            "status_error": "Ошибка: ",
        },
        "en": {
            "window_title": "Roombook setup check " + APP_VERSION,
            "label_language": "Interface language:",
            "label_port": "Archicad instance:",
            "checkbutton_auto": "Check all detected instances",
            "btn_refresh": "Refresh ports",
            "btn_run": "Run check",
            "status_ready": "Ready",
            "status_scanning": "Scanning for Archicad...",
            "status_running": "Check in progress...",
            "status_no_archicad": "Archicad not found. Start Archicad and open the project.",
            "status_done_ok": "Check finished: all good.",
            "status_done_fail": "Check finished: problems found.",
            "status_done_incomplete": "Check incomplete: some data could not be checked.",
            "status_error": "Error: ",
        },
    },
}


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(STRINGS["i18n"]["ru"]["window_title"])
        self.geometry("780x560")
        self.minsize(640, 420)
        self.lang = "ru"
        self.instances = []          # список экземпляров: {port, name, addon_loaded, version}
        self._port_by_label = {}     # мапа «имя — порт» -> порт для выпадающего списка
        self._busy = False
        self._worker_error = None
        self._build_widgets()
        self._apply_language()

    # ------------------------------------------------------------------ UI
    def _build_widgets(self):
        pad = {"padx": 6, "pady": 4}

        top = ttk.Frame(self)
        top.pack(fill="x", **pad)

        self.lbl_lang = ttk.Label(top, text="")
        self.lbl_lang.pack(side="left", **pad)
        self.cmb_lang = ttk.Combobox(top, values=["ru", "en"], state="readonly", width=5)
        self.cmb_lang.current(0)
        self.cmb_lang.pack(side="left", **pad)
        self.cmb_lang.bind("<<ComboboxSelected>>", lambda _e: self._apply_language())

        selection = ttk.Frame(self)
        selection.pack(fill="x", **pad)
        self.lbl_port = ttk.Label(selection, text="")
        self.lbl_port.pack(side="left", **pad)
        self.cmb_port = ttk.Combobox(selection, values=[], state="readonly", width=32)
        self.cmb_port.pack(side="left", **pad)

        self.btn_refresh = ttk.Button(selection, text="", command=self.refresh_ports)
        self.btn_refresh.pack(side="left", **pad)

        actions = ttk.Frame(self)
        actions.pack(fill="x", **pad)
        self.var_auto = tk.BooleanVar(value=False)
        self.chk_auto = ttk.Checkbutton(actions, text="", variable=self.var_auto)
        self.chk_auto.pack(side="left", **pad)

        self.btn_run = ttk.Button(actions, text="", command=self.run_check)
        self.btn_run.pack(side="right", **pad)

        self.status = tk.StringVar()
        self.status.set("")
        lbl_status = ttk.Label(self, textvariable=self.status, anchor="w")
        lbl_status.pack(fill="x", **pad)

        self.txt = scrolledtext.ScrolledText(self, wrap="word", font=("Consolas", 9))
        self.txt.pack(fill="both", expand=True, **pad)

    def _apply_language(self):
        self.lang = self.cmb_lang.get() or "ru"
        s = STRINGS["i18n"][self.lang]
        self.title(s["window_title"])
        self.lbl_lang.config(text=s["label_language"])
        self.lbl_port.config(text=s["label_port"])
        self.btn_refresh.config(text=s["btn_refresh"])
        self.chk_auto.config(text=s["checkbutton_auto"])
        self.btn_run.config(text=s["btn_run"])
        if self.status.get() in ("", "Ready", "Готов к работе"):
            self.status.set(s["status_ready"])
        self._fill_port_list()

    def _set_busy(self, busy):
        self._busy = busy
        state = "disabled" if busy else "normal"
        self.btn_run.config(state=state)
        self.btn_refresh.config(state=state)
        self.chk_auto.config(state=state)
        self.cmb_port.config(state="disabled" if busy else "readonly")

    def _show_worker_error(self):
        self.status.set(STRINGS["i18n"][self.lang]["status_error"] + self._worker_error)
        self._set_busy(False)

    # ----------------------------------------------------------- действия
    def refresh_ports(self):
        if self._busy:
            return
        self._set_busy(True)
        self._worker_error = None
        s = STRINGS["i18n"][self.lang]
        self.status.set(s["status_scanning"])
        self.btn_run.config(state="disabled")
        self._scan_result = None
        self._scan_thread = threading.Thread(
            target=self._scan_worker, daemon=True)
        self._scan_thread.start()
        # after() из фонового потока в tkinter небезопасен — опрос из главного
        self.after(100, self._poll_scan)

    def _scan_worker(self):
        try:
            self._scan_result = discover_instances(None)
        except Exception as exc:
            self._worker_error = str(exc)

    def _poll_scan(self):
        if self._scan_thread.is_alive():
            self.after(100, self._poll_scan)
        else:
            if self._worker_error is not None:
                self._show_worker_error()
            else:
                self._ports_found(self._scan_result or [])

    def _ports_found(self, instances):
        self.instances = instances
        self._fill_port_list()
        s = STRINGS["i18n"][self.lang]
        if instances:
            names = ", ".join(i["name"] for i in instances)
            self.status.set("Archicad: " + names)
        else:
            self.status.set(s["status_no_archicad"])
        self._set_busy(False)

    def _fill_port_list(self):
        # Выпадающий список: «имя проекта — порт» (имя приходит из Health аддона).
        labels = []
        for i in self.instances:
            label = "%s — %d" % (i["name"], i["port"])
            labels.append(label)
        self._port_by_label = dict(zip(labels, [i["port"] for i in self.instances]))
        self.cmb_port.config(values=labels)
        if labels:
            if self.cmb_port.get() not in labels:
                self.cmb_port.current(0)
        else:
            self.cmb_port.set("")

    def run_check(self):
        if self._busy:
            return
        s = STRINGS["i18n"][self.lang]
        if self.var_auto.get():
            port = None
        else:
            port = self._port_by_label.get(self.cmb_port.get())
            if port is None:
                self.status.set(s["status_no_archicad"])
                return
        self._set_busy(True)
        self._worker_error = None
        self.status.set(s["status_running"])
        self.btn_run.config(state="disabled")
        self.txt.delete("1.0", "end")
        self._check_result = None
        self._check_thread = threading.Thread(
            target=self._check_worker, args=(port,), daemon=True)
        self._check_thread.start()
        self.after(100, self._poll_check)

    def _check_worker(self, port):
        try:
            self._collect_check_result(port)
        except Exception as exc:
            self._worker_error = str(exc)

    def _collect_check_result(self, port):
        ports = [port] if port else discover_ports(None)
        if not ports:
            self._check_result = ("", False, False)
            return
        text_parts = []
        any_fail = False
        any_unknown = False
        for p in ports:
            sections = collect_report(p)
            text_parts.append(self._format_report(p, sections))
            if any(sec["ok"] is False for sec in sections.values()):
                any_fail = True
            if any(sec["ok"] is None for sec in sections.values()):
                any_unknown = True
        self._check_result = ("".join(text_parts), any_fail, any_unknown)

    def _poll_check(self):
        if self._check_thread.is_alive():
            self.after(100, self._poll_check)
        else:
            if self._worker_error is not None:
                self._show_worker_error()
            else:
                self._check_done(*(self._check_result or ("", False, False)))

    @staticmethod
    def _format_report(port, sections):
        lines = ["=" * 72, "Archicad JSON-порт 127.0.0.1:%d" % port, "=" * 72]
        for title, section in sections.items():
            ok = section["ok"]
            mark = "?" if ok is None else ("OK " if ok else "FAIL")
            lines.append("")
            lines.append("[%s] %s" % (mark, title))
            lines.append("    " + section["detail"])
            for row in section.get("rows", []):
                label = row.get("name") or row.get("marker") or row.get("cls")
                found = row.get("found")
                req = row.get("required")
                mark = "OK " if found else "MISS"
                extra = " (обяз.)" if req else " (опц.)"
                lines.append("    %-6s %s%s" % (mark, label, extra))
        lines.append("")
        return "\n".join(lines) + "\n"

    def _check_done(self, text, any_fail=False, any_unknown=False):
        if not text:
            self.status.set(STRINGS["i18n"][self.lang]["status_no_archicad"])
        else:
            self.txt.insert("1.0", text)
            self.status.set(STRINGS["i18n"][self.lang][
                "status_done_fail" if any_fail else (
                    "status_done_incomplete" if any_unknown else "status_done_ok")])
        self._set_busy(False)


# ---------------------------------------------------------------------------
# Точка входа: без аргументов — GUI; --cli — консольный отчёт
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(description="Проверка готовности проекта к функции отделки (Roombook).")
    parser.add_argument("--port", type=int, default=None,
                        help="консольный режим: проверить только указанный порт")
    parser.add_argument("--cli", action="store_true",
                        help="консольный режим (по умолчанию открывается GUI)")
    args = parser.parse_args()

    if not args.cli and args.port is None:
        App().mainloop()
        return 0

    ports = discover_ports(args.port)
    if not ports:
        print("Archicad не найден на портах %d..%d. Запустите Archicad и откройте проект." % (PORT_RANGE.start, PORT_RANGE.stop - 1))
        return 1
    print("Найдены живые порты: %s" % ", ".join(map(str, ports)))

    exit_code = 0
    for port in ports:
        sections = collect_report(port)
        report(port, sections)
        if any(s["ok"] is False for s in sections.values()):
            exit_code = 2
        elif exit_code == 0 and any(s["ok"] is None for s in sections.values()):
            exit_code = 3
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
