//------------ kuvbur 2022 ------------
#pragma once
#if !defined(REVISION_HPP)
    #define REVISION_HPP
    #include "DG.h"
    #include "Helpers.hpp"

// Подсистема работы с маркерами изменений, заметками и ревизионными метками на листах.
static const Int32 TypeNone = 0;
static const Int32 TypeIzm = 1;
static const Int32 TypeZam = 2;
static const Int32 TypeNov = 3;
static const Int32 TypeAnnul = 4;

struct Change {
    API_Coord startpoint = {0, 0};
    API_Guid markerguid = APINULLGuid;
    GS::UniString changeId = "";
    GS::UniString changeName = "";
    GS::UniString note = ""; // Текст изменения
    GS::UniString nizm = ""; // Номер изменения
    GS::UniString nuch = ""; // Номер участка
    GS::UniString fam = "";  // Фамилия
    GS::Int32 code = 0;      // Код изменения
    GS::Int32 typeizm = TypeNone;
}; // Хранение одного изменения (облака)

struct Changes {
    GS::Array<Change> arr;
    GS::Int32 nuch = 0;     // Количество участков
    GS::UniString fam = ""; // Фамилия
    GS::Int32 typeizm = TypeNone;
    GS::UniString changeId = "";
    GS::UniString nizm = ""; // Номер изменения
    GS::UniString note = ""; // Описание изменений
    GS::Int32 code = 0;      // Код изменения
}; // Массив изменений на листе

typedef GS::HashTable<GS::UniString, Changes> ChangeMarkerDict;

struct Notes {
    GS::HashTable<GS::UniString, GS::Int32> layoutId; // Список листов. Ключ - ID макета, значение - тип изменения
    GS::Int32 code = 0;                               // Код изменения
    GS::UniString nizm = "";                          // Номер изменения
};

typedef GS::HashTable<GS::UniString, Notes> NoteDict; // Словарь с описаниями. Ключ - описание изменения (note)
typedef GS::HashTable<GS::UniString, NoteDict>
    NoteByChangeDict; // Словарь с описаниями по изменениям. Ключ - ID изменения
typedef GS::HashTable<GS::UniString, API_DatabaseUnId> LayoutRevisionDict; // Словарь листов с РВИ. Ключ - ID изменения

namespace Revision {
    // Создаёт или обновляет ревизионные маркеры и связанные с ними свойства на листах.
    void SetRevision (void);

    // Получает схему ревизионных маркеров по листам проекта.
    bool GetScheme (GS::HashTable<GS::UniString, API_Guid> &layout_note_guid);

    // Собирает все маркеры изменений, связанные с листами проекта.
    void GetAllChangesMarker (GS::HashTable<GS::UniString, API_Guid> &layout_note_guid);

    // Применяет изменения к свойствам листа и связанной с ним ревизионной информации.
    bool ChangeLayoutProperty (ChangeMarkerDict &changes,
                               GS::HashTable<GS::UniString, API_Guid> &layout_note_guid,
                               API_DatabaseUnId &databaseUnId,
                               GS::UniString &layoutId,
                               LayoutRevisionDict &layoutRVI,
                               NoteByChangeDict &allchanges);

    // Проверяет, есть ли нужные изменения в наборе маркеров для листа.
    bool CheckChanges (ChangeMarkerDict &changes, GS::UniString &subsetName, GS::UniString &layoutid);

    // Формирует список изменений для конкретного листа из собранных маркеров.
    void GetChangesLayout (GS::Array<API_RVMChange> &layoutchange,
                           ChangeMarkerDict &changes,
                           GS::HashTable<GS::UniString, API_Guid> &layout_note_guid);

    // Считывает маркеры изменений из модели и заполняет словарь.
    bool GetChangesMarker (ChangeMarkerDict &changes);

    // Возвращает позицию маркера изменения в модели.
    bool GetMarkerPos (API_Guid &markerguid, API_Coord &startpoint);

    // Считывает текстовые поля маркера: примечание, номер участка, номер изменения и др.
    bool GetMarkerText (API_Guid &markerguid,
                        GS::UniString &note,
                        GS::UniString &nuch,
                        GS::UniString &nizm,
                        GS::Int32 &typeizm,
                        GS::UniString &fam,
                        GS::Int32 &code);

    // Обновляет текст маркеров на листах по собранным данным изменений.
    void ChangeMarkerTextOnLayout (ChangeMarkerDict &changes);

    // Меняет текст маркера изменения для выбранного GUID.
    void ChangeMarkerText (API_Guid &markerguid, GS::UniString &nuch, GS::UniString &nizm);
} // namespace Revision

#endif
