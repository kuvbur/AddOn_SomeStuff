//------------ kuvbur 2022 ------------
#if !defined(RENUM_HPP)
#pragma once
#define RENUM_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include "APICommon28.h"
#endif // AC_28
#ifdef AC_29
#include	"APICommon29.h"
#endif // AC_29
#include "DG.h"
#include "Helpers.hpp"
#include "alphanum.h"
// Типы нумерации (см. RenumElement.state)
#define RENUM_SKIP -1  // Исключить из обработки
#define RENUM_IGNORE 0 // Не менять позмцию, но добавлять похожие элементы
#define RENUM_ADD 1	   // Не менять позицию, если нет пропусков
#define RENUM_NORMAL 2 // Обычная нумерация/перенумерация

// Типы простановки нулей для СТРОКОВОГО (API_PropertyStringValueType) свойства (см. RenumRule.nulltype)
#define NOZEROS 0	  // Не добавлять нули в текстовое свойство
#define ADDZEROS 1	  // Добавлять нули с учётом разбивки
#define ADDMAXZEROS 2 // Добавлять нули по максимальному количеству без учёта разбивки
#define ADDSPACE 3	  // Добавлять пробелы с учётом разбивки
#define ADDMAXSPACE 4 // Добавлять пробелы по максимальному количеству без учёта разбивки

class RenumPos
{
public:
    API_Guid guid = APINULLGuid;
    std::string strpos = ""; // Полный текст позиции
    int numpos = 0;			 // Численная часть позиции
    bool isNum = false;		 // Есть численное значение
    std::string prefix = ""; // Префикс
    std::string suffix = ""; // Суффикс
    GSCharCode chcode = GChCode;
    RenumPos ()
    {
    }

    // TODO Добавить парсинг префикса и суффикса позиции
    RenumPos (const ParamValue& param)
    {
        guid = param.fromGuid;
        if (param.val.canCalculate) {
            isNum = true;
            numpos = param.val.intValue;
        }
        chcode = GetCharCode (param.val.uniStringValue);
        strpos = param.val.uniStringValue.ToCStr (0, MaxUSize, this->chcode).Get ();
    }
    RenumPos (int pos)
    {
        isNum = true;
        numpos = pos;
        this->setStr ();
    }

    GS::UniString ToUniString ()
    {
        this->setStr ();
        GS::UniString unipos = GS::UniString (this->strpos.c_str (), this->chcode);
        return unipos;
    }

    void Add (const int& i)
    {
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

    void FormatToMax (RenumPos& pos, short nulltype, int nullcount)
    {
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

    void SetToMax (RenumPos& pos)
    {
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

    void SetPrefix (GS::UniString& prefix)
    {
        GSCharCode chcode = GetCharCode (prefix);
        this->prefix = prefix.ToCStr (0, MaxUSize, chcode).Get ();
    }

    ParamValue ToParamValue (GS::UniString& rawname)
    {
        ParamValue posvalue;
        GS::UniString unipos = this->ToUniString ();
        ParamHelpers::ConvertStringToParamValue (posvalue, rawname, unipos);
        return posvalue;
    }

    bool operator==(const RenumPos& b)
    {
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
    void setStr ()
    {
        if (this->isNum) {
            strpos = this->prefix + std::to_string (this->numpos) + this->suffix;
        }
    }
};

struct RenumElem
{
    GS::Array<RenumPos> elements;
    RenumPos mostFrequentPos;
};

struct RenumRule
{
    bool state = false;
    bool oldalgoritm = true;
    GS::UniString flag = "";	   // Описание свойства, в которое ставим позицию
    GS::UniString position = "";   // Описание свойства, в которое ставим позицию
    GS::UniString criteria = "";   // Описание свойства-критерия
    GS::UniString delimetr = "";   // Описание свойства-разбивки
    short nulltype = NOZEROS;	   // Тип постановки нулей в позиции
    int nullcount = 0;             // Количество нулей, если задано жёское количество
    GS::Array<API_Guid> elemts;	   // Массив элементов
    API_Guid guid = APINULLGuid;   // GUID свойства с правилом
};

typedef std::map<std::string, RenumElem, doj::alphanum_less<std::string>> Values; // Словарь элементов по критериям

typedef std::unordered_map<short, Values> TypeValues; // Словарь по типам нумерации

typedef std::unordered_map<std::string, TypeValues> Delimetr; // Словарь по разделителю

typedef std::map<std::string, RenumPos, doj::alphanum_less<std::string>> StringDict;
typedef std::map<std::string, StringDict, doj::alphanum_less<std::string>> DStringDict;

typedef std::map<std::string, std::string, doj::alphanum_less<std::string>> RenumPosDict;
typedef std::map<std::string, RenumPosDict, doj::alphanum_less<std::string>> DRenumPosDict;

typedef GS::HashTable<API_Guid, RenumRule> Rules; // Таблица правил
GSErrCode ReNumSelected (SyncSettings& syncSettings);

bool GetRenumElements (GS::Array<API_Guid>& guidArray, ParamDictElement& paramToWriteelem, GS::HashTable<API_Guid, API_PropertyDefinition>& rule_definitions);

bool ReNumHasFlag (const GS::Array<API_PropertyDefinition> definitions);
short ReNumGetFlag (const ParamValue& paramflag, const ParamValue& paramposition);
bool ReNum_GetElement (const API_Guid& elemGuid, ParamDictElement& paramToRead, Rules& rules, GS::HashTable<GS::UniString, bool>& error_propertyname);
RenumPos GetMostFrequentPos (const GS::Array<RenumPos>& eleminpos);
RenumPos GetPos (DRenumPosDict& unicpos, DStringDict& unicriteria, const std::string& delimetr, const std::string& criteria);
bool ElementsSeparation (const RenumRule& rule, const ParamDictElement& paramToReadelem, Delimetr& delimetrList, bool& has_error);
void ReNumOneRule (const RenumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem, bool& has_error);

#endif
