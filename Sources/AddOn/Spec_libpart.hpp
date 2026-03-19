//------------ kuvbur 2022 ------------
#pragma once
#if !defined (SPECLIBPART_HPP)
#define	SPECLIBPART_HPP
#include "CommonFunction.hpp"

namespace ListData
{

struct Prokat
{
    GS::UniString pos = "";  //Позиция
    GS::UniString tip_konstr = "";  //Тип конструкции
    GS::UniString obozn_mater = "";  //ГОСТ на сталь
    GS::UniString mater = "";  //Сталь
    GS::UniString obozn = "";  //ГОСТ на профиль
    GS::UniString tip_profile = "";  //Профиль
    double qty = 1; //Количество
    double dlin_prof = 0;  //Длина
    double ves_t = 0;  //Вес погонный
    double ves = 0;  //Вес
    GS::UniString units = "";  //Ед. измерения
}; // Стальной профиль

struct Mat
{
    GS::UniString pos = "";  // Позиция
    GS::UniString tip_konstr = "";  // Тип конструкции
    GS::UniString obozn = "";  // ГОСТ
    GS::UniString naen = "";  // Наименование
    double qty = 0; //Количество
    double ves = 0;  //Вес
    GS::UniString units = "";  //Ед. измерения
}; // Материал

struct Subpos
{
    GS::HashTable<GS::UniString, Prokat> prokat = {};
    GS::HashTable<GS::UniString, Mat> mat = {};
    GS::UniString pos = "";  // Позиция
    GS::UniString obozn = "";  // ГОСТ
    GS::UniString naen = "";  // Наименование
    double qty = 0; // Количество
    double ves = 0; // Масса ед.
    GS::UniString units = "";  //Ед. измерения
    bool IsEmpty ()
    {
        if (!prokat.IsEmpty ()) return false;
        if (!mat.IsEmpty ()) return false;
        if (!pos.IsEmpty ()) return false;
        if (!obozn.IsEmpty ()) return false;
        if (!naen.IsEmpty ()) return false;
        return true;
    }
    void Clear ()
    {
        prokat.Clear ();
    }
}; // Сборочная позиция

struct LibElement
{
    GS::HashTable<GS::UniString, Subpos> subpos = {};
    GS::UniString pos = "";  // Позиция
    GS::UniString obozn = "";  // ГОСТ
    GS::UniString naen = "";  // Наименование
    double qty = 0; // Количество
    double ves = 0; // Масса ед.
    GS::UniString units = "";  //Ед. измерения
    bool IsEmpty ()
    {
        if (!subpos.IsEmpty ()) return false;
        if (!pos.IsEmpty ()) return false;
        if (!obozn.IsEmpty ()) return false;
        if (!naen.IsEmpty ()) return false;
        return true;
    }
    void Clear ()
    {
        subpos.Clear ();
    }
};

typedef GS::HashTable<API_Guid, LibElement> LibElements; // Данные, считанные из элементов

void AddProkat (LibElement& paramListDataToRead, const GS::Array<GS::UniString>& partstring, GS::UniString& unitcode, double& qty, const short& version);

void Add (LibElement& paramListDataToRead, GS::UniString& name, GS::UniString& unitcode, double& qty);

}
#endif
