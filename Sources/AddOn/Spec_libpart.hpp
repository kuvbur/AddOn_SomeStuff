// ------------ kuvbur 2022 ------------
#pragma once
#if !defined (SPECLIBPART_HPP)
#define	SPECLIBPART_HPP
#include "CommonFunction.hpp"

namespace ListData {

    struct ArmUch {
        double l = 0;  // Длина участка
        double dop = 0;  // Диаметр поворота на конце участка
        double ang = 0;  // Угол поворота на конце участка
    }; // Координаты участка

    struct Arm {
        GS::UniString pos = "";  // Позиция
        GS::UniString klass = "";  // Класс
        double diam = 0;
        double qty = 1; // Количество
        double dlin = 0;  // Длина
        double ves_t = 0;  // Вес погонный
        double ves = 0;  // Вес
        bool isGnut = false; // Гнутый стержень
        bool isPm = false; // в п.м.
        GS::UniString naen = "";  // Наименование
        GS::UniString unit = "";  // Ед. измерения
        GS::UniString key = "";  // Уникальный код
    }; // Арматура

    struct Prokat {
        GS::UniString pos = "";  // Позиция
        GS::UniString tip_konstr = "";  // Тип конструкции
        GS::UniString obozn_mater = "";  // ГОСТ на сталь
        GS::UniString mater = "";  // Сталь
        GS::UniString obozn = "";  // ГОСТ на профиль
        GS::UniString naen = "";  // Наименование
        GS::UniString tip_profile = "";  // Профиль
        double qty = 1; // Количество
        double dlin = 0; // Длина
        double ves_t = 0; // Вес погонный
        double ves = 0; // Вес ед.
        bool isPm = false; // в п.м.
        GS::UniString key = "";  // Уникальный код
    }; // Стальной профиль

    struct Mat {
        GS::UniString pos = "";  // Позиция
        GS::UniString tip_konstr = "";  // Тип конструкции
        GS::UniString obozn = "";  // ГОСТ
        GS::UniString naen = "";  // Наименование
        double qty = 0; // Количество
        double ves = 0;  // Вес ед.
        GS::UniString unit = "";  // Ед. измерения
        GS::UniString key = "";  // Уникальный код
    }; // Материал

    struct Subpos {
        GS::HashTable<GS::UniString, Prokat> prokat = {};
        GS::HashTable<GS::UniString, Mat> mat = {};
        GS::HashTable<GS::UniString, Arm> arm = {};
        GS::UniString pos = "";  // Позиция
        GS::UniString obozn = "";  // ГОСТ
        GS::UniString naen = "";  // Наименование
        double qty = 0; // Количество
        double ves = 0; // Масса ед.
        GS::UniString unit = "";  // Ед. измерения
        GS::UniString key = "";  // Уникальный код
        bool IsEmpty () {
            if(!prokat.IsEmpty ()) return false;
            if(!mat.IsEmpty ()) return false;
            if(!arm.IsEmpty ()) return false;
            if(!pos.IsEmpty ()) return false;
            if(!obozn.IsEmpty ()) return false;
            if(!naen.IsEmpty ()) return false;
            return true;
        }
        void Clear () {
            prokat.Clear ();
            mat.Clear ();
            arm.Clear ();
        }
    }; // Сборочная позиция

    struct LibElement {
        GS::HashTable<GS::UniString, Subpos> subpos = {};
        GS::UniString pos = "";  // Позиция
        GS::UniString obozn = "";  // ГОСТ
        GS::UniString naen = "";  // Наименование
        double qty = 0; // Количество
        double ves = 0; // Масса ед.
        GS::UniString unit = "";  // Ед. измерения
        GS::UniString key = "";  // Уникальный код
        GS::Array<GS::Pair<GS::UniString, GS::UniString>> keys = {}; // Массив пар "Имя параметра - значение параметра" для поиска элемента в библиотеке по индексу
        bool IsEmpty () {
            if(!subpos.IsEmpty ()) return false;
            if(!pos.IsEmpty ()) return false;
            if(!obozn.IsEmpty ()) return false;
            if(!naen.IsEmpty ()) return false;
            return true;
        }
        void Clear () {
            subpos.Clear ();
        }
    };

    typedef GS::HashTable<API_Guid, LibElement> LibElements; // Данные, считанные из элементов

    void AddProkat (LibElement& paramListDataToRead, const GS::Array<GS::UniString>& partstring, GS::UniString& unitcode, double& qty, const short& version);

    GS::Array<GS::Pair<GS::UniString, GS::UniString>> GetAllKeys (const LibElement& el);

    void Add (LibElement& paramListDataToRead, GS::UniString& name, GS::UniString& unitcode, double& qty);

    bool AddLibdataToParamValueDict (const API_Guid& elemguid, const GS::Int32& n_layer, const LibElements& paramListDataToRead, const GS::UniString& rawname, ParamDictValue& params);

}
#endif
