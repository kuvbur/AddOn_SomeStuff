# SEQUENCES — Ключевые сценарии

> Хеш коммита: после `68efad7` (2026-10-15, ветвь `roombook_refactor`). Набор из 11 сценариев (10 согласованы 2026-09-22 после `12874ab`; сценарий 11 добавлен 2026-10-15). Координаты сверены с исходником 2026-10-15 (коммиты `18d05b7..68efad7` сместили строки в `SomeStuff_Main.cpp`, `Sync.cpp`, `spec/`, `ReNum.cpp`, `Helpers.cpp`, `Dimensions.cpp`, `pk/`, `dialogs/BrowserPalette.cpp`). Все сообщения — из `Docs/_generated/callgraph.json` и `body_scan.json` (координаты в тексте — 1-based).

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

    UI->>Main: команда Sync All (SomeStuff_Main.cpp:426, case :423)
    Main->>SS: LoadSyncSettingsFromPreferences (:401)
    Main->>PC: Update (:410)
    Main->>Sync: SyncAndMonAll (:426)
    Sync->>RP: ResetProperty (:225)
    RP-->>Sync: true → ранний выход (сброс выполнен)
    Sync->>SBT: SyncByType ×6 (wallS/widoS/objS/cwallS/dim/ext, :242-283)
    SBT->>SBT: ACAPI_Element_GetElemList (:347)
    SBT->>SE: SyncElement (:374; определение :404/:412)
    SE->>SD: SyncData (:432/:446; определение :753)
    SD->>SD: ParseSyncString (:809, определение :1178) → SyncCalcRule (:835/:840, определение :894)
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
    EEH->>Sync: MonAll (Main:421) / SyncSelected (:431)
    Sync->>Sync: MonByType — ACAPI_Interface catch (:168)
    Note over Sync: на каждое событие
    Sync->>T: IsElementThrottled (guid, :39)
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

    UI->>Main: команда Spec (:462, case :460)
    Main->>SA: SpecAll (Spec.cpp:155)
    SA->>SA: GetSelectedElements (:192) / GetRuleFromDefaultElem (:40, вызов :204)
    SA->>F: SpecFilter (:196/:215, определение :251) — типы/БД
    SA->>SAR: SpecArray (:229; определение :633)
    Note over SA: отказ «все флаги выключены» идёт<br/>в накопитель результата, окно одно<br/>(ShowRunResult :226), не всплывает
    SAR->>GER: GetElementsForRule (:2086) — группировка, параметры
    SAR->>GER: PlanRuleRows (:1839)
    SAR->>PE: PlaceElements (SpecExecutor.cpp:12)
    PE->>GEP: GetElementForPlace (Spec.cpp:3085)
    PE->>PE: ACAPI_Element_Create (:162) — БЕЗ CallUndoableCommand<br/>(APIERR_NOTMINE, комментарий :275)
    PE->>PE: UnhideUnlockElementLayer (:90)
    PE->>PE: WriteSpecProperties (:281)
    Note over PE: вклад источника — BuildContribution<br/>(SpecPlanning.cpp:20)
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

    UI->>Main: команда ReNum (:446, case)
    Main->>RS: ReNumSelected (ReNum.cpp:60)
    RS->>RS: GetRuleFromSelected (Helpers:356)
    RS->>GE: GetRenumElements (:301)
    GE->>ROR: ReNumOneRule (вызов :457, определение :945)
    ROR->>ES: ElementsSeparation (:1134) — группировка по критериям
    ROR->>GP: GetPos (:924, вызовы :1047/:1062) + GetMostFrequentPos (:896, вызовы :983/:1003)
    ROR->>ROR: RenumPos::ToParamValue (:1099) + FormatToMax (:1098)
    ROR->>H: AddParamValue2ParamDictElement (:872-878) → ElementsWrite (undo, :112/:117)
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

    UI->>Main: команда Sum (:452, case)
    Main->>SS: SumSelected (Summ.cpp:29)
    SS->>SS: GetSelectedElements (Helpers:647)
    SS->>GSV: GetSumValuesOfElements (:120, вызов :44)
    GSV->>GSV: Sum_GetElement (вызов :149, определение :193) → Sum_Rule (разбор правил)
    GSV->>SOR: Sum_OneRule (вызов :185, определение :409)
    SOR->>SOR: NUM_SUM/TEXT_SUM/MIN/MAX, ignore_vals
    SOR->>H: результат → свойство или инфо проекта (WriteInfo, :67)
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

    EV->>DRA: DimRoundAll (Dimensions.cpp:429; вызовы в Main:142/509)
    DRA->>DRA: ACAPI_CallUndoableCommand (:436)
    DRA->>DRB: DimRoundByType (:437/:441; определение :451)
    DRB->>DAR: DimAutoRound (:36)
    DAR->>DAR: preadelem — предчтение параметров привязанного элемента
    DAR->>DP: DimParse (:190, определение :291)
    DP->>DP: правила из PROPERTYCACHE (DimReadPref), формулы
    DP-->>DAR: custom_txt + flag_change/highlight
    DAR->>AC: ACAPI_Element_Change (:275)
