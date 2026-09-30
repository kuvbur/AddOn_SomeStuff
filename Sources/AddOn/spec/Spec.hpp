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

    // Причина отказа в разборе описания правила. Задаётся только парсером
    // (GetRuleFromDescription и вызываемые им ParseOutputSchema / ParseGroups) и
    // означает ровно одно: какое условие не выполнилось. Текст не хранится —
    // он строится вызывающим по описанию, которое у него уже есть
    // (R4.4; текст на каждое правило строить нельзя, см. R7.3 плана).
    enum class ParseError {
        None,              // разбор прошёл
        NoGroupMarker,     // в описании нет ни одного маркера группы "g@@".
                           // НЕДОСТИЖИМО при текущем StringSplt: тот основан на
                           // UniString::Split и всегда возвращает хотя бы одну
                           // часть, поэтому условие не наступает. Значение
                           // оставлено как защита на случай строгой разбивки.
        GroupNotSplit,     // внутри группы не нашлось ни одной точки с запятой
        OutputPartCount,   // выходная схема s() не состоит ровно из двух частей
        NoSummary,         // нет части "s@@" либо частей меньше двух
        NoGroupsAccepted,  // ни одна разобранная группа не попала в правило
        EmptyOutputSchema, // не набрано ни одного имени для выхода
        EmptySumSchema,    // не набрано ни одного имени для сумм
    };

    // Человекочитаемый текст причины отказа. Один аргумент, чтобы вызывающий
    // подставлял описание правила сам и не зависел от формата.
    GS::UniString ParseErrorText (ParseError error);

    // Полное описание одного правила спецификации.
    // Содержит критерий, группы, параметры для чтения/записи и информацию о существующих элементах.
    //
    // Готовность правила к запуску разнесена на три независимых признака (было
    // одним флагом is_Valid с четырьмя разными смыслами — см. выписку
    // Reviews/2026-09-28_r3-state-inventory.md):
    //   parseValid        - описание правила разобрано без ошибок (GetRuleFromDescription);
    //                       неизменно после разбора, снимается только парсером;
    //   selected          - правило выбрано пользователем (SpecDG) или перечислено в ruleNames;
    //   destinationReady  - у элемента избранного есть все выходные свойства и суммы.
    // Каждый признак пишется своей стадией и не затирает остальные.
    struct SpecRule {
        GS::UniString rule_name = EMPTYSTRING;              // Имя свойства-правила для отображения во всплывающем окне
        GS::Array<GroupSpec> groups = {};                   // Массив с группами подэлементов
        GS::Array<GS::UniString> out_paramrawname = {};     // Массив имён параметров новых элементов
        GS::Array<GS::UniString> out_sum_paramrawname = {}; // Массив имён параметров сумм новых элементов
        // Маркер GUID-связи из описания правила (вида Sync_GUID+Имя правила). НЕизменяем:
        // разрешённое свойство избранного пишется в destinationParamGuidName.
        GS::UniString subguid_paramrawname = "";
        // Имя свойства избранного, найденное по маркеру выше (заполняется в SpecArray).
        GS::UniString destinationParamGuidName = EMPTYSTRING;
        GS::UniString subguid_rulename = EMPTYSTRING; // Имя свойства с правилом, на основании которого созданы элементы
        GS::UniString subguid_rulevalue = EMPTYSTRING;
        GS::Array<API_Guid> elements = {};        // Элементы, которые обрабатываются правилом
        GS::Array<API_Guid> exsist_elements = {}; // Прежде созданные элементы для этого правила
        API_PropertyDefinition rule_definitions =
            {}; // Определение свойства с правилом для поиска элементов, в которых оно доступно
        GS::UniString favorite_name = EMPTYSTRING; // Имя элемента в избранном
        // Причина отказа в разборе: None при parseValid == true. Имеет смысл
        // только вместе с parseValid == false; вызывающий печатает её вместе с
        // исходным описанием свойства (AddRule).
        ParseError parseError = ParseError::None;
        bool parseValid = true;       // Разбор описания правила удался
        bool selected = true;         // Правило выбрано для текущего запуска
        bool destinationReady = true; // Избранное содержит все выходные свойства
        bool delete_old = false;
        bool stop_on_error = true;
        bool only_visible = true;
        bool isKM = false;  // Правило для КМ (техничка)
        bool isKZH = false; // Правило для КЖ (ведомость расхода стали)

        // Совместимый адаптер: правило участвует в текущем запуске, если разобрано,
        // выбрано и его назначение готово. Заменяет прежние чтения is_Valid.
        bool IsRunnableForRun () const { return parseValid && selected && destinationReady; }
    };

    // R5.3: ОДИН набор прочитанных словарей на весь запуск.
    //
    // Три словаря, которые ParamHelpers::ElementsRead заполняет для всего набора
    // элементов, перестали быть отдельными параметрами и стали полями одной
    // структуры: раздельные словари можно было заполнить из разных вызовов
    // чтения, и рассинхронизация была невозможна только по договорённости.
    // Элемент для чтения НЕ хранится в контексте — контекст владеет данными
    // чтения, а не списком элементов.
    //
    // Доказанная граница чистоты (проверено чтением кода, НЕ измерением):
    //   - значения берутся ТОЛЬКО из этих трёх словарей, элемент не открывается;
    //   - НО GetParamValue транзитивно зовёт ParamHelpers::ReadFormula ->
    //     EvalExpression, а тот:
    //       * под TESTING пишет PROPERTYCACHE ().formulaCacheStats (calls,
    //         fullHits, fullClears) — то есть НАГРАЖДЁТ чтение значений;
    //       * держит статический кэш exprResultFullCache (сброс при >4096).
    //   Отсутствие записи в модель НЕ означает полностью чистую функцию: эти два
    //  ambient-эффекта живут в Helpers/CommonFunction и в этом шаге не тронуты,
    //   потому что Helpers.cpp под замком (#228) и правка затронула бы все
    //   26 включающих его единиц. Устранение — отдельная задача с собственным
    //   A/B по счётчикам формул.
    struct SpecReadContext {
        ParamDictElement read = {};               // Прочитанные значения свойств/GDL
        ParamDictCompositeElement composite = {}; // Прочитанные составы конструкции (материалы слоёв)
        ListData::LibElements listData = {};      // Прочитанные данные ведомостей (list-data)
    };

    // R5.4: read-only доступ к значениям для вычислителя.
    //
    // Единственное место, где решается, «как читается значение», и единственный
    // держатель контекста чтения во время расчёта правила. Все методы const и
    // не пишут ни в контекст, ни куда-либо ещё: вычислитель не может случайно
    // изменить прочитанные данные, а число чтений определяется вызывающим, а не
    // самой структурой.
    //
    // Про «без повторного ACAPI-чтения из методов строки»: повторного чтения из
    // МЕТОДОВ элемента строки в коде не было и нет — все чтения идут отсюда, по
    // уже прочитанным словарям. R5.4 делает это свойство структурным, а не
    // случайным: единственный владелец контекста один, читателей много, писателей
    // нет.
    //
    // Параметр fromMaterial у прежней GetParamValue НЕ использовался в теле
    // (проверено чтением: признак материала берётся из прочитанного pvalue, а не
    // из аргумента), поэтому в accessor не перенесён — иначе контракт новой
    // структуры сразу содержал бы параметр, который ничего не делает.
    struct SpecValueReader {
        const SpecReadContext &context;

        explicit SpecValueReader (const SpecReadContext &readContext) : context (readContext) {}

        // Читает одно значение параметра элемента. Поддерживаются обычные
        // свойства, формулы, материалы слоёв и данные из list-data.
        // n_layer < 0 означает «слой не задан»: чтение материала и list-data при
        // этом не выполняется. Возвращает false, если значение не получено или
        // помечено невалидным; pvalue.isValid выставляется только при успехе.
        bool Read (const API_Guid &elemguid, const GS::UniString &rawname, ParamValue &pvalue, GS::Int32 n_layer) const;
    };

    // R5.4: видимость источника на момент расчёта.
    // Тот же вызов с теми же флагами, что и до выноса, вызывается ОДИН раз на
    // элемент и в том же порядке (rule.elements). Доказанная эквивалентность:
    // внутри вычислительного цикла нет ни одного вызова ACAPI и ни одного
    // присваивания rule.elements — список заполняется только при разборе
    // (Spec.cpp GetRuleFromElement / AddRule), а модель в цикле не меняется,
    // поэтому видимость элемента за время прохода измениться не может.
    // onlyVisible == false — проверки не было и не появляется.
    bool IsSourceVisible (const API_Guid &elemguid, bool onlyVisible);

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

    // Дамп одного созданного/изменённого элемента спецификации.
    // Заполняется только при runResult->includeDetails (необязательный параметр
    // includeParameters в JSON-команде) — в обычных запусках остаётся пустым.
    struct SpecElementDump {
        API_Guid guid = APINULLGuid;               // GUID созданного/изменяемого элемента
        GS::UniString favorite_name = EMPTYSTRING; // Элемент из избранного, из которого сделан объект
        GS::Array<API_Guid> sourceElements = {};   // Элементы-источники строки (агрегация по правилу)
        // Значения в rawname-виде (с префиксом {@...}) -> строковое представление,
        // каким оно пишется в модель. Порядок при выдаче наружу сортируется по имени.
        GS::HashTable<GS::UniString, GS::UniString> properties = {};
        GS::HashTable<GS::UniString, GS::UniString> gdlParameters = {};
    };

    struct SpecRunResult {
        UInt32 elementsToCreate = 0;
        UInt32 elementsToModify = 0;
        UInt32 elementsToDelete = 0;
        // Собирать ли подробности элементов. Выключено по умолчанию, чтобы обычный
        // запуск Spec не платил за формирование дампа.
        bool includeDetails = false;
        GS::Array<SpecElementDump> created = {};
        GS::Array<SpecElementDump> modified = {};
        GS::Array<API_Guid> deleted = {};
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

    // Определяет политику правила (delete_old / stop_on_error / only_visible /
    // isKM / isKZH) по имени префикса описания. Разбор групп и полей не входит.
    // Имя сравнивается в нижнем регистре и по МЕСТУ "pec_rule", а не по префиксу
    // целиком, поэтому вхождение в любой части строки тоже даёт политику.
    // Ветвление: v2 раньше v3, v3 раньше KM/KZH, первый совпавший выигрывает;
    // KM/KZH затем перекрывает значения v2/v3.
    void ApplyRulePolicy (const GS::UniString &description, SpecRule &rule);

    // Приводит описание свойства-правила к виду, который понимает GetRuleFromDescription:
    // убирает переводы строк, схлопывает пробелы, подтягивает скобки/точки с запятой
    // вплотную и переписывает вызовы g()/s()/gl()/gm() во внутренние маркеры.
    // Порядок замен существенен (см. комментарий у определения). Исходную строку
    // не изменяет — возвращает нормализованную копию.
    GS::UniString NormalizeRuleDescription (const GS::UniString &source);

    // Разбирает описание свойства и добавляет правило в словарь.
    void AddRule (const API_PropertyDefinition &definition, const API_Guid &elemguid, SpecRuleDict &rules);

    // Разбирает ВЫХОДНУЮ СХЕМУ правила - часть описания после "s@@":
    // s (Pn1, Pn2, Pn3; Qn1, Qn2). Часть 0 - свойства элемента-результата,
    // часть 1 - свойства количеств. Требует РОВНО двух частей, иначе отвергает
    // всё правило. Суффикс "[N]" номера строки срезается и отбрасывается
    // (разворачивание массива делает разбор группы). Возвращает false и сбрасывает
    // rule.parseValid, только если частей не две; при успехе флаг не трогается.
    void ExpandGroup (GroupSpec &group, Int32 min_row, SpecRule &rule);
    bool ParseGroups (const GS::UniString &readPart, GS::Array<GS::UniString> &scratch, SpecRule &rule);
    bool ParseOutputSchema (const GS::UniString &writePart, GS::Array<GS::UniString> &scratch, SpecRule &rule);

    // Разбирает НОРМАЛИЗОВАННУЮ строку описания (результат NormalizeRuleDescription)
    // и превращает её в структуру SpecRule. Вход не изменяется: внутренние обрезки
    // выполняются на локальной копии, поэтому вызывающий может пользоваться
    // своей строкой после вызова (например, построить из неё ключ словаря).
    SpecRule GetRuleFromDescription (const GS::UniString &normalizedDescription);

    // Формирует набор свойств, которые нужно передать в элемент для размещения.
    GSErrCode GetElementForPlaceProperties (const GS::UniString &favorite_name,
                                            GS::HashTable<GS::UniString, GS::UniString> &paramdict);

    // Читает одно значение параметра для конкретного элемента.
    // Поддерживаются обычные свойства, формулы, материалы слоёв и данные из list-data.
    // R5.3: три словаря чтения приходят одной структурой.
    bool GetParamValue (const API_Guid &elemguid,
                        const GS::UniString &rawname,
                        const SpecReadContext &context,
                        ParamValue &pvalue,
                        bool fromMaterial,
                        const GS::Int32 &n_layer);

    // R6.1: расчётная часть правила — всё до сверки существующих строк.
    // Заполняет elements и out_param, возвращает число созданных строк
    // (0, если правило отвергнуто). Ничего не удаляет и не создаёт в модели:
    // результат можно проверить, не имея модели. Контейнеры прежние, ключи
    // прежние — это вынос, а не изменение модели (R6.2-R6.5 вводят SpecRow).
    Int32 PlanRuleRows (SpecRule &rule,
                        const SpecReadContext &context,
                        ElementDict &elements,
                        UnicGuid &error_element,
                        bool showUserInterface,
                        GS::HashTable<GS::UniString, GS::UniString> &out_param);

    // Формирует набор элементов для создания или обновления по одному правилу.
    // После расчётной части (PlanRuleRows) идёт сверка существующих строк —
    // это отдельный шаг R7, здесь она остаётся на прежнем месте.
    Int32 GetElementsForRule (SpecRule &rule,
                              const SpecReadContext &context,
                              ElementDict &elements,
                              ElementDict &elements_mod,
                              GS::Array<API_Guid> &elements_delete,
                              UnicGuid &error_element,
                              bool showUserInterface);

    // Выбирает из параметров групп имена свойств, которые нужно прочитать в начале обработки.
    bool OutSlotsMatchSchema (const Element &element, UInt32 outSlots, UInt32 sumSlots);

    // Привязка одного выходного слота к полю группы, из которого он наполняется.
    // Имя НЕ копируется: rule.groups на время исполнения не меняется, поэтому
    // хранится указатель на элемент массива группы (R4.3 — привязка готовится
    // один раз до цикла по элементам, а не на каждый источник).
    struct SlotBinding {
        const GS::UniString *rawname = nullptr; // Поле группы, наполняющее слот
        bool isSumLiteral = false;              // Слот суммы начисляется константой "1"
    };

    // Привязка выходных слотов ОДНОЙ группы к её полям и к схеме правила.
    // sizesMatchSchema — предупреждение «группа не может наполнить схему», а НЕ
    // замена проверки внутри цикла: Push () выполняется только при успешном
    // чтении, поэтому фактическое число слотов элемента известно лишь в цикле.
    struct GroupSlotBinding {
        GS::Array<SlotBinding> outSlots = {}; // Поля, наполняющие element.out_param
        GS::Array<SlotBinding> sumSlots = {}; // Поля, наполняющие element.out_sum_param
        UInt32 schemaOutSlots = 0;            // Сколько слотов выхода объявлено схемой правила
        UInt32 schemaSumSlots = 0;            // Сколько слотов сумм объявлено схемой правила
        bool sizesMatchSchema = false;        // Совпадает ли число полей группы со схемой
    };

    // Связывает поля всех групп правила с выходными слотами ОДИН раз до цикла по
    // элементам. Возвращает ровно одну привязку на каждую группу, в том же
    // порядке, поэтому размер равен rule.groups.GetSize ().
    GS::Array<GroupSlotBinding> PrepareSlotBindings (const SpecRule &rule);

    // R5.1: что одному правилу нужно прочитать у источников и что потом
    // записать. Собирается из уже разобранного правила и не обращается к
    // модели: это объявление данных, а не их чтение. Объединение запросов
    // между правилами делает вызывающий — словари paramToRead/paramToWrite
    // у всего запуска общие, и кратность их наполнения задаётся именно там.
    struct RuleDependencies {
        ParamDict read = {};  // Имена, которые нужно прочитать у источников
        ParamDict write = {}; // Имена, которые потом записываются в результат
    };

    // Перечисляет зависимости правила: флаги, суммы, уникальные и выходные поля
    // всех групп (с пропуском слоёв материалов/ведомостей, кроме нулевого) плюс
    // выходные имена схемы на запись. Литерал "1" в суммах не читается; флаг
    // невалидной группы читается, потому что по нему элемент отбрасывается.
    RuleDependencies CollectRuleDependencies (const SpecRule &rule);

    // Разворачивает собранные имена в словарь параметров ОДНОГО элемента:
    // обычные имена добавляются как есть, а имена материалов и формул
    // разбираются на служебные свойтия слоя и само выражение.
    void BuildReadParamDict (const ParamDict &readNames, ParamDictValue &paramDict);

    // Сверяет выходную схему правила с набором свойств избранного: все имена
    // должны найтись. Отсутствующие имена копятся в error_name, признак
    // готовности правила снимается. Возвращает признак готовности.
    bool MatchDestinationProperties (SpecRule &rule,
                                     const GS::HashTable<GS::UniString, GS::UniString> &favorite,
                                     ParamDict &error_name);

    // Ищет у избранного два служебных свойства: носитель имени правила
    // (описание со словом "spec_rule_name") и носитель GUID (описание с
    // маркером subguid_paramrawname и словом "sync_guid"). Найденное имя
    // GUID-свойства пишется в destinationParamGuidName — маркер из описания
    // не подменяется. Значения берутся из кэша свойств; неудачное чтение НЕ
    // кэшируется (R5.2 запрещает новую политику кэша без F), переход к
    // следующему свойству сохраняется. Возвращает признак, что носитель GUID
    // найден (прежде локальный flag_find).
    bool ResolveFavoriteLinks (SpecRule &rule,
                               const GS::HashTable<GS::UniString, GS::UniString> &favorite,
                               ParamDictValue &paramToWrite);

    // Отбирает ранее созданные элементы правила в поле rule.exsist_elements.
    // ПУСТОЙ selected_elements означает «взять все найденные»; непустой —
    // фильтр по выделению, причём элементы ДОПИСЫВАЮТСЯ, а не заменяют
    // прежнее содержимое поля (прежнее поведение, менять нельзя: это изменило
    // бы объём удаляемых строк).
    void SelectExistingElements (SpecRule &rule, const GS::Array<API_Guid> &found, const UnicGuid &selected_elements);

    // Запрашивает чтение выходных имён, сумм и носителя GUID у ранее созданных
    // элементов, чтобы их можно было сравнить с правилом.
    void AddExistingReadRequests (const SpecRule &rule,
                                  const GS::Array<API_Guid> &elements,
                                  ParamDictElement &paramToRead);

    void GetParamToReadFromRule (SpecRule &rules, ParamDictElement &paramToRead, ParamDictValue &paramToWrite);

    // Создаёт или настраивает элемент, который будет размещён согласно правилу.
    GSErrCode GetElementForPlace (const GS::UniString &favorite_name, API_Element &element, API_ElementMemo &memo);

    // Получает размеры элемента для размещения по сетке.
    bool GetSizePlaceElement (const API_Element &elementt, const API_ElementMemo &memot, double &dx, double &dy);

    // Размещает сформированные элементы в модели и заполняет их параметры.
    // При runResult != nullptr и runResult->includeDetails заполняет runResult->created
    // (в т.ч. GDL-параметры, удалённые из param при записи в memo на :PlaceElements).
    GSErrCode PlaceElements (GS::Array<ElementDict> &elementstocreate,
                             ParamDictValue &paramToWrite,
                             ParamDictElement &paramOut,
                             Point2D &startpos,
                             SpecRunResult *runResult);
} // namespace Spec

#endif
