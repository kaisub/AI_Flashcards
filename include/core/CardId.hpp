#pragma once

#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>

namespace core {

    inline std::string generateUniqueCardId() {
        static thread_local std::mt19937_64 rng(std::random_device{}());
        static constexpr uint64_t kMaxIdValue = 999999999999ULL; // 12 digits
        std::uniform_int_distribution<uint64_t> dist(0, kMaxIdValue);

        std::ostringstream oss;
        oss << std::setw(12) << std::setfill('0') << dist(rng);
        return oss.str();
    }

    inline std::string generateUniqueCardId(std::unordered_set<std::string>& usedIds) {
        std::string candidate;
        do {
            candidate = generateUniqueCardId();
        } while (usedIds.contains(candidate));
        usedIds.insert(candidate);
        return candidate;
    }

} // namespace core