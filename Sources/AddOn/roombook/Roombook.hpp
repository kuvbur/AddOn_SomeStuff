//------------ kuvbur 2022 ------------
#pragma once
#if !defined(ROOMBOOK_HPP)
    #define ROOMBOOK_HPP

    // SDK подключается раньше стандартных заголовков.
    // clang-format off
    #include "ACAPinc.h"
// clang-format on

    #include <cstddef>

    #include "CoordinateSearchCache.hpp"
    #include "Helpers.hpp"
    #include "roombook/RoombookConstants.hpp"

namespace Roombook {
    struct CoordinateSearchResult {
        GSErrCode error;
        API_Guid guid;
    };

    using CoordinateSearchCache = ExactCoordinateSearchCache<CoordinateSearchResult>;
    #ifndef ServerMainVers_2400
    void RoomBook_Run ();
    #else
        #ifdef ServerMainVers_2700
            #define ACAPI_Element_Get3DInfo ACAPI_ModelAccess_Get3DInfo
            #define ACAPI_3D_GetComponent ACAPI_ModelAccess_GetComponent
        #endif

    // Входные снимки, кэши чтения и результаты сопоставления с основаниями.

    enum class RoomSurfaceStatus { NotRead, Extracted, Empty, ReadError, UnsupportedVersion };

    struct RoomSurfaceVertex {
        API_Coord3D local = {};
        API_Coord3D world = {};
    };

    struct RoomSurfaceContour {
        bool hole = false;
        GS::Array<RoomSurfaceVertex> vertices;
    };

    // Native3D хранит SDK-индексы; PreparedAnalytic — собственные конечные грани без native UMAT.
    enum class RoomSurfaceOrigin { Native3D, PreparedAnalytic };

    struct RoomSurface {
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
        Int32 analyticFaceId = 0;
        Int32 preparedWallIndex = -1;
        Int32 bodyIndex = 0;
        Int32 polygonIndex = 0;
        API_AttributeIndex modelMaterialIndex = {};
        API_Coord3D normalLocal = {};
        API_Coord3D normalWorld = {};
        double planeOffset = 0;
        GS::Array<RoomSurfaceContour> contours;
    };

    struct RoomSurfaceBody {
        Int32 index = 0;
        GS::Array<RoomSurface> surfaces;
    };

    struct RoomSurfaceReadError {
        GS::UniString stage;
        GSErrCode code = NoError;
    };

    // Индексы исходных граней различают контакты внутри одного снимка; они не являются постоянными GUID.
    struct RoomSurfaceSnapshot {
        API_Guid guid = APINULLGuid;
        RoomSurfaceStatus status = RoomSurfaceStatus::NotRead;
        GS::Array<RoomSurfaceBody> bodies;
        GS::Array<RoomSurfaceReadError> errors;

        bool IsComplete () const {
            return status == RoomSurfaceStatus::Extracted || status == RoomSurfaceStatus::Empty;
        }
    };

    // Преобразует собственные контуры и ориентированную нормаль; вырожденную матрицу не принимает.
    bool RoomSurface_Transform (const API_Tranmat &matrix, RoomSurface &surface);

    // Оригинальный элемент: кэшируются и успешное чтение, и ошибка SDK.
    struct ElementRead {
        API_Element element = {};
        GSErrCode error = NoError;
    };

    // Сырые классификационные пары с кодом ошибки SDK; семантику ошибки выбирает потребитель.
    struct ClassificationRead {
        GS::Array<GS::Pair<API_Guid, API_Guid>> items;
        GSErrCode error = NoError;
    };

    // Value-снимок API_RoomRelation: перечисления и части не владеют хендлами, niches не читаем.
    struct RoomRelationValues {
        #ifdef ServerMainVers_2600
        GS::HashTable<API_ElemType, GS::Array<API_Guid>> elementsGroupedByType;
        #else
        GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> elementsGroupedByType;
        #endif
        GS::Array<API_WallPart> wallPart;
        GS::Array<API_BeamPart> beamPart;
    };

    struct ConnectedElementsRead {
        API_ElemTypeID type = API_ZombieElemID;
        API_ElemFilterFlags filter = APIFilt_None;
        API_ElemVariationID variation = APIVarId_Generic;
        API_Guid renovationFilterGuid = APINULLGuid;
        GS::Array<API_Guid> items;
        GSErrCode error = NoError;
    };

    // Только один этап чтения: после записи модели этот кэш больше не используется.
    struct RoomSurfaceCache {
        GS::HashTable<API_Guid, RoomSurfaceSnapshot> snapshots;
        // Кэши read-only стадии: действительны до первой записи модели.
        GS::HashTable<API_Guid, ElementRead> elements;
        GS::HashTable<API_Guid, ClassificationRead> classifications;
        GS::HashTable<API_Guid, RoomRelationValues> relations;
        GS::HashTable<API_Guid, GS::Array<ConnectedElementsRead>> connectedElements;
    };

    RoomSurfaceSnapshot RoomSurfaces_Read (const API_Elem_Head &head, RoomSurfaceCache *cache = nullptr);

    // Callback позволяет regression проверить настоящий storage-путь без вызовов SDK.
    template <class Read>
    ConnectedElementsRead ReadConnectedElementsCached (const API_Guid &guid,
                                                       API_ElemTypeID type,
                                                       API_ElemFilterFlags filter,
                                                       API_ElemVariationID variation,
                                                       const API_Guid &renovationFilterGuid,
                                                       RoomSurfaceCache *cache,
                                                       Read &&read) {
        if (cache != nullptr) {
            if (const auto *entries = cache->connectedElements.GetPtr (guid)) {
                for (const auto &entry : *entries) {
                    if (entry.type == type && entry.filter == filter && entry.variation == variation &&
                        entry.renovationFilterGuid == renovationFilterGuid) {
                        return entry;
                    }
                }
            }
        }
        ConnectedElementsRead result;
        result.type = type;
        result.filter = filter;
        result.variation = variation;
        result.renovationFilterGuid = renovationFilterGuid;
        result.error = read (result.items);
        if (cache != nullptr) {
            if (!cache->connectedElements.ContainsKey (guid))
                cache->connectedElements.Add (guid, GS::Array<ConnectedElementsRead> ());
            cache->connectedElements.Get (guid).Push (result);
        }
        return result;
    }

    ConnectedElementsRead ConnectedElements_Read (const API_Guid &guid,
                                                  API_ElemTypeID type,
                                                  API_ElemFilterFlags filter,
                                                  RoomSurfaceCache *cache = nullptr,
                                                  API_ElemVariationID variation = APIVarId_Generic,
                                                  const API_Guid &renovationFilterGuid = APINULLGuid);

