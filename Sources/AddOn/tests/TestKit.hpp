//------------ kuvbur 2026 ------------
// TestKit — вывод и учёт результатов тестов AddOn_SomeStuff (#231).
//
// Прод-DBtest/DBprnt в тестах НЕ используются: DBprnt печатает "== ERROR =="
// префикс, если текст случайно содержит "err"/"ERROR" (CommonFunction.cpp:310),
// склеивает аргументы через " : " без структуры и пишет только в панель
// «Отладка» — мимо файла отчёта. Продовые вызовы (Helpers.cpp,
// CommonFunction.cpp) макросом НЕ перекрываются: TestKit.hpp включается только
// в тестовых файлах.
//
// Единственный канал — файл отчёта: успех молчит (printPass), печатаются
// отклонения, измерения (TestKit::Note) и сводка. Файл переживает падение
// ArchiCAD, потому что каждая строка сбрасывается на диск сразу.
#pragma once

#ifdef TESTING

    #include <initializer_list>
    #include <string>
    #include <type_traits>

    #include "CommonFunction.hpp" // GS::UniString, is_equal

namespace TestKit {

    // Настройки вывода. Меняются до вызова Run.
    struct Config {
        bool printPass = false;      // печатать успешные проверки (в файл)
        int maxFailuresPerTest = 20; // дальше только счётчик «ещё N подавлено»
        std::string reportPath;      // UTF-8; пусто -> каталог TEMP
        // Уровень записей Note как целое (0/1/2 = NoteLevel) — на момент
        // объявления Config тип NoteLevel ещё не определён, поэтому поле
        // хранит номер уровня, а NoteLevel печатается по нему.
        int noteLevel = 1;
        // noteLevel задан вызывающим явно: переменная окружения SMSTF_VERBOSE
        // его не перекрывает. Ставится из SetNoteLevel.
        bool noteLevelExplicit = false;
    };

    Config &GetConfig ();

    struct Loc {
        const char *file;
        int line;
        bool required; // true -> падение прерывает набор
    };

    struct AbortTest {}; // бросается DBrequire/DBskip, ловится раннером

    typedef void (*TestFn) ();

    // РЕГИСТРАЦИЯ: реестр наполняется явно из одного места (см. RegisterAll в
    // TestFunc.cpp), а не статическими инициализаторами. Причина: при разбиении
    // файла по модулям статический регистратор вместе со своей static-функцией
    // может быть выкинут линковкой, и набор молча исчезнет из прогона.
    // Tools/check_test_registry.py сверяет реестр с определёнными наборами.
    void Register (const char *name, const char *group, TestFn fn);

    // Запуск: "" — все; "spec" — группа; "TestA,TestB" — список; "Sync*" — префикс.
    // Возвращает число упавших проверок.
    int Run (const char *filter = "");

    // Пропустить весь набор с причиной (нет проекта, нет элемента).
    [[noreturn]] void SkipTest (const GS::UniString &reason);

    // Аналог DBprnt для тестов (#231). Прод-DBprnt не годится: печатает
    // "== ERROR ==" префикс, если текст случайно содержит "err"/"ERROR"
    // (CommonFunction.cpp:310) — измерение выглядит как ошибка; склеивает
    // аргументы через " : " без структуры; пишет только в панель «Отладка»,
    // мимо файла отчёта, поэтому прогон без IDE ничего не показывает.
    //
    // Форма строки отчёта (парная ключ=значение — читается и глазом, и grep'ом):
    //
    //   NOTE [Verbose] suite | key=value | key=value | текст
    //
    // Именованные поля идут перед свободным текстом и всегда разобраны по '=',
    // поэтому значение, содержащее '=', остаётся однозначным.
    // -------------------------------------------------------------------------

    // Уровни записи, меняются через Config::noteLevel до Run.
    enum class NoteLevel {
        Quiet,   // только сводка и отклонения
        Normal,  // измерения, осмысленные на фиксированном наборе
        Verbose, // всё, включая заведомо шумные замеры по элементам проекта
    };

    // Явная установка уровня из кода: после неё SMSTF_VERBOSE не перекрывает
    // значение (иначе «по умолчанию 1» не отличить от «явно попросили Normal»).
    void SetNoteLevel (NoteLevel level);

    // Именованное поле: структурная замена позиционного "msg : value".
    struct Field {
        const char *key;
        GS::UniString value;
    };

    // Запись измерения; suite подставляет раннер (набор известен текущим прогоном).
    // fields/count — ноль, если полей нет; text — свободный текст в конце.
    // minLevel — минимальный уровень, при котором запись печатается. Normal
    // печатается всегда, Verbose — только при SMSTF_VERBOSE=2: заведомо шумные
    // замеры (тексты описаний по 24 штуки на элемент) на фиксированном наборе
    // бесполезны и забивают отчёт.
    void Note (const char *subject, const Field *fields, USize count, const GS::UniString &text, NoteLevel minLevel);

    inline void Note (const char *subject, const GS::UniString &text) {
        Note (subject, nullptr, 0, text, NoteLevel::Normal);
    }

