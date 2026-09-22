# pk — Автоматизация, сброс свойств, ревизии

> Хеш коммита: 8f02825 (2026-09-22). Номера строк — определения в `.cpp` (1-based; проверены grep'ом и clangd callHierarchy, см. `_progress.md`, «Проблема 3»).

## Назначение модуля
Модуль `pk/` содержит утилиты для автоматического выравнивания чертежей по точкам, сброса пользовательских свойств элементов к значениям по умолчанию и управления ревизионными маркерами (изменения, замены, нововведения, аннулирование) на чертежах и раскладках. [по коду]

## Файлы модуля

| Файл | Назначение | Строки |
|------|-----------|--------|
| `AutomateFunction.cpp` | Выравнивание чертежей: поиск точек, построение 3D-сечений, выравнивание по точкам [по коду] | ~1085 |
| `AutomateFunction.hpp` | Объявления namespace AutoFunc: SSectLine структура, 11 функций [по коду] | ~86 |
| `ResetProperty.cpp` | Сброс свойств к значениям по умолчанию [по коду] | ~427 |
| `ResetProperty.hpp` | Объявления сброса свойств: 6 функций [по коду] | ~50 |
| `Revision.cpp` | Ревизионные маркеры: установка, отображение, изменение текста [по коду] | ~1113 |
| `Revision.hpp` | Структуры Change/Changes/Notes, типы маркеров, hash-таблицы [по коду] | ~100 |

## Ключевые типы

| Тип | Файл | Описание |
|-----|------|----------|
| `SSectLine` | AutomateFunction.hpp:12 | Структура сегментов для 3D-документов (углы, сектор, GUID, UnId, матрица) [из комментария] |
| `Change` | Revision.hpp:15 | Одно изменение (координаты, GUID, текст, номер, тип) [из комментария] |
| `Changes` | Revision.hpp:28 | Массив изменений на листе с метаданными [из комментария] |
| `Notes` | Revision.hpp:41 | Описание изменений на листе (связь ID макета с типом) [из комментария] |
| `ChangeMarkerDict` | Revision.hpp:39 | HashTable: ID изменения → Changes [из комментария] |
| `NoteDict` | Revision.hpp:47 | HashTable: описание → код изменения [из комментария] |
| `NoteByChangeDict` | Revision.hpp:49 | HashTable: ID изменения → NoteDict [из комментария] |
| `LayoutRevisionDict` | Revision.hpp:50 | HashTable: ID макета → UnId базы [из комментария] |

## Публичный API (сводно; нетривиальные функции — карточками в разделе «Карточки функций»)

### AutoFunc (AutomateFunction.cpp)

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `AutoFunc::GetNear` | 15 | Ищет отрезок, начало/конец которого возле точки; возвращает индекс и флаг isend [из комментария] |
| `AutoFunc::GetCuplane` | 34 | Устанавливает подрезку по отрезку, возвращает параметры плоскостей сечения [из комментария] |
| `AutoFunc::Get3DProjectionInfo` | 149 | Устанавливает камеру перпендикулярно angz, масштаб по x/y [из комментария] |
| `AutoFunc::Get3DDocument` | 208 | Ищет 3D-документ по имени/ID; если не находит — создаёт [из комментария] |
| `AutoFunc::GetSectLine` | 260 | Извлекает отрезки из морфа, сортирует по удалению от startpos [из комментария] |
| `AutoFunc::DoSect` | 406 | Создаёт 3D-документ для одного отрезка [из комментария] |
| `AutoFunc::PlaceDocSect` | 489 | Размещает на сечении элементы оформления и hotspot [из комментария] |
| `AutoFunc::ProfileByLine` | 556 | Строит 3D-документы вдоль морфа [из комментария] |
| `AutoFunc::GetDrawingsSort` | 892 | Сортирует GUID чертежей по именам [из комментария] |
| `AutoFunc::AlignOneDrawingsByPoints` | 771 | Выравнивает один чертёж по точкам, возвращает новую позицию [из комментария] |
| `AutoFunc::AlignDrawingsByPoints` | 926 | Выравнивает все чертежи по hotspot внутри их содержимого [из комментария] |

### ResetProperty (ResetProperty.cpp)

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `ResetProperty` | 14 | Сброс пользовательских свойств с фильтром "Sync_reset" [из комментария] |
| `ResetPropertyElement2Defult` | 40 | Сброс во всех БД файла и настройках по умолчанию [из комментария] |
| `ResetElementsInDB` | 134 | Сброс в одной конкретной БД проекта [из комментария] |
| `ResetOneElemen` | 252 | Сброс одного элемента к значениям по умолчанию [из комментария] |
| `ResetElementsDefault` | 293 | Сброс всех подходящих элементов к значениям по умолчанию [из комментария] |
| `ResetOneElemenDefault` | 376 | Сброс одного типа элементов по варианту обработки [из комментария] |

### Revision (Revision.cpp)

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `Revision::SetRevision` | 14 | Создаёт или обновляет ревизионные маркеры и связанные с ними свойства на листах [из комментария] |
| `Revision::GetScheme` | hpp:57 | Получает схему ревизионных маркеров по листам проекта [из комментария] |
| `Revision::GetAllChangesMarker` | hpp:60 | Собирает все маркеры изменений, связанные с листами [из комментария] |
| `Revision::ChangeLayoutProperty` | 400 | Применяет изменения к свойствам листа и связанной с ним ревизионной информации [из комментария] |
| `Revision::CheckChanges` | hpp:71 | Проверяет, есть ли нужные изменения в наборе маркеров для листа [из комментария] |
| `Revision::GetChangesLayout` | hpp:74 | Формирует список изменений для конкретного листа из собранных маркеров [из комментария] |
| `Revision::GetChangesMarker` | hpp:79 | Считывает маркеры изменений из модели и заполняет словарь [из комментария] |
| `Revision::GetMarkerPos` | hpp:82 | Возвращает позицию маркера изменения в модели [из комментария] |
| `Revision::GetMarkerText` | hpp:85 | Считывает текстовые поля маркера: примечание, номер участка, номер изменения и др. [из комментария] |
| `Revision::ChangeMarkerTextOnLayout` | 993 | Обновляет текст маркеров на листах по собранным данным изменений [из комментария] |
| `Revision::ChangeMarkerText` | 1020 | Меняет текст маркера изменения для выбранного GUID [из комментария] |

### Константы (Revision.hpp) [из комментария]

| Имя | Значение | Описание |
|-----|----------|----------|
| `TypeNone` | 0 | Тип не задан |
| `TypeIzm` | 1 | Изменение |
| `TypeZam` | 2 | Замена |
| `TypeNov` | 3 | Нововведение |
| `TypeAnnul` | 4 | Аннулирование |

## Карточки функций

### `AutoFunc::GetCuplane(const SSectLine sline, API_3DCutPlanesInfo &cutInfo, const double &depth) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:34`
- Назначение: устанавливает подрезку по отрезку, возвращает параметры плоскостей сечения. [из комментария]
- Контракт: `BNZeroMemory(&cutInfo)` внутри перед заполнением; ошибка `ACAPI_View_Get3DCuttingPlanes` (AC27+) / `APIEnv_Get3DCuttingPlanesID` (AC25-) возвращается как GSErrCode. [по коду]
- Побочные эффекты: меняет состояние 3D-подрезки активного окна через SDK. [по коду]

### `AutoFunc::Get3DDocument(API_DatabaseInfo &dbInfo, const GS::UniString &name, const GS::UniString &id) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:208`
- Назначение: ищет 3D-документ по имени и ID; если не находит — создаёт новый, возвращает его БД в `dbInfo`. [из комментария]
- Контракт: ошибки `ACAPI_Database` возвращаются как GSErrCode, логируются `msg_rep` (×3). [по коду]
- Побочные эффекты: **создаёт новый 3D-документ в проекте**, если существующий не найден; переключение текущей БД (`APIDb_ChangeCurrentDatabaseID`/`ACAPI_Database_ChangeCurrentDatabase`); аллокации строк (`snuprintf`, UniString). [по коду]
- Вызывает: `msg_rep` ×3; SDK: `ACAPI_Database` (×3), `BNZeroMemory` (×2), конструкторы UniString, `snuprintf`. [из callgraph.json]
- Вызывается из: `DoSect` (AutomateFunction.cpp:437). [из callgraph.json]

### `AutoFunc::GetSectLine(API_Guid &elemguid, GS::Array<SSectLine> &lines, GS::UniString &id, const Point2D &startpos) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:260`
- Назначение: извлекает из морфа отрезки, сортирует их по удалению от startpos, возвращает массив сегментов. [из комментария]
- Контракт: ошибка `ACAPI_Element_Get`/`GetMemo` возвращается; `ACAPI_DisposeElemMemoHdls` вызывается на всех путях (6 мест в callgraph); мемо инициализирован `= {}`. [по коду]
- Побочные эффекты: чтение морфа через `ACAPI_Element_GetMemo`; геометрические вычисления (`atan2`, `GetLength`, MeshBody-геттеры); без записи в проект. [по коду]
- Вызывает: `GetNear`, `GetElemTypeID`, `GetWordPoint2DTM`, `is_equal`, `msg_rep` ×4. [из callgraph.json]
- Вызывается из: `ProfileByLine` (AutomateFunction.cpp:645). [из callgraph.json]

### `AutoFunc::DoSect(SSectLine &sline, const GS::UniString &name, const GS::UniString &id, const double &koeff, const double &depth) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:406`
- Назначение: создаёт 3D-документ для одного отрезка. [из комментария]
- Контракт: при ошибке любого подэтапа (подрезка/камера/БД) — ранний выход через `msg_rep` и `BMKillHandle`; коды SDK возвращаются наружу. [по коду]
- Побочные эффекты: **создаёт 3D-документ** (`Get3DDocument`); меняет 3D-подрезку и камеру (`GetCuplane`, `Get3DProjectionInfo` → `ACAPI_Environment`); `BMAllocateHandle`/`BMKillHandle` (8 мест). [по коду]
- Вызывает: `GetCuplane` (:414), `Get3DProjectionInfo` (:429), `Get3DDocument` (:437), `msg_rep` ×5. [из callgraph.json]
- Вызывается из: `ProfileByLine` (AutomateFunction.cpp:673). [из callgraph.json]

### `AutoFunc::ProfileByLine()`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:556`
- Назначение: строит 3D-документы вдоль морфа. [из комментария]
- Контракт: вход — точка через `ClickAPoint`; ошибки этапов — `msg_rep` (×12); результат собирается в `lines`/созданные документы. [по коду]
- Побочные эффекты: **создаёт 3D-документы** (DoSect) и размещает оформление (PlaceDocSect); читает выделение (`GetSelectedElements2`); меняет типы элементов (`SetElemTypeID`); `ACAPI_CallCommand`, `ACAPI_Automate` (×2), `ACAPI_Database` (×5), `ACAPI_Environment` (×4), `ACAPI_Element_GetDefaults`, `BMKillHandle`. [по коду]
- Вызывает: `GetSectLine`, `DoSect`, `PlaceDocSect`, `ClickAPoint`, `GetSelectedElements2`, `SetElemTypeID`, `UniStringToDouble`, `msg_rep` ×12. [из callgraph.json]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:461, команда Auto3D). [из callgraph.json]

