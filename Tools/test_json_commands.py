#!/usr/bin/env python3
"""
Тестирование ВСЕХ JSON-команд SomeStuff Add-On через ArchiCAD JSON API.

Использование:
    python test_json_commands.py [--port PORT] [--test TEST_NAME]

Интеграция с restart_archicad_for_test.ps1:
    Добавить в основной цикл после проверки test_results.txt:
    python D:\SomeStuff_addon\Tools\test_json_commands.py
    if ($LASTEXITCODE -ne 0) { $jsonTestsFailed = $true }
"""

import argparse
import json
import sys
import time
import urllib.request
import urllib.error

# ==============================================================================
# КОНФИГУРАЦИЯ
# ==============================================================================

DEFAULT_PORT_RANGE = range(19723, 19744)
TIMEOUT = 30
COMMAND_NAMESPACE = "SomeStuffCommand"

# ==============================================================================
# УТИЛИТЫ
# ==============================================================================

def find_archicad_port(timeout=1.0):
    """
    Автоопределение порта ArchiCAD.
    Сканирует порты 19723-19743, проверяет ответ JSON API.
    Возвращает порт или None.
    """
    for port in DEFAULT_PORT_RANGE:
        try:
            req = urllib.request.Request(
                f"http://127.0.0.1:{port}",
                data=json.dumps({"command": "API.GetProjectInfo", "parameters": {}}).encode("utf-8"),
                headers={"Content-Type": "application/json"},
                method="POST"
            )
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                if resp.status == 200:
                    # Проверяем, что ответ действительно от ArchiCAD
                    body = resp.read().decode("utf-8")
                    response = json.loads(body)
                    if "succeeded" in response or "result" in response:
                        return port
        except Exception:
            continue
    return None


def wait_for_archicad_port(max_attempts=30, delay=1.0):
    """
    Ожидание появления порта ArchiCAD (для случая, когда ArchiCAD ещё не запустился).
    Возвращает порт или None.
    """
    for attempt in range(max_attempts):
        port = find_archicad_port()
        if port:
            return port
        time.sleep(delay)
    return None


def call_addon_command(port, command_name, parameters=None):
    """Вызов JSON-команды аддона."""
    if parameters is None:
        parameters = {}

    payload = {
        "command": "API.ExecuteAddOnCommand",
        "parameters": {
            "addOnCommandId": {
                "commandNamespace": COMMAND_NAMESPACE,
                "commandName": command_name
            },
            "addOnCommandParameters": parameters
        }
    }

    url = f"http://127.0.0.1:{port}"
    data = json.dumps(payload).encode("utf-8")

    req = urllib.request.Request(
        url,
        data=data,
        headers={"Content-Type": "application/json"},
        method="POST"
    )

    try:
        with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
            return json.loads(resp.read().decode("utf-8"))
    except urllib.error.HTTPError as e:
        return {"error": f"HTTP {e.code}", "details": e.read().decode("utf-8")}
    except Exception as e:
        return {"error": str(e)}


def extract_response(api_response):
    """Извлечение ответа аддона из обёртки ArchiCAD."""
    return api_response.get("result", {}).get("addOnCommandResponse", {})


def print_test_header(test_name):
    print(f"\n{'='*60}")
    print(f"ТЕСТ: {test_name}")
    print(f"{'='*60}")


def print_result(success, message=""):
    status = "✓ PASS" if success else "✗ FAIL"
    color = "\033[92m" if success else "\033[91m"
    reset = "\033[0m"
    print(f"{color}{status}{reset} {message}")
    return success


# ==============================================================================
# ТЕСТЫ ВСЕХ КОМАНД
# ==============================================================================

def test_01_health(port):
    """
    Команда: Health
    Назначение: Проверка доступности аддона.
    Ожидаемый ответ: {"status": "ok", "version": "...", "addon": "SomeStuff"}
    """
    print_test_header("HealthCommand — проверка доступности")
    
    response = call_addon_command(port, "Health")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if addon_resp.get("status") == "ok":
        version = addon_resp.get("version", "unknown")
        return print_result(True, f"Аддон доступен, версия: {version}")
    else:
        return print_result(False, f"Неверный ответ: {addon_resp}")


def test_02_get_property_definitions(port):
    """
    Команда: GetPropertyDefinitions
    Назначение: Получение списка всех свойств проекта.
    Ожидаемый ответ: {"items": [...], "count": N}
    """
    print_test_header("GetPropertyDefinitionsCommand — список свойств")
    
    response = call_addon_command(port, "GetPropertyDefinitions")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    items = addon_resp.get("items", [])
    count = len(items)
    
    if count > 0:
        print(f"  Получено свойств: {count}")
        for i, item in enumerate(items[:3]):
            raw = item.get("rawName", "N/A")
            name = item.get("name", "N/A")
            print(f"    [{i+1}] {raw} → {name}")
        return print_result(True)
    else:
        return print_result(False, "Пустой список свойств")


