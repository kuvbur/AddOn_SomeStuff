//------------ kuvbur 2022 ------------
#pragma once
#if !defined (ROOMBOOK_HPP)
#define	ROOMBOOK_HPP
#include    "Helpers.hpp"
#include    "Polygon2DData.h"
#include	"Polygon2DDataConv.h"
#include    "Sector2DData.h"
namespace AutoFunc
{
const double min_dim = 0.0001; // Минимальный размер элемента

typedef struct
{
    const GS::UniString material_main = "{@gdl:votw}";
    const GS::UniString material_up = "{@gdl:votw2}";
    const GS::UniString material_down = "{@gdl:votp}";
    const GS::UniString material_column = "{@gdl:votc}";
    const GS::UniString material_pot = "{@gdl:vots}";
    const GS::UniString height_down = "{@gdl:hpan}";
    const GS::UniString height_main = "{@gdl:hroom_pot}";
    const GS::UniString has_ceil = "{@gdl:offPot}";
    const GS::UniString tip_pot = "{@gdl:tip_pot}";
    const GS::UniString tip_pol = "{@gdl:tip_pol}";
} GDLParamRoom;

typedef struct
{
    const GS::UniString otdwall_class = "some_stuff_fin_walls";
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
    double lower = 0; // Привязка к низу стену
    bool has_reveal = false; // Наличие откосов у родительского проёма
    bool has_side = false; // Проём лежит на верхней или нижней грани стены
    double base_reveal_width = 0; // Глубина откоса базового проёма
    API_Guid base_guid = APINULLGuid; // GUID базового проёма
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки, в которую вставляется проём
} OtdOpening; // Проём в стене

typedef struct
{
    Geometry::Polygon2D poly;
    double zBottom = 0;
    double height = 0;
    API_AttributeIndex material = 0;
    API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента 
    API_ElemTypeID otd_type = API_SlabID; // Каким элементом строить (стеной/балкой)
    API_Guid base_guid = APINULLGuid; // GUID базового элемента
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
} OtdSlab;

typedef struct
{
    double height = 0; // Высота стены-отделки
    double zBottom = 0; // Аболютная координата z низа
    API_Coord begC; // Координата начала
    API_Coord endC; // Координата конца
    API_Guid base_guid = APINULLGuid; // GUID базового элемента
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
    bool base_flipped = false; // Базовая конструкция отзеркалена (для чтения состава)
    GS::Array<ParamValueComposite> base_composite; // Состав базового элемента (только отделочные слои)
    GS::Array<OtdOpening> openings; // Проёмы в стене-отделке
    API_AttributeIndex material = 0;
    API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента 
    API_ElemTypeID otd_type = API_WallID; // Каким элементом строить (стеной/балкой)
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
    double height = 0; // Высота зоны
    double zBottom = 0; // Аболютная координата z низа
    GS::HashTable <API_Guid, GS::Array<Sector>> walledges; // Границы стен, не явяющихся границей зоны
    GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
    GS::Array<Sector> edges; // Границы зоны
    GS::Array<API_WallPart> wallPart; // Участки стен в зоне
    GS::Array<API_BeamPart> beamPart; // Участки балок в зоне
    GS::Array<API_CWSegmentPart> cwSegmentPart; // Навесные стены в зоне
    GS::Array<API_Niche> niches; // Ниши в зоне
    API_AttributeIndex material = 0; // Материал в параметрах зоны
    API_AttributeIndex material_main = 0; // Основной материал отделки
    API_AttributeIndex material_up = 0;
    API_AttributeIndex material_down = 0;
    API_AttributeIndex material_column = 0;
    API_AttributeIndex material_reveal = 0;
    API_AttributeIndex material_pot = 0;
    GS::UniString smaterial = ""; // Материал в параметрах зоны
    GS::UniString smaterial_main = ""; // Основной материал отделки
    GS::UniString smaterial_up = "";
    GS::UniString smaterial_down = "";
    GS::UniString smaterial_column = "";
    GS::UniString smaterial_reveal = "";
    GS::UniString smaterial_pot = "";
    double height_down = 0; // Высота панелей
    double height_main = 0; // Высота основной отделки
    double height_up = 0; // Высота верхней части отделки
    GS::UniString rawname_up = "";
    GS::UniString rawname_main = "";
    GS::UniString rawname_down = "";
    GS::UniString rawname_column = "";
    OtdSlab poly; // Полгон зоны для полов/потолков
    GS::Array<API_Guid> floorslab; // Перекрытия в уровне пола
    GS::Array<API_Guid> ceilslab; // Перекрытия в уровне потолка
    GS::Array<OtdWall> otdwall; // Стены-отделки, созданные для расчётов и отрисовки
    GS::Array<OtdSlab> otdslab; // Потолки/полы для построения
    bool has_ceil = true;
    bool has_floor = true;
    GS::UniString tip_pot = "";
    GS::UniString tip_pol = "";
} OtdRoom; // Структура для хранения информации о зоне
typedef GS::HashTable <API_Guid, OtdRoom> OtdRooms; // Словарь отделки всех зон
typedef GS::HashTable<API_Guid, GS::Array<API_Guid>> UnicElement; // Словарь GUID элемента - массив GUID зон, где они встречаются
typedef GS::HashTable<API_ElemTypeID, UnicElement> UnicElementByType;
typedef GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> UnicGUIDByType;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ();

bool IsOtdClass (const API_Guid& elGuid, const UnicGuid& finclassguids);

// -----------------------------------------------------------------------------
// Убираем задвоение Guid зон у элементов
// -----------------------------------------------------------------------------
void ClearZoneGUID (UnicElementByType& elementToRead, GS::Array<API_ElemTypeID>& typeinzone);

// -----------------------------------------------------------------------------
// Получение информации из зоны о полгионах и находящейся в ней элементах
// -----------------------------------------------------------------------------
bool CollectRoomInfo (const Stories& storyLevels, API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead);

// -----------------------------------------------------------------------------
// Чтение данных об одном проёме
// -----------------------------------------------------------------------------
void ReadOneWinDoor (const Stories& storyLevels, const API_Guid& elGuid, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall);

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
void GetParamForBase (API_Guid& elGuid, ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite);

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из зон
// -----------------------------------------------------------------------------
void GetParamForRooms (API_Guid& elGuid, ParamDictValue& propertyParams, ParamDictValue& paramDict);

// -----------------------------------------------------------------------------
// Проверка свойств зон и замена имен
// -----------------------------------------------------------------------------
void SetParamToRooms (OtdRooms& roomsinfo, ParamDictElement& paramToRead);

// -----------------------------------------------------------------------------
// Запись прочитанных свойств в отделочные стены
// -----------------------------------------------------------------------------
void SetParamToBase (OtdRooms& roomsinfo, ParamDictElement& paramToRead, UnicGUIDByType& guidselementToRead, ParamValue& param_composite);

// -----------------------------------------------------------------------------
// Создание стенок для откосов проёмов
// -----------------------------------------------------------------------------
void CreateOpeningReveals (OtdRooms& roomsinfo);

// -----------------------------------------------------------------------------
// Создание стенок для откосов одного проёма
// -----------------------------------------------------------------------------
void CreateOpeningReveals (const OtdWall& otdw, OtdOpening& op, const Geometry::Vector2<double>& walldir_perp, GS::Array<OtdWall>& opw);

// -----------------------------------------------------------------------------
// Разбивка созданных стен по высотам на основании информации из зоны
// -----------------------------------------------------------------------------
void DelimOtdWalls (OtdRooms& roomsinfo);

// -----------------------------------------------------------------------------
// Добавляет стену с заданной высотой
// Удаляет отверстия, не попадающие в диапазон
// Подгоняет размер отверсий
// -----------------------------------------------------------------------------
bool DelimOneWall (OtdWall otdn, GS::Array<OtdWall>& opw, double height, double zBottom, API_AttributeIndex& material, GS::UniString& smaterial);

// -----------------------------------------------------------------------------
// Получение очищенного полигона зоны, включая стены, колонны
// -----------------------------------------------------------------------------
void GetZoneEdges (const API_ElementMemo& zonememo, API_Element& zoneelement, GS::Array<Sector>& walledges, GS::Array<Sector>& columnedges, GS::Array<Sector>& restedges, GS::Array<Sector>& gableedges);

void FloorRooms (const Stories& storyLevels, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

void FloorOneRoom (const Stories& storyLevels, OtdSlab& poly, GS::Array<API_Guid>& slabGuids, GS::Array<OtdSlab>& otdslabs, GS::Array<OtdWall> otdwall, bool on_top);

bool FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge);

// -----------------------------------------------------------------------------
// Проверка наличия отрезка в массиве отрезков.
// В случае нахождение проверяется направление, при необходимости разворачивается
// -----------------------------------------------------------------------------
bool FindEdge (Sector& edge, GS::Array<Sector>& edges);

void DrawEdges (const Stories& storyLevels, OtdRooms& zoneelements, UnicElement& subelementByparent, ClassificationFunc::ClassificationDict& finclass);
void DrawEdge (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, UnicElement& subelementByparent);

// -----------------------------------------------------------------------------
// Связывание созданных элементов отделки с базовыми элементами
// -----------------------------------------------------------------------------
void SetSyncOtdWall (UnicElement& subelementByparent, ParamDictValue& propertyParams);

GSErrCode ConstructPolygon2DFromElementMemo (const API_ElementMemo& memo, Geometry::Polygon2D& poly);

GSErrCode ConvertPolygon2DToAPIPolygon (const Geometry::Polygon2D& polygon, API_Polygon& poly, API_ElementMemo& memo);

void Do_CreateWindow (API_Element& wallelement, OtdOpening& op, UnicElement& subelementByparent);

void Do_CreateSlab (const Stories& storyLevels, API_Element& slabelement, OtdSlab& otdslab, UnicElement& subelementByparent);

// -----------------------------------------------------------------------------
// Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
// -----------------------------------------------------------------------------
void FindFinClass (ClassificationFunc::SystemDict& systemdict, ClassificationFunc::ClassificationDict& findict, UnicGuid& finclassguids);

}

#endif
