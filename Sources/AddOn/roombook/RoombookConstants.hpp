//------------ kuvbur 2022 ------------
#pragma once
#if !defined(ROOMBOOKCONST_HPP)
    #define ROOMBOOKCONST_HPP

constexpr double DistanceTolerance = 1.0e-6;
constexpr double PlaneTolerance = 1.0e-4;
constexpr double AreaTolerance = 1.0e-6;

namespace Roombook {

    const double min_dim = 0.0001; // Минимальный размер элемента
    const double min_area = 0.01;  // Минимальная площадь элемента
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

    const GS::Array<TypeOtd> TypeOtdAll{
        Wall_Main,
        Wall_Up,
        Wall_Down,
        Reveal_Main,
        Reveal_Up,
        Reveal_Down,
        Column,
        Floor,
        Ceil,
        Floor_Skirting,
        Ceil_Cornice,
        Beam_Ceil,
        Beam_Wall,
    };

    // Ключевые имена классов, которыми помечаются элементы отделки.
    // Они используются для поиска и последующей привязки созданных элементов к нужной группе.
    const GS::UniString otd_class_prefix = "some_stuff_fin_";

    const GS::UniString wall_class = otd_class_prefix + "walls";           // Стены отделки
    const GS::UniString wall_down_class = otd_class_prefix + "down_walls"; // Нижняя чать отделки зоны (Если есть)
    const GS::UniString reveal_class = otd_class_prefix + "reveals";       // Отделка откосов, общий класс
    const GS::UniString reveal_windows_class = otd_class_prefix + "windows_reveals"; // Отделка откосов, проёмы окон
    const GS::UniString reveal_doors_class = otd_class_prefix + "doors_reveals";     // Отделка откосов, проёмы дверей
    const GS::UniString column_class = otd_class_prefix + "columns";                 // Отделка колонн
    const GS::UniString beam_ceil_class = otd_class_prefix + "ceil_beams";           // Отделка балок в потолке
    const GS::UniString beam_wall_class = otd_class_prefix + "wall_beams";           // Отделка балок в стенах
    const GS::UniString floor_class = otd_class_prefix + "floors";                   // Отделка пола
    const GS::UniString floor_skirting_class = otd_class_prefix + "skirting";        // Плинтус пола
    const GS::UniString ceil_cornice_class = otd_class_prefix + "cornice";           // Карниз потолка
    const GS::UniString ceil_class = otd_class_prefix + "ceils";                     // Отделка потолка
    const GS::UniString all_class = otd_class_prefix + "class";                      // Общий класс отделки
    const GS::Array<GS::UniString> class_all{all_class,
                                             wall_class,
                                             wall_down_class,
                                             reveal_class,
                                             reveal_windows_class,
                                             reveal_doors_class,
                                             column_class,
                                             beam_ceil_class,
                                             beam_wall_class,
                                             floor_class,
                                             floor_skirting_class,
                                             ceil_cornice_class,
                                             ceil_class};

    // Ключевые имена избранного по умолчанию
    static const GS::UniString favorite_prefix = "smstf"; // Суффикс для избранного, чтобы отличать от обычного
    static const GS::UniString favorite_suffix =
        "_" + favorite_prefix;                                     // Суффикс для избранного, чтобы отличать от обычного
    const GS::UniString favorite_wall = favorite_prefix + " wall"; // Стена отделки

    const GS::UniString favorite_reveal_up = favorite_prefix + " reveal up";     // Отделка откоса сверху
    const GS::UniString favorite_reveal_side = favorite_prefix + " reveal side"; // Отделка откоса сбоку

    const GS::UniString favorite_ceil = favorite_prefix + " ceil";           // Отделка потолка
    const GS::UniString favorite_ceil_wall = favorite_prefix + " ceil side"; // Стены на перепадах потолка

    const GS::UniString favorite_floor = favorite_prefix + " floor";           // Отделка пола
    const GS::UniString favorite_floor_wall = favorite_prefix + " floor side"; // Стены на перепадах пола

    const GS::UniString favorite_window = favorite_prefix + " window";           // Проём в отделочных стенах
    const GS::UniString favorite_floor_skirting = favorite_prefix + " skirting"; // Плинтус пола
    const GS::UniString favorite_ceil_cornice = favorite_prefix + " cornice";    // Плинтус потолка

    const GS::Array<GS::UniString> favorite_all{favorite_wall,
                                                favorite_reveal_up,
                                                favorite_reveal_side,
                                                favorite_ceil,
                                                favorite_ceil_wall,
                                                favorite_floor,
                                                favorite_floor_wall,
                                                favorite_window,
                                                favorite_floor_skirting,
                                                favorite_ceil_cornice};

    // Поиск свойств со включением слоя отделки и имени
    static const GS::UniString propdesc_prefix_layer = "some_stuff_layer_";
    static const GS::UniString propdesc_prefix_element = "some_stuff_element_";
    static const GS::UniString propdesc_onoff = propdesc_prefix_layer + "onoff";
    static const GS::UniString propdesc_desc = propdesc_prefix_layer + "description";
    static const GS::UniString propdesc_fav = propdesc_prefix_layer + "favorite_name";
    static const GS::UniString propdesc_hasfin = propdesc_prefix_layer + "has_finish";
    static const GS::UniString propdesc_hasfin_elem = propdesc_prefix_element + "has_finish";
    static const GS::UniString propdesc_onoff_elem = propdesc_prefix_element + "onoff";

} // namespace Roombook

#endif