    struct RoomSurfaceSource {
        API_Guid baseGuid = APINULLGuid;
        API_ElemTypeID baseType = API_ZombieElemID;
        API_Guid geometryGuid = APINULLGuid;
        RoomSurfaceSnapshot snapshot;
        // Параметрическая геометрия сознательно игнорирует SEO/подрезки (решение владельца).
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
    };

    struct MeshFloorSource {
        RoomSurfaceSource surface;
        bool hasBottom = false;
    };

    struct RoomSurfaceSourceFace {
        API_Guid baseGuid = APINULLGuid;
        API_ElemTypeID baseType = API_ZombieElemID;
        API_Guid geometryGuid = APINULLGuid;
        Int32 bodyIndex = 0;
        Int32 polygonIndex = 0;
        // Материал модельной грани; сам по себе не устанавливает слой многослойного основания.
        API_AttributeIndex modelMaterialIndex = {};
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
        Int32 analyticFaceId = 0;
        Int32 preparedWallIndex = -1;
    };

    enum class RoomSurfacePieceStatus { Matched, Unmatched, Ambiguous };

    struct RoomSurfacePieceContour {
        bool hole = false;
        GS::Array<API_Coord3D> vertices;
    };

    struct RoomSurfacePiece {
        Int32 zoneBodyIndex = 0;
        Int32 zonePolygonIndex = 0;
        RoomSurfacePieceStatus status = RoomSurfacePieceStatus::Unmatched;
        API_Guid baseGuid = APINULLGuid;
        API_ElemTypeID baseType = API_ZombieElemID;
        API_Coord3D normal = {};
        double planeOffset = 0;
        double area = 0;
        GS::Array<RoomSurfacePieceContour> contours;
        // Ссылки на все реальные грани участка; покрытие не доказывает компонент/стройматериал.
        GS::Array<RoomSurfaceSourceFace> sources;
    };

    struct RoomSurfacePartition {
        API_Guid zoneGuid = APINULLGuid;
        bool complete = false;
        GS::Array<RoomSurfacePiece> pieces;
        GS::Array<RoomSurfaceReadError> errors;
    };

    // Не читает SDK-базу и не создаёт элементы; входы — собственные проверяемые снимки.
    // Несколько разных оснований дают Ambiguous без выбора первого; незаявленные площади — Unmatched.
    // complete подтверждает разбиение снимков, но не разрешимость всех оснований или writer-геометрии.
    RoomSurfacePartition RoomSurfaces_Partition (const RoomSurfaceSnapshot &zone,
                                                 const GS::Array<RoomSurfaceSource> &sources);

    enum class RoomSurfaceBeamFace { Bottom, Top, LeftSide, RightSide };

    struct RoomSurfaceBeamPiece {
        RoomSurfaceBeamFace face = RoomSurfaceBeamFace::Bottom;
        double area = 0;
        GS::Array<API_Coord3D> vertices;
    };

    struct RoomSurfaceBeamFinish {
        API_Guid baseGuid = APINULLGuid;
        API_Guid geometryGuid = APINULLGuid;
        double tBeg = 0;
        double tEnd = 0;
        GS::Array<RoomSurfaceBeamPiece> pieces;
    };

    struct RoomSurfaceBeamPartitions {
        bool complete = false;
        GS::Array<RoomSurfaceBeamFinish> beams;
        GS::Array<RoomSurfaceReadError> errors;
    };

    // beamPart выбирает балку/связную часть; при zone != nullptr низ и бока пересекаются с 3D-зоной.
    // tBeg/tEnd — исходные проекции SDK AC25, не параметры [0,1]; переводятся в расстояния по оси.
    // Обрезка по вертикали комнаты [zBottom, zTop]. Прямые балки с постоянным сечением.
    // Без zone применяется прежняя обрезка по осевому интервалу; с zone учитываются высотные ступени.
    // Ошибочное сечение или отверстие в конечном участке даёт явную ошибку без частичных pieces.
    RoomSurfaceBeamFinish RoomBeamFinish_Build (const RoomSurfaceSource &beamSource,
                                                const API_Coord &beg,
                                                const API_Coord &end,
                                                double tBeg,
                                                double tEnd,
                                                double zBottom,
                                                double zTop,
                                                RoomSurfaceBeamPartitions &result,
                                                const RoomSurfaceSnapshot *zone = nullptr);

    struct RoomSurfaceCandidateError {
        GS::UniString stage;
        GSErrCode code = NoError;
    };

    struct RoomSurfaceCandidates {
        API_Guid zoneGuid = APINULLGuid;
        bool complete = false;
        GS::Array<RoomSurfaceSource> sources;
        // Участки балок в зоне из API_RoomRelation.beamPart (копируются до disposal).
        GS::Array<API_BeamPart> beamParts;
        GS::Array<RoomSurfaceCandidateError> errors;
    };

    using MeshFloorIndex = GS::HashTable<API_Guid, GS::Array<MeshFloorSource>>;

    typedef GS::HashTable<TypeOtd, UnicGuid> UnicGuidByTypeOtd;
    typedef GS::HashTable<API_Guid, UnicGuidByTypeOtd> UnicGuidByBase;

    bool Otd_IsNativeTypeForFinish (TypeOtd type_otd, API_ElemTypeID type_subguid);

    UnicGuidByBase Otd_BuildByParent (GS::HashTable<API_Guid, TypeOtd> &otd_elements,
                                      UnicGuidByGuid &parentdict,
                                      bool &has_base_element);

    // Описание одного проёма внутри отделочной стены.
    // Здесь хранится его геометрия, привязка к базовой стене и состояние откосов.
    struct OtdOpening {
        double zBottom = 0; // Аболютная координата z низа
        double height = 0;  // Высота проёма
        double width = 0;   // Ширина проёма
        double objLoc = 0;  // Расстояние от начала стены для середины проёма
        double lower = 0;   // Привязка к низу стены
        bool reflected = false;
        bool ref_side = false;            // Сторона четверти в системе координат базовой стены
        bool reveal_from_sill = false;    // Отделка находится со стороны подоконника, а не за коробкой
        bool has_reveal = false;          // Наличие откосов у родительского проёма
        bool create_reveal = true;        // Создавать элементы отделки откосов для данного проёма
        bool surface_geometry = false;    // Вырез контакта: геометрия не корректируется по откосам
        double base_reveal_width = 0;     // Глубина откоса базового проёма
        double base_frame_width = 0;      // Толщина коробки, м; половина добавляется только к заходу пола
        API_Guid base_guid = APINULLGuid; // GUID базового проёма
        // Перекрывающийся аналитический вырез хранит все реальные IDs без выбора одного владельца.
        GS::Array<API_Guid> source_opening_guids;
        API_Guid otd_guid = APINULLGuid; // GUID созданного проёма
    }; // Проём в стене