### `AutoFunc::AlignOneDrawingsByPoints(const API_Guid &elemguid, API_DatabaseInfo &databasestart, API_WindowInfo &windowstart, const API_Coord &zeropos, API_Coord &startpos, API_Coord &drawingpos) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:771`
- Назначение: выравнивает один чертёж по точкам, возвращает его новую позицию. [из комментария]
- Контракт: `RestoreStartDatabaseAndWindow` вызывается на всех 3 путях выхода для возврата в исходную БД/окно; ошибки — `msg_rep` (×5). [по коду]
- Побочные эффекты: **переключает текущую БД/окно** (`ACAPI_Database` ×2, RestoreStartDatabaseAndWindow ×3); читает элементы (`ACAPI_Element_Get`, `ACAPI_Element_GetElemList`, `GetElemTypeID`); открывает окно через `ACAPI_Automate`; **меняет позицию чертежа**. [по коду]
- Вызывает: `RestoreStartDatabaseAndWindow` (:750, ×3), `GetElemTypeID`, `msg_rep` ×5. [из callgraph.json]
- Вызывается из: `AlignDrawingsByPoints` (AutomateFunction.cpp:1006). [из callgraph.json]

### `AutoFunc::AlignDrawingsByPoints()`
- Расположение: `Sources/AddOn/pk/AutomateFunction.cpp:926`
- Назначение: выравнивает чертежи по hotspot, найденным внутри их содержимого. [из комментария]
- Контракт: изменение позиций — внутри `ACAPI_CallUndoableCommand` (один undo-регион на все чертежи); пустое выделение — ранний выход; ошибки — `msg_rep` (×6). [по коду]
- Побочные эффекты: **меняет позиции чертежей на раскладке** (`ACAPI_Element_Change`); читает выделение (`GetSelectedElements2`), ввод точки (`ClickAPoint`), `ACAPI_Automate` (×2), `ACAPI_Database` (×9), `ACAPI_Element_Get`. [по коду]
- Вызывает: `AlignOneDrawingsByPoints` (:1006), `GetDrawingsSort` (:987), `GetSelectedElements2`, `ClickAPoint`, `msg_rep` ×6. [из callgraph.json]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:464, команда AutoLay). [из callgraph.json]

