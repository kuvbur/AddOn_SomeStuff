// ------------ kuvbur 2022 ------------
#pragma once
#if !defined(SPECLIBPART_HPP)
    #define SPECLIBPART_HPP
    #include "CommonFunction.hpp"

namespace ListData {

    // -----------------------------------------------------------------------------
    // Координаты и геометрия участка (сегмента) гнутого арматурного стержня
    // -----------------------------------------------------------------------------
    struct ArmUch {
        double l = 0;   // Длина участка
        double dop = 0; // Диаметр поворота на конце участка
        double ang = 0; // Угол поворота на конце участка
    };

    // -----------------------------------------------------------------------------
    // Данные об арматурном изделии или отдельном стержне
    // -----------------------------------------------------------------------------
    struct Arm {
        GS::UniString pos = EMPTYSTRING;   // Позиция
        GS::UniString klass = EMPTYSTRING; // Класс
        double diam = 0;                   // Диаметр
        double qty = 1;                    // Количество
        double dlin = 0;                   // Длина
        double ves_t = 0;                  // Вес погонный
        double ves = 0;                    // Вес
        bool isGnut = false;               // Гнутый стержень
        bool isPm = false;                 // В п.м.
        GS::UniString naen = EMPTYSTRING;  // Наименование
        GS::UniString unit = EMPTYSTRING;  // Ед. измерения
        GS::UniString key = EMPTYSTRING;   // Уникальный код

        // -----------------------------------------------------------------------------
        // Очистка данных арматуры
        // -----------------------------------------------------------------------------
        void Clear () {}
    };

    // -----------------------------------------------------------------------------
    // Данные о металлопрокате / стальном профиле
    // -----------------------------------------------------------------------------
    struct Prokat {
        GS::UniString pos = EMPTYSTRING;         // Позиция
        GS::UniString tip_konstr = EMPTYSTRING;  // Тип конструкции
        GS::UniString obozn_mater = EMPTYSTRING; // ГОСТ на сталь
        GS::UniString mater = EMPTYSTRING;       // Сталь
        GS::UniString obozn = EMPTYSTRING;       // ГОСТ на профиль
        GS::UniString naen = EMPTYSTRING;        // Наименование
        GS::UniString tip_profile = EMPTYSTRING; // Профиль
        double qty = 1;                          // Количество
        double dlin = 0;                         // Длина
        double ves_t = 0;                        // Вес погонный
        double ves = 0;                          // Вес ед.
        bool isPm = false;                       // В п.м.
        GS::UniString key = EMPTYSTRING;         // Уникальный код
    };

    // -----------------------------------------------------------------------------
    // Данные о строительном материале
    // -----------------------------------------------------------------------------
    struct Mat {
        GS::UniString pos = EMPTYSTRING;        // Позиция
        GS::UniString tip_konstr = EMPTYSTRING; // Тип конструкции
        GS::UniString obozn = EMPTYSTRING;      // ГОСТ
        GS::UniString naen = EMPTYSTRING;       // Наименование
        double qty = 0;                         // Количество
        double ves = 0;                         // Вес ед.
        GS::UniString unit = EMPTYSTRING;       // Ед. измерения
        GS::UniString key = EMPTYSTRING;        // Уникальный код
    };

    // -----------------------------------------------------------------------------
    // Сборочная позиция (узел, сборка или марка, содержащая прокат, материалы и арматуру)
    // -----------------------------------------------------------------------------
    struct Subpos {
        GS::HashTable<GS::UniString, Prokat> prokat = {}; // Таблица проката
        GS::HashTable<GS::UniString, Mat> mat = {};       // Таблица материалов
        GS::HashTable<GS::UniString, Arm> arm = {};       // Таблица арматуры
        GS::UniString pos = EMPTYSTRING;                  // Позиция
        GS::UniString obozn = EMPTYSTRING;                // ГОСТ
        GS::UniString naen = EMPTYSTRING;                 // Наименование
        double qty = 0;                                   // Количество
        double ves = 0;                                   // Масса ед.
        GS::UniString unit = EMPTYSTRING;                 // Ед. измерения
        GS::UniString key = EMPTYSTRING;                  // Уникальный код

