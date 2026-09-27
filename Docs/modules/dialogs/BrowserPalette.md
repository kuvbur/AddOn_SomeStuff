# dialogs/BrowserPalette — UI Entry Point

> Хеш коммита: 493caf5 (2026-09-22). Номера строк .cpp — выборочно проверены.

## Назначение
Браузерная палитра с HTML-интерфейсом: видимость, свойства, JS-мост, мониторинг выделения. [по коду]

## Файлы
- `dialogs/BrowserPalette.cpp/hpp`, HTML: `RFIX/HTML/Interface_ru.html`

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ShowOrHideBrowserPalette` | Переключение видимости палитры [из комментария, hpp:23]; вызывается из `MenuCommandHandler` (Main:469), определение BrowserPalette.cpp:37 [из callgraph.json] |
| `BrowserPalette::Show` | Показ; reloadContent=false — без перезагрузки HTML (APIPalMsg_HidePalette_End; перезагрузка сбрасывала вкладку/фильтр) [из комментария, hpp:106-109; .cpp:127] |
| `BrowserPalette::Hide` | Скрытие [из комментария, .cpp:169] |
| `BrowserPalette::ManualGetSelection` | Ручное получение выделения [из комментария, hpp:112] |
| `BrowserPalette::SelectionChangeHandler` | Обработчик изменения выделения [из комментария, hpp:115] — карточка |
| `UpdateSelectionInfoInUI` | Обновление представления выделения в HTML [из комментария, .cpp:175] |
| `RegisterACAPIJavaScriptObject` | Регистрация JS объекта для вызовов из HTML [из комментария, hpp:49] |

## Карточки

### `BrowserPalette::SelectionChangeHandler(const API_Neig *) -> GSErrCode`
- Расположение: `Sources/AddOn/dialogs/BrowserPalette.cpp` (строка не проверена)
- Назначение: обработка смены выделения, обновление UI. [из комментария]
- Контракт: защищён `suppressSelectionRefresh` — при программной подсветке (`APIIo_HighlightElementsID`/`APIDo_ZoomToElementsID`) обновление подавляется. [из комментария, hpp:91-95]
- Побочные эффекты: обновление HTML (execute JS). [по коду]

### `BrowserPalette::Show(bool reloadContent)`
- Расположение: `Sources/AddOn/dialogs/BrowserPalette.cpp:127`
- Назначение: показ палитры; при reloadContent перечитывает HTML и обновляет выделение (UpdateSelectionInfoInUI, .cpp:163). [по коду]
- Побочные эффекты: показ окна; при reloadContent — сброс активной вкладки/фильтра (FIX). [из комментария + по коду]

## Инварианты
- Palette resize: `UnDock()` → `SetClientWidth()` → `Dock()`; min width ослаблять перед сжатием (AGENTS.md §6) [из AGENTS.md]
- JS bridge: `DynamicCast<JSValue>` — `DynamicCast<JSArray>` крашит ArchiCAD (AGENTS.md §6) [из AGENTS.md]
- Поля ширины: expandedClientWidth/expandedMinClientWidth/collapsedClientWidth (hpp:74-88) [из комментария]
- «Монитор»: закрепления свойств (`pinnedProperties`) и групп (`pinnedPropertyGroups`) сессионные; после локальной фильтрации закреплённые свойства идут первой синтетической группой, затем закреплённые группы без дублирования свойств. [по коду, HTML #160/#204]
- Инлайн-кнопка сброса в строке свойства временно отсутствует; мост `resetPropertyToDefault` и иконка цепочки сохранены для отдельного этапа режима выборочного сброса #189. [по коду, HTML #189]
- Версионные ветки моста и подсветки (проверено сборкой AC26, #170): подсветка — AC25 `APIIo_HighlightElementsID` (единственная ветка с кодом ошибки), AC26 `ACAPI_Interface_Clear/SetElementHighlight`, AC27+ `ACAPI_UserInput_*` (в обеих новых ветках SDK-функции возвращают `void`, поэтому `hlErr` остаётся `NoError`); `browser.UnregisterJSObject` удалён из DGLib в AC26 и вернулся в AC27 (перегрузка по имени, модуль `JavascriptEngine`); вызов компилируется под `#if defined(ServerMainVers_2700) || !defined(ServerMainVers_2600)`. [по коду DevKit + сборка AC26]
- `FilterElementsByType` получает тип элемента через `GetElemTypeID(head)` (CommonFunction), а не через `head.typeID`: в AC26 поле переименовано в `API_Elem_Head::type` (`API_ElemType`). [по коду DevKit + сборка AC26]
- AC27 (`#170`): JS-мост переехал из DGLib в модуль `JavascriptEngine` — `DG::JSBase/JSFunction/JSObject/JSArray/JSValue` стали `JS::Base/Function/Object/Array/Value`; в `BrowserPalette.hpp` под `#ifdef AC_27` объявлены псевдонимы в `namespace DG`, поэтому тела функций моста не менялись. [по коду DevKit + сборка AC27]
- AC27: `DGModule.hpp` больше не включает `DGBrowser.hpp` — браузерный контрол подключается явно (`BrowserPalette.hpp`). [по коду DevKit + сборка AC27]