### `ResetProperty() -> bool`
- Расположение: `Sources/AddOn/pk/ResetProperty.cpp:14`
- Назначение: выполняет сброс пользовательских свойств в режиме add-on. [из комментария]
- Контракт: **возвращает `false` на AC27+** (`#ifdef ServerMainVers_2700`, строки 15-17) — на этой версии функция неработоспособна (актуальный баг, не задел на будущее); требует прочитанного кэша определений (`isPropertyDefinitionRead`); false — если определений с `Sync_reset` в описаниях нет. [из комментария + по коду]
- Побочные эффекты: делегирует `ResetPropertyElement2Defult` (**меняет свойства элементов проекта**); читает `PROPERTYCACHE().property`. [по коду]
- Вызывает: не проверено — clangd не резолвит callHierarchy на позиции определения (ResetProperty.cpp:14, попытки в двух столбцах). [из callgraph.json]
- Вызывается из: не проверено (см. выше). [из callgraph.json]

### `ResetPropertyElement2Defult(const GS::Array<API_PropertyDefinition> &definitions_to_reset) -> UInt32`
- Расположение: `Sources/AddOn/pk/ResetProperty.cpp:40`
- Назначение: сбрасывает свойства элементов к значениям по умолчанию во всех базах данных файла. [из комментария]
- Контракт: при ошибке чтения текущей БД — ранний выход с `msg_rep` и 0 обработанных («без валидной origDB вернуть исходную БД невозможно», FIX 2026-09-12); возвращает количество обработанных элементов. [из комментария + по коду]
- Побочные эффекты: **переключает текущую БД и комбинацию слоёв** (`ACAPI_Database_GetCurrentDatabase`, `GetCurrLayerComb`) и через вызовы `ResetElementsInDB` (текущая/фасады/…) **меняет свойства элементов**. [по коду]
- Вызывает: `ResetElementsDefault`, `ResetElementsInDB` (несколько БД), `msg_rep`. [по коду]
- Вызывается из: `ResetProperty` (ResetProperty.cpp:34). [по коду]

