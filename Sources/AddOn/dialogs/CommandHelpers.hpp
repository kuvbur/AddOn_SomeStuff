//------------ kuvbur 2022 ------------
#if !defined(COMMANDHELPERS_HPP)
    #define COMMANDHELPERS_HPP

    #include "Helpers.hpp"
    #include "Sync.hpp"

// -----------------------------------------------------------------------------
// Структура для представления правила синхронизации в интерфейсе
// -----------------------------------------------------------------------------
struct SyncRuleInfo {
    GS::UniString
        commandType; // "Sync_from", "Sync_to", "Sync_from_sub", "Sync_to_sub", "Sync_from_GUID", "Sync_to_GUID"
    GS::UniString fullCommand; // Полная команда с фигурными скобками
    GS::UniString parameters;  // Параметры внутри скобок
    GS::UniString
        sourceType; // Тип источника: "Property", "GDL", "Coord", "Formula", "ID", "Material", "File", "Classification",
                    // "Morph", "Info", "IFC", "Glob", "Class", "Attrib", "Listdata", "Element", "MEP"
    GS::UniString sourceName;            // Имя источника (без префикса типа)
    GS::UniString targetType;            // Тип цели: "Property", "GDL", "Coord", "Classification", "Attrib", "ID"
    GS::UniString targetName;            // Имя цели (без префикса типа)
    GS::UniString formatString;          // Формат строки (например, ".3m", ".3mp", "0mm")
    GS::Array<GS::UniString> ignoreVals; // Игнорируемые значения
    bool isValid = false;                // Успешно ли распарсилось правило
    GS::UniString errorMessage;          // Сообщение об ошибке, если isValid == false
    bool hasSub = false;                 // Есть ли ссылка на подэлементы (from_sub/to_sub)
    bool hasGUID = false;                // Есть ли ссылка на другой элемент по GUID (from_GUID/to_GUID)
    GS::UniString guidSourceProperty;    // Имя свойства-источника GUID (для from_GUID/to_GUID)
};

// -----------------------------------------------------------------------------
// Результат парсинга описания свойства
// -----------------------------------------------------------------------------
struct ParsePropertyResult {
    GS::Array<SyncRuleInfo> syncRules;              // Найденные правила синхронизации
    GS::Array<ParsedPropertyCommand> otherCommands; // Остальные команды (Renum, Sum, Spec)
    GS::UniString remainingText;                    // Текст, не вошедший в команды
    bool hasSyncRules = false;                      // Есть ли правила синхронизации
    bool hasOtherCommands = false;                  // Есть ли другие команды
};

// -----------------------------------------------------------------------------
// Парсит описание свойства и возвращает структурированные правила синхронизации
// для использования в интерфейсе (BrowserPalette).
// Вызывает ParsePropertyDescription и дополнительно разбирает Sync-команды
// на отдельные поля для удобного отображения/редактирования.
// -----------------------------------------------------------------------------
ParsePropertyResult ParsePropertyDescriptionToRules (const GS::UniString &description);

#endif // COMMANDHELPERS_HPP