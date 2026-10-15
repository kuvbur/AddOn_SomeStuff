// Отдельный executable использует production-helper без SDK и имитации модели/замеров.
#ifdef COORDINATE_SEARCH_CACHE_REGRESSION
    #include <cstdlib>
    #include <iostream>
    #include <limits>

    #include "roombook/CoordinateSearchCache.hpp"

namespace {
    struct Result {
        int error;
        int guid;
    };

    void Require (bool condition, const char *message) {
        if (!condition) {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (1);
        }
    }
} // namespace

int main () {
    using namespace Roombook;
    ExactCoordinateSearchCache<Result> cache;
    CoordinateSearchRequest request = {1, 2, 3, 4, 0, 5};
    std::size_t calls = 0;
    const auto successful = [&] () {
        ++calls;
        return Result{0, 42};
    };
    Require (cache.SearchOrRead (request, successful).guid == 42, "success result");
    Require (cache.SearchOrRead (request, successful).guid == 42 && calls == 1, "identical request calls once");
    for (int field = 0; field < 6; ++field) {
        auto different = request;
        switch (field) {
        case 0:
            different.x = std::nextafter (request.x, 10.0);
            break;
        case 1:
            different.y = std::nextafter (request.y, 10.0);
            break;
        case 2:
            different.z = std::nextafter (request.z, 10.0);
            break;
        case 3:
            ++different.type;
            break;
        case 4:
            ++different.variation;
            break;
        case 5:
            ++different.filter;
            break;
        }
        const auto before = calls;
        cache.SearchOrRead (different, successful);
        cache.SearchOrRead (different, successful);
        Require (calls == before + 1, "all request fields distinguish keys exactly");
    }
    auto nullRequest = request;
    nullRequest.type = 100;
    const auto nullResult = [&] () {
        ++calls;
        return Result{0, 0};
    };
    Require (cache.SearchOrRead (nullRequest, nullResult).guid == 0, "null result");
    const auto beforeNull = calls;
    Require (cache.SearchOrRead (nullRequest, successful).guid == 0 && calls == beforeNull, "null cached");
    auto errorRequest = request;
    errorRequest.type = 101;
    const auto errorResult = [&] () {
        ++calls;
        return Result{-7, 17};
    };
    cache.SearchOrRead (errorRequest, errorResult);
    const auto beforeError = calls;
    const auto error = cache.SearchOrRead (errorRequest, successful);
    Require (error.error == -7 && error.guid == 17 && calls == beforeError, "error and output cached");
    for (int field = 0; field < 3; ++field) {
        for (double nonfinite : {std::numeric_limits<double>::quiet_NaN (),
                                 std::numeric_limits<double>::infinity (),
                                 -std::numeric_limits<double>::infinity ()}) {
            auto invalid = request;
            if (field == 0)
                invalid.x = nonfinite;
            if (field == 1)
                invalid.y = nonfinite;
            if (field == 2)
                invalid.z = nonfinite;
            const auto before = calls;
            Require (cache.SearchOrRead (invalid, errorResult).error == -7, "nonfinite preserves callback result");
            Require (cache.SearchOrRead (invalid, errorResult).guid == 17 && calls == before + 2,
                     "nonfinite bypasses storage every time");
        }
    }
    Require (cache.SearchOrRead (request, successful).guid == 42, "finite cache intact after NaN/infinity");
    auto zero = request;
    zero.x = 0.0;
    cache.SearchOrRead (zero, successful);
    zero.x = -0.0;
    const auto beforeZero = calls;
    cache.SearchOrRead (zero, successful);
    Require (calls == beforeZero + 1, "signed zero remains bit-exact");
    ExactCoordinateSearchCache<Result> nextRun;
    const auto beforeRun = calls;
    nextRun.SearchOrRead (request, successful);
    Require (calls == beforeRun + 1, "new run never reuses earlier results");
    const auto stats = cache.GetStats ();
    Require (stats.calls == 29 && stats.hits == 10 && stats.refused == 18, "actual calls/hits/refusals counters");
    Require (calls == stats.calls + nextRun.GetStats ().calls, "counters match actual callback executions");
    std::cout
        << "PASS: identical; exact XY/Z(nextafter)/type/variation/filter; null/error; finite guard; signed zero; run isolation\n"
        << "calls=" << stats.calls << " hits=" << stats.hits << " refused=" << stats.refused
        << " nextRunCalls=" << nextRun.GetStats ().calls << '\n';
}
#endif
