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

    struct CoordinateSearchStats {
        std::size_t calls = 0;
        std::size_t hits = 0;
        std::size_t refused = 0;
    };

    // Хранилище без SDK: production и исполняемый regression используют один helper.
    // Побитовый ключ сохраняет nextafter и знаковый ноль; нечисловые запросы всё равно вызывают API.
    template <class Result> class ExactCoordinateSearchCache {
      public:
        template <class Search> Result SearchOrRead (const CoordinateSearchRequest &request, Search &&search) {
            if (!std::isfinite (request.x) || !std::isfinite (request.y) || !std::isfinite (request.z)) {
                ++stats.refused;
                ++stats.calls;
                return search ();
            }
            const Key key = {
                Bits (request.x), Bits (request.y), Bits (request.z), request.type, request.variation, request.filter};
            const auto found = results.find (key);
            if (found != results.end ()) {
                ++stats.hits;
                return found->second;
            }
            ++stats.calls;
            Result result = search ();
            results.emplace (key, result); // Кэш сохраняет успешный, пустой и ошибочный результат.
            return result;
        }

        const CoordinateSearchStats &GetStats () const { return stats; }

      private:
        using Key = std::array<std::uint64_t, 6>;

        static std::uint64_t Bits (double value) {
            static_assert (sizeof (value) == sizeof (std::uint64_t), "64-bit double required");
            std::uint64_t bits;
            std::memcpy (&bits, &value, sizeof (bits));
            return bits;
        }

        std::map<Key, Result> results;
        CoordinateSearchStats stats;
    };
} // namespace Roombook
