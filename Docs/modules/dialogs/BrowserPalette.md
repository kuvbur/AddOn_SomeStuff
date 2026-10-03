# dialogs/BrowserPalette — UI Entry Point

> Хеш коммита: 493caf5 (2026-09-22). Номера строк .cpp — выборочно проверены.

## Назначение
Браузерная палитра с HTML-интерфейсом: видимость, свойства, JS-мост, мониторинг выделения. [по коду]

## Файлы
- `dialogs/BrowserPalette.cpp/hpp`
- `RFIX/HTML/Interface_ru.html` — единственный редактируемый исходник интерфейса
- `RFIX/HTML/Interface_en.html` — генерируется из RU, руками не правится
- `RFIX/HTML/i18n/` — каталог переводов (`en.json`), глоссарий, карта строк
- `Tools/localize_html.js` — status/generate/check/export-pending

## Локализация
- `LoadHtmlFromResource` берёт `ID_ADDON_HTML + isEng()`. `isEng()` возвращает
  1000 для не-русского языка, поэтому `1100 + 1000 = 2100 = ID_ADDON_HTML_ENG`
  — переключение языка уже корректно, отдельного кода выбора не требуется. [по коду]
- Строки интерфейса лежат в `I18N_CATALOG` (внутри HTML) и берутся через
  `t(key, params)`; отсутствующий ключ пишет в `console.error` и показывает
  исходный русский текст. [по коду]
- Правка русской строки помечает запись EN как `stale` — перевод требует
  явного пересмотра, автоматического подтверждения нет. [по коду]
- Сообщения из моста (`parseErrorText`, `sourceName`) не переводятся — они
  приходят из C++; расширение моста отдельная задача. [по коду]

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ShowOrHideBrowserPalette` | Переключение видимости палитры [из комментария, hpp:23]; вызывается из `MenuCommandHandler` (Main:469), определение BrowserPalette.cpp:37 [из callgraph.json] |
| `BrowserPalette::Show` | Показ; reloadContent=false — без перезагрузки HTML (APIPalMsg_HidePalette_End; перезагрузка сбрасывала вкладку/фильтр) [из комментария, hpp:106-109; .cpp:127] |
| `BrowserPalette::Hide` | Скрытие [из комментария, .cpp:169] |
| `BrowserPalette::ManualGetSelection` | Ручное получение выделения [из комментария, hpp:112] |
| `BrowserPalette::SelectionChangeHandler` | Обработчик изменения выделения [из комментария, hpp:115] — карточка |
| `UpdateSelectionInfoInUI` | Обновление представления выделения в HTML [из комментария, .cpp:175] |
| `RegisterACAPIJavaScriptObject` | Регистрация JS объекта для вызовов из HTML [из комментария, hpp:49] |

## Карточки

### `BrowserPalette::SelectionChangeHandler(const API_Neig *) -> GSErrCode`
- Расположение: `Sources/AddOn/dialogs/BrowserPalette.cpp` (строка не проверена)
- Назначение: обработка смены выделения, обновление UI. [из комментария]
- Контракт: защищён `suppressSelectionRefresh` — при программной подсветке (`APIIo_HighlightElementsID`/`APIDo_ZoomToElementsID`) обновление подавляется. [из комментария, hpp:91-95]
- Побочные эффекты: обновление HTML (execute JS). [по коду]

### `BrowserPalette::Show(bool reloadContent)`
- Расположение: `Sources/AddOn/dialogs/BrowserPalette.cpp:127`
- Назначение: показ палитры; при reloadContent перечитывает HTML и обновляет выделение (UpdateSelectionInfoInUI, .cpp:163). [по коду]
- Побочные эффекты: показ окна; при reloadContent — сброс активной вкладки/фильтра (FIX). [из комментария + по коду]

## Инварианты
- Palette resize: `UnDock()` → `SetClientWidth()` → `Dock()`; min width ослаблять перед сжатием (AGENTS.md §6) [из AGENTS.md]
- JS bridge: `DynamicCast<JSValue>` — `DynamicCast<JSArray>` крашит ArchiCAD (AGENTS.md §6) [из AGENTS.md]
- Поля ширины: expandedClientWidth/expandedMinClientWidth/collapsedClientWidth (hpp:74-88) [из комментария]
- «Монитор»: закрепления свойств (`pinnedProperties`) и групп (`pinnedPropertyGroups`) сессионные; после локальной фильтрации закреплённые свойства идут первой синтетической группой, затем закреплённые группы без дублирования свойств. [по коду, HTML #160/#204]
- Инлайн-кнопка сброса в строке свойства временно отсутствует; мост `resetPropertyToDefault` и иконка цепочки сохранены для отдельного этапа режима выборочного сброса #189. [по коду, HTML #189]
- Версионные ветки моста и подсветки (проверено сборкой AC26, #170): подсветка — AC25 `APIIo_HighlightElementsID` (единственная ветка с кодом ошибки), AC26 `ACAPI_Interface_Clear/SetElementHighlight`, AC27+ `ACAPI_UserInput_*` (в обеих новых ветках SDK-функции возвращают `void`, поэтому `hlErr` остаётся `NoError`); `browser.UnregisterJSObject` удалён из DGLib в AC26 и вернулся в AC27 (перегрузка по имени, модуль `JavascriptEngine`); вызов компилируется под `#if defined(ServerMainVers_2700) || !defined(ServerMainVers_2600)`. [по коду DevKit + сборка AC26]
- `FilterElementsByType` получает тип элемента через `GetElemTypeID(head)` (CommonFunction), а не через `head.typeID`: в AC26 поле переименовано в `API_Elem_Head::type` (`API_ElemType`). [по коду DevKit + сборка AC26]
- AC27 (`#170`): JS-мост переехал из DGLib в модуль `JavascriptEngine` — `DG::JSBase/JSFunction/JSObject/JSArray/JSValue` стали `JS::Base/Function/Object/Array/Value`; в `BrowserPalette.hpp` под `#ifdef AC_27` объявлены псевдонимы в `namespace DG`, поэтому тела функций моста не менялись. [по коду DevKit + сборка AC27]
- AC27: `DGModule.hpp` больше не включает `DGBrowser.hpp` — браузерный контрол подключается явно (`BrowserPalette.hpp`). [по коду DevKit + сборка AC27]

