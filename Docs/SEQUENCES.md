# SEQUENCES — Ключевые сценарии

> Хеш коммита: после 12874ab (2026-09-22). Набор из 10 сценариев согласован пользователем. Все сообщения — из `Docs/_generated/callgraph.json` и `body_scan.json` (координаты в тексте — 1-based).

## 1. Полная синхронизация

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню)
    participant Main as MenuCommandHandler
    participant SS as SyncSettings
    participant PC as PROPERTYCACHE
    participant Sync as SyncAndMonAll
    participant RP as ResetProperty (pk)
    participant SBT as SyncByType
    participant SE as SyncElement
    participant SD as SyncData
    participant H as Helpers (ElementsWrite)

    UI->>Main: команда Sync All (SomeStuff_Main.cpp:405)
    Main->>SS: LoadSyncSettingsFromPreferences (:372)
    Main->>PC: Update (:389)
    Main->>Sync: SyncAndMonAll (:405)
    Sync->>RP: ResetProperty (:174)
    RP-->>Sync: true → ранний выход (сброс выполнен)
    Sync->>SBT: SyncByType ×5 (wallS/widoS/objS/cwallS, :191-229)
    SBT->>SBT: ACAPI_Element_GetElemList (:321)
    SBT->>SE: SyncElement (:352)
    SE->>SD: SyncData (:411/425)
    SD->>SD: ParseSyncString (:729) → SyncCalcRule (:754)
    SD->>H: ElementsWrite (запись свойств)
    Note over H: запись в undoable-области
```

## 2. Мониторинг изменений элементов

```mermaid
sequenceDiagram
    participant AC as ArchiCAD (notify)
    participant EEH as ElementEventHandlerProc
    participant Sync as Sync/MonAll
    participant SE as SyncElement
    participant T as IsElementThrottled

    AC->>EEH: событие изменения элемента
    EEH->>Sync: MonAll (Main:400) / SyncSelected (:410)
    Sync->>Sync: MonByType — ACAPI_Interface catch (:96)
    Note over Sync: на каждое событие
    Sync->>T: IsElementThrottled (guid)
    alt throttled
        T-->>Sync: пропуск
    else
        Sync->>SE: SyncElement → SyncData (запись)
    end
```

## 3. Спецификация

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню)
    participant Main as MenuCommandHandler
    participant SA as SpecAll
    participant F as SpecFilter
    participant SAR as SpecArray
    participant GER as GetElementsForRule
    participant PE as PlaceElements
    participant GEP as GetElementForPlace

    UI->>Main: команда Spec (:441)
    Main->>SA: SpecAll
    SA->>SA: GetSelectedElements / GetRuleFromDefaultElem (:152/:165)
    SA->>F: SpecFilter (:157/176) — типы/БД
    SA->>SAR: SpecArray (:185)
    SAR->>GER: GetElementsForRule (:1463) — группировка, параметры
    SAR->>PE: PlaceElements (:950)
    PE->>GEP: GetElementForPlace (:2461)
    PE->>PE: ACAPI_Element_Create в CallUndoableCommand (:2449)
    PE->>PE: UnhideUnlockElementLayer (:2473)
```

## 4. Перенумерация

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню)
    participant Main as MenuCommandHandler
    participant RS as ReNumSelected
    participant GE as GetRenumElements
    participant ES as ElementsSeparation
    participant GP as GetPos
    participant ROR as ReNumOneRule
    participant H as Helpers (ElementsWrite)

    UI->>Main: команда ReNum (:427)
    Main->>RS: ReNumSelected
    RS->>RS: GetRuleFromSelected (Helpers:355)
    RS->>GE: GetRenumElements (:87)
    GE->>ROR: ReNumOneRule (:345)
    ROR->>ES: ElementsSeparation (:674) — группировка по критериям
    ROR->>GP: GetPos (:765/780) + GetMostFrequentPos (:701/721)
    ROR->>ROR: RenumPos::ToParamValue (:817) + FormatToMax (:816)
    ROR->>H: AddParamValue2ParamDictElement → ElementsWrite (undo, :133-139)
```

## 5. Суммирование

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню)
    participant Main as MenuCommandHandler
    participant SS as SumSelected
    participant GSV as GetSumValuesOfElements
    participant SGE as Sum_GetElement
    participant SOR as Sum_OneRule
    participant H as Helpers (WriteInfo/ElementsWrite)

    UI->>Main: команда Sum (:433)
    Main->>SS: SumSelected
    SS->>SS: GetSelectedElements (Helpers:695)
    SS->>GSV: GetSumValuesOfElements (:44)
    GSV->>GSV: Sum_GetElement → Sum_Rule (разбор правил)
    GSV->>SOR: Sum_OneRule (:213)
    SOR->>SOR: NUM_SUM/TEXT_SUM/MIN/MAX, ignore_vals
    SOR->>H: результат → свойство или инфо проекта (WriteInfo, :95)
    Note over H: SumSelected оборачивает в CallUndoableCommand (:58)
```

