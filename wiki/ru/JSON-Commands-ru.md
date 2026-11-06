# JSON-команды аддона (внешний вызов)

Начиная с версии для Archicad 25 аддон SomeStuff регистрирует собственные JSON-команды. Их можно вызывать из Python-скриптов, агентов и других внешних инструментов через штатный JSON-интерфейс Archicad — без открытия интерфейса аддона.

Все команды выполняются в главном потоке Archicad и требуют открытого проекта. Команды, меняющие модель (построение отделки, спецификаций, синхронизация, запись описания свойства), внутри открывают собственный undo-скоуп — отмена доступна стандартным Ctrl+Z.

## Как вызвать

Адрес — HTTP-порт запущенного экземпляра Archicad (19723 и далее). Формат вызова:

```python
import json, urllib.request

payload = {
    "command": "API.ExecuteAddOnCommand",
    "parameters": {
        "addOnCommandId": {
            "commandNamespace": "SomeStuffCommand",
            "commandName": "Health"
        },
        "addOnCommandParameters": {}
    }
}
body = json.dumps(payload).encode("utf-8")
req = urllib.request.Request("http://127.0.0.1:19723", data=body,
                             headers={"Content-Type": "application/json"})
result = json.load(urllib.request.urlopen(req))
response = result["result"]["addOnCommandResponse"]
```

Ответ команды — в `addOnCommandResponse`. При ошибке команды там будет `{"errorCode": ..., "error": "..."}`.

## Список команд

| Команда (`commandName`) | Что делает | Меняет модель |
|---|---|---|
| `Health` | Проверка доступности аддона: версия и имя открытого проекта | нет |
| `SyncAll` | Полная синхронизация (то же, что «Синхронизировать все») | да |
| `RoomBook` | Построение/обновление ведомости отделки | да |
| `FavoritesList` | Список избранных элементов отделки (имя и тип) | нет |
| `Favorite_CreateOtd` | Создание избранного отделки | да |
| `Roombook_CheckFinishConfiguration` | Проверка классов отделки и свойств Sync_GUID | нет |
| `Spec` | Построение спецификаций (`Spec_rule`) в неинтерактивном режиме | да |
| `CreateClassificationItem` | Создание элемента классификации | да |
| `SetPropertyDescription` | Изменение описания определения свойства | да |

---

## Health

Проверка доступности аддона и выбор экземпляра Archicad.

**Параметры:** нет.

**Ответ:**

```json
{ "status": "ok", "version": "2.01", "addon": "SomeStuff", "projectName": "Мой проект" }
```

Если проект не сохранён или не открыт — `projectName` равен `"untitled"`.

---

## SyncAll

Запускает полную синхронизацию — то же действие, что пункт меню «Синхронизировать все». Состав обхода (стены, окна/двери, объекты, навесные стены) берётся из настроек аддона, как при обычном запуске.

**Параметры:** нет.

**Ответ:**

```json
{ "status": "returned", "elapsedSeconds": 4.1818899 }
```

`status: "returned"` означает только факт завершения вызова и его длительность — успех каждой синхронизации проверяйте обычными средствами (значения свойств, журнал аддона).

---

## RoomBook

Запускает построение ведомости отделки. Без параметров работает как обычный запуск (интерактивный отбор зон). С параметром `zones` обрабатывает только перечисленные зоны.

**Параметры:**

- `zones` (необязательный) — непустой массив строк с GUID зон. Формат GUID: 36 символов с дефисами (`"XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX"`) или тот же GUID в фигурных скобках. Повторы допускаются и отбрасываются.

Каждая зона проверяется: должна существовать, быть зоной, редактируемой, видимой на видимом слое и доступной (без этих условий команда возвращает ошибку с GUID зоны).

**Ответ без `zones`** — общий статус вызова; **с `zones`** — отчёт по построенным элементам:

```json
{
  "status": "returned",
  "rooms": [
    {
      "zone_guid": "F5B7E2A1-...",
      "types": [
        {
          "TypeOtd": 1,
          "elements": [
            {
              "otd_guid": "A1B2C3D4-...",
              "base_guid": "9E8D7C6B-...",
              "write_succeeded": true,
              "was_created": true,
              "is_valid": true,
              "drawing_enabled": true,
              "draw_error": "",
              "length": 3.0,
              "length_available": true,
              "area": 12.5,
              "area_available": true,
              "material": 42,
              "bmaterial": 0,
              "structtype": 1
            }
          ]
        }
      ]
    }
  ]
}
```

- `rooms[].types` — группы по типу отделки (`TypeOtd`); `elements` — созданные/обновлённые элементы отделки.
- `otd_guid` — GUID созданного/обновлённого элемента отделки, `base_guid` — GUID базового элемента.
- `write_succeeded` / `was_created` — запись свойств и факт создания; `is_valid` / `drawing_enabled` / `draw_error` — валидность, разрешение построения и текст ошибки.
- `length`/`area` — длина/площадь; при недоступности соответствующий `*_available` равен `false`, а значение не вычислялось.
- `material`/`bmaterial`/`structtype` — внутренние номера материалов и типа структуры элемента.

---

## FavoritesList

Возвращает текущий словарь избранных элементов отделки (то, что видит инструмент «Избранное» отделки).

**Параметры:** нет.

**Ответ:**

```json
{ "favorites": [ { "name": "Стена_кирпич", "type": 1 } ] }
```

`type` — числовой тип отделки (тот же код, что в `Favorite_CreateOtd` и в `TypeOtd` ответа `RoomBook`).

