# pk — Автоматизация, сброс свойств, ревизии

## Назначение модуля
Модуль `pk/` содержит утилиты для автоматического выравнивания чертежей по точкам, сброса пользовательских свойств элементов к значениям по умолчанию, и управления ревизионными маркерами (изменения, замены, нововведения, аннулирование) на чертежах и раскладках.

## Файлы модуля

| Файл | Назначение | Строки |
|------|-----------|--------|
| `AutomateFunction.cpp` | Выравнивание чертежей: поиск точек, построение 3D-сечений, выравнивание по точкам | ~1085 |
| `AutomateFunction.hpp` | Объявления namespace AutoFunc: SSectLine структура, 11 функций | ~86 |
| `ResetProperty.cpp` | Сброс свойств к значениям по умолчанию | ~427 |
| `ResetProperty.hpp` | Объявления сброса свойств: 6 функций | ~50 |
| `Revision.cpp` | Ревизионные маркеры: установка, отображение, изменение текста | ~1113 |
| `Revision.hpp` | Структуры Change/Changes/Notes, типы маркеров, hash-таблицы | ~100 |

## Ключевые типы

| Тип | Файл | Описание |
|-----|------|----------|
| `SSectLine` | AutomateFunction.hpp:12 | Структура сегментов для 3D-документов (углы, сектор, GUID, UnId, матрица) |
| `Change` | Revision.hpp:15 | Одно изменение (координаты, GUID, текст, номер, тип) |
| `Changes` | Revision.hpp:28 | Массив изменений на листе с метаданными |
| `Notes` | Revision.hpp:41 | Описание изменений на листе (связь ID макета с типом) |
| `ChangeMarkerDict` | Revision.hpp:39 | HashTable: ID изменения → Changes |
| `NoteDict` | Revision.hpp:47 | HashTable: описание → код изменения |
| `NoteByChangeDict` | Revision.hpp:49 | HashTable: ID изменения → NoteDict |
| `LayoutRevisionDict` | Revision.hpp:50 | HashTable: ID макета → UnId базы |

## Публичный API

### AutoFunc (AutomateFunction.cpp/hpp)

| Функция | Сигнатура | Строка | Назначение |
|---------|----------|--------|------------|
| `AutoFunc::GetNear` | `(const GS::Array<Sector>&, const Point2D&, UInt32&, bool&)` | 15 | Ищет отрезок, начало/конец которого возле точки; возвращает индекс и флаг isend |
| `AutoFunc::GetCuplane` | `(const SSectLine, API_3DCutPlanesInfo&, const double&)` | 34 | Устанавливает подрезку по отрезку, возвращает параметры плоскостей сечения |
| `AutoFunc::Get3DProjectionInfo` | `(API_3DProjectionInfo&, const double&, const double&)` | 36 | Устанавливает камеру перпендикулярно angz, масштаб по x/y |
| `AutoFunc::Get3DDocument` | `(API_DatabaseInfo&, const GS::UniString&, const GS::UniString&)` | 42 | Ищет 3D-документ по имени/ID; если не находит — создаёт |
| `AutoFunc::GetSectLine` | `(API_Guid&, GS::Array<SSectLine>&, GS::UniString&, const Point2D&)` | 47 | Извлекает отрезки из морфа, сортирует по удалению от startpos |
| `AutoFunc::DoSect` | `(SSectLine&, const GS::UniString&, const GS::UniString&, const double&, const double&)` | 52 | Создаёт 3D-документ для одного сечения |
| `AutoFunc::PlaceDocSect` | `(SSectLine&, API_Element&)` | 58 | Размещает на сечении элементы оформления и hotspot |
| `AutoFunc::ProfileByLine` | `()` | 63 | Строит 3D-документы вдоль морфа |
| `AutoFunc::GetDrawingsSort` | `(const GS::Array<API_Guid>&)` | 68 | Сортирует GUID чертежей по именам |
| `AutoFunc::AlignOneDrawingsByPoints` | `(const API_Guid&, API_DatabaseInfo&, API_WindowInfo&, API_Coord, API_Coord&, API_Coord&)` | 73 | Выравнивает один чертёж по точкам, возвращает новую позицию |
| `AutoFunc::AlignDrawingsByPoints` | `()` | 83 | Выравнивает все чертежи по hotspot внутри их содержимого |

### «Вызывает» / «Вызывается из» (clangd callHierarchy, AC25)

Только вызовы внутри проекта (Sources/AddOn/); вызовы SDK см. `Docs/_generated/callgraph.json`.

| Функция | Вызывает | Вызывается из |
|---------|----------|---------------|
| `GetNear` | — | `GetSectLine` (AutomateFunction.cpp:371) |
| `GetCuplane` | — | `DoSect` (AutomateFunction.cpp:413) |
| `Get3DProjectionInfo` | `msg_rep` (CommonFunction.cpp:436) ×5 | `DoSect` (AutomateFunction.cpp:428) |
| `Get3DDocument` | `msg_rep` ×3 | `DoSect` (AutomateFunction.cpp:436) |
| `GetSectLine` | `GetNear`, `GetElemTypeID`, `GetWordPoint2DTM`, `is_equal`, `msg_rep` | `ProfileByLine` (AutomateFunction.cpp:644) |
| `DoSect` | `GetCuplane`, `Get3DProjectionInfo`, `Get3DDocument`, `msg_rep` | `ProfileByLine` (AutomateFunction.cpp:672) |
| `ProfileByLine` | `GetSectLine`, `DoSect`, `PlaceDocSect`, `ClickAPoint`, `GetSelectedElements2`, `SetElemTypeID`, `UniStringToDouble`, `msg_rep` ×12 | `MenuCommandHandler` (SomeStuff_Main.cpp:461) |
| `AlignOneDrawingsByPoints` | `RestoreStartDatabaseAndWindow` ×3, `GetElemTypeID`, `msg_rep` ×5 | `AlignDrawingsByPoints` (AutomateFunction.cpp:1005) |
| `AlignDrawingsByPoints` | `AlignOneDrawingsByPoints`, `GetDrawingsSort`, `GetSelectedElements2`, `ClickAPoint`, `msg_rep` ×6 | `MenuCommandHandler` (SomeStuff_Main.cpp:464) |
| `ResetProperty` | not verified — clangd не резолвит callHierarchy на позиции определения (ResetProperty.cpp:14) | |
| `Revision::SetRevision` | not verified — clangd: No call hierarchy available (Revision.cpp:14) | |

