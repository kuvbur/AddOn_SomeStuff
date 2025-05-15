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
    API_Guid otd_guid = APINULLGuid; // GUID созданного проёма
    GS::UniString favorite_name = ""; // Имя избранного для создания элемента
} OtdOpening; // Проём в стене


typedef struct
{
    short material = 0; // Индекс материала
    GS::UniString smaterial = ""; // Имя материала
    GS::UniString rawname = ""; // Имя свойства для записи
    GS::UniString favorite = ""; // Имя избранного для создания элемента
} OtdMaterial;

typedef struct
{
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

typedef struct
{
    double height = 0; // Высота стены-отделки
    double zBottom = 0; // Аболютная координата z низа
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
    API_ElemTypeID otd_type = API_WallID; // Каким элементом строить (стеной/балкой)
    GS::UniString favorite_name = ""; // Имя избранного для создания элемента
    TypeOtd type = NoSet; // В какую графу заносить элемент (пол, потолок, стены и т.д.)
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
    bool create_all_elements = false; // Создавать элементы отделки
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
typedef GS::HashTable<TypeOtd, OtdMaterialAreaDict> OtdMaterialAreaDictByType;

typedef struct
{
    bool isValid = false; // Параметр считан успешно
    GS::Array<GS::UniString> rawnames; // Варианты имён параметров для считывания
    ParamValueData val = {}; // Прочитанное значение
} ReadParam;
typedef GS::HashTable<GS::UniString, ReadParam> ReadParams;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ();

void WriteOtdDataToRoom (OtdRoom& otd, ParamDictElement& paramToWrite, ParamDictElement& paramToRead);

void WriteOtdDataToRoom_AddValue (OtdMaterialAreaDictByType& dct, const TypeOtd& t, GS::UniString& mat, const double& area);

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

// -----------------------------------------------------------------------------
// Создание стен-отделок для стен 
// -----------------------------------------------------------------------------
void OtdWall_Create_One (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Создание стен-отделок для колонны 
// -----------------------------------------------------------------------------
void Column_Create_One (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Обработка полов и потолков
// -----------------------------------------------------------------------------
void Floor_FindInOneRoom (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из стен/колонн/балок
// -----------------------------------------------------------------------------
void Param_GetForBase (ParamDictValue& propertyParams, ParamDictElement& paramDict, ParamValue& param_composite);

void Param_SetToRooms (OtdRoom& roominfo, ParamDictElement& paramToRead, ReadParams& readparams);

void Param_SetToBase (OtdRoom& roominfo, ParamDictElement& paramToRead, ParamValue& param_composite);

bool Param_SetToRoomsFromProperty (ParamDictValue& params, OtdRoom& otd);

// -----------------------------------------------------------------------------
// Задание прочитанных параметров для окон
// -----------------------------------------------------------------------------
void Param_SetToWindows (OtdRoom& roominfo, ParamDictElement& paramToRead, ReadParams& readparams);

// -----------------------------------------------------------------------------
// Создание стенок для откосов проёмов
// -----------------------------------------------------------------------------
void OpeningReveals_Create_All (OtdRooms& roomsinfo);

// -----------------------------------------------------------------------------
// Создание стенок для откосов одного проёма
// -----------------------------------------------------------------------------
void OpeningReveals_Create_One (const OtdWall& otdw, OtdOpening& op, const Geometry::Vector2<double>& walldir_perp, GS::Array<OtdWall>& opw);

void SetMaterialByType (OtdRooms& roomsinfo);

// -----------------------------------------------------------------------------
// Разбивка созданных стен по высотам на основании информации из зоны
// -----------------------------------------------------------------------------
void OtdWall_Delim_All (OtdRooms& roomsinfo);

// -----------------------------------------------------------------------------
// Добавляет стену с заданной высотой
// Удаляет отверстия, не попадающие в диапазон
// Подгоняет размер отверсий
// -----------------------------------------------------------------------------
bool OtdWall_Delim_One (OtdWall otdn, GS::Array<OtdWall>& opw, double height, double zBottom, TypeOtd& type);

// -----------------------------------------------------------------------------
// Получение очищенного полигона зоны, включая стены, колонны
// -----------------------------------------------------------------------------
void Edges_GetFromRoom (const API_ElementMemo& zonememo, API_Element& zoneelement, GS::Array<Sector>& walledges, GS::Array<Sector>& columnedges, GS::Array<Sector>& restedges, GS::Array<Sector>& gableedges);

void Floor_Create_All (const Stories& storyLevels, OtdRooms& roomsinfo, UnicGUIDByType& guidselementToRead, ParamDictElement& paramToRead);

void Floor_Create_One (const Stories& storyLevels, OtdSlab& poly, GS::Array<API_Guid>& slabGuids, GS::Array<OtdSlab>& otdslabs, GS::Array<OtdWall>& otdwall, ParamDictElement& paramToRead, bool on_top);

bool Edge_FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge);

// -----------------------------------------------------------------------------
// Проверка наличия отрезка в массиве отрезков.
// В случае нахождение проверяется направление, при необходимости разворачивается
// -----------------------------------------------------------------------------
bool Edge_FindEdge (Sector& edge, GS::Array<Sector>& edges);

void Draw_Elements (const Stories& storyLevels, OtdRooms& zoneelements, UnicElementByType& subelementByparent, ClassificationFunc::ClassificationDict& finclass, GS::Array<API_Guid>& deletelist);

void OtdWall_Draw (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent);

void OtdWall_Draw_Object (const Stories& storyLevels, OtdWall& edges, API_Element& wallobjelement, API_ElementMemo& wallobjmemo, UnicElementByType& subelementByparent);

bool OtdWall_GetDefult_Object (const GS::UniString& favorite_name, API_Element& wallobjelement, API_ElementMemo& wallobjmemo);

void OtdWall_Draw_Wall (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, UnicElementByType& subelementByparent);

bool OtdWall_GetDefult_Wall (const GS::UniString& favorite_name, API_Element& wallelement);

void Opening_Draw (API_Element& wallelement, API_Element& windowelement, API_ElementMemo& windowmemo, OtdOpening& op, UnicElementByType& subelementByparent);

bool Opening_GetDefult (const GS::UniString& favorite_name, API_Element& windowelement, API_ElementMemo& memo);

void Floor_Draw (const Stories& storyLevels, API_Element& slabelement, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent);

void Floor_Draw_Slab (const Stories& storyLevels, API_Element& slabelement, OtdSlab& otdslab, UnicElementByType& subelementByparent);

void Floor_Draw_Object (const Stories& storyLevels, API_Element& slabobjelement, API_ElementMemo& slabobjmemo, OtdSlab& otdslab, UnicElementByType& subelementByparent);

bool Floor_GetDefult_Object (const GS::UniString& favorite_name, API_Element& slabobjelement, API_ElementMemo& slabobjmemo);

bool Floor_GetDefult_Slab (const GS::UniString& favorite_name, API_Element& slabelement);


// -----------------------------------------------------------------------------
// Связывание созданных элементов отделки с базовыми элементами
// -----------------------------------------------------------------------------
void SetSyncOtdWall (UnicElementByType& subelementByparent, ParamDictValue& propertyParams, ParamDictElement& paramToWrite);

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

API_ElemTypeID Favorite_GetType (const GS::UniString& favorite_name);
bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element);
bool Favorite_GetByName (const GS::UniString& favorite_name, API_Element& element, API_ElementMemo& memo);

void Param_ToParamDict (ParamDictValue& paramDict, ReadParams& zoneparams);
void Param_FindPropertyInParams (ParamDictValue& propertyParams, ReadParams& zoneparams);
ReadParams Param_GetForWindowParams (ParamDictValue& propertyParams);
ReadParams Param_GetForZoneParams (ParamDictValue& propertyParams);
#endif
}

#endif
