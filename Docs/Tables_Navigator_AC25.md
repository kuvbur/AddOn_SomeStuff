# Ведомости и спецификации в Навигаторе — редакция для Archicad 25

Дата проверки: 2026-09-17. Проект: AddOn_SomeStuff. Целевая версия этого документа: **AC25**; перенос на AC22–24/26–29 не проверен.

**Статус: проверенная редакция документации** (read-only задача). Код не изменялся, сборка и runtime не выполнялись. Проверены Navigator, MyDraw, IDF, Add-On Object, фильтры/metadata, палитра и события. Runtime-пункты — отдельный список в §10.

Исходник: `C:/Users/da-rogojin/AppData/Local/hermes/attachments/pasted_content_2026-09-17_15-07-50-513_a3082f.txt` (378 строк, карта опыта Ext-Table AC29). Это требования/гипотезы другого проекта, не документация SDK. Часть имён функций AC29 в AC25 не существует; ключевые расхождения отмечены по тексту.

## Источники и смысл отметок

- **Verified (SDK25)** — контракт установленного SDK, заголовок или конкретный пример; это не runtime-тест.
- **Предложение** — проектное решение, а не требование SDK.
- **not verified** — недостаточно доказательств; указана необходимая проверка.

Сокращения путей, все относительно `D:/SomeStuff_addon/Build/DevKit/APIDevKit-25/`:
- `H/` = `Support/Inc/`.
- `F/` = `Documentation/HTML/APIDevKit/APIHTMLLibrary/Functions/`.
- `S/` = `Documentation/HTML/APIDevKit/APIHTMLLibrary/Structures/`.
- `N/` = `Examples/Navigator_Test/Src/`.

Поиск выполнен через LightRAG `http://127.0.0.1:9621/query` (`only_need_context: true`, local и уточняющие naive). Контракты подтверждены установленным SDK25. Сгенерированные описания графа LightRAG не заменяют исходник.

## 1. Архитектура

Собственная ведомость — не нативный Interactive Schedule. Проверенный механизм Navigator_Test:

**Собственная точка зрения в Project Map → MyDraw для просмотра → CreateIDFStore для генерации содержимого чертежа.**

Данные модели, фильтры, группировки, раскладка таблицы, стили и разбиение на листы — наша реализация. Navigator_Test копирует элементы плана (`N/NavigatorUtility.cpp:41–73`), а не реализует таблицы.

Предложение: отделить определение ведомости и её оформление от рассчитанных строк, 2D-представления и состояния выбора. Один генератор 2D-содержимого использовать для окна и IDF. Реальная печать/размещение/обновление таблицы на макете в SomeStuff — **not verified**, необходим сквозной runtime-прототип.

## 2. Точная таблица соответствий AC25

Ниже **формы вызовов**, не объявления отдельных функций с такими именами: в AC25 Navigator и Database используют диспетчеры `ACAPI_Navigator` и `ACAPI_Database`.

| В исходнике AC29 | Форма вызова в AC25 | Источник | Статус |
|---|---|---|---|
| `ACAPI_Navigator_RegisterCallbackInterface` | `ACAPI_Navigator(APINavigator_RegisterCallbackInterfaceID, &callback)` | `F/APINavigator_RegisterCallbackInterfaceID.html`, `N/NavigatorTest.cpp:134` | Verified (SDK25) |
| `ACAPI_Navigator_GetNavigatorVPRootGroups` | `ACAPI_Navigator(APINavigator_GetNavigatorVPRootGroupsID, &rootGuids)`; результат `GS::Array<API_Guid>` | `F/APINavigator_GetNavigatorVPRootGroupsID.html`, `N/NavigatorUtility.cpp:164–170` | Verified (SDK25) |
| `ACAPI_Navigator_GetNavigatorVPItem` | `ACAPI_Navigator(APINavigator_GetNavigatorVPItemID, &vpData)`; вход `vpData.guid` | `N/NavigatorWindowHandling.cpp:127–132` | Verified (SDK25) |
| `ACAPI_Navigator_CreateNavigatorVPItem` | `ACAPI_Navigator(APINavigator_CreateNavigatorVPItemID, &vpData)`; данные in/out | `F/APINavigator_CreateNavigatorVPItemID.html` | Verified (SDK25) |
| `ACAPI_Navigator_ChangeNavigatorVPItem` | `ACAPI_Navigator(APINavigator_ChangeNavigatorVPItemID, &vpData)`; данные in, GUID выбирает существующую запись | `F/APINavigator_ChangeNavigatorVPItemID.html` | Verified (SDK25) |
| `ACAPI_Navigator_GetNavigatorVPItemChildren` | `ACAPI_Navigator(APINavigator_GetNavigatorVPItemChildrenID, &parentGuid, &childrenGuids)` | `F/APINavigator_GetNavigatorVPItemChildrenID.html`, `N/NavigatorUtility.cpp:186–200` | Verified (SDK25) |
| Удаление ведомости | `ACAPI_Navigator(APINavigator_DeleteNavigatorVPItemID, &guid)` | `N/NavigatorCallbackInterface.cpp:74–91` | Verified (пример SDK25) |
| `ACAPI_Window_NewWindow` | `ACAPI_Database(APIDb_NewWindowID, &windowPars, (void*)handlerProc)` | `N/NavigatorWindowHandling.cpp:156–165` | Verified (пример SDK25) |
| `ACAPI_Drawing_StartDrawingData` | `ACAPI_Database(APIDb_StartDrawingDataID, &scale)`; второй параметр диспетчера после scale необязательный `API_PenType**` | `F/APIDb_StartDrawingDataID.html` | Verified (SDK25) |
| `ACAPI_Drawing_StopDrawingData` | `ACAPI_Database(APIDb_StopDrawingDataID, &idfStore, &boundingBox)`; выходы `GSPtr` и `API_Box` | `F/APIDb_StopDrawingDataID.html`, `N/NavigatorCallbackInterface.cpp:36–45` | Verified (SDK25) |
| `ACAPI_ProjectOperation_CatchProjectEvent` | **Отсутствует.** `ACAPI_Notify_CatchProjectEvent(GSFlags, APIProjectEventHandlerProc*)` | `H/ACAPinc.h:1300–1301` | Verified (SDK25) |
| `ACAPI_Attribute_GetAttributesByType` | **Отсутствует.** `ACAPI_Attribute_GetNum` + `ACAPI_Attribute_Get` | `H/ACAPinc.h:412–414` | Verified (SDK25) |
| `ACAPI_ProjectSetting_GetStorySettings` | **Отсутствует.** `ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr)` | `H/APIdefs_Environment.h:88` | Verified (SDK25) |
| `ACAPI_AddOnObject_*` | Свободные функции с префиксом `ACAPI_AddOnObject_` и суффиксами `CreateObject`, `CreateUniqueObject`, `CreateUniqueObjectMore`, `CreateClientOnlyObject`, `GetObjectList`, `GetObjectContent`, `ModifyObject`, `DeleteObject`, `GetObjectGuidFromName`, `GetUniqueObjectGuidFromName`, `GetClientOnlyObjectGuidFromName` | `H/ACAPinc.h:1262–1282` | Verified (SDK25) |

