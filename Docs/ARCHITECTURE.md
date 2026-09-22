# Архитектура AddOn_SomeStuff

## Обзор

ArchiCAD C++ Add-On для работы со стройдокументацией. Поддерживает ArchiCAD 22–29 (Win + macOS). Код расположен в `Sources/AddOn/`.

## Структура каталогов

```
Sources/AddOn/
├── SomeStuff_Main.cpp/hpp   — Точка входа, интерфейс, меню, observer'ы
├── Helpers.*                — Ядро: свойства, выбор, параметры
├── Propertycache.*          — Кэш свойств/классификации/атрибутов
├── Sync.*                   — Синхронизация свойств, мониторинг
├── Summ.*                   — Суммирование свойств
├── Spec.*                   — Правила спецификаций
├── ReNum.*                  — Перенумерация
├── Revision.*               — Ревизионные маркеры
├── Dimensions.*             — Размеры (см. AGENTS.md §16: pen_original не менять)
├── Roombook.*               — Спецификация отделки
├── ClassificationFunction.* — Авто-классификация
├── ResetProperty.*          — Сброс свойств (см. AGENTS.md §6: undo-регионы)
├── AutomateFunction.*       — Автоматизация/выравнивание (pk/)
├── MEPv1.*                  — MEP
├── CommonFunction.*         — Утилиты
├── dialogs/                 — DG-диалоги, HTML-интерфейс
│   ├── BrowserPalette.*     — Браузер-палитра (HTML из RFIX/HTML/)
│   ├── CommandHelpers.*
│   ├── DG4rule.*
│   └── SyncSettings.*       — Настройки синхронизации (SyncSettings.dat)
├── spec/                    — Движок спецификаций
├── table/                   — Рендерер таблиц, навигатор
├── pk/                      — Автоматизация, сброс свойств, ревизии
├── api_headers/             — Заголовки ArchiCAD API
└── third_party/             — qrcodegen (встроенная библиотека)
```

## Слои архитектуры

```
┌─────────────────────────────────────────────┐
│              ArchiCAD (AC-API)              │
├─────────────────────────────────────────────┤
│            api_headers/                     │
│  APIEnvir, APICommon*, ACAPI_* wrappers     │
├─────────────────────────────────────────────┤
│          Core Helpers / Propertycache       │
│  ParamHelpers, GetSelectedElements,         │
│  PROPERTYCACHE(), ReadSyncSettingsFromFile  │
├─────────────────────────────────────────────┤
│           Application Logic                 │
│  Sync, Summ, Spec, ReNum, ResetProperty,    │
│  Revision, AutomateFunction, MEPv1,         │
│  ClassificationFunction, Roombook,         │
│  Dimensions, SomeStuff_Main                 │
├─────────────────────────────────────────────┤
│              UI Layer                       │
│  Dialogs (DG::Palette, BrowserPalette,      │
│  SyncSettings), HTML Interface              │
├─────────────────────────────────────────────┤
│           Utilities / Third-party           │
│  CommonFunction, qrcodegen                  │
└─────────────────────────────────────────────┘
```

## Граф зависимостей модулей

Строится из `#include` и `callgraph.json`

```mermaid
graph TD
    Main[SomeStuff_Main] --> Sync
    Main --> Summ
    Main --> Spec
    Main --> ReNum
    Main --> Helpers
    Main --> BP[dialogs/BrowserPalette]
    Sync --> Helpers
    Sync --> PC[Propertycache]
    Sync --> SS[dialogs/SyncSettings]
    Summ --> Helpers
    Spec --> Helpers
    Spec --> PC
    ReNum --> Helpers
    Helpers --> PC
    Helpers --> CF[CommonFunction]
    Helpers --> CFu[ClassificationFunction]
    Helpers --> SL[spec/Spec_libpart]
    Helpers --> SS
    PC --> Helpers
    PC --> CH[dialogs/CommandHelpers]
    PC --> CFu
    MEPv1 --> Helpers
    MEPv1 --> PC
    Dim[Dimensions] --> Helpers
    Dim --> SS
    RB[Roombook] --> Helpers
    RB --> CF
    PK[pk: AutomateFunction/ResetProperty/Revision] --> Helpers
    PK --> CF
    TR[table/TableRenderer] --> CF
    TN[table/TablesNavigator]
    BP --> Sync
    BP --> Helpers
    CH[dialogs/CommandHelpers] --> Helpers
    SS --> CF
```

## Ключевые паттерны и правила

### Настройки (Settings)

- Настройки хранятся в `…/GRAPHISOFT/SomeStuff/SyncSettings.dat` (локально)
- `ACAPI_SetPreferences` / `Preferences_Save` — НЕ ИСПОЛЬЗОВАТЬ (пишет в каждый проект, ломает Teamwork)
- `ReadSyncSettingsFromFile` отвергает файл с mismatched `PreferencesVersion` — при добавлении настроек увеличить версию

### Кэш свойств (Propertycache)

