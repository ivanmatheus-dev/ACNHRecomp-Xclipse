#include "android_driver_policy.h"
#include "android_pipeline_cache.h"
#include <cstdio>
#include <map>

using namespace nx::plat;

int main() {
    int errors = 0;
    auto expect = [&](const char* pick, bool adreno, const char* gpu, AndroidDriver driver, bool fallback) {
        const auto p = android_driver_plan(pick, adreno, gpu);
        if (p.preferred != driver || p.fallback_to_system != fallback || (driver == AndroidDriver::error) != (p.error != nullptr)) {
            std::printf("FAIL: %s, %s\n", pick, gpu);
            ++errors;
        }
    };
    expect("auto", false, "Samsung Xclipse 960", AndroidDriver::radv, true);
    expect("auto", false, "XCLIPSE-960", AndroidDriver::radv, true);
    expect("auto", false, "Xclipse 9600", AndroidDriver::system, false);
    expect("auto", false, "Samsung Xclipse 950", AndroidDriver::system, false);
    expect("auto", false, "", AndroidDriver::system, false);
    expect("auto", true, "Adreno", AndroidDriver::turnip, true);
    expect("system", false, "Xclipse 960", AndroidDriver::system, false);
    expect("radv-xclipse", false, "Xclipse 960", AndroidDriver::radv, false);
    expect("radv-xclipse", false, "Mali", AndroidDriver::error, false);
    expect("turnip", true, "Adreno", AndroidDriver::turnip, false);
    expect("turnip", false, "Xclipse 960", AndroidDriver::error, false);
    expect("bad", true, "Adreno", AndroidDriver::error, false);
    int calls = 0;
    auto load_system_only = [&](AndroidDriver d) -> void* {
        ++calls;
        return d == AndroidDriver::system ? &calls : nullptr;
    };
    auto fallback = select_android_driver(android_driver_plan("auto", false, "Xclipse 960"), load_system_only);
    if (fallback.loader != &calls || fallback.effective != AndroidDriver::system || calls != 2) ++errors;
    calls = 0;
    auto explicit_radv = select_android_driver(android_driver_plan("radv-xclipse", false, "Xclipse 960"), load_system_only);
    if (explicit_radv.loader || explicit_radv.effective != AndroidDriver::error || calls != 1) ++errors;
    calls = 0;
    auto no_loader = select_android_driver(android_driver_plan("auto", false, "Xclipse 960"), [&](AndroidDriver) -> void* {
        ++calls;
        return nullptr;
    });
    if (no_loader.loader || no_loader.effective != AndroidDriver::error || calls != 2) ++errors;
    calls = 0;
    auto unsupported = select_android_driver(android_driver_plan("radv-xclipse", false, "Mali"), load_system_only);
    if (unsupported.loader || calls != 0) ++errors;
    std::map<std::string, std::string> settings;
    auto missing = [&](const char* key) { return !settings.count(key); };
    auto set = [&](const char* key, const char* value) { settings[key] = value; };
    apply_xclipse_defaults(missing, set);
    if (settings["video.resolution"] != "720" || settings["video.frame_cap"] != "30" ||
        settings["video.async_shaders"] != "0" || settings["video.frame_gen"] != "off") ++errors;
    settings = {{"video.frame_rate", "60"}, {"video.resolution", "1080"}, {"video.async_shaders", "1"}};
    apply_xclipse_defaults(missing, set);
    if (settings["video.resolution"] != "1080" || settings["video.async_shaders"] != "1" ||
        settings.count("video.frame_cap") != 0) ++errors;
    if (!android_pipeline_cache_file("pipelines_1002_1234.bin") ||
        !android_pipeline_cache_file("pipelines_1002_1234_abcd.keys") ||
        !android_pipeline_cache_file("pipelines_1002_1234.bin.tmp") ||
        android_pipeline_cache_file("pipelines_../../save.bin") ||
        android_pipeline_cache_file("pipelines_save.bin") || android_pipeline_cache_file("save.bin")) ++errors;
    std::puts(errors ? "android_driver_policy_test: FAILED" : "android_driver_policy_test: all passed");
    return errors ? 1 : 0;
}