### Регистрация и интерфейс

`INavigatorCallbackInterface` существует в AC25. Объект callback должен жить всё время его использования Archicad; временем жизни управляет аддон (`F/APINavigator_RegisterCallbackInterfaceID.html`, Remarks).

В исходнике пропущены два обязательных pure virtual метода: `ExecuteMergePostProcess` и `GetElemsForDrawingCheck`. Полный перечень обязательных методов находится в `H/INavigatorCallbackInterface.hpp:82–90`:
`OpenView`, `OpenSettings`, `ExecuteMergePostProcess`, `CreateIDFStore`, `GetElemsForDrawingCheck`, `NewItem`, `DeleteItem`, `RenameItem`, `GetIcon`. Настройки контекстного меню и TW-тексты имеют реализации по умолчанию (`:91–94`, `:108–129`).

Точные объявления AC25:

```cpp
virtual GSErrCode CreateIDFStore (const API_Guid& viewPointID, double scale, double& clipBoxWidth, double& clipBoxHeight, GSPtr& idfStore, API_Box& boundingBox, double& paddingX, double& paddingY, GS::Array<API_Guid>& elems) const = 0;
virtual GSErrCode GetElemsForDrawingCheck (const API_Guid& viewPointID, GS::Array<API_Guid>& elems) const = 0;
virtual GSErrCode ExecuteMergePostProcess () const = 0;
```

Это справочные объявления из `H/INavigatorCallbackInterface.hpp:84–86`, не готовый класс реализации.

Обработка файлового жизненного цикла — отдельная регистрация: `ACAPI_Register_NavigatorAddOnViewPointDataHandler()` в `RegisterInterface`, установка merge/save-old/convert-new handlers в `Initialize` (`F/ACAPI_Register_NavigatorAddOnViewPointDataHandler.html`; `N/NavigatorTest.cpp:84–145`). Нельзя ограничиться одной регистрацией callback интерфейса и считать merge/миграции реализованными.

## 3. Идентичность и ограничения Navigator

**Verified (SDK25):**
- При создании leaf node SDK игнорирует входной `guid` и всегда выдаёт новый. Root/group допускают явно заданный GUID. Сохранить возвращённый GUID; не пытаться назначить его leaf node самостоятельно (`F/APINavigator_CreateNavigatorVPItemID.html`, Remarks).
- `ChangeNavigatorVPItemID` изменяет существующий узел по GUID, но игнорирует `parentGuid` и `itemType`: этим вызовом нельзя перенести узел или поменять его тип (`F/APINavigator_ChangeNavigatorVPItemID.html`).
- В Teamwork запрещено создавать, изменять и удалять roots/groups. Для изменения не зарезервированного узла возвращается `APIERR_NOTMINE`. Это ограничение отсутствовало в исходной карте AC29.

**Предложение:** GUID ведомости сохранять при обычном пересчёте. Это необходимое проектное условие стабильных связей, но сохранность всех вариантов Drawing/форматирования после обновлений ещё требует runtime-проверки. Merge в другой PLN — отдельный случай с переназначением идентичностей, а не обещание неизменного GUID в любых проектах.

Счётчик `ScheduleInternal:<serial>` — пользовательский формат, не API-требование. Коллизии при merge/Teamwork должны решаться явно; локального increment недостаточно для доказательства уникальности.

## 4. MyDraw: окно, идентификатор и обновление

- `APIWind_MyDrawID` допускает реальные 2D-элементы, но не конструктивные элементы; пользовательский ввод и редактирование отключены (`F/APIDb_NewWindowID.html`, Remarks). **Verified (SDK25)**. Редактор в DG/BrowserPalette — возможный отдельный UI, не обязательный выбор SDK.
- `API_NewWindowPars.userRefId` — `API_Guid`, возвращаемый в уведомлениях (`H/APIdefs_Database.h:256–264`). Пример задаёт GUID точки зрения (`N/NavigatorWindowHandling.cpp:156–162`).
- Callback получает `(const API_Guid& userRefId, API_NotifyWindowEventID notifID)`; события Activate, Rebuild и Close (`F/APICustomWindowHandlerProc.html`).
- Идентичность доступна не только через Activate: пример получает `API_WindowInfo.databaseUnId.elemSetId` через запрос текущего окна и сверяет с собственными окнами (`N/NavigatorWindowHandling.cpp:195–215`). `index` не использовать как ключ ведомости.
- Открытие нового окна запрещено на notification level: `APIERR_REFUSEDCMD` (`F/APIDb_NewWindowID.html`). Не открывать окна произвольно из обработчиков изменений.

