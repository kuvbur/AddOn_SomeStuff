//------------ kuvbur 2022 ------------
#pragma once
#if !defined (ROOMBOOK_HPP)
#define	ROOMBOOK_HPP

#include    "Helpers.hpp"
namespace AutoFunc {
    #if defined(AC_22) || defined(AC_23)
    void RoomBook ();
    #else
    typedef enum {
        NoSet = 0,
        Wall_Main = 1,
        Wall_Up = 2,
        Wall_Down = 3,
        Reveal_Main = 4,
        Reveal_Up = 5,
        Reveal_Down = 6,
        Column = 7,
        Floor = 8,
        Ceil = 9,
    } TypeOtd;


    const double min_dim = 0.0001; // Минимальный размер элемента
    const double otd_thickness = 0.001;
    typedef struct {
        const GS::UniString material_main = "{@gdl:votw}";
        const GS::UniString material_up = "{@gdl:votw2}";
        const GS::UniString material_down = "{@gdl:votp}";
        const GS::UniString material_column = "{@gdl:votc}";
        const GS::UniString material_pot = "{@gdl:vots}";
        const GS::UniString height_down = "{@gdl:hpan}";
        const GS::UniString height_main = "{@gdl:hroom_pot}";
        const GS::UniString has_ceil = "{@gdl:offpot}";
        const GS::UniString tip_pot = "{@gdl:tip_pot}";
        const GS::UniString tip_pol = "{@gdl:tip_pol}";
    } GDLParamRoom;

    typedef struct {
        const GS::UniString sill = "{@gdl:gs_wido_sill}";
        const GS::UniString frame = "{@gdl:gs_frame_thk}";
        const GS::UniString useWallFinishSkin = "{@gdl:gs_usewallfinishskin}";
        const GS::UniString maxPlasterThk = "{@gdl:gs_maxplasterthk}";
        const GS::UniString AutoTurnIn = "{@gdl:gs_bautoturnin}";
        const GS::UniString bOverIn = "{@gdl:gs_boverin}";
        const GS::UniString plaster_show_3D = "{@gdl:gs_turn_plaster_show_3d}";
        const GS::UniString plaster_show_2D = "{@gdl:gs_turn_plaster_dim_2d}";
    } GDLParamWindow;

    typedef struct {
        const GS::UniString otdwall_class = "some_stuff_fin_walls";
        const GS::UniString reveal_class = "some_stuff_fin_reveals";
        const GS::UniString column_class = "some_stuff_fin_columns";
        const GS::UniString floor_class = "some_stuff_fin_floors";
        const GS::UniString ceil_class = "some_stuff_fin_ceils";
        const GS::UniString all_class = "some_stuff_fin_class";
    } ClassOtd;

    typedef struct {
        double zBottom = 0; // Аболютная координата z низа
        double height = 0; // Высота проёма
        double width = 0; // Ширина проёма
        double objLoc = 0; // Расстояние от начала стены для середины проёма
        double lower = 0; // Привязка к низу стену
        bool has_reveal = false; // Наличие откосов у родительского проёма
        bool has_side = false; // Проём лежит на верхней или нижней грани стены
        double base_reveal_width = 0; // Глубина откоса базового проёма
        API_Guid base_guid = APINULLGuid; // GUID базового проёма
        API_Guid otd_guid = APINULLGuid; // GUID созданного проёма
        GS::UniString favorite_name = ""; // Имя избранного для создания элемента
    } OtdOpening; // Проём в стене


    typedef struct {
        short material = 0; // Индекс материала
        GS::UniString smaterial = ""; // Имя материала
        GS::UniString rawname = ""; // Имя свойства для записи
        GS::UniString favorite = ""; // Имя избранного для создания элемента
    } OtdMaterial;

    typedef struct {
        Geometry::Polygon2D poly;
        double zBottom = 0;
        double height = 0;
        OtdMaterial material;
        API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента 
        API_ElemTypeID otd_type = API_SlabID; // Каким элементом строить (стеной/балкой)
        API_Guid base_guid = APINULLGuid; // GUID базового элемента
        API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
        GS::UniString favorite_name = ""; // Имя избранного для создания элемента
        TypeOtd type = NoSet; // В какую графу заносить элемент (пол, потолок, стены и т.д.)
    } OtdSlab;

