#include "nvn_cache_identity.h"

#include <array>
#include <cstdio>

using nx::nvn::pipeline_cache_compatible;

int main() {
    std::array<uint8_t, 40> cache{};
    std::array<uint8_t, 16> uuid{};
    for (size_t i = 0; i < uuid.size(); ++i) cache[16 + i] = uuid[i] = uint8_t(i);
    cache[0] = 32; cache[4] = 1; cache[8] = 2; cache[12] = 3;
    int failed = 0;
    auto check = [&](bool actual, bool expected) { if (actual != expected) ++failed; };
    check(pipeline_cache_compatible(cache.data(), cache.size(), 2, 3, uuid.data()), true);
    check(pipeline_cache_compatible(nullptr, cache.size(), 2, 3, uuid.data()), false);
    check(pipeline_cache_compatible(cache.data(), 31, 2, 3, uuid.data()), false);
    check(pipeline_cache_compatible(cache.data(), cache.size(), 9, 3, uuid.data()), false);
    check(pipeline_cache_compatible(cache.data(), cache.size(), 2, 9, uuid.data()), false);
    uuid[0] ^= 1;
    check(pipeline_cache_compatible(cache.data(), cache.size(), 2, 3, uuid.data()), false);
    uuid[0] ^= 1;
    cache[4] = 2;
    check(pipeline_cache_compatible(cache.data(), cache.size(), 2, 3, uuid.data()), false);
    cache[4] = 1; cache[0] = 41;
    check(pipeline_cache_compatible(cache.data(), cache.size(), 2, 3, uuid.data()), false);
    cache[0] = 31;
    check(pipeline_cache_compatible(cache.data(), cache.size(), 2, 3, uuid.data()), false);
    std::printf("pipeline_cache_identity_test: %s\n", failed ? "FAILED" : "all passed");
    return failed ? 1 : 0;
}