    struct OtdMaterial {
        short material = 0;                                         // Индекс материала покрытия
        short bmaterial = 0;                                        // Индекс многослойки / строительного материала
        API_ModelElemStructureType structtype = API_BasicStructure; // Тип структуры отделочного элемента
        GS::UniString smaterial = EMPTYSTRING;                      // Имя материала
        GS::UniString rawname = EMPTYSTRING;                        // Имя свойства для записи
        GS::UniString rawname_bytype = EMPTYSTRING;                 // Имя свойства для записи c разбивкой
    };

    struct MatarialToFavorite {
        GS::UniString name = EMPTYSTRING;         // Имя избранного
        API_ElemTypeID type = API_ZombieElemID;   // Тип элемента в избранном
        GS::Array<ParamValueComposite> composite; // Состав элемента (только отделочные слои)
        bool is_composite_read = false;
        double thickness = 0; // Полная толщина Slab в метрах; 0 — не прочитана.
    };

    typedef GS::HashTable<GS::UniString, MatarialToFavorite> MatarialToFavoriteDict;

    struct OtdLinearSegment {
        API_Coord3D begC = {0, 0, 0};
        API_Coord3D endC = {0, 0, 0};
    };

    struct OtdLinearElement {
        OtdLinearSegment seg;
        double ang_begC = 90 * DEGRAD; // Угол подрезки начала
        double ang_endC = 90 * DEGRAD; // Угол подрезки конца
        double height = 0;             // Высота стены-отделки
        double width = 0;              // Ширина (для откоса)
        double length = 0;             // Длина (для откоса)
        short floorInd = 0;            // Этаж

        // Длина сегмента по координатам начала и конца
        double Length () const {
            return std::hypot (std::hypot (seg.endC.x - seg.begC.x, seg.endC.y - seg.begC.y), seg.endC.z - seg.begC.z);
        }
    };

    struct OtdDrawData {
        API_Guid base_guid = APINULLGuid;            // GUID базового элемента
        API_Guid otd_guid = APINULLGuid;             // GUID созданного/найденного элемента
        API_ElemTypeID base_type = API_ZombieElemID; // Тип базового элемента
        API_ElemTypeID draw_type = API_ZombieElemID; // Тип отрисовываемого элемента
        TypeOtd type = NoSet;                        // В какую графу заносить элемент (пол, потолок, стены и т.д.)
        OtdMaterial material;
        MatarialToFavorite favorite;
        GSErrCode draw_error = NoError;
        bool is_upper_reveal = false; // Геометрический верх проёма, независимо от высотной полосы type.
        bool write_succeeded = false;
        bool was_created = false;
    };

    struct RoomBookFinishResult {
        API_Guid zone_guid = APINULLGuid;
        OtdDrawData draw;
        double length = 0;
        double area = 0;
        bool has_length = false;
        bool has_area = false;
    };

    struct OtdSlab {
        Geometry::Polygon2D poly;
        OtdDrawData draw;
        double zBottom = 0; // Аболютная координата z низа
        short floorInd = 0; // Этаж
        double height = 0;
        double length_skirting = 0; // Длина примыкающих плинтусов
        double reference_z = 0;     // Исходная абсолютная отметка reference plane основания
        API_Coord plane_origin = {0, 0};
        double slope_x = 0;
        double slope_y = 0;
        GS::UniString tip = EMPTYSTRING;               // Тип потолка/пола из зоны
        GS::Array<ParamValueComposite> base_composite; // Состав базового элемента (только отделочные слои)
        bool isValid = true;
        bool surface_geometry = false;
    };

    void OtdLinearSegment_SetZBottom (OtdLinearElement &el, double zBottom);
    double OtdLinearSegment_GetZBottom (const OtdLinearElement &el);

    double GetOffsetFromStory (const OtdLinearElement &el, const Stories &stories);

    // Представление отделочной стены, созданной для одной зоны.
    // Помимо геометрии хранит состав, тип отделки и связанные с ней проёмы.
    struct OtdWall {
        OtdLinearElement geom; // Геометрия отделочной стены
        OtdDrawData draw;
        double floor_core_offset = 0; // Глубина до ядра/первой отделки из ПОЛНОГО состава основания
        bool floor_core_offset_valid = false;
        double base_th = 0;                            // Толщина базовой стены (для расчёта откосов)
        bool base_flipped = false;                     // Базовая конструкция отзеркалена (для чтения состава)
        GS::Array<ParamValueComposite> base_composite; // Состав базового элемента (только отделочные слои)
        GS::Array<OtdOpening> openings;                // Проёмы в стене-отделке
        bool isValid = true;
        // Ноль сохраняет прежние настройки; угол задаётся только адаптером 3D-грани.
        double slant_angle = 0;
        bool surface_geometry = false;
        // Горизонтальный подъём плоскости на метр высоты для подрезки полос.
        double rise_x = 0;
        double rise_y = 0;
        // Происхождение боковины перепада сохраняется при смене типа материальными правилами.
        bool slab_step = false;
        bool slab_step_writer_eligible = true;
    }; // Структура со стенами для отделки

    struct OtdBeam {
        OtdLinearElement geom;
        OtdDrawData draw;
        GS::UniString tip = EMPTYSTRING; // Тип пола/потолка из зоны
        Int32 wallIndex = -1;            // Индекс в итоговом otdwall для последующей связи с созданной стеной.
        Int32 slabIndex = -1;
    };

