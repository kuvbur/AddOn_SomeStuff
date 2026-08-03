//------------ kuvbur 2022 ------------
#pragma once
#if !defined(CLASSIF_HPP)
    #define CLASSIF_HPP
    #include "Constants.hpp"

namespace ClassificationFunc {

    // Вспомогательная структура, описывающая один найденный класс классификации.
    // Хранит как сам элемент классификации, так и его родителя и имя для быстрого поиска.
    struct ClassificationValues {
        API_ClassificationSystem system;        // Система классификации, из которой взят класс
        API_ClassificationItem item;            // Сам класс
        GS::UniString parentname = EMPTYSTRING; // Имя родительского класса в текущей иерархии
        GS::UniString itemname = EMPTYSTRING;   // Уникальное имя класса для ключа словаря
    }; // Структура для хранения класса

    typedef GS::HashTable<GS::UniString, ClassificationValues> ClassificationDict; // Словарь классов в системе

    typedef GS::HashTable<GS::UniString, ClassificationDict> SystemDict; // Словарь систем с вложенными классами

    // -----------------------------------------------------------------------------
    // Загружает все доступные классы из систем классификации и сохраняет их в словарь.
    // Используется как подготовительный шаг для последующего поиска и назначения классов.
    // -----------------------------------------------------------------------------
    GSErrCode GetAllClassification (SystemDict &systemdict);

    // Перебирает всех потомков заданного класса и добавляет их в словарь классификаций.
    void GatherAllDescendantOfClassification (const API_ClassificationItem &item,
                                              ClassificationDict &classifications,
                                              const API_ClassificationSystem &system);

    // Добавляет один элемент классификации в словарь с привязкой к родителю и системе.
    void AddClassificationItem (const API_ClassificationItem &item,
                                const API_ClassificationItem &parent,
                                ClassificationDict &classifications,
                                const API_ClassificationSystem &system);

    // -----------------------------------------------------------------------------
    // Составляет полное имя класса с учётом его родительской иерархии.
    // -----------------------------------------------------------------------------
    void GetFullName (const API_ClassificationItem &item,
                      const ClassificationDict &classifications,
                      GS::UniString &fullname);

    // -----------------------------------------------------------------------------
    // Ищет класс по имени в конкретной системе классификации и возвращает его GUID.
    // -----------------------------------------------------------------------------
    API_Guid FindClass (const GS::UniString &systemname, const GS::UniString &classname);

    // Возвращает имя системы классификации по её GUID.
    GS::UniString GetSystemName (const API_Guid &systemguid);

    // Ищет класс по паре GUID системы и GUID элемента классификации.
    API_ClassificationItem FindClass (const GS::Pair<API_Guid, API_Guid> &classitem);

    // -----------------------------------------------------------------------------
    // Назначает элементу специальный автокласс, если у него ещё нет классификации.
    // Автокласс определяется по описанию, содержащему some_stuff_class.
    // -----------------------------------------------------------------------------
    void SetAutoclass (const API_Guid elemGuid);

    // Считывает словарь систем и классов из кэша или проекта.
    bool ReadSystemDict ();
} // namespace ClassificationFunc

#endif
