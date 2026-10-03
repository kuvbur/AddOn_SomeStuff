# Глоссарий локализации — BrowserPalette

> Источники проверены на машине 2026-10-03, Archicad 25 (`C:\Program Files\GRAPHISOFT\ArchiCAD 25`).
> Уровень доказательности для каждой строки указан явно.

## Правило доказательности

| Уровень | Что означает | Как использовать |
|---|---|---|
| **A — официальный** | Строка из поставки AC25 или официальной документации Graphisoft | брать как есть |
| **B — доменное имя API** | Имя символа в AC25 DLL | уверенно, но это имя в коде, не подпись в UI |
| **C — рабочее решение** | Термин SomeStuff, смысл выведен из функции | согласовать с владельцем |

**Чего получить не удалось** и почему:
- `help.graphisoft.com/25/0/en/` отдаёт **404** (проверено `curl` и панелью предпросмотра). Угадывать GUID-ссылки нельзя.
- Строки интерфейса AC25 лежат в хешированном ресурсе `STRS`/`MDID` — при прямом чтении получаются хеши, а не текст. `RT_STRING` в DLL содержит только маркер `STRS`.
- MUI-файлы `ru-RU\*.dll.mui` содержат **только** version-info (27–31 строка, кириллицы 0) — интерфейсных строк там нет.

Поэтому термины ниже опираются на **поставку AC25**, а не на веб-справку.

## Доменные имена AC25 (уровень B)

Источник: деманглированные символы `ACUserInterface.dll`.

| Термин | Символы AC25 | Считать |
|---|---|---|
| Property | `VUserDefinedPropertyDefinition@Property` | `Property` |
| Classification | `VClassificationSystemUserID@CLS`, `ClassificationItemUserID` | `Classification` |
| Favorite | `FavoriteList`, `FavoriteInfo`, `FavoritePopupItemData` | `Favorite` / `Favorites` |
| Story | `GeneralHomeStoryPropertyId`, `ActualStoryChangeNotifier` | `Story` |
| Building Material | `GeneralBuildingMaterialsPropertyId`, `GetBuildingMaterialName` | `Building Material` |
| Layer | `GetLayer`, `GeneralLayerIndexPropertyId` | `Layer` |
| Zone | `GeneralCollidingZonesNamesPropertyId` | `Zone` |
| Palette | `ILoadCasePaletteCallbackInterface` и др. | `Palette` |
| Surface | `SurfaceLoadSettingsPage` — про `Surface Load`, не про отделку | термин не подтверждён как подпись |

`Floor` в AC25 — это **Floor Plan** (чертёж этажа), **не** Story.
`Этаж` = `Story`, а не `Floor`. Это критично: `Floor Type` в официальных правилах
COBie (`Значения по умолчанию\IFC Rules\COBie Floor and Zone Types.xml`) относится
к `IfcBuildingStorey`, то есть к Story, а не к Floor Plan.

## Официальные подписи (уровень A)

Источник: `Значения по умолчанию\IFC Rules\*.xml` — поставляемые правила AC25,
подписи полей свойств (`Title=`):

`Code` · `Common Space Type` · `Description` · `Floor Type` · `Number` ·
`OmniClass Number` · `OmniClass Title` · `Title` · `Uniclass Number` ·
`Uniclass Title` · `Value` · `Zone Type`

Из них берутся термины: `Story` (через Floor Type → IfcBuildingStorey),
`Zone`, `Description`, `Number`, `Title`, `Value`, `Code`,
`Classification Reference` (тот же XML, тег `<CreateClassificationReference>`).

## Решения по терминам SomeStuff

| RU | EN | Уровень | Обоснование |
|---|---|---|---|
| Свойство | Property | A/B | доменное имя AC25 |
| Классификация | Classification | A/B | доменное имя + официальное правило |
| Избранное (набор) | Favorites | B | `FavoriteList` |
| избранное (объект) | Favorite | B | `FavoriteInfo` |
| Элемент по умолчанию | Default Element | C | формулировка SomeStuff, в SDK — `DefaultElem` |
| Этаж | Story | A/B | **не** Floor |
| Слой | Layer | B | |
| Зона | Zone | A/B | |
| Палитра | Palette | B | |

### Спорные — требуют решения владельца

| RU | Кандидат | Проблема |
|---|---|---|
| Спецификация | **Schedule** | `Interactive Schedule` — отдельная функция Archicad с другим смыслом. Вкладка SomeStuff проверяет правила записи и не строит интерактивную ведомость. `SchedulePreviewRefresh` — единственное символьное вхождение в AC25, слабый источник. |
| Суммирование | Summation / Summary | В AC25 нет ни одного символа с `Summation`. Смысл функции SomeStuff — сумма по группам (`Sum{…}`). |
| Нумерация | Renumber | В AC25 нет символов с `Renumber`. Термин SomeStuff (`Renum{…}`). |
| Синхронизация | Sync | Термин SomeStuff (`Sync_{…}`), официального аналога нет. |
| Покрытие | Surface | Единственное подтверждение — `Surface Load`, другое значение. Не подтверждено. |
| Строительный материал | Building Material | `GeneralBuildingMaterialsPropertyId`, `GetBuildingMaterialName` — подтверждено именем. |
| Правило / Флаг | Rule / Flag | Понятия SomeStuff, не Archicad. `Sync_flag`, `Renum_flag` — имена команд. |

**Решение по умолчанию, если владелец не ответит:** термины SomeStuff оставляются
как есть (`Sync`, `Renum`, `Sum`, `Schedule`), потому что они совпадают с именами
команд DSL, и пользователь увидит одинаковое слово в интерфейсе и в скрипте.
Это осознанное решение, а не молчаливое угадывание.

## Стиль

- Заголовки вкладок и кнопок — Title Case: `Specification`, `Check`, `Apply to All`.
- Строки состояний — предложение с точкой: `No errors found.`
- Многоточие — `…` (U+2026), как в русском оригинале, не `...`.
- Плейсхолдеры — без точки, с многоточием: `Filter by names… (or /regex/)`.
- `aria-label` — краткое существительное, как в русском оригинале.
- Технические значения (`NULL`, `SPACE`, `ALLNULL`, `без суффикса`) — **не
  переводятся**: они попадают в генерируемое правило.