def test_03_get_settings(port):
    """
    Команда: GetSettings
    Назначение: Чтение настроек синхронизации.
    Ожидаемый ответ: {"syncAll": bool, "syncMon": bool, ...}
    """
    print_test_header("GetSettingsCommand — чтение настроек")
    
    response = call_addon_command(port, "GetSettings")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    required_fields = ["syncAll", "syncMon", "wallS", "widoS", "objS", 
                       "cwallS", "logMon", "showpalette"]
    
    missing = [f for f in required_fields if f not in addon_resp]
    
    if not missing:
        settings_str = ", ".join([f"{f}={addon_resp[f]}" for f in required_fields[:3]])
        return print_result(True, f"Настройки: {settings_str}...")
    else:
        return print_result(False, f"Отсутствуют поля: {missing}")


def test_04_set_settings(port):
    """
    Команда: SetSettings
    Назначение: Запись настроек синхронизации (с откатом).
    Проверка: запись → чтение → сравнение → возврат.
    """
    print_test_header("SetSettingsCommand — запись настроек")
    
    # Читаем текущие настройки
    resp_before = call_addon_command(port, "GetSettings")
    if "error" in resp_before:
        return print_result(False, "Не удалось прочитать текущие настройки")
    
    settings_before = extract_response(resp_before)
    orig_sync_mon = settings_before.get("syncMon", False)
    new_sync_mon = not orig_sync_mon
    
    # Записываем новое значение
    resp_set = call_addon_command(port, "SetSettings", {"syncMon": new_sync_mon})
    if "error" in resp_set:
        return print_result(False, f"Ошибка записи: {resp_set['error']}")
    
    set_result = extract_response(resp_set)
    if set_result.get("status") != "ok":
        return print_result(False, f"SetSettings вернул: {set_result}")
    
    # Читаем снова и проверяем
    resp_after = call_addon_command(port, "GetSettings")
    settings_after = extract_response(resp_after)
    
    if settings_after.get("syncMon") == new_sync_mon:
        # Возвращаем исходное значение
        call_addon_command(port, "SetSettings", {"syncMon": orig_sync_mon})
        return print_result(True, f"syncMon изменён: {orig_sync_mon} → {new_sync_mon} → восстановлено")
    else:
        return print_result(False, "Значение не изменилось после записи")


def test_05_sync_all(port):
    """
    Команда: SyncAll
    Назначение: Запуск полной синхронизации всех элементов.
    Ожидаемый ответ: {"status": "ok"}
    """
    print_test_header("SyncAllCommand — полная синхронизация")
    
    response = call_addon_command(port, "SyncAll")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if addon_resp.get("status") == "ok":
        return print_result(True, "Синхронизация запущена")
    else:
        return print_result(False, f"Ответ: {addon_resp}")


def test_06_mon_all(port):
    """
    Команда: MonAll
    Назначение: Запуск мониторинга всех элементов.
    Ожидаемый ответ: {"status": "ok"}
    """
    print_test_header("MonAllCommand — мониторинг всех элементов")
    
    response = call_addon_command(port, "MonAll")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if addon_resp.get("status") == "ok":
        return print_result(True, "Мониторинг запущен")
    else:
        return print_result(False, f"Ответ: {addon_resp}")


def test_08_get_selection_info(port):
    """
    Команда: GetSelectionInfo
    Назначение: Получение количества выделенных элементов.
    Ожидаемый ответ: {"count": N}
    """
    print_test_header("GetSelectionInfoCommand — информация о выделении")
    
    response = call_addon_command(port, "GetSelectionInfo")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if "count" in addon_resp:
        count = addon_resp["count"]
        if isinstance(count, int) and count >= 0:
            return print_result(True, f"Выделено элементов: {count}")
        else:
            return print_result(False, f"Некорректное значение count: {count}")
    else:
        return print_result(False, f"Отсутствует поле count в ответе: {addon_resp}")


def test_09_get_properties_list(port):
    """
    Команда: GetPropertiesList
    Назначение: Получение значений свойств для выделенных элементов.
    Ожидаемый ответ: {"elements": [...], "count": N}
    """
    print_test_header("GetPropertiesListCommand — значения свойств выделенных элементов")
    
    response = call_addon_command(port, "GetPropertiesList")
    
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if "elements" in addon_resp:
        elements = addon_resp["elements"]
        count = len(elements)
        if count > 0:
            print(f"  Элементов: {count}")
            for i, elem in enumerate(elements[:3]):
                guid = elem.get("guid", "N/A")[:20]
                props = elem.get("properties", [])
                print(f"    [{i+1}] {guid}... — свойств: {len(props)}")
                for prop in props[:2]:
                    name = prop.get("name", "N/A")
                    val = prop.get("value", "N/A")
                    vtype = prop.get("valueType", "N/A")
                    print(f"        {name} = {val} ({vtype})")
            return print_result(True, f"Получены свойства для {count} элементов")
        else:
            return print_result(True, "Нет выделенных элементов (OK — пустой ответ)")
    else:
        return print_result(False, f"Отсутствует поле elements в ответе: {addon_resp}")


def test_10_get_property_value(port):
    """
    Команда: GetPropertyValue
    Назначение: Получение значения конкретного свойства для выделенных элементов.
    Ожидаемый ответ: {"propertyName": "...", "common": bool, "values": [{"value": "...", "count": N}]}
    """
    print_test_header("GetPropertyValueCommand — значение свойства для выделенных элементов")
    
    # Сначала получаем список свойств, чтобы знать какие есть
    response = call_addon_command(port, "GetPropertiesList")
    if "error" in response:
        return print_result(False, f"Ошибка GetPropertiesList: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if "elements" not in addon_resp or not addon_resp["elements"]:
        return print_result(True, "Нет выделенных элементов (OK — пустой ответ)")
    
    # Берем первый propertyGuid из первого элемента
    first_elem = addon_resp["elements"][0]
    props = first_elem.get("properties", [])
    if not props:
        return print_result(True, "У элемента нет свойств (OK — пустой ответ)")
    
    property_id = props[0].get("propertyGuid", "")
    if not property_id:
        return print_result(False, "У свойства нет GUID")
    
    print(f"  Тестируем propertyId: {property_id}")
    
    # Вызываем GetPropertyValue
    response = call_addon_command(port, "GetPropertyValue", {"propertyId": property_id})
    if "error" in response:
        return print_result(False, f"Ошибка вызова: {response['error']}")
    
    addon_resp = extract_response(response)
    
    if addon_resp.get("status") == "ok":
        property_name = addon_resp.get("propertyName", "")
        common = addon_resp.get("common", False)
        values = addon_resp.get("values", [])
        print(f"  propertyName: {property_name}")
        print(f"  common: {common}")
        print(f"  values count: {len(values)}")
        for v in values[:3]:
            print(f"    value: {v.get('value')}, count: {v.get('count')}")
        return print_result(True, "Команда выполнена успешно")
    else:
        return print_result(False, f"Ответ: {addon_resp}")


def test_07_parse_property_for_element(port):
    """
    Команда: ParsePropertyForElement
    Назначение: Парсинг описания свойства для конкретного элемента.
    Проверка: вызов с описанием и GUID элемента.
    Ожидаемый ответ: {"status": "ok", "syncRules": [...], ...}
    """
    print_test_header("ParsePropertyForElementCommand — парсинг описания")

    # Получаем какой-нибудь элемент для теста
    try:
        payload = {
            "command": "API.GetAllElements",
            "parameters": {}
        }
        url = f"http://127.0.0.1:{port}"
        data = json.dumps(payload).encode("utf-8")
        req = urllib.request.Request(url, data=data, 
                                     headers={"Content-Type": "application/json"},
                                     method="POST")
        with urllib.request.urlopen(req, timeout=TIMEOUT) as resp:
            all_elements_resp = json.loads(resp.read().decode("utf-8"))
        
        elements = all_elements_resp.get("result", {}).get("elements", [])
        
        if not elements:
            return print_result(False, "В проекте нет элементов для теста")
        
        elem_guid = elements[0].get("elementId", {}).get("guid", "")
        
        if not elem_guid:
            return print_result(False, "Не удалось получить GUID элемента")
        
        print(f"  Тестируем на элементе: {elem_guid[:20]}...")
        print(f"  Описание: Sync_from{{Property:TestProperty}}")
        
        # Вызываем нашу команду с description и elemGuid
        response = call_addon_command(port, "ParsePropertyForElement", 
                                      {"description": "Sync_from{Property:TestProperty}",
                                       "elemGuid": elem_guid})
        
        if "error" in response:
            return print_result(False, f"Ошибка вызова: {response['error']}")
        
        addon_resp = extract_response(response)
        
        if addon_resp.get("status") == "ok":
            sync_rules = addon_resp.get("syncRules", [])
            has_rules = addon_resp.get("hasSyncRules", False)
            print(f"  hasSyncRules: {has_rules}")
            print(f"  Найдено правил: {len(sync_rules)}")
            for rule in sync_rules[:3]:
                ct = rule.get("commandType", "?")
                st = rule.get("sourceType", "?")
                sn = rule.get("sourceName", "?")
                print(f"    [{ct}] {st} -> {sn}")
            return print_result(True, "Команда выполнена успешно")
        else:
            return print_result(False, f"Ответ: {addon_resp}")
    
    except Exception as e:
        return print_result(False, f"Исключение: {e}")


# ==============================================================================
# ОСНОВНОЙ ЦИКЛ
# ==============================================================================

def run_all_tests(port, test_filter=None):
    """Запуск всех тестов."""
    print("=" * 60)
    print("ТЕСТИРОВАНИЕ JSON-КОМАНД SOMESTUFF ADD-ON")
    print(f"Порт ArchiCAD: {port}")
    print("=" * 60)
    
    # Список всех тестов (в порядке регистрации команд)
    all_tests = [
        ("01_Health", test_01_health),
        ("02_GetPropertyDefinitions", test_02_get_property_definitions),
        ("03_GetSettings", test_03_get_settings),
        ("04_SetSettings", test_04_set_settings),
        ("05_SyncAll", test_05_sync_all),
        ("06_MonAll", test_06_mon_all),
        ("07_GetSelectionInfo", test_08_get_selection_info),
        ("08_GetPropertiesList", test_09_get_properties_list),
        ("09_GetPropertyValue", test_10_get_property_value),
        ("10_ParsePropertyForElement", test_07_parse_property_for_element),
    ]
    
    # Фильтрация
    if test_filter:
        all_tests = [(name, func) for name, func in all_tests 
                     if test_filter.lower() in name.lower() or 
                        test_filter.lower() in func.__name__.lower()]
    
    if not all_tests:
        print(f"Тесты не найдены по фильтру: {test_filter}")
        return False
    
    # Запуск
    results = []
    for test_name, test_func in all_tests:
        try:
            success = test_func(port)
            results.append((test_name, success))
        except Exception as e:
            print(f"\n✗ FAIL — Исключение: {e}")
            results.append((test_name, False))
    
    # Итоги
    print("\n" + "=" * 60)
    print("ИТОГИ ТЕСТИРОВАНИЯ")
    print("=" * 60)
    
    passed = sum(1 for _, success in results if success)
    total = len(results)
    
    for test_name, success in results:
        status = "PASS ✓" if success else "FAIL ✗"
        print(f"  {test_name:35s} {status}")
    
    print("=" * 60)
    print(f"Результат: {passed}/{total} тестов пройдено")
    
    if passed == total:
        print("ВСЕ ТЕСТЫ УСПЕШНЫ!")
        return True
    else:
        print("ЕСТЬ НЕУДАЧНЫЕ ТЕСТЫ.")
        return False


# ==============================================================================
# ТОЧКА ВХОДА
# ==============================================================================

def main():
    parser = argparse.ArgumentParser(
        description="Тестирование JSON-команд SomeStuff Add-On",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Примеры:
  python test_json_commands.py                    # все тесты (автопоиск порта)
  python test_json_commands.py --wait             # ждать запуска ArchiCAD до 30 сек
  python test_json_commands.py --test Health      # только тест Health
        """
    )
    parser.add_argument("--port", type=int, 
                        help="Порт ArchiCAD (по умолчанию: автоопределение)")
    parser.add_argument("--test", type=str, 
                        help="Имя теста для запуска (часть имени)")
    parser.add_argument("--wait", action="store_true",
                        help="Ждать запуска ArchiCAD (до 30 сек)")
    
    args = parser.parse_args()
    
    # Определяем порт
    if args.port:
        port = args.port
        print(f"Используется порт: {port}")
    else:
        print("Автоопределение порта ArchiCAD...")
        if args.wait:
            print("Ожидание запуска ArchiCAD (до 30 сек)...")
            port = wait_for_archicad_port()
        else:
            port = find_archicad_port()
        
        if not port:
            print("ОШИБКА: ArchiCAD не найден.")
            print("Убедитесь, что ArchiCAD запущен и аддон загружен.")
            print("Используйте --wait для ожидания запуска.")
            sys.exit(1)
        
        print(f"Найден ArchiCAD на порту: {port}")
    
    # Запускаем тесты
    success = run_all_tests(port, args.test)
    
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
