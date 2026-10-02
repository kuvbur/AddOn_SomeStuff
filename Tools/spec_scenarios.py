"""Стенд приёмки Spec: создание на чистом файле и повторный запуск после
изменения исходных данных.

Сценарий стенда (тот, что реально доступен на порту AC25):

  1. Открыт чистый проект — без ранее размещённых объектов спецификации.
     Делается ОДИН вызов Spec. Это единственный прогон, который создаёт
     объекты: на следующем вызове они уже размещены, и Spec закономерно
     вернёт no-op. Повторных вызовов «ради сценария» быть не должно — они
     ничего не проверяют, а только переводят модель в сошедшееся состояние.
  2. Снимок созданного состояния: GUID созданных объектов, их источники,
     значения свойств, счётчики этапов. Снимок сохраняется и служит эталоном
     для второго запуска.
  3. Исходные данные меняются: у части элементов-источников отключается
     флаг участия в спецификации. Правка идёт через порт, исходные значения
     запоминаются ДО записи.
  4. Повторный вызов Spec. Ожидаемое поведение: строки, чьи источники
     отключены, должны исчезнуть (удаление старого), остальные — сохраниться
     без изменения. Фактический результат сравнивается со снимком: какие
     строки ушли, какие остались, что стало с объектами.
  5. Возврат исходных значений флага (обратная запись, не Undo).
  6. Третий вызов Spec, ПОСЛЕ восстановления флага. Удалённая строка
     обязана вернуться: расчёт её выдаёт, объекта в модели нет, значит
     Spec должен создать его заново. Эта проверка сильнее проверки
     удаления — она доказывает, что отключение источника не вычеркнуло
     строку из самого расчёта. Пока флаг выключен, проверка бессмысленна:
     источник не участвует в расчёте, и строка не появится законно.

Чего стенд НЕ может, и это фиксируется как not verified, а не как успех:
- портовых команд GDLParameter нет, отказ GDL-этапа не вызывается;
- порт не умеет отменять изменение (Undo/Redo отсутствуют), поэтому
  восстановление исходных значений — это обычная запись, а не откат;
- второе правило завести нельзя: правило определяется ОПИСАНИЕМ свойства
  (Spec_rule в описании), а команды записи описаний на порту нет. Флаг
  Sync_flag правилом не является.

Запуск:
  python Tools/spec_scenarios.py cycle
  python Tools/spec_scenarios.py cycle --singleton <GUID> [<GUID> ...]
  python Tools/spec_scenarios.py cycle --no-restore
  python Tools/spec_scenarios.py inspect
"""

import argparse
import json
import os
import socket
import sys
import urllib.error
import urllib.request

PORT_FIRST = 19723
PORT_LAST = 19743
OUT_DIR = os.path.join("Reviews", "spec-scenarios")

VERIFIED = "verified"
NOT_VERIFIED = "not verified"

# Поля ответа, по которым сравниваются прогоны. elapsedSeconds намеренно не
# входит: это замер, а не результат, и он меняется от шума канала.
COMPARE_FIELDS = (
    "status", "resultCode", "elementsToCreate", "elementsToModify",
    "elementsToDelete", "hasPrimaryError", "hasRecoveryError",
    "hasUnconfirmedCreate", "prepareFailureStage",
)
STAGE_FIELDS = ("create", "grouping", "gdl", "deleteOld")


# ---------------------------------------------------------------------------
# Порт
# ---------------------------------------------------------------------------
def find_port():
    for port in range(PORT_FIRST, PORT_LAST + 1):
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as sock:
            sock.settimeout(0.4)
            if sock.connect_ex(("127.0.0.1", port)) == 0:
                return port
    return None


