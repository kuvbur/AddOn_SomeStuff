//------------ kuvbur 2022 ------------
#pragma once
#ifndef DIM_HPP
    #define DIM_HPP
    #include "DG.h"
    #include "dialogs/SyncSettings.hpp"
    #include "Helpers.hpp"

// Модуль округления размеров: правила форматирования, обработка текста размеров и привязка к типам элементов.
// Обрабатывает один элемент и применяет к его размерам правила округления.
GSErrCode DimAutoRoundOne (const API_Guid &elemGuid, const SyncSettings &syncSettings, bool checktype);

// -----------------------------------------------------------------------------
// Обработка одного размера
// -----------------------------------------------------------------------------
// Обрабатывает один размер и решает, нужно ли менять его текст, цвет или сбросить формат.
GSErrCode DimAutoRound (const API_Guid &elemGuid, const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Обрабатывает размер и решает - что с ним делать
//	flag_change - менять текст размера, сбросить или не менять (DIM_CHANGE_ON, DIM_CHANGE_OFF, DIM_NOCHANGE)
//	flag_highlight - изменять перо текста, сбросить на оригинальное или не менять (DIM_HIGHLIGHT_ON, DIM_HIGHLIGHT_OFF,
// DIM_NOCHANGE)
// -----------------------------------------------------------------------------
// Разбирает значение размера и формирует текст с учётом правил округления и форматирования.
// FIX (ревью 2026-09-12): п.32 — полная копия dimrule.paramDict (HashTable<UniString, ParamValue>)
// на каждый размер × каждое правило больше не создаётся ради одного значения measuredvalue;
// п.62 — входной content больше не мутируется, вычисленный текст возвращается через
// out-параметр custom_txt — следующее правило сравнивает с исходным текстом размера.
bool DimParse (const double &dimVal,
               const API_Guid &elemGuid,
               const API_NoteContentType &contentType,
               const GS::UniString &content,
               GS::UniString &custom_txt,
               UInt32 &flag_change,
               UInt32 &flag_highlight,
               const DimRule &dimrule);

// -----------------------------------------------------------------------------
// Округление всего доступного согласно настроек
// -----------------------------------------------------------------------------
// Применяет округление размеров ко всем доступным элементам согласно текущим настройкам.
void DimRoundAll (const SyncSettings &syncSettings, bool isUndo);

// -----------------------------------------------------------------------------
// Округление одного типа размеров
// -----------------------------------------------------------------------------
// Округляет размеры только для одного типа элементов.
bool DimRoundByType (const API_ElemTypeID &typeID, const SyncSettings &syncSettings);

#endif
