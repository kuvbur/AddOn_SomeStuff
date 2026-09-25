//------------ kuvbur 2022 ------------
#pragma once
#if !defined(SPEC_HPP)
    #define SPEC_HPP

    #include "Helpers.hpp"

// Модуль генерации спецификаций по правилам: разбор описаний, выбор
// подходящих элементов, их группировка и последующее создание/обновление элементов.
namespace Spec {
    // Описание одной группы внутри правила спецификации.
    // Группа определяет, как из исходных элементов формируется один итоговый подэлемент.
    struct GroupSpec {
        GS::Array<GS::UniString> unic_paramrawname = {}; // Массив имён уникальных параметров
        GS::Array<GS::UniString> out_paramrawname = {};  // Массив имён параметров для передачи новым элементам
        GS::Array<GS::UniString> sum_paramrawname = {};  // Массив имён параметров, которые требуется просуммировать
        GS::UniString flag_paramrawname =
            ""; // Имя параметра-флага, определяющего - будет ли элемент учтён в спецификации
        bool is_Valid = true;
        bool fromMaterial = false; // Чтение из компонент
        bool fromLibData = false;  // Чтение ведомостей
        GS::Int32 n_layer = 0;
    };

    // Полное описание одного правила спецификации.
    // Содержит критерий, группы, параметры для чтения/записи и информацию о существующих элементах.
    struct SpecRule {
        GS::UniString rule_name = EMPTYSTRING;              // Имя свойства-правила для отображения во всплывающем окне
        GS::Array<GroupSpec> groups = {};                   // Массив с группами подэлементов
        GS::Array<GS::UniString> out_paramrawname = {};     // Массив имён параметров новых элементов
        GS::Array<GS::UniString> out_sum_paramrawname = {}; // Массив имён параметров сумм новых элементов
        GS::UniString subguid_paramrawname =
            ""; // Имя свойства для записи GUID созданных элементов (в описании должно содержатся Sync_GUID+Имя правила)
        GS::UniString subguid_rulename = EMPTYSTRING; // Имя свойства с правилом, на основании которого созданы элементы
        GS::UniString subguid_rulevalue = EMPTYSTRING;
        GS::Array<API_Guid> elements = {};        // Элементы, которые обрабатываются правилом
        GS::Array<API_Guid> exsist_elements = {}; // Прежде созданные элементы для этого правила
        API_PropertyDefinition rule_definitions =
            {}; // Определение свойства с правилом для поиска элементов, в которых оно доступно
        GS::UniString favorite_name = EMPTYSTRING; // Имя элемента в избранном
        bool is_Valid = true;
        bool delete_old = false;
        bool stop_on_error = true;
        bool only_visible = true;
        bool isKM = false;  // Правило для КМ (техничка)
        bool isKZH = false; // Правило для КЖ (ведомость расхода стали)
    };

    // Временный контейнер для одного элемента, который будет создан или обновлён по правилу.
    struct Element {
        GS::Array<ParamValue> out_param = {};
        GS::Array<ParamValue> out_sum_param = {};
        GS::Array<GS::UniString> out_paramrawname;
        GS::Array<GS::UniString> out_sum_paramrawname;
        GS::UniString subguid_paramrawname = EMPTYSTRING;
        GS::UniString subguid_rulename = EMPTYSTRING;
        GS::UniString subguid_rulevalue = EMPTYSTRING;
        GS::UniString favorite_name = EMPTYSTRING; // Имя элемента в избранном
        GS::Array<API_Guid> elements = {};         // Элементы, которые обрабатываются правилом
        API_Guid exs_guid = APINULLGuid;           // GUID существующего элемента для перезаписи
    };

    typedef GS::HashTable<GS::UniString, Element>
        ElementDict; // Словарь элементов для создания, ключ - сцепка значений уникальных параметров

    typedef GS::HashTable<GS::UniString, SpecRule> SpecRuleDict; // Словарь правил, ключ - имя правила

    struct SpecRunResult {
        UInt32 elementsToCreate = 0;
        UInt32 elementsToModify = 0;
        UInt32 elementsToDelete = 0;
    };

