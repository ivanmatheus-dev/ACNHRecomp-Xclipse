# Apply a narrow linkernsbypass namespace link for RADV's libhardware.so dependency.
# The source revision is fixed in CMakeLists.txt. Keep this idempotent for repeat builds.
set(hook "${ADRENO_SOURCE}/src/hook/hook_impl.cpp")
file(READ "${hook}" content)
set(anchor "    // Preload ourself, a new instance will be created since we have different linker ancestory")
set(marker "RADV Xclipse: cannot link system libhardware.so")
string(FIND "${content}" "${marker}" already)
if(already EQUAL -1)
    string(FIND "${content}" "${anchor}" found)
    if(found EQUAL -1)
        message(FATAL_ERROR "Unexpected libadrenotools hook source; refusing to patch")
    endif()
    set(insertion [=[    // RADV's Android WSI needs libhardware. Link only that library from the default
    // namespace; do not enable the Adreno KGSL memory mapping hooks for Xclipse.
    if (hook_params->customDriverName == "libvulkan_radeon.so") {
        auto systemNs{android_create_namespace_escape("radv_system", nullptr, nullptr,
                                                       ANDROID_NAMESPACE_TYPE_SHARED, nullptr, nullptr)};
        if (!systemNs || !android_link_namespaces(driverNs, systemNs, "libhardware.so")) {
            LOGI("RADV Xclipse: cannot link system libhardware.so");
            return fallback();
        }
    }

]=])
    string(REPLACE "${anchor}" "${insertion}${anchor}" content "${content}")
    file(WRITE "${hook}" "${content}")
endif()
