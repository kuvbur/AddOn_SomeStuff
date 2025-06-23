//------------ kuvbur 2022 ------------
#pragma once
#if !defined (ROOMBOOK_HPP)
#define	ROOMBOOK_HPP

#include    "Helpers.hpp"
namespace AutoFunc
{
#if defined(AC_22) || defined(AC_23)
void RoomBook ();
#else
typedef enum
{
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

typedef struct
{
    const GS::UniString otdwall_class = "some_stuff_fin_walls";
    const GS::UniString otdwall_down_class = "some_stuff_fin_down_walls";
    const GS::UniString reveal_class = "some_stuff_fin_reveals";
    const GS::UniString column_class = "some_stuff_fin_columns";
    const GS::UniString floor_class = "some_stuff_fin_floors";
    const GS::UniString ceil_class = "some_stuff_fin_ceils";
    const GS::UniString all_class = "some_stuff_fin_class";
} ClassOtd;

typedef struct
{
    double zBottom = 0; // Аболютная координата z низа
    double height = 0; // Высота проёма
    double width = 0; // Ширина проёма
    double objLoc = 0; // Расстояние от начала стены для середины проёма
    double lower = 0; // Привязка к низу стены
    bool reflected = false;
    bool has_reveal = false; // Наличие откосов у родительского проёма
    double base_reveal_width = 0; // Глубина откоса базового проёма
    API_Guid base_guid = APINULLGuid; // GUID базового проёма
    API_Guid otd_guid = APINULLGuid; // GUID созданного проёма
} OtdOpening; // Проём в стене


typedef struct
{
    short material = 0; // Индекс материала
    GS::UniString smaterial = ""; // Имя материала
    GS::UniString rawname = ""; // Имя свойства для записи
} OtdMaterial;


typedef struct
{
    GS::UniString name = ""; // Имя избранного 
    API_ElemTypeID type = API_ZombieElemID; // Тип элемента в избранном
    GS::Array<ParamValueComposite> composite; // Состав элемента (только отделочные слои)
    bool is_composite_read = false;
} MatarialToFavorite;
typedef GS::HashTable<GS::UniString, MatarialToFavorite> MatarialToFavoriteDict;

typedef struct
{
    Geometry::Polygon2D poly;
    double zBottom = 0; // Аболютная координата z низа
    short floorInd = 0; // Этаж
    double height = 0;
    OtdMaterial material;
    GS::UniString tip = ""; // Тип потолка/пола из зоны
    API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента
    API_ElemTypeID draw_type = API_SlabID; // Тип отрисовываемого элемента
    API_Guid base_guid = APINULLGuid; // GUID базового элемента
    GS::Array<ParamValueComposite> base_composite; // Состав базового элемента (только отделочные слои)
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
    TypeOtd type = NoSet; // В какую графу заносить элемент (пол, потолок, стены и т.д.)
    MatarialToFavorite favorite = {}; // Избранное
} OtdSlab;

typedef struct
{
    double ang_begC = 90 * DEGRAD; // Угол подрезки начала
    double ang_endC = 90 * DEGRAD; // Угол подрезки конца
    double height = 0; // Высота стены-отделки
    double width = 0; // Ширина (для откоса)
    double length = 0; // Длина (для откоса)
    double zBottom = 0; // Аболютная координата z низа
    short floorInd = 0; // Этаж
    double base_th = 0; // Толщина базовой стены (для расчёта откосов)
    API_Coord begC = { 0, 0 }; // Координата начала
    API_Coord endC = { 0, 0 }; // Координата конца
    API_Guid base_guid = APINULLGuid; // GUID базового элемента
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
    bool base_flipped = false; // Базовая конструкция отзеркалена (для чтения состава)
    GS::Array<ParamValueComposite> base_composite; // Состав базового элемента (только отделочные слои)
    GS::Array<OtdOpening> openings; // Проёмы в стене-отделке
    OtdMaterial material;
    API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента
    API_ElemTypeID draw_type = API_WallID; // Тип отрисовываемого элемента
    TypeOtd type = NoSet; // В какую графу заносить элемент (пол, потолок, стены и т.д.)
    MatarialToFavorite favorite = {}; // Избранное
} OtdWall; // Структура со стенами для отделки

typedef struct
{
    GS::Array<Sector> walledges; // Границы стен, не явяющихся границей зоны
    GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
    GS::Array<Sector> restedges; // Границы зоны
    GS::Array<Sector> gableedges;
} RoomEdges;

typedef struct
{
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
    short floorInd = 0; // Этаж
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
    bool ceil_by_slab = false; // Создавать потолок только по перекрытиям
    bool floor_by_slab = false; // Создавать потолок только по перекрытиям
    GS::UniString tip_pot = "";
    GS::UniString tip_pol = "";
    API_Guid zone_guid = APINULLGuid; // GUID базового элемента
    bool isValid = true; // Зона считана нормально
    bool create_all_elements = true; // Создавать элементы отделки
    bool create_ceil_elements = false; // Создавать элементы отделки потолка
    bool create_floor_elements = false; // Создавать элементы отделки пола
    bool create_wall_elements = false; // Создавать элементы отделки стен
    bool create_column_elements = false; // Создавать элементы отделки колонн
    bool create_reveal_elements = false; // Создавать элементы отделки откосов
} OtdRoom; // Структура для хранения информации о зоне

typedef GS::HashTable <API_Guid, OtdRoom> OtdRooms; // Словарь отделки всех зон
typedef GS::HashTable<API_Guid, GS::Array<API_Guid>> UnicElement; // Словарь GUID элемента - массив GUID зон, где они встречаются
typedef GS::HashTable<API_ElemTypeID, UnicElement> UnicElementByType;
typedef GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> UnicGUIDByType;

typedef GS::HashTable<GS::UniString, double> OtdMaterialAreaDict;
typedef GS::HashTable<GS::UniString, OtdMaterialAreaDict> OtdMaterialAreaDictByType;

typedef struct
{
    bool isValid = false; // Параметр считан успешно
    GS::Array<GS::UniString> rawnames; // Варианты имён параметров для считывания
    ParamValueData val = {}; // Прочитанное значение
} ReadParam;
typedef GS::HashTable<GS::UniString, ReadParam> ReadParams;

typedef struct
{
    short font = 1; // Индекс шрифта
    double fontsize = 2.5; // Размер шрифта
    double width_mat = 40; // Ширина для названия материала
    double width_area = 20; // Ширина для площади
    double width_no_breake_space = 1; // Ширина пробела
    double width_narow_space = 1; // Ширина пробела
    GS::UniString no_breake_space = u8"\u2007";
    GS::UniString narow_space = u8"\u202F";
    GS::UniString space_line = "";
    GS::UniString delim_line = "";
} ColumnFormat;

typedef GS::HashTable<GS::UniString, ColumnFormat> ColumnFormatDict;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ();

GS::HashTable<API_Guid, UnicGuidByGuid> Otd_GetOtd_ByZone (const GS::Array<API_Guid>& zones, const UnicGuid& finclassguids, ParamDictValue& propertyParams);

UnicGuidByGuid Otd_GetOtd_Parent (const GS::Array<API_Guid>& otd_elements, ParamDictValue& propertyParams);

void WriteOtdData_GetColumnfFormat (GS::UniString descripton, const GS::UniString& rawname, ColumnFormatDict& columnFormat);

void WriteOtdDataToRoom (const ColumnFormatDict& columnFormat, const OtdRoom& otd, ParamDictElement& paramToWrite, const ParamDictElement& paramToRead);

void WriteOtdDataToRoom_AddValue (OtdMaterialAreaDictByType& dct, const GS::UniString& rawname, const GS::UniString& mat, const double& area);

// -----------------------------------------------------------------------------
// Убираем задвоение Guid зон у элементов
// -----------------------------------------------------------------------------
void ClearZoneGUID (UnicElementByType& elementToRead, GS::Array<API_ElemTypeID>& typeinzone);

// -----------------------------------------------------------------------------
// Получение площади отделочной стены с учётом отверстий
// -----------------------------------------------------------------------------
double OtdWall_GetArea (const OtdWall& otdw);

// -----------------------------------------------------------------------------
// Получение информации из зоны о полгионах и находящейся в ней элементах
// -----------------------------------------------------------------------------
bool CollectRoomInfo (const Stories& storyLevels, API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead, GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone);


// -----------------------------------------------------------------------------
// Находит все перекрытия, котрые пересекают зону и не являются элементами отделки
// -----------------------------------------------------------------------------
void Floor_FindAll (GS::HashTable<API_Guid, GS::Array<API_Guid>>& slabsinzone, const UnicGuid& finclassguids, const GS::Array<API_Guid>& zones);

// -----------------------------------------------------------------------------
// Чтение данных об одном проёме
// -----------------------------------------------------------------------------
void Opening_Create_One (const Stories& storyLevels, const API_Guid& elGuid, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead);

void Opening_Add_One (const OtdOpening& op, const bool& is_fliped, const double& zBottom, const double& tBeg, const double& tEnd, const double& wallLength, OtdWall& wallotd);

bool OtdWall_Add_One (const API_Guid& wallguid, const Sector& walledge, const bool& is_fliped, const double& height, const double& zBottom, const short& floorInd, const double& thickness, OtdWall& wallotd);

// -----------------------------------------------------------------------------
// Создание стен-отделок для стен 
// -----------------------------------------------------------------------------
void OtdWall_Create_FromWall (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Создание стен-отделок для колонны 
// -----------------------------------------------------------------------------
void OtdWall_Create_FromColumn (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Обработка полов и потолков
// -----------------------------------------------------------------------------
void Floor_FindInOneRoom (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из стен/колонн/балок
// -----------------------------------------------------------------------------
void Param_GetForBase (ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite);

void Param_SetToRooms (GS::HashTable<GS::UniString, GS::Int32>& material_dict, OtdRoom& roominfo, ParamDictElement& paramToRead, ReadParams readparams);

void Param_SetToBase (const API_Guid& base_guid, const bool& base_flipped, GS::Array<ParamValueComposite>& otdcpmpoosite, ParamDictElement& paramToRead, ParamValue& param_composite, GS::UniString& fav_name);

void Param_SetComposite (const ParamValue& base_composite, const bool& base_flipped, GS::Array<ParamValueComposite>& otdcpmpoosite, GS::UniString& fav_name);

// -----------------------------------------------------------------------------
// Задание прочитанных параметров для окон
// -----------------------------------------------------------------------------
void Param_SetToWindows (OtdOpening& op, ParamDictElement& paramToRead, ReadParams readparams, const OtdWall& otdw);

// -----------------------------------------------------------------------------
// Создание стенок для откосов одного проёма
// -----------------------------------------------------------------------------
void OpeningReveals_Create_One (GS::Array<OtdSlab>& otdslabs, const OtdWall& otdw, OtdOpening& op, const Geometry::Vector2<double>& walldir_perp, GS::Array<OtdWall>& opw, double& otd_zBottom, double& otd_height_down, double& otd_height_main, double& otd_height_up, double& otd_height, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone);

void SetMaterialByType (OtdWall& otdw, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone);

void SetMaterialFinish (const OtdMaterial& material, GS::Array<ParamValueComposite>& base_composite);

// -----------------------------------------------------------------------------
// Разбивка созданных стен по высотам на основании информации из зоны
// -----------------------------------------------------------------------------
void OtdWall_Delim_All (GS::Array<OtdWall>& opw, OtdWall& otdw, double& otd_zBottom, double& otd_height_down, double& otd_height_main, double& otd_height_up, double& otd_height, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone);

// -----------------------------------------------------------------------------
// Добавляет стену с заданной высотой
// Удаляет отверстия, не попадающие в диапазон
// Подгоняет размер отверсий
// -----------------------------------------------------------------------------
bool OtdWall_Delim_One (OtdWall otdn, GS::Array<OtdWall>& opw, double height, double zBottom, TypeOtd& type, OtdMaterial& om_main, OtdMaterial& om_up, OtdMaterial& om_down,
        OtdMaterial& om_reveals, OtdMaterial& om_column, OtdMaterial& om_floor, OtdMaterial& om_ceil, OtdMaterial& om_zone);

// -----------------------------------------------------------------------------
// Получение очищенного полигона зоны, включая стены, колонны
// -----------------------------------------------------------------------------
void Edges_GetFromRoom (const API_ElementMemo& zonememo, API_Element& zoneelement, GS::Array<Sector>& walledges, GS::Array<Sector>& columnedges, GS::Array<Sector>& restedges, GS::Array<Sector>& gableedges);

void Floor_Create_All (const Stories& storyLevels, OtdRoom& roominfo, UnicGUIDByType& guidselementToRead, ParamDictElement& paramToRead);

void Floor_Create_One (const Stories& storyLevels, const short& floorInd, OtdSlab& poly, GS::Array<API_Guid>& slabGuids, GS::Array<OtdSlab>& otdslabs, GS::Array<OtdWall>& otdwall, ParamDictElement& paramToRead, TypeOtd type, OtdMaterial& material, bool only_on_slab);

bool Edge_FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge);

// -----------------------------------------------------------------------------
// Проверка наличия отрезка в массиве отрезков.
// В случае нахождение проверяется направление, при необходимости разворачивается
// -----------------------------------------------------------------------------
bool Edge_FindEdge (Sector& edge, GS::Array<Sector>& edges);

// -----------------------------------------------------------------------------
// Создание элементов отделки
// -----------------------------------------------------------------------------
void Draw_Elements (const Stories& storyLevels, OtdRooms& zoneelements, UnicElementByType& subelementByparent, ClassificationFunc::ClassificationDict& finclass, GS::Array<API_Guid>& deletelist);

// -----------------------------------------------------------------------------
// Построение отделочных стен (общая)
// -----------------------------------------------------------------------------
void OtdWall_Draw (const Stories& storyLevels, OtdWall& edges, UnicElementByType& subelementByparent);

void OtdBeam_Draw_Beam (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges);

bool OtdBeam_GetDefult_Beam (const GS::UniString& favorite_name, API_Element& beamelement, API_ElementMemo& beammemo);

void OtdBeam_Draw_Object (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges);

bool OtdBeam_GetDefult_Object (const GS::UniString& favorite_name, API_Element& beamelement, API_ElementMemo& beammemo);

// -----------------------------------------------------------------------------
// Построение отделочных стен аксессуаром
// -----------------------------------------------------------------------------
void OtdWall_Draw_Object (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges);

// -----------------------------------------------------------------------------
// Получение из избранного аксессуара стены для простроения 
// -----------------------------------------------------------------------------
bool OtdWall_GetDefult_Object (const GS::UniString& favorite_name, API_Element& wallobjelement, API_ElementMemo& wallobjmemo);

// -----------------------------------------------------------------------------
// Построение отделочных стен стандартной стеной
// -----------------------------------------------------------------------------
void OtdWall_Draw_Wall (const GS::UniString& favorite_name, const Stories& storyLevels, OtdWall& edges, UnicElementByType& subelementByparent);

// -----------------------------------------------------------------------------
// Получение из избранного стены для простроения 
// -----------------------------------------------------------------------------
bool OtdWall_GetDefult_Wall (const GS::UniString& favorite_name, API_Element& wallelement);

// -----------------------------------------------------------------------------
// Построение проёмов в отделочных стенах 
// -----------------------------------------------------------------------------
void Opening_Draw (API_Element& wallelement, OtdOpening& op, UnicElementByType& subelementByparent, const double& zBottom);

// -----------------------------------------------------------------------------
// Получение настроек проёмов в отделочных стенах 
// -----------------------------------------------------------------------------
bool Opening_GetDefult (const GS::UniString& favorite_name, API_Element& windowelement, API_ElementMemo& windowmemo);

// -----------------------------------------------------------------------------
// Построение потолка/пола (общая)
// -----------------------------------------------------------------------------
void Floor_Draw (const Stories& storyLevels, OtdSlab& otdslab, UnicElementByType& subelementByparent);

void Floor_Draw_Slab (const GS::UniString& favorite_name, const Stories& storyLevels, OtdSlab& otdslab);

void Floor_Draw_Object (const GS::UniString& favorite_name, const Stories& storyLevels, OtdSlab& otdslab);

bool Floor_GetDefult_Object (const GS::UniString& favorite_name, API_Element& slabobjelement, API_ElementMemo& slabobjmemo);

bool Floor_GetDefult_Slab (const GS::UniString& favorite_name, API_Element& slabelement);

// -----------------------------------------------------------------------------
// Связывание созданных элементов отделки с базовыми элементами
// -----------------------------------------------------------------------------
void SetSyncOtdWall (UnicElementByType& subelementByparent, ParamDictValue& propertyParams, ParamDictElement& paramToWrite);

void Class_SetClass (const OtdWall& op, const ClassificationFunc::ClassificationDict& finclass);

void Class_SetClass (const OtdOpening& op, const ClassificationFunc::ClassificationDict& finclass);

void Class_SetClass (const OtdSlab& op, const ClassificationFunc::ClassificationDict& finclass);

API_Guid Class_GetClassGuid (const TypeOtd type, const ClassificationFunc::ClassificationDict& finclass);

// -----------------------------------------------------------------------------
// Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
// -----------------------------------------------------------------------------
void Class_FindFinClass (ClassificationFunc::SystemDict& systemdict, ClassificationFunc::ClassificationDict& findict, UnicGuid& finclassguids);

bool Class_IsElementFinClass (const API_Guid& elGuid, const UnicGuid& finclassguids);

void Param_AddUnicElementByType (const API_Guid& parentguid, const API_Guid& guid, API_ElemTypeID elemtype, UnicElementByType& elementToRead);

void Param_AddUnicGUIDByType (const API_Guid& elGuid, API_ElemTypeID elemtype, UnicGUIDByType& guidselementToRead);

API_ElemTypeID Favorite_GetType (const GS::UniString& favorite_name);
MatarialToFavoriteDict Favorite_GetDict ();
void Favorite_ReadComposite (ParamDictValue& propertyParams, const ParamValue& param_composite, MatarialToFavoriteDict& favdict, ParamDictValue& paramToRead_favorite, const GS::UniString& fav_name);
void Favorite_FindName (ParamDictValue& propertyParams, MatarialToFavorite& favorite, const OtdMaterial& material, const TypeOtd& type, const API_ElemTypeID& draw_type, MatarialToFavoriteDict& favdict, ParamDictValue& paramToRead_favorite, ParamValue& param_composite);
bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element);
bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element, API_ElementMemo& memo);

void Param_ToParamDict (ParamDictValue& paramDict, ReadParams& zoneparams);
void Param_Property_FindInParams (ParamDictValue& propertyParams, ReadParams& zoneparams);
bool Param_Property_Read (const API_Guid& elGuid, ParamDictElement& paramToRead, ReadParams& zoneparams);
void Param_Material_Get (GS::HashTable<GS::UniString, GS::Int32>& material_dict, ParamValueData& val);
ReadParams Param_GetForWindowParams (ParamDictValue& propertyParams);
ReadParams Param_GetForRooms (ParamDictValue& propertyParams);

bool Check (const ClassificationFunc::ClassificationDict& finclass, const ParamDictValue& propertyParams, UnicGuid& finclassguids);
#endif
}

#endif