---

## Favorite_CreateOtd

Создаёт избранный элемент отделки с заданным именем, типом и папкой (настройки берутся из инструмента типа отделки).

**Параметры (все обязательны):**

| Поле | Тип | Значение |
|---|---|---|
| `type` | число | тип отделки (код `TypeOtd`) |
| `name` | строка | имя избранного (непустое) |
| `folder` | строка | папка избранного |

**Ответ:** `{ "status": "success" }` либо ошибка, если создание не удалось (например, имя занято).

---

## Roombook_CheckFinishConfiguration

Проверяет, что классы отделки и свойства `Sync_GUID` настроены корректно. Удобно вызывать перед построением отделки.

**Параметры:** нет.

**Ответ:**

```json
{ "succeeded": false, "errors": "Класс отделки не найден: ...\n..." }
```

`succeeded` — итог проверки; `errors` — найденные проблемы, по одной на строку (пустая строка при успехе).

---

## Spec

Запускает построение спецификаций (`Spec_rule`) в неинтерактивном режиме: без диалогов, с точкой вставки и необязательным списком правил.

**Параметры:**

| Поле | Тип | Значение |
|---|---|---|
| `placementPoint` | объект `{ "x": число, "y": число }` | точка вставки элементов спецификации на плане; **обязателен** |
| `ruleNames` | массив строк, ≥ 1 элемента (необязателен) | имена правил `Spec_rule`; без него обрабатываются все правила |
| `includeParameters` | булево (необязателен) | `true` — включить в ответ дамп свойств и GDL-параметров созданных/изменённых элементов |

**Ответ:**

```json
{
  "status": "ok",
  "resultCode": 0,
  "elementsToCreate": 12,
  "elementsToModify": 3,
  "elementsToDelete": 1,
  "elapsedSeconds": 42.1,
  "includeParameters": false,
  "create":   { "attempted": 12, "succeeded": 12, "failed": 0 },
  "grouping": { "attempted": 3,  "succeeded": 3,  "failed": 0 },
  "gdl":      { "attempted": 0,  "succeeded": 0,  "failed": 0 },
  "deleteOld":{ "attempted": 1,  "succeeded": 1,  "failed": 0 },
  "hasPrimaryError": false,
  "hasRecoveryError": false,
  "hasUnconfirmedCreate": false,
  "prepareFailureStage": "none"
}
```

- Счётчики `create/grouping/gdl/deleteOld` показывают фактические результаты этапов (попытки, успехи, отказы), а не только планы.
- `prepareFailureStage` — этап, на котором запуск прервался до обращения к модели (`none` при успешном старте): `rulesNotFound`, `readParamsNotFound`, `writeParamsNotFound`, `placeParamsNotFound`, `tooManyErrorElements`, `emptyElementsList`, `nothingCreated`, `canceledByUser`, `existingElementsLocked`.
- При `includeParameters: true` добавляются списки `created`/`modified` (каждый элемент: `guid`, `favoriteName`, `sourceElement[].guid`, `property[]` и `gdlParameter[]` с парами `name`/`value`), список `deleted[].guid`, статистика `rules[]` (имя правила и числа created/modified/deleted) и `messages[]` (`ruleName`, `text`).

---

## CreateClassificationItem

Создаёт элемент классификации (пункт системы классификации).

**Параметры:**

| Поле | Тип | Значение |
|---|---|---|
| `systemGuid` | строка | GUID системы классификации; обязателен |
| `itemId` | строка | Id нового элемента; обязателен |
| `itemName` | строка | имя нового элемента; обязательно |
| `itemDescription` | строка | описание; обязательно |
| `parentItemGuid` | строка (необязателен) | GUID родительского элемента; пустой или опущенный — корневой элемент |

**Ответ:** `{ "itemGuid": "A1B2C3D4-..." }` — GUID созданного элемента.

GUID системы и родителя можно получить штатными командами `GetAllClassificationSystems` и `GetAllClassificationsInSystem` (родительский GUID виден в дереве системы).

---

## SetPropertyDescription

Меняет описание определения свойства. Именно по описанию аддон ищет команды (`Sync_from{...}`, `Spec_rule{...}` и т. п.), поэтому команда удобна для автоматической настройки проекта: имя свойства остаётся прежним.

**Параметры:**

| Поле | Тип | Значение |
|---|---|---|
| `description` | строка | новое описание; **обязателен** (пустая строка очищает описание) |
| `propertyGuid` | строка (необязателен) | GUID определения свойства |
| `name` | строка (необязателен) | полное имя свойства `"Группа/Имя"`; используется, только если `propertyGuid` не задан |

**Ответ:**

```json
{ "propertyGuid": "A1B2C3D4-...", "changed": true, "description": "Sync_from{Material:Layers; ...}" }
```

`changed: false` означает, что описание уже совпадало и запись не выполнялась. В ответе возвращается описание, прочитанное обратно из модели, а не поданное на вход.

---

## Замечания

- Все команды доступны только в сборках аддона для Archicad 25 и новее.
- Команды `RoomBook`, `SyncAll`, `Spec` выполняют ту же логику, что пункты меню аддона; `Spec` — единственная команда с неинтерактивным режимом (диалоги подавлены, параметры передаются извне).
- `status: "returned"` и `elapsedSeconds` — факт возврата и длительность вызова, а не подтверждение корректности результата; для этого читайте содержательные поля ответа или проверяйте модель.