### `ResetElementsInDB(const API_DatabaseID commandID, const GS::Array<API_PropertyDefinition> &definitions_to_reset, API_AttributeIndex layerCombIndex, UnicGuid &doneelemguid) -> UInt32`
- Расположение: `Sources/AddOn/pk/ResetProperty.cpp:134`
- Назначение: сбрасывает свойства элементов в одной конкретной базе данных проекта. [из комментария]
- Контракт: для текущей БД переключение БД не требуется, меняется только комбинация слоёв (если положительная); ошибки — `msg_rep`; возвращает количество сброшенных. [по коду]
- Побочные эффекты: **меняет комбинацию слоёв** (`ACAPI_Navigator_ChangeCurrLayerComb` / `APIEnv_ChangeCurrLayerCombID`); перечисляет все элементы (`ACAPI_Element_GetElemList`) и сбрасывает каждый через `ResetOneElemen`; дедупликация через `doneelemguid`. [по коду]
- Вызывает: `ResetOneElemen`, `msg_rep`; SDK: `ACAPI_Element_GetElemList`, ChangeCurrLayerComb. [по коду]
- Вызывается из: `ResetPropertyElement2Defult` (несколько вызовов, по БД). [по коду]

### `ResetOneElemen(const API_Guid elemGuid, const GS::Array<API_PropertyDefinition> &definitions_to_reset) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/ResetProperty.cpp:252`
- Назначение: сбрасывает свойства одного элемента к значениям по умолчанию. [из комментария]
- Контракт: если сбрасывать нечего — `APIERR_MISSINGCODE` (не ошибка SDK); ошибки Get/SetProperties — `msg_rep` и возврат кода. [по коду]
- Побочные эффекты: **меняет свойства элемента**: `ACAPI_Element_SetProperties` внутри `ACAPI_CallUndoableCommand("Reset properties", …)` — запись только в undoable-области (иначе SDK отклоняет: APIERR_REFUSEDCMD/APIERR_UNDOEMPTY, FIX 2026-09-12); сбрасываются только не-default значения. [из комментария + по коду]
- Вызывает: SDK: `ACAPI_Element_GetPropertyValues`, `ACAPI_Element_SetProperties`, `ACAPI_CallUndoableCommand`; `msg_rep`. [по коду]
- Вызывается из: `ResetElementsInDB` (в цикле по элементам). [по коду]

