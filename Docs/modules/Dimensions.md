# Dimensions — Округление размеров

> Хеш коммита: f8f599c (2026-09-22)

## Назначение модуля
Округление размеров: правила форматирования (читаются из информации о проекте, свойство `Addon_Dimenstions`), обработка текста размеров и привязка к типам элементов. [из комментария, Dimensions.hpp:9]

## Файлы модуля
- `Sources/AddOn/Dimensions.cpp/hpp`

## Публичный API

| Функция | Сигнатура | Назначение |
|---------|-----------|------------|
| `DimAutoRoundOne` | `(const API_Guid&, const SyncSettings&, bool checktype) -> GSErrCode` | Обрабатывает один элемент и применяет к его размерам правила округления [из комментария] |
| `DimAutoRound` | `(const API_Guid&, const SyncSettings&) -> GSErrCode` | Обрабатывает один размер: менять текст/цвет или сбросить формат [из комментария] |
| `DimParse` | `(const double& dimVal, const API_Guid&, const API_NoteContentType&, const GS::UniString& content, GS::UniString& custom_txt, UInt32& flag_change, UInt32& flag_highlight, const DimRule&, const ParamDictValue* preadelem) -> bool` | Разбирает значение размера и формирует текст по правилам; flags: DIM_CHANGE_ON/OFF/NOCHANGE, DIM_HIGHLIGHT_ON/OFF/NOCHANGE [из комментария] |
| `DimRoundAll` | `(const SyncSettings&, bool isUndo)` | Округление всех доступных элементов согласно настройкам [из комментария] |
| `DimRoundByType` | `(const API_ElemTypeID&, const SyncSettings&) -> bool` | Округление размеров одного типа элементов [из комментария] |

## Зависимости
- `DG.h`, `dialogs/SyncSettings.hpp`, `Helpers.hpp`

## Зависимости (используется в)
- `SomeStuff_Main` (команды меню), `Propertycache` (DimReadPref/DimParsePref — правила читаются в кэш)

## Инварианты и подводные камни
- `Dimensions.cpp:158` — `pen_original`: зафиксировано как «не чинить без явного запроса» (AGENTS.md §16) [из AGENTS.md]
- FIX (ревью 2026-09-12, Dimensions.cpp-1): `preadelem` — предпрочитанный словарь параметров привязанного элемента, читается один раз на размер; `nullptr` — прежнее поведение чтения внутри [из комментария, Dimensions.hpp:26-32]
- FIX: п.32 — полная копия `dimrule.paramDict` больше не создаётся на каждый размер; п.62 — входной `content` не мутируется, результат через out-параметр `custom_txt` [из комментария, Dimensions.hpp:29-32]