## 6. Округление размеров

```mermaid
sequenceDiagram
    participant EV as Обработчики (Menu/Element/Project)
    participant DRA as DimRoundAll
    participant DRB as DimRoundByType
    participant DAR as DimAutoRound
    participant DP as DimParse
    participant AC as ArchiCAD (Element_Change)

    EV->>DRA: DimRoundAll (Main:128/165/489)
    DRA->>DRA: ACAPI_CallUndoableCommand (:433)
    DRA->>DRB: DimRoundByType (:434/438)
    DRB->>DAR: DimAutoRound (:458)
    DAR->>DAR: preadelem — предчтение параметров привязанного элемента
    DAR->>DP: DimParse (:188)
    DP->>DP: правила из PROPERTYCACHE (DimReadPref), формулы
    DP-->>DAR: custom_txt + flag_change/highlight
    DAR->>AC: ACAPI_Element_Change (:266)
```

## 7. Браузерная палитра

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню/выделение)
    participant Main as MenuCommandHandler
    participant BP as BrowserPalette
    participant HTML as HTML (DG::Browser)
    participant SCH as SelectionChangeHandler

    UI->>Main: команда палитры (:469)
    Main->>BP: ShowOrHideBrowserPalette
    BP->>HTML: Show → загрузка Interface_ru.html
    BP->>HTML: UpdateSelectionInfoInUI — executeJS (:163)
    UI->>SCH: смена выделения
    SCH->>SCH: suppressSelectionRefresh? (:719) — при программной подсветке выход
    SCH->>BP: ManualGetSelection (:1632) → UpdateSelectionInfoInUI
    HTML->>BP: JS-мост (RegisterACAPIJavaScriptObject; аргументы DynamicCast<JSValue>)
```

## 8. Ревизии

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню)
    participant Main as MenuCommandHandler
    participant SR as Revision::SetRevision
    participant CLP as ChangeLayoutProperty
    participant CMT as ChangeMarkerTextOnLayout
    participant CMT1 as ChangeMarkerText
    participant AC as ArchiCAD

    UI->>Main: команда SetRevision (:449)
    Main->>SR: SetRevision
    SR->>AC: StoreViewSettings (сохранение вида)
    SR->>SR: GetScheme → GetAllChangesMarker
    SR->>SR: обход БД (ChangeCurrentDatabase)
    SR->>CLP: ChangeLayoutProperty (:220) — свойства листа
    SR->>CMT: ChangeMarkerTextOnLayout (:219)
    CMT->>CMT1: ChangeMarkerText (:1017) — на каждый маркер
    CMT1->>AC: ACAPI_Element_Change (фильтры доступа/TW-резерв)
    SR->>AC: восстановление вида/окна
```

## 9. Выравнивание чертежей

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню)
    participant Main as MenuCommandHandler
    participant ADB as AlignDrawingsByPoints
    participant A1 as AlignOneDrawingsByPoints
    participant RS as RestoreStartDatabaseAndWindow
    participant AC as ArchiCAD

    UI->>Main: команда AutoLay (:465)
    Main->>ADB: AlignDrawingsByPoints
    ADB->>ADB: GetSelectedElements (:928) + ClickAPoint (:932)
    ADB->>ADB: GetDrawingsSort (:987)
    ADB->>AC: ACAPI_CallUndoableCommand (:1048)
    ADB->>A1: AlignOneDrawingsByPoints (:1006)
    A1->>AC: переключение БД/окна (:794/871)
    A1->>A1: hotspot внутри чертежа → новая позиция
    A1->>RS: RestoreStartDatabaseAndWindow (:806/811/846)
    ADB->>AC: ACAPI_Element_Change (:1061)
```

## 10. Сброс свойств

```mermaid
sequenceDiagram
    participant SAM as SyncAndMonAll
    participant RP as ResetProperty
    participant RPE as ResetPropertyElement2Defult
    participant RED as ResetElementsDefault
    participant RID as ResetElementsInDB
    participant ROE as ResetOneElemen / ResetOneElemenDefault
    participant AC as ArchiCAD

    SAM->>RP: ResetProperty (:174)
    RP->>RP: PROPERTYCACHE().property → определения с "Sync_reset"
    RP->>RPE: ResetPropertyElement2Defult (:34)
    RPE->>RED: ResetElementsDefault (:73) — дефолты типов (undo, :404)
    RPE->>RID: ResetElementsInDB ×N — текущая/фасады/… (:74-79)
    RID->>ROE: ResetOneElemen (в цикле, :166)
    ROE->>AC: ACAPI_Element_SetProperties в CallUndoableCommand (:283)
    Note over RP: AC27+: ResetProperty возвращает false — неработоспособна (контракт в pk.md)
```