### `ResetElementsDefault(const GS::Array<API_PropertyDefinition> &definitions_to_reset) -> UInt32`
- Расположение: `Sources/AddOn/pk/ResetProperty.cpp:293`
- Назначение: сбрасывает свойства всех подходящих элементов к значениям по умолчанию (в настройках по умолчанию). [из комментария]
- Контракт: возвращает сумму успешных сбросов; ошибки делегируются `ResetOneElemenDefault`. [по коду]
- Побочные эффекты: **меняет настройки по умолчанию** нескольких типов (в т.ч. MEP-объекты через `ResetOneElemenDefault` с фиксированными variationID). [по коду]
- Вызывает: `ResetOneElemenDefault` (многократно, variationID 1146245920 и др.). [по коду]
- Вызывается из: `ResetPropertyElement2Defult`. [по коду]

### `ResetOneElemenDefault(API_ElemTypeID typeId, const GS::Array<API_PropertyDefinition> &definitions_to_reset, int variationID) -> GSErrCode`
- Расположение: `Sources/AddOn/pk/ResetProperty.cpp:376`
- Назначение: сбрасывает свойства одного типа элементов в зависимости от варианта обработки. [из комментария]
- Контракт: сбрасывать нечего — `APIERR_MISSINGCODE`; ветки AC26+ / AC25- по вызовам `GetPropertyValuesOfDefaultElem`/`SetPropertiesOfDefaultElem`. [по коду]
- Побочные эффекты: **меняет свойства по умолчанию типа**: `ACAPI_Element_SetPropertiesOfDefaultElem` внутри `ACAPI_CallUndoableCommand` (FIX 2026-09-12: передаётся `properties_to_reset`, а не все properties — иначе AC26+ ветка перезаписывала и несбрасываемые, рассинхрон с AC25). [из комментария + по коду]
- Вызывает: SDK: `ACAPI_Element_GetPropertyValuesOfDefaultElem`, `ACAPI_Element_SetPropertiesOfDefaultElem`, `ACAPI_CallUndoableCommand`; `msg_rep`. [по коду]
- Вызывается из: `ResetElementsDefault` (с фиксированными variationID). [по коду]