- Ключи кэша всегда в нижнем регистре (`ToLowerCase` + `BRACEEND`)
- Значения свойств берутся только из `PROPERTYCACHE()`, никогда из `ACAPI_Property_GetPropertyValue`
- `property` — определения; значения ищутся отдельно по элементу

### JS Bridge

- Bridge = inline функции через `RegisterACAPIJavaScriptObject`
- JSON-команды были удалены (b7a996b) — не reintroduce
- Парсить аргументы через `DynamicCast<JSValue>` — `DynamicCast<JSArray>` крашит ArchiCAD

### Undo Regions

- Один undo-region на пользовательское действие, никогда на элемент (сотни undo-шагов иначе)
- `ResetProperty` — см. AGENTS.md §6

### Highlight Elements

- `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID` триггерят `SelectionChangeHandler` → сброс палитры
- Фикс: `static suppressSelectionRefresh` на время подсветки
- `SetElementHighlight` — только AC26+/27+; AC25 использовать `ACAPI_Interface` напрямую

### Palette Window Resize

- Docked palette width принадлежит dock manager — `SetClientWidth` игнорируется при доке
- Рецепт: `UnDock()` → `SetClientWidth()` → `Dock()`
- `GetMinClientWidth()` == original width → `SetMinClientWidth` релаксировать ПЕРЕД уменьшением

## Сборка

- `Tools/BuildAddOn.py` — сборка и конфигурация
- Win: `python Tools\BuildAddOn.py -c config.json -v <version>`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version>`
- LSP: добавить `--lsp` к команде
- Тесты: `Sources/AddOn/TestFunc.cpp/.hpp` (активны под `TESTING`)

## Версии

- Поддержка: AC 22–29 (определять по задаче через build config, #if блоки или тесты)
- Условные компиляции: `ServerMainVers_2300`, `ServerMainVers_2700`, `ServerMainVers_2800`
- `#ifdef AC_25/26/27/28` — для APICommon заголовков в ResetProperty

## Ключевые сценарии (согласованы 2026-09-22; sequence-диаграммы построены из callgraph.json/body_scan.json)

Sequence-диаграммы: `Docs/SEQUENCES.md` (набор согласован 2026-09-22).

Список для sequence-диаграмм (фаза 4, рисовать после согласования):

1. Полная синхронизация: меню → SyncAndMonAll → SyncByType → SyncElement → SyncData → запись свойств
2. Мониторинг изменений: ElementEventHandlerProc → SyncData (throttling через IsElementThrottled)
3. Спецификация: SpecAll → GetRuleFromDescription → GetElementsForRule → PlaceElements
4. Перенумерация: ReNumSelected → RenumDG (выбор правил) → ReNumOneRule
5. Суммирование: SumSelected → Sum_GetElement → Sum_OneRule → запись в свойство/проект
6. Округление размеров: DimRoundAll → DimAutoRound → DimParse (правила из PROPERTYCACHE)
7. Палитра: ShowOrHideBrowserPalette → HTML → JS bridge → ManualGetSelection/HighlightElements
8. Ревизии: SetRevision → обход маркеров → ChangeMarkerText
9. Выравнивание чертежей: AlignDrawingsByPoints → AlignOneDrawingsByPoints
10. Сброс свойств: ResetProperty → ResetPropertyElement2Defult → обход БД


### 1. Полная синхронизация (меню)
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant S as SyncAndMonAll
    participant RP as ResetProperty
    participant T as SyncByType
    participant E as SyncElement
    participant D as SyncData
    participant W as Helpers Write
    U->>M: Sync All (:404)
    M->>S: SyncAndMonAll (Sync.cpp:170)
    S->>RP: ResetProperty (:174)
    RP-->>S: true → ранний выход / false
    S->>T: SyncByType ×5 wallS/widoS/objS/cwallS (:191-229)
    T->>E: SyncElement (:352)
    E->>D: SyncData (:411/425)
    D->>D: ParseSyncString → WriteDict → SyncCalcRule
    S->>W: ElementsWrite (:265) / WriteInfo (:291) / SyncArray (:296)
```

### 2. Мониторинг изменений (событие элемента)
```mermaid
sequenceDiagram
    participant AC as ArchiCAD (notify)
    participant E as ElementEventHandlerProc
    participant DC as PROPERTYCACHE
    participant DR as DimRoundAll
    participant S as Sync-цепочка
    AC->>E: событие
    E->>DC: Update (:389)
    E->>DR: DimRoundAll (:165)
    E->>S: SyncAndMonAll / SyncSelected / MonAll / AttachObserver / Do_ElementMonitor
```

### 3. Спецификация
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant A as SpecAll
    participant R as GetRuleFromDefaultElem
    participant F as SpecFilter
    participant AR as SpecArray
    participant PE as PlaceElements
    U->>M: Spec (:441)
    M->>A: SpecAll (Spec.cpp:140)
    A->>R: GetRuleFromDefaultElem (:165)
    A->>F: SpecFilter (:157/176)
    A->>AR: SpecArray (:185)
    AR->>PE: PlaceElements (Spec.cpp:950)
    PE->>PE: GetElementForPlace (:2461) → ACAPI_Element_Create (undoable :2449)
    PE->>PE: UnhideUnlockElementLayer (:2473)
```

