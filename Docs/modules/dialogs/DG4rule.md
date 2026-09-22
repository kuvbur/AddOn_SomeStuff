# dialogs/DG4rule — Диалог выбора правил

## Назначение
Модальный диалог с таблицей правил спецификации/нумерации/суммирования: изменение состояния чекбоксов, подтверждение выбора. Используется как общее решение для диалогов выбора правил.

## Файлы
- `dialogs/DG4rule.cpp/hpp` — диалог

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `RuleSelectData` | Данные правил: rules (HashTable<string, bool>), qty_elements, color, titleResID, is_warn |

## Класс RuleSelectDialog

Наследование: `DG::ModalDialog`, `DG::PanelObserver`, `DG::ListBoxObserver`, `DG::ButtonItemObserver`, `DG::CheckItemObserver`, `DG::CompoundItemObserver`, `DG::StaticTextObserver`

### Элементы диалога
- `closeButton` (ID 1), `okButton` (ID 2), `ListBox` (ID 3), `TextBox` (ID 4)

### Методы
| Метод | Назначение |
|-------|------------|
| `ButtonClicked` | Обработка кнопок |
| `PanelResized` | Перерасчёт при изменении размера |
| `ListBoxClicked` | Щелчок по строке |
| `SetSize` | Настройка размеров/позиций |
| `InitListBox` | Заполнение списка правил + чекбоксы |
| `SetIcon` | Переключение иконки чекбокса |

### Поля
- `ChekboxTab` = 1, `NameTab` = 2, `QtyTab` = 3 — вкладки
- `ChekboxTab_w` = 30, `QtyTab_w` = 50 — ширины
- `itemCount` = QtyTab — начальный счётчик

## Зависимости
- `DGModule.hpp`, `DGStaticItem.hpp`

## Зависимости (используется в)
- `Spec` — выбор правил
- `ReNum` — выбор правил перенумерации
- `Summ` — выбор правил суммирования