    typedef struct {
        double height = 0; // Высота стены-отделки
        double zBottom = 0; // Аболютная координата z низа
        double base_th = 0; // Толщина базовой стены (для расчёта откосов)
        API_Coord begC = {0, 0}; // Координата начала
        API_Coord endC = {0, 0}; // Координата конца
        API_Guid base_guid = APINULLGuid; // GUID базового элемента
        API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
        bool base_flipped = false; // Базовая конструкция отзеркалена (для чтения состава)
        GS::Array<ParamValueComposite> base_composite; // Состав базового элемента (только отделочные слои)
        GS::Array<OtdOpening> openings; // Проёмы в стене-отделке
        OtdMaterial material;
        API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента 
        API_ElemTypeID otd_type = API_WallID; // Каким элементом строить (стеной/балкой)
        GS::UniString favorite_name = ""; // Имя избранного для создания элемента
        TypeOtd type = NoSet; // В какую графу заносить элемент (пол, потолок, стены и т.д.)
    } OtdWall; // Структура со стенами для отделки

    typedef struct {
        GS::Array<Sector> walledges; // Границы стен, не явяющихся границей зоны
        GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
        GS::Array<Sector> restedges; // Границы зоны
        GS::Array<Sector> gableedges;
    } RoomEdges;

    typedef struct {
        OtdMaterial om_up; // Отделка стен выше потолка
        OtdMaterial om_main; // Отделка стен основная
        OtdMaterial om_down; // Отделка низа стен
        OtdMaterial om_column; // Отделка колонн
        OtdMaterial om_reveals; // Отделка откосов
        OtdMaterial om_floor; // Отделка пола
        OtdMaterial om_ceil; // Отделка потолка
        OtdMaterial om_zone; // Отделка из параметров зоны
        double height = 0; // Высота зоны
        double zBottom = 0; // Аболютная координата z низа
        GS::HashTable <API_Guid, GS::Array<Sector>> walledges; // Границы стен, не явяющихся границей зоны
        GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
        GS::Array<Sector> edges; // Границы зоны
        GS::Array<API_WallPart> wallPart; // Участки стен в зоне
        GS::Array<API_BeamPart> beamPart; // Участки балок в зоне
        GS::Array<API_CWSegmentPart> cwSegmentPart; // Навесные стены в зоне
        GS::Array<API_Niche> niches; // Ниши в зоне
        double height_down = 0; // Высота панелей
        double height_main = 0; // Высота основной отделки
        double height_up = 0; // Высота верхней части отделки
        OtdSlab poly; // Полгон зоны для полов/потолков
        GS::Array<API_Guid> floorslab; // Перекрытия в уровне пола
        GS::Array<API_Guid> ceilslab; // Перекрытия в уровне потолка
        GS::Array<OtdWall> otdwall; // Стены-отделки, созданные для расчётов и отрисовки
        GS::Array<OtdSlab> otdslab; // Потолки/полы для построения
        bool has_ceil = true;
        bool has_floor = true;
        GS::UniString tip_pot = "";
        GS::UniString tip_pol = "";
        API_Guid zone_guid = APINULLGuid; // GUID базового элемента
        bool isValid = true; // Зона считана нормально
        bool create_all_elements = true; // Создавать элементы отделки
        bool create_ceil_elements = true; // Создавать элементы отделки потолка
        bool create_floor_elements = true; // Создавать элементы отделки пола
        bool create_wall_elements = true; // Создавать элементы отделки стен
        bool create_column_elements = true; // Создавать элементы отделки колонн
        bool create_reveal_elements = true; // Создавать элементы отделки откосов
    } OtdRoom; // Структура для хранения информации о зоне

    typedef GS::HashTable <API_Guid, OtdRoom> OtdRooms; // Словарь отделки всех зон
    typedef GS::HashTable<API_Guid, GS::Array<API_Guid>> UnicElement; // Словарь GUID элемента - массив GUID зон, где они встречаются
    typedef GS::HashTable<API_ElemTypeID, UnicElement> UnicElementByType;
    typedef GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> UnicGUIDByType;

    typedef GS::HashTable<GS::UniString, double> OtdMaterialAreaDict;
    typedef GS::HashTable<TypeOtd, OtdMaterialAreaDict> OtdMaterialAreaDictByType;

    // -----------------------------------------------------------------------------
    // Запись в зону информации об отделке
    // -----------------------------------------------------------------------------
    void RoomBook ();

