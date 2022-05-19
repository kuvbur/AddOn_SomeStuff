#include	<regex>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Dimensions.hpp"

#define DIM_NOCHANGE 0
#define DIM_HIGHLIGHT_ON 1
#define DIM_HIGHLIGHT_OFF 2
#define DIM_CHANGE_ON 1
#define DIM_CHANGE_FORCE 2
#define DIM_CHANGE_OFF 3
// TODO Попробовать добавить все подходящие слои в правила на этапе чтения настроек
// -----------------------------------------------------------------------------
// Чтение настроек из инфлрмации о проекте
//	Имя свойства: "Addon_Dimenstions"
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					Слой - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА"
// -----------------------------------------------------------------------------
GSErrCode DimReadPref(DimRules& dimrules) {
	GS::Array<GS::ArrayFB<GS::UniString, 3> >	autotexts;
	API_AutotextType	type = APIAutoText_Custom;
	GSErrCode	err = ACAPI_Goodies(APIAny_GetAutoTextsID, &autotexts, (void*)(GS::IntPtr)type);
	if (err != NoError) {
		msg_rep("DimReadPref", "ACAPI_Goodies", err, APINULLGuid);
		return err;
	}
	for (UInt32 i = 0; i < autotexts.GetSize(); i++) {
		if (autotexts[i][0].Contains("Addon_Dimenstions")) {
			if (autotexts[i][2].Contains(";")){
				GS::Array<GS::UniString> partstring;
				StringSplt(autotexts[i][2], ";", partstring);
				for (UInt32 k = 0; k < partstring.GetSize(); k++) {
					DimRule dimrule;
					if (DimParsePref(partstring[k], dimrule)) {
						GS::UniString kstr;
						if (dimrule.layer.IsEmpty()) {
							kstr = GS::UniString::Printf("%d", dimrule.pen_original);
						}
						else {
							kstr = dimrule.layer;
						}
						dimrules.Add(kstr, dimrule);
					}
				}
			}
			else {
				DimRule dimrule;
				if (DimParsePref(autotexts[i][2], dimrule)) {
					GS::UniString kstr;
					if (dimrule.layer.IsEmpty()) {
						kstr = GS::UniString::Printf("%d", dimrule.pen_original);
					}
					else {
						kstr = dimrule.layer;
					}
					dimrules.Add(kstr, dimrule);
				}
			}
			return err;
		}
	}
	return err;
}

// -----------------------------------------------------------------------------
// Обработка одного правила
// -----------------------------------------------------------------------------
bool DimParsePref(GS::UniString rawrule, DimRule& dimrule) {
	GS::Array<GS::UniString> partstring_1;
	if (StringSplt(rawrule, "-", partstring_1) == 2) {
		//Проверяем - что указано в правиле: слой или номер пера
		// Слой указываем в кавычках, в regexp формате
		if (partstring_1[0].Contains('"')) {
			GS::UniString layer = partstring_1[0];
			layer.ReplaceAll('"', ' ');
			layer.Trim();
			dimrule.layer = layer;
		}
		else {
			dimrule.pen_original = std::atoi(partstring_1[0].ToCStr());
		}
		GS::Array<GS::UniString> partstring_2;
		if (StringSplt(partstring_1[1], ",", partstring_2) > 1) {
			dimrule.round_value = std::atoi(partstring_2[0].ToCStr());
			dimrule.pen_rounded = std::atoi(partstring_2[1].ToCStr());
			if (partstring_2.GetSize() == 3) {
				if (partstring_2[2].Contains("<") && partstring_2[2].Contains(">")) {
					ParamDict paramDict;
					GetParamNameDict(partstring_2[2], paramDict);
					dimrule.paramDict = paramDict;
					dimrule.expression = partstring_2[2];
				}
				else {
					dimrule.flag_change = (std::atoi(partstring_2[2].ToCStr()) > 0);
				}
			}
			if (partstring_2.GetSize() == 4) {
				if (partstring_2[3].Contains("<") && partstring_2[3].Contains(">")) {
					ParamDict paramDict;
					GetParamNameDict(partstring_2[3], paramDict);
					dimrule.paramDict = paramDict;
					dimrule.expression = partstring_2[3];
				}
			}
			return true;
		}
	}
	return false;
}