### ResetProperty (ResetProperty.cpp/hpp)

| Функция | Сигнатура | Строка | Назначение |
|---------|----------|--------|------------|
| `ResetProperty` | `()` | 14 | Сброс пользовательских свойств с фильтром "Sync_reset" |
| `ResetPropertyElement2Defult` | `(const GS::Array<API_PropertyDefinition>&)` | 40 | Сброс во всех БД файла и настройках по умолчанию |
| `ResetElementsInDB` | `(API_DatabaseID, const GS::Array<API_PropertyDefinition>&, API_AttributeIndex, UnicGuid&)` | 34 | Сброс в одной конкретной БД проекта |
| `ResetOneElemen` | `(const API_Guid, const GS::Array<API_PropertyDefinition>&)` | 40 | Сброс одного элемента к значениям по умолчанию |
| `ResetElementsDefault` | `(const GS::Array<API_PropertyDefinition>&)` | 43 | Сброс всех подходящих элементов к значениям по умолчанию |
| `ResetOneElemenDefault` | `(API_ElemTypeID, const GS::Array<API_PropertyDefinition>&, int)` | 46 | Сброс одного типа элементов по варианту обработки |

### Revision (Revision.cpp/hpp)

| Функция | Сигнатура | Строка | Назначение |
|---------|----------|--------|------------|
| `Revision::SetRevision` | `()` | 14 | Установка ревизии (по умолчанию) на текущем макете |
| `Revision::GetScheme` | `(GS::HashTable<GS::UniString, API_Guid>&)` | - | Получение схемы ревизии (layout_note_guid) |
| `Revision::GetAllChangesMarker` | `(GS::HashTable<GS::UniString, API_Guid>&)` | - | Получение всех маркеров изменений |
| `Revision::ChangeLayoutProperty` | `()` | - | Изменение свойства раскладки |
| `Revision::CheckChanges` | `(GS::HashTable<GS::UniString, API_Guid>&, GS::HashTable<GS::UniString, GS::HashTable<...>>&)` | - | Проверка изменений |
| `Revision::GetChangesLayout` | `(GS::HashTable<GS::UniString, API_Guid>&)` | - | Получение раскладки изменений |
| `Revision::GetChangesMarker` | `(GS::HashTable<GS::UniString, API_Guid>&)` | - | Получение маркера изменений |
| `Revision::GetMarkerPos` | `(API_Coord&)` | - | Получение позиции маркера |
| `Revision::GetMarkerText` | `(GS::UniString&)` | - | Получение текста маркера |
| `Revision::ChangeMarkerTextOnLayout` | `(GS::UniString&)` | - | Изменение текста маркера на раскладке |
| `Revision::ChangeMarkerText` | `(GS::UniString&)` | - | Изменение текста маркера |

### Константы (Revision.hpp)

| Имя | Значение | Описание |
|-----|----------|----------|
| `TypeNone` | 0 | Тип не задан |
| `TypeIzm` | 1 | Изменение |
| `TypeZam` | 2 | Замена |
| `TypeNov` | 3 | Нововведение |
| `TypeAnnul` | 4 | Аннулирование |

## Зависимости (зависит от)
- `Helpers.hpp` — ParamHelpers, GetSelectedElements
- `Propertycache.hpp` — PROPERTYCACHE(), property definitions
- `CommonFunction.hpp` — msg_rep(), DBprnt(), UI-функции
- `DG.h` — диалоговые элементы
- `dialogs/SyncSettings.hpp` — настройки синхронизации (для ResetProperty)
- `Sector2DData.h` — типы секторов (для AutomateFunction)
- `api_headers/APICommon*.h` — AC_25/26/27/28 API (для ResetProperty)
- `api_headers/APIEnvir.h` — APIEnvir (для ResetProperty, Revision)

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — через MenuCommandHandler (команды меню для автоматизации, сброса, ревизий)

## Инварианты и подводные камни
- `ResetOneElemen` и `ResetOneElemenDefault` — опечатка в имени (Elemen вместо Element) — исправлять без подтверждения не нужно
- `ResetProperty` возвращает `false` на AC27+ (`#ifdef ServerMainVers_2700`) — функция неработоспособна на этой версии
- `ResetPropertyElement2Defult` содержит FIX-комментарии (ревью 2026-09-12) о ранее исправленных багах с `APIDb_ChangeCurrentDatabaseID`
- `Revision::SetRevision` сохраняет и восстанавливает вид и окно (`StoreViewSettings`) — при ошибках корректно откатывает настройки

## Справедливо для версий
- `ServerMainVers_2300` минимум для AutomateFunction (includes Helpers.hpp, MeshBody.hpp)
- `ServerMainVers_2700` условные компиляции для ResetProperty, Revision (ACAPI_View_* вместо ACAPI_Environment/ACAPI_Database)
- `ServerMainVers_2800` условия в ResetProperty (ParamValue доступ через `cIt->value`)
