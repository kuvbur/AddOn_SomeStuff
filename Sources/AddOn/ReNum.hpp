//------------ kuvbur 2022 ------------
#if !defined(RENUM_HPP)
    #pragma once
    #define RENUM_HPP
    #include "ACAPinc.h"

    #include "Helpers.hpp"
    #include "third_party/alphanum.h"

class RenumPos {
  public:
    API_Guid guid = APINULLGuid;
    std::string strpos = ""; // Полный текст позиции
    int numpos = 0;          // Численная часть позиции
    bool isNum = false;      // Есть численное значение
    std::string prefix = ""; // Префикс
    std::string suffix = ""; // Суффикс
    GSCharCode chcode = GChCode;

    RenumPos () {}

    // TODO Добавить парсинг префикса и суффикса позиции
    RenumPos (const ParamValue &param) {
        guid = param.fromGuid;
        if (param.val.canCalculate) {
            isNum = true;
            numpos = param.val.intValue;
        }
        chcode = GetCharCode (param.val.uniStringValue);
        strpos = param.val.uniStringValue.ToCStr (0, MaxUSize, this->chcode).Get ();
    }

    RenumPos (int pos) {
        isNum = true;
        numpos = pos;
        this->setStr ();
    }

    GS::UniString ToUniString () {
        this->setStr ();
        GS::UniString unipos = GS::UniString (this->strpos.c_str (), this->chcode);
        return unipos;
    }

    void Add (const int &i) {
        if (isNum) {
            this->numpos = this->numpos + i;
        } else {
            this->isNum = true;
            this->numpos = i;
            if (this->prefix.empty ())
                this->prefix = "-";
        }
        this->setStr ();
    }

    void FormatToMax (RenumPos &pos, ZeroPaddingMode nulltype, int nullcount) {
        // Длина от начала строки до конца числа должна быть как у pos
        // Для заполнения используем либо нули, либо пробелы
        if (nulltype == NOZEROS)
            return;
        if (!this->isNum)
            return;

        size_t nthis = this->prefix.size () + std::to_string (this->numpos).size ();
        size_t npos = 0;
        if (pos.isNum) {
            npos = pos.prefix.size () + std::to_string (pos.numpos).size ();
        } else {
            npos = pos.strpos.size ();
        }
        if (nullcount > 0 && npos < nullcount) {
            npos = nullcount;
        }
        if (npos > nthis) {
            size_t nadd = npos - nthis;
            char charrepl = ' ';
            if (nulltype == ADDZEROS || nulltype == ADDMAXZEROS)
                charrepl = '0';
            this->prefix = this->prefix + std::string (nadd, charrepl);
            this->setStr ();
        }
    }

    void SetToMax (RenumPos &pos) {
        this->setStr ();
        if (doj::alphanum_comp (this->strpos, pos.strpos) < 0) {
            this->isNum = pos.isNum;
            this->prefix = pos.prefix;
            this->suffix = pos.suffix;
            this->numpos = pos.numpos;
            this->strpos = pos.strpos;
            this->guid = pos.guid;
        }
    }

    void SetPrefix (GS::UniString &prefix) {
        GSCharCode chcode = GetCharCode (prefix);
        this->prefix = prefix.ToCStr (0, MaxUSize, chcode).Get ();
    }

    ParamValue ToParamValue (GS::UniString &rawname) {
        ParamValue posvalue;
        GS::UniString unipos = this->ToUniString ();
        ParamHelpers::ConvertStringToParamValue (posvalue, rawname, unipos);
        return posvalue;
    }

    // const обязателен: с AC29 проект собирается как C++20, а для неконстантного
    // operator== компилятор добавляет перевёрнутый кандидат (y == x) — сравнение
    // двух объектов становится неоднозначным (MSVC C2666).
    bool operator== (const RenumPos &b) const {
        if (this->isNum == b.isNum && this->isNum) {
            if (this->prefix != b.prefix)
                return false;
            if (this->suffix != b.suffix)
                return false;
            if (this->numpos != b.numpos)
                return false;
        } else {
            if (this->strpos != b.strpos)
                return false;
        }
        return true;
    }

    // std::size_t operator()(const RenumPos& k) const
    //{
    //	std::size_t res = 17;
    //	res = res * 31 + std::hash<std::string>()(this->strpos);
    //	return res;
    // }

