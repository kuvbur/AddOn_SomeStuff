//------------ kuvbur 2022 ------------
#pragma once
#ifndef HELPERS_HPP
#define HELPERS_HPP
#include "basicgeometry.h"
#include "ClassificationFunction.hpp"
#include "CommonFunction.hpp"
#include "StringConversion.hpp"
#include "SyncSettings.hpp"

#define SYNC_RESET 1
#define SYNC 2

#define DUMMY_MODE_UNDEF 0
#define DUMMY_MODE_ON 1
#define DUMMY_MODE_OFF 2

static const Int32 AddOnNameID = 1;
static const Int32 AddOnDescriptionID = 2;

static const Int32 UndoSyncId = 1;
static const Int32 SyncAllId = 2;
static const Int32 UndoReNumId = 3;
static const Int32 UndoSumId = 32;

static const Int32 TrueId = 4;
static const Int32 FalseId = 5;
static const Int32 ErrorSelectID = 6;
static const Int32 UndoDimRound = 7;

static const Int32 BuildingMaterialNameID = 8;
static const Int32 BuildingMaterialDescriptionID = 9;
static const Int32 BuildingMaterialDensityID = 10;
static const Int32 BuildingMaterialManufacturerID = 11;
static const Int32 ThicknessID = 12;
static const Int32 RenumIgnoreID = 13;
static const Int32 RenumAddID = 14;
static const Int32 RenumSkipID = 15;
static const Int32 BuildingMaterialCutFillID = 16;

static const Int32 N_StringID = 20;
static const Int32 NW_StringID = 21;
static const Int32 W_StringID = 22;
static const Int32 SW_StringID = 23;
static const Int32 S_StringID = 24;
static const Int32 SE_StringID = 25;
static const Int32 E_StringID = 26;
static const Int32 NE_StringID = 27;

static const Int32 Izm_StringID = 28;
static const Int32 Zam_StringID = 29;
static const Int32 Nov_StringID = 30;
static const Int32 Annul_StringID = 31;
static const Int32 RVI_StringID = 33;

static const Int32 SubElementHotFoundId = 34;
static const Int32 SubElementHotFoundId1 = 35;
static const Int32 SubElementHotFoundId2 = 36;
static const Int32 SubElementOtherPlanId = 37;
static const Int32 SubElementHiddenId = 38;
static const Int32 SubElementTotalId = 39;
static const Int32 SubElementNoSelectId = 40;
static const Int32 SubElementHalfId = 41;
static const Int32 SpecRuleNotFoundId = 42;
static const Int32 SpecRuleReadFoundId = 43;
static const Int32 SpecWriteNotFoundId = 44;
static const Int32 SpecEmptyListdId = 45;
static const Int32 SpecNotFoundParametersId = 46;
static const Int32 RoombookId = 47;
static const Int32 SubElementNotExsistId = 48;
static const Int32 SpecParamPlaceNotFoundId = 65;
static const Int32 SpecFlagOff = 66;

struct SortGUID
{
    GS::Array<API_Guid> guid = {};
};

struct SortInx
{
    GS::Array<UInt32> inx = {};
};

// Массив отрезков с указанием точки начала

struct OrientedSegments
{
    Vector2D cut_direction;
    Point2D cut_start;
    Point2D start;
};


int IsDummyModeOn ();

