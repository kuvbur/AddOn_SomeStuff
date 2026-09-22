# dialogs/SyncSettings — Настройки синхронизации

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Хранение и сериализация настроек в ЛОКАЛЬНОМ файле (`…/GRAPHISOFT/SomeStuffAddonConfig.json`), НЕ в preferences проекта (ACAPI_SetPreferences ломает Teamwork). [из комментария, SyncSettings.hpp:5-11]

## Ключевые типы
- `SyncSettings` (GS::Object): флаги syncAll, syncMon, wallS, widoS, objS, cwallS, logMon, showpalette, catchSelectionChanges, maxSelectionCount, filterPresets [из комментария, hpp:26-104]
- `FilterPreset` {label, query} [из комментария, hpp:16-19]

## Публичный API

| Функция | Назначение |
|---------|------------|
| `CreateDefault` / `CreateWithSyncAll` | Стандартный набор / +syncAll [из комментария] |
| `Read` / `Write` | Сериализация (GS::I/OChannel) [из комментария] |
| `Get*/Set*` | Доступ к каждому флагу (тривиальные, одной строкой) [из комментария] |
| `GetSyncSettingsCache(forceReload)` | Единственный экземпляр настроек из локального файла [из комментария, hpp:119] — карточка |
| `LoadSyncSettingsFromPreferences` | Загрузка из кэша [из комментария, hpp:123] |
| `WriteSyncSettingsToPreferences` | Сохранение в локальный файл (без записи в план) [из комментария, hpp:127] |

## Карточки

### `GetSyncSettingsCache(bool forceReload) -> SyncSettings &`
- Расположение: `Sources/AddOn/dialogs/SyncSettings.cpp` (строка не проверена)
- Назначение: единственный экземпляр настроек, загруженный из локального файла. [из комментария, hpp:115-119]
- Контракт: forceReload — перечитать локальный файл; `ReadSyncSettingsFromFile` отвергает файл с mismatched `PreferencesVersion` (AGENTS.md §6 — bump версии при новых настройках). [из AGENTS.md + по коду]
- Побочные эффекты: глобальное состояние (singleton); чтение файла. [по коду]

## Инварианты
- НЕ использовать `ACAPI_SetPreferences` (Teamwork) [из комментария, hpp:8-10]
- Новые поля настроек → увеличить `PreferencesVersion` [из AGENTS.md §6]
- AC25: `MemoryIChannel = GS::MemoryIChannel`; AC26+: `IO::MemoryIChannel` [по коду, hpp:106-112]
