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
    // (текст на каждое правило не строится).
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
    // Готовность правила к запуску задают три независимых признака:
    //   parseValid        - описание правила разобрано без ошибок (GetRuleFromDescription);
    //                       неизменно после разбора, снимается только парсером;
    //   selected          - правило выбрано пользователем (SpecDG) или перечислено в ruleNames;
    //   destinationReady  - у элемента избранного есть все выходные свойства и суммы.
    // Каждый признак пишется своей стадией и не затирает остальные.
    // Состояние запуска правила отделено от его определения.
    //
    // exsist_elements заполняется SelectExistingElements; остальные поля
    // состояния (elements, selected, destinationReady,
    // destinationParamGuidName) хранятся в SpecRule.
    //
    // Состояние найденных существующих элементов хранится отдельно от определения правила.
    struct SpecRuleRunState {
        // Существующие элементы этого правила, в порядке обхода.
        //
        // ПРАВИЛО ЗАПОЛНЕНИЯ (не менять): при пустом отборе — ПРИСВАИВАНИЕ
        // найденного списка (SelectExistingElements, Spec.cpp:1482); при
        // непустом — ДОПИСЫВАНИЕ без очистки (:1492). Присваивание во втором
        // случае изменило бы поведение на непустом входном поле, и сегодня это
        // безопасно только тем, что словарь правил создаётся заново на каждый
        // запуск. Порядок и накопление найденных элементов значимы.
        GS::Array<API_Guid> exsist_elements = {};
    };

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
        GS::Array<API_Guid> elements = {}; // Элементы правила; заполняются до его выбора.
        SpecRuleRunState runState;         // Найденные существующие элементы.
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
        // выбрано и его назначение готово.
        bool IsRunnableForRun () const { return parseValid && selected && destinationReady; }
    };

    // Один набор прочитанных словарей на весь запуск.
    //
    // Три словаря, которые ParamHelpers::ElementsRead заполняет для всего набора
    // хранятся в одной структуре, чтобы чтение использовало согласованный контекст.
    // Элемент для чтения НЕ хранится в контексте — контекст владеет данными
    // чтения, а не списком элементов.
    //
    // Ограничение на побочные эффекты:
    //   - значения берутся ТОЛЬКО из этих трёх словарей, элемент не открывается;
    //   - НО GetParamValue транзитивно зовёт ParamHelpers::ReadFormula ->
    //     EvalExpression, а тот:
    // под TESTING обновляет PROPERTYCACHE ().formulaCacheStats (calls,
    //         fullHits, fullClears);
    //       * держит статический кэш exprResultFullCache (сброс при >4096).
    //   Отсутствие записи в модель НЕ означает полностью чистую функцию: эти два
    // побочные эффекты живут в Helpers/CommonFunction.
    struct SpecReadContext {
        ParamDictElement read = {};               // Прочитанные значения свойств/GDL
        ParamDictCompositeElement composite = {}; // Прочитанные составы конструкции (материалы слоёв)
        ListData::LibElements listData = {};      // Прочитанные данные ведомостей (list-data)
    };

    // Read-only доступ к значениям для вычислителя.
    //
    // Единственное место, где решается, «как читается значение», и единственный
    // держатель контекста чтения во время расчёта правила. Все методы const и
    // не пишут ни в контекст, ни куда-либо ещё: вычислитель не может случайно
    // изменить прочитанные данные, а число чтений определяется вызывающим, а не
    // самой структурой.
    //
    // Методы строки не повторяют ACAPI-чтение: значения берутся из
    // уже прочитанных словарей. Контекст один, читателей много, писателей нет.
    //
    // Признак материала берётся из прочитанного pvalue.
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

    // Видимость источника на момент расчёта.
    // Проверка вызывается один раз на элемент в порядке rule.elements:
    // внутри вычислительного цикла нет ни одного вызова ACAPI и ни одного
    // присваивания rule.elements — список заполняется только при разборе
    // (Spec.cpp GetRuleFromElement / AddRule), а модель в цикле не меняется,
    // поэтому видимость элемента за время прохода измениться не может.
    // При onlyVisible == false проверка не выполняется.
    bool IsSourceVisible (const API_Guid &elemguid, bool onlyVisible);

    // Один выходной слот: имя поля плюс набранное значение.
    //
    // Имя и значение хранятся вместе, поэтому не расходятся по длине.
    //
    // Слот — это копия, а не ссылка: строки наполняются в цикле, и ссылка на
    // элемент массива инвалидировалась бы при Push. Размер слота — это один
    // ParamValue (32+ байта) плюс UniString, поэтому хранение имён отдельно от
    // значений экономии не даёт; зато схема становится единственным источником
    // правды о том, что выходит из правила.
    struct OutputSlot {
        GS::UniString rawname = EMPTYSTRING; // Поле, в которое пишется значение
        ParamValue value = {};               // Значение слота (исходник для paramTo.val)
        bool isSum = false;                  // true — слот суммы, false — выходной
    };

    // Общая выходная схема правила: слоты по порядку схемы. Имена и значения
    // хранятся вместе, порядок слотов совпадает с порядком, который задаёт
    // rule.out_paramrawname, затем rule.out_sum_paramrawname.
    //
    // Схема копируется один раз при создании строки, а обращение к значению и
    // имени идёт по индексу слота.
    //
    // OutParam и OutSlotName возвращают значение и имя по индексу.
    typedef GS::Array<OutputSlot> OutputSlots;

    // Временный контейнер для одного элемента, который будет создан или обновлён по правилу.
    struct Element {
        // Единственный владелец значений и имён строки; аксессоры ниже читают слоты.
        //
        // ПОРЯДОК слотов значим: сначала выходные, затем суммы.
        // Место источника у слота — флаг isSum, а не позиция, поэтому порядок
        // сохраняется нарочно: по нему считаются индексы и в сверке, и при
        // размещении.
        OutputSlots out_slots = {};
        GS::UniString subguid_paramrawname = EMPTYSTRING;
        GS::UniString subguid_rulename = EMPTYSTRING;
        GS::UniString subguid_rulevalue = EMPTYSTRING;
        GS::UniString favorite_name = EMPTYSTRING; // Имя элемента в избранном
        GS::Array<API_Guid> elements = {};         // Элементы, которые обрабатываются правилом
        API_Guid exs_guid = APINULLGuid;           // GUID существующего элемента для перезаписи

        // Значение слота по индексу в порядке схемы: сначала выходные, затем
        // суммы. Индекс вне схемы даёт ЗАВЕДОМО БЕЗОПАСНЫЙ ответ, а не выход за
        // границу: потребитель сам решает, что делать с отсутствующим значением.
        const ParamValue &OutParam (UInt32 index) const {
            static const ParamValue emptyValue = {};
            return index < out_slots.GetSize () ? out_slots[index].value : emptyValue;
        }

        // Имя поля слота в порядке схемы.
        const GS::UniString &OutSlotName (UInt32 index) const {
            return index < out_slots.GetSize () ? out_slots[index].rawname : EMPTYSTRING;
        }

        // Число слотов схемы: выходные плюс суммы.
        UInt32 OutSlotCount () const { return out_slots.GetSize (); }

        // Число выходных (не суммарных) слотов — нужно там, где текущий код
        // спрашивал размер out_param, а по схеме это позиция первого слота суммы.
        UInt32 OutParamSlotCount () const {
            UInt32 count = 0;
            for (const OutputSlot &slot : out_slots)
                if (!slot.isSum)
                    ++count;
            return count;
        }

        // Аксессоры индексируют выходные слоты и суммы отдельно.
        // OutParam (index) индексирует объединённую схему.
        //
        // Выходные слоты занимают первые OutParamSlotCount () позиций схемы
        // (сначала выходные, затем суммы).
        UInt32 OutSumSlotCount () const { return out_slots.GetSize () - OutParamSlotCount (); }

        // Число слотов каждого вида.
        UInt32 OutParamCount () const { return OutParamSlotCount (); }

        UInt32 OutSumCount () const { return OutSumSlotCount (); }

        const ParamValue &OutParamValue (UInt32 index) const {
            return index < OutParamSlotCount () ? out_slots[index].value : OutParam (out_slots.GetSize ());
        }

        const ParamValue &OutSumValue (UInt32 index) const {
            const UInt32 offset = OutParamSlotCount ();
            return index < OutSumSlotCount () ? out_slots[offset + index].value : OutParam (out_slots.GetSize ());
        }

        const GS::UniString &OutParamName (UInt32 index) const {
            return index < OutParamSlotCount () ? out_slots[index].rawname : EMPTYSTRING;
        }

        const GS::UniString &OutSumName (UInt32 index) const {
            const UInt32 offset = OutParamSlotCount ();
            return index < OutSumSlotCount () ? out_slots[offset + index].rawname : EMPTYSTRING;
        }

        // Имя и значение одного слота — то, ради чего слот и введён. Возвращает
        // false, если индекс вне схемы: потребитель решает сам, а получает
        // заведомо безопасный ответ.
        bool TryGetOutSlot (UInt32 index, GS::UniString &rawname, ParamValue &value, bool &isSum) const {
            if (index >= out_slots.GetSize ())
                return false;
            const OutputSlot &slot = out_slots[index];
            rawname = slot.rawname;
            value = slot.value;
            isSum = slot.isSum;
            return true;
        }
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
    // Ветвление: v2 перед v3, v3 перед KM/KZH, первый совпавший выигрывает;
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

    // Источник, из которого прочитаны свойства и GDL-параметры при поиске
    // элемента для размещения.
    //
    // favoriteFound и fromDefaultElem — РАЗНЫЕ признаки, и это разделение
    // обязательно: при ненайденном избранном функция читает настройки объекта
    // по умолчанию и возвращает NoError, поэтому успешная сверка по умолчанию
    // НЕ доказывает, что указанное избранное существует.
    struct PlaceSourceInfo {
        GS::Array<API_Property> properties = {}; // Свойства источника (избранного или объекта по умолчанию)
        GS::UniString name = EMPTYSTRING;        // Имя избранного из правила (пусто — объект по умолчанию)
        bool favoriteFound = false;              // Избранное найдено по имени
        bool fromDefaultElem = false;            // Читались настройки объекта по умолчанию
    };

    // Состояние значения свойства-флага правила.
    //
    // Отдельный перечислитель, а не bool: «недоступно», «не вычислено»,
    // «взято значение по умолчанию из определения» и фактическое значение флага
    // — разные ответы, и сведение их к одному признаку теряет причину отказа.
    // В AC22-23 вычисленность свойства выражает isEvaluated, с AC24 — status,
    // поэтому приведение к одному полю выполняется только внутри EvaluateRuleFlag.
    enum class RuleFlagStatus : unsigned char {
        Unknown,      // определение свойства не найдено либо источник не задан
        NotPresent,   // в источнике нет этого свойства
        NotAvailable, // свойство недоступно на источнике
        NotEvaluated, // не вычислено
        HasValue      // значение получено
    };

    // Откуда взято значение флага: различать нужно, потому что подстановка
    // настроек объекта по умолчанию вместо ненайденного избранного меняет смысл
    // проверки (см. PlaceSourceInfo).
    enum class RuleFlagOrigin : unsigned char {
        NotChecked,       // не проверялось
        ElementValue,     // значение на переданном элементе
        FavoriteValue,    // значение у избранного
        DefaultElemValue, // значение у объекта по умолчанию
        DefaultDefinition // значение по умолчанию из определения (свойство не вычислено)
    };

    // Результат чтения флага правила. checked означает «значение можно читать»,
    // value осмысленно только при checked == true.
    struct RuleFlagCheck {
        bool checked = false;
        bool value = false;
        RuleFlagStatus status = RuleFlagStatus::Unknown;
        RuleFlagOrigin origin = RuleFlagOrigin::NotChecked;
        GS::UniString sourceName = EMPTYSTRING; // имя избранного или GUID источника
        bool isSingleValue = false;             // definition.collectionType == API_PropertySingleCollectionType
        bool isDefault = true;
        bool evaluated = false;
    };

    // Разбирает API_Property как флаг правила, НЕ схлопывая состояния: с AC24
    // status == API_Property_NotAvailable НЕ означает «флаг включён», хотя прод
    // GetRuleFromElement трактует его именно так. Здесь этот случай остаётся
    // отдельным ответом (NotAvailable, checked == false), а origin задаёт
    // вызывающий, потому что только он знает источник.
    void EvaluateRuleFlag (const API_Property &property, RuleFlagCheck &flag);

    // Имена из dependencies.read, которых НЕТ в прочитанном словаре элемента
    // (ключ отсутствует) либо значение которых помечено невалидным. Возвращает
    // число расхождений. Литерал "1" и пустые имена пропускаются — читать нечего.
    UInt32 CollectUnreadRuleNames (const SpecRule &rule,
                                   const API_Guid &elemguid,
                                   const ParamDictElement &read,
                                   GS::Array<GS::UniString> &missing);

    // Проверка правила спецификации по GUID свойства-правила (read-only).
    // Ничего не пишет в модель: правило разбирается теми же функциями, что и
    // запуск, наличие свойств проверяется существующими чтениями.
    struct RuleCheckResult {
        SpecRule rule = {};                       // Разобранное правило (заполняется всегда, даже при отказе разбора)
        GS::UniString propertyName = EMPTYSTRING; // Полное имя свойства-правила
        bool definitionFound = false;             // Определение свойства найдено в проекте
        bool ruleParsed = false;                  // Описание разобрано без ошибок
        ParseError parseError = ParseError::None;
        bool checkedElement = false;                       // Проверялось ли наличие у элемента
        RuleFlagCheck elementFlag = {};                    // Флаг на переданном элементе
        RuleFlagCheck destinationFlag = {};                // Флаг у избранного или объекта по умолчанию
        bool favoriteFound = false;                        // Избранное найдено по имени
        bool fromDefaultElem = false;                      // Назначение прочитано по умолчанию (fallback)
        GS::Array<GS::UniString> missingRead = {};         // Не прочитано у элемента
        GS::Array<GS::UniString> missingWrite = {};        // Нет выходных свойств у избранного/объекта по умолчанию
        GS::Array<GS::UniString> unresolvedInProject = {}; // Не подтверждено наличием определения в проекте
    };

    // Проверяет правило по GUID его свойства. elemguid == APINULLGuid означает
    // «элемент не задан»: тогда читается только проект (наличие определений
    // свойств) и назначение правила, а чтение с элемента не выполняется.
    // Правило всегда возвращается в result.rule (в том числе неразобранным), а
    // признак удачи — result.definitionFound.
    bool CheckRuleByPropertyGuid (const API_Guid &propertyGuid, const API_Guid &elemguid, RuleCheckResult &result);

    // Формирует набор свойств, которые нужно передать в элемент для размещения.
    // readInfo — необязательный наблюдатель источника чтения (nullptr — прежнее
    // поведение вызова: словарь наполняется, источник не отслеживается).
    GSErrCode GetElementForPlaceProperties (const GS::UniString &favorite_name,
                                            GS::HashTable<GS::UniString, GS::UniString> &paramdict,
                                            PlaceSourceInfo *readInfo = nullptr);

    // Читает одно значение параметра для конкретного элемента.
    // Поддерживаются обычные свойства, формулы, материалы слоёв и данные из list-data.
    // Три словаря чтения передаются одной структурой.
    bool GetParamValue (const API_Guid &elemguid,
                        const GS::UniString &rawname,
                        const SpecReadContext &context,
                        ParamValue &pvalue,
                        bool fromMaterial,
                        const GS::Int32 &n_layer);

    // План изменений по существующим строкам. Объявлен здесь, потому что
    // PlanRuleRows принимает его наблюдателем; полное определение — ниже, после
    // описания строки Element.
    struct SpecChangePlan;

    // Расчётная часть правила — всё до сверки существующих строк.
    // Заполняет elements и out_param, возвращает число созданных строк
    // (0, если правило отвергнуто). Ничего не удаляет и не создаёт в модели:
    // результат можно проверить без изменения модели.
    // plan — необязательный наблюдатель полноты расчёта. Не влияет на
    // решение: ни одно условие не читает и не пишет его. Default nullptr —
    // вызывающие, которым полнота не нужна, не меняются.
    Int32 PlanRuleRows (SpecRule &rule,
                        const SpecReadContext &context,
                        ElementDict &elements,
                        UnicGuid &error_element,
                        bool showUserInterface,
                        GS::HashTable<GS::UniString, GS::UniString> &out_param,
                        SpecChangePlan *plan = nullptr);

    // Формирует набор элементов для создания или обновления по одному правилу.
    // После расчётной части (PlanRuleRows) идёт сверка существующих строк —
    // сверка выполняется после расчёта.
    // --------------------------------------------------------------------
    // План изменений по существующим строкам.
    //
    // Сверка определяет обновления, удаления и оставшиеся в elements строки.
    // План собирает ответы в одну структуру без создания
    // текста объяснения на каждую корректную строку.
    //
    // В ReconcileExistingRows текст отчёта строится только в ветвях отклонения.
    // Происхождение решения хранится Дёшево: перечислимыми значениями и
    // счётчиками, а не склеенными строками.
    //
    // Поля:
    //   create  — строки, оставшиеся создать (те, что не сопоставлены ни одному
    //             существующему объекту); это остатки в elements после сверки;
    //   update  — сопоставленные строки, для которых поднят flag_change;
    //             они же лежат в elementsMod под ключом key;
    //   delete  — GUID существующих объектов, которые нужно удалить;
    //   unchanged — сопоставленные строки БЕЗ изменений. Их НЕ храним копией:
    //             копия всех свойств ради строки, которую не надо трогать,
    //             не нужна. Считаем их количество.
    //
    // deleteOld == 0 означает: сверка не выполнялась вовсе, план пустой, и это
    // НЕ то же самое, что «всё unchanged».
    // --------------------------------------------------------------------
    struct SpecChangePlan {
        // Причина, по которой существующий объект удаляется. Порядок ветвей
        // сверки задаёт и порядок причин — он значим.
        enum class DeleteReason : unsigned char {
            NoUniqueFields = 0, // не собрался key_out: не прочитано уникальное поле
            NoNewRow,           // key_out не найден в out_param: нет такой строки
            RowAlreadyClaimed,  // строка по key_out уже израсходована другим объектом
            Obsolete            // объект не попал в guids: нет сопоставления
        };

        // Что удалить и ПОЧЕМУ — одной структурой, а не двумя параллельными
        // массивами: параллельный массив причин рассинхронизируется с delete
        // при изменении одной из ветвей.
        struct Deletion {
            API_Guid guid;
            DeleteReason reason;
        };

        GS::Array<Deletion> removals = {}; // что удалить, в порядке обхода
        ElementDict update = {};           // что обновить: ключ строки -> строка
        GS::Array<API_Guid> create = {};   // создаваемые GUID (порядок строк)
        Int32 unchanged = 0;               // сопоставлено без изменений
        // Сверка выполнялась? По умолчанию НЕТ: план, который никто не
        // заполнял, не должен утверждать, что отражает решение. Ставится 1
        // только в точке, где сверка действительно прошла.
        Int32 deleteOld = 0;
        // Полнота чтения и расчёта. Считаются по ВКЛАДАМ, а не по
        // словарям not_found_*: те словари хранят только ЗАСООБЩЁННЫЕ поля, то
        // есть зависимы от stop_on_error, и при stop_on_error = false остаются
        // пустыми при реально неполном чтении. Полнота — свойство расчёта, а не
        // политики отчётов, поэтому и источник другой.
        Int32 notFoundUnicCount = 0;    // не прочитано уникальных полей (все вклады)
        Int32 notFoundParamCount = 0;   // не прочитано выходных и суммарных полей
        Int32 contributionsTotal = 0;   // вкладов, дошедших до раскладки в строку
        Int32 contributionsPartial = 0; // из них с неполным чтением
        Int32 schemaMismatchCount = 0;  // вкладов, отброшенных сверкой схемы

        // Чтение и расчёт прошли полностью? Раздельные признаки, потому что
        // неполное чтение и неполная схема — разные причины, их нужно
        // различать их при решении, разрешать ли разрушительные действия.
        bool ReadComplete () const { return notFoundUnicCount == 0 && notFoundParamCount == 0; }

        bool CalcComplete () const { return contributionsPartial == 0 && schemaMismatchCount == 0; }

        // Объединённый признак полноты чтения и расчёта.
        bool IsComplete () const { return ReadComplete () && CalcComplete (); }

        // Согласованность плана с реально выполненным. Сверка пишет план и
        // фактические списки из одних и тех же точек, поэтому проверка должна
        // всегда проходить; она существует как страховка от будущей правки.
        bool Matches (const ElementDict &elementsMod, const GS::Array<API_Guid> &elementsDelete) const {
            if (deleteOld == 0)
                return removals.IsEmpty () && update.IsEmpty () && create.IsEmpty () && unchanged == 0;
            if (removals.GetSize () != elementsDelete.GetSize ())
                return false;
            for (UInt32 i = 0; i < removals.GetSize (); ++i)
                if (removals[i].guid != elementsDelete[i])
                    return false;
            if (update.GetSize () != elementsMod.GetSize ())
                return false;
            for (auto &cIt : update) {
    #ifdef ServerMainVers_2800
                const GS::UniString &rowKey = cIt.key;
    #else
                const GS::UniString &rowKey = *cIt.key;
    #endif
                if (!elementsMod.ContainsKey (rowKey))
                    return false;
            }
            return true;
        }
    };

    Int32 GetElementsForRule (SpecRule &rule,
                              const SpecReadContext &context,
                              ElementDict &elements,
                              ElementDict &elements_mod,
                              GS::Array<API_Guid> &elements_delete,
                              UnicGuid &error_element,
                              bool showUserInterface,
                              // План изменений. nullptr означает «план не нужен» —
                              // обычный рабочий путь его не строит вовсе.
                              SpecChangePlan *plan = nullptr);

    // Выбирает из параметров групп имена свойств, которые нужно прочитать в начале обработки.
    bool OutSlotsMatchSchema (const Element &element, UInt32 outSlots, UInt32 sumSlots);

    // Привязка одного выходного слота к полю группы, из которого он наполняется.
    // Имя НЕ копируется: rule.groups на время исполнения не меняется, поэтому
    // хранится указатель на элемент массива группы (привязка готовится
    // один раз до цикла по элементам, а не на каждый источник).
    struct SlotBinding {
        const GS::UniString *rawname = nullptr; // Поле группы, наполняющее слот
        bool isSumLiteral = false;              // Слот суммы начисляется константой "1"
    };

    // Привязка выходных слотов ОДНОЙ группы к её полям и к схеме правила.
    // sizesMatchSchema заранее проверяет число полей группы относительно схемы;
    // это не заменяет проверку внутри цикла: Push () выполняется только при успешном
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

    // Зависимости одного правила: что нужно прочитать у источников и что потом
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
    // кэшируется; переход к
    // следующему свойству сохраняется. Возвращает признак, что носитель GUID
    // найден.
    bool ResolveFavoriteLinks (SpecRule &rule,
                               const GS::HashTable<GS::UniString, GS::UniString> &favorite,
                               ParamDictValue &paramToWrite);

    // Отбирает ранее созданные элементы правила в поле rule.runState.exsist_elements.
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

    // Размеры элемента для сетки размещения и дамп значений элемента
    // объявлены в spec/SpecHelpers.hpp — отдельном внутреннем модуле.

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