  private:
    void setStr () {
        if (this->isNum) {
            strpos = this->prefix + std::to_string (this->numpos) + this->suffix;
        }
    }
};

struct RenumElem {
    // Список позиций, найденных у элементов правила.
    GS::Array<RenumPos> elements;
    // Наиболее частая позиция среди элементов правила.
    RenumPos mostFrequentPos;
};

struct RenumRule {
    // Признак того, что правило активно и должно применяться.
    bool state = false;
    // Признак использования старого алгоритма расчёта позиции.
    bool oldalgoritm = true;
    // Описание свойства, в которое ставим позицию.
    GS::UniString flag = "";
    // Описание свойства, в которое ставим позицию.
    GS::UniString position = "";
    // Описание свойства-критерия для группировки элементов.
    GS::UniString criteria = "";
    // Описание свойства-разбивки для группировки элементов.
    GS::UniString delimetr = "";
    // Шаблон формулы критерия — то, что стояло в двойных кавычках в Renum{...}.
    // Пусто, если критерий задан обычным свойством.
    GS::UniString criteria_formula = "";
    // Шаблон формулы разбивки. Пусто, если разбивка не задана или задана свойством.
    GS::UniString delimetr_formula = "";
    // Тип постановки нулей или пробелов в позиционном значении.
    ZeroPaddingMode nulltype = NOZEROS;
    // Количество нулей или пробелов, если задано жёсткое количество.
    int nullcount = 0;
    // Массив элементов, участвующих в правиле.
    GS::Array<API_Guid> elemts;
    // GUID свойства, которое описывает правило.
    API_Guid guid = APINULLGuid;
    // Имя свойства-правила для отображения во всплывающем окне.
    GS::UniString rule_name = "";
    // Количество элементов, которые были проигнорированы.
    int n_ignore = 0;
    // Количество элементов, пропущенных по логике правила.
    int n_skip = 0;
    // Количество записанных изменений.
    int n_write = 0;
    // Количество элементов, отброшенных из-за ошибки в свойстве. Отдельно от
    // n_skip: элемент с флагом «пропустить» отброшен по решению правила, а
    // элемент с невалидным значением - по ошибке, и пользователю это различие
    // нужно видеть в отчёте.
    int n_error = 0;
};

typedef std::map<std::string, RenumElem, doj::alphanum_less<std::string>> Values; // Словарь элементов по критериям

typedef std::unordered_map<RenumMode, Values> TypeValues; // Словарь по типам нумерации

typedef std::unordered_map<std::string, TypeValues> Delimetr; // Словарь по разделителю

typedef std::map<std::string, RenumPos, doj::alphanum_less<std::string>> StringDict;
typedef std::map<std::string, StringDict, doj::alphanum_less<std::string>> DStringDict;

typedef std::map<std::string, std::string, doj::alphanum_less<std::string>> RenumPosDict;
typedef std::map<std::string, RenumPosDict, doj::alphanum_less<std::string>> DRenumPosDict;

typedef GS::HashTable<API_Guid, RenumRule> Rules; // Таблица правил

// -----------------------------------------------------------------------------------------------------------------------
// Накопитель результата запуска перенумерации.
//
// Устроен по образцу Spec::SpecRunResult: сообщения копятся и показываются
// ОДНИМ окном в конце запуска (ShowRenumResult). Раньше каждая точка отказа
// писала ACAPI_WriteReport (…, true), то есть открывала своё окно, и при
// нескольких проблемах пользователь получал их по одному.
// -----------------------------------------------------------------------------------------------------------------------

// Сообщение окна результата. ruleName — правило, к которому относится
// сообщение. Пустое имя означает сообщение БЕЗ правила: оно идёт внизу окна и
// не подсвечивает ни одну строку списка.
struct RenumMessage {
    GS::UniString ruleName = EMPTYSTRING;
    GS::UniString text = EMPTYSTRING;
};

// Итог по ОДНОМУ правилу за запуск. Рядом с суммарными счётчиками запуска,
// потому что суммарные не позволяют сказать, какое правило что перенумеровало,
// а именно это и показывает окно.
struct RenumRuleStats {
    UInt32 elements = 0; // элементов под правилом
    UInt32 written = 0;  // элементов с изменённой позицией
    // Отобранные и отказанные элементы. Показываются отдельными колонками,
    // иначе из «Элементов» не видно, куда делась разница: пользователю
    // кажется, что позицию изменили не у всех.
    UInt32 ignored = 0; // флаг «не менять» (RENUM_IGNORE)
    UInt32 skipped = 0; // флаг «пропустить» (RENUM_SKIP)
    UInt32 errors = 0;  // отброшены из-за невалидного значения
};

