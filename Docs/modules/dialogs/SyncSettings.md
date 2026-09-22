# dialogs/SyncSettings — Настройки синхронизации

## Назначение
Хранение и сериализация настроек аддона. Настройки хранятся в ЛОКАЛЬНОМ файле пользователя (`…/GRAPHISOFT/SomeStuffAddonConfig.json`), НЕ в preferences проекта (ACAPI_SetPreferences ломает Teamwork).

## Файлы
- `dialogs/SyncSettings.cpp/hpp` — настройки

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SyncSettings` | Основной класс (наследует GS::Object, DECLARE_CLASS_INFO) |
| `FilterPreset` | {label, query} — шаблоны фильтров |

## Флаги настроек

| Флаг | Описание |
|------|----------|
| `syncAll` | Полная синхронизация всех элементов |
| `syncMon` | Мониторинг изменений элементов |
| `wallS` | Обработка стен |
| `widoS` | Обработка окон, дверей, Skylight |
| `objS` | Обработка объектов, светильников, зон |
| `cwallS` | Обработка curtain wall |
| `logMon` | Логирование мониторинга |
| `showpalette` | Показывать палитру браузера |
| `catchSelectionChanges` | Автообновление интерфейса при изменении выделения |
| `maxSelectionCount` | Макс. количество в мультивыделении |
| `filterPresets` | Массив фильтров |

## Методы

| Метод | Назначение |
|-------|------------|
| `CreateDefault()` | Стандартные значения |
| `CreateWithSyncAll()` | Дефолт + syncAll=true |
| `Read/Write` | Сериализация (GS::I/O Channel) |
| `Get/Set*` | Доступ к каждому флагу |
| `GetFilterPresets/SetFilterPresets` | Фильтры |
| `GetSyncSettingsCache(forceReload)` | Глобальный кэш настроек |
| `LoadSyncSettingsFromPreferences(s, forceReload)` | Загрузка из кэша |
| `WriteSyncSettingsToPreferences(s)` | Сохранение в локальный файл |

## Условные компиляции
- AC25: `MemoryIChannel = GS::MemoryIChannel` (без namespace IO)
- AC26+: `MemoryIChannel = IO::MemoryIChannel`

## Зависимости
- `MemoryIChannel.hpp`, `MemoryOChannel.hpp`
- `Object.hpp`
- `CommonFunction.hpp` (для чтения/записи)
- `Helpers.hpp`

## Зависимости (используется в)
- `Sync.cpp` — основное потребление
- `BrowserPalette.cpp` — UI
- `SyncSettings.cpp/dialog` — диалог редактирования

## Инварианты
- НЕ использовать `ACAPI_SetPreferences` — пишет в каждый проект → Teamwork конфликт
- `ReadSyncSettingsFromFile` отвергает файл с mismatched `PreferencesVersion`
- При добавлении настроек: увеличить `PreferencesVersion` в SyncSettings.dat