    // Краткая модель комнаты/зоны с уже рассчитанными параметрами отделки.
    // В этом объекте собираются стены, полы, потолки и все свойства, нужные для создания элементов.
    struct OtdRoom {
        OtdMaterial om_up;                                    // Отделка стен выше потолка
        OtdMaterial om_main;                                  // Отделка стен основная
        OtdMaterial om_down;                                  // Отделка низа стен
        OtdMaterial om_column;                                // Отделка колонн
        OtdMaterial om_reveals;                               // Отделка откосов
        OtdMaterial om_floor;                                 // Отделка пола
        OtdMaterial om_ceil;                                  // Отделка потолка
        OtdMaterial om_zone;                                  // Отделка из параметров зоны
        double height = 0;                                    // Высота зоны
        double zBottom = 0;                                   // Аболютная координата z низа
        short floorInd = 0;                                   // Этаж
        GS::HashTable<API_Guid, GS::Array<Sector>> walledges; // Границы стен, не явяющихся границей зоны
        GS::Array<Sector> columnedges;                        // Границы колонн, не явяющихся границей зоны
        GS::Array<Sector> edges;                              // Границы зоны
        // Разбиение сохраняется до чтения правил и любых записей отделки в модель.
        RoomSurfaceSnapshot surfaceSnapshot;
        RoomSurfaceCandidates surfaceCandidates;
        RoomSurfacePartition surfacePartition;
        GS::Array<API_WallPart> wallPart;           // Участки стен в зоне
        GS::Array<API_BeamPart> beamPart;           // Участки балок в зоне
        GS::Array<API_CWSegmentPart> cwSegmentPart; // Навесные стены в зоне
        GS::Array<API_Niche> niches;                // Ниши в зоне
        double height_down = 0;                     // Высота панелей
        double height_main = 0;                     // Высота основной отделки
        double height_up = 0;                       // Высота верхней части отделки
        double height_ceil = 0;                     // Высота до потолка (height_down+height_main)
        OtdSlab poly;                               // Полгон зоны для полов/потолков
        GS::Array<API_Guid> floorslab;              // Перекрытия в уровне пола
        GS::Array<API_Guid> ceilslab;               // Перекрытия в уровне потолка
        // Полные исходные снимки кандидатов; роли discovery не выводятся заново из отметки зоны.
        GS::Array<RoomSurfaceSource> slabSources;
        // Typed Mesh-кандидаты: 3D-снимки заполняются только для включённого пола.
        GS::Array<MeshFloorSource> meshFloorSources;
        // Неизменяемые основания/проёмы отделены от очищаемых и разбиваемых результатов ниже.
        GS::Array<OtdWall> preparedWalls;
        GS::Array<OtdSlab> preparedSlabs;
        GS::Array<OtdWall> otdwall;       // Стены-отделки, созданные для расчётов и отрисовки
        GS::Array<OtdSlab> otdslab;       // Потолки/полы для построения
        GS::Array<OtdBeam> otdbeam_floor; // Плинтуса
        GS::Array<OtdBeam> otdbeam_ceil;  // Потолочные плинтуса
        bool has_ceil = true;
        bool has_floor = true;
        bool ceil_by_slab = false;                 // Создавать потолок только по перекрытиям
        bool floor_by_slab = false;                // Создавать потолок только по перекрытиям
        GS::UniString tip_pot = EMPTYSTRING;       // Тип потолка для записи в аксессуар
        GS::UniString tip_pol = EMPTYSTRING;       // Тип пола для записи в аксессуар
        GS::UniString tip_plint_pot = EMPTYSTRING; // Тип плинтуса потолка
        GS::UniString tip_plint_pol = EMPTYSTRING; // Тип плинтуса пола
        GS::UniString tip_otd = EMPTYSTRING;       // Тип отделки для разбивки
        API_Guid zone_guid = APINULLGuid;          // GUID базового элемента
        bool isValid = true;                       // Зона считана нормально
        bool create_ceil_elements = true;          // Создавать элементы отделки потолка
        bool create_floor_elements = true;         // Создавать элементы отделки пола
        bool create_wall_elements = true;          // Создавать элементы отделки стен
        bool create_column_elements = true;        // Создавать элементы отделки колонн
        bool create_reveal_elements = true;        // Создавать элементы отделки откосов
    }; // Структура для хранения информации о зоне

    typedef GS::HashTable<API_Guid, OtdRoom> OtdRooms; // Словарь отделки всех зон
    typedef GS::HashTable<API_Guid, GS::Array<API_Guid>>
        UnicElement; // Словарь GUID элемента - массив GUID зон, где они встречаются
    typedef GS::HashTable<API_ElemTypeID, UnicElement> UnicElementByType;
    typedef GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> UnicGUIDByType;

    typedef GS::HashTable<GS::UniString, double> OtdMaterialAreaDict;
    typedef GS::HashTable<TypeOtd, OtdMaterialAreaDict> OtdMaterialAreaDictByType;
    typedef GS::HashTable<GS::UniString, OtdMaterialAreaDictByType> OtdMaterialAreaDictByOtdType;

    struct ReadParam {
        bool isValid = false;              // Параметр считан успешно
        GS::Array<GS::UniString> rawnames; // Варианты имён параметров для считывания
        ParamValueData val;                // Прочитанное значение
    };

    typedef GS::HashTable<GS::UniString, ReadParam> ReadParams;

    struct ColumnFormat {
        short font = 1;                   // Индекс шрифта
        double fontsize = 2.5;            // Размер шрифта
        double width_mat = 40;            // Ширина для названия материала
        double width_area = 20;           // Ширина для площади
        double width_no_breake_space = 1; // Ширина пробела
        double width_narow_space = 1;     // Ширина пробела
        GS::UniString no_breake_space = reinterpret_cast<const char *> (u8"\u2007");
        GS::UniString narow_space = reinterpret_cast<const char *> (u8"\u202F");
        GS::UniString space_line = EMPTYSTRING;
        GS::UniString delim_line = EMPTYSTRING;
    };

    typedef GS::HashTable<GS::UniString, ColumnFormat> ColumnFormatDict;

    // Расчёт хранит подготовленные значения; свойства модели здесь ещё не записаны.
    struct MaterialSummary {
        ParamDictElement paramToWrite;
        ColumnFormatDict columnFormat;
        OtdMaterialAreaDictByOtdType dct_bytype;
        GS::Array<API_Guid> zones_bytype;
    };

    // Подготовка геометрии отделки; создание элементов объявлено отдельно ниже.
    // Геометрические адаптеры без SDK-чтения, назначения материала и создания.
    // При отказе выход не меняется; zBottom — абсолютная отметка контакта.
    // Один параллелограмм без отверстий; линия и уклон задают контактную плоскость, не габарит грани.
    bool SurfaceWall_Build (const RoomSurfacePiece &piece, short floorInd, OtdWall &wall);
    // Готовые нижние отрезки стен без доходящих до низа проёмов; повтор заменяет массив.
    // При ошибке напольный массив пуст; потолочный массив и исходные стены не меняет.
    bool FloorSkirtingSegments_Build (OtdRoom &room);
    // Оба массива из итоговых Otd-геометрий; отверстия и Z концов сохраняются.
    // Ошибка оставляет оба массива пустыми; модель и исходные стены/перекрытия не изменяет.
    bool SkirtingSegments_Build (OtdRoom &room, const GS::Array<OtdSlab> &slabFaces);
    // Конечный пространственный стык двух заранее выбранных граней стены и пола/потолка.
    // Координаты — мировые, метры; проёмы/отверстия обеих граней сохраняются.
    // true с пустым массивом — нет стыка; false — некорректные данные или почти параллельные плоскости.
    // Повтор заменяет выход, ошибка оставляет его пустым. Выбор граней, GUID и создание — у вызывающего.
    bool SkirtingContact_Build (const RoomSurface &wallFace,
                                const RoomSurface &slabFace,
                                GS::Array<OtdLinearSegment> &segments);