struct RenumRunResult {
    // Сколько позиций записано по ВСЕМ правилам. Принадлежит запуску, а не
    // отдельному правилу, поэтому в окне идёт общей строкой внизу.
    UInt32 elementsToWrite = 0;

    // Статистика по правилам в порядке регистрации. Параллельные массивы, а не
    // словарь: порядок нужен окну результата, а хеш-таблица его не даёт.
    GS::Array<GS::UniString> ruleNames = {};
    GS::Array<RenumRuleStats> ruleStats = {};

    // Сообщения за запуск, привязанные к правилу и без правила вперемешку, в
    // порядке появления. Окно разбирает их само: строки правил по ruleName,
    // остальные вниз без привязки.
    GS::Array<RenumMessage> messages = {};

    // Регистрирует правило, участвующее в запуске, и возвращает индекс его
    // статистики. Повторная регистрация того же имени НЕ создаёт второй строки:
    // список строк окна строится по этим индексам, а дубль означал бы две
    // строки об одном правиле. Одно правило может быть описано несколькими
    // свойствами, поэтому регистрация повторяется штатно.
    UIndex EnsureRuleStats (const GS::UniString &ruleName) {
        for (UIndex i = 0; i < ruleNames.GetSize (); ++i)
            if (ruleNames[i] == ruleName)
                return i;
        ruleNames.Push (ruleName);
        ruleStats.Push (RenumRuleStats ());
        return ruleNames.GetSize () - 1;
    }

    // Сообщение без правила: идёт внизу окна и не подсвечивает строку.
    void AddGeneralMessage (const GS::UniString &text) {
        RenumMessage message = {};
        message.text = text;
        messages.Push (message);
    }

    // Сообщение, привязанное к правилу. Правило регистрируется при первом
    // обращении, поэтому вызывающему не нужно отдельно объявлять его участие:
    // строка появится в окне вместе с сообщением.
    void AddRuleMessage (const GS::UniString &ruleName, const GS::UniString &text) {
        EnsureRuleStats (ruleName);
        RenumMessage message = {};
        message.ruleName = ruleName;
        message.text = text;
        messages.Push (message);
    }

    // Есть ли у правила хотя бы одно сообщение. Строка правила с ошибкой
    // подсвечивается по этому признаку, а не по счётчикам: нули в колонках при
    // отказе подготовки — законное состояние, и по ним ошибку не увидеть.
    bool HasRuleError (UIndex index) const {
        if (index >= ruleNames.GetSize ())
            return false;
        for (const RenumMessage &message : messages)
            if (message.ruleName == ruleNames[index])
                return true;
        return false;
    }
};

// Свойства, которые не нашлись у элемента, по правилам-владельцам:
// имя правила -> имена отсутствующих свойств. Привязка к правилу обязательна:
// свойство отсутствует у конкретного правила, и общий список имён не показал
// бы, какое из правил негодно.
typedef GS::HashTable<GS::UniString, GS::Array<GS::UniString>> RenumMissingProps;
// Запускает перенумерацию выбранных элементов по правилам, заданным в свойствах.
GSErrCode ReNumSelected (SyncSettings &syncSettings);

// Окно результата запуска перенумерации.
//
// Показывается ОДИН раз, в конце работы ReNumSelected, накопителем сообщений:
// прежде каждая точка отказа открывала своё всплывающее окно, поэтому при
// нескольких проблемах пользователь получал их по одной. Окно только
// показывает: строка правила с ошибкой красная, текст ошибки идёт внизу под
// своим именем.
void ShowRenumResult (RenumRunResult *runResult);

// Формирует список правил, доступных для диалогового выбора, и проверяет наличие правила для одного элемента.
bool RenumDG (Rules &renum_rules, bool &rule_from_one);

// Собирает элементы, которые должны участвовать в перенумерации, и подготавливает параметры для записи.
// runResult — накопитель результата запуска: сообщения пишутся в него, а не
// открывают окна. При nullptr отказ остаётся невидимым, и тогда обязан
// пережить его вызывающий.
bool GetRenumElements (GS::Array<API_Guid> &guidArray,
                       ParamDictElement &paramToWriteelem,
                       GS::HashTable<API_Guid, API_PropertyDefinition> &rule_definitions,
                       bool &rule_from_one,
                       RenumRunResult *runResult);