// -----------------------------------------------------------------------------
// Добавление отслеживания (для разных версий)
// -----------------------------------------------------------------------------
GSErrCode AttachObserver (const API_Guid& objectId, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool CheckElementType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// -----------------------------------------------------------------------------
bool IsElementEditable (const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType);

bool IsElementEditable (const API_Elem_Head& tElemHead, const SyncSettings& syncSettings, const bool needCheckElementType);

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// Возвращает тип элемента
// -----------------------------------------------------------------------------
bool IsElementEditable (const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType, API_ElemTypeID& eltype);

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// Настройки будут считаны при вызове функции
// -----------------------------------------------------------------------------
GS::Array<API_Guid> GetSelectedElements (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, bool addSubelement);

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов в соответсвии с настройками обработки
// -----------------------------------------------------------------------------
GS::Array<API_Guid> GetSelectedElements (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/, const SyncSettings& syncSettings, bool addSubelement);

// -----------------------------------------------------------------------------
// Возвращает GUID родительского элемента для API_SectElemType
// -----------------------------------------------------------------------------
void GetParentGUIDSectElem (const API_Guid& sectElemguid, API_Guid& parentguid, API_ElemTypeID& parentType);

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid SyncSettings
// -----------------------------------------------------------------------------
void CallOnSelectedElemSettings (void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings, GS::UniString& funcname, bool addSubelement);

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid);

// --------------------------------------------------------------------
// Поиск связанных элементов для определённого типа
// --------------------------------------------------------------------
void GetRelationsElement (const API_Guid& elemGuid, const API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid);

void test ();

// -----------------------------------------------------------------------------
// По заданному углу поворота и глобальному углу направления на север возвращает ориентацию объекта
// и текст с обозначением стороны света (RUS+ENG)
// -----------------------------------------------------------------------------
void CoordNorthAngle (double north, double angz, double& angznorth, GS::UniString& angznorthtxt, GS::UniString& angznorthtxteng);

// -----------------------------------------------------------------------------
// Вычисляет уголв поворота элемента по координатам его начала и конца
// -----------------------------------------------------------------------------
void CoordRotAngle (double sx, double sy, double ex, double ey, bool isFliped, double& angz);

// -----------------------------------------------------------------------------
// Проверяет наличие дробной части у угла с заданной точностью
// -----------------------------------------------------------------------------
bool CoordCorrectAngle (double angz, double& tolerance_ang, double& symb_rotangle_fraction, bool& bsymb_rotangle_correct_1000);

// -----------------------------------------------------------------------------
// Получение имени внутренних свойств по русскому имени
// -----------------------------------------------------------------------------
GS::UniString GetPropertyENGName (GS::UniString& name);

namespace PropertyHelpers
{
GS::UniString ToString (const API_Variant& variant, const FormatString& stringformat);
GS::UniString ToString (const API_Variant& variant);
GS::UniString ToString (const API_Property& property, const FormatString& stringformat);
GS::UniString ToString (const API_Property& property);
}