    // После правил вычитает только принятое конечное покрытие боковин перепадов.
    // При непредставимом остатке сохраняет исходный контакт и возвращает false.
    bool SlabContacts_DeduplicateAccepted (OtdRoom &room);
    // Удаляет избыточные вершины сонаправленных рёбер с одинаковыми edge user data.
    // SDK сохраняет покрытие, отверстия и геометрию дуг; возвращает число удалённых вершин.
    USize PolygonVertices_RemoveCollinear (Geometry::Polygon2D &polygon);
    // Доводит до ядра только пол; затем очищает лишние вершины пола и потолка.
    // Стены — read-only источник конечных контактов.
    bool FloorContours_ExtendToCore (OtdRoom &room);
        #if defined(TESTING)
    bool ProfileWall_BuildFloorExclusion (const Geometry::MultiPolygon2D &profile,
                                          const API_WallType &wall,
                                          double wallBase,
                                          double zoneBottom,
                                          const GS::Array<OtdOpening> &openings,
                                          Geometry::MultiPolygon2D &exclusion);
    bool FloorContours_SubtractProfileWalls (OtdRoom &room, const Geometry::MultiPolygon2D &exclusion);
        #endif
    // Входит существующим полом одного отделочного проёма на глубину откоса; новые OtdSlab не создаются.
    bool FloorContour_ExtendToOpening (OtdRoom &room, const OtdWall &openingWall, const OtdOpening &opening);
    // Добавляет одну стену в плоскости контакта; прямоугольные вырезы и отверстия задаёт проёмами.
    // Диагональные рёбра в координатах ось/отметка отвергаются без частичного добавления.
    bool SurfaceWalls_Build (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdWall> &walls);
    // Учитывает roomEdge, объединяет интервалы и доводит только крайние внешние концы.
    // contacts — неизменённый набор контактов комнаты, необходимый для сохранения разрывов.
    // Без частей основания сохраняет стену; невалидная/непредставимая часть даёт пустой результат.
    GS::Array<OtdWall> SurfaceWall_ClampToParts (const OtdWall &wall,
                                                 const API_Coord &wallBeg,
                                                 const API_Coord &wallEnd,
                                                 const GS::Array<API_WallPart> &parts,
                                                 const GS::Array<Sector> &roomEdges,
                                                 double roomBottom,
                                                 const GS::Array<OtdWall> &contacts);
    // Сопоставляет вырезы с окнами/дверями того же основания, не меняя покрытие стены.
    // Native-неоднозначность отвергается; analyticOverlap сохраняет множественные IDs без ложного owner.
    bool SurfaceOpenings_Link (OtdWall &wall, const GS::Array<OtdWall> &openingWalls, bool analyticOverlap = false);
    // Объединяет стеновые контакты с одинаковым полным набором исходных граней до проверки writer.
    // Основание, геометрия, тело, полигон, материал снимка и направление нормали должны совпадать.
    // Чужие/непредставимые участки сохраняет; при успехе добавляет результат к grouped.
    // При ошибке объединения выход не меняется. Материалы и исходный partition не изменяет.
    bool SurfaceWallPieces_Group (const GS::Array<RoomSurfacePiece> &pieces,
                                  short floorInd,
                                  GS::Array<RoomSurfacePiece> &grouped);
    // Горизонтальные контакты с отверстиями; тип пола/потолка определяется нормалью зоны.
    bool SurfaceSlabs_Build (const RoomSurfacePiece &piece, short floorInd, GS::Array<OtdSlab> &slabs);

    // Конечная верхняя грань Mesh до ограничения комнатой; координаты XY мировые, метры.
    struct MeshFloorFiniteSupport {
        API_Guid ownerGuid = APINULLGuid;
        API_Guid geometryGuid = APINULLGuid;
        API_ElemTypeID ownerType = API_ZombieElemID;
        Int32 bodyIndex = 0;
        Int32 polygonIndex = 0;
        API_AttributeIndex modelMaterialIndex = {};
        OtdSlab plane;
        Geometry::MultiPolygon2D projected; // Исходная проекция с отверстиями, до clipping и конкуренции.
        Geometry::MultiPolygon2D admitted;  // Допуск по локальному верху/низу, ещё без ограничения комнатой.
    };

    // Вклады копланарных граней могут перекрываться; это provenance, не распределение площади по материалам.
    struct MeshFloorWinnerContribution {
        UIndex supportIndex = 0;
        UIndex floorIndex = 0; // Индекс в полном floors, включая прежний префикс.
        Geometry::MultiPolygon2D domain;
    };

    struct MeshFloorProvenance {
        GS::Array<MeshFloorFiniteSupport> supports;
        GS::Array<MeshFloorWinnerContribution> contributions;
    };

    // Только конечная область допустимых Mesh; за её пределами сохраняется прежний маршрут Slab.
    // slabSupports содержит уже допущенные верхние контакты Slab, не отфильтрованные по максимуму Mesh.
    // Локально выбирается верхняя опора; неоднозначность/невалидный снимок — отказ без изменения выхода.
    bool MeshFloorFinishes_BuildProjected (const Geometry::Polygon2D &footprint,
                                           double roomFloor,
                                           short floorInd,
                                           const GS::Array<MeshFloorSource> &meshes,
                                           const GS::Array<OtdSlab> &slabSupports,
                                           GS::Array<OtdSlab> &floors,
                                           Geometry::MultiPolygon2D &meshCoverage);

    // Только подготовка: при успехе дополняет Mesh provenance; при false все выходы сохранены.
    // Проверяет допустимость и вне комнаты; Slab-вкладов не выдаёт, слои материала по UMAT не определяет.
    // floorIndex действителен до перестановки/слияния floors; mixed caller обязан перенести соответствие.
    bool MeshFloorFinishes_BuildProjectedWithProvenance (const Geometry::Polygon2D &footprint,
                                                         double roomFloor,
                                                         short floorInd,
                                                         const GS::Array<MeshFloorSource> &meshes,
                                                         const GS::Array<OtdSlab> &slabSupports,
                                                         GS::Array<OtdSlab> &floors,
                                                         Geometry::MultiPolygon2D &meshCoverage,
                                                         MeshFloorProvenance &provenance);

    // Только provenance Slab: не использовать MeshFloorFiniteSupport как ложный Mesh-owner.
    // PreparedAnalytic не является native face: body/polygon/UMAT остаются 0/{}.
    struct SlabFloorFiniteSupport {
        API_Guid ownerGuid = APINULLGuid;
        API_Guid geometryGuid = APINULLGuid;
        API_ElemTypeID ownerType = API_ZombieElemID;
        RoomSurfaceOrigin origin = RoomSurfaceOrigin::Native3D;
        UIndex preparedIndex = 0; // Только при origin == PreparedAnalytic; индекс исходного prepared.
        Int32 bodyIndex = 0;
        Int32 polygonIndex = 0;
        API_AttributeIndex modelMaterialIndex = {};
        OtdSlab plane;
        Geometry::MultiPolygon2D projected;
        Geometry::MultiPolygon2D admitted;
    };