// -----------------------------------------------------------------------------
// Прослойка для чтения настроек
// -----------------------------------------------------------------------------
void DimAutoRoundSel(const API_Guid& elemGuid, const SyncSettings& syncSettings) {
	DimRules dimrules;
	GSErrCode err = DimReadPref(dimrules);
	if (dimrules.GetSize() == 0 || err != NoError) return;
	err = DimAutoRound(elemGuid, dimrules);
	return;
}

void DimSelected(const SyncSettings& syncSettings) {
	CallOnSelectedElemSettings(DimAutoRoundSel, false, true, syncSettings);
}

// -----------------------------------------------------------------------------
// Обработка одного размера
// -----------------------------------------------------------------------------
GSErrCode DimAutoRound(const API_Guid& elemGuid, DimRules& dimrules) {
	GSErrCode	err = NoError;
	API_Element			element;
	API_Element			mask;
	API_ElementMemo		memo;
	BNZeroMemory(&element, sizeof(API_Element));
	BNZeroMemory(&memo, sizeof(API_ElementMemo));
	BNZeroMemory(&mask, sizeof(API_Element));
	element.header.guid = elemGuid;
	err = ACAPI_Element_Get(&element);
	if (err != NoError) {
		msg_rep("DimAutoRound", "ACAPI_Element_Get", err, elemGuid);
		return err;
	}
	bool flag_round = false;
	UInt32 round_value = 0;
	short pen_dimenstion = 0;
	short pen_original = 0;
	short pen_rounded = 0;
	bool flag_change_rule = false;
	ParamDict paramDict;
	GS::UniString expression = "";
	ACAPI_ELEMENT_MASK_CLEAR(mask);
	ACAPI_ELEMENT_MASK_SETFULL(mask);
	switch (element.header.typeID) {
	case API_DimensionID:
		pen_original = element.dimension.defNote.notePen;
		pen_dimenstion = element.dimension.linPen;
		//element.dimension.defNote.contentType == API_NoteContent_Custom ? element.dimension.defNote.contentType = API_NoteContent_Measured
		//	: element.dimension.defNote.contentType = API_NoteContent_Custom;
		//strcpy((char*)&element.dimension.defNote.content, "Custom");
		//ACAPI_ELEMENT_MASK_SET(mask, API_DimensionType, defNote.contentType);
		//ACAPI_ELEMENT_MASK_SET(mask, API_DimensionType, defNote.content);
		break;
	case API_RadialDimensionID:
		pen_original = element.radialDimension.note.notePen;
		pen_dimenstion = element.radialDimension.linPen;
		//element.radialDimension.note.contentType == API_NoteContent_Custom ? element.radialDimension.note.contentType = API_NoteContent_Measured
		//	: element.radialDimension.note.contentType = API_NoteContent_Custom;
		//strcpy((char*)&element.radialDimension.note.content, "Custom");
		//ACAPI_ELEMENT_MASK_SET(mask, API_RadialDimensionType, note.contentType);
		//ACAPI_ELEMENT_MASK_SET(mask, API_RadialDimensionType, note.content);
		break;
	case API_LevelDimensionID:
		pen_original = element.levelDimension.note1.notePen;
		pen_dimenstion = element.levelDimension.pen;
		//element.levelDimension.note1.contentType == API_NoteContent_Custom ? element.levelDimension.note1.contentType = API_NoteContent_Measured
		//	: element.levelDimension.note1.contentType = API_NoteContent_Custom;
		//strcpy((char*)&element.levelDimension.note1.content, "Custom");
		//ACAPI_ELEMENT_MASK_SET(mask, API_LevelDimensionType, note1.contentType);
		//ACAPI_ELEMENT_MASK_SET(mask, API_LevelDimensionType, note1.content);
		break;
	default:
		break;
	}
	bool find_rule = false;
	GS::UniString kstr = GS::UniString::Printf("%d", pen_dimenstion);
	if (pen_dimenstion > 0 && dimrules.ContainsKey(kstr)) {
		pen_rounded = dimrules[kstr].pen_rounded;
		flag_change_rule = dimrules[kstr].flag_change;
		find_rule = true;
	}
	if (!find_rule) {
		API_Attribute layer;
		BNZeroMemory(&layer, sizeof(API_Attribute));
		layer.header.typeID = API_LayerID;
		layer.header.index = element.header.layer;
		if (ACAPI_Attribute_Get(&layer) != NoError) return err;
		GS::UniString layert = GS::UniString::Printf("%s", layer.header.name);
		for (GS::HashTable<GS::UniString, DimRule>::ConstPairIterator cIt = dimrules.EnumeratePairs(); cIt != NULL; ++cIt) {
			const GS::UniString& regexpstring = *cIt->key;
			if (layert.Contains(regexpstring)) {
				kstr = regexpstring;
				find_rule = true;
			}
		}
		// TODO добавить regex
		//kstr = GS::UniString::Printf("%d", pen_dimenstion).ToCStr().Get();
		//for (GS::HashTable<GS::UniString, DimRule>::ConstPairIterator cIt = dimrules.EnumeratePairs(); cIt != NULL; ++cIt) {
		//	const GS::UniString& regexpstring = *cIt->key;
		//	static const std::regex r(regexpstring.ToCStr().Get());
		//	if (std::regex_match(kstr.ToCStr().Get(), r)) {

		//	}
		//}
	}
	if (!find_rule) return err;
	short pen = pen_rounded;
	bool flag_write = true;
	if (element.header.hasMemo) {
		err = ACAPI_Element_GetMemo(element.header.guid, &memo);
		if (err != NoError) {
			msg_rep("DimAutoRound", "ACAPI_Element_GetMemo", err, elemGuid);
			return err;
		}
		for (int k = 1; k < element.dimension.nDimElem; k++) {
			UInt32 flag_change = DIM_NOCHANGE;
			UInt32 flag_highlight = DIM_NOCHANGE;
			GS::UniString content = GS::UniString::Printf("%s", (*memo.dimElems)[k].note.content);
			API_Guid ref_elemGuid = (*memo.dimElems)[k].base.base.guid;
			API_NoteContentType contentType = (*memo.dimElems)[k].note.contentType;
			if (DimParse((*memo.dimElems)[k].dimVal, ref_elemGuid, contentType, content, flag_change, flag_highlight, dimrules[kstr])) {
				if (!flag_change_rule && flag_change != DIM_CHANGE_FORCE) flag_change = DIM_CHANGE_OFF;
				if (flag_change == DIM_CHANGE_ON || flag_change == DIM_CHANGE_FORCE) {
					flag_write = true;
					(*memo.dimElems)[k].note.contentType = API_NoteContent_Custom;
					strcpy((char*)&((*memo.dimElems)[k].note.content), content.ToCStr());
					*(*memo.dimElems)[k].note.contentUStr = content;
				}
				if (flag_change == DIM_CHANGE_OFF && (*memo.dimElems)[k].note.contentType != API_NoteContent_Measured) {
					flag_write = true;
					(*memo.dimElems)[k].note.contentType = API_NoteContent_Measured;
					//strcpy((char*)&((*memo.dimElems)[k].note.content), "");
					//*(*memo.dimElems)[k].note.contentUStr = "";
				}
				if (flag_highlight == DIM_HIGHLIGHT_ON) pen = pen_rounded;
				if (flag_highlight == DIM_HIGHLIGHT_OFF) pen = pen_original;
				if (flag_highlight != DIM_NOCHANGE && (*memo.dimElems)[k].note.notePen != pen) {
					flag_write = true;
					(*memo.dimElems)[k].note.notePen = pen;
				}
			}
		}
		if (flag_write) {
			err = ACAPI_CallUndoableCommand("Create text", [&]() -> GSErrCode {
				return ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_All, true);
				});
			if (err == APIERR_REFUSEDCMD ) { // Я сказал надо!
				if (!ACAPI_Element_Filter(elemGuid, APIFilt_InMyWorkspace)) return err;
				if (!ACAPI_Element_Filter(elemGuid, APIFilt_HasAccessRight)) return err;
				if (!ACAPI_Element_Filter(elemGuid, APIFilt_IsEditable)) return err;
				try
				{
					err = ACAPI_Element_Change(&element, &mask, &memo, APIMemoMask_All, true);
				}
				catch (const std::exception&)
				{
					return err;
				}
			}
		}
		if (err != NoError) {
			msg_rep("DimAutoRound", "ACAPI_Element_Change_1", err, elemGuid);
			return err;
		}
		ACAPI_DisposeElemMemoHdls(&memo);
	}
	else {
		err = ACAPI_CallUndoableCommand("Create text", [&]() -> GSErrCode {
			return ACAPI_Element_Change(&element, &mask, NULL, 0, true);
			});
		if (err == APIERR_REFUSEDCMD) {
			err = ACAPI_Element_Change(&element, &mask, NULL, 0, true);
		}
		if (err != NoError) {
			msg_rep("DimAutoRound", "ACAPI_Element_Change_2", err, elemGuid);
			return err;
		}
	}
	return err;
}

