//------------ kuvbur 2026 ------------
// TestKit — реализация: файловый отчёт, счётчики, отбор наборов (#231).
//
// Каждая строка сбрасывается на диск сразу: если набор уронит ArchiCAD, всё
// напечатанное до этого сохраняется, и в файле остаётся BEGIN без парного
// END — по нему видно, где прогон оборвался.
#ifdef TESTING

    #include <cstdio>
    #include <cstdlib>
    #include <vector>

    #include "tests/TestKit.hpp"

    // Вывод в панель «Отладка» идёт тем же каналом, что и у прод-DBprnt:
    // DBPrintf (AC24+), DBPrint (AC22-23). Канал зеркалит отчёт, поэтому
    // результаты видны и в IDE, и в файле.
    #ifndef ServerMainVers_2300
        #define SMSTF_PRINT(...) DBPrintf (__VA_ARGS__)
    #else
        // DBPrint принимает ровно один аргумент (без varargs), поэтому строка
        // собирается в буфер, а не форматируется на месте.
        #define SMSTF_PRINT(...)                                                                                       \
            do {                                                                                                       \
                char smSTF_buf_[1024];                                                                                 \
                std::snprintf (smSTF_buf_, sizeof (smSTF_buf_), __VA_ARGS__);                                          \
                DBPrint (smSTF_buf_);                                                                                  \
            } while (false)
    #endif

namespace TestKit {

    namespace {

        struct Entry {
            const char *name;
            const char *group;
            TestFn fn;
        };

        // Реестр живёт здесь, а не в статических инициализаторах наборов: при
        // разбиении файла по модулям регистратор вместе со своей static-функцией
        // может быть выкинут линковкой, и набор молча исчезнет из Run() без
        // единого предупреждения. Наполняется явно из TestFunc::RegisterAll.
        std::vector<Entry> &Registry () {
            static std::vector<Entry> registry;
            return registry;
        }

        struct Stats {
            int passed = 0;
            int failed = 0;
            int suppressed = 0;
            int suites = 0;
            int failedSuites = 0;
            bool aborted = false; // набор упал через DBrequire
            bool skipped = false; // набор пропущен через DBskip
        };

        Stats g_stats;
        std::FILE *g_file = nullptr;

        // Путь к файлу отчёта: явно заданный путь, иначе каталог TEMP.
        std::string ReportPath () {
            const std::string custom = GetConfig ().reportPath;
            if (!custom.empty ())
                return custom;
            const char *tmp = std::getenv ("TMPDIR");
            if (tmp == nullptr || *tmp == '\0')
                tmp = std::getenv ("TEMP");
            if (tmp == nullptr || *tmp == '\0')
                tmp = "/tmp";
            return std::string (tmp) + "/somestuff_test_report.txt";
        }

        void Open () {
            if (g_file != nullptr)
                return;
            const std::string path = ReportPath ();
            g_file = std::fopen (path.c_str (), "w");
            if (g_file == nullptr) {
                // Файл недоступен — не молчим: сообщаем в отладочный вывод.
                SMSTF_PRINT ("somestuff tests: cannot open report %s\n", path.c_str ());
                return;
            }
            SMSTF_PRINT ("somestuff tests: report %s\n", path.c_str ());
        }

        void Emit (const std::string &line) {
            if (g_file != nullptr) {
                std::fputs (line.c_str (), g_file);
                std::fputc ('\n', g_file);
                // Сброс на каждую строку: файл обязан пережить падение ArchiCAD.
                std::fflush (g_file);
            }
            // Дублируем в панель вывода VS, иначе после запуска через раннер
            // результаты негде смотреть.
            SMSTF_PRINT ("%s\n", line.c_str ());
            const std::function<void (const GS::UniString &)> &mirror = GetConfig ().mirror;
            if (mirror)
                mirror (GS::UniString (line.c_str ()));
        }

        // Точное совпадение, префикс "X*" или список через запятую.
        bool Matches (std::string filter, const char *name) {
            if (filter.empty ())
                return true;
            if (filter == name)
                return true;
            const std::string f = filter;
            const std::string n (name);
            // "Sync*" — префикс по группам и именам наборов.
            if (!f.empty () && f[f.size () - 1] == '*') {
                const std::string stem = f.substr (0, f.size () - 1);
                return n.compare (0, stem.size (), stem) == 0;
            }
            // Список через запятую: "TestA,TestB".
            size_t pos = 0;
            while (pos <= f.size ()) {
                const size_t comma = f.find (',', pos);
                const std::string item = f.substr (pos, comma == std::string::npos ? std::string::npos : comma - pos);
                if (!item.empty () && n == item)
                    return true;
                if (comma == std::string::npos)
                    break;
                pos = comma + 1;
            }
            return false;
        }

    } // namespace