// -----------------------------------------------------------------------------
// Функции для работы с ParamDict и ParamValue
// -----------------------------------------------------------------------------
namespace ParamHelpers
{
// -----------------------------------------------------------------------------
// Возвращает строку в формате rawname
// Очищает от единиц измерения, добавляет скобки
// -----------------------------------------------------------------------------
GS::UniString NameToRawName (const GS::UniString& name, FormatString& formatstring);

// -----------------------------------------------------------------------------
// Получение размеров Морфа
// Формирует словарь ParamDictValue& pdictvalue со значениями
// -----------------------------------------------------------------------------
bool ReadMorphParam (const API_Element& element, ParamDictValue& pdictvalue);

// -----------------------------------------------------------------------------
// Задаёт источник получения информации по rawName
// -----------------------------------------------------------------------------
void SetParamValueSourseByName (ParamValue& pvalue);

// -----------------------------------------------------------------------------
// Получение координат объекта
// symb_pos_x , symb_pos_y, symb_pos_z
// Для панелей навесной стены возвращает центр панели
// Для колонны или объекта - центр колонны и отм. низа
// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool ReadCoords (const API_Element& element, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Замена имен параметров на значения в выражении
// Значения передаются словарём, вычисление значений см. GetParamValueDict
// -----------------------------------------------------------------------------
bool ReplaceParamInExpression (const ParamDictValue& pdictvalue, GS::UniString& expression);

bool GetParamValueForElements (const API_Guid& elemguid, const GS::UniString& rawname, const ParamDictElement& paramToRead, ParamValue& pvalue);

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool ParseParamNameMaterial (GS::UniString& expression, ParamDictValue& paramDict, bool fromMaterial = true);

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки {}
// -----------------------------------------------------------------------------
bool ParseParamName (GS::UniString& expression, ParamDictValue& paramDict);

// -----------------------------------------------------------------------------
// Добавление пустого значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void AddValueToParamDictValue (ParamDictValue& params, const GS::UniString& name);

// -----------------------------------------------------------------------------
// Проверяет необходимость добавления в словарь параметров
// Если в имени параметра содержится информация о номере аттрибута и имя такого параметра есть в словаре - вернёт истину
// -----------------------------------------------------------------------------
bool needAdd (ParamDictValue& params, GS::UniString& rawName);

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь ParamDict, если его там прежде не было
// --------------------------------------------------------------------
void AddParamValue2ParamDict (const API_Guid& elemGuid, ParamValue& param, ParamDictValue& paramToRead);

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void AddParamValue2ParamDictElement (const API_Guid& elemGuid, const ParamValue& param, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void AddParamValue2ParamDictElement (const ParamValue& param, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Сопоставляет параметры
// --------------------------------------------------------------------
bool CompareParamValue (ParamValue& paramFrom, ParamValue& paramTo, FormatString stringformat);

// --------------------------------------------------------------------
// Запись словаря ParamDictValue в словарь элементов ParamDictElement
// --------------------------------------------------------------------
void AddParamDictValue2ParamDictElement (const API_Guid& elemGuid, ParamDictValue& param, ParamDictElement& paramToRead);

// -----------------------------------------------------------------------------
// Добавление массива свойств в словарь
// -----------------------------------------------------------------------------
bool AddProperty (ParamDictValue& params, GS::Array<API_Property>& properties);

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void AddBoolValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const bool val);

// -----------------------------------------------------------------------------
// Добавление значения длины в словарь ParamDictValue
// -----------------------------------------------------------------------------
void AddLengthValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const double val);

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void AddDoubleValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const double val);

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void AddStringValueToParamDictValue (ParamDictValue& params, const API_Guid& elemGuid, const GS::UniString& rawName_prefix, const GS::UniString& name, const GS::UniString val);

// -----------------------------------------------------------------------------
// Список возможных префиксов типов параметров
// -----------------------------------------------------------------------------
void GetParamTypeList (GS::Array<GS::UniString>& paramTypesList);

// -----------------------------------------------------------------------------
// Конвертация значений ParamValue в свойства, находящиеся в нём
// Возвращает true если значения отличались
// -----------------------------------------------------------------------------
//bool ConvertToProperty (ParamValue& pvalue);

// -----------------------------------------------------------------------------
// Синхронизация ParamValue и API_Property
// Возвращает true и подготовленное для записи свойство в случае отличий
// TODO Переписать всё под запись ParamValue
// -----------------------------------------------------------------------------
bool ConvertToProperty (const ParamValue& pvalue, API_Property& property);

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue 
// Не добавляет отсутствующие в paramsTo элементы
// --------------------------------------------------------------------
void CompareParamDictValue (ParamDictValue& paramsFrom, ParamDictValue& paramsTo, bool addInNotEx /* = false*/);

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void CompareParamDictValue (ParamDictValue& paramsFrom, ParamDictValue& paramsTo);