// -----------------------------------------------------------------------------
// Обрабатывает размер и решает - что с ним делать
//	flag_change - менять текст размера, сбросить или не менять (DIM_CHANGE_ON, DIM_CHANGE_OFF, DIM_NOCHANGE)
//	flag_highlight - изменять перо текста, сбросить на оригинальное или не менять (DIM_HIGHLIGHT_ON, DIM_HIGHLIGHT_OFF, DIM_NOCHANGE)
// -----------------------------------------------------------------------------
bool DimParse(const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, DimRule& dimrule) {
	flag_change = DIM_NOCHANGE;
	flag_highlight = DIM_NOCHANGE;
	double round_value = dimrule.round_value / 1.0;
	if (round_value < 1) round_value = 1.0;
	double dimVal_r = round(dimVal * 1000.0);
	Int32 dimValmm_round = DoubleM2IntMM(round(round(dimVal_r / round_value) * round_value));
	double dx = abs(dimValmm_round * 1.0 - dimVal_r); // Разница в размерах в мм
	GS::UniString custom_txt = GS::UniString::Printf("%d", dimValmm_round);
	bool flag_expression = false; //В описании найдена формула
	if (!dimrule.expression.IsEmpty() ) {
		ParamDictValue pdictvalue;
		if (elemGuid != APINULLGuid) GetParamValueDict(elemGuid, dimrule.paramDict, pdictvalue); //Получим значения, если размер привязан к элементу
		// Добавляем в словарь округлённое значение
		ParamValue pvalue;
		ConvParamValue(pvalue, "MeasuredValue", dimValmm_round);
		pdictvalue.Add(pvalue.name, pvalue);
		GS::UniString expression = dimrule.expression;
		// Заменяем вычисленное
		if (ReplaceParamInExpression(pdictvalue, expression)) {
			// Вычисляем значения
			if (EvalExpression(expression)) {
				custom_txt = expression;
				flag_expression = true;
			}
		}
	}
	//Если указано округление до нуля - просто подсветим кривые размеры
	if (round_value < 1) {
		if (contentType == API_NoteContent_Custom) flag_change = DIM_CHANGE_OFF;
		if (contentType == API_NoteContent_Measured) flag_change = DIM_NOCHANGE;
		if (dx > 0.099) {
			flag_highlight = DIM_HIGHLIGHT_ON;
		}
		else {
			flag_highlight = DIM_HIGHLIGHT_OFF;
		}
	}
	else {
		// Если стоит пользовательский текст - сверим с вычисленным значением
		if (contentType == API_NoteContent_Custom) {
			if (flag_change == DIM_NOCHANGE && flag_expression == false && dx < 1.0) {
				flag_change = DIM_CHANGE_OFF;
				flag_highlight = DIM_HIGHLIGHT_OFF;
			}
			if (flag_change == DIM_NOCHANGE && custom_txt != content) {
				flag_highlight = DIM_HIGHLIGHT_ON;
				flag_change = DIM_CHANGE_ON;
			}
		}
		// Если стоит автотекст и формул нет - снимем выделение
		if (contentType == API_NoteContent_Measured) {
			if (flag_change == DIM_NOCHANGE && (flag_expression == true || dx >= 1.0)) {
				flag_change = DIM_CHANGE_ON;
				flag_highlight = DIM_HIGHLIGHT_ON;
			}
			else {
				flag_highlight = DIM_HIGHLIGHT_OFF;
			}
		}
	}
	if (flag_expression && flag_change == DIM_CHANGE_ON) flag_change = DIM_CHANGE_FORCE;
	if (flag_change == DIM_CHANGE_ON || flag_change == DIM_CHANGE_FORCE) content = custom_txt;
	return (flag_change != DIM_NOCHANGE || flag_highlight != DIM_NOCHANGE);
}

