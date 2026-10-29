//------------ kuvbur 2022 ------------
#pragma once
#if !defined(ROOMBOOKCONST_HPP)
    #define ROOMBOOKCONST_HPP

constexpr double DistanceTolerance = 1.0e-6;
// Допуск плоскости 0,1 мм равен минимальному размеру отделочного элемента.
constexpr double PlaneTolerance = 1.0e-4;
constexpr double AreaTolerance = 1.0e-6;

namespace Roombook {

    const double min_dim = 0.0001; // Минимальный размер элемента
    const double otd_thickness = 0.001;

    static const GS::UniString IGNORENAME = "----";

    static const API_ElemTypeID typeinzone[] = {
        API_WindowID, API_DoorID, API_WallID, API_ColumnID, API_SlabID, API_ZoneID, API_BeamID};

    // Типы отделочных элементов
    enum TypeOtd {
        NoSet = 0,           // Не определено
        Wall_Main = 1,       // Основная часть отделки зоны
        Wall_Up = 2,         // Верхняя часть отделки зоны, выше потолка
        Wall_Down = 3,       // Нижняя часть отделки зоны
        Reveal_Main = 4,     // Отделка откосов, боковая часть
        Reveal_Up = 5,       // Отделка откосов, верхняя часть
        Reveal_Down = 6,     // Отделка откосов, нижняя часть
        Column = 7,          // Отделка колонн
        Floor = 8,           // Отделка пола
        Ceil = 9,            // Отделка потолка
        Floor_Skirting = 10, // Плинтус пола
        Ceil_Cornice = 11,   // Плинтус потолка
        Beam_Ceil = 12,      // Отделка балок в потолке
        Beam_Wall = 13,      // Отделка балок в стенах
    };

    // Ключевые имена классов, которыми помечаются элементы отделки.
    // Они используются для поиска и последующей привязки созданных элементов к нужной группе.
    struct ClassOtd {
        const GS::UniString otdwall_class = "some_stuff_fin_walls";           // Стены отделки
        const GS::UniString otdwall_down_class = "some_stuff_fin_down_walls"; // Нижняя чать отделки зоны (Если есть)
        const GS::UniString reveal_class = "some_stuff_fin_reveals";          // Отделка откосов, общий класс
        const GS::UniString reveal_windows_class = "some_stuff_fin_windows_reveals"; // Отделка откосов, проёмы окон
        const GS::UniString reveal_doors_class = "some_stuff_fin_doors_reveals";     // Отделка откосов, проёмы дверей
        const GS::UniString column_class = "some_stuff_fin_columns";                 // Отделка колонн
        const GS::UniString beam_ceil_class = "some_stuff_fin_ceil_beams";           // Отделка балок в потолке
        const GS::UniString beam_wall_class = "some_stuff_fin_wall_beams";           // Отделка балок в стенах
        const GS::UniString floor_class = "some_stuff_fin_floors";                   // Отделка пола
        const GS::UniString floor_skirting_class = "some_stuff_fin_skirting";        // Плинтус пола
        const GS::UniString ceil_cornice_class = "some_stuff_fin_cornice";           // Карниз потолка
        const GS::UniString ceil_class = "some_stuff_fin_ceils";                     // Отделка потолка
        const GS::UniString all_class = "some_stuff_fin_class";                      // Общий класс отделки
    };

    // Ключевые имена избранного по умолчанию
    struct FavoriteDefaultOtd {
        const GS::UniString favorite_wall = "smstf wall";               // Стена отделки
        const GS::UniString favorite_reveal_up = "smstf reveal up";     // Отделка откоса сверху
        const GS::UniString favorite_reveal_side = "smstf reveal side"; // Отделка откоса сбоку
        const GS::UniString favorite_floor_side = "smstf floor";        // Отделка пола
        const GS::UniString favorite_ceil = "smstf ceil";               // Отделка потолка
        const GS::UniString favorite_ceil_wall = "smstf ceil side";     // Стены на перепадах потолка
        const GS::UniString favorite_floor = "smstf floor";             // Отделка пола
        const GS::UniString favorite_floor_wall = "smstf floor side";   // Стены на перепадах пола
        const GS::UniString favorite_window = "smstf window";           // Проём в отделочных стенах
        const GS::UniString favorite_floor_skirting = "smstf skirting"; // Плинтус пола
        const GS::UniString favorite_ceil_cornice = "smstf cornice";    // Плинтус потолка
        const GS::UniString favorite_suffix = "_smstf"; // Суффикс для избранного, чтобы отличать от обычного
    };

    // Поиск свойств со включением слоя отделки и имени
    static const GS::UniString propdesc_onoff = "some_stuff_layer_onoff";
    static const GS::UniString propdesc_desc = "some_stuff_layer_description";
    static const GS::UniString propdesc_fav = "some_stuff_layer_favorite_name";
    static const GS::UniString propdesc_hasfin = "some_stuff_layer_has_finish";
    static const GS::UniString propdesc_hasfin_elem = "some_stuff_element_has_finish";
    static const GS::UniString propdesc_onoff_elem = "some_stuff_element_onoff";

} // namespace Roombook

#endif
