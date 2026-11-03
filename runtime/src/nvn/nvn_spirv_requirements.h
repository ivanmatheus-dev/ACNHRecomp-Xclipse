// A conservative subset of SPIR-V capability checks. Unknown capabilities are not assumed supported.
#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace nx::nvn {

struct SpirvFeatureSupport {
    bool geometry = false, tessellation = false, float16 = false, float64 = false;
    bool int64 = false, int16 = false, int8 = false;
    bool pixel_interlock = false, sample_interlock = false, shading_rate_interlock = false;
    uint32_t subgroup_stages = 0, subgroup_operations = 0;
    bool subgroup_quad_all_stages = false;
};
struct SpirvRequirements {
    bool valid = false;
    std::vector<uint32_t> unsupported;
    std::vector<uint32_t> unchecked;
};

inline SpirvRequirements check_spirv_requirements(const uint32_t* words, size_t count, const SpirvFeatureSupport& f,
                                                   uint32_t stage = 0) {
    SpirvRequirements r;
    if (!words || count < 5 || words[0] != 0x07230203) return r;
    for (size_t i = 5; i < count;) {
        const uint32_t wc = words[i] >> 16, op = words[i] & 0xffff;
        if (!wc || wc > count - i || (op == 17 && wc != 2)) return r; // OpCapability
        if (op == 17) {
            const uint32_t c = words[i + 1];
            bool known = true, supported = true;
            switch (c) {
            case 1: break; // Shader
            case 2: supported = f.geometry; break;
            case 3: supported = f.tessellation; break;
            case 9: supported = f.float16; break;
            case 10: supported = f.float64; break;
            case 11: supported = f.int64; break;
            case 22: supported = f.int16; break;
            case 39: supported = f.int8; break;
            case 61: case 62: case 63: case 64: case 65: case 66: case 67: case 68:
                supported = (f.subgroup_stages & stage) && (f.subgroup_operations & (1u << (c - 61)));
                if (c == 68 && stage != 0x10 && stage != 0x20 && !f.subgroup_quad_all_stages) supported = false;
                break;
            case 5363: supported = f.sample_interlock; break;
            case 5372: supported = f.shading_rate_interlock; break;
            case 5378: supported = f.pixel_interlock; break;
            default: known = false; break;
            }
            if (!known) r.unchecked.push_back(c);
            else if (!supported) r.unsupported.push_back(c);
        }
        i += wc;
    }
    r.valid = true;
    return r;
}
} // namespace nx::nvn
