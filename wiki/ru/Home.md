> 🇷🇺 [Русский](#-somestuff--дополнение-для-archicad) | 🇬🇧 [English](#-somestuff--archicad-add-on)

---

# 🇷🇺 SomeStuff — дополнение для Archicad

Аддон решает узкоспециализированные задачи автоматизации в Archicad: синхронизацию свойств и GDL-параметров, нумерацию, анализ конструкций, оформление чертежей и многое другое.

---

## 🚀 Начало работы

|                                                                                                        |                                                                                |
| ------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------ |
| [Установка (WIN / MAC)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Install-ru)                     | Скачать, скопировать в папку расширений, добавить команды в меню               |
| [FAQ](https://github.com/kuvbur/AddOn_SomeStuff/wiki/FAQ-ru)                                           | Зачем аддон, что делать если ничего не работает, влияние на производительность |
| [Список всех команд свойств](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Property-Commands-List-ru) | Полный справочник: `Sync_from`, `Sync_to`, флаги, формулы, МЕР-команды         |
| [Тестовые версии](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Testing-Help-ru)                      | Как скачать сборку из Actions и присоединиться к тестированию                  |

---

## 🔗 Синхронизация свойств и GDL

Двусторонний обмен данными между свойствами Archicad и GDL-параметрами библиотечных элементов.

### [Синхронизация GDL-параметров и свойств](https://github.com/kuvbur/AddOn_SomeStuff/wiki/GDLParameter2Property-ru)

- Копирование GDL-параметра **в свойство**: `Sync_from{ИМЯ_ПАРАМЕТРА}`
- Копирование свойства **в GDL-параметр**: `Sync_to{ИМЯ_ПАРАМЕТРА}`
- Копирование одного свойства в другое: `Sync_from{Property:ГРУППА/СВОЙСТВО}`
- Режимы: **Отслеживать** (авто при изменении) / **Синхронизировать все** / **Синхронизировать выделенные**

### [Синхронизация по GUID и вложенным элементам](https://github.com/kuvbur/AddOn_SomeStuff/wiki/LinkPropertyByGUID-ru)

- Навесные стены, зоны и другие иерархические структуры: `Sync_from_sub{...}` / `Sync_to_sub{...}`
- Связь произвольных элементов через GUID: `Sync_from_GUID{...}`

---

## Конструкции и материалы

### [Вывод состава конструкций в свойства](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Construction-Composition-ru)

Шаблонная строка в описании свойства формирует текстовый состав многослойных конструкций и сложных профилей.

- Порядок слоёв: `Layers` / `Layers_inv` / `Layers_auto`
- Толщина, штриховка, плотность, пользовательские свойства материала
- Объёмы и площади компонент (`%area%`, `%volume%`, `%qty%`)
- Формулы с суммированием по слоям
- Форматирование для выносок (`\CRLF`, `\TAB` и др.)

### [Создать элементы спецификации (Spec_rule)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Breakdown-Composites-Create-Elements-ru)

Генерирует строки спецификации для составных GDL-объектов (перемычки, арматура, прокат, материалы).

- `Spec_rule` — базовая версия
- `Spec_rule_v2` — с автообновлением и подсветкой изменений
- `Spec_rule_v3` — с игнорированием элементов без нужных параметров
- Поддержка сборок, арматуры, проката и материалов из библиотеки kuvbur

### [Ведомость отделки](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Finishing-ru)

Анализирует прилегающие к зоне конструкции, собирает данные об отделочных слоях. Может создавать элементы отделки (аналог Мастера интерьера) и/или записывать состав в свойства зоны.

---

## Нумерация элементов

### [Нумерация элементов](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Element-Renumbering-ru)

Аналог Менеджера ID с сохранением позиций в свойства.

- Флаг нумерации в описании свойства: `Renum_flag{имя_свойства}`
- Автозаполнение нулями / пробелами: `NULL`, `SPACE`, `ALLNULL`, `n_NULL` и др.
- Группировка по критерию и разбивке
- Запись позиции в ID элемента: `Sync_to{ID}`

---

## Размеры и округление

### [Работа с размерами](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Dimension-Functions-ru)

Автоматическая проверка и обработка размеров на активном виде по правилам из информации о проекте (`Addon_Dimensions`).

- Правила по слою или перу размера
- Подсветка некратных размеров, замена текста
- Скрытие толщин стен (`DeleteWall`), сброс перебитых значений (`ResetText`)
- Классическое округление (`ClassicRound`)

### [Округление числовых значений в свойствах](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Rounding-Thicknesses-Properties-ru)

Формат округления задаётся в имени свойства после точки: `1mm`, `01mm`, `01mp` и др.

---

## 🗺️ Профили и чертежи

### [Построить профиль по линии](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Build-Profile-Line-ru)

Создаёт 3д документы разрезов вдоль морфа-линии или ограждения для построения профилей инженерных сетей по ГОСТ.

1. Рабочий лист с хотспотом (перо 163) → общее оформление
2. Морф/ограждение с ID `ИМЯ_УЧАСТКА@МАСШТАБ`
3. Запуск **Построить профиль по линии** → создаются 3д документы
4. Размещение на макете → **Выровнять выделенные чертежи**

---

## Классификация

### [Автоматическая классификация элементов](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Automatic-Element-Classification-ru)

- Автоназначение класса неклассифицированным элементам: добавить `some_stuff_class` в описание класса
- Смена класса при изменении свойства: `Sync_to{Class:ИМЯ_КЛАССИФИКАЦИИ}`
- Вывод полного имени класса в свойство: `Sync_from{Class:ИМЯ_КЛАССИФИКАЦИИ; FullName}`

---

## Оформление документации

### [Изменения по ГОСТ 21.1101](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Layout-Changes-ru)

Заполняет штампы и маркеры изменений на макетах в соответствии с ГОСТ Р 21.101-2020. Совместимо с библиотекой [kuvbur_Формат по ГОСТ](https://github.com/kuvbur/gdl_bibl).

### [Вывод QR-кода из свойства](https://github.com/kuvbur/AddOn_SomeStuff/wiki/QR-Code-Output-ru)

Формирует строку для макроса `macro_qrcode` на основе значения свойства: `Sync_from{QRCode:Property:ИМЯ_СВОЙСТВА}`. Поддерживает вывод в 2д и 3д скрипты, выносные надписи и IFC.

---

---

# 🇬🇧 SomeStuff — Archicad Add-on

SomeStuff solves specialized automation tasks in Archicad: synchronizing properties with GDL parameters, element numbering, construction analysis, drawing layout automation, and more.

---

## 🚀 Getting Started

|                                                                                                             |                                                                           |
| ----------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------- |
| [Installation (WIN / MAC)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Install-en)                       | Download, copy to the Add-Ons folder, add commands to the menu            |
| [FAQ](https://github.com/kuvbur/AddOn_SomeStuff/wiki/FAQ-en)                                                | Why this add-on, what to do if nothing works, performance notes           |
| [Full Property Command Reference](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Property-Commands-List-en) | Complete reference: `Sync_from`, `Sync_to`, flags, formulas, MEP commands |
| [Testing / Beta Builds](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Testing-Help-en)                     | How to download builds from GitHub Actions and join testing               |

---

## 🔗 Property & GDL Synchronization

Two-way data exchange between Archicad properties and GDL parameters of library elements.

### [GDL Parameter ↔ Property Sync](https://github.com/kuvbur/AddOn_SomeStuff/wiki/GDLParameter2Property-en)

- Copy GDL parameter **into a property**: `Sync_from{PARAM_NAME}`
- Copy a property **into a GDL parameter**: `Sync_to{PARAM_NAME}`
- Copy one property into another: `Sync_from{Property:GROUP/PROPERTY}`
- Modes: **Monitor** (auto on change) / **Sync All** / **Sync Selected**

### [GUID-based and Hierarchical Sync](https://github.com/kuvbur/AddOn_SomeStuff/wiki/LinkPropertyByGUID-en)

- Curtain walls, zones and other hierarchical structures: `Sync_from_sub{...}` / `Sync_to_sub{...}`
- Link arbitrary elements via GUID: `Sync_from_GUID{...}`

---

## Constructions & Materials

### [Output Construction Layers to Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Construction-Composition-en)

A template string in the property description generates a text composition of composite walls and complex profiles.

- Layer order: `Layers` / `Layers_inv` / `Layers_auto`
- Thickness, hatch, density, custom material properties
- Component volumes and areas (`%area%`, `%volume%`, `%qty%`)
- Formulas with per-layer summation
- Label formatting (`\CRLF`, `\TAB`, etc.)

### [Create Schedule Elements (Spec_rule)](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Breakdown-Composites-Create-Elements-en)

Generates schedule rows for composite GDL objects (lintels, rebar, structural sections, materials).

- `Spec_rule` — basic version
- `Spec_rule_v2` — with auto-update and change highlighting
- `Spec_rule_v3` — silently skips elements with missing parameters
- Supports assemblies, rebar, rolled sections and materials from the kuvbur library

### [Finishing Schedule](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Finishing-en)

Analyses construction layers adjacent to a zone and collects finishing layer data. Can create finish elements (similar to Interior Elevation wizard) and/or write the composition into zone properties.

---

## Element Numbering

### [Element Numbering](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Element-Renumbering-en)

An ID Manager alternative that writes positions into properties.

- Numbering flag in the property description: `Renum_flag{property_name}`
- Auto-padding with zeros or spaces: `NULL`, `SPACE`, `ALLNULL`, `n_NULL`, etc.
- Grouping by criterion and partition properties
- Write position to Element ID: `Sync_to{ID}`

---

## Dimensions & Rounding

### [Dimension Processing](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Dimension-Functions-en)

Automatically checks and processes dimensions on the active view using rules defined in Project Info (`Addon_Dimensions`).

- Rules by layer name or dimension line pen
- Highlight non-multiple dimensions, replace text
- Hide wall thicknesses (`DeleteWall`), reset overridden values (`ResetText`)
- Classic rounding mode (`ClassicRound`)

### [Numeric Value Rounding in Properties](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Rounding-Thicknesses-Properties-en)

Rounding format is set in the property name after a dot: `1mm`, `01mm`, `01mp`, etc.

## Classification

### [Automatic Element Classification](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Automatic-Element-Classification-en)

- Auto-assign class to unclassified elements: add `some_stuff_class` to the class description
- Change class when a property changes: `Sync_to{Class:CLASSIFICATION_NAME}`
- Output full class name to a property: `Sync_from{Class:CLASSIFICATION_NAME; FullName}`

---

## Drawing Documentation

### [QR Code Output from Property](https://github.com/kuvbur/AddOn_SomeStuff/wiki/QR-Code-Output-en)

Generates a string for the `macro_qrcode` GDL macro from a property value: `Sync_from{QRCode:Property:PROPERTY_NAME}`. Supports output in 2D/3D scripts, labels and IFC.