### 4. Перенумерация
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant S as ReNumSelected
    participant G as GetRenumElements
    participant R as ReNumOneRule
    participant ES as ElementsSeparation
    participant W as ElementsWrite
    U->>M: Renum (:427)
    M->>S: ReNumSelected (ReNum.cpp:56)
    S->>S: RenumDG — выбор правил
    S->>G: GetRenumElements (:87)
    G->>R: ReNumOneRule (:345)
    R->>ES: ElementsSeparation (:674)
    R->>R: GetMostFrequentPos / GetPos / FormatToMax / SetToMax
    S->>W: ElementsWrite (:134/139, undoable :133)
    S->>S: SyncArray (:162)
```

### 5. Суммирование
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant S as SumSelected
    participant G as GetSumValuesOfElements
    participant R as Sum_OneRule
    participant W as Helpers WriteInfo
    U->>M: Sum (:433)
    M->>S: SumSelected (Summ.cpp:29)
    S->>G: GetSumValuesOfElements (:44)
    G->>R: Sum_OneRule (:213)
    R->>R: NUM/TEXT/MIN/MAX по sum_type
    S->>W: ElementsWrite (:70) / WriteInfo (:95)
```

### 6. Округление размеров
```mermaid
sequenceDiagram
    participant AC as ArchiCAD (3 обработчика)
    participant D as DimRoundAll
    participant T as DimRoundByType
    participant A as DimAutoRound
    participant P as DimParse
    AC->>D: меню (:489) / ElementEvent (:165) / ProjectEvent (:128)
    D->>D: ACAPI_CallUndoableCommand (:433)
    D->>T: DimRoundByType (:434/438)
    T->>A: DimAutoRound (:458)
    A->>P: DimParse (:188)
    P-->>A: custom_txt + flag_change/highlight
    A->>A: ACAPI_Element_Change (:266)
    Note over A: GetMemo + Dispose на 7 путях
```

### 7. Палитра (BrowserPalette)
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant B as BrowserPalette
    participant J as HTML (JS bridge)
    U->>M: Palette (:469)
    M->>B: ShowOrHideBrowserPalette (:37)
    B->>B: Show (:128) — reloadContent или без
    B->>J: UpdateSelectionInfoInUI (:176)
    J-->>B: ManualGetSelection (:1632)
    AC-->>B: SelectionChangeHandler (:1643), guard suppressSelectionRefresh
```

### 8. Ревизии
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant S as SetRevision
    participant G as GetAllChangesMarker
    participant C as ChangeLayoutProperty
    participant T as ChangeMarkerTextOnLayout
    participant X as ChangeMarkerText
    U->>M: SetRevision (:449)
    M->>S: SetRevision (Revision.cpp:14)
    S->>S: GetScheme — свойство-правило найдено?
    S->>S: StoreViewSettings (сохранить вид)
    S->>G: GetAllChangesMarker
    G->>C: ChangeLayoutProperty (:220)
    G->>T: ChangeMarkerTextOnLayout (:219)
    T->>T: ACAPI_CallUndoableCommand
    T->>X: ChangeMarkerText (:1017) — на каждый маркер
    X->>X: Фильтры HasAccessRight/Editable/InMyWorkspace → ACAPI_Element_Change
    S->>S: восстановить БД/окно/вид
```

### 9. Выравнивание чертежей
```mermaid
sequenceDiagram
    participant U as Пользователь
    participant M as MenuCommandHandler
    participant A as AlignDrawingsByPoints
    participant O as AlignOneDrawingsByPoints
    participant R as RestoreStartDatabaseAndWindow
    U->>M: AutoLay (:465)
    M->>A: AlignDrawingsByPoints (AutomateFunction.cpp:926)
    A->>A: GetSelectedElements2 / ClickAPoint / GetDrawingsSort (:987)
    A->>O: AlignOneDrawingsByPoints (:1006, undoable :1048)
    O->>O: ACAPI_Element_Get / GetElemList
    O->>R: RestoreStartDatabaseAndWindow ×3 (:806/811/846)
    O-->>A: drawingpos
    A->>A: ACAPI_Element_Change (:1061)
```

### 10. Сброс свойств
```mermaid
sequenceDiagram
    participant U as Пользователь / Sync
    participant R as ResetProperty
    participant P as ResetPropertyElement2Defult
    participant D as ResetElementsDefault
    participant I as ResetElementsInDB
    participant O as ResetOneElemen
    Note over R: AC27+: return false (баг)
    U->>R: ResetProperty (ResetProperty.cpp:14; из SyncAndMonAll :174)
    R->>P: ResetPropertyElement2Defult (:34)
    P->>D: ResetElementsDefault
    D->>D: ResetOneElemenDefault ×N (variationID)
    P->>I: ResetElementsInDB — по БД (текущая/фасады/…)
    I->>O: ResetOneElemen — на каждый элемент
    O->>O: ACAPI_Element_SetProperties (undoable :286)
    Note over O: только не-default; нечего сбрасывать → APIERR_MISSINGCODE
```