```

## 7. Браузерная палитра

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню/выделение)
    participant Main as MenuCommandHandler
    participant BP as BrowserPalette
    participant HTML as HTML (DG::Browser)
    participant SCH as SelectionChangeHandler

    UI->>Main: команда палитры (:489, case Pallete_CommandID)
    Main->>BP: ShowOrHideBrowserPalette (BrowserPalette.cpp:45)
    BP->>HTML: Show → загрузка Interface_ru.html
    BP->>HTML: UpdateSelectionInfoInUI — executeJS (:340, вызов :328)
    UI->>SCH: смена выделения
    Note over SCH: SelectionChangeHandlerProc (SomeStuff_Main.cpp:355)<br/>→ BrowserPalette::SelectionChangeHandler
    SCH->>SCH: suppressSelectionRefresh? (BrowserPalette.cpp:40/:869,<br/>SuppressSelectionRefreshGuard) — при программной подсветке выход
    SCH->>BP: ManualGetSelection (:1977) → UpdateSelectionInfoInUI
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

    UI->>Main: команда SetRevision (:468, case)
    Main->>SR: SetRevision (Revision.cpp:14)
    SR->>AC: StoreViewSettings (сохранение вида)
    SR->>SR: GetScheme → GetAllChangesMarker
    SR->>SR: обход БД (ChangeCurrentDatabase)
    SR->>CLP: ChangeLayoutProperty (вызов :219, определение :398) — свойства листа
    SR->>CMT: ChangeMarkerTextOnLayout (вызов :218, определение :984)
    CMT->>CMT1: ChangeMarkerText (:1011) — на каждый маркер
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

    UI->>Main: команда AutoLay (:485, case)
    Main->>ADB: AlignDrawingsByPoints (AutomateFunction.cpp:913)
    ADB->>ADB: GetSelectedElements2 (:915) + ClickAPoint (:919)
    ADB->>ADB: GetDrawingsSort (:879, вызов :971)
    ADB->>A1: AlignOneDrawingsByPoints (:764, вызов :987)
    A1->>AC: переключение БД/окна (APIDb_ChangeCurrentDatabaseID :1002 / APIDo_ChangeWindowID :1011)
    A1->>A1: hotspot внутри чертежа → новая позиция
    A1->>RS: RestoreStartDatabaseAndWindow (:744, вызовы :797/:801/:833)
    ADB->>AC: ACAPI_CallUndoableCommand «Move drawing» (:1027) → ACAPI_Element_Get (:1032) → ACAPI_Element_Change (:1040)
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

    SAM->>RP: ResetProperty (Sync.cpp:225 → pk/ResetProperty.cpp:14)
    RP->>RP: PROPERTYCACHE().property → определения с "Sync_reset"
    RP->>RPE: ResetPropertyElement2Defult (вызов :34, определение :40)
    RPE->>RED: ResetElementsDefault (вызов :65, определение :275) — дефолты типов (undo, :386/:390)
    RPE->>RID: ResetElementsInDB ×N — текущая/фасады/… (определение :122, вызовы :67-78)
    RID->>ROE: ResetOneElemen (в цикле, определение :240, вызовы :145/:220)
    ROE->>AC: ACAPI_Element_SetProperties (:260) в CallUndoableCommand (:259)
    Note over RP: AC27+: ResetProperty возвращает false — неработоспособна (контракт в pk.md)
```