    struct SlabFloorWinnerContribution {
        UIndex supportIndex = 0;
        UIndex floorIndex = 0;
        Geometry::MultiPolygon2D domain;
    };

    struct SlabFloorProvenance {
        GS::Array<SlabFloorFiniteSupport> supports;
        GS::Array<SlabFloorWinnerContribution> contributions;
    };

    struct MixedFloorProvenance {
        MeshFloorProvenance mesh;
        SlabFloorProvenance slab;
    };

    bool MeshFloor_ValidateSourceIdentity (const OtdRoom &room, const MixedFloorProvenance &provenance);
    bool MeshFloor_ValidateFinalDomains (const OtdRoom &room, const MixedFloorProvenance &provenance);
    bool MeshFloor_OpeningDemandIsCovered (const OtdRoom &room,
                                           const OtdWall &wall,
                                           const OtdOpening &opening,
                                           const MixedFloorProvenance &provenance);
    bool MeshFloor_CoreDemandIsCovered (const OtdRoom &room, const MixedFloorProvenance &provenance);

    // Геометрический план пола; сам по себе не разрешает native-запись.
    struct MixedFloorGeometry {
        GS::Array<OtdSlab> floors;
        GS::Array<OtdWall> steps;
        Geometry::MultiPolygon2D meshDomain;
        Geometry::MultiPolygon2D changedDomain;
        double rejectedArea = 0;
    };

    // Победители внутри всей допустимой области Mesh и все части прежнего пола снаружи.
    // Без Mesh делегирует прежнему Slab-маршруту; при отказе result не меняется.
    bool OtdSlab_BuildFiniteMixedGeometry (const RoomSurfaceSnapshot &zone,
                                           const GS::Array<RoomSurfaceSource> &sources,
                                           const GS::Array<API_Guid> &roles,
                                           short floorInd,
                                           const OtdSlab &nominal,
                                           bool onlyOnSlab,
                                           const GS::Array<OtdSlab> &prepared,
                                           double roomFloor,
                                           const GS::Array<MeshFloorSource> &meshes,
                                           MixedFloorGeometry &result);

    // Append API: сохраняет result/provenance prefix; индексы относятся ко всему result.floors.
    // False не публикует ни геометрию, ни supports/contributions. Production Mesh не включает.
    bool OtdSlab_BuildFiniteMixedGeometryWithProvenance (const RoomSurfaceSnapshot &zone,
                                                         const GS::Array<RoomSurfaceSource> &sources,
                                                         const GS::Array<API_Guid> &roles,
                                                         short floorInd,
                                                         const OtdSlab &nominal,
                                                         bool onlyOnSlab,
                                                         const GS::Array<OtdSlab> &prepared,
                                                         double roomFloor,
                                                         const GS::Array<MeshFloorSource> &meshes,
                                                         MixedFloorGeometry &result,
                                                         MixedFloorProvenance &provenance);

    // XY-покрытие комнаты пересекается с реальными гранями оснований; Z остаётся исходным.
    // Прежние приоритет по убыванию, наибольшие части, исключение отверстий и опциональные остаток/перепады.
    // Для потолка номинальная отметка зоны участвует в выборе наравне с перекрытиями.
    // Владелец при равных отметках неоднозначен; фактическая Z грани не округляется.
    bool OtdSlab_BuildProjectedFinishes (const RoomSurfaceSnapshot &zone,
                                         const GS::Array<RoomSurfaceSource> &sources,
                                         const GS::Array<API_Guid> &roles,
                                         TypeOtd type,
                                         short floorInd,
                                         GS::Array<OtdSlab> &slabs,
                                         double &rejectedArea,
                                         const OtdSlab *nominal = nullptr,
                                         bool onlyOnSlab = true,
                                         GS::Array<OtdWall> *walls = nullptr,
                                         const GS::Array<OtdSlab> *prepared = nullptr);

    // Дополняет сводку одной комнаты; не выбирает зоны на создание и не записывает модель.
    void OtdData_AddRoomToMaterialSummary (const OtdRoom &otd, ParamDictElement &paramToRead, MaterialSummary &summary);

    // Дополняет подготовленные свойства сводкой по типу помещения, не записывая модель.
    void OtdData_WriteRoomMaterialData (OtdRooms &roomsinfo, ParamDictElement &paramToRead, MaterialSummary &summary);

    // -----------------------------------------------------------------------------
    // Запуск построения и записи информации об отделке
    // -----------------------------------------------------------------------------
    void RoomBook_Run (GS::Array<RoomBookFinishResult> *results = nullptr);

    // Непустой список задаёт целевые зоны; пустой сохраняет выбор из выделения/всего проекта. Вход не изменяется.
    void RoomBook_Run (GS::Array<RoomBookFinishResult> *results, const GS::Array<API_Guid> &zones);

    GS::HashTable<API_Guid, UnicGuidByBase> Otd_GetOtd_ByZone (const GS::Array<API_Guid> &zones,
                                                               const UnicGuid &finclassguids,
                                                               const ClassificationFunc::ClassificationDict &finclass,
                                                               bool &has_base_element,
                                                               UnicGuid &reserv_elements,
                                                               RoomSurfaceCache *cache = nullptr);

    void OtdData_CalcForRoom (const ColumnFormatDict &columnFormat,
                              const OtdRoom &otd,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              OtdMaterialAreaDictByOtdType &dct_bytype);

    void OtdData_WriteToRoom (const ColumnFormatDict &columnFormat,
                              const API_Guid &zone_guid,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              const OtdMaterialAreaDictByType &dct,
                              const GS::HashTable<TypeOtd, GS::UniString> &paramnamebytype);

    // -----------------------------------------------------------------------------
    // Убираем задвоение Guid зон у элементов
    // -----------------------------------------------------------------------------
    void Zone_ClearDuplicateGUIDs (UnicElementByType &elementToRead);

    // -----------------------------------------------------------------------------
    // Получение площади отделочной стены с учётом отверстий
    // -----------------------------------------------------------------------------
    double OtdWall_GetArea (const OtdWall &otdw);

    // Конечный XY-discovery сохраняет тип Mesh; native-тела читаются отдельно после включения пола.
    bool Mesh_FindAll (MeshFloorIndex &meshesinzone,
                       const UnicGuid &finclassguids,
                       const GS::HashTable<API_Guid, Geometry::Polygon2D> &zonePolygons,
                       RoomSurfaceCache *cache = nullptr);

    void OtdOpening_Add (const OtdOpening &op,
                         bool is_fliped,
                         double zBottom,
                         double tBeg,
                         double tEnd,
                         double wallLength,
                         OtdWall &wallotd);

