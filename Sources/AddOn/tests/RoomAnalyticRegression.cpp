// Самостоятельная проверка общих production-helper без SDK и модели Archicad.
#ifdef ROOM_ANALYTIC_REGRESSION
    #include <iostream>

    #include "tests/RoomAnalyticChecks.hpp"

int main () {
    int passed = 0, failed = 0;
    Roombook::CheckRoomAnalyticGeometry ([&] (bool ok, const char *name) {
        std::cout << (ok ? "PASS: " : "FAIL: ") << name << '\n';
        ok ? ++passed : ++failed;
    });
    std::cout << "passed=" << passed << " failed=" << failed << '\n';
    return failed == 0 ? 0 : 1;
}
#endif