## 11. Спецификация отделки (RoomBook) — обновлено 2026-10-30 после `b199125`

```mermaid
sequenceDiagram
    participant UI as ArchiCAD (меню/HTTP)
    participant Main as MenuCommandHandler
    participant JC as RoomBookCommand (JSON)
    participant RB as Roombook::RoomBook_Run
    participant GTZ as Zone_CollectTargets
    participant CTX as RoomProcessingContext_Prepare
    participant PEF as RoomProcessingContext_ProcessElementsForRoomData
    participant PRP as RoomProcessingContext_PreparePartitions
    participant PF as RoomProcessingContext_ReadElementParameters
    participant OTD as Otd_GetOtd_ByZone
    participant FIN as OtdRooms_ProcessFinishes
    participant DR as OtdRooms_Draw
    participant APP as AdvanceProcessPhase
    participant PEU as PrepareElementsForUpdate

    UI->>Main: команда RoomBook (SomeStuff_Main.cpp:458)
    UI->>JC: SomeStuffCommand.RoomBook (JSON API, AC25+)
    JC->>JC: Execute (RoomBookCommand.cpp:33)
    Main->>RB: RoomBook_Run (Roombook.cpp:102)
    JC->>RB: RoomBook_Run (RoomBookCommand.cpp:37)
    RB->>RB: LayerCombination_ApplyFinishIfExists (:68, вызов :110)
    RB->>GTZ: Zone_CollectTargets (:25, вызов :113)
    RB->>CTX: RoomProcessingContext_Prepare (RoombookData.cpp:4810, вызов :120)
    Note over CTX: BuildElementReadIndex (:4835) →<br/>ProcessElementsForRoomData (:4907) →<br/>PreparePartitions (:4973) → ReadElementParameters (:5028)
    CTX->>CTX: surface-pipeline внутри RoombookData.cpp<br/>(RoomSurfaces → Partition → Finish)
    RB->>APP: AdvanceProcessPhase (CommonFunction.cpp:2651, вызов :132)
    RB->>RB: ElementsRead (Helpers.cpp:5583, вызов :137) — мемо всех элементов
    RB->>RB: Favorite_GetDict (RoombookData.cpp:8933, вызов :139)
    RB->>OTD: Otd_GetOtd_ByZone (RoombookData.cpp:5888, вызов :144)
    RB->>FIN: OtdRooms_ProcessFinishes (RoombookData.cpp:5711, вызов :147)
    RB->>RB: OtdData_AddRoomToMaterialSummary / OtdData_WriteRoomMaterialData<br/>(RoombookQuantities.cpp:61/:137, вызовы :167/:169)
    RB->>RB: Roombook_CheckFinishConfiguration (RoombookData.cpp:9448, вызов :174)
    RB->>DR: OtdRooms_Draw (RoombookDraw.cpp:2670, вызов :178)
    DR->>DR: RoofBatch_Validate (:2007) → OtdRooms_AssignExistingWallFinishes (:444)
    DR->>PEU: PrepareElementsForUpdate (CommonFunction.cpp:2511, вызов :2705)
    DR->>AC: ACAPI_Element_Delete (deletelist, вызов :2709)
    DR->>DR: OtdRoom_Draw (:2535) → Wall/Slab/Beam/Object/Roof_Draw + Class_Set
    DR->>DR: CollectRoomFinishes (:2654) → OtdElements_SetSync (:645)
    Note over RB: результат — GS::Array<Roombook::RoomBookFinishResult>
```