    bool OtdWall_Add_One (const API_Guid &wallguid,
                          const Sector &walledge,
                          bool is_fliped,
                          double height,
                          double zBottom,
                          short floorInd,
                          double thickness,
                          OtdWall &wallotd);

    // -----------------------------------------------------------------------------
    // Чистая подготовка конечных оснований без чтения native 3D; отказ оставляет bases неизменными.
    bool OtdWall_BuildPreparedSource (const API_Element &element,
                                      double bottom,
                                      const GS::Array<OtdOpening> &openings,
                                      GS::Array<OtdWall> &bases,
                                      RoomSurfaceSource &source);
    bool OtdSlab_BuildPreparedSource (const API_Element &element,
                                      double reference,
                                      const Geometry::Polygon2D &polygon,
                                      GS::Array<OtdSlab> &bases,
                                      RoomSurfaceSource &source);

    void Param_SetToRooms (GS::HashTable<GS::UniString, GS::Int32> &material_dict,
                           OtdRoom &roominfo,
                           ParamDictElement &paramToRead,
                           ReadParams &readparams);

    bool Param_SetToBase (const API_Guid &base_guid,
                          bool base_flipped,
                          GS::Array<ParamValueComposite> &otdComposite,
                          ParamDictElement &paramToRead,
                          ParamDictCompositeElement &paramCompositeToRead,
                          const ParamValue &param_composite,
                          GS::UniString &fav_name,
                          OtdWall *floorWall = nullptr);

    void Param_SetComposite (const ParamComposite &base_composite,
                             bool base_flipped,
                             GS::Array<ParamValueComposite> &otdcpmpoosite,
                             GS::UniString &fav_name,
                             bool has_fin);

    // -----------------------------------------------------------------------------
    // Задание прочитанных параметров для окон
    // -----------------------------------------------------------------------------
    void Param_SetToWindows (OtdOpening &op,
                             ParamDictElement &paramToRead,
                             ReadParams &readparams,
                             const OtdWall &otdw);

    // -----------------------------------------------------------------------------
    // Создание стенок для откосов одного проёма
    // -----------------------------------------------------------------------------
    void OtdOpening_CreateReveals (GS::Array<OtdSlab> &otdslabs,
                                   const OtdWall &otdw,
                                   OtdOpening &op,
                                   const Geometry::Vector2<double> &walldir_perp,
                                   GS::Array<OtdWall> &opw,
                                   double &otd_zBottom,
                                   double &otd_height_down,
                                   double &otd_height_main,
                                   double &otd_height_up,
                                   double &otd_height,
                                   OtdMaterial &om_main,
                                   OtdMaterial &om_up,
                                   OtdMaterial &om_down,
                                   OtdMaterial &om_reveals,
                                   OtdMaterial &om_column,
                                   OtdMaterial &om_floor,
                                   OtdMaterial &om_ceil,
                                   OtdMaterial &om_zone);

    void OtdWall_SetMaterialByType (OtdWall &otdw,
                                    OtdMaterial &om_main,
                                    OtdMaterial &om_up,
                                    OtdMaterial &om_down,
                                    OtdMaterial &om_reveals,
                                    OtdMaterial &om_column,
                                    OtdMaterial &om_floor,
                                    OtdMaterial &om_ceil,
                                    OtdMaterial &om_zone);

    // -----------------------------------------------------------------------------
    // Разбивка созданных стен по высотам на основании информации из зоны
    // -----------------------------------------------------------------------------
    void OtdWall_Delim_All (GS::Array<OtdWall> &opw,
                            OtdWall &otdw,
                            double otd_zBottom,
                            double otd_height_down,
                            double otd_height_main,
                            double otd_height_up,
                            double otd_height,
                            OtdMaterial &om_main,
                            OtdMaterial &om_up,
                            OtdMaterial &om_down,
                            OtdMaterial &om_reveals,
                            OtdMaterial &om_column,
                            OtdMaterial &om_floor,
                            OtdMaterial &om_ceil,
                            OtdMaterial &om_zone);

    // -----------------------------------------------------------------------------
    // Добавляет стену с заданной высотой
    // Удаляет отверстия, не попадающие в диапазон
    // Подгоняет размер отверсий
    // -----------------------------------------------------------------------------
    bool OtdWall_Delim_One (const OtdWall &otdn,
                            GS::Array<OtdWall> &opw,
                            double height,
                            double zBottom,
                            TypeOtd &type,
                            OtdMaterial &om_main,
                            OtdMaterial &om_up,
                            OtdMaterial &om_down,
                            OtdMaterial &om_reveals,
                            OtdMaterial &om_column,
                            OtdMaterial &om_floor,
                            OtdMaterial &om_ceil,
                            OtdMaterial &om_zone);

    // -----------------------------------------------------------------------------
    // Создание элементов отделки
    // -----------------------------------------------------------------------------
    void OtdRooms_Draw (GS::HashTable<API_Guid, UnicGuidByBase> &exsistot_byzone,
                        GS::Array<API_Guid> &deletelist,
                        OtdRooms &roomsinfo,
                        const Stories &storyLevels,
                        ClassificationFunc::ClassificationDict &finclass,
                        UnicGuid &reserv_elements,
                        ParamDictElement &paramToWrite,
                        const API_ElemTypeID *elementTypes,
                        std::size_t elementTypeCount,
                        GS::Int32 &nPhase,
                        GS::Array<RoomBookFinishResult> *results = nullptr);

    // -----------------------------------------------------------------------------
    // Построение отделочных стен (общая)
    // -----------------------------------------------------------------------------
    // Расходует один совместимый GUID из пула; геометрию и классификацию не изменяет.
    API_Guid OtdWall_TakeExistingFinish (UnicGuid &pool,
                                         const API_Element &target,
                                         const GS::HashTable<API_Guid, API_Element> &existing);

    void OtdWall_Draw (const Stories &storyLevels, OtdWall &edges, UnicElementByType &subelementByparent);

    // -----------------------------------------------------------------------------
    // Получение из избранного стены для простроения
    // Если не найдено избранное - вернёт настроенный элемент по-умолчанию
    // -----------------------------------------------------------------------------
    bool Wall_GetDefault (const GS::UniString &favorite_name,
                          const TypeOtd &type,
                          API_Element &wallelement,
                          bool activate = true);
    // -----------------------------------------------------------------------------
    // Получение из избранного балки для простроения
    // Если не найдено избранное - вернёт настроенный элемент по-умолчанию
    // -----------------------------------------------------------------------------
    bool Beam_GetDefault (const GS::UniString &favorite_name,
                          const TypeOtd &type,
                          API_Element &beamelement,
                          API_ElementMemo &beammemo);