// --------------------------------------------------------------------
// Чтение значений свойств в ParamDictValue
// --------------------------------------------------------------------
bool ReadProperty (const API_Guid& elemGuid, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Получение значения IFC свойств в ParamDictValue
// -----------------------------------------------------------------------------
#if !defined (AC_29)
bool ReadIFC (const API_Guid& elemGuid, ParamDictValue& params);
#endif
// -----------------------------------------------------------------------------
// Обработка данных о классификации
// -----------------------------------------------------------------------------
bool ReadClassification (const API_Guid& elemGuid, const ClassificationFunc::SystemDict& systemdict, ParamDictValue& paramByType);

// -----------------------------------------------------------------------------
// Обработка данных о аттрибутах
// -----------------------------------------------------------------------------
bool ReadAttributeValues (const API_Elem_Head& elem_head, ParamDictValue& propertyParams, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Получение ID элемента
// -----------------------------------------------------------------------------
bool ReadID (const API_Elem_Head& elem_head, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Получить значение GDL параметра по его имени или описанию в ParamValue
// -----------------------------------------------------------------------------
bool ReadGDL (const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись словаря параметров для множества элементов
// --------------------------------------------------------------------
GS::Array<API_Guid> ElementsWrite (ParamDictElement& paramToWrite);

// --------------------------------------------------------------------
// Запись ParamDictValue в один элемент
// --------------------------------------------------------------------
bool Write (const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись ParamDictElement в информацию о проекте
// --------------------------------------------------------------------
void InfoWrite (ParamDictElement& paramToWrite);

// --------------------------------------------------------------------
// Смена классификации
// --------------------------------------------------------------------
bool WriteClassification (const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись ParamDictValue в ID
// --------------------------------------------------------------------
void WriteID (const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись ParamDictValue в аттрибуты элемента (слой)
// --------------------------------------------------------------------
void WriteAttribute (const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись ParamDictValue в координаты элемента
// --------------------------------------------------------------------
void WriteCoord (const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись ParamDictValue в GDL параметры
// --------------------------------------------------------------------
void WriteGDL (const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Запись ParamDictValue в свойства
// --------------------------------------------------------------------
void WriteProperty (const API_Guid& elemGuid, ParamDictValue& params);

bool hasGlob (ParamDictValue& propertyParams);

bool hasInfo (ParamDictValue& propertyParams);

bool has_LocOrigin (ParamDictValue& propertyParams);

bool hasAttribute (ParamDictValue& propertyParams);

bool hasProperyDefinition (ParamDictValue& propertyParams);

bool hasUnreadProperyDefinition (ParamDictElement& paramToRead);

bool hasUnreadCoord (ParamDictElement& paramToRead);

bool hasUnreadAttribute (ParamDictElement& paramToRead);

bool hasUnreadInfo (ParamDictElement& paramToRead, ParamDictValue& propertyParams);

bool hasUnreadGlob (ParamDictElement& paramToRead, ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Заполнение словаря параметров для множества элементов
// --------------------------------------------------------------------
void ElementsRead (ParamDictElement& paramToRead, ParamDictValue& propertyParams, ClassificationFunc::SystemDict& systemdict);

// --------------------------------------------------------------------
// Заполнение словаря с параметрами
// --------------------------------------------------------------------
void Read (const API_Guid& elemGuid, ParamDictValue& params, ParamDictValue& propertyParams, ClassificationFunc::SystemDict& systemdict);

void Array2ParamValue (GS::Array<ParamValueData>& pvalue, ParamValueData& pvalrezult);
bool ConvertToParamValue (ParamValueData& pvalue, const API_AddParID& typeIDr, const GS::UniString& pstring, const double& preal);
bool ConvertToParamValue (ParamValueData& pvalue, const API_AddParID& typeIDr, const GS::Array<GS::UniString>& pstring, const GS::Array<double>& preal, const GS::Int32& dim1, const GS::Int32& dim2);

// -----------------------------------------------------------------------------
// Конвертация параметров библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ConvertBoolToParamValue (ParamValue& pvalue, const GS::UniString& paramName, const bool boolValue);

// -----------------------------------------------------------------------------
// Конвертация аттрибута в ParamValue
// -----------------------------------------------------------------------------
bool ConvertAttributeToParamValue (ParamValue& pvalue, const GS::UniString& paramName, const API_Attribute attr);

// -----------------------------------------------------------------------------
// Конвертация параметров библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ConvertToParamValue (ParamValue& pvalue, const API_AddParType& nthParameter);

// -----------------------------------------------------------------------------
// Заполнение rawName для ParamValue по описанию в API_Property
// -----------------------------------------------------------------------------
void SetrawNameFromProperty (ParamValue& pvalue, const API_Property& property);

// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ConvertToParamValue (ParamValue& pvalue, const API_Property& property);

void ConvertToParamValue_CheckAttrib (ParamValue& pvalue, const API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// Конвертация определения свойства в ParamValue
// -----------------------------------------------------------------------------
bool ConvertToParamValue (ParamValue& pvalue, const API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// Конвертация строки в ParamValue
// -----------------------------------------------------------------------------
bool ConvertStringToParamValue (ParamValue& pvalue, const GS::UniString& paramName, const GS::UniString strvalue);

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ConvertIntToParamValue (ParamValue& pvalue, const GS::UniString& paramName, const Int32 intValue);

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ConvertDoubleToParamValue (ParamValue& pvalue, const GS::UniString& paramName, const double doubleValue);

// -----------------------------------------------------------------------------
// Конвертация API_IFCProperty в ParamValue
// -----------------------------------------------------------------------------
#if !defined (AC_29)
bool ConvertToParamValue (ParamValue& pvalue, const API_IFCProperty& property);
#endif // !AC_29

void ConvertByFormatString (ParamValue& pvalue);

// --------------------------------------------------------------------
// Заполнение информации о локальном начале координат
// --------------------------------------------------------------------
void GetLocOriginToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Заполнение информации о проекте
// --------------------------------------------------------------------
void GetAllInfoToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение списка аттрибутов (имён слоёв, материалов)
// --------------------------------------------------------------------
void GetAllAttributeToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение списка глобальных переменных о местоположении проекта, солнца
// --------------------------------------------------------------------
void GetAllGlobToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение массива описаний свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool SubGuid_GetDefinition (const GS::Array<API_PropertyDefinition>& definitions, GS::Array<API_PropertyDefinition>& definitionsout);

// --------------------------------------------------------------------
// Получение словаря значений свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool SubGuid_GetParamValue (const API_Guid& elemGuid, ParamDictValue& propertyParams, const GS::Array<API_PropertyDefinition>& definitions, ParamDictValue& subproperty);

// --------------------------------------------------------------------
// Заполнение свойств для элемента
// --------------------------------------------------------------------
void AllPropertyDefinitionToParamDict (ParamDictValue& propertyParams, const API_Guid& elemGuid);

// --------------------------------------------------------------------
// Перевод GS::Array<API_PropertyDefinition> в ParamDictValue
// --------------------------------------------------------------------
void AllPropertyDefinitionToParamDict (ParamDictValue& propertyParams, GS::Array<API_PropertyDefinition>& definitions);

// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
void AllPropertyDefinitionToParamDict (ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictElement
// --------------------------------------------------------------------
void CompareParamDictElement (ParamDictElement& paramsFrom, ParamDictElement& paramsTo);

// -----------------------------------------------------------------------------
// Поиск по описанию GDL параметра
// Данный способ работат только с объектами (только чтение)
// -----------------------------------------------------------------------------
bool GDLParamByDescription (const API_Element& element, ParamDictValue& params, ParamDictValue& find_params, GS::HashTable<GS::UniString, GS::Array<GS::UniString>>& paramnamearray);

// -----------------------------------------------------------------------------
// Поиск по имени GDL параметра (чтение/запись)
// -----------------------------------------------------------------------------
bool GDLParamByName (const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params, GS::HashTable<GS::UniString, GS::Array<GS::UniString>>& paramnamearray);

// -----------------------------------------------------------------------------
// Обработка свойств с формулами
// -----------------------------------------------------------------------------
bool ReadFormula (ParamDictValue& paramByType, ParamDictValue& params);

bool ReadListData (const API_Elem_Head& elem_head, ParamDictValue& params);

void ReadQuantities (const API_Guid& elemGuid, ParamDictValue& params, ParamDictValue& propertyParams, GS::HashTable<API_AttributeIndex, bool>& existsmaterial, ParamDictValue& paramlayers);

bool ReadElementValues (const API_Element& element, ParamDictValue& params);

GS::UniString GetUnitsPrefix (GS::UniString& unit);

void SetUnitsAndQty2ParamValueComposite (ParamValueComposite& comp);

// -----------------------------------------------------------------------------
// Получение информации о материалах и составе конструкции
// -----------------------------------------------------------------------------
bool ReadMaterial (const API_Element& element, ParamDictValue& params, ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Получение данных из однородной конструкции
// --------------------------------------------------------------------
bool ComponentsBasicStructure (const API_AttributeIndex& constrinx, const double& fillThick, const API_AttributeIndex& constrinx_ven, const double& fillThick_ven, ParamDictValue& params, ParamDictValue& paramlayers, ParamDictValue& paramsAdd, short& structype_ven, double& width, double& length);

void ComponentsGetUnic (GS::Array<ParamValueComposite>& composite);

// --------------------------------------------------------------------
// Получение данных из многослойной конструкции
// --------------------------------------------------------------------
bool ComponentsCompositeStructure (const API_Guid& elemguid, API_AttributeIndex& constrinx, ParamDictValue& params, ParamDictValue& paramlayers, ParamDictValue& paramsAdd, GS::HashTable<API_AttributeIndex, bool>& existsmaterial, double& width, double& length);

// --------------------------------------------------------------------
// Получение данных из сложного профиля, для АС24 и выше
// --------------------------------------------------------------------
bool ComponentsProfileStructure (ProfileVectorImage& profileDescription, ParamDictValue& params, ParamDictValue& paramlayers, ParamDictValue& paramsAdd, GS::HashTable<API_AttributeIndex, bool>& existsmaterial, double& width, double& length);

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
bool Components (const API_Element& element, ParamDictValue& params, ParamDictValue& paramsAdd, GS::HashTable<API_AttributeIndex, bool>& existsmaterial);

// --------------------------------------------------------------------
// Заполнение данных для одного слоя
// --------------------------------------------------------------------
bool GetAttributeValues (const API_AttributeIndex& constrinx, ParamDictValue& params, ParamDictValue& paramsAdd);

// -----------------------------------------------------------------------------
// Перевод значения в строку в соответсвии с stringformat
// -----------------------------------------------------------------------------
GS::UniString ToString (const ParamValue& pvalue, const FormatString stringformat);

// -----------------------------------------------------------------------------
// Перевод значения в строку в соответсвии с stringformat
// -----------------------------------------------------------------------------
GS::UniString ToString (const ParamValue& pvalue);
}

ParamValueData operator+ (const ParamValueData& lhs, const ParamValueData& rhs);

bool operator==(const ParamValue& lhs, const ParamValue& rhs);

bool operator==(const API_Variant& lhs, const API_Variant& rhs);

bool operator==(const API_SingleVariant& lhs, const API_SingleVariant& rhs);

bool operator==(const API_ListVariant& lhs, const API_ListVariant& rhs);

bool operator==(const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs);

#if defined(AC_24)
bool operator==(const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs);
#endif

bool Equals (const API_PropertyDefaultValue& lhs, const API_PropertyDefaultValue& rhs, API_PropertyCollectionType collType);

bool Equals (const API_PropertyValue& lhs, const API_PropertyValue& rhs, API_PropertyCollectionType collType);

bool operator==(const API_PropertyGroup& lhs, const API_PropertyGroup& rhs);

bool operator==(const API_PropertyDefinition& lhs, const API_PropertyDefinition& rhs);

bool operator==(const API_Property& lhs, const API_Property& rhs);

template <typename T>
bool operator!=(const T& lhs, const T& rhs)
{
    return !(lhs == rhs);
}

//--------------------------------------------------------------------------------------------------------------------------
// Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState (const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition>& definitions, GS::UniString property_flag_name, bool& flagfind, bool check);

bool GetElemStateReverse (const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition>& definitions, GS::UniString property_flag_name, bool& flagfind);


#endif