// Проверяет, есть ли у правил переключатель флага перенумерации.
bool ReNumHasFlag (const GS::Array<API_PropertyDefinition> definitions);

// Возвращает состояние флага перенумерации по данным параметров.
RenumMode ReNumGetFlag (const ParamValue &paramflag, const ParamValue &paramposition);

// Обрабатывает один элемент и применяет к нему правила перенумерации.
// missingProps — свойства, не найденные у элемента, по правилам-владельцам.
// Привязка к правилу обязательна: без неё список имён не показал бы, какое
// правило негодно.
bool ReNum_GetElement (const API_Guid &elemGuid,
                       ParamDictElement &paramToRead,
                       Rules &rules,
                       RenumMissingProps &missingProps,
                       const GS::Array<API_PropertyDefinition> &definitions);

// Имя свойства в читаемом виде для сообщения пользователю:
// из «{@property:этаж}» делает «этаж», из «{@gdl:тип}» - «тип».
//
// Известная приставка отбрасывается целиком, вместе с двоеточием. Двоеточие
// внутри имени не трогается: у формул оно встречается в самом имени
// («{@formula:renum_criteria;%A%}»).
GS::UniString RenumReadableName (const GS::UniString &rawname);

// Регистрирует отсутствующее свойство в словаре по правилам-владельцам.
// Формульная часть (непустая formula) и уже существующее свойство молча
// пропускаются: отсутствующего свойства у них нет. Повторное добавление того
// же имени под тем же правилом игнорируется.
void AddMissingProp (RenumMissingProps &missing_props,
                     const GS::UniString &rule_name,
                     const GS::UniString &rawname,
                     const GS::UniString &formula,
                     const ParamDictValue &propertyParams);

// Имя правила для показа в окне результата: текст между фигурными скобками
// описания свойства-флага. Вычисляется ДО разбора правила на годность, потому
// что сообщение о негодном правиле привязывается к строке окна.
GS::UniString RenumRuleDisplayName (const GS::UniString &description);

// Роль части правила в имени параметра-формулы. Критерий и разбивка получают
// разные имена, иначе формулы с одинаковым шаблоном получили бы один ключ.
enum class RenumPart { Criteria, Delimetr };

// Имя (без префикса) параметра-формулы для критерия/разбивки правила.
GS::UniString RenumFormulaName (RenumPart part);

// Имя параметра-формулы целиком: префикс FORMULANAMEPREFIX + имя + ';' + шаблон
// (как в Sync.cpp). Шаблон входит в имя, поэтому две разные формулы одного
// правила не склеиваются и не совпадают с формулой такого же шаблона в другом
// правиле.
GS::UniString GetFormulaRawName (const GS::UniString &paramName, const GS::UniString &templateFormula);

// Вырезает шаблон формулы из части правила Renum{...} вида «"...%A%x<%B%>..."».
// Кавычки считаются детектором, берётся первая пара — как в ParseSyncString.
// Возвращает false, если часть не кавычена (тогда это обычное свойство) или
// кавычки непарные. templateOut получает текст между кавычками без
// приведения к нижнему регистру: литерал шаблона регистрозависим, в отличие
// от имени свойства (то, что между процентами, приводит ReplaceProcToBrace).
bool GetFormulaTemplate (const GS::UniString &rulepart, GS::UniString &templateOut);

// Выбирает наиболее частую позицию среди вариантов для одного правила.
RenumPos GetMostFrequentPos (const GS::Array<RenumPos> &eleminpos);

// Возвращает позицию для заданной группы элементов по критерию и разделителю.
RenumPos GetPos (DRenumPosDict &unicpos,
                 DStringDict &unicriteria,
                 const std::string &delimetr,
                 const std::string &criteria);

// Разделяет элементы правила по группам на основании критериев и разделителя.
bool ElementsSeparation (RenumRule &rule,
                         const ParamDictElement &paramToReadelem,
                         Delimetr &delimetrList,
                         bool &has_error);

// Применяет одно правило перенумерации к набору элементов.
void ReNumOneRule (RenumRule &rule,
                   ParamDictElement &paramToReadelem,
                   ParamDictElement &paramToWriteelem,
                   bool &has_error);

#endif