    // Ищет правила спецификации в свойствах элемента по умолчанию и собирает связанные с ними элементы.
    bool GetRuleFromDefaultElem (SpecRuleDict &rules,
                                 API_DatabaseInfo &homedatabaseInfo,
                                 bool &has_elementspec,
                                 bool showUserInterface = true);

    // Создаёт спецификацию из текущего выбора, всех видимых элементов или правил по умолчанию.
    // При переданной точке выполняет non-interactive запуск: `ruleNames == nullptr` означает все валидные правила.
    GSErrCode SpecAll (const SyncSettings &syncSettings,
                       const GS::Array<GS::UniString> *ruleNames = nullptr,
                       const Point2D *placementPoint = nullptr,
                       SpecRunResult *runResult = nullptr);

    // Исключает из обработки элементы неподходящих типов и элементы из других баз данных.
    void SpecFilter (API_Guid &elemguid, API_DatabaseInfo &homedatabaseInfo);

    // Применяет ту же фильтрацию к целому массиву GUID элементов.
    void SpecFilter (GS::Array<API_Guid> &guidArray, API_DatabaseInfo &homedatabaseInfo);

    // Обрабатывает массив элементов по набору правил спецификации и формирует итоговые элементы.
    GSErrCode SpecArray (const SyncSettings &syncSettings,
                         GS::Array<API_Guid> &guidArray,
                         SpecRuleDict &rules,
                         const UnicGuid &selected_elements,
                         const GS::Array<GS::UniString> *ruleNames,
                         const Point2D *placementPoint,
                         SpecRunResult *runResult);

    // Получает правила из свойства выбранного элемента и добавляет их в словарь.
    GSErrCode GetRuleFromElement (const API_Guid &elemguid, SpecRuleDict &rules);

    // Разбирает описание свойства и добавляет правило в словарь.
    void AddRule (const API_PropertyDefinition &definition, const API_Guid &elemguid, SpecRuleDict &rules);

    // Разбирает строку описания правила и превращает её в структуру SpecRule.
    SpecRule GetRuleFromDescription (GS::UniString &description);

    // Формирует набор свойств, которые нужно передать в элемент для размещения.
    GSErrCode GetElementForPlaceProperties (const GS::UniString &favorite_name,
                                            GS::HashTable<GS::UniString, GS::UniString> &paramdict);

    // Читает одно значение параметра для конкретного элемента.
    bool GetParamValue (const API_Guid &elemguid,
                        const GS::UniString &rawname,
                        const ParamDictElement &paramToRead,
                        ParamValue &pvalue,
                        bool fromMaterial,
                        const GS::Int32 &n_layer,
                        const ParamDictCompositeElement &paramCompositeToRead,
                        const ListData::LibElements &paramListDataToRead);

    // Формирует набор элементов для создания или обновления по одному правилу.
    Int32 GetElementsForRule (SpecRule &rule,
                              const ParamDictElement &paramToRead,
                              const ParamDictCompositeElement &paramCompositeToRead,
                              const ListData::LibElements &paramListDataToRead,
                              ElementDict &elements,
                              ElementDict &elements_mod,
                              GS::Array<API_Guid> &elements_delete,
                              UnicGuid &error_element,
                              bool showUserInterface);

    // Выбирает из параметров групп имена свойств, которые нужно прочитать в начале обработки.
    void GetParamToReadFromRule (SpecRule &rules, ParamDictElement &paramToRead, ParamDictValue &paramToWrite);

    // Создаёт или настраивает элемент, который будет размещён согласно правилу.
    GSErrCode GetElementForPlace (const GS::UniString &favorite_name, API_Element &element, API_ElementMemo &memo);

    // Получает размеры элемента для размещения по сетке.
    bool GetSizePlaceElement (const API_Element &elementt, const API_ElementMemo &memot, double &dx, double &dy);

    // Размещает сформированные элементы в модели и заполняет их параметры.
    GSErrCode PlaceElements (GS::Array<ElementDict> &elementstocreate,
                             ParamDictValue &paramToWrite,
                             ParamDictElement &paramOut,
                             Point2D &startpos);
} // namespace Spec

#endif
