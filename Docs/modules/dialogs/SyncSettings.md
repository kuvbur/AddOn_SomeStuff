# dialogs/SyncSettings — Настройки синхронизации

> База уточнения preferences: f4e0054; обновление #273 (2026-10-14). Остальные исторические координаты ниже не пересобирались.

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
| `LoadSyncSettingsFromPreferences` | Загрузка из кэша [из комментария]; вызывается из `MenuCommandHandler` (Main:372), сама вызывает `GetSyncSettingsCache` (:432) [из callgraph.json] |
| `WriteSyncSettingsToPreferences` | Сохранение в локальный файл (без записи в план) [из комментария]; вызывается из `MenuCommandHandler` (Main:494) [из callgraph.json] |

## Карточки

### `GetSyncSettingsCache(bool forceReload) -> SyncSettings &`
- Расположение: clangd не резолвит callHierarchy на определении (2 попытки; grep: SyncSettings.cpp:419)
- Назначение: единственный экземпляр настроек, загруженный из локального файла. [из комментария, hpp:115-119]
- Контракт: forceReload — перечитать локальный файл; `ReadSyncSettingsFromFile` отвергает файл с mismatched `PreferencesVersion` (AGENTS.md §6 — bump версии при новых настройках). [из AGENTS.md + по коду]
- Вызывает: `ReadSyncSettings` (чтение локального файла). [по коду, grep тела — body_scan.json]
- Вызывается из: `LoadSyncSettingsFromPreferences` (SyncSettings.cpp:432), `WriteSyncSettingsToPreferences` (:442). [по коду]
- Побочные эффекты: глобальное состояние (singleton); чтение файла. [по коду]

## Инварианты
- Текущие настройки сохранять только в локальный JSON; `ACAPI_SetPreferences` используется исключительно для очистки непустого legacy payload, с проверкой результата чтением. Поведение на Teamwork не считать доказанным документацией. [по коду; runtime Teamwork не проверен]
- Новые поля настроек → увеличить `PreferencesVersion` [из AGENTS.md §6]
- AC25: `MemoryIChannel = GS::MemoryIChannel`; AC26+: `IO::MemoryIChannel` [по коду, hpp:106-112]
- `#pragma clang diagnostic` вокруг rapidjson закрыты `#ifdef __clang__`: MSVC не знает прагм clang (C4068), при `/WX` это ошибка. До AC25 предупреждение глушил заголовок DevKit (`APIdefs_AnalyticalModel.h`), в AC26 такого глушения нет — без `#ifdef` файл не собирался под AC26 [по коду; сборка AC26, #170]
## Очистка старых preferences

### `ClearLegacyProjectPreferences() -> GSErrCode`
- Расположение: `Sources/AddOn/dialogs/SyncSettings.cpp:402`; объявление в `SyncSettings.hpp`. [по текущему коду]
- Контракт: `ACAPI_GetPreferences_Platform` проверяет размер без разбора blob. При размере 0 запись не выполняется; иначе `ACAPI_SetPreferences(0, 0, nullptr)` и повторное чтение. Ошибки SDK возвращаются; ненулевой остаточный размер даёт `APIERR_GENERAL`. Нулевая версия сама по себе не означает пустые данные. [по коду]
- Вызывается из: `ProjectEventHandlerProc` при New/NewAndReset/Open; также при ChangeProjectDB/ChangeWindow/ChangeFloor только под TESTING. Обработчик возвращает `err`; основная ветка сообщает ошибку английским `msg_rep`. [по коду]
- Побочные эффекты: только legacy preferences; локальный `SomeStuffAddonConfig.json`, user data, ElementSet, AddOnObject, модель, слои и права не меняются. Полная очистка файла от всех данных аддона не обещается. [по коду; уточнённый scope владельца]
- Проверка: canonical AC25 Windows Debug build success; 8/8 SDK-double checks, извлечение сверено с текущим helper. Финальный runner выполнен владельцем по его сообщению; собственный повтор отменён по указанию владельца после защитного exit20. Host/read-back после save/reopen независимо не наблюдались. [build log + выполненный стенд + сообщение владельца; persisted runtime not verified]
- Clangd callHierarchy не разрешил два точных положения; новая запись generated использует MCP имя/вид и координату текущего source. Полнота графа и остальные старые координаты не заявляются. [MCP + source fallback]

## Версии
- AC27 (`#170`): `ACAPI_Environment (APIEnv_GetSpecFolderID, …)` → `ACAPI_ProjectSettings_GetSpecFolder` под `#ifdef ServerMainVers_2700`. [по коду DevKit + сборка AC27]
