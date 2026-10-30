#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""roombook_setup.py — первичная проверка готовности проекта Archicad к функции отделки.

Проверяет через JSON-порт Archicad (порт 19723+):
  1. Доступность порта (автопоиск среди запущенных экземпляров).
  2. Загружен ли аддон SomeStuff (команда Health в namespace SomeStuffCommand).
  3. Наличие рекомендуемых избранных, включая плинтус и карниз.
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
import ctypes
import queue
import json
import sys
import webbrowser
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

# Имена по умолчанию из FavoriteDefaultOtd (RoombookConstants.hpp).
# Первые четыре проверяет Favorite_GetDict; остальные зависят от вида отделки.
REQUIRED_FAVORITES = [
    ("smstf wall", True),
    ("smstf floor", True),
    ("smstf ceil", True),
    ("smstf window", True),
    ("smstf reveal side", False),
    ("smstf reveal up", False),
    ("smstf ceil side", False),
    ("smstf floor side", False),
    ("smstf skirting", False),
    ("smstf cornice", False),
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

# Понятные тексты описаний свойств (wiki/ru/Finishing-ru.md, колонка «Назначение»).
# Маркер обязан остаться в описании (аддон ищет свойства по Contains(marker)),
# поэтому описание ставится в формате «Понятный текст (маркер)».
MARKER_DESCRIPTIONS = {
    "some_stuff_fin_has_ceil": "Наличие потолка",
    "some_stuff_fin_has_floor": "Наличие пола",
    "some_stuff_fin_floor_by_slab": "Считать и строить пол (и плинтус пола) только по перекрытиям",
    "some_stuff_fin_ceil_by_slab": "Считать и строить потолок (и карниз потолка) только по перекрытиям",
    "some_stuff_fin_type": "Тип отделки стен для объединения зон",
    "some_stuff_fin_type_ceil": "Тип отделки потолка для объединения зон",
    "some_stuff_fin_type_floor": "Тип отделки пола для объединения зон",
    "some_stuff_fin_main_material": "Имя покрытия основной части стены (чистовая отделка)",
    "some_stuff_fin_main_height": "Высота зоны до потолка (если не задан, либо равен нулю - принимается высота зоны)",
    "some_stuff_fin_up_material": "Имя покрытия верхней части (разность высоты зоны и высоты основной отделки, чистовая отделка)",
    "some_stuff_fin_down_material": "Имя покрытие панелей (низа стен и колонн, чистовая отделка)",
    "some_stuff_fin_down_height": "Высота панелей",
    "some_stuff_fin_column_material": "Имя покрытия колонн (чистовая отделка)",
    "some_stuff_fin_ceil_material": "Имя покрытия потолка (чистовая отделка)",
    "some_stuff_fin_ceil_result": "Отделка потолка (запись результата)",
    "some_stuff_fin_main_result": "Отделка стен (запись результата)",
    "some_stuff_fin_up_result": "Отделка верха стен, от подвесного потолка до верха стены (запись результата)",
    "some_stuff_fin_down_result": "Отделка низа стен/колонн (запись результата)",
    "some_stuff_fin_reveal_result": "Отделка откосов (запись результата)",
    "some_stuff_fin_ceil_result_bytype": "Отделка потолка (запись результата с группировкой по типам отделки)",
    "some_stuff_fin_main_result_bytype": "Отделка стен (запись результата с группировкой по типам отделки)",
    "some_stuff_fin_up_result_bytype": "Отделка верха стен (запись результата с группировкой по типам отделки)",
    "some_stuff_fin_down_result_bytype": "Отделка низа стен/колонн (запись результата с группировкой по типам отделки)",
    "some_stuff_fin_reveal_result_bytype": "Отделка откосов (запись результата с группировкой по типам отделки)",
}


def build_property_description(marker):
    """Описание свойства: понятный текст + маркер в скобках.

    Маркер обязателен в описании (аддон ищет свойства по Contains(marker) в
    описании, RoombookData); понятный текст показывает назначение в Archicad.
    """
    text = MARKER_DESCRIPTIONS.get(marker)
    return "%s (%s)" % (text, marker) if text else marker


def extract_format_block(description):
    """Выделяет блок форматирования «{ ШИРИНА ; ШИРИНА ; РАЗМЕР ; ШРИФТ ... }»
    из существующего описания свойства результата (wiki/ru/Finishing-ru.md).
    Возвращает строку с блоком (с фигурными скобками) или пустую строку."""
    start = description.find("{")
    if start < 0:
        return ""
    end = description.find("}", start)
    if end < 0:
        return ""
    return description[start:end + 1]

# Комбинация слоёв ищется по суффиксу smstf_fin (Roombook.cpp:69-83).
LAYER_COMBINATION_SUFFIX = "smstf_fin"

# Классы отделки: (маркер = id = description, человекочитаемое имя).
# id и description обязаны содержать маркер: Class_FindFinClass (RoombookData.cpp)
# ищет some_stuff_fin_* в описании элемента классификации.
FINISH_CLASSES = [
    ("some_stuff_fin_class", "Отделка"),
    ("some_stuff_fin_walls", "Отделка стен"),
    ("some_stuff_fin_down_walls", "Отделка стен, нижняя часть"),
    ("some_stuff_fin_reveals", "Отделка откосов"),
    ("some_stuff_fin_windows_reveals", "Отделка откосов окон"),
    ("some_stuff_fin_doors_reveals", "Отделка откосов дверей"),
    ("some_stuff_fin_columns", "Отделка колонн"),
    ("some_stuff_fin_ceil_beams", "Отделка балок в потолке"),
    ("some_stuff_fin_wall_beams", "Отделка балок в стенах"),
    ("some_stuff_fin_floors", "Отделка пола"),
    ("some_stuff_fin_skirting", "Плинтус пола"),
    ("some_stuff_fin_cornice", "Карниз потолка"),
    ("some_stuff_fin_ceils", "Отделка потолка"),
]

# Маркеры ClassOtd (RoombookConstants.hpp); поиск идёт в описаниях классов.
REQUIRED_CLASSES = [
    "some_stuff_fin_class",
    "some_stuff_fin_walls",
    "some_stuff_fin_down_walls",
    "some_stuff_fin_reveals",
    "some_stuff_fin_windows_reveals",
    "some_stuff_fin_doors_reveals",
    "some_stuff_fin_columns",
    "some_stuff_fin_floors",
    "some_stuff_fin_ceil_beams",
    "some_stuff_fin_wall_beams",
    "some_stuff_fin_skirting",
    "some_stuff_fin_cornice",
    "some_stuff_fin_ceils",
]


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

    FavoritesList использует тот же словарь, что и Roombook.
    """
    data, err = addon_call(port, "FavoritesList")
    if data is not None and data.get("succeeded"):
        response = data.get("result", {}).get("addOnCommandResponse", {})
        favorites = response.get("favorites") if isinstance(response, dict) else None
        if isinstance(favorites, list) and all(
            isinstance(item, dict) and isinstance(item.get("name"), str) for item in favorites
        ):
            names = {item["name"].lower().strip() for item in favorites}
            rows = [{"name": name, "required": required, "found": True if name in names else None}
                    for name, required in REQUIRED_FAVORITES]
            complete = all(r["found"] is True for r in rows)
            return {"ok": True if complete else None, "rows": rows,
                    "detail": "FavoritesList не сообщает об ошибках чтения отдельных типов; ненайденные имена не подтверждены"}
    # При отсутствии команды показываем имена, не выдавая их за отсутствующие.
    return {
        "ok": None,
        "rows": [{"name": name, "required": required, "found": None} for name, required in REQUIRED_FAVORITES],
        "detail": "избранное не проверено: команда FavoritesList недоступна или вернула ошибку",
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
    incomplete = False
    for system in systems:
        guid = ((system.get("classificationSystemId") or {}).get("guid"))
        if not guid:
            incomplete = True
            continue
        system_names.append(system.get("name", "?"))
        items, _ = raw_post(port, "API.GetAllClassificationsInSystem", {
            "classificationSystemId": {"guid": guid}})
        if not (items and items.get("succeeded")):
            incomplete = True
            continue
        for root in items.get("result", {}).get("classificationItems", []) or []:
            _collect_classification_ids(root, found)
    rows = [{"cls": c, "found": True if c in found else (None if incomplete else False)} for c in REQUIRED_CLASSES]
    matched = sum(1 for r in rows if r["found"] is True)
    return {
        "ok": None if incomplete else matched == len(REQUIRED_CLASSES),
        "rows": rows,
        "detail": "системы: %s; маркеров в описаниях найдено %d из %d%s; наличие маркера не подтверждает его использование аддоном" % (
            ", ".join(system_names) or "—", matched, len(REQUIRED_CLASSES),
            "; часть систем не прочитана" if incomplete else ""),
    }


def _collect_classification_ids(node, found):
    """Ищет маркеры во всём описании, как Class_FindFinClass."""
    item = node.get("classificationItem") or node
    description = (item.get("description") or "").lower()
    for marker in REQUIRED_CLASSES:
        if marker in description:
            found.add(marker)
    for child in item.get("children", []) or []:
        _collect_classification_ids(child, found)


def check_layer_combination(port):
    """Проверяет наличие комбинации слоёв с суффиксом smstf_fin.

    Штатный JSON API AC25 не отдаёт комбинации слоёв; проверка требует аддонной команды.
    """
    return {"ok": None, "rows": [], "detail": "штатный JSON API AC25 не отдаёт комбинации слоёв; требуется аддонная команда (todo)"}


# ---------------------------------------------------------------------------
# Свойства проекта: список, описание, установка описания (команда аддона #304)
# ---------------------------------------------------------------------------

def fetch_user_properties(port):
    """UserDefined-свойства проекта для выпадающих списков.

    Возвращает ([(метка «Группа/Имя», localizedName)], None) или (None, ошибка).
    Список строится одним вызовом GetAllPropertyNames; GUID при применении
    резолвится отдельно (GetPropertyIds) — не замедляем загрузку списка.
    """
    data, err = raw_post(port, "API.GetAllPropertyNames", {})
    if data is None:
        return None, "нет ответа: %s" % err
    if not data.get("succeeded"):
        return None, "код %s" % data.get("error", {}).get("code")
    props = []
    for prop in data.get("result", {}).get("properties") or []:
        if prop.get("type") != "UserDefined":
            continue
        localizedName = prop.get("localizedName") or []
        if len(localizedName) >= 2:
            props.append(("%s / %s" % (localizedName[0], localizedName[1]), localizedName))
    return props, None


def get_property_guid_and_description(port, localizedName):
    """Резолвит GUID и текущее описание одного свойства.

    GetPropertyIds -> GetDetailsOfProperties. Возвращает (guid, description)
    или (None, None) при ошибке.
    """
    data, _ = raw_post(port, "API.GetPropertyIds", {
        "properties": [{"type": "UserDefined", "localizedName": localizedName}]})
    if data is None or not data.get("succeeded"):
        return None, None
    for p in data.get("result", {}).get("properties", []):
        guid = (p.get("propertyId") or {}).get("guid")
        if not guid:
            continue
        det, _ = raw_post(port, "API.GetDetailsOfProperties", {
            "properties": [{"propertyId": {"guid": guid}}]})
        description = ""
        if det and det.get("succeeded"):
            items = det.get("result", {}).get("propertyDefinitions", []) or []
            for item in items:
                defn = item.get("propertyDefinition") or item
                description = defn.get("description", "") or ""
                break
        return guid, description
    return None, None


def set_property_description(port, property_guid, description):
    """Ставит описание свойства командой аддона SetPropertyDescription (#304).

    Возвращает (True, None) или (False, текст_ошибки).
    """
    data, err = addon_call(port, "SetPropertyDescription", {
        "propertyGuid": property_guid, "description": description})
    if data is None:
        return False, "нет ответа: %s" % err
    if not data.get("succeeded"):
        error = data.get("error", {})
        return False, "код %s: %s" % (error.get("code"), error.get("message"))
    return True, None


# ---------------------------------------------------------------------------
# Создание классов отделки (режим --create-classes, только CLI)
# ---------------------------------------------------------------------------

def fetch_classification_systems(port):
    """Возвращает список систем классификации: [(guid, name)] или (None, ошибка)."""
    data, err = raw_post(port, "API.GetAllClassificationSystems", {})
    if data is None:
        return None, "нет ответа: %s" % err
    if not data.get("succeeded"):
        return None, "API.GetAllClassificationSystems: код %s" % data.get("error", {}).get("code")
    systems = []
    for system in data.get("result", {}).get("classificationSystems") or []:
        guid = (system.get("classificationSystemId") or {}).get("guid")
        if guid:
            systems.append((guid, system.get("name", "?")))
    return systems, None


def fetch_class_tree(port, system_guid):
    """Читает дерево элементов классификации системы в плоский список кортежей
    (guid, уровень, id, имя, описание) в порядке обхода в глубину."""
    data, err = raw_post(port, "API.GetAllClassificationsInSystem", {
        "classificationSystemId": {"guid": system_guid}})
    if data is None:
        return None, "нет ответа: %s" % err
    if not data.get("succeeded"):
        return None, "API.GetAllClassificationsInSystem: код %s" % data.get("error", {}).get("code")
    flat = []

    def walk(nodes, level):
        for node in nodes:
            item = node.get("classificationItem") or node
            flat.append(((item.get("classificationItemId") or {}).get("guid") or "",
                         level, item.get("id", ""), item.get("name", ""),
                         item.get("description", "")))
            walk(item.get("children", []) or [], level + 1)

    walk(data.get("result", {}).get("classificationItems") or [], 0)
    return flat, None


def collect_tree_markers(tree):
    """Маркеры some_stuff_fin_*, найденные в описаниях элементов дерева."""
    found = set()
    for _guid, _level, _iid, _name, description in tree:
        for marker in REQUIRED_CLASSES:
            if marker_matches((description or "").lower(), marker):
                found.add(marker)
    return found


def create_classification_item(port, system_guid, parent_guid, item_id, item_name, item_description):
    """Создаёт элемент классификации командой аддона CreateClassificationItem.

    Возвращает (True, guid_или_None, None) или (False, None, текст_ошибки).
    """
    parameters = {
        "systemGuid": system_guid,
        "itemId": item_id,
        "itemName": item_name,
        "itemDescription": item_description,
    }
    if parent_guid:
        parameters["parentItemGuid"] = parent_guid
    data, err = addon_call(port, "CreateClassificationItem", parameters)
    if data is None:
        return False, None, "нет ответа: %s" % err
    if not data.get("succeeded"):
        error = data.get("error", {})
        return False, None, "код %s: %s" % (error.get("code"), error.get("message"))
    guid = data.get("result", {}).get("addOnCommandResponse", {}).get("itemGuid")
    return True, guid, None


def run_create_classes(port):
    """Интерактивное создание классов отделки: выбор системы и родительской ветви,
    создание только отсутствующих классов из FINISH_CLASSES."""
    systems, err = fetch_classification_systems(port)
    if systems is None:
        print("Не удалось прочитать системы классификации: %s" % err)
        return 4
    if not systems:
        print("В проекте нет систем классификации — создайте систему вручную и повторите.")
        return 4
    print("\nСистемы классификации:")
    for i, (_guid, name) in enumerate(systems, 1):
        print("  %2d. %s" % (i, name))
    try:
        choice = int(input("Номер системы [1]: ").strip() or 1)
    except ValueError:
        print("Отменено: номер системы не введён.")
        return 4
    if not 1 <= choice <= len(systems):
        print("Отменено: нет такой системы.")
        return 4
    system_guid, system_name = systems[choice - 1]

    tree, err = fetch_class_tree(port, system_guid)
    if tree is None:
        print("Не удалось прочитать дерево классификации (%s): %s" % (system_name, err))
        return 4
    print("\nВетви системы «%s»:" % system_name)
    print("   0. — корень системы —")
    for i, (guid, level, iid, name, description) in enumerate(tree, 1):
        marker = " [some_stuff_fin_*]" if collect_tree_markers([(guid, 0, iid, name, description)]) else ""
        print("  %2d. %s%s%s" % (i, "  " * level, name or iid, marker))
    try:
        parent_choice = int(input("Номер родительской ветви [0]: ").strip() or 0)
    except ValueError:
        print("Отменено: номер ветви не введён.")
        return 4
    parent_guid = ""
    if parent_choice:
        if not 1 <= parent_choice <= len(tree):
            print("Отменено: нет такой ветви.")
            return 4
        parent_guid = tree[parent_choice - 1][0]

    existing = collect_tree_markers(tree)
    missing = [(marker, name) for marker, name in FINISH_CLASSES if marker not in existing]
    print("\nНайдено маркеров: %d из %d. Будет создано: %d." % (
        len(existing), len(REQUIRED_CLASSES), len(missing)))
    for marker, name in missing:
        print("  + %s (%s)" % (marker, name))
    if not missing:
        print("Все классы отделки уже есть — создавать нечего.")
        return 0
    answer = input("Создать отсутствующие классы? [y/N]: ").strip().lower()
    if answer not in ("y", "yes", "д", "да"):
        print("Отменено пользователем.")
        return 0

    created = 0
    failed = []
    for marker, name in missing:
        ok, guid, err = create_classification_item(port, system_guid, parent_guid, marker, name, marker)
        if ok:
            created += 1
            print("  создан %s -> %s" % (marker, guid))
        else:
            failed.append(marker)
            print("  ОШИБКА %s: %s" % (marker, err))
    print("\nСоздано %d из %d%s." % (created, len(missing), "; ошибки: " + ", ".join(failed) if failed else ""))

    # Повторная read-only проверка: подтверждает, что маркеры видны аддону.
    section = check_classifications(port)
    print("\n[%s] Классификационные классы после создания" % (
        "OK" if section["ok"] else ("?" if section["ok"] is None else "FAIL")))
    print("    %s" % section["detail"])
    return 0 if not failed else 4

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
            marker = row.get("marker")
            label = row.get("name") or (build_property_description(marker) if marker else None) or row.get("cls")
            found = row.get("found")
            req = row.get("required")
            mark = "?" if found is None else ("OK " if found else "MISS")
            extra = (" (обяз.)" if req else " (опц.)") if "required" in row else ""
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

APP_VERSION = "v2.0"
SITE_URL = "https://kuvbur.org/go/archicad"

# ---------------------------------------------------------------------------
# GUI (tkinter) — дизайн по образцу somestuff_updater.py; логика проверок — функции выше
# ---------------------------------------------------------------------------

# Палитра: светлая и тёмная темы (автовыбор определяется в App по системе Windows)
THEMES = {
    "light": {
        "bg": "#f4f5f8", "card": "#ffffff", "border": "#e0e3ea",
        "text": "#1f2430", "muted": "#6b7385",
        "accent": "#2563eb", "accent_dark": "#1d4ed8",
        "hover": "#eef1f6", "trough": "#e3e7ef",
        "err_fg": "#b91c1c", "ok_fg": "#15803d",
    },
    "dark": {
        "bg": "#1e1f25", "card": "#2a2b33", "border": "#3a3c46",
        "text": "#e2e4ec", "muted": "#9aa1b2",
        "accent": "#3b82f6", "accent_dark": "#2563eb",
        "hover": "#35373f", "trough": "#3a3c46",
        "err_fg": "#f87171", "ok_fg": "#4ade80",
    },
}
C_BG = THEMES["light"]["bg"]
C_CARD = THEMES["light"]["card"]
C_BORDER = THEMES["light"]["border"]
C_TEXT = THEMES["light"]["text"]
C_MUTED = THEMES["light"]["muted"]
C_ACCENT = THEMES["light"]["accent"]
C_ACCENT_DARK = THEMES["light"]["accent_dark"]
# вид плашки статуса: (фон, текст) — тёмные варианты генерируются при переключении темы
BADGES = {
    "idle": ("#eceff4", "#6b7385"),
    "info": ("#dbeafe", "#1d4ed8"),
    "ok": ("#dcfce7", "#15803d"),
    "warn": ("#fef3c7", "#b45309"),
    "err": ("#fee2e2", "#b91c1c"),
}
BADGES_DARK = {
    "idle": ("#2f313a", "#9aa1b2"),
    "info": ("#1e3a8a", "#93c5fd"),
    "ok": ("#14532d", "#4ade80"),
    "warn": ("#78350f", "#fbbf24"),
    "err": ("#7f1d1d", "#f87171"),
}
FONT = "Segoe UI"


def _windows_prefers_dark():
    """Тема системы Windows: 1 = тёмная (реестр AppsUseLightTheme)."""
    try:
        import winreg
        key = winreg.OpenKey(winreg.HKEY_CURRENT_USER,
                             r"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize")
        value, _ = winreg.QueryValueEx(key, "AppsUseLightTheme")
        winreg.CloseKey(key)
        return value == 0
    except Exception:
        return False

STRINGS = {
    "i18n": {
        "ru": {
            "window_title": "Проверка отделки (Roombook) " + APP_VERSION,
            "subtitle": "Проверка готовности проекта Archicad к ведомости отделки",
            "label_language": "ЯЗЫК",
            "label_theme": "ТЕМА",
            "theme_light": "Светлая",
            "theme_dark": "Тёмная",
            "label_instance": "ARCHICAD",
            "checkbutton_auto": "Проверить все найденные экземпляры",
            "btn_refresh": "Обновить порты",
            "btn_run": "Запустить проверку",
            "cap_check": "ПРОВЕРКА",
            "tab_properties": "СВОЙСТВА",
            "btn_load_props": "Загрузить свойства проекта",
            "btn_apply_props": "Применить выбранные",
            "checkbutton_overwrite": "Перезаписывать существующее описание",
            "props_hint": "Для каждого маркера выберите свойство — его описание получит понятный текст с маркером.",
            "status_ready": "Готов к работе",
            "status_scanning": "Поиск Archicad...",
            "status_running": "Проверка выполняется...",
            "status_loading_props": "Загрузка свойств...",
            "status_props_loaded": "Свойства загружены: %d (UserDefined).",
            "status_props_done": "Описания применены: %d; пропущено: %d; ошибки: %d.",
            "status_no_archicad": "Archicad не найден. Запустите Archicad и откройте проект.",
            "status_done_ok": "Проверка завершена: всё в порядке.",
            "status_done_fail": "Проверка завершена: есть проблемы.",
            "status_done_incomplete": "Проверка завершена не полностью: часть данных не проверена.",
            "status_error": "Ошибка: ",
            "props_already": "уже есть описание",
            "props_set": "описание установлено",
            "props_no_selection": "свойство не выбрано",
            "props_no_guid": "GUID не найден",
            "details": "Подробности",
        },
        "en": {
            "window_title": "Roombook setup check " + APP_VERSION,
            "subtitle": "Check the Archicad project readiness for the Roombook finish",
            "label_language": "LANGUAGE",
            "label_theme": "THEME",
            "theme_light": "Light",
            "theme_dark": "Dark",
            "label_instance": "ARCHICAD",
            "checkbutton_auto": "Check all detected instances",
            "btn_refresh": "Refresh ports",
            "btn_run": "Run check",
            "cap_check": "CHECK",
            "tab_properties": "PROPERTIES",
            "btn_load_props": "Load project properties",
            "btn_apply_props": "Apply selected",
            "checkbutton_overwrite": "Overwrite existing description",
            "props_hint": "For each marker pick a property — its description gets a readable text plus the marker.",
            "status_ready": "Ready",
            "status_scanning": "Scanning for Archicad...",
            "status_running": "Check in progress...",
            "status_loading_props": "Loading properties...",
            "status_props_loaded": "Properties loaded: %d (UserDefined).",
            "status_props_done": "Descriptions applied: %d; skipped: %d; errors: %d.",
            "status_no_archicad": "Archicad not found. Start Archicad and open the project.",
            "status_done_ok": "Check finished: all good.",
            "status_done_fail": "Check finished: problems found.",
            "status_done_incomplete": "Check incomplete: some data could not be checked.",
            "status_error": "Error: ",
            "props_already": "description exists",
            "props_set": "description set",
            "props_no_selection": "no property selected",
            "props_no_guid": "GUID not found",
            "details": "Details",
        },
    },
}


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.lang = "ru"
        s = STRINGS["i18n"][self.lang]
        self.title(s["window_title"])
        # Автовыбор темы: по теме системы Windows (AppsUseLightTheme); тёмная по умолчанию
        self.theme = "dark" if _windows_prefers_dark() else "light"
        self._badge_current = ("idle", "")
        self._card_outer = []        # внешние рамки карточек — перекрашиваются при смене темы
        self.configure(bg=THEMES[self.theme]["bg"])
        self.minsize(820, 1)
        self.instances = []          # список экземпляров: {port, name, addon_loaded, version}
        self._port_by_label = {}     # метка «имя — порт» -> порт
        self.q = queue.Queue()       # события из рабочих потоков → GUI
        self._busy = False
        self._worker_error = None
        self._setup_style()
        self._build()
        self._apply_language()
        self._pump()
        self.after(200, self.refresh_ports)   # первый поиск портов сразу при запуске

    # ------------------------------------------------------------ стиль
    def _setup_style(self):
        s = ttk.Style(self)
        s.theme_use("clam")
        # Палитра берётся из выбранной темы; модульные C_* перенастраиваются,
        # чтобы виджеты, созданные позже (_build), окрасились той же темой.
        pal = THEMES[self.theme]
        global C_BG, C_CARD, C_BORDER, C_TEXT, C_MUTED, C_ACCENT, C_ACCENT_DARK
        C_BG = pal["bg"]
        C_CARD = pal["card"]
        C_BORDER = pal["border"]
        C_TEXT = pal["text"]
        C_MUTED = pal["muted"]
        C_ACCENT = pal["accent"]
        C_ACCENT_DARK = pal["accent_dark"]
        s.configure(".", background=C_BG, foreground=C_TEXT, font=(FONT, 10))
        s.configure("TFrame", background=C_BG)
        s.configure("TLabel", background=C_BG)
        s.configure("Card.TFrame", background=C_CARD)
        s.configure("Card.TLabel", background=C_CARD)
        s.configure("Muted.TLabel", foreground=C_MUTED)
        s.configure("Title.TLabel", font=(FONT, 17, "bold"))
        s.configure("Cap.TLabel", font=(FONT, 8, "bold"), foreground=C_MUTED, background=C_CARD)
        s.configure("Small.TLabel", font=(FONT, 9), foreground=C_MUTED)
        s.configure("SmallCard.TLabel", font=(FONT, 9), foreground=C_MUTED, background=C_CARD)

        s.configure("TCombobox", fieldbackground=C_CARD, background=C_CARD,
                    bordercolor=C_BORDER, lightcolor=C_CARD, darkcolor=C_CARD,
                    arrowcolor=C_MUTED, padding=6)
        s.map("TCombobox", fieldbackground=[("readonly", C_CARD), ("disabled", C_BG)],
              bordercolor=[("focus", C_ACCENT)], foreground=[("disabled", C_MUTED)])
        self.option_add("*TCombobox*Listbox.font", (FONT, 10))
        self.option_add("*TCombobox*Listbox.selectBackground", C_ACCENT)

        s.configure("Accent.TButton", background=C_ACCENT, foreground="white",
                    borderwidth=0, focusthickness=0, padding=(22, 9), font=(FONT, 10, "bold"))
        s.map("Accent.TButton",
              background=[("disabled", pal["hover"]), ("pressed", C_ACCENT_DARK), ("active", C_ACCENT_DARK)],
              foreground=[("disabled", "white")])
        s.configure("Ghost.TButton", background=C_CARD, foreground=C_TEXT,
                    bordercolor=C_BORDER, lightcolor=C_CARD, darkcolor=C_CARD,
                    focusthickness=0, padding=(18, 8))
        s.map("Ghost.TButton", background=[("active", pal["hover"])],
              foreground=[("disabled", C_MUTED)])

        s.configure("Horizontal.TProgressbar", troughcolor=pal["trough"], background=C_ACCENT,
                    bordercolor=pal["trough"], borderwidth=0, lightcolor=C_ACCENT, darkcolor=C_ACCENT, thickness=6)

    def _card(self, parent):
        outer = tk.Frame(parent, bg=C_BORDER)
        self._card_outer.append(outer)   # для перекраски при смене темы
        inner = ttk.Frame(outer, style="Card.TFrame", padding=(18, 14))
        inner.pack(fill="both", expand=True, padx=1, pady=1)
        return outer, inner

    # ---------------------------------------------------------- вёрстка
    def _build(self):
        main = ttk.Frame(self, padding=(22, 18, 22, 14))
        main.pack(fill="both", expand=True)

        # --- заголовок (справа — ссылка на сайт, как в somestuff_updater) ---
        header = ttk.Frame(main)
        header.pack(fill="x")
        self.lbl_title = ttk.Label(header, text="", style="Title.TLabel")
        self.lbl_title.pack(side="left")
        self.lbl_site = tk.Label(header, text="Сайт ↗", font=(FONT, 10),
                                 fg=C_ACCENT, bg=C_BG, cursor="hand2")
        self.lbl_site.pack(side="right")
        self.lbl_site.bind("<Button-1>", lambda _e: webbrowser.open(SITE_URL))
        self.lbl_site.bind("<Enter>", lambda _e: self.lbl_site.config(fg=C_ACCENT_DARK))
        self.lbl_site.bind("<Leave>", lambda _e: self.lbl_site.config(fg=C_ACCENT))
        self.lbl_subtitle = ttk.Label(main, text="", style="Muted.TLabel")
        self.lbl_subtitle.pack(anchor="w", pady=(0, 12))

        # --- карточка: экземпляр Archicad ---
        outer, card = self._card(main)
        outer.pack(fill="x")
        card.columnconfigure(1, weight=1)
        self.lbl_port = ttk.Label(card, text="", style="Cap.TLabel")
        self.lbl_port.grid(row=0, column=0, sticky="w", padx=(0, 18))
        self.cmb_port = ttk.Combobox(card, values=[], state="readonly", width=40)
        self.cmb_port.grid(row=0, column=1, sticky="ew")
        self.lbl_lang = ttk.Label(card, text="", style="Cap.TLabel")
        self.lbl_lang.grid(row=1, column=0, sticky="w", padx=(0, 18), pady=(10, 0))
        self.cmb_lang = ttk.Combobox(card, values=["ru", "en"], state="readonly", width=6)
        self.cmb_lang.current(0)
        self.cmb_lang.grid(row=1, column=1, sticky="w", pady=(10, 0))
        self.cmb_lang.bind("<<ComboboxSelected>>", lambda _e: self._apply_language())
        # Переключатель темы (тёмная/светлая); начальное значение — автовыбор из __init__
        self.lbl_theme = ttk.Label(card, text="", style="Cap.TLabel")
        self.lbl_theme.grid(row=1, column=2, sticky="e", padx=(18, 6))
        self.cmb_theme = ttk.Combobox(card, values=[], state="readonly", width=9)
        self.cmb_theme.grid(row=1, column=3, sticky="e", pady=(10, 0))
        self.cmb_theme.bind("<<ComboboxSelected>>", lambda _e: self._apply_theme())
        self.var_auto = tk.BooleanVar(value=False)
        self.chk_auto = ttk.Checkbutton(card, text="", variable=self.var_auto)
        self.chk_auto.grid(row=2, column=1, sticky="w", pady=(10, 0))

        # --- карточка: отчёт проверки (скрыта до нажатия «Проверка») ---
        outer2, card2 = self._card(main)
        self.report_outer = outer2
        card2.columnconfigure(0, weight=1)
        self.cap_check = ttk.Label(card2, text="", style="Cap.TLabel")
        self.cap_check.grid(row=0, column=0, sticky="w")
        self.badge = tk.Label(card2, text="", font=(FONT, 9, "bold"), padx=10, pady=4,
                              anchor="w", justify="left")
        self.badge.grid(row=0, column=0, sticky="e")
        self._set_badge("idle", "")

        self.progress = ttk.Progressbar(card2, mode="indeterminate")
        self.progress.grid(row=1, column=0, sticky="ew", pady=(10, 0))

        self.txt = tk.Text(card2, height=14, wrap="word", relief="flat",
                           font=("Consolas", 9), bg=C_CARD, fg=C_TEXT, padx=10, pady=8,
                           state="disabled", borderwidth=0, highlightthickness=0)
        sb = ttk.Scrollbar(card2, command=self.txt.yview)
        self.txt.configure(yscrollcommand=sb.set)
        sb.grid(row=2, column=1, sticky="ns")
        self.txt.grid(row=2, column=0, sticky="nsew", pady=(10, 0))
        card2.rowconfigure(2, weight=1)
        pal = THEMES[self.theme]
        self.txt.tag_configure("err", foreground=pal["err_fg"])
        self.txt.tag_configure("ok", foreground=pal["ok_fg"])
        self.txt.tag_configure("dim", foreground=C_MUTED)

        # --- карточка: свойства (описания маркеров) ---
        outer3, card3 = self._card(main)
        outer3.pack(fill="x", pady=(12, 0))
        card3.columnconfigure(1, weight=1)
        self.cap_props = ttk.Label(card3, text="", style="Cap.TLabel")
        self.cap_props.grid(row=0, column=0, columnspan=2, sticky="w")
        self.lbl_props_hint = ttk.Label(card3, text="", style="SmallCard.TLabel")
        self.lbl_props_hint.grid(row=1, column=0, columnspan=2, sticky="w", pady=(4, 8))

        self.var_overwrite = tk.BooleanVar(value=False)
        self.chk_overwrite = ttk.Checkbutton(card3, text="", variable=self.var_overwrite)
        self.chk_overwrite.grid(row=2, column=0, columnspan=2, sticky="w")

        # Прокручиваемый список строк «маркер — свойство — статус».
        canvas = tk.Canvas(card3, highlightthickness=0, bg=C_CARD, height=180)
        scroll = ttk.Scrollbar(card3, orient="vertical", command=canvas.yview)
        self.props_inner = ttk.Frame(canvas, style="Card.TFrame")
        self.props_inner.bind(
            "<Configure>",
            lambda _e: canvas.configure(scrollregion=canvas.bbox("all")))
        canvas.create_window((0, 0), window=self.props_inner, anchor="nw")
        canvas.configure(yscrollcommand=scroll.set)
        canvas.grid(row=3, column=0, columnspan=2, sticky="ew")
        scroll.grid(row=3, column=2, sticky="ns")
        self.props_canvas = canvas

        self.btn_load_props = ttk.Button(card3, text="", style="Ghost.TButton", command=self.load_properties)
        self.btn_load_props.grid(row=4, column=0, sticky="w", pady=(10, 0))
        self.btn_apply_props = ttk.Button(card3, text="", style="Accent.TButton", command=self.apply_descriptions)
        self.btn_apply_props.grid(row=4, column=1, sticky="e", pady=(10, 0))

        self._property_labels = []   # метки «Группа / Имя» для поиска
        self._property_by_label = {} # метка -> localizedName (list)
        self._prop_rows = []         # [{marker, combo, status}]
        for marker in REQUIRED_PROPERTY_MARKERS:
            row = ttk.Frame(self.props_inner, style="Card.TFrame")
            row.pack(fill="x", pady=2)
            # Человекочитаемое имя вместо технического маркера
            # (some_stuff_fin_has_ceil → «Наличие потолка»); маркер — мелкой строкой.
            human = MARKER_DESCRIPTIONS.get(marker, marker)
            ttk.Label(row, text=human, width=36, style="Card.TLabel").pack(side="left")
            ttk.Label(row, text=marker, width=28, style="SmallCard.TLabel").pack(side="left", padx=(0, 6))
            combo = ttk.Combobox(row, values=[], width=48)
            combo.pack(side="left", padx=6)
            status = tk.Label(row, text="", font=(FONT, 8), padx=8, pady=2, anchor="w")
            status.pack(side="left")
            combo.bind("<KeyRelease>", lambda event, r=row: self._on_property_search(r, event))
            self._prop_rows.append({"marker": marker, "combo": combo, "status": status})

        # --- строка статуса ---
        self.status = tk.StringVar(value="")
        lbl_status = ttk.Label(main, textvariable=self.status, style="Small.TLabel", anchor="w")
        lbl_status.pack(fill="x", pady=(10, 0))

        # --- кнопки ---
        btns = ttk.Frame(main)
        btns.pack(fill="x", pady=(12, 0))
        self.btn_run = ttk.Button(btns, text="", style="Accent.TButton", command=self.run_check)
        self.btn_run.pack(side="right")
        self.btn_refresh = ttk.Button(btns, text="", style="Ghost.TButton", command=self.refresh_ports)
        self.btn_refresh.pack(side="right", padx=(0, 8))

    # ------------------------------------------------- утилиты очереди
    def _pump(self):
        try:
            while True:
                self.q.get_nowait()()
        except queue.Empty:
            pass
        except Exception as e:  # ошибка в обработчике не должна убить очередь
            self.status.set(STRINGS["i18n"][self.lang]["status_error"] + str(e))
        self.after(50, self._pump)

    def post(self, fn, *args):
        """Вызов функции в GUI-потоке из рабочего потока."""
        self.q.put(lambda: fn(*args))

    def _set_badge(self, kind, text):
        self._badge_current = (kind, text)
        bg, fg = self._badge_colors(kind)
        self.badge.configure(text=text, bg=bg, fg=fg)

    def _badge_colors(self, kind):
        return (BADGES_DARK if self.theme == "dark" else BADGES)[kind]

    def _set_busy(self, busy):
        self._busy = busy
        state = "disabled" if busy else "normal"
        self.btn_run.config(state=state)
        self.btn_refresh.config(state=state)
        self.chk_auto.config(state=state)
        self.btn_load_props.config(state=state)
        self.btn_apply_props.config(state=state)
        self.chk_overwrite.config(state=state)
        self.cmb_port.config(state="disabled" if busy else "readonly")
        self.cmb_lang.config(state="disabled" if busy else "readonly")
        if busy:
            self.progress.start(14)
        else:
            self.progress.stop()

    def _show_worker_error(self):
        self.status.set(STRINGS["i18n"][self.lang]["status_error"] + self._worker_error)
        self._set_badge("err", self._worker_error)
        self._set_busy(False)

    # ------------------------------------------------- вкладка «Свойства»
    def _selected_port(self):
        return self._port_by_label.get(self.cmb_port.get())

    def load_properties(self):
        if self._busy:
            return
        port = self._selected_port()
        s = STRINGS["i18n"][self.lang]
        if port is None:
            self.status.set(s["status_no_archicad"])
            return
        self._set_busy(True)
        self._worker_error = None
        self.status.set(s["status_loading_props"])
        self._set_badge("idle", s["status_loading_props"])
        threading.Thread(target=self._props_worker, args=(port,), daemon=True).start()

    def _props_worker(self, port):
        try:
            result = fetch_user_properties(port)
            self.post(self._props_done, result)
        except Exception as exc:
            self._worker_error = str(exc)
            self.post(self._show_worker_error)

    def _props_done(self, result):
        s = STRINGS["i18n"][self.lang]
        props, err = result or (None, None)
        if props is None:
            self.status.set(s["status_error"] + (err or "?"))
            self._set_busy(False)
            return
        self._property_labels = [label for label, _loc in props]
        self._property_by_label = dict(props)
        for row in self._prop_rows:
            row["combo"]["values"] = self._property_labels
        self.status.set(s["status_props_loaded"] % len(props))
        self._set_badge("ok", s["status_props_loaded"] % len(props))
        self._set_busy(False)

    @staticmethod
    def _filter_properties(labels, typed):
        typed = typed.strip().lower()
        if not typed:
            return labels
        return [label for label in labels if label.lower().startswith(typed)] or [
            label for label in labels if typed in label.lower()]

    def _on_property_search(self, row, event):
        # Поиск по первым буквам: фильтруем значения и держим список раскрытым.
        if event.keysym in ("Up", "Down", "Return", "Escape", "Tab"):
            return
        combo = row["combo"]
        matches = self._filter_properties(self._property_labels, combo.get())
        combo["values"] = matches
        if matches:
            try:
                combo.tk.eval("ttk::combobox::Post %s" % combo)
            except tk.TclError:
                pass

    def apply_descriptions(self):
        """Ставит выбранным свойствам описание «Понятный текст (маркер)»."""
        if self._busy:
            return
        port = self._selected_port()
        s = STRINGS["i18n"][self.lang]
        if port is None:
            self.status.set(s["status_no_archicad"])
            return
        tasks = []
        overwrite = self.var_overwrite.get()
        for row in self._prop_rows:
            label = row["combo"].get().strip()
            if not label:
                bg, fg = self._badge_colors("idle")
                row["status"].config(text=s["props_no_selection"], bg=bg, fg=fg)
                continue
            localizedName = self._property_by_label.get(label)
            if localizedName is None:
                bg, fg = self._badge_colors("warn")
                row["status"].config(text=s["props_no_guid"], bg=bg, fg=fg)
                continue
            tasks.append((row, localizedName))
        if not tasks:
            return
        self._set_busy(True)
        self._worker_error = None
        self.status.set(s["status_loading_props"])
        self._set_badge("idle", s["status_loading_props"])
        threading.Thread(target=self._apply_worker, args=(port, tasks, overwrite), daemon=True).start()

    def _apply_worker(self, port, tasks, overwrite):
        try:
            results = []
            for row, localizedName in tasks:
                guid, description = get_property_guid_and_description(port, localizedName)
                if guid is None:
                    results.append((row["marker"], "failed", "GUID не найден"))
                    continue
                # Существующее чужое описание без явного разрешения не перезаписываем.
                expected = build_property_description(row["marker"])
                block = extract_format_block(description or "")
                if block:
                    # Блок форматирования результата { ... ; шрифт ... } сохраняем.
                    expected = expected + "\n" + block
                if not overwrite and description and description != expected:
                    results.append((row["marker"], "skipped", description))
                    continue
                ok, err = set_property_description(port, guid, expected)
                results.append((row["marker"], "applied" if ok else "failed", err))
            self.post(self._apply_done, results)
        except Exception as exc:
            self._worker_error = str(exc)
            self.post(self._show_worker_error)

    def _apply_done(self, results):
        s = STRINGS["i18n"][self.lang]
        applied = skipped = failed = 0
        status_by_marker = {}
        for marker, state, info in results or []:
            if state == "applied":
                applied += 1
                status_by_marker[marker] = (s["props_set"], "ok")
            elif state == "skipped":
                skipped += 1
                status_by_marker[marker] = ("%s: %s" % (s["props_already"], info), "info")
            else:
                failed += 1
                status_by_marker[marker] = ("ошибка: %s" % info, "err")
        for row in self._prop_rows:
            text, kind = status_by_marker.get(row["marker"], ("", "idle"))
            bg, fg = self._badge_colors(kind)
            row["status"].config(text=text, bg=bg, fg=fg)
        self.status.set(s["status_props_done"] % (applied, skipped, failed))
        badge_kind = "ok" if failed == 0 else "err"
        self._set_badge(badge_kind, s["status_props_done"] % (applied, skipped, failed))
        self._set_busy(False)

    # ----------------------------------------------------------- действия
    def refresh_ports(self):
        if self._busy:
            return
        self._set_busy(True)
        self._worker_error = None
        s = STRINGS["i18n"][self.lang]
        self.status.set(s["status_scanning"])
        self._set_badge("idle", s["status_scanning"])
        threading.Thread(target=self._scan_worker, daemon=True).start()

    def _scan_worker(self):
        try:
            result = discover_instances(None)
            self.post(self._ports_found, result)
        except Exception as exc:
            self._worker_error = str(exc)
            self.post(self._show_worker_error)

    def _ports_found(self, instances):
        self.instances = instances
        self._fill_port_list()
        s = STRINGS["i18n"][self.lang]
        if instances:
            names = ", ".join(i["name"] for i in instances)
            self.status.set("Archicad: " + names)
            self._set_badge("info", "Archicad: " + names)
        else:
            self.status.set(s["status_no_archicad"])
            self._set_badge("warn", s["status_no_archicad"])
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
        # Развернуть секцию проверки внизу окна (скрыта до первого запуска)
        self.report_outer.pack(fill="both", expand=True, pady=(12, 0))
        self.status.set(s["status_running"])
        self._set_badge("idle", s["status_running"])
        self.txt.configure(state="normal")
        self.txt.delete("1.0", "end")
        self.txt.configure(state="disabled")
        threading.Thread(target=self._check_worker, args=(port,), daemon=True).start()

    def _check_worker(self, port):
        try:
            ports = [port] if port else discover_ports(None)
            if not ports:
                self.post(self._check_done, "", False, False)
                return
            text_parts = []
            any_fail = False
            any_unknown = False
            for p in ports:
                sections = collect_report(p)
                self.post(self._append_report, self._format_report(p, sections))
                if any(sec["ok"] is False for sec in sections.values()):
                    any_fail = True
                if any(sec["ok"] is None for sec in sections.values()):
                    any_unknown = True
            self.post(self._check_done, None, any_fail, any_unknown)
        except Exception as exc:
            self._worker_error = str(exc)
            self.post(self._show_worker_error)

    def _append_report(self, text):
        self.txt.configure(state="normal")
        self.txt.insert("end", text)
        self.txt.see("end")
        self.txt.configure(state="disabled")

    def _check_done(self, _text, any_fail=False, any_unknown=False):
        s = STRINGS["i18n"][self.lang]
        key = ("status_done_fail" if any_fail else
               ("status_done_incomplete" if any_unknown else "status_done_ok"))
        self.status.set(s[key])
        self._set_badge("err" if any_fail else ("warn" if any_unknown else "ok"), s[key])
        self._set_busy(False)

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
                marker = row.get("marker")
                label = row.get("name") or (build_property_description(marker) if marker else None) or row.get("cls")
                found = row.get("found")
                req = row.get("required")
                mark = "?" if found is None else ("OK " if found else "MISS")
                extra = (" (обяз.)" if req else " (опц.)") if "required" in row else ""
                lines.append("    %-6s %s%s" % (mark, label, extra))
        lines.append("")
        return "\n".join(lines) + "\n"

    def _apply_language(self):
        self.lang = self.cmb_lang.get() or "ru"
        s = STRINGS["i18n"][self.lang]
        self.title(s["window_title"])
        self.lbl_title.config(text=s["window_title"])
        self.lbl_subtitle.config(text=s["subtitle"])
        self.lbl_lang.config(text=s["label_language"])
        self.lbl_theme.config(text=s["label_theme"])
        self.cmb_theme.config(values=[s["theme_light"], s["theme_dark"]])
        self.cmb_theme.current(1 if self.theme == "dark" else 0)
        self.lbl_port.config(text=s["label_instance"])
        self.chk_auto.config(text=s["checkbutton_auto"])
        self.btn_refresh.config(text=s["btn_refresh"])
        self.btn_run.config(text=s["btn_run"])
        self.cap_check.config(text=s["cap_check"])
        self.cap_props.config(text=s["tab_properties"])
        self.lbl_props_hint.config(text=s["props_hint"])
        self.btn_load_props.config(text=s["btn_load_props"])
        self.chk_overwrite.config(text=s["checkbutton_overwrite"])
        self.btn_apply_props.config(text=s["btn_apply_props"])
        if self.status.get() in ("", "Ready", "Готов к работе"):
            self.status.set(s["status_ready"])

    def _apply_theme(self):
        """Смена темы: пересобрать стиль и перекрасить «сырые» (tk) виджеты."""
        self.theme = "dark" if self.cmb_theme.current() == 1 else "light"
        self._setup_style()
        pal = THEMES[self.theme]
        self.configure(bg=pal["bg"])
        self.lbl_site.config(bg=pal["bg"])
        for outer in self._card_outer:
            outer.config(bg=pal["border"])
        self.txt.config(bg=pal["card"], fg=pal["text"])
        self.txt.tag_configure("err", foreground=pal["err_fg"])
        self.txt.tag_configure("ok", foreground=pal["ok_fg"])
        self.txt.tag_configure("dim", foreground=pal["muted"])
        self.props_canvas.config(bg=pal["card"])
        kind, text = self._badge_current
        self._set_badge(kind, text)


def main():
    parser = argparse.ArgumentParser(description="Проверка готовности проекта к функции отделки (Roombook).")
    parser.add_argument("--port", type=int, default=None,
                        help="консольный режим: проверить только указанный порт")
    parser.add_argument("--cli", action="store_true",
                        help="консольный режим (по умолчанию открывается GUI)")
    parser.add_argument("--create-classes", action="store_true",
                        help="интерактивное создание классов отделки (выбор системы, родителя и создание отсутствующих some_stuff_fin_*); только CLI")
    args = parser.parse_args()

    if args.create_classes:
        ports = discover_ports(args.port)
        if not ports:
            print("Archicad не найден на портах %d..%d. Запустите Archicad и откройте проект." % (
                PORT_RANGE.start, PORT_RANGE.stop - 1))
            return 1
        if len(ports) > 1:
            print("Найдено несколько экземпляров: %s. Укажите один ключом --port." % ", ".join(map(str, ports)))
            return 1
        return run_create_classes(ports[0])

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
