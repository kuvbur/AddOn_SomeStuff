//------------ kuvbur 2022 ------------
#pragma once
#if !defined(REVISION_HPP)
#define REVISION_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_27
#include "DG.h"
#include "Helpers.hpp"

static const Int32 TypeNone = 0;
static const Int32 TypeIzm = 1;
static const Int32 TypeZam = 2;
static const Int32 TypeNov = 3;
static const Int32 TypeAnnul = 4;

typedef struct
{
    API_Coord startpoint = { 0,0 };
    API_Guid markerguid = APINULLGuid;
    GS::UniString changeId = "";
    GS::UniString changeName = "";
    GS::UniString note = ""; // Текст изменения
    GS::UniString nizm = ""; // Номер изменения
    GS::UniString nuch = ""; // Номер участка
    GS::UniString fam = "";  // Фамилия
    GS::Int32 code = 0; // Код изменения
    GS::Int32 typeizm = TypeNone;
} Change; // Хранение одного изменения (облака)

typedef struct
{
    GS::Array<Change> arr;
    GS::Int32 nuch = 0; // Количество участков
    GS::UniString fam = "";  // Фамилия
    GS::Int32 typeizm = TypeNone;
    GS::UniString changeId = "";
    GS::UniString nizm = ""; // Номер изменения
    GS::UniString note = ""; // Описание изменений
    GS::Int32 code = 0; // Код изменения
} Changes; // Массив изменений на листе

typedef GS::HashTable < GS::UniString, Changes> ChangeMarkerDict;

typedef struct
{
    GS::HashTable < GS::UniString, GS::Int32> layoutId; // Список листов. Ключ - ID макета, значение - тип изменения
    GS::Int32 code = 0; // Код изменения
    GS::UniString nizm = ""; // Номер изменения
} Notes;

typedef GS::HashTable< GS::UniString, Notes> NoteDict; //Словарь с описаниями. Ключ - описание изменения (note)
typedef GS::HashTable< GS::UniString, NoteDict> NoteByChangeDict; //Словарь с описаниями по изменениям. Ключ - ID изменения
typedef GS::HashTable< GS::UniString, API_DatabaseUnId> LayoutRevisionDict; //Словарь листов с РВИ. Ключ - ID изменения

namespace Revision
{
void SetRevision (void);

bool GetScheme (GS::HashTable<GS::UniString, API_Guid>& layout_note_guid);

void GetAllChangesMarker (GS::HashTable<GS::UniString, API_Guid>& layout_note_guid);

bool ChangeLayoutProperty (ChangeMarkerDict& changes, GS::HashTable<GS::UniString, API_Guid>& layout_note_guid, API_DatabaseUnId& databaseUnId, GS::UniString& layoutId, LayoutRevisionDict& layoutRVI, NoteByChangeDict& allchanges);

void CheckChanges (ChangeMarkerDict& changes, GS::UniString& subsetName, GS::UniString& layoutid);

void GetChangesLayout (GS::Array<API_RVMChange>& layoutchange, ChangeMarkerDict& changes, GS::HashTable<GS::UniString, API_Guid>& layout_note_guid);

bool GetChangesMarker (ChangeMarkerDict& changes);

bool GetMarkerPos (API_Guid& markerguid, API_Coord& startpoint);

bool GetMarkerText (API_Guid& markerguid, GS::UniString& note, GS::UniString& nuch, GS::UniString& nizm, GS::Int32& typeizm, GS::UniString& fam, GS::Int32& code);

void ChangeMarkerTextOnLayout (ChangeMarkerDict& changes);

void ChangeMarkerText (API_Guid& markerguid, GS::UniString& nuch, GS::UniString& nizm);
}

#endif