**Расхождение внутри SDK25:** старый пример в `F/APIDb_NewWindowID.html` использует отсутствующее в текущей структуре `userRefCon`; `S/API_WindowInfo.html` содержит старое описание custom window через index. Для реализации брать актуальный `H/APIdefs_Database.h:256–264`, GUID-сигнатуру callback и `N/NavigatorWindowHandling.cpp:125–168,195–215`, а не эти старые фрагменты HTML.

### Reset не равен Rebuild

`APIDb_ResetCurrentDatabaseID` **удаляет все элементы** собственной drawing database (`F/APIDb_ResetCurrentDatabaseID.html`). `APIDb_RebuildCurrentDatabaseID` перестраивает текущую базу; документация не подтверждает, что один этот вызов обязательно уничтожает последнее корректное содержимое (`F/APIDb_RebuildCurrentDatabaseID.html`).

В Navigator_Test `APIWindowGuard` вызывает Reset в конструкторе, а SetZoom/Rebuild в деструкторе (`N/NavigatorWindowHandling.cpp:111–122`). Это **не guard переключения/восстановления базы** и не rollback.

Предложение: предупреждение показывать без разрушительной регенерации; данные и раскладку готовить до Reset, не менять отметку успешного содержимого при ошибке. Гарантия last-good после ошибки посередине создания геометрии — **not verified**, нужна отдельная стратегия восстановления.

## 5. IDF и валидаторы

`StartDrawingDataID` перенаправляет создание элементов во временное хранилище; допустимы только 2D-элементы, включая изображения. Вложенные сессии запрещены (`APIERR_NESTING`). Масштаб 1:100 передаётся числом 100, а не 0.01. Пользовательская таблица перьев содержит ровно 255 записей и задаётся до генерации (`F/APIDb_StartDrawingDataID.html`).

`StopDrawingDataID` завершает сессию и возвращает непрозрачный сериализованный поток `GSPtr`, а не GUID чертежа. Возврат потока из `CreateIDFStore` и самостоятельное создание drawing element — разные пути. Само получение IDF ещё не означает, что чертёж размещён на макете.

В реализации Start/Stop должны быть сбалансированы только для успешно начатой своей сессии; сохранить исходную ошибку генерации, не затирать её результатом cleanup. Пример `N/NavigatorCallbackInterface.cpp:36–45` не является образцом полной обработки ошибок: безусловно вызывает Stop и не проверяет результат копирования элементов.

`API_WindowValidatorInfo` хранит GUID окна, `elemList` и `checkSumList` (`S/API_WindowValidatorInfo.html`). Пример сам вызывает проверку при активации, обновляет валидатор, обрабатывает Rebuild и уничтожает валидатор на Close (`N/NavigatorWindowHandling.cpp:15–82`). Это **не автономная подписка на любые изменения свойств/фильтров**.

Предложение: зависимости включают актуальный состав исходных элементов и определение ведомости. Изменение набора по фильтру, новые элементы, проектные настройки и внешние для GUID зависимости необходимо учитывать отдельно. `GetElemsForDrawingCheck` возвращает элементы, от которых зависит чертёж; одного валидатора окна недостаточно, чтобы обещать автоматическое обновление размещённого Drawing во всех случаях.

## 6. Хранение и владение памятью

### 6.1. Payload точки зрения

**Verified (SDK25):** `API_NavigatorAddOnViewPointData` — не POD. Нельзя инициализировать её через `memset/BNZeroMemory/BNClear`. Членом `data` владеет сама структура: copy-конструктор копирует handle, move переносит и обнуляет источник, деструктор освобождает (`H/APIdefs_Navigator.h:226–283`; `S/API_NavigatorAddOnViewPointData.html`, Remarks).

Не управлять lifetime `.data` отдельным `BMKillHandle`, `BMHandleToHandle` или вторым HandleGuard. Не создавать shallow alias между владельцами. В `F/APINavigator_CreateNavigatorVPItemID.html` прямо указано: деструкторы освободят `nodeVP.data` и `nodeVP2.data`. `ChangeNavigatorVPItemID` принимает const input; передачи владения API не документирует. Обобщение исходника об ownership после Change заменено этим контрактом.

Payload точки зрения сохраняется в проекте: `S/API_NavigatorAddOnViewPointData.html`, член `data`; `F/APINavigatorAddOnViewPointDataSaveOldFormatHandlerProc.html`, параметры currentFormatVPDataArray/oldFormatVPDataArray и Remarks. **Отдельный Add-On Object не обязателен по SDK.** Он остаётся архитектурным вариантом для данных, которым нужен отдельный жизненный цикл. Скорость/лимиты больших payload здесь не проверены.

### 6.2. Add-On Object

Сигнатуры сверены с `H/ACAPinc.h:1262–1280`. Таблица содержит формы вызовов, где переменные предполагают соответствующие объявленные типы.

| Форма AC25 | Контракт | Источник |
|---|---|---|
| `ACAPI_AddOnObject_CreateObject(name, content, &guid)` | `name`: const UniString&, `content`: const GSHandle&, GUID — out; content копируется, свой входной handle освобождает вызывающий | `F/ACAPI_AddOnObject_CreateObject.html` |
| `ACAPI_AddOnObject_ModifyObject(guid, &newName, &newContent)` | GUID — const reference; имя/content — указатели, можно передать nullptr вместо одного для сохранения старого поля, но не обоих; content копируется | `F/ACAPI_AddOnObject_ModifyObject.html` |
| `ACAPI_AddOnObject_GetObjectContent(guid, &name, &content)` | оба выходных указателя обязательны; Archicad выделяет handle, вызывающий освобождает `BMKillHandle` | `F/ACAPI_AddOnObject_GetObjectContent.html` |
| `ACAPI_AddOnObject_GetObjectGuidFromName(name, &guid)` | при отсутствии — APINULLGuid; при нескольких одинаковых именах не определено, какой GUID вернётся; для unique не подходит | `F/ACAPI_AddOnObject_GetObjectGuidFromName.html` |
| `ACAPI_AddOnObject_DeleteObject(guid)` | GUID передаётся const reference; удаление из project database; в Teamwork нужен владелец | `F/ACAPI_AddOnObject_DeleteObject.html` |

