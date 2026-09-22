# dialogs/BrowserPalette — UI Entry Point

> Хеш коммита: 493caf5 (2026-09-22). Номера строк .cpp — выборочно проверены.

## Назначение
Браузерная палитра с HTML-интерфейсом: видимость, свойства, JS-мост, мониторинг выделения. [по коду]

## Файлы
- `dialogs/BrowserPalette.cpp/hpp`, HTML: `RFIX/HTML/Interface_ru.html`

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ShowOrHideBrowserPalette` | Переключение видимости палитры [из комментария, hpp:23] |
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
