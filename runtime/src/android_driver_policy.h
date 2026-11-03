// Android Vulkan driver selection. The hardware name is read from the system Vulkan driver.
#pragma once

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>

namespace nx::plat {

enum class AndroidDriver { system, turnip, radv, error };
struct AndroidDriverPlan {
    AndroidDriver preferred;
    bool fallback_to_system;
    const char* error;
};

inline bool supported_xclipse(const std::string& device_name) {
    std::string normalized;
    for (unsigned char c : device_name)
        if (std::isalnum(c)) normalized += char(std::tolower(c));
    const auto pos = normalized.find("xclipse960");
    return pos != std::string::npos &&
           (pos + 10 == normalized.size() || !std::isdigit(static_cast<unsigned char>(normalized[pos + 10])));
}

inline AndroidDriverPlan android_driver_plan(const std::string& requested, bool adreno, const std::string& system_gpu) {
    if (requested == "system") return {AndroidDriver::system, false, nullptr};
    if (requested == "auto" || requested.empty()) {
        if (supported_xclipse(system_gpu)) return {AndroidDriver::radv, true, nullptr};
        if (adreno) return {AndroidDriver::turnip, true, nullptr};
        return {AndroidDriver::system, false, nullptr};
    }
    if (requested == "radv-xclipse") {
        if (supported_xclipse(system_gpu)) return {AndroidDriver::radv, false, nullptr};
        return {AndroidDriver::error, false, "RADV Xclipse requires a detected Xclipse 960 GPU"};
    }
    if (requested == "turnip") {
        if (adreno) return {AndroidDriver::turnip, false, nullptr};
        return {AndroidDriver::error, false, "Turnip requires an Adreno GPU"};
    }
    return {AndroidDriver::error, false, "Unknown Vulkan driver selection"};
}

struct AndroidDriverResult {
    void* loader;
    AndroidDriver effective;
};

template <typename Load>
AndroidDriverResult select_android_driver(const AndroidDriverPlan& plan, Load load) {
    if (plan.preferred == AndroidDriver::error) return {nullptr, AndroidDriver::error};
    if (void* p = load(plan.preferred)) return {p, plan.preferred};
    if (plan.fallback_to_system)
        if (void* p = load(AndroidDriver::system)) return {p, AndroidDriver::system};
    return {nullptr, AndroidDriver::error};
}

// Fill absent settings only. An existing frame-rate choice must not gain an implicit 30 FPS cap.
template <typename Missing, typename Set>
void apply_xclipse_defaults(Missing missing, Set set) {
    const bool no_rate = missing("video.frame_rate");
    const std::pair<const char*, const char*> defaults[] = {
        {"video.mode", "handheld"}, {"video.resolution", "720"}, {"video.upscaler", "fsr"},
        {"video.frame_rate", "30"}, {"video.frame_gen", "off"}, {"video.shader_prepare", "1"},
        {"video.async_shaders", "0"}, {"video.thermal", "balanced"},
    };
    for (const auto& [key, value] : defaults)
        if (missing(key)) set(key, value);
    if (no_rate && missing("video.frame_cap")) set("video.frame_cap", "30");
}
}  // namespace nx::plat
