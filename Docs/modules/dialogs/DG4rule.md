# dialogs/DG4rule — Диалог выбора правил

> Обновлено 2026-10-03 (#256): добавлены `isReadOnly`, `columnTitles`,
> `valuesPerRule`, `footerText`, `footerIsWarn`, `valueColumnWidth`.
> Ранее: 493caf5 (2026-09-22).

## Назначение
Модальный диалог с таблицей правил (спецификация/нумерация/суммирование): чекбоксы + подтверждение. [из комментария, DG4rule.hpp:30-32]

## Ключевые типы
- `RuleSelectData`: rules (HashTable<string,bool>), qty_elements, color, titleResID,
  is_warn, **isReadOnly**, **columnTitles**, **valuesPerRule**, **footerText**,
  **footerIsWarn**, **valueColumnWidth** [из комментария, DG4rule.hpp:13-58]

## Два режима списка (#245, #256)
| Признак | Режим | Поведение |
|---------|-------|-----------|
| `columnTitles` пуст | выбор правил | одна колонка из `qty_elements`, ширина `QtyTab_w` (50), флажки |
| `columnTitles` непуст | отчёт (`isReadOnly`) | столько колонок, сколько заголовков; ширина `valueColumnWidth`, иначе `ValueTab_w` (60) |

Ширина колонки задаётся вызывающим через `valueColumnWidth`: подпись длиннее
колонки обрезается молча, поэтому колонок много — ширину задаёт владелец.
Окно 32590 — 680x300 px, `MultiSelList` 650 px: пять колонок по 70 px + имя.

`isReadOnly` убирает флажок, клик по строке и `footerText` становится основным
содержимым окна (ошибки показываются в нём).

## Класс RuleSelectDialog
Наследование: DG::ModalDialog + 6 observer'ов [из комментария, DG4rule.hpp:33-39]
Элементы: closeButton (1), okButton (2), ListBox (3), TextBox (4) [из комментария, :42]

| Метод | Назначение |
|-------|------------|
| `ButtonClicked` | Нажатия кнопок [из комментария, :63] |
| `PanelResized` | Пересчёт размеров при ресайзе [из комментария, :66] |
| `ListBoxClicked` | Щелчок по строке [из комментария, :69] |
| `SetSize` | Размеры/позиции элементов [из комментария, :78] |
| `InitListBox` | Заполнение списка + чекбоксы [из комментария, :81] |
| `SetIcon` | Иконка чекбокса выбранной строки [из комментария, :84] |

## Зависимости
- `DGModule.hpp`, `DGStaticItem.hpp` [по include]

## Зависимости (используется в)
Spec, ReNum, Summ [по коду]
