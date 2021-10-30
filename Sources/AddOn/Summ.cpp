#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Summ.hpp"
#include	"Helpers.hpp"
//#include	"sort.h"

// -----------------------------------------------------------------------------------------------------------------------
// Объединение уникальных значений свойств выделенных элементов
// Во многом похоже на модуль ReNum
// Флаг суммирования делать не буду - ибо смысла не вижу.
// Будем суммировать все выделенные элементы, у которых задано правило
// 1. Получаем список объектов, в свойствах которых ищем
//					Sum{*имя свойства-значения*; *имя свойства-критерия*}
// 2. Собираем правила суммирования
// 3. Для каждого правила отбираем элементы
// 4. Средни них ищем уни кальные значения
// 5. Записываем их через разделитель
// TODO Попробовать сделать диапазоны 1...10 и т.д.
// -----------------------------------------------------------------------------------------------------------------------

GSErrCode SumSelected(void) {
	GSErrCode err = NoError;
	SumRules rules;
	GS::Array<API_Guid> guidArray = GetSelectedElements(true, true);
	GS::UniString undoString = RSGetIndString(AddOnStringsID, UndoReNumId, ACAPI_GetOwnResModule());
	ACAPI_CallUndoableCommand(undoString, [&]() -> GSErrCode {
		if (!guidArray.IsEmpty()) {
			// Получаем список правил суммирования
			for (UInt32 i = 0; i < guidArray.GetSize(); i++) {
				err = Sum_GetElement(guidArray[i], rules);
			}
			//if (!rules.IsEmpty()) {
			//	// Теперь у нас есть списк правил. Можем пройти по каждому правилу и обработать элементы
			//	for (GS::HashTable<API_Guid, SumRule>::PairIterator cIt = rules.EnumeratePairs(); cIt != NULL; ++cIt) {
			//		const SumRule& rule = *cIt->value;
			//		if (!rule.elemts.IsEmpty()) {
			//			err = SumOneRule(rule);
			//		}
			//	}
			//}
		}
		GS::UniString intString = GS::UniString::Printf("Qty elements - %d", guidArray.GetSize());
		msg_rep("SumSelected", intString, err, APINULLGuid);
		rules.Clear();
		return err;
		});
	return err;
}

// -----------------------------------------------------------------------------------------------------------------------
// Функция распределяет элемент в таблицу с правилами нумерации
// -----------------------------------------------------------------------------------------------------------------------
GSErrCode Sum_GetElement(const API_Guid& elemGuid, SumRules& rules) {
	GSErrCode							err = NoError;
	GS::Array<API_PropertyDefinition>	definitions;
	SumElement el = {};
	err = ACAPI_Element_GetPropertyDefinitions(elemGuid, API_PropertyDefinitionFilter_UserDefined, definitions);
	if (err != NoError) {
		return err;
	}
	for (UInt32 j = 0; j < definitions.GetSize(); j++) {
		// Является ли свойство описанием системы нумерации?
		if (definitions[j].description.Contains("Sum")) {
		} // definitions[j].description.Contains("Sumnum")
	}//for
	return err;
}// ReNum_GetElement

