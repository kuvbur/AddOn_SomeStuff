# Sync — Синхронизация свойств

> Базовый хеш карточки: 3a24131 (2026-09-22); #199 — в `9ae0138` (2026-09-24); #210 (вынесение `OtherDbDialog`) и #203 (script marker) — рабочее дерево `llm_test` (2026-09-25); #202 — `6c9fc4d` (2026-09-28). Исторические номера строк относятся к базовой ревизии и могут не совпадать с текущим кодом. CallHierarchy собран для 6 ключевых функций (0-based у clangd; в таблицах 1-based).

## Назначение
Главный модуль: синхронизация и мониторинг свойств между элементами по правилам из описаний свойств (Sync_from/Sync_to). Ядро add-on. [по коду]

## Файлы
- `Sync.cpp/hpp`
- `dialogs/OtherDbDialog.cpp/hpp` — выбор базы/этажа для связанных элементов вне текущего контекста. [по коду]

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SyncRule` | paramNameFrom/paramFrom, paramNameTo/paramTo, ignorevals, templatestring, synctype, syncdirection [из комментария, Sync.hpp:13-30] |
| `WriteData` | guidTo, guidFrom, paramFrom, paramTo, ignorevals, formatstring, toSub, fromSub [из комментария] |
| `ParsedPropertyCommand` | commandType ("Sync", "Renum_flag", "Renum", "Sum", "Spec_rule"), fullCommand, parameters, isValid, errorMessage [из комментария, Sync.hpp:205-211] |

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `MonAll` | 64 | Мониторинг для всех активных типов [из комментария]; вызывается из `Initialize` (Main:555) и `MenuCommandHandler` (Main:400) [из callgraph.json] |
| `MonByType` | 117 | Мониторинг для заданного типа [из комментария] |
| `SyncAndMonAll` | 170 | Полная синхронизация + мониторинг [из комментария] |
| `SyncByType` | 306 | Синхронизация элементов выбранного типа [из комментария]; вызывается из `SyncAndMonAll` ×5 [из callgraph.json] |
| `SyncElement` | 383/391 | Синхронизация одного элемента и подэлементов [из комментария] |
| `SyncSelected` | 435 | Синхронизация выбранных [из комментария]; вызывается из SomeStuff_Main.cpp:410 и внутри Sync.cpp:565/2246 [по коду, grep] |
| `SyncArray` | 456 | Синхронизация массива, возвращает обработанные GUID [из комментария] |
| `RunParamSelected` / `RunParam` | 777 / 825 | Запуск параметрических скриптов выбранных элементов; после успешного скрипта окна/двери `RunParam` определяет тип через кросс-версионный `GetElemTypeID(element)` и запускает скрипт marker из `openingBase.markGuid`, если GUID задан [по коду]; RunParamSelected — из SomeStuff_Main.cpp:441 [по коду, grep] |
| `SyncRelationsElement` | 641 | Синхронизация связанных элементов [из комментария]; вызывается из `SyncElement` (:416) [из callgraph.json] |
| `SyncData` | 669 | Синхронизация по описаниям свойств [из комментария] |
| `Name2Rawname` | 1074 | Преобразование имени в ключ параметра: независимо дополняет отсутствующие `{` и `}`, затем определяет префикс и нормализует регистр. Пустое имя отвергает; оба выходных аргумента могут изменяться. [по коду; RED→GREEN AC25 на неполном имени, полный TESTING-набор не прошёл из-за ошибок классификации в `CompareParamValue`] |
| `ParseSyncString` | 1171 | Парсинг описания в WriteData [из комментария] |
| `SyncString` | 1528 | Парсинг одной команды [из комментария] |
| `ParseFileNumber` | 90 | Разбор числового аргумента правила `File:`; `static`, только для `SyncString`. Принимает токен, целиком состоящий из числа (#202) |
| `SyncSetSubelement` | 2163 | Запись GUID подэлементов в основной элемент [из комментария]; вызывается из SomeStuff_Main.cpp:453 [по коду, grep] |
| `SyncSetSubelementScope` | 2258 | Запись GUID (для CallUndoableCommand) [из комментария] |
| `SyncShowSubelement` | 2316 | Подсветка элементов по Sync_GUID [из комментария]; вызывается из SomeStuff_Main.cpp:445 [по коду, grep] |
| `SyncGetParentelement` / `SyncGetSubelement` | 2531 / 2649 | Словари родительских/дочерних GUID [из комментария] |
| `ParsePropertyDescription` | 2750 | Парсинг полного описания (все команды) [из комментария] |

## Карточки

### `SyncAndMonAll(SyncSettings &syncSettings)`
- Расположение: `Sources/AddOn/Sync.cpp:170`
- Назначение: полная синхронизация и мониторинг для активных элементов. [из комментария]
- Контракт: перед обходом типов — `ResetProperty()`: при успехе сброса ранний выход (Sync.cpp:173-174). [по коду]
- Побочные эффекты: **запись свойств элементов** (SyncByType-цепочка: wallS/widoS/objS/cwallS); подключение мониторинга; `ElementsWrite`/`WriteInfo` (Helpers); информирование о непрочитанных GDL (`CountUnreadGDLParams`). [по коду]
- Вызывает: `SyncByType` ×5 (:191/202/212/222/229), `SyncArray` (:296), `ResetProperty` (:174), `ElementsWrite`, `WriteInfo`, `IsDummyModeOn`, `CountUnreadGDLParams`, `isEng`, `Get*S` (SyncSettings), `msg_rep`. [из callgraph.json]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:405). [из callgraph.json]

### `SyncData(const API_Guid &elemGuid, ..., ParamDictElement &paramToWrite, int dummymode) -> bool`
- Расположение: `Sources/AddOn/Sync.cpp:669`
- Назначение: применяет правила синхронизации к элементу и подэлементам. [из комментария]
- Контракт: не редактируемый элемент пропускается (`IsElementEditable`, :683); dummy-режим — ранний выход (:700). [по коду]
- Побочные эффекты: **запись значений в свойства/параметры/GDL** целевых элементов (через SyncCalcRule → ElementsWrite-цепочку); может назначить автокласс (`SetAutoclass`, :689). [по коду]
- Вызывает: `ParseSyncString` (:729), `SyncAddSubelement` (:746), `SyncCalcRule` (:754/759), `SyncNeedResync` (:760), `SetAutoclass`, `ElementsRead`, `CompareParamDictElement`, `GetElemState`/`Reverse`, `IsElementEditable`, `SubGuid_GetParamValue`, `AddPropertyDefinition`. [из callgraph.json]

### `SyncElement(const API_Guid &elemGuid, const SyncSettings &syncSettings, ParamDictElement &paramToWrite, int dummymode) -> bool`
- Расположение: `Sources/AddOn/Sync.cpp:383` (перегрузка с UnicGuid — :391)
- Назначение: синхронизация одного элемента и его подэлементов. [из комментария]
- Контракт: дедупликация через `ContainsKey` по словарям (guid уже в worklist — пропуск); рекурсия: перегрузка вызывает основную (:388). [по коду]
- Побочные эффекты: **запись свойств** (через SyncData); трогает связанные элементы (`SyncRelationsElement`, :416). [по коду]
- Вызывает: `SyncData` (:411/425), `SyncRelationsElement` (:416), `GetTypeByGUID` (:399), `GetParentGUIDSectElem` (:404), `GetRelationsElement` (:410). [из callgraph.json]
- Вызывается из: `SyncArray` (:479), `SyncByType` (:352), перегрузка `SyncElement` (:388). [из callgraph.json]

### `SyncCalcRule(...)` — выбор адресатов правила
- Расположение: `Sources/AddOn/Sync.cpp:1057` (рабочее дерево #199; номер подтверждён по коду).
- Контракт: сначала обходятся переданные GUID текущего элемента/подэлементов в исходном порядке, затем однократно добавляются целевые GUID из ключей `WriteDict`, отсутствующие в этом списке; `APINULLGuid` не добавляется. Это позволяет обработать `Sync_to_GUID`, направленный на внешний элемент. Повторный расчёт после `CompareParamDictElement` использует тот же набор адресатов. [по коду]
- Побочные эффекты: формируется `paramToWrite`; собственно запись в Archicad выполняется позднее в `ElementsWrite`. Вызов на реальной связке элементов — не проверено. [по коду]
- Регрессия: `TestSyncAddSubelement` воспроизвёл отсутствие записи для внешнего GUID (RED), затем сформировал запись (GREEN) на AC25. [по результату запуска TESTING]

### `OtherDbDialog` — переход к элементам в другой базе данных
- Расположение: `Sources/AddOn/dialogs/OtherDbDialog.cpp/.hpp` (#210); `Sync.cpp` формирует цели и вызывает модуль. [по коду]
- Контракт: `OtherDbTarget` группирует GUID по базе и этажу. Заголовок получает `SubElementHalfId`, а подписи кнопок и столбцов — `OtherDbCloseId`…`OtherDbShow3DId` (`OtherDbShow3DId` = 87, добавлен в #246) из `ID_ADDON_STRINGS`/`ID_ADDON_STRINGS_ENG` через `RSGetIndString`. Встроенных RU/EN строк интерфейса в классе нет. Кнопок три: `CloseButtonId`=1, `ShowButtonId`=2, `ListBoxId`=3, `Show3DButtonId`=4 (#246). [по коду и ресурсам]
- Побочные эффекты: после принятия модального окна результат (`ResultID`) выбирает ветку. `SelectOtherDbTarget` (кнопка «Показать») работает с ВЫБРАННОЙ строкой: переключает текущую БД/этаж, выделяет GUID и выполняет zoom. `ShowAllOtherDbTargetsIn3D` (кнопка «Показать в 3Д») выбор строки игнорирует и показывает объединение GUID'ов из ВСЕХ целей списка; порядок вызовов: `ACAPI_View_ShowAllIn3D` / `APIDo_ShowAllIn3DID` → выделение → `HighlightElements` → `ACAPI_View_ZoomToElements` / `APIDo_ZoomToElementsID` → redraw. Отказ выделения или зума не прерывает показ. БД и этаж в 3D-ветке НЕ меняются. Отмена и невалидный индекс ничего не меняют. [по коду, проверено вживую на AC25]
- Ресурсы кнопки заданы в `Tools/AddOn.grc.in` (строка 87 RU/EN, элемент `[4]` блока `'GDLG' ID_ADDON_OTHER_DB_DLG` и `Button_2` в `'DLGH'`); `Sources/AddOnResources/RINT/AddOn.grc` — генерируемый CMake-файл, в git не лежит (`.gitignore:371`). [по коду и CMakeCommon.cmake:447]
- Проверка вживую (AC25, 2026-10-02): «Показать» и «Показать в 3Д» работают — 3D-окно открывается, элементы со всех баз подсвечены и выделены, камера наводится. Автотестами сценарий не покрыт (интерактивный).

## Зависимости
- `Helpers.hpp`, `Propertycache.hpp`, `CommonFunction.hpp`, `DG.h`, `dialogs/SyncSettings.hpp`, `dialogs/OtherDbDialog.hpp` (#210) [по include]

## Зависимости (используется в)
- `BrowserPalette`, `SyncSettings.cpp`, `SomeStuff_Main` [по коду]

## Инварианты
- Правила `Sync_to{Attribute:Composite/BuildingMaterial/CompositeType}` используют существующий парсер `Name2Rawname` → `{@attrib:...}` и общий путь `ElementsWrite` → `ParamHelpers::WriteAttribute`; в текущем объёме (#235) только стены, перекрытия, крыши и оболочки и переходы Basic ↔ Composite. `Profile`, балки и колонны исключены решением владельца. [по коду Sync.cpp:1117-1172, Helpers.cpp; runtime назначения не проверен]
- `IsElementThrottled` ограничивает повторную обработку одного GUID на 500 мс от первого принятого уведомления; пропущенные уведомления не продлевают окно. Используется только после проверки обрабатываемого `notifID` и возможности обработки; `New` подключает observer до throttle. Для размеров адресное округление также ограничено по GUID. [по коду, Sync.cpp:258+, SomeStuff_Main.cpp:155+]
- `IsDimensionScanThrottled` хранит независимую временную метку полного обхода после `EndEvents`, только когда есть правила; `BeginEvents`/`EndEvents` обрамляют отдельные уведомления, а не весь каскад (по наблюдению пользователя на AC25). Обе метки очищаются при открытии/закрытии проекта и отключении монитора. [по коду, Sync.cpp:288+, SomeStuff_Main.cpp:93+]
- Таймер не доказывает неизменность входных данных: самостоятельное изменение того же GUID внутри 500 мс может быть пропущено. Синхронность вложенных уведомлений при записи и длительность каскада не проверены. [не проверено]
- Спец-значения игнорирования: "empty", "trim_empty", "def" [из комментария, Sync.hpp:52-56]
- Правила `File:`: все пять числовых аргументов (номер возвращаемого столбца, конец/начало диапазона столбцов, конец/начало диапазона строк) разбираются только если токен целиком число — `2junk` отклоняется, а не читается как `2` (#202). Отказ фиксируется в `fileNumberValid`, и на границе ветки `SyncString` возвращает `false`: установка `syncdirection = SYNC_NO` внутри ветки правило не отбраковывает, т.к. `synctypefind` уже `true`. [по коду, Sync.cpp:90, 1619, 1724]
## Версии
- AC27 (`#170`): `ACAPI_Database (APIDb_RebuildCurrentDatabaseID)` → `ACAPI_Database_RebuildCurrentDatabase ()` под `#ifdef ServerMainVers_2700`. [по коду DevKit + сборка AC27]
