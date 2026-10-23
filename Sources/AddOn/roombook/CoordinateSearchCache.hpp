#pragma once

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <map>
#include <utility>

namespace Roombook {
    struct CoordinateSearchRequest {
        double x;
        double y;
        double z;
        std::uint64_t type;
        std::uint64_t variation;
        std::uint64_t filter;
    };

    // Хранилище без SDK: production и исполняемый regression используют один helper.
    // Побитовый ключ сохраняет nextafter и знаковый ноль; нечисловые запросы всё равно вызывают API.
    template <class Result> class ExactCoordinateSearchCache {
      public:
        template <class Search> Result SearchOrRead (const CoordinateSearchRequest &request, Search &&search) {
            if (!std::isfinite (request.x) || !std::isfinite (request.y) || !std::isfinite (request.z)) {
                return search ();
            }
            const Key key = {
                Bits (request.x), Bits (request.y), Bits (request.z), request.type, request.variation, request.filter};
            const auto found = results.find (key);
            if (found != results.end ()) {
                return found->second;
            }
            Result result = search ();
            results.emplace (key, result); // Кэш сохраняет успешный, пустой и ошибочный результат.
            return result;
        }

      private:
        using Key = std::array<std::uint64_t, 6>;

        static std::uint64_t Bits (double value) {
            static_assert (sizeof (value) == sizeof (std::uint64_t), "64-bit double required");
            std::uint64_t bits;
            std::memcpy (&bits, &value, sizeof (bits));
            return bits;
        }

        std::map<Key, Result> results;
    };
} // namespace Roombook
