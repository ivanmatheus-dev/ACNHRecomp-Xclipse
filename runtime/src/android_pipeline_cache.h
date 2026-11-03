#pragma once
#include <string>

namespace nx::plat {
// Restrict cleanup to pipeline files produced by this runtime, not arbitrary game data.
inline bool android_pipeline_cache_file(const std::string& name) {
    constexpr const char* prefix = "pipelines_";
    if (name.rfind(prefix, 0) != 0) return false;
    size_t end = name.size();
    if (end >= 8 && name.compare(end - 8, 8, ".bin.tmp") == 0) end -= 8;
    else if (end >= 5 && name.compare(end - 5, 5, ".keys") == 0) end -= 5;
    else if (end >= 4 && name.compare(end - 4, 4, ".bin") == 0) end -= 4;
    else return false;
    if (end <= 10) return false;
    for (size_t i = 10; i < end; ++i) {
        const char c = name[i];
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '_')) return false;
    }
    return true;
}
} // namespace nx::plat
