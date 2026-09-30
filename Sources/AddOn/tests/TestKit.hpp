//------------ kuvbur 2026 ------------
// TestKit — вывод и учёт результатов тестов AddOn_SomeStuff (#231).
//
// Заменяет вывод через прод-DBtest/DBprnt ТОЛЬКО внутри тестовых единиц
// трансляции. Продовые вызовы (Helpers.cpp, CommonFunction.cpp) макросом НЕ
// перекрываются: TestKit.hpp включается только в тестовых файлах.
//
// Контракт результата: файл отчёта, в котором успех молчит (printPass), а
// печатаются только отклонения и сводка. Файл переживает падение ArchiCAD,
// потому что каждая строка сбрасывается на диск сразу.
#pragma once

#ifdef TESTING

    #include <functional>
    #include <string>
    #include <type_traits>

    #include "CommonFunction.hpp" // GS::UniString, is_equal

namespace TestKit {

    // Настройки вывода. Меняются до вызова Run.
    struct Config {
        bool printPass = false;      // печатать успешные проверки (в файл)
        int maxFailuresPerTest = 20; // дальше только счётчик «ещё N подавлено»
        std::string reportPath;      // UTF-8; пусто -> каталог TEMP
        // Куда дублировать отклонения и сводку (панель «Отладка»).
        std::function<void (const GS::UniString &)> mirror;
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

    // Замена содержательных DBprnt (измерения и т.п.) — только в файл.
    void Info (const char *key, const GS::UniString &value);

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
