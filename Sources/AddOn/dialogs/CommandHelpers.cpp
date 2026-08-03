//------------ kuvbur 2022 ------------
#include "dialogs/CommandHelpers.hpp"

#include "Constants.hpp"
#include "Helpers.hpp"
#include "Sync.hpp"

// -----------------------------------------------------------------------------
// Вспомогательная функция: определяет тип источника/цели и имя из rawName
// -----------------------------------------------------------------------------
static void ExtractSourceInfo (const GS::UniString &rawName, GS::UniString &sourceType, GS::UniString &sourceName) {
    sourceType = "";
    sourceName = "";

    if (rawName.BeginsWith (PROPERTYNAMEPREFIX)) {
        sourceType = "Property";
        sourceName = rawName.GetSubstring (PROPERTYNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (GDLNAMEPREFIX)) {
        sourceType = "GDL";
        sourceName = rawName.GetSubstring (GDLNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (COORDNAMEPREFIX)) {
        sourceType = "Coord";
        sourceName = rawName.GetSubstring (COORDNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (FORMULANAMEPREFIX)) {
        sourceType = "Formula";
        sourceName = rawName.GetSubstring (FORMULANAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (IDNAMEPREFIX)) {
        sourceType = "ID";
        sourceName = rawName.GetSubstring (IDNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (MATERIALNAMEPREFIX)) {
        sourceType = "Material";
        sourceName = rawName.GetSubstring (MATERIALNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (FILENAMEPREFIX)) {
        sourceType = "File";
        sourceName = rawName.GetSubstring (FILENAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (CLASSNAMEPREFIX)) {
        sourceType = "Classification";
        sourceName = rawName.GetSubstring (CLASSNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (MORPHNAMEPREFIX)) {
        sourceType = "Morph";
        sourceName = rawName.GetSubstring (MORPHNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (INFONAMEPREFIX)) {
        sourceType = "Info";
        sourceName = rawName.GetSubstring (INFONAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (IFCNAMEPREFIX)) {
        sourceType = "IFC";
        sourceName = rawName.GetSubstring (IFCNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (GLOBNAMEPREFIX)) {
        sourceType = "Glob";
        sourceName = rawName.GetSubstring (GLOBNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (ATTRIBNAMEPREFIX)) {
        sourceType = "Attrib";
        sourceName = rawName.GetSubstring (ATTRIBNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (LISTDATANAMEPREFIX)) {
        sourceType = "Listdata";
        sourceName = rawName.GetSubstring (LISTDATANAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (ELEMENTNAMEPREFIX)) {
        sourceType = "Element";
        sourceName = rawName.GetSubstring (ELEMENTNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    } else if (rawName.BeginsWith (MEPNAMEPREFIX)) {
        sourceType = "MEP";
        sourceName = rawName.GetSubstring (MEPNAMEPREFIX.GetLength (), rawName.GetLength () - 1);
        sourceName.ReplaceAll (BRACEEND, "");
    }
}

// -----------------------------------------------------------------------------
// Вспомогательная функция: извлекает формат строки из полной команды
// -----------------------------------------------------------------------------
static GS::UniString ExtractFormatString (const GS::UniString &fullCommand) {
    GS::UniString formatStr = "";
    UIndex dotPos = fullCommand.FindLast (CHARFORMULAEND); // ищем после '>'
    if (dotPos != MaxUSize) {
        // Ищем точку после закрывающей скобки формулы
        UIndex formatStart = fullCommand.FindFirst (DOT, dotPos);
        if (formatStart != MaxUSize) {
            UIndex braceEnd = fullCommand.FindFirst (BRACEEND, formatStart);
            if (braceEnd != MaxUSize) {
                formatStr = fullCommand.GetSubstring (formatStart, braceEnd - 1);
            }
        }
    } else {
        // Нет формулы - ищем точку перед закрывающей скобкой
        UIndex formatStart = fullCommand.FindLast (DOT);
        if (formatStart != MaxUSize) {
            UIndex braceEnd = fullCommand.FindFirst (BRACEEND, formatStart);
            if (braceEnd != MaxUSize && braceEnd > formatStart) {
                formatStr = fullCommand.GetSubstring (formatStart, braceEnd - 1);
            }
        }
    }
    return formatStr;
}

// -----------------------------------------------------------------------------
// Вспомогательная функция: извлекает ignore values из параметров
// -----------------------------------------------------------------------------
static GS::Array<GS::UniString> ExtractIgnoreVals (const GS::UniString &parameters) {
    GS::Array<GS::UniString> ignoreVals;
    GS::Array<GS::UniString> parts;
    GS::Array<GS::UniString> scratch;
    UInt32 n = StringSplt (parameters, SEMICOLON, parts, true, &scratch);
    for (UInt32 i = 1; i < n; ++i) { // первый элемент — это источник/цель, остальные — модификаторы
        GS::UniString part = parts[i];
        part.Trim ();
        if (part == "empty" || part == "trim_empty" || part == "def" || part == "NULL") {
            ignoreVals.Push (part);
        }
    }
    return ignoreVals;
}

// -----------------------------------------------------------------------------
// Вспомогательная функция: извлекает тип и имя цели (target) из fullCommand
// Для SYNC_TO / SYNC_TO_SUB цель находится внутри фигурных скобок после префикса
// -----------------------------------------------------------------------------
static void ExtractTargetInfo (const GS::UniString &fullCommand, GS::UniString &targetType, GS::UniString &targetName) {
    targetType = "";
    targetName = "";

    // Извлекаем содержимое скобок
    UIndex braceStart = fullCommand.FindFirst (CHARBRACESTART);
    UIndex braceEnd = fullCommand.FindFirst (CHARBRACEEND, braceStart);
    if (braceStart == MaxUSize || braceEnd == MaxUSize) {
        return;
    }

    GS::UniString inside = fullCommand.GetSubstring (braceStart + 1, braceEnd - 1);
    // Параметры разделены ; - первый параметр это цель для TO команд
    GS::Array<GS::UniString> parts;
    GS::Array<GS::UniString> scratch;
    UInt32 n = StringSplt (inside, SEMICOLON, parts, true, &scratch);
    if (n == 0) {
        return;
    }

    GS::UniString targetParam = parts[0];
    targetParam.Trim ();

    // Определяем тип цели по префиксу
    if (targetParam.BeginsWith (PROPERTYPREF)) {
        targetType = "Property";
        targetName = targetParam.GetSubstring (PROPERTYPREF.GetLength (), targetParam.GetLength () - 1);
    } else if (targetParam.BeginsWith (GDLNAMEPREFIX)) {
        targetType = "GDL";
        targetName = targetParam.GetSubstring (GDLNAMEPREFIX.GetLength (), targetParam.GetLength () - 1);
    } else if (targetParam.BeginsWith (COORDPREF)) {
        targetType = "Coord";
        targetName = targetParam.GetSubstring (COORDPREF.GetLength (), targetParam.GetLength () - 1);
    } else if (targetParam.BeginsWith (CLASSPREF)) {
        targetType = "Classification";
        targetName = targetParam.GetSubstring (CLASSPREF.GetLength (), targetParam.GetLength () - 1);
    } else if (targetParam.BeginsWith (ATTRIBPREF)) {
        targetType = "Attrib";
        targetName = targetParam.GetSubstring (ATTRIBPREF.GetLength (), targetParam.GetLength () - 1);
    } else if (targetParam.BeginsWith ("{id}") || targetParam.BeginsWith ("{ID}")) {
        targetType = "ID";
        targetName = "id";
    }
}

// -----------------------------------------------------------------------------
// Основная функция: парсит описание и возвращает структурированные правила
// -----------------------------------------------------------------------------
ParsePropertyResult ParsePropertyDescriptionToRules (const GS::UniString &description) {
    ParsePropertyResult result;

    if (description.IsEmpty ()) {
        return result;
    }

    // Используем существующую функцию для базового парсинга
    GS::Array<ParsedPropertyCommand> commands;
    GS::UniString remainingText;
    ParsePropertyDescription (description, commands, remainingText);

    result.remainingText = remainingText;

    // Обрабатываем найденные команды
    for (const auto &cmd : commands) {
        if (cmd.commandType == "Sync" && cmd.isValid) {
            // Это команда синхронизации — разбираем подробно
            SyncRuleInfo ruleInfo;
            ruleInfo.commandType = "Sync"; // будет уточнен ниже
            ruleInfo.fullCommand = cmd.fullCommand;
            ruleInfo.parameters = cmd.parameters;

            // Определяем направление синхронизации по префиксу fullCommand
            GS::UniString fullCmdLower = cmd.fullCommand.ToLowerCase ();
            if (fullCmdLower.BeginsWith ("sync_from{")) {
                ruleInfo.commandType = "Sync_from";
            } else if (fullCmdLower.BeginsWith ("sync_to{")) {
                ruleInfo.commandType = "Sync_to";
            } else if (fullCmdLower.BeginsWith ("sync_from_sub{")) {
                ruleInfo.commandType = "Sync_from_sub";
                ruleInfo.hasSub = true;
            } else if (fullCmdLower.BeginsWith ("sync_to_sub{")) {
                ruleInfo.commandType = "Sync_to_sub";
                ruleInfo.hasSub = true;
            } else if (fullCmdLower.BeginsWith ("sync_from_guid{")) {
                ruleInfo.commandType = "Sync_from_GUID";
                ruleInfo.hasGUID = true;
            } else if (fullCmdLower.BeginsWith ("sync_to_guid{")) {
                ruleInfo.commandType = "Sync_to_GUID";
                ruleInfo.hasGUID = true;
            }

            // Используем SyncString для детального разбора параметров
            ParamValue param;
            SkipValues ignorevals;
            FormatString stringformat;
            SyncMode syncdirection = SYNC_NO;
            API_ElemTypeID elementType = API_ObjectID;

            bool ok = SyncString (
                elementType, cmd.fullCommand, syncdirection, param, ignorevals, stringformat, true, false, false);

            if (ok) {
                ruleInfo.isValid = true;

                // Источник (from) - определяется по флагам в param
                if (param.fromProperty) {
                    ExtractSourceInfo (param.rawName, ruleInfo.sourceType, ruleInfo.sourceName);
                } else if (param.fromGDLparam) {
                    ruleInfo.sourceType = "GDL";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (GDLNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromCoord) {
                    ruleInfo.sourceType = "Coord";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (COORDNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.val.hasFormula) {
                    ruleInfo.sourceType = "Formula";
                    ruleInfo.sourceName = param.val.uniStringValue;
                } else if (param.fromID) {
                    ruleInfo.sourceType = "ID";
                    ruleInfo.sourceName = "id";
                } else if (param.fromMaterial) {
                    ruleInfo.sourceType = "Material";
                    ruleInfo.sourceName = param.val.uniStringValue;
                } else if (param.fromFile) {
                    ruleInfo.sourceType = "File";
                    ruleInfo.sourceName = param.val.uniStringValue;
                } else if (param.fromClassification) {
                    ruleInfo.sourceType = "Classification";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (CLASSNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromMorph) {
                    ruleInfo.sourceType = "Morph";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (MORPHNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromInfo) {
                    ruleInfo.sourceType = "Info";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (INFONAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromIFCProperty) {
                    ruleInfo.sourceType = "IFC";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (IFCNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromGlob) {
                    ruleInfo.sourceType = "Glob";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (GLOBNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromAttribElement) {
                    ruleInfo.sourceType = "Attrib";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (ATTRIBNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromListData) {
                    ruleInfo.sourceType = "Listdata";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (LISTDATANAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromElement) {
                    ruleInfo.sourceType = "Element";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (ELEMENTNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                } else if (param.fromMEP) {
                    ruleInfo.sourceType = "MEP";
                    ruleInfo.sourceName =
                        param.rawName.GetSubstring (MEPNAMEPREFIX.GetLength (), param.rawName.GetLength () - 1);
                    ruleInfo.sourceName.ReplaceAll (BRACEEND, "");
                }

                // Цель (to) - для Sync_to и Sync_to_sub
                if (syncdirection == SYNC_TO || syncdirection == SYNC_TO_SUB) {
                    ExtractTargetInfo (cmd.fullCommand, ruleInfo.targetType, ruleInfo.targetName);
                }

                // Формат строки
                ruleInfo.formatString = stringformat.stringformat;

                // Ignore values
                if (ignorevals.skip_empty)
                    ruleInfo.ignoreVals.Push ("empty");
                if (ignorevals.skip_trim_empty)
                    ruleInfo.ignoreVals.Push ("trim_empty");
                if (ignorevals.reset_to_def)
                    ruleInfo.ignoreVals.Push ("def");
                for (const auto &val : ignorevals.ignorevals)
                    ruleInfo.ignoreVals.Push (val);

                // GUID source property для from_GUID/to_GUID
                if (ruleInfo.hasGUID) {
                    // Имя свойства-источника GUID находится в параметрах перед первым ';'
                    GS::Array<GS::UniString> parts;
                    GS::Array<GS::UniString> scratch;
                    UInt32 n = StringSplt (cmd.parameters, SEMICOLON, parts, true, &scratch);
                    if (n >= 1) {
                        GS::UniString guidProp = parts[0];
                        guidProp.Trim ();
                        ruleInfo.guidSourceProperty = guidProp;
                    }
                }
            } else {
                ruleInfo.isValid = false;
                ruleInfo.errorMessage = "SyncString не смог распарсить команду";
            }

            result.syncRules.Push (std::move (ruleInfo));
            result.hasSyncRules = true;
        } else {
            // Остальные команды (Renum, Sum, Spec) — оставляем как есть
            result.otherCommands.Push (cmd);
            result.hasOtherCommands = true;
        }
    }

    return result;
}