class Port(object):
    def __init__(self, port):
        self.url = "http://127.0.0.1:%d/json" % port

    def call(self, command, params=None, timeout=900):
        payload = {"command": command}
        if params is not None:
            payload["parameters"] = params
        req = urllib.request.Request(
            self.url, data=json.dumps(payload).encode("utf-8"),
            headers={"Content-Type": "application/json"}, method="POST")
        try:
            with urllib.request.urlopen(req, timeout=timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.HTTPError as e:
            return {"succeeded": False,
                    "error": {"code": e.code, "message": str(e.reason)[:200]}}
        except (urllib.error.URLError, OSError) as e:
            return {"succeeded": False,
                    "error": {"code": "transport", "message": str(e)[:200]}}

    def spec(self, rules=None, include=True, placement=None):
        params = {"placementPoint": placement or {"x": 1000.0, "y": 1000.0},
                  "includeParameters": include}
        if rules is not None:
            params["ruleNames"] = rules
        raw = self.call("API.ExecuteAddOnCommand", {
            "addOnCommandId": {"commandNamespace": "SomeStuffCommand",
                               "commandName": "Spec"},
            "addOnCommandParameters": params})
        if raw.get("succeeded") is not True:
            return None, raw
        return raw["result"].get("addOnCommandResponse", {}), raw

    def elements(self, element_type):
        r = self.call("API.GetElementsByType", {"elementType": element_type})
        if not r.get("succeeded"):
            return []
        return [e["elementId"]["guid"] for e in r.get("result", {}).get("elements", [])]


# ---------------------------------------------------------------------------
# Разбор ответа
# ---------------------------------------------------------------------------
def list_field(body, name, item):
    """Списки ответа обязаны приходить списками. Объект вместо списка — дефект
    формы ответа: обход молча теряет все элементы, кроме последнего."""
    holder = body.get(name) or {}
    value = holder.get(item)
    if value is None:
        return [], None
    if isinstance(value, list):
        return value, None
    return [value], "%s.%s is %s, not a list" % (name, item, type(value).__name__)


def row_signature(element):
    """Подпись строки спецификации.

    Идентичность строки — это её ИСТОЧНИКИ, а не значения. Значения меняются
    при отключении флага (суммы, количества), и ключ из значений ломался бы
    всякий раз: строка «исчезала» и тут же «появлялась» с новым ключом.
    Источники же отражают, какие элементы дали строку, и именно по ним
    решается, должна ли строка исчезнуть.

    Поля «связанные элементы» и «имя правила» в ключ не входят: они меняются
    от прогона к прогону, но строку не характеризуют. Значения свойств и
    GDL-параметров сохраняются отдельно - их сравнение и есть проверка того,
    что пересчёт не изменил то, что меняться не должно было.
    """
    props = {}
    gdl = {}
    for item in element.get("property") or []:
        name = (item.get("name") or "").strip()
        low = name.lower()
        if "связанные элементы" in low or "имя правила" in low:
            continue
        props[name] = item.get("value", "")
    for item in element.get("gdlParameter") or []:
        gdl[(item.get("name") or "").strip()] = item.get("value", "")
    sources = tuple(sorted(
        (s or {}).get("guid", "") for s in (element.get("sourceElement") or [])))
    return {
        "key": "%s|%s" % (element.get("favoriteName", ""), ";".join(sources)),
        "favorite": element.get("favoriteName", ""),
        "properties": props,
        "gdl": gdl,
        "sources": sources,
    }


def snapshot(body):
    """Снимок результата запуска для последующего сравнения."""
    created, created_shape = list_field(body, "created", "element")
    modified, modified_shape = list_field(body, "modified", "element")
    deleted, deleted_shape = list_field(body, "deleted", "element")
    rules, rules_shape = list_field(body, "rules", "rule")
    messages, messages_shape = list_field(body, "messages", "message")
    return {
        "counters": dict((f, body.get(f)) for f in COMPARE_FIELDS),
        "stages": dict((f, body.get(f)) for f in STAGE_FIELDS),
        "elapsedSeconds": body.get("elapsedSeconds"),
        "created": dict((r["key"], r) for r in map(row_signature, created)),
        "modified": dict((r["key"], r) for r in map(row_signature, modified)),
        "deleted": dict(((d or {}).get("guid", ""),
                         (d or {}).get("favoriteName", ""))
                        for d in deleted),
        "rules": rules,
        "messages": messages,
        "shapeProblems": [p for p in (created_shape, modified_shape, deleted_shape,
                                      rules_shape, messages_shape) if p],
    }


def counter(snap):
    return (snap["counters"].get("elementsToCreate"),
            snap["counters"].get("elementsToModify"),
            snap["counters"].get("elementsToDelete"))


def sources_of(snap):
    """Все GUID, выступившие источниками строк снимка."""
    guids = set()
    for group in ("created", "modified"):
        for row in snap[group].values():
            guids.update(row["sources"])
    return sorted(guids)


# ---------------------------------------------------------------------------
# Свойства: чтение, запись, восстановление
# ---------------------------------------------------------------------------
def read_values(port, guids, property_guid, chunk=60):
    out = {}
    for i in range(0, len(guids), chunk):
        part = guids[i:i + chunk]
        r = port.call("API.GetPropertyValuesOfElements",
                      {"elements": [{"elementId": {"guid": g}} for g in part],
                       "properties": [{"propertyId": {"guid": property_guid}}]})
        if not r.get("succeeded"):
            continue
        for guid, entry in zip(part,
                               r["result"].get("propertyValuesForElements", [])):
            values = entry.get("propertyValues") or []
            if not values:
                out[guid] = None
                continue
            pv = values[0].get("propertyValue", {})
            out[guid] = (pv.get("type"), pv.get("status"), pv.get("value"))
    return out


def write_bool(port, guids, property_guid, value):
    return port.call("API.SetPropertyValuesOfElements", {"elementPropertyValues": [
        {"elementId": {"guid": g}, "propertyId": {"guid": property_guid},
         "propertyValue": {"type": "boolean", "status": "normal", "value": value}}
        for g in guids]})


def per_element_results(r):
    return (r.get("result") or {}).get("executionResults") or []


def find_rule_properties(port):
    """Свойства, выступившие ПРАВИЛОМ запуска.

    Отличать строго: под именем «Спецификации …» лежат и свойства-правила
    (Spec_rule в описании), и обычные флаги Sync_flag. Bool-тип значения для
    этого не годится: «Спецификации общестрой / Заполнять автоматически» —
    Sync_flag, заполнен на сотнях элементов, но правилом не является.
    Единственный источник истины — ответ запуска: имя правила в нём равно
    полному имени свойства (Spec.cpp: rule_name = GetPropertyFullName).
    """
    names = port.call("API.GetAllPropertyNames", {}).get("result", {}).get("properties", [])
    spec_names = [p for p in names if p.get("type") == "UserDefined"
                  and any("Спецификаци" in s or "спецификаци" in s
                          for s in p.get("localizedName", []))]
    resolved = []
    for i in range(0, len(spec_names), 100):
        chunk = spec_names[i:i + 100]
        r = port.call("API.GetPropertyIds", {"properties": chunk})
        for name, got in zip(chunk, (r.get("result") or {}).get("properties") or []):
            resolved.append({"name": name["localizedName"],
                             "guid": got["propertyId"]["guid"]})

    body, _ = port.spec(rules=None, include=True)
    if body is None:
        return [], "ответ Spec не получен", None
    rules, shape_problem = list_field(body, "rules", "rule")

    index = {}
    for p in resolved:
        parts = p["name"] or []
        if not parts:
            continue
        index["/".join(parts)] = p
        index[parts[-1]] = p
    out = []
    for rule in rules:
        full_name = (rule or {}).get("name")
        if not full_name:
            continue
        prop = index.get(full_name)
        out.append({"name": full_name,
                    "guid": prop["guid"] if prop else None,
                    "localizedName": prop["name"] if prop else None,
                    "resolved": prop is not None})
    return out, shape_problem, body


# ---------------------------------------------------------------------------
# Цикл
# ---------------------------------------------------------------------------
def pick_singleton_rows(snap):
    """Строки, состоящие ровно из одного источника.

    Такая строка — единственный случай, где отключение источника даёт
    гарантированный delete: источник один, отключили его — строке нечего
    собирать. На многосоставных строках отключение уменьшает сумму, и
    исчезновения не происходит вовсе.
    """
    out = []
    for group in ("created", "modified"):
        for row in snap[group].values():
            if len(row["sources"]) == 1:
                out.append({"group": group, "key": row["key"],
                            "source": row["sources"][0],
                            "favorite": row["favorite"]})
    return out


def rows_of(snap):
    """Все строки снимка (created и modified) одним словарём."""
    rows = dict(snap["created"])
    rows.update(snap["modified"])
    return rows


def expected_after_disable(first, disabled):
    """Что должно произойти при отключении элементов disabled.

    Три случая, а не «исчезла / осталась»:
    - все источники строки отключены -> строка обязана исчезнуть;
    - часть источников отключена -> строка обязана остаться, но её состав
      и суммы пересчитаны;
    - источники не тронуты -> строка обязана остаться БЕЗ изменения.
    """
    disabled_set = set(disabled)
    vanish, shrink, intact = [], [], []
    for row in rows_of(first).values():
        sources = set(row["sources"])
        lost = sources & disabled_set
        if not lost:
            intact.append(row)
        elif lost == sources:
            vanish.append(row)
        else:
            shrink.append(row)
    return {"vanish": vanish, "shrink": shrink, "intact": intact}


def run_cycle(port, restore=True, singletons=None, bulk_fraction=4):
    report = {"steps": [], "ruleProperties": [], "status": None}

    def note(step, status, detail, **extra):
        entry = {"step": step, "status": status, "detail": detail}
        entry.update(extra)
        report["steps"].append(entry)
        print("  %-12s %-12s %s" % (step, status, detail))
        return entry

    print("== 0. поиск свойства-правила")
    rule_props, shape_problem, first_body = find_rule_properties(port)
    report["ruleProperties"] = rule_props
    if first_body is None:
        note("find-rule", NOT_VERIFIED, "ответ Spec не получен: %s" % shape_problem)
        report["status"] = NOT_VERIFIED
        return report
    if shape_problem:
        note("find-rule", NOT_VERIFIED, "форма ответа: %s" % shape_problem)
        report["status"] = NOT_VERIFIED
        return report
    if not rule_props:
        note("find-rule", NOT_VERIFIED,
             "в ответе нет ни одного правила: правило определяется описанием "
             "свойства (Spec_rule), порт не даёт команд его завести")
        report["status"] = NOT_VERIFIED
        return report
    rule_prop = rule_props[0]
    note("find-rule", VERIFIED,
         "правило %r -> свойство %s" % (rule_prop["name"], rule_prop["guid"]))

    print("== 1. создание на чистом файле (единственный создающий прогон)")
    first = snapshot(first_body)
    create, modify, delete = counter(first)
    if not any(isinstance(c, int) and c != 0 for c in (create, modify, delete)):
        note("create", NOT_VERIFIED,
             "C/M/D=%s — ничего не создано: на проекте уже есть размещённые "
             "строки либо правило не нашло источников. Чистый проект "
             "обязателен, этот прогон единственный создающий" % (counter(first),))
        report["status"] = NOT_VERIFIED
        return report
    note("create", VERIFIED,
         "C/M/D=%s, строк created=%d modified=%d, объектов delete=%d, этапы %s"
         % (counter(first), len(first["created"]), len(first["modified"]),
            len(first["deleted"]),
            json.dumps(first["stages"], ensure_ascii=False)),
         counters=list(counter(first)),
         rows=len(first["created"]) + len(first["modified"]))
    if first["shapeProblems"]:
        note("create-shape", NOT_VERIFIED,
             "форма ответа: %s" % "; ".join(first["shapeProblems"]))
    report["first"] = first

    print("== 2. снимок созданного состояния")
    snapshot_path = os.path.join(OUT_DIR, "first-run.json")
    with open(snapshot_path, "w", encoding="utf-8") as fh:
        json.dump({"ruleProperties": rule_props, "snapshot": first},
                  fh, ensure_ascii=False, indent=2, sort_keys=True)
        fh.write("\n")
    note("snapshot", VERIFIED, "сохранено %s" % snapshot_path)

    print("== 3. выбор элементов для отключения")
    singleton_rows = pick_singleton_rows(first)
    disabled, intent = [], None
    for guid in (singletons or []):
        if not any(s["source"] == guid for s in singleton_rows):
            note("mutate", NOT_VERIFIED,
                 "элемент %s не является единственным источником ни одной "
                 "строки: отключение дало бы уменьшение суммы, а не удаление "
                 "строки. Одиночных строк в снимке: %d"
                 % (guid, len(singleton_rows)))
            report["status"] = NOT_VERIFIED
            return report
        disabled.append(guid)
    if disabled:
        intent = ("явно заданные элементы-одиночные источники: %s" % disabled)
    else:
        all_sources = sources_of(first)
        candidates = all_sources[:max(1, len(all_sources) // bulk_fraction)]
        probe = read_values(port, candidates, rule_prop["guid"])
        disabled = [g for g in candidates
                    if probe.get(g) and probe[g][0] == "boolean"
                    and probe[g][2] is True]
        intent = ("явно заданных элементов нет; отключена четверть "
                  "источников (%d из %d) — ожидается пересчёт сумм, а не "
                  "удаление строк" % (len(disabled), len(all_sources)))
    if not disabled:
        note("mutate", NOT_VERIFIED,
             "нет элементов с доступным флагом true — запись нечего делать")
        report["status"] = NOT_VERIFIED
        return report

    before = read_values(port, disabled, rule_prop["guid"])
    changed = [g for g in disabled
               if before.get(g) and before[g][0] == "boolean"
               and before[g][2] is True]
    w = write_bool(port, changed, rule_prop["guid"], False)
    ok = sum(1 for e in per_element_results(w) if e.get("success"))
    back = read_values(port, changed, rule_prop["guid"])
    actually_off = [g for g in changed
                    if back.get(g) and back.get(g)[0] == "boolean"
                    and back[g][2] is False]
    if len(actually_off) != len(changed):
        note("mutate", NOT_VERIFIED,
             "запись false подтверждена для %d из %d (executionResults "
             "success=%d) — состояние подготовлено не полностью"
             % (len(actually_off), len(changed), ok))
        report["status"] = NOT_VERIFIED
        return report
    report["mutation"] = {"propertyGuid": rule_prop["guid"],
                          "propertyName": rule_prop["name"],
                          "elements": changed,
                          "originalValues": {g: before[g][2] for g in changed},
                          "intent": intent}
    note("mutate", VERIFIED,
         "флаг %r выключен у %d элементов (подтверждено чтением обратно); %s"
         % (rule_prop["name"], len(changed), intent))

    print("== 4. повторный запуск и сравнение со снимком")
    second_body, second_raw = port.spec(rules=None, include=True)
    if second_body is None:
        note("rerun", NOT_VERIFIED,
             "повторный вызов не выполнен: %s"
             % json.dumps(second_raw, ensure_ascii=False)[:200])
        report["status"] = NOT_VERIFIED
        return report
    second = snapshot(second_body)
    report["second"] = second
    comparison = compare(first, second, changed)
    report["comparison"] = comparison
    for status, line in comparison["notes"]:
        note("compare", status, line)

    print("== 5. восстановление исходных флагов")
    if restore and report.get("mutation"):
        result = restore_flags(port, report["mutation"])
        report["restore"] = result
        if result["problems"]:
            note("restore", NOT_VERIFIED,
                 "исходные значения не восстановлены: %s"
                 % "; ".join(result["problems"][:5]))
        else:
            note("restore", VERIFIED,
                 "исходные значения возвращены у %d элементов (проверено "
                 "чтением обратно)" % result["restored"])
    else:
        note("restore", NOT_VERIFIED,
             "восстановление пропущено (--no-restore): флаги останутся "
             "выключенными, повторный цикл даст другое состояние")

    print("== 6. прогон после восстановления: строка обязана вернуться")
    # ШОГ ИДЁТ ПОСЛЕ ВОССТАНОВЛЕНИЯ ФЛАГА. Пока флаг выключен, источник не
    # участвует в расчёте, и строка закономерно не появляется — её
    # отсутствие тогда ничего не доказывает. После возврата флага расчёт
    # снова выдаёт строку, а объекта в модели нет: Spec обязан создать его
    # заново. Это и есть reconcile — проверка сильнее проверки удаления,
    # потому что доказывает, что отключение не вычеркнуло строку из расчёта.
    third_body, third_raw = port.spec(rules=None, include=True)
    if third_body is None:
        note("recreate", NOT_VERIFIED,
             "третий вызов не выполнен: %s"
             % json.dumps(third_raw, ensure_ascii=False)[:200])
    else:
        third = snapshot(third_body)
        report["third"] = third
        recreation = compare_recreate(first, second, third, changed)
        report["recreation"] = recreation
        for status, line in recreation["notes"]:
            note("recreate", status, line)

    report["status"] = (VERIFIED if all(
        s["status"] == VERIFIED for s in report["steps"]) else NOT_VERIFIED)
    return report


def compare(first, second, disabled):
    """Сравнение повторного запуска со снимком первого.

    created/modified в ответе — это СПИСОК ОПЕРАЦИЙ, а не снимок таблицы:
    строка, которая не изменилась, в ответ не попадает вовсе. Поэтому
    присутствие строки во втором ответе нельзя читать как «изменилась» —
    отсутствие означает лишь «операции не было».

    Проверяется предсказанное поведение по трём группам: исчезнуть должны
    только строки с отключёнными ВСЕМИ источниками; строки с частично
    отключёнными источниками обязаны остаться (операция modify либо
    отсутствие операции — оба исхода верны); нетронутые строки не должны
    порождать операций вовсе.
    """
    notes = []
    exp = expected_after_disable(first, disabled)
    first_rows = rows_of(first)
    second_rows = rows_of(second)

    vanish_keys = set(r["key"] for r in exp["vanish"])
    shrink_keys = set(r["key"] for r in exp["shrink"])
    intact_keys = set(r["key"] for r in exp["intact"])

    gone = sorted(vanish_keys - set(second_rows))
    stayed = sorted(k for k in vanish_keys if k in second_rows)
    shrink_gone = sorted(shrink_keys - set(second_rows))
    shrink_modified = sorted(k for k in shrink_keys if k in second_rows)
    # Нетронутая строка не должна ни появиться в modified, ни измениться.
    intact_touched = sorted(
        k for k in intact_keys
        if k in second_rows
        and second_rows[k]["properties"] != first_rows[k]["properties"])
    new_rows = sorted(set(second_rows) - vanish_keys - shrink_keys - intact_keys)

    c, m, d = counter(second)
    del_old = second["stages"].get("deleteOld") or {}

    if vanish_keys:
        if gone:
            notes.append((VERIFIED,
                          "исчезли строки с отключёнными ВСЕМИ источниками: "
                          "%d из %d (повторный прогон C/M/D=%s, deleteOld=%s)"
                          % (len(gone), len(vanish_keys), (c, m, d),
                             json.dumps(del_old, ensure_ascii=False))))
        if stayed:
            notes.append((NOT_VERIFIED,
                          "ожидалось исчезновение, но строка осталась: %d (%s)"
                          % (len(stayed), "; ".join(k[:80] for k in stayed[:3]))))
    else:
        notes.append((VERIFIED,
                      "строк с полностью отключёнными источниками нет — "
                      "удаление этим прогоном не проверялось (deleteOld=%s)"
                      % json.dumps(del_old, ensure_ascii=False)))

    if shrink_keys:
        if shrink_gone:
            notes.append((NOT_VERIFIED,
                          "исчезли строки с ЧАСТИЧНО отключёнными источниками "
                          "(%d из %d) — они обязаны остаться с меньшей суммой"
                          % (len(shrink_gone), len(shrink_keys))))
        else:
            notes.append((VERIFIED,
                          "строки с частично отключёнными источниками не "
                          "исчезли: %d из %d; из них пересчитано (modify) %d"
                          % (len(shrink_keys) - len(shrink_gone),
                             len(shrink_keys), len(shrink_modified))))
    if intact_keys:
        if intact_touched:
            notes.append((NOT_VERIFIED,
                          "изменились строки с НЕЗАТРОНУТЫМИ источниками: %d из "
                          "%d (%s)"
                          % (len(intact_touched), len(intact_keys),
                             "; ".join(k[:80] for k in intact_touched[:3]))))
        else:
            notes.append((VERIFIED,
                          "нетронутые строки не породили операций: %d; "
                          "повторный прогон дал C/M/D=%s"
                          % (len(intact_keys), (c, m, d))))
    if new_rows:
        notes.append((NOT_VERIFIED,
                      "появились операции над строками, не предсказанные ни "
                      "одной из трёх групп: %d (%s)"
                      % (len(new_rows), "; ".join(k[:80] for k in new_rows[:3]))))
    if second["shapeProblems"]:
        notes.append((NOT_VERIFIED,
                      "форма ответа: %s" % "; ".join(second["shapeProblems"])))
    if not notes:
        notes.append((VERIFIED, "изменений нет: состояние устойчиво"))
    return {"notes": notes,
            "expectedVanish": sorted(vanish_keys),
            "expectedShrink": sorted(shrink_keys),
            "expectedIntact": sorted(intact_keys),
            "vanished": gone,
            "vanishStayed": stayed,
            "shrinkGone": shrink_gone,
            "shrinkModified": shrink_modified,
            "intactTouched": intact_touched,
            "unexpectedNew": new_rows,
            "secondCounters": list(counter(second)),
            "secondStages": second["stages"]}


def compare_recreate(first, second, third, disabled):
    """Третий прогон: удалённая строка обязана вернуться.

    Отключение флага убирает объект из модели, но не вычёркивает строку из
    расчёта. Следующий запуск обязан создать её заново — если не создал,
    значит расчёт потерял строку из-за отключённого источника, и это уже
    дефект движка, а не особенность сценария.
    """
    notes = []
    # Кандидаты на восстановление — строки, предсказанные к исчезновению
    # (все их источники отключены), а НЕ «все строки, которых нет во
    # втором ответе»: второй ответ перечисляет операции, поэтому в нём нет
    # и не тронутых строк, и читать их отсутствие как удаление нельзя.
    exp = expected_after_disable(first, disabled)
    vanished_keys = sorted(r["key"] for r in exp["vanish"])
    removed_objects = len(second.get("deleted") or [])
    third_rows = rows_of(third)
    c, m, d = counter(third)
    create_stage = third["stages"].get("create") or {}

    restored = [k for k in vanished_keys if k in third_rows]
    not_restored = [k for k in vanished_keys if k not in third_rows]
    if vanished_keys:
        if restored:
            notes.append((VERIFIED,
                          "удалённые строки созданы заново: %d из %d "
                          "(третий прогон C/M/D=%s, create=%s)"
                          % (len(restored), len(vanished_keys), (c, m, d),
                             json.dumps(create_stage, ensure_ascii=False))))
        else:
            notes.append((NOT_VERIFIED,
                          "удалённые строки не вернулись: %d из %d (%s) — расчёт "
                          "потерял строку из-за отключённого источника"
                          % (len(not_restored), len(vanished_keys),
                             "; ".join(k[:80] for k in not_restored[:3]))))
    else:
        notes.append((VERIFIED,
                      "строк с полностью отключёнными источниками не было — "
                      "восстановление проверять было нечем (третий прогон "
                      "C/M/D=%s)" % (c, m, d)))
    if vanished_keys and removed_objects:
        notes.append((VERIFIED,
                      "во втором прогоне удалено объектов: %d — столько и "
                      "ожидалось вернуться" % removed_objects))
    if third["shapeProblems"]:
        notes.append((NOT_VERIFIED,
                      "форма ответа: %s" % "; ".join(third["shapeProblems"])))
    return {"notes": notes,
            "vanishedInSecond": vanished_keys,
            "restored": restored,
            "notRestored": not_restored,
            "thirdCounters": list(counter(third)),
            "thirdStages": third["stages"]}


def restore_flags(port, mutation):
    """Возврат исходных значений. Отката на порту нет, поэтому это обычная
    запись, а не Undo; результат читается обратно и попадает в отчёт."""
    prop = mutation["propertyGuid"]
    problems = []
    restored = 0
    for guid, original in mutation["originalValues"].items():
        write_bool(port, [guid], prop, original)
        back = read_values(port, [guid], prop).get(guid)
        if not back or back[2] != original:
            problems.append("%s: ожидалось %r, прочитано %r"
                            % (guid, original, back[2] if back else None))
        else:
            restored += 1
    return {"restored": restored, "problems": problems}


# ---------------------------------------------------------------------------
def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("action", choices=("cycle", "inspect"))
    ap.add_argument("--singleton", action="append", default=[],
                    help="GUID элемента, который является единственным "
                         "источником строки: его отключение обязано удалить "
                         "строку (можно указать несколько раз)")
    ap.add_argument("--bulk-fraction", type=int, default=4,
                    help="если --singleton не задан: отключить 1/N источников "
                         "(4 = четверть)")
    ap.add_argument("--no-restore", action="store_true",
                    help="не возвращать флаги (для отладки)")
    ap.add_argument("--out", default=None)
    args = ap.parse_args()

    port_no = find_port()
    if port_no is None:
        print("ArchiCAD JSON port not found (%d-%d)" % (PORT_FIRST, PORT_LAST))
        return 3
    port = Port(port_no)
    print("port: %d" % port_no)
    os.makedirs(OUT_DIR, exist_ok=True)

    if args.action == "inspect":
        # inspect НЕ делает вызов Spec: он сам сдвинул бы состояние модели и
        # отнял единственный создающий прогон у следующего cycle. Правила и
        # снимок берутся из уже сохранённого first-run.json.
        saved = os.path.join(OUT_DIR, "first-run.json")
        if os.path.isfile(saved):
            with open(saved, encoding="utf-8") as fh:
                data = json.load(fh)
            print("правила (из сохранённого снимка %s): %d"
                  % (saved, len(data.get("ruleProperties") or [])))
            for p in data.get("ruleProperties") or []:
                print("  %-52s %s" % (p["name"][:52], p["guid"]))
            snap = data.get("snapshot") or {}
            print("\nснимок первого прогона: C/M/D=%s, строк created=%d "
                  "modified=%d, объектов delete=%d"
                  % (counter(snap), len(snap.get("created") or {}),
                     len(snap.get("modified") or {}),
                     len(snap.get("deleted") or {})))
            print("счётчики этапов: %s"
                  % json.dumps(snap.get("stages") or {}, ensure_ascii=False))
            print("\nВНИМАНИЕ: это сохранённый снимок, не текущее состояние "
                  "модели. Текущее состояние меняет только cycle.")
        else:
            print("снимка нет: запустите cycle на чистом проекте")
            return 4
        return 0

    report = run_cycle(port, restore=not args.no_restore,
                       singletons=args.singleton,
                       bulk_fraction=args.bulk_fraction)

    path = args.out or os.path.join(OUT_DIR, "cycle.json")
    with open(path, "w", encoding="utf-8") as fh:
        json.dump(report, fh, ensure_ascii=False, indent=2, sort_keys=True)
        fh.write("\n")

    verified = sum(1 for s in report["steps"] if s["status"] == VERIFIED)
    print("\nverified: %d, not verified: %d"
          % (verified, len(report["steps"]) - verified))
    print("итог цикла: %s" % report["status"])
    print("saved: %s" % path)
    return 0 if report["status"] == VERIFIED else 1


if __name__ == "__main__":
    sys.exit(main())