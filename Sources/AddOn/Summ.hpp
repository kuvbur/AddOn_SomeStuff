//------------ kuvbur 2022 ------------
#pragma once
#if !defined(SUMM_HPP)
    #define SUMM_HPP
    #include "DG.h"
    #include "Helpers.hpp"

// Правило суммирования значений элементов и записи результата в свойство или информацию проекта.
struct SumRule {
    // Свойство, куда будет записан результат суммирования.
    GS::UniString position = EMPTYSTRING;
    // Свойство с исходными значениями для суммирования.
    GS::UniString value = EMPTYSTRING;
    // Свойство-критерий для группировки элементов.
    GS::UniString criteria = EMPTYSTRING;
    // Разделитель для текстовой конкатенации.
    std::string delimetr = "; ";
    // Значения, которые следует игнорировать при суммировании.
    std::string ignore_val = "";
    // Тип суммирования: текст, число, минимум или максимум.
    SumMode sum_type = NUM_SUM;
    // Место записи результата: в свойство элемента или в информацию проекта.
    SumTarget write_to = SUM_TO_PROPERTY;
    // Список элементов, участвующих в правиле.
    GS::Array<API_Guid> elemts = {};
    // Имя свойства-правила для отображения во всплывающем окне.
    GS::UniString rule_name = EMPTYSTRING;
    // Признак активного правила.
    bool state = true;
    // Количество значений, пропущенных при обработке.
    int n_ignore = 0;
    // Количество записанных значений.
    int n_write = 0;
};

typedef GS::HashTable<API_Guid, SumRule> SumRules;

// Запускает суммирование значений свойств для выбранных элементов.
GSErrCode SumSelected (SyncSettings &syncSettings);

// Собирает значения свойств из массива элементов и готовит их к суммированию.
bool GetSumValuesOfElements (GS::Array<API_Guid> &guidArray, ParamDictElement &paramToWriteelem);

// ----------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// ----------------------------------------------------------------------------
// Разбирает свойства элементов и распределяет их по правилам суммирования.
bool Sum_GetElement (const GS::Array<API_Guid> &guidArray,
                     const GS::HashTable<API_Guid, API_PropertyDefinition> &rule_definitions,
                     ParamDictElement &paramToRead,
                     SumRules &rules);

// Разбирает описание свойства и формирует правило суммирования.
bool Sum_Rule (const API_PropertyDefinition &definition, SumRule &paramtype);

// Выполняет суммирование по одному правилу и пишет результат в целевые свойства.
void Sum_OneRule (SumRule &rule, ParamDictElement &paramToReadelem, ParamDictElement &paramToWriteelem);

#endif