### `Revision::SetRevision()`
- Расположение: `Sources/AddOn/pk/Revision.cpp:14`
- Назначение: создаёт или обновляет ревизионные маркеры и связанные с ними свойства на листах. [из комментария]
- Контракт: если свойство-правило не найдено (`GetScheme` false) — `msg_rep` и ранний выход; при ошибке сохранения вида настройки снимаются перед возвратом (2 защитных пути); ошибки получения БД/окна — ранний выход с `msg_rep`. [по коду]
- Побочные эффекты: **сохраняет/восстанавливает настройки вида и окно** (`ACAPI_View_StoreViewSettings` / `APIDb_StoreViewSettingsID`), **переключает БД и окно** (`ChangeCurrentDatabase`/`ChangeWindow`); изменяет маркеры и свойства листов через `ChangeLayoutProperty`/`ChangeMarkerTextOnLayout`. [по коду]
- Вызывает: не проверено — clangd: No call hierarchy (Revision.cpp:14, попытки в двух столбцах). [из callgraph.json]
- Вызывается из: не проверено (см. выше). [из callgraph.json]

### `Revision::ChangeLayoutProperty(ChangeMarkerDict &changes, GS::HashTable<GS::UniString, API_Guid> &layout_note_guid, API_DatabaseUnId &databaseUnId, GS::UniString &layoutId, LayoutRevisionDict &layoutRVI, NoteByChangeDict &allchanges) -> bool`
- Расположение: `Sources/AddOn/pk/Revision.cpp:400`
- Назначение: применяет изменения к свойствам листа и связанной с ним ревизионной информации. [из комментария]
- Контракт: ошибки `APIEnv_GetLayoutSetsID` и `layoutInfo.customData == nullptr` — `msg_rep` и обработка как «не найдено»; ошибки поиска свойства логируются с `show=false`. [по коду]
- Побочные эффекты: **меняет свойства листа** (customData листа); работает с LayoutSets; FIX-комментарий: без customData — разыменование nullptr (образец). [из комментария + по коду]
- Вызывает: SDK: `ACAPI_Environment`/`ACAPI_ProjectSetting_*` (LayoutSets); `msg_rep` (многократно). [по коду]
- Вызывается из: `SetRevision` (Revision.cpp:219). [по коду]

### `Revision::ChangeMarkerTextOnLayout(ChangeMarkerDict &changes)`
- Расположение: `Sources/AddOn/pk/Revision.cpp:993`
- Назначение: обновляет текст маркеров на листах по собранным данным изменений. [из комментария]
- Контракт: весь обход внутри одного `ACAPI_CallUndoableCommand` (undo-строка из ресурсов); нумерация участков (`nuch`) пересчитывается только для `TypeIzm`, остальные сбрасываются в "". [по коду]
- Побочные эффекты: **меняет текст ревизионных маркеров** (`ChangeMarkerText` на каждый маркер); модифицирует входной `changes` по ссылке (FIX 2026-09-12: итерация по ссылке, чтобы изменения попали в исходную таблицу). [из комментария + по коду]
- Вызывает: `ChangeMarkerText` (в цикле), `isEng`, `RSGetIndString`. [по коду]
- Вызывается из: `SetRevision` (Revision.cpp:218). [по коду]