Обычные имена объектов **не уникальны**. Поэтому «найти по имени → создать» не гарантирует singleton, особенно при параллельном создании пользователями Teamwork.

`ACAPI_AddOnObject_CreateUniqueObject(name, &guid)` создаёт объект без начального content, после чего нужен Modify. Имя уникально среди unique, но может совпадать с обычным объектом; поиск — `ACAPI_AddOnObject_GetUniqueObjectGuidFromName`. В Modify unique нельзя менять имя: newObjectName = nullptr, newObjectContent != nullptr. В online Teamwork создание unique вызывает **Full Receive + Full Send**, в offline запрещено (`F/ACAPI_AddOnObject_CreateUniqueObject.html`). Это не бесплатная замена обычного реестра.

Client-only объекты не отправляются на сервер (`F/ACAPI_AddOnObject_CreateClientOnlyObject.html`). Modify/Delete обычных общих объектов требуют владения (`APIERR_NOTMINE`); резервирование может вызвать Receive Changes (`F/ACAPI_AddOnObject_ReserveObjects.html`). Нельзя предполагать, что владение viewpoint автоматически даёт владение отдельным объектом.

Общий источник persistence-контракта: `Documentation/HTML/APIDevKit/APIHTMLLibrary/Level3/AddOnObject_Manager_id.html`: байты сохраняются в project database; поддерживаются callbacks merge/конвертации старых форматов. Save/reopen и Teamwork фактически не исполнялись.

**Предложение:** выбрать единый источник истины: payload viewpoint либо отдельный объект с явной связью по GUID. Не дублировать изменяемую таблицу в двух реестрах. Версионировать сериализацию с первой версии: magic/version, размеры, кодировка, проверки границ, политика неизвестных версий. Не сериализовать сырые std::map/std::vector/GS::UniString или машинный layout структуры через memcpy.

## 7. Отбор модели и metadata

### 7.1. Доступные API25

