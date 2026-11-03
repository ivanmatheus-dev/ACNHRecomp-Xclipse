// Vulkan pipeline-cache header validation, independent of Vulkan for standalone tests.
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace nx::nvn {

inline uint32_t cache_u32le(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

inline bool pipeline_cache_compatible(const uint8_t* data, size_t size, uint32_t vendor, uint32_t device,
                                      const uint8_t uuid[16]) {
    // VkPipelineCacheHeaderVersionOne is serialized little endian and begins with 32 bytes.
    if (!data || size < 32) return false;
    const uint32_t header_size = cache_u32le(data);
    return header_size >= 32 && header_size <= size && cache_u32le(data + 4) == 1 &&
           cache_u32le(data + 8) == vendor && cache_u32le(data + 12) == device &&
           std::memcmp(data + 16, uuid, 16) == 0;
}

inline uint64_t cache_identity_hash(const char* text) {
    uint64_t h = 14695981039346656037ull;
    for (; *text; ++text) h = (h ^ static_cast<uint8_t>(*text)) * 1099511628211ull;
    return h;
}
}  // namespace nx::nvn