    // -----------------------------------------------------------------------------
    // Получение из избранного объекта для простроения
    // Если не найдено избранное - вернёт настроенный элемент по-умолчанию
    // -----------------------------------------------------------------------------
    bool Object_GetDefault (const GS::UniString &favorite_name,
                            const TypeOtd &type,
                            API_Element &element,
                            API_ElementMemo &memo);
    // -----------------------------------------------------------------------------
    // Получение из избранного перекрытия для простроения
    // Если не найдено избранное - вернёт настроенный элемент по-умолчанию
    // -----------------------------------------------------------------------------
    bool Slab_GetDefault (const GS::UniString &favorite_name, const TypeOtd &type, API_Element &element);

    // Создаёт односкатный пол по контактной плоскости с настройками выбранного перекрытия.
    // Вызывается внутри существующей undo-транзакции; при отказе GUID результата не меняется.
    GSErrCode Roof_Draw (const Stories &storyLevels,
                         OtdSlab &floor,
                         const API_Element &slabSettings,
                         const GS::Array<OtdSlab> *neighbors = nullptr);

    API_Guid Class_GetClassGuid (const TypeOtd &type, const ClassificationFunc::ClassificationDict &finclass);

    TypeOtd Class_GetOtdTypeByClass (const API_Guid &classguid,
                                     const ClassificationFunc::ClassificationDict &finclass,
                                     const API_ElemTypeID &type);

    // -----------------------------------------------------------------------------
    // Поиск классов для отделочных стен (some_stuff_fin_ в описании класса)
    // -----------------------------------------------------------------------------
    void Class_FindFinClass (ClassificationFunc::ClassificationDict &findict, UnicGuid &finclassguids);

    bool Class_IsElementFinClass (const API_Guid &elGuid,
                                  const UnicGuid &finclassguids,
                                  API_Guid &classguid,
                                  RoomSurfaceCache *cache = nullptr);

    void Param_AddUnicElementByType (const API_Guid &parentguid,
                                     const API_Guid &guid,
                                     API_ElemTypeID elemtype,
                                     UnicElementByType &elementToRead);

    bool Favorite_GetDict (MatarialToFavoriteDict &favdict);
    void Favorite_ReadComposite (const ParamValue &param_composite,
                                 ParamDictValue &paramToRead_favorite,
                                 const GS::UniString &fav_name,
                                 MatarialToFavoriteDict &favdict);
    bool Favorite_GetByName (const GS::UniString &favorite_name, API_Element &element);
    bool Favorite_ReadByName (const GS::UniString &favorite_name, API_Element &element);
    bool Favorite_GetByName (const GS::UniString &favorite_name, API_Element &element, API_ElementMemo &memo);

    // -----------------------------------------------------------------------------
    // Создаёт избранное с заданным именем из настроек по умолчанию инструмента типа отделки
    // и кладёт его в заданную папку (пустая папка — избранное без папки).
    // Полы и потолки — перекрытия (Slab_GetDefault), плинтус пола и потолка, а также верхний
    // откос reveal up — балки (Beam_GetDefault), остальные стены и откосы — стены (Wall_GetDefault).
    // Для пустого имени только собирает настройки, не сохраняя избранное.
    // -----------------------------------------------------------------------------
    bool Favorite_CreateOtd (const TypeOtd &type, const GS::UniString &name, const GS::UniString &folder);

    void Param_Property_FindInParams (ReadParams &zoneparams);
    bool Param_Property_Read (const API_Guid &elGuid, ParamDictElement &paramToRead, ReadParams &zoneparams);
    ReadParams Param_GetForWindowParams ();
    ReadParams Param_GetForRooms ();

    bool Roombook_CheckFinishConfiguration (const ClassificationFunc::ClassificationDict &finclass,
                                            UnicGuid &finclassguids);

    // Тот же контракт, но собирает все ошибки в errors (по одной на строку).
    bool Roombook_CheckFinishConfiguration (const ClassificationFunc::ClassificationDict &finclass,
                                            UnicGuid &finclassguids,
                                            GS::UniString &errors);

    // Для JSON: собирает классы отделки из систем классификации и выполняет общую проверку конфигурации,
    // возвращая список ошибок текстом (по одной на строку) для передачи в json.
    bool Roombook_CheckFinishConfiguration (GS::UniString &errors);

    // Для JSON: собирает классы отделки из систем классификации и выполняет общую проверку конфигурации.
    bool Roombook_CheckFinishConfiguration ();

    // Контейнеры одного запуска живут от сбора до записи; порядок типов ссылается на единственный массив Data.
    struct RoomProcessingContext {
        ClassificationFunc::ClassificationDict finclass;
        UnicGuid finclassguids;
        Stories storyLevels;
        CoordinateSearchCache coordinateCache;
        RoomSurfaceCache surfaceCache;
        GS::Array<API_Guid> deletelist;
        GS::HashTable<API_Guid, UnicGuidByBase> exsistot_byzone;
        OtdRooms roomsinfo;
        UnicElementByType elementToRead;
        ParamDictElement paramToRead;
        ParamValue param_composite;
        ParamDictValue paramDict_favorite;
        MatarialToFavoriteDict favdict;
        UnicGUIDByType guidselementToRead;
        GS::HashTable<API_Guid, GS::Array<OtdOpening>> openinginwall;
        GS::HashTable<API_Guid, GS::Array<API_Guid>> slabsinzone;
        MeshFloorIndex meshesinzone;
        const API_ElemTypeID *elementTypes = nullptr;
        std::size_t elementTypeCount = 0;
    };

    struct ReadParamsForRoomBook {
        ReadParams windowParams = Param_GetForWindowParams ();
        ReadParams roomParams = Param_GetForRooms ();
    };

    bool RoomProcessingContext_Prepare (RoomProcessingContext &context, const GS::Array<API_Guid> &zones);
    bool RoomProcessingContext_BuildElementReadIndex (const GS::Array<API_Guid> &zones,
                                                      RoomProcessingContext &context,
                                                      GS::UniString &funcname,
                                                      GS::Int32 &nPhase);
    bool RoomProcessingContext_ProcessElementsForRoomData (RoomProcessingContext &context,
                                                           GS::UniString &funcname,
                                                           GS::Int32 &nPhase);
    bool RoomProcessingContext_PreparePartitions (RoomProcessingContext &context);
    void RoomProcessingContext_ReadElementParameters (RoomProcessingContext &context,
                                                      ReadParamsForRoomBook &readParams);
    bool OtdRooms_ProcessFinishes (RoomProcessingContext &context,
                                   ReadParamsForRoomBook &readParams,
                                   ParamDictCompositeElement &paramCompositeToRead,

                                   bool has_base_element,
                                   GS::UniString &funcname,
                                   GS::Int32 &nPhase);

    #endif
} // namespace Roombook

#endif