| Задача | AC25 | Источник |
|---|---|---|
| Type, Layer, Story элемента | `ACAPI_Element_GetHeader(&header, mask)`; поля `typeID`, `layer`, `floorInd` | `H/ACAPinc.h:573–574`, `H/APIdefs_Elements.h:180–191` |
| Слои | вместо нового `ACAPI_Attribute_GetAttributesByType`: `ACAPI_Attribute_GetNum(API_LayerID, &count)`, затем `ACAPI_Attribute_Get(&attribute)` с header.typeID/index | `H/ACAPinc.h:412–414`, `F/ACAPI_Attribute_GetNum.html`, `F/ACAPI_Attribute_Get.html` |
| Этажи | вместо `ACAPI_ProjectSetting_GetStorySettings`: `ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr)`; освободить `storyInfo.data` | `H/APIdefs_Environment.h:88`, `F/APIEnv_GetStorySettingsID.html`, `S/API_StoryInfo.html` |
| Системы классификации | `ACAPI_Classification_GetClassificationSystems`, `ACAPI_Classification_GetClassificationSystemRootItems`, `ACAPI_Classification_GetClassificationItemChildren` | `H/ACAPinc.h:1465,1477,1471` |
| Классификации элемента | `ACAPI_Element_GetClassificationItems(guid, pairs)`; pairs — выход по ссылке, пары system GUID / item GUID | `H/ACAPinc.h:852`, `F/ACAPI_Element_GetClassificationItems.html` |
| Определения свойств | `ACAPI_Property_GetPropertyDefinitions(groupGuid, definitions)`; definitions по ссылке, APINULLGuid = все; `ACAPI_Property_GetPropertyGroup(group)` требует заполненного group.guid | `H/ACAPinc.h:1433–1435`, соответствующие F/*.html |
| Значение свойства | `ACAPI_Element_GetPropertyValue(elemGuid, definitionGuid, property)` — третий параметр **API_Property&**, не указатель | `H/ACAPinc.h:904–906`, `F/ACAPI_Element_GetPropertyValue.html` |
| Отображаемая строка | `ACAPI_Property_GetPropertyValueString(property, &text)` — property const reference, выход UniString* | `H/ACAPinc.h:1456–1457`, `F/ACAPI_Property_GetPropertyValueString.html` |

Оговорки:
- GetNum возвращает **максимальный индекс**, не число живых записей; удалённые атрибуты оставляют пропуски.
- Этаж над верхним в `API_StoryInfo` — виртуальный: не включать как обычный этаж пользовательского фильтра.
- Значение свойства читать только при `API_Property_HasValue`; `NotAvailable/NotEvaluated` — не пустая строка (`S/API_Property.html`).
- Display string зависит от единиц Project Preferences и раскрывает перечисления. Сравнивать числа и перечисления по типизированным значениям, строку использовать для вывода. Указанное имя функции доступно с API25; документация отмечает прежнее имя `APIAny_GetPropertyValueStringID`, но реализации других версий здесь не проверялись.
- `ACAPI_Element_GetElemList` по умолчанию использует `APIFilt_None`; `API_ZombieElemID` обозначает все типы. Один такой вызов не является доказательством обхода всех самостоятельных баз проекта (`F/ACAPI_Element_GetElemList.html`). Определить смысл «весь проект» отдельно.
- Read-only отбор по выделению не должен неявно исключать чужие/неeditable элементы. `onlyEditable` выбирать осознанно, marquee handle освобождать (`F/ACAPI_Selection_Get.html`). Сохранённое выделение как фиксированный набор GUID и «текущее выделение при каждом обновлении» — разные требования.

**SomeStuff:** значения «Монитора» брать из `PROPERTYCACHE()`, а не добавлять независимое чтение каждого свойства каждого элемента. В `.property` — только определения, значения ищутся отдельно по элементам (`AGENTS.md`, §6). Таблица API выше справочная, не предписание обойти кэш.

### 7.2. Current database

AC25: `ACAPI_Database(APIDb_GetCurrentDatabaseID, &original)` → `ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &model)` → восстановление original с проверкой ошибок. Меняется фоновая текущая база для database-dependent вызовов, а не переднее окно (`F/APIDb_ChangeCurrentDatabaseID.html`).

Исходное «GetHeader обязательно ломается в MyDraw» слишком категорично: для чтения floor-plan заголовка без переключения предусмотрена маска. Правильное имя **`APIElemMask_FromFloorplan`** (`H/APIdefs_Elements.h:227`); в `F/ACAPI_Element_GetHeader.html` осталось старое `APIElemMask_FloorPlan`.

Для `GetPropertyValue` обязательный сбой из MyDraw контрактом не подтверждён. DatabaseGuard — проектный способ контролировать контекст и восстановление, не доказательство конкретного сбоя. Данные предпочтительно собирать до начала IDF-сессии/изменения MyDraw, не перемешивать чтение модели и генерацию без проверки контекстов.

## 8. События и обновление

`ACAPI_ProjectOperation_CatchProjectEvent` из исходника заменяется в AC25 на `ACAPI_Notify_CatchProjectEvent(GSFlags, APIProjectEventHandlerProc*)` (`H/ACAPinc.h:1300–1301`). Но одного переименования недостаточно:

- События проекта и наблюдение за элементами различны. Для последних: `ACAPI_Notify_InstallElementObserver`, `ACAPI_Element_AttachObserver(guid)`; для новых элементов — `ACAPI_Notify_CatchNewElement(nullptr, handler)` (`H/ACAPinc.h:673,1314–1317`).
- `APINotifyElement_PropertyValueChange`, `APINotifyElement_ClassificationChange`, Change/Edit/Delete, Undo/Redo, BeginEvents/EndEvents существуют в AC25 (`H/APIdefs_Callback.h:216–232`).
- `APINotify_PropertyDefinitionChanged`, `APINotify_ClassificationItemChanged`, изменения visibility и units — отдельные события. **`API_AllProjectNotificationMask = 0x00000FFF` их не включает** (`H/APIdefs_Callback.h:155–172`). Маски задавать по зависимостям ведомости, не полагаться на слово All.
- Наблюдение только за уже выбранными строками пропускает элемент, который позже впервые удовлетворит фильтру. Добавления, изменения классификации/свойств и состав кандидатов нужно пересматривать.
- Уведомления не место для тяжёлого глобального пересчёта (`Documentation/HTML/APIDevKit/APIHTMLLibrary/Level3/notification_manager_id.html`). EndEvents — конец серии, не гарантия безопасного свободного UI-контекста (`S/API_ElementDBEventID.html`).

**Предложение:** уведомление помечает ведомость устаревшей; пересчёт выполняется в допустимой точке и объединяет накопившиеся изменения. Первый прототип может обновляться явно и при открытии. Полнота автопересчёта — runtime-задача, не свойство Navigator по умолчанию.

AC25 имеет `ACAPI_Command_CallFromEventLoop` — вызов **зарегистрированной команды аддона**, не произвольной lambda (`H/ACAPinc.h:1241–1246`, `F/ACAPI_Command_CallFromEventLoop.html`). Пример SDK использует этот специальный bridge из worker; это не разрешение читать/менять модель в фоне. В SomeStuff не вводить новые потоки или механизм команд без отдельного согласования архитектуры. CatchProjectEvent не является диспетчером потоков.

## 9. Палитра, форматирование и собственная модель таблицы

MyDraw-флага среди `API_PalEnabled_*` нет (`H/APIdefs_Interface.h:436–446`). Однако из этого не следует, что безопасно «не регистрировать DG::Palette». SDK предписывает уведомлять API о палитре через `ACAPI_RegisterModelessWindow`; регистрация обеспечивает специальные hide/disable/close-сообщения, Work Environment и связана с удержанием аддона в памяти (`F/ACAPI_RegisterModelessWindow.html`; `F/APIPaletteControlCallBackProc.html`). `ACAPI_UnregisterModelessWindow` документирован для завершения использования/FreeData, не как обход MyDraw.

**not verified:** исчезновение конкретной палитры в MyDraw и безопасный обход. Проверить отдельно, если редактор нужен; для первой read-only ведомости его можно не создавать. Ручные ячейки/стили — опциональный UI, не функция Navigator.

Проектные рекомендации, не контракты SDK:
- Строка может соответствовать одному элементу, группе элементов или части элемента. Нужны устойчивый логический ключ строки и связь с исходными GUID; позиция row и один elementGuid не покрывают все спецификации.
- `row * cols + col` годится для адреса ячейки текущего снимка, но не для сохранения ручных значений после сортировки/группировки/изменения cols. Для них нужны устойчивые ключи строки и колонки и политика конфликтов.
- Полный пересчёт строк при стабильном GUID ведомости допустим; incremental refresh не обязателен до замеров/требований ручных переопределений.
- Sparse-контейнер — выбор по заполненности и замерам, не автоматически лучший вариант. Формулы SUM/COUNT и ручные выражения — разные объёмы работ; вычисленные итоги не обязательно требуют хранения пользовательского expression.
- Стили, объединения, высоты строк, ширины колонок, перенос текста, повтор шапок и пагинация — самостоятельный движок. Исходников Ext-Table здесь нет, переносимость его Reflow «без изменений» не проверена.
- Имя семейства шрифта предпочтительнее индекса для переносимого формата, но наличие шрифта, fallback и метрики на Windows/macOS требуют проверки.
- UTF-8 — предлагаемый формат хранения. Не объявлять `GS::UniString(L"…")` универсальным лечением кодировки или заменой ресурсной локализации. Причина ошибок GRC и корректность конкретных литералов/настроек сборки в SomeStuff здесь не исследовались.
- Перечитывать/инвалидировать project metadata при смене проекта и изменениях атрибутов/определений. Singleton UI не гарантирует актуальности данных. При удалённом property/classification выдавать явное состояние, не молча трактовать как пустоту.
- Backup → persist → обновление геометрии — не атомарная SDK-транзакция. Откат отдельного storage может также завершиться ошибкой, особенно в Teamwork. Политика восстановления нужна отдельно; не обещать rollback одним guard.
- Selection/HUD сбрасывать или переносить по устойчивым ключам при смене модели таблицы. Global keyboard hook не нужен без отдельного требования и проверки focus owner.

## 10. Runtime-проверки перед реализационными гарантиями

1. Своя точка зрения → MyDraw → чертёж на макете → повторное обновление без дублей и потери ссылки.
2. Save/close/reopen PLN; неизвестная версия payload; merge с коллизиями имён/serial и переназначением GUID.
3. Teamwork: исходно отсутствующий root, reserve/release узла и отдельного объекта, конфликт владельцев, offline, отмена Full Send/Receive.
4. Данные из MyDraw/плана; восстановление текущей базы на каждом error-path; согласованность окна и IDF.
5. Новые/удалённые элементы, переход через границу фильтра, PropertyValueChange, ClassificationChange, metadata, Undo/Redo, Receive Changes.
6. Отсутствующее свойство/классификация, NotAvailable/NotEvaluated, нулевое число строк, длинный русский текст, объединения, перенос шапки.
7. Ошибка до и после Reset/StartDrawingData: сохранность last-good, очистка памяти, отсутствие незакрытой IDF-сессии.
8. Большие таблицы: время отбора/раскладки/рендера и размер хранения; свойства читаются согласованным путём кэша.
9. Палитра в MyDraw — только при включении редактора в scope; отдельно Windows/macOS.

Документ не утверждает реализацию этих сценариев. Текущее состояние: проверка контрактов AC25 завершена; каркас ведомостей и общий класс отрисовки таблиц лежат в `Sources/AddOn/table/` (`TablesNavigator.hpp/.cpp`, `TableRenderer.hpp/.cpp`). Следующее действие — согласовать первую ведомость, хранение и её критерии приёмки.

## 11. Implementation Log

### Цель первого реализационного шага

Создать безопасный каркас подсистемы ведомостей в Navigator/MyDraw для AC25, который:

1. Компилируется вместе с add-on.
2. Подключён к штатным фазам `RegisterInterface` и `Initialize`.
3. Создаёт корневой раздел Navigator с локализованным именем `SomeStuff Schedules` / `SomeStuff Каталоги`, но ещё не создаёт реальные ведомости.
4. Фиксирует правильные границы будущей реализации: definition → snapshot rows → единый renderer → MyDraw/IDF.
5. Не смешивается с существующими `Spec`, `Summ` и HTML Monitor.

### План реализации

- [x] Завести отдельный модуль `Sources/AddOn/table/TablesNavigator.cpp/.hpp` (изначально создан в `Sources/AddOn/`, затем перенесён).
- [x] Описать минимальные структуры: `ColumnDefinition`, `RowIdentity`, `ScheduleDefinition`, `TableCell`, `TableRow`, `TableSnapshot`.
- [x] Добавить `INavigatorCallbackInterface` skeleton со всеми pure virtual методами AC25: `OpenView`, `OpenSettings`, `ExecuteMergePostProcess`, `CreateIDFStore`, `GetElemsForDrawingCheck`, `NewItem`, `DeleteItem`, `RenameItem`, `GetIcon`.
- [x] Добавить выключатель регистрации Navigator и включить его после согласования имени root-раздела.
- [x] Реализовать `TablesNavigator::RegisterInterface()` как будущую точку `ACAPI_Register_NavigatorAddOnViewPointDataHandler()`.
- [x] Реализовать `TablesNavigator::Initialize()` как будущую установку merge/save-old/convert-new handlers и `APINavigator_RegisterCallbackInterfaceID`.
- [x] Подключить `TablesNavigator::RegisterInterface()` и `TablesNavigator::Initialize()` из `SomeStuff_Main.cpp`.
- [x] Зафиксировать имя будущего раздела Navigator: `SomeStuff Schedules` / `SomeStuff Каталоги`.
- [x] Реализовать `TablesNavigator::EnsureNavigatorRoot()` и вызвать его на `APINotify_AllInputFinished`.
- [x] Проверить runtime-сценарий появления `SomeStuff Schedules` / `SomeStuff Каталоги` в Project Map после открытия `test_25`.
- [x] Реализовать создание первого node-каталога под `SomeStuff Schedules` / `SomeStuff Каталоги` через `NewItem`.
- [x] Реализовать общий класс отрисовки таблиц `TableRenderer` (layout + render) по проверенной редакции ТЗ `Docs/TableRenderer_AC25.md`.
- [x] Реализовать IDF path: сбалансированный `StartDrawingData`/`StopDrawingData`, сохранение исходной ошибки генерации, корректные bounding box/padding/elems.
- [/] Реализовать безопасный MyDraw renderer без разрушения last-good содержимого при ошибке: окно открывается и рисует содержимое; стратегия восстановления last-good при ошибке в середине создания геометрии не реализована.
- [x] Перенести табличный код в отдельную папку `Sources/AddOn/table/`.
- [/] Отладить текст в окне ведомости: кириллица выводится чужими глифами (пользователь, 2026-09-18). Исправлены порядок «layout до сброса БД окна» и `charCode` из гарнитуры; результат проверяется пользователем.
- [/] Габарит `StopDrawingDataID`: сетка и заливки совпадают с расчётом, текстовый слой раздувает габарит. Причина not verified.
- [ ] Реализовать первую read-only ведомость: источник строк, колонки, устойчивые ключи строк, зависимости для validator.
- [ ] Выбрать и реализовать persistence: viewpoint payload или Add-On Object, версия формата, политика неизвестных версий.
- [ ] Реализовать содержимое первого каталога: колонки, фильтры, устойчивые ключи строк вместо демонстрационного прототипа.

### Сделано в коде

- `TablesNavigator.hpp` отделяет намерение ведомости (`ScheduleDefinition`) от рассчитанного снимка (`TableSnapshot`).
- Имя раздела Navigator вынесено в `ID_ADDON_STRINGS` / `ID_ADDON_STRINGS_ENG` как `SomeStuff Каталоги` / `SomeStuff Schedules`; текущий ресурс выбирается через `isEng()`. Стабильный root GUID: `4CDA3758-ECF8-4B03-BC1C-5C6F444E4F31`.
- `displayId` у root-раздела оставлен пустым: Navigator показывает `displayId + displayName`, и непустой служебный ID даёт кривую подпись вида старое имя + новое имя. Внутренний ключ остаётся в `ScheduleDefinition::internalId`.
- `EnsureNavigatorRoot()` ищет существующий root по GUID или имени, обновляет `displayName/displayId` при расхождении и создаёт root, если его нет.
- `NewItem()` создаёт leaf node-каталог под выбранным root/group или рядом с выбранным node, задаёт уникальные `SS001`, `SS002`... и локализованное имя `Новый каталог` / `New Schedule`, затем сразу открывает `OpenSettings()` для редактирования ID/Name.
- `OpenSettings()` / `RenameItem()` открывают компактный DG-диалог ID/Name и сохраняют изменения через `APINavigator_ChangeNavigatorVPItemID`; root-раздел не редактируется этим диалогом.
- `DeleteItem()` удаляет не-root элементы через `APINavigator_DeleteNavigatorVPItemID`; Teamwork-владение и конфликтные сценарии остаются runtime-задачей.
- Root создаётся в `ProjectEventHandlerProc` на `APINotify_AllInputFinished`; ошибка не прерывает открытие проекта и должна отдельно проверяться в Teamwork/runtime.
- `RowIdentity` не сводится к позиции строки или одному GUID: строка может соответствовать элементу, группе или агрегату.
- `TableSnapshot` предназначен как общий вход renderer для открытого MyDraw-окна и `CreateIDFStore`.
- `CreateIDFStore` реализован: `APIDb_StartDrawingDataID` → `TableRenderer::Draw` → `APIDb_StopDrawingDataID`; `clipBoxWidth/Height` отдаются до открытия сессии, ошибка рисования не затирается ошибкой cleanup, память IDF освобождается через `BMKillPtr`.
- `OpenView` открывает окно MyDraw и в том же вызове создаёт содержимое (как `NavigatorCallbackInterface::OpenView` в примере): `APIDb_NewWindowID` → layout считается ДО сброса БД окна → `APIDb_ResetCurrentDatabaseID` → отрисовка → `APIDb_SetZoomID` + `APIDb_RebuildCurrentDatabaseID`. Перед сбросом проверяется, что текущее окно — окно ведомости.
- `ScheduleWindowHandlerProc` обслуживает последующие `APINotifyWindow_Rebuild`/`_Activate`/`_Close`; окно переиспользуется через `APIDb_SetWindowIdID` + `APIDo_ChangeWindowID`, валидатор — `APIDb_Build/Check/Rebuild/DestroyWindowValidatorID`.
- `GetElemsForDrawingCheck` больше не вызывает рендер: проверка свежести не должна менять БД.
- Общий класс отрисовки таблиц вынесен в `Sources/AddOn/table/TableRenderer.hpp/.cpp`: вход — данные ячеек + правила форматирования; внутри — валидация входных данных, ширины столбцов (`Auto`/`Fixed`, min/max), распределение под многостолбцовые объединения, перенос текста, высоты строк, узлы сетки, подавление внутренних линий объединений, заливки диапазонами и отрисовка в порядке «заливки → сетка → текст».
- Единицы: правила в миллиметрах бумаги, layout наружу — в модельных единицах (`мм * scale / 1000`), где `scale` — то же значение, что передано в `APIDb_StartDrawingDataID`.
- Текст рисуется по одной строке на элемент (`API_TextType` не имеет вертикального выравнивания), позиция строки считается рендерером; `charCode` берётся из самой гарнитуры.
- Демонстрационное содержимое узла — `TableRenderer::SetPrototypeContent()` (таблица 3×3 с объединением и узким `Fixed`-столбцом) как временный прототип до появления строк ведомости отделки.
- Self-test `TableRenderer::RunSelfTest()` под `TESTING`, вызов — из `TablesNavigator::EnsureNavigatorRoot` один раз за сессию: логирует размеры layout, коды ошибок, габарит drawing data и послойную изоляцию (`fills`/`grid`/`texts`).
- Merge/save-old/convert-new handlers существуют как точки расширения, но при выключенной регистрации не вызываются.
- `SomeStuff_Main.cpp` вызывает модуль в штатном lifecycle; при текущем выключателе эти вызовы возвращают `NoError`.

### Намеренно не сделано

- Не реализовано содержимое каталога: реальные колонки, фильтры, строки по модели (сейчас демонстрационный прототип).
- Не пишется payload в viewpoint и не создаётся Add-On Object.
- Не добавлены иконки Navigator и новые пункты меню.
- Не заявлена поддержка Teamwork, merge, save/reopen и размещения на макете.
- Не реализована стратегия last-good: ошибка в середине создания геометрии окна оставит окно частично заполненным.

### Validation status

#### 2026-09-18 — общий класс отрисовки таблиц (#177) и видимое окно ведомости

- Проверенная редакция ТЗ на renderer: `Docs/TableRenderer_AC25.md`.
- `OpenView` узла открывает окно MyDraw (`APIDb_NewWindowID`,
  `API_NewWindowPars.userRefId = guid узла`) и **сразу в этом же вызове** создаёт содержимое —
  как `NavigatorCallbackInterface::OpenView` в `Navigator_Test`: `OpenWindow` → `APIWindowGuard`
  (сброс `APIDb_ResetCurrentDatabaseID`) → `TableRenderer::Draw` → `APIDb_SetZoomID` +
  `APIDb_RebuildCurrentDatabaseID`.
  Грабли (проверено runtime 2026-09-18): если создавать содержимое только в обработчике окна,
  окно открывается **пустым** — при создании окна `APINotifyWindow_Rebuild` обработчику не приходит
  (в `test_results.txt` отсутствовала строка `TablesNavigator schedule window render err`).
  Обработчик оставлен для последующих Rebuild/Activate.
  Перед сбросом проверяется, что текущее окно — именно окно ведомости (`APIDb_GetCurrentWindowID`,
  `typeID == APIWind_MyDrawID` и `databaseUnId.elemSetId == guid узла`), иначе сброс затронул бы
  чужое окно (например план). Приём взят из `RegenerateContentIfAppropriate` примера.
- `CreateIDFStore` реализован: `APIDb_StartDrawingDataID` → отрисовка → `APIDb_StopDrawingDataID`;
  `clipBoxWidth/Height` отдаются до открытия сессии, ошибка рисования не затирается ошибкой cleanup,
  память IDF освобождается через `BMKillPtr`.
- `GetElemsForDrawingCheck` больше не вызывает рендер (проверка свежести не должна менять БД).
- Содержимое узла — демонстрационная таблица-прототип (`TableRenderer::SetPrototypeContent`),
  пока не подключены строки ведомости отделки.
- Габарит drawing data в self-test не совпадает с расчётным (см. `Docs/TableRenderer_AC25.md` §8) —
  причина not verified, добавлена послойная изоляция.

#### 2026-09-18 — состояние на момент остановки (после переноса в `Sources/AddOn/table/`)

- Расположение кода: `Sources/AddOn/table/TablesNavigator.hpp/.cpp` и `Sources/AddOn/table/TableRenderer.hpp/.cpp`. Сборка подхватывает подпапки (`GLOB_RECURSE` в `Tools/CMakeCommon.cmake:193-197`), `Sources/AddOn` уже в include-путях (`:272`), поэтому CMake не менялся; внешний include обновлён на `table/TablesNavigator.hpp`.
- Self-test (AC25 Windows Debug, 14:21, `test_25`): layout считается верно — столбец `Fixed` ровно `0.020`, перенос длинного текста даёт строку `0.011`; `Draw err = 0`, `StopDrawingData err = 0`; `font=594 charCode=14` (`14` = `CC_Cyrillic`, `GSRoot/CH.hpp:78`).
- Послойная изоляция габарита: `fills` = 0.055552 × 0.018, `grid` = 0.060582 × 0.018 (совпадает с расчётом), `texts` = 1.528275 × 0.181667 — габарит раздувает текстовый слой. Причина **not verified**, разбор в `Docs/TableRenderer_AC25.md` §8.
- Открыто и требует проверки пользователем: кириллица в окне ведомости выводилась чужими глифами. Исправлены (а) порядок «layout считается до сброса БД окна» — иначе дефолты текста берутся из пустого окна, и (б) `charCode` теперь берётся из гарнитуры, а не из `ACAPI_Element_GetDefaults`. Что именно было причиной — не доказано, нужен повторный визуальный просмотр.
- Не проверено runtime: размещение ведомости на макете, обновление после правок модели, Teamwork, save/reopen.

SDK-контракты для использованных точек сверены с DevKit-25 и `Navigator_Test`.
Compiled: AC25 Windows Debug собран штатным runner 2026-09-18 14:21 (`build=True`). Ранее в тот же день — 12:16, 14:05, 14:09, 14:19; сборки после 14:05 включают окно MyDraw.
Runtime tested: `test_25` открывается runner-ом; `test_results.txt` содержит `TablesNavigator::EnsureNavigatorRoot : created schedules root`, `TablesNavigator::NewItem : created schedule node`, строки self-test и послойной изоляции, `TEST : end`, 483 строки `: ok`, 0 `ERROR IN TEST`. Визуально подтверждено пользователем: окно ведомости открывается; содержимое таблицы требует доработки (текст). Полная ведомость, MyDraw-редактор и размещение на макете — not verified.

## Validation

Verified: перечисленные выше контракты по установленному DevKit-25 и указанным исходникам; формат `API_TextType`/`API_HatchType`/`API_TextLinePars` и коды `CC_Cyrillic = 14`, `APIInvalidAttributeIndex = 0`, `APIFace_Plain = 0`.
Compiled: AC25 Windows Debug, последняя сборка 2026-09-18 14:21 (runner, `build=True`).
Tested: root-раздел, node-каталог и окно MyDraw создаются в `test_25`; self-test layout/render проходит с нулевыми кодами ошибок. Текст в окне (кириллица), габарит IDF текстов, размещение на макете и обновление по модели — not verified.