        // -----------------------------------------------------------------------------
        // Проверка, пуста ли сборочная позиция
        // -----------------------------------------------------------------------------
        bool IsEmpty () {
            if (!prokat.IsEmpty ())
                return false;
            if (!mat.IsEmpty ())
                return false;
            if (!arm.IsEmpty ())
                return false;
            if (!pos.IsEmpty ())
                return false;
            if (!obozn.IsEmpty ())
                return false;
            if (!naen.IsEmpty ())
                return false;
            return true;
        }

        // -----------------------------------------------------------------------------
        // Очистка сборочной позиции
        // -----------------------------------------------------------------------------
        void Clear () {
            prokat.Clear ();
            mat.Clear ();
            arm.Clear ();
        }
    };

    // -----------------------------------------------------------------------------
    // Данные библиотечного элемента сметы
    // -----------------------------------------------------------------------------
    struct LibElement {
        GS::HashTable<GS::UniString, Subpos> subpos = {}; // Таблица сборочных позиций
        GS::UniString pos = EMPTYSTRING;                  // Позиция
        GS::UniString obozn = EMPTYSTRING;                // ГОСТ
        GS::UniString naen = EMPTYSTRING;                 // Наименование
        double qty = 0;                                   // Количество
        double ves = 0;                                   // Масса ед.
        GS::UniString unit = EMPTYSTRING;                 // Ед. измерения
        GS::UniString key = EMPTYSTRING;                  // Уникальный код
        GS::Array<GS::Pair<GS::UniString, GS::UniString>> keys =
            {}; // Массив пар "Имя параметра - значение параметра" для поиска элемента в библиотеке по индексу

        // -----------------------------------------------------------------------------
        // Проверка, пуст ли библиотечный элемент
        // -----------------------------------------------------------------------------
        bool IsEmpty () {
            if (!subpos.IsEmpty ())
                return false;
            if (!pos.IsEmpty ())
                return false;
            if (!obozn.IsEmpty ())
                return false;
            if (!naen.IsEmpty ())
                return false;
            return true;
        }

        // -----------------------------------------------------------------------------
        // Очистка данных библиотечного элемента
        // -----------------------------------------------------------------------------
        void Clear () { subpos.Clear (); }
    };

    // -----------------------------------------------------------------------------
    // Данные, считанные из элементов (ассоциативный массив по GUID элемента)
    // -----------------------------------------------------------------------------
    typedef GS::HashTable<API_Guid, LibElement> LibElements;

    // -----------------------------------------------------------------------------
    // Парсинг и добавление проката в сметную структуру библиотечного элемента
    // -----------------------------------------------------------------------------
    void AddProkat (LibElement &paramListDataToRead,
                    const GS::Array<GS::UniString> &partstring,
                    GS::UniString &unitcode,
                    double &qty,
                    const short &version);

    // -----------------------------------------------------------------------------
    // Получение массива всех пар "ключ-значение" параметров библиотечного элемента
    // -----------------------------------------------------------------------------
    GS::Array<GS::Pair<GS::UniString, GS::UniString>> GetAllKeys (const LibElement &el);

    // -----------------------------------------------------------------------------
    // Добавление общего материала/элемента в сметную структуру
    // -----------------------------------------------------------------------------
    void Add (LibElement &paramListDataToRead, GS::UniString &name, GS::UniString &unitcode, double &qty);

    // -----------------------------------------------------------------------------
    // Запись распарсенных сметных данных в словарь параметров элемента
    // -----------------------------------------------------------------------------
    bool AddLibdataToParamValueDict (const API_Guid &elemguid,
                                     const GS::Int32 &n_layer,
                                     const LibElements &paramListDataToRead,
                                     const GS::UniString &rawname,
                                     ParamDictValue &params);

} // namespace ListData
#endif