    // Пара «ключ = значение» для Note. Арифметика приводится продовым
    // GS::ValueToUniString, поэтому у вызывающего нет возни с форматом.
    // Квалификация — с оператором глобальной области (::TestKit), а не просто
    // TestKit: вызовы идут изнутри namespace TestFunc, и короткое имя искалось бы
    // как TestFunc::TestKit (MSVC C2039). Без квалификации вовсе — C2065:
    // пространства TestKit и TestFunc соседние, не вложенные.
    #define SMSTF_FIELD(key, value)                                                                                    \
        ::TestKit::Field { key, ::TestKit::detail::FieldValue (value) }
    #define SMSTF_FIELD_U(key, value)                                                                                  \
        ::TestKit::Field { key, value }

    // Переменное число полей: принимает список SMSTF_FIELD через initializer_list,
    // поэтому последним аргументом можно передать текст — тип определяет перегрузка.
    void NoteFields (const char *subject, std::initializer_list<Field> fields, NoteLevel minLevel = NoteLevel::Normal);

    inline void NoteFields (const char *subject,
                            std::initializer_list<Field> fields,
                            const GS::UniString &text,
                            NoteLevel minLevel = NoteLevel::Normal) {
        Note (subject, fields.begin (), (USize)fields.size (), text, minLevel);
    }

    #define SMSTF_NOTE(subject, ...) ::TestKit::NoteFields (subject, __VA_ARGS__)

    // Проверка, продолжающая набор при падении / прерывающая его (предусловия,
    // разыменование указателей).
    #define DBtest(...) TestKit::Check (TestKit::Loc{__FILE__, __LINE__, false}, __VA_ARGS__)
    #define DBrequire(...) TestKit::Check (TestKit::Loc{__FILE__, __LINE__, true}, __VA_ARGS__)
    #define DBskip(reason) TestKit::SkipTest (reason)

    namespace detail {
        bool Report (
            const Loc &l, bool ok, const std::string &expected, const std::string &actual, const GS::UniString &msg);

        std::string FmtBool (bool v);
        std::string FmtInt (long long v);
        std::string FmtDbl (double v);
        std::string FmtStr (const GS::UniString &v);

        // Приведение значения поля к строке. GS::ValueToUniString — шаблон поверх
        // GS::valuetostr, у которого нет перегрузок для bool и USize
        // (DevKit/APIDevKit-25/Support/Modules/GSRoot/CH.hpp:524-558): поэтому
        // bool печатается как true/false, а USize (= UInt32, Definitions.hpp:387)
        // приводится явно. Точные перегрузки выигрывают у шаблона.
        inline GS::UniString FieldValue (bool v) { return v ? GS::UniString ("true") : GS::UniString ("false"); }

        inline GS::UniString FieldValue (USize v) { return GS::ValueToUniString ((UInt32)v); }

        template <class T> GS::UniString FieldValue (const T &v) { return GS::ValueToUniString (v); }

        template <class T>
        struct IsNum : std::integral_constant<bool, std::is_arithmetic<T>::value || std::is_enum<T>::value> {};

        // Двойная ветка НЕ вводит новую константу допуска: используется продовый
        // is_equal (CommonFunction.cpp) с absTol=1e-12 / relTol=1e-9. Вторая
        // константа в тестах дала бы расхождение с прода на больших значениях
        // (набор TestSpecValueEdges ждёт 2147483648.0 и -2147483649.0).
        template <class T, class U> bool CheckNum (const Loc &l, T a, U e, const GS::UniString &msg, std::true_type) {
            const double da = static_cast<double> (a);
            const double de = static_cast<double> (e);
            return Report (l, is_equal (da, de), FmtDbl (de), FmtDbl (da), msg);
        }

        template <class T, class U> bool CheckNum (const Loc &l, T a, U e, const GS::UniString &msg, std::false_type) {
            const long long la = static_cast<long long> (a);
            const long long le = static_cast<long long> (e);
            return Report (l, la == le, FmtInt (le), FmtInt (la), msg);
        }
    } // namespace detail

    // ---- перегрузки Check: те же формы вызова, что у старых DBtest ----
    inline bool Check (const Loc &l, bool cond, const GS::UniString &msg) {
        return detail::Report (l, cond, "true", "false", msg);
    }

    inline bool Check (const Loc &l, bool a, bool e, const GS::UniString &msg) {
        return detail::Report (l, a == e, detail::FmtBool (e), detail::FmtBool (a), msg);
    }

    inline bool Check (const Loc &l, const GS::UniString &a, const GS::UniString &e, const GS::UniString &msg) {
        return detail::Report (l, a == e, detail::FmtStr (e), detail::FmtStr (a), msg);
    }

    // Указатель/сырой указатель как условие: приводится к bool неявно.
    inline bool Check (const Loc &l, const void *cond, const GS::UniString &msg) {
        return detail::Report (l, cond != nullptr, "non-null", "null", msg);
    }

    template <class T, class U>
    typename std::enable_if<detail::IsNum<T>::value && detail::IsNum<U>::value, bool>::type Check (
        const Loc &l, T a, U e, const GS::UniString &msg) {
        return detail::CheckNum (
            l,
            a,
            e,
            msg,
            std::integral_constant<bool, std::is_floating_point<T>::value || std::is_floating_point<U>::value> ());
    }

} // namespace TestKit

#endif // TESTING
