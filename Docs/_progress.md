# Состояние документации

Регламент: `Docs/tools/UPDATE_PROCEDURE.md`. Правило: `Docs/` правится только перед коммитом, по команде владельца.
Задачи — `IDEA.md`, история — `IDEA_ARCHIVE.md` и git.

## Сейчас

- `Docs/modules/Roombook.md` переведена на имена символов; в остальных карточках осталось 91 ссылка `файл:строка` — миграция по одной карточке за проход.
- `Docs/_generated/callgraph.json`: roombook-срез устарел (собран до выделения `RoombookData.cpp`/`RoombookDraw.cpp`); пересобирать перед использованием.
- `Docs/_generated/symbols.json` — единственный источник координат (clangd MCP, §2 процедуры); `Docs/_generated/source_hashes.json` — хеши 106 исходников.
- `generate_symbols.py` удалён (clangd-клиент без LSP-хендшейка → regex-скан); символы собираются только через clangd MCP.
- `Docs/lifecycle/` — задуманные стадии сквозной обработки; `finishing-wall` ещё не написан.

## Проверка

```bash
python Tools/check_docs.py            # ошибки путей, номера строк в карточках, STALE
python Tools/check_docs.py --update   # перезаписать source_hashes.json
python Tools/check_docs.py --strict   # STALE и номера строк — ошибка
```

Структура и точки входа: `Docs/REPOMAP.md`, `Docs/ARCHITECTURE.md`.
