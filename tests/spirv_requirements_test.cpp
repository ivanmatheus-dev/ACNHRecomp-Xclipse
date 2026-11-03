#include "nvn_spirv_requirements.h"
#include <cstdio>

using namespace nx::nvn;
int main() {
    // Minimal vertex shader with Shader and Float64 capabilities. Assembled and validated with SPIRV-Tools
    // for Vulkan 1.1; it is synthetic and contains no game data.
    const uint32_t vertex[] = {
        0x07230203, 0x00010300, 0x00070000, 0x00000005, 0x00000000, 0x00020011, 0x00000001,
        0x00020011, 0x0000000a, 0x0003000e, 0x00000000, 0x00000001, 0x0005000f, 0x00000000,
        0x00000001, 0x6e69616d, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003,
        0x00000002, 0x00050036, 0x00000002, 0x00000001, 0x00000000, 0x00000003, 0x000200f8,
        0x00000004, 0x000100fd, 0x00010038};
    const uint32_t shader[] = {0x07230203, 0x00010300, 0, 10, 0,
                               0x00020011, 1, 0x00020011, 9, 0x00020011, 5378, 0x00020011, 9999};
    SpirvFeatureSupport features;
    features.float16 = true;
    auto vertex_result = check_spirv_requirements(vertex, sizeof(vertex) / sizeof(*vertex), features, 1);
    int failed = !vertex_result.valid || vertex_result.unsupported.size() != 1 || vertex_result.unsupported[0] != 10;
    features.float64 = true;
    failed += !check_spirv_requirements(vertex, sizeof(vertex) / sizeof(*vertex), features, 1).unsupported.empty();
    auto result = check_spirv_requirements(shader, sizeof(shader) / sizeof(*shader), features);
    failed += !result.valid || result.unsupported.size() != 1 || result.unsupported[0] != 5378 ||
              result.unchecked.size() != 1 || result.unchecked[0] != 9999;
    features.pixel_interlock = true;
    result = check_spirv_requirements(shader, sizeof(shader) / sizeof(*shader), features);
    failed += !result.valid || !result.unsupported.empty();
    const uint32_t subgroup[] = {0x07230203, 0x00010300, 0, 1, 0, 0x00020011, 61, 0x00020011, 65};
    features.subgroup_stages = 0x20; // compute
    features.subgroup_operations = (1u << 0) | (1u << 4); // basic and shuffle
    failed += !check_spirv_requirements(subgroup, sizeof(subgroup) / sizeof(*subgroup), features, 0x20).unsupported.empty();
    failed += check_spirv_requirements(subgroup, sizeof(subgroup) / sizeof(*subgroup), features, 0x10).unsupported.size() != 2;
    features.subgroup_operations = 1;
    failed += check_spirv_requirements(subgroup, sizeof(subgroup) / sizeof(*subgroup), features, 0x20).unsupported.size() != 1;
    const uint32_t truncated[] = {0x07230203, 0x00010000, 0, 1, 0, 0x00020011};
    failed += check_spirv_requirements(truncated, sizeof(truncated) / sizeof(*truncated), features).valid;
    const uint32_t zero_words[] = {0x07230203, 0x00010000, 0, 1, 0, 0};
    failed += check_spirv_requirements(zero_words, sizeof(zero_words) / sizeof(*zero_words), features).valid;
    failed += check_spirv_requirements(nullptr, 0, features).valid;
    std::puts(failed ? "spirv_requirements_test: FAILED" : "spirv_requirements_test: all passed");
    return failed ? 1 : 0;
}