// -----------------------------------------------------------------------------
// Округление всего доступного согласно настроек
// TODO добавить резервирование
// -----------------------------------------------------------------------------
void DimRoundAll(const SyncSettings& syncSettings) {
	//if (!syncSettings.logMon) return;
	DoneElemGuid doneelemguid;
	DimRules dimrules;
	GSErrCode err = DimReadPref(dimrules);
	if (dimrules.GetSize() == 0 || err != NoError) return;
	bool flag_chanel = false;
	if (!flag_chanel) flag_chanel = DimRoundByType(API_DimensionID, doneelemguid, dimrules);
	if (!flag_chanel) flag_chanel = DimRoundByType(API_RadialDimensionID, doneelemguid, dimrules);
	if (!flag_chanel) flag_chanel = DimRoundByType(API_LevelDimensionID, doneelemguid, dimrules);
}

// -----------------------------------------------------------------------------
// Округление одного типа размеров
// TODO добавить резервирование
// -----------------------------------------------------------------------------
bool DimRoundByType(const API_ElemTypeID typeID, DoneElemGuid& doneelemguid, DimRules& dimrules) {
	GSErrCode	err = NoError;
	//GS::Array<API_Guid>	guidArray_all;
	//err = ACAPI_Element_GetElemList(typeID, &guidArray_all, APIFilt_OnVisLayer | APIFilt_IsInStructureDisplay);
	//if (guidArray_all.GetSize() == 0 || err != NoError) return false;
	//if (ACAPI_TeamworkControl_HasConnection()) {
	//	GS::Array<API_Guid>	elements;
	//	GS::HashTable<API_Guid, short>  conflicts;
	//	for (UInt32 i = 0; i < guidArray_all.GetSize(); i++) {

	//		elements.Push(guidArray_all.Get(i));
	//	}
	//	ACAPI_TeamworkControl_ReserveElements(elements, &conflicts);
	//}
	GS::Array<API_Guid>	guidArray;
	err = ACAPI_Element_GetElemList(typeID, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_OnVisLayer | APIFilt_IsInStructureDisplay);
	if (guidArray.GetSize() == 0 || err != NoError) return false;
	if (err == NoError) {
		for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
			if (!doneelemguid.ContainsKey(guidArray.Get(i))) {
				err = DimAutoRound(guidArray.Get(i), dimrules);
				if (err == NoError) doneelemguid.Add(guidArray.Get(i), false);
			}
			if (ACAPI_Interface(APIIo_IsProcessCanceledID, nullptr, nullptr)) return true;
		}
	}
	else {
		msg_rep("DimAutoRound", "ACAPI_Element_GetElemList", err, APINULLGuid);
	}
	return false;
}
