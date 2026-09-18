//------------ kuvbur 2026 ------------
#pragma once
#ifndef TABLES_NAVIGATOR_HPP
    #define TABLES_NAVIGATOR_HPP

    #include "ACAPinc.h"

// Каркас пользовательских ведомостей в Navigator/MyDraw.
//
// Создан как отдельный модуль, чтобы будущая реализация по
// Docs/Tables_Navigator_AC25.md не смешалась с существующими Spec/Summ/Monitor.
// Первый шаг только фиксирует границы подсистемы: определение ведомости,
// рассчитанный снимок таблицы, renderer и точки регистрации в Archicad.
namespace TablesNavigator {

    struct ColumnDefinition {
        GS::UniString id;
        GS::UniString title;

        // id хранится отдельно от title: title можно переименовывать в UI, а id нужен
        // как устойчивый ключ колонки для ручных значений, ширин и будущих миграций.
    };

    struct RowIdentity {
        GS::UniString stableKey;
        GS::Array<API_Guid> sourceElements;

        // Одна строка будущей ведомости может соответствовать элементу, группе или
        // агрегату. Поэтому ключ строки не равен позиции row и не сводится к одному GUID.
    };

    struct ScheduleDefinition {
        API_Guid navigatorGuid = APINULLGuid;
        GS::UniString internalId;
        GS::UniString displayName;
        GS::Array<ColumnDefinition> columns;

        // Это описание намерения пользователя, а не рассчитанная таблица.
        // Хранение payload viewpoint/Add-On Object ещё не выбрано: документ AC25
        // допускает payload viewpoint, но Teamwork/merge/runtime нужно проверить отдельно.
    };

    struct TableCell {
        GS::UniString columnId;
        GS::UniString displayText;

        // На первом слое храним только отображаемый текст. Типизированные значения,
        // формулы, итоги и единицы измерения добавляются позже, когда будет согласована
        // первая реальная ведомость и источник данных.
    };

    struct TableRow {
        RowIdentity identity;
        GS::Array<TableCell> cells;
    };

    struct TableSnapshot {
        ScheduleDefinition definition;
        GS::Array<TableRow> rows;

        // Снимок отделён от definition, чтобы renderer мог одинаково работать для
        // открытого MyDraw-окна и для CreateIDFStore, не перечитывая модель в середине
        // IDF-сессии.
    };

    GSErrCode RegisterInterface ();
    GSErrCode Initialize ();
    GSErrCode EnsureNavigatorRoot ();
    bool IsNavigatorRegistrationEnabled ();

} // namespace TablesNavigator

#endif
