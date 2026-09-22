# dialogs/BrowserPalette — UI Entry Point

## Назначение
Браузерная палитра с HTML-интерфейсом — основная точка входа UI add-on. Управляет видимостью палитры, отображением свойств, JS-мостом для вызовов из HTML, мониторингом выделения.

## Файлы
- `dialogs/BrowserPalette.cpp/hpp` — палитра
- `RFIX/HTML/Interface_ru.html` — HTML-страница (загружается из resources)

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `BrowserPalette` | Класс палитры: наследует DG::Palette, DG::PanelObserver |
| `SelectionModification` | Enum: RemoveFromSelection, AddToSelection |

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ShowOrHideBrowserPalette` | Переключение видимости палитры |
| `ElementCanHaveProperty` | Проверка типа элемента |
| `FilterElementsByType` | Фильтрация по типу с ограничением maxSelectionCount |
| `BrowserPalette::Show` | Показ (reloadContent = false — без перезагрузки HTML) |
| `BrowserPalette::Hide` | Скрытие |
| `BrowserPalette::ManualGetSelection` | Ручное получение выделения |
| `BrowserPalette::RegisterPaletteControlCallBack` | Регистрация колбэка палитры |
| `BrowserPalette::SelectionChangeHandler` | Обработчик изменения выделения |
| `BrowserPalette::GetInstance` | Получение singleton |

## Поля
- `browser` — DG::Browser для HTML
- `maxSelectionCount` = 10 — максимум элементов в selection
- `expandedClientWidth`, `expandedMinClientWidth`, `collapsedClientWidth` — размеры палитры (см. AGENTS.md §6 — palette resize)
- `suppressSelectionRefresh` (static) — подавление сброса палитры при подсветке (см. AGENTS.md §6 — element highlight)

## JS Bridge
- `RegisterACAPIJavaScriptObject` — регистрация JS объекта для вызовов из HTML
- Парсинг аргументов через `DynamicCast<JSValue>` (НЕ DynamicCast<JSArray> — краш!)

## Зависимости
- `Sync.hpp` — SyncSettings, SyncRule
- `DGModule.hpp`
- `Helpers.hpp`
- `RFIX/HTML/Interface_ru.html` — HTML ресурс

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — создание палитры

## Инварианты
- Palette resize: `UnDock()` → `SetClientWidth()` → `Dock()` (см. AGENTS.md §6)
- `SelectionChangeHandler` триггерится при programmatic highlight — нужен `suppressSelectionRefresh`
- HTML перезагружается при Show по умолчанию; reloadContent=false сохраняет состояние