    void WriteOtdDataToRoom (OtdRoom& otd);

    // -----------------------------------------------------------------------------
    // Убираем задвоение Guid зон у элементов
    // -----------------------------------------------------------------------------
    void ClearZoneGUID (UnicElementByType& elementToRead, GS::Array<API_ElemTypeID>& typeinzone);

    // -----------------------------------------------------------------------------
    // Получение информации из зоны о полгионах и находящейся в ней элементах
    // -----------------------------------------------------------------------------
    bool CollectRoomInfo (const Stories& storyLevels, API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead, GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone);


    // -----------------------------------------------------------------------------
    // Находит все перекрытия, котрые пересекают зону и не являются элементами отделки
    // -----------------------------------------------------------------------------
    void FloorFindInZones (GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone, const UnicGuid& finclassguids, const GS::Array<API_Guid>& zones);

    // -----------------------------------------------------------------------------
    // Чтение данных об одном проёме
    // -----------------------------------------------------------------------------
    void ReadOneWinDoor (const Stories& storyLevels, const API_Guid& elGuid, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead);

    // -----------------------------------------------------------------------------
    // Создание стен-отделок для стен 
    // -----------------------------------------------------------------------------
    void ReadOneWall (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead);

    // -----------------------------------------------------------------------------
    // Создание стен-отделок для колонны 
    // -----------------------------------------------------------------------------
    void ReadOneColumn (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

    // -----------------------------------------------------------------------------
    // Обработка полов и потолков
    // -----------------------------------------------------------------------------
    void ReadOneSlab (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

    // -----------------------------------------------------------------------------
    // Подготовка словаря с параметрами для чтения из стен/колонн/балок
    // -----------------------------------------------------------------------------
    void Param_GetForBase (const API_Guid& elGuid, ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite);

    // -----------------------------------------------------------------------------
    // Подготовка словаря с параметрами для чтения из зон
    // -----------------------------------------------------------------------------
    void Param_GetForRooms (const API_Guid& elGuid, ParamDictValue& propertyParams, ParamDictValue& paramDict);

    // -----------------------------------------------------------------------------
    // Подготовка словаря с параметрами для чтения из окон
    // -----------------------------------------------------------------------------
    void Param_GetForWindows (const API_Guid& elGuid, ParamDictValue& propertyParams, ParamDictValue& paramDict);

    // -----------------------------------------------------------------------------
    // Проверка свойств зон и замена имен
    // -----------------------------------------------------------------------------
    void Param_SetToRooms (OtdRooms& roomsinfo, ParamDictElement& paramToRead);

    bool Param_SetToRoomsFromProperty (ParamDictValue& params, OtdRoom& otd);

    bool Param_SetToRoomsMyZone (ParamDictValue& params, OtdRoom& otd);

    // -----------------------------------------------------------------------------
    // Запись прочитанных свойств в отделочные стены
    // -----------------------------------------------------------------------------
    void Param_SetToBase (OtdRooms& roomsinfo, ParamDictElement& paramToRead, UnicGUIDByType& guidselementToRead, ParamValue& param_composite);

    // -----------------------------------------------------------------------------
    // Задание прочитанных параметров для окон
    // -----------------------------------------------------------------------------
    void Param_SetToWindows (OtdRooms& roomsinfo, ParamDictElement& paramToRead);

    // -----------------------------------------------------------------------------
    // Создание стенок для откосов проёмов
    // -----------------------------------------------------------------------------
    void ReadAllOpeningReveals (OtdRooms& roomsinfo);

    // -----------------------------------------------------------------------------
    // Создание стенок для откосов одного проёма
    // -----------------------------------------------------------------------------
    void ReadOneOpeningReveals (const OtdWall& otdw, OtdOpening& op, const Geometry::Vector2<double>& walldir_perp, GS::Array<OtdWall>& opw);

    // -----------------------------------------------------------------------------
    // Разбивка созданных стен по высотам на основании информации из зоны
    // -----------------------------------------------------------------------------
    void DelimOtdWalls (OtdRooms& roomsinfo);

    // -----------------------------------------------------------------------------
    // Добавляет стену с заданной высотой
    // Удаляет отверстия, не попадающие в диапазон
    // Подгоняет размер отверсий
    // -----------------------------------------------------------------------------
    bool DelimOneWall (OtdWall otdn, GS::Array<OtdWall>& opw, double height, double zBottom, OtdMaterial& material);

    // -----------------------------------------------------------------------------
    // Получение очищенного полигона зоны, включая стены, колонны
    // -----------------------------------------------------------------------------
    void GetZoneEdges (const API_ElementMemo& zonememo, API_Element& zoneelement, GS::Array<Sector>& walledges, GS::Array<Sector>& columnedges, GS::Array<Sector>& restedges, GS::Array<Sector>& gableedges);

    void FloorRooms (const Stories& storyLevels, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead, ParamDictElement& paramToRead);

    void FloorOneRoom (const Stories& storyLevels, OtdSlab& poly, GS::Array<API_Guid>& slabGuids, GS::Array<OtdSlab>& otdslabs, GS::Array<OtdWall>& otdwall, ParamDictElement& paramToRead, bool on_top);

    bool Edge_FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge);

    // -----------------------------------------------------------------------------
    // Проверка наличия отрезка в массиве отрезков.
    // В случае нахождение проверяется направление, при необходимости разворачивается
    // -----------------------------------------------------------------------------
    bool Edge_FindEdge (Sector& edge, GS::Array<Sector>& edges);

    void Draw_Edges (const Stories& storyLevels, OtdRooms& zoneelements, UnicElementByType& subelementByparent, ClassificationFunc::ClassificationDict& finclass, GS::Array<API_Guid>& deletelist);

    void Draw_Edge (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent);

    void Draw_WallObject (const Stories& storyLevels, OtdWall& edges, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, UnicElementByType& subelementByparent);

    bool GetDefult_WallObject (const GS::UniString& favorite_name, API_Element& wallobjelement, API_ElementMemo& wallobjmemo);

    void Draw_Wall (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent);

    bool GetDefult_Wall (const GS::UniString& favorite_name, API_Element& wallelement);

    void Draw_Window (API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, OtdOpening& op, UnicElementByType& subelementByparent);

    bool GetDefult_Window (const GS::UniString& favorite_name, API_Element& windowelement, API_ElementMemo& memo);

    void Draw_CeilFloor (const Stories& storyLevels, API_Element& slabelement, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent);

    void Draw_Slab (const Stories& storyLevels, API_Element& slabelement, OtdSlab& otdslab, UnicElementByType& subelementByparent);

    void Draw_SlabObject (const Stories& storyLevels, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent);

    bool GetDefult_SlabObject (const GS::UniString& favorite_name, API_Element& slabobjelement, API_ElementMemo& slabobjmemo);

    bool GetDefult_Slab (const GS::UniString& favorite_name, API_Element& slabelement);


    // -----------------------------------------------------------------------------
    // Связывание созданных элементов отделки с базовыми элементами
    // -----------------------------------------------------------------------------
    void SetSyncOtdWall (UnicElementByType& subelementByparent, ParamDictValue& propertyParams);

    void Class_SetClass (const OtdWall& op, const ClassificationFunc::ClassificationDict& finclass);

    void Class_SetClass (const OtdOpening& op, const ClassificationFunc::ClassificationDict& finclass);

    void Class_SetClass (const OtdSlab& op, const ClassificationFunc::ClassificationDict& finclass);

    API_Guid Class_GetClassGuid (const API_ElemTypeID& base_type, const ClassificationFunc::ClassificationDict& finclass);

    // -----------------------------------------------------------------------------
    // Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
    // -----------------------------------------------------------------------------
    void Class_FindFinClass (ClassificationFunc::SystemDict& systemdict, ClassificationFunc::ClassificationDict& findict, UnicGuid& finclassguids);

    bool Class_IsElementFinClass (const API_Guid& elGuid, const UnicGuid& finclassguids);

    void Param_AddUnicElementByType (const API_Guid& parentguid, const API_Guid& guid, API_ElemTypeID elemtype, UnicElementByType& elementToRead);

    void Param_AddUnicGUIDByType (const API_Guid& elGuid, API_ElemTypeID elemtype, UnicGUIDByType& guidselementToRead);

    void Param_AddProperty (const API_Guid& elGuid, ParamDictValue& propertyParams, ParamDictValue& paramDict);
    API_ElemTypeID Favorite_GetType (const GS::UniString& favorite_name);
    bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element);
    bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element, API_ElementMemo& memo);
    #endif
}

#endif