### `Revision::ChangeMarkerText(API_Guid &markerguid, GS::UniString &nuch, GS::UniString &nizm)`
- Расположение: `Sources/AddOn/pk/Revision.cpp:1020`
- Назначение: меняет текст маркера изменения для выбранного GUID. [из комментария]
- Контракт: `APINULLGuid` — тихий выход; маркер без прав доступа / не редактируемый / не в моём рабочем пространстве (Teamwork) — `msg_rep` и выход; ошибка `ACAPI_Element_Get`/`GetMemo` — выход; `memo.params == nullptr` — Dispose и выход (FIX 2026-09-12). [из комментария + по коду]
- Побочные эффекты: **изменяет параметр маркера** через `ACAPI_Element_Change`; читает `ACAPI_Element_Get`, `ACAPI_Element_GetMemo` (мемо `= {}`, Dispose после); фильтры `APIFilt_HasAccessRight`/`IsEditable`/`InMyWorkspace`. [по коду]
- Вызывает: SDK: `ACAPI_Element_Get`, `ACAPI_Element_GetMemo`, `ACAPI_Element_Change`, `ACAPI_Element_Filter` (×3), `ACAPI_Teamwork_HasConnection`/`ACAPI_TeamworkControl_HasConnection`; `msg_rep` ×4. [по коду]
- Вызывается из: `ChangeMarkerTextOnLayout` (Revision.cpp:1016, в цикле). [по коду]

## Зависимости (зависит от)
- `Helpers.hpp` — ParamHelpers, GetSelectedElements [по include]
- `Propertycache.hpp` — PROPERTYCACHE(), property definitions [по include]
- `CommonFunction.hpp` — msg_rep(), ClickAPoint, GetElemTypeID, UniStringToDouble, is_equal, GetWordPoint2DTM [из callgraph.json]
- `DG.h` — диалоговые элементы [по include]
- `dialogs/SyncSettings.hpp` — настройки синхронизации (для ResetProperty) [по include]
- `Sector2DData.h` — типы секторов (для AutomateFunction) [по include]
- `api_headers/APICommon*.h` — AC_25/26/27/28 API (для ResetProperty) [по include]
- `api_headers/APIEnvir.h` — APIEnvir (для ResetProperty, Revision) [по include]

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — через MenuCommandHandler (Auto3D → ProfileByLine :461, AutoLay → AlignDrawingsByPoints :464; команды сброса и ревизий) [из callgraph.json]

## Инварианты и подводные камни
- `ResetOneElemen` и `ResetOneElemenDefault` — опечатка в имени (Elemen вместо Element) [по коду]
- `ResetPropertyElement2Defult` содержит FIX-комментарии (ревью 2026-09-12) о ранее исправленных багах с `APIDb_ChangeCurrentDatabaseID` [из комментария]
- `Revision::SetRevision` сохраняет и восстанавливает вид и окно (`StoreViewSettings`) — при ошибках корректно откатывает настройки [по коду]
- Запись свойств в ResetProperty-семействе — только внутри `ACAPI_CallUndoableCommand` (иначе SDK отклоняет модификацию БД) [из комментария, ResetProperty.cpp:281-285]

## Справедливо для версий
- `ServerMainVers_2300` минимум для AutomateFunction (includes Helpers.hpp, MeshBody.hpp) [по include]
- `ServerMainVers_2700` условные компиляции для ResetProperty, Revision (ACAPI_View_* вместо ACAPI_Environment/ACAPI_Database) [по коду]
- `ServerMainVers_2800` условия в ResetProperty (ParamValue доступ через `cIt->value`) и Revision (итерация HashTable) [по коду]