    Config &GetConfig () {
        static Config config;
        return config;
    }

    void Register (const char *name, const char *group, TestFn fn) { Registry ().push_back (Entry{name, group, fn}); }

    void SkipTest (const GS::UniString &reason) {
        g_stats.skipped = true;
        Emit ("SKIP " + std::string (reason.ToCStr (0, MaxUSize, CC_UTF8).Get ()));
        throw AbortTest ();
    }

    void Info (const char *key, const GS::UniString &value) {
        Open ();
        Emit ("INFO " + std::string (key) + " = " + value.ToCStr (0, MaxUSize, CC_UTF8).Get ());
    }

    namespace detail {

        std::string FmtBool (bool v) { return v ? "true" : "false"; }

        std::string FmtInt (long long v) {
            char buf[32];
            std::snprintf (buf, sizeof (buf), "%lld", v);
            return std::string (buf);
        }

        std::string FmtDbl (double v) {
            char buf[64];
            std::snprintf (buf, sizeof (buf), "%g", v);
            return std::string (buf);
        }

        std::string FmtStr (const GS::UniString &v) { return v.ToCStr (0, MaxUSize, CC_UTF8).Get (); }

        bool Report (
            const Loc &l, bool ok, const std::string &expected, const std::string &actual, const GS::UniString &msg) {
            Open ();
            const std::string text = msg.ToCStr (0, MaxUSize, CC_UTF8).Get ();
            if (ok) {
                ++g_stats.passed;
                // Успех по умолчанию молчит: на фиксированном наборе иначе
                // каждая правка продукта засоряет отчёт тысячами строк.
                if (GetConfig ().printPass)
                    Emit ("PASS " + text + " = " + actual);
                return true;
            }

            ++g_stats.failed;
            if (g_stats.failed <= GetConfig ().maxFailuresPerTest) {
                Emit ("FAIL " + text + " | expected " + expected + " got " + actual + " | " + l.file + ":" +
                      detail::FmtInt (l.line));
            } else if (g_stats.failed == GetConfig ().maxFailuresPerTest + 1) {
                Emit ("FAIL ... further failures suppressed (maxFailuresPerTest=" +
                      detail::FmtInt (GetConfig ().maxFailuresPerTest) + ")");
            } else {
                ++g_stats.suppressed;
            }

            // DBrequire останавливает набор: следующая строка обычно разыменует
            // результат, и без остановки ArchiCAD упал бы целиком.
            if (l.required) {
                Emit ("ABORT " + text + " (DBrequire)");
                g_stats.aborted = true;
                throw AbortTest ();
            }
            return false;
        }

    } // namespace detail

    int Run (const char *filter) {
        const std::string sel = filter ? filter : "";
        Open ();
        g_stats = Stats ();
        Emit ("=== somestuff tests begin ===");

        std::vector<std::string> failedNames;
        for (const Entry &entry : Registry ()) {
            if (!Matches (sel, entry.name) && !Matches (sel, entry.group))
                continue;
            const int before = g_stats.failed;
            const int passedBefore = g_stats.passed;
            ++g_stats.suites;
            g_stats.skipped = false;
            Emit ("BEGIN " + std::string (entry.name));
            try {
                entry.fn ();
            } catch (const AbortTest &) {
                // набор прерван DBrequire или DBskip — уже отмечено в Emit
            } catch (...) {
                ++g_stats.failed;
                Emit ("FAIL unhandled exception in suite " + std::string (entry.name));
            }
            // passed= и failed= по набору: без них нельзя сверить число
            // выполненных проверок с числом вызовов DBtest в коде и поймать
            // набор, который молча перестал выполняться.
            Emit ("END " + std::string (entry.name) + " passed=" + detail::FmtInt (g_stats.passed - passedBefore) +
                  " failed=" + detail::FmtInt (g_stats.failed - before));
            if (g_stats.failed > before) {
                ++g_stats.failedSuites;
                failedNames.push_back (entry.name);
            } else if (g_stats.skipped) {
                Emit ("SKIPPED " + std::string (entry.name));
            }
        }

        Emit ("SUMMARY suites=" + detail::FmtInt (g_stats.suites) + " passed=" + detail::FmtInt (g_stats.passed) +
              " failed=" + detail::FmtInt (g_stats.failed) + " suppressed=" + detail::FmtInt (g_stats.suppressed));
        for (const std::string &name : failedNames)
            Emit ("FAILED_SUITE " + name);
        // exit-код для скрипта: ненулевой при любом провале.
        Emit (std::string ("EXIT ") + detail::FmtInt (g_stats.failed > 0 ? 1 : 0));
        Emit ("=== somestuff tests end ===");
        return g_stats.failed;
    }

} // namespace TestKit

#endif // TESTING
