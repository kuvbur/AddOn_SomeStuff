# table/TablesNavigator — Каркас ведомостей

## Назначение
Каркас пользовательских ведомостей в Navigator/MyDraw. Отделён от Spec/Summ/Monitor для чистоты архитектуры. Первый шаг — фиксация границ подсистемы.

## Файлы
- `table/TablesNavigator.cpp/hpp` — навигатор таблиц

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `ColumnDefinition` | Колонка: id (стабильный ключ), title |
| `RowIdentity` | Строка: stableKey + sourceElements (одна строка = элемент/группа/агрегат) |
| `ScheduleDefinition` | Определение ведомости: navigatorGuid, internalId, displayName, columns |
| `TableCell` | Ячейка: columnId, displayText |
| `TableRow` | Строка: identity + cells |
| `TableSnapshot` | Снимок таблицы: definition + rows (отделён от definition для renderer) |

## Публичный API

| Функция | Назначение |
|---------|------------|
| `RegisterInterface` | Регистрация интерфейса в ArchiCAD |
| `Initialize` | Инициализация |
| `EnsureNavigatorRoot` | Гарантия корня Navigator |
| `IsNavigatorRegistrationEnabled` | Проверка регистрации |

## Проектирование
- `id` отдельно от `title`: title переименовывается в UI, id — устойчивый ключ
- `TableSnapshot` отделён от `ScheduleDefinition`: renderer работает одинаково для MyDraw и CreateIDFStore
- Payload viewpoint/Add-On Object ещё не выбран

## Зависимости
- `ACAPinc.h`

## Зависимости (используется в)
- `SomeStuff_Main` — регистрация в UI

## Статус
- Это каркас/заготовка — реальные данные пока не отображаются
- Связь с TableRenderer — на стадии проектирования