## Открытие сайта (AC22–29)
- Функция моста `OpenWebsite` (строка) — аргумент одиночный `DynamicCast<DG::JSValue>`, ответ `DG::JSValue(bool)`; nullptr не возвращается (роняет CEF-мост). [по коду]
- В SDK функции «открыть URL» нет: в `APIdefs_Interface.h` AC22–29 только файловые диалоги (`APIIo_OpenLibPartFileDialogID`/`OpenPictureDialogID`/`OpenOtherObjectDialogID`); в документации AC25 (898 функций) про URL только `APIDb_Get/SetElementURLRefID` — ссылка на элемент, не браузер. Проверено LightRAG (`hybrid`, `naive` ×3) + перебор заголовков AC25 и AC29. [по коду DevKit + LightRAG]
- Отдельного `<a href>` в HTML быть не может: палитра грузится `LoadHTML` из ресурса, базового URL нет, а переход увёл бы панель на сайт; `Show(bool)` перезагружает HTML через `ReloadIgnoreCache` и теряет состояние вкладки/фильтра. [по коду .cpp:273, .cpp:336]
- Windows: `ShellExecuteW(nullptr, L"open", …, SW_SHOWNORMAL)`, успех — результат `> 32` (при ошибке возвращается код, а не HINSTANCE). Адрес приводится через `address.ToUStr()` — тип результата **нельзя назвать**: `UStr` вложен в приватную секцию `UniString`, поэтому только `auto`. [по заголовку Windows SDK 10.0.26100 shellapi.h:96 + диагностика clangd]
- macOS: `GS::Process::Create("open", {address})`; процесс не ждём — синхронного признака успеха у `GS::Process` нет, `IsValid()` отражает только запуск. **`not verified` вживую — mac-машины нет.** [по заголовку Process.hpp AC22–29]
- Линковка не требует правок: `shell32.lib` уже в `CMAKE_CXX_STANDARD_LIBRARIES`, `Cocoa` подключается в `CMakeCommon.cmake:235-246`. [по кэшу CMake + CMakeCommon.cmake]

## Версии (AC28)
- AC28: `__ACENV_CALL` удалён из SDK (`APICalls.h` в 27, в 28 макроса нет) — объявления/определения `PaletteControlCallBack` и `SelectionChangeHandler` обёрнуты `#ifdef ServerMainVers_2800` (без макроса) / `#else`. [по коду DevKit-28 + сборка]
- AC28: `GS::HashTable::CurrentPair::value` стал ссылкой `Value&` (в ≤27 — `Value*`) — в блоке разбора классификаций это `classPair.value.item`, `&sysPair.value` под `#ifdef ServerMainVers_2800`. [по коду DevKit-28 + сборка]
- Псевдонимы `DG::JS* → JS::*` (AC27 перенёс JS-мост в модуль JavascriptEngine) теперь под `#ifdef ServerMainVers_2700`, а не под `AC_27`: в AC28 определён `AC_28`, и ветка `AC_27` не срабатывала — сборка AC28 падала на `"JSObject": не является членом "DG"`. [по коду DevKit-28 + сборка]

