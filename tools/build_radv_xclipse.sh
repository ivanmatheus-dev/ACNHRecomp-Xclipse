#!/usr/bin/env bash
# Build the pinned RADV Xclipse Android driver using its own non-Wayland build.
set -euo pipefail
cd "$(dirname "$0")/.."

readonly revision=fe9b8943aaeec11ed9fccbfcc382da07379afbe5
readonly source_dir="$PWD/private/third_party/radv-xclipse"
readonly sdk="${ANDROID_SDK_ROOT:-${ANDROID_HOME:-$HOME/Android/Sdk}}"
readonly ndk="$sdk/ndk/29.0.14206865"

if [[ ! -f "$ndk/source.properties" ]] || ! grep -q 'Pkg.Revision = 29.0.14206865' "$ndk/source.properties"; then
    echo "Android NDK 29.0.14206865 not found at $ndk" >&2
    exit 1
fi
for tool in git meson ninja python3 glslangValidator; do
    command -v "$tool" >/dev/null || { echo "Missing tool: $tool" >&2; exit 1; }
done
mkdir -p "$(dirname "$source_dir")"
if [[ ! -e "$source_dir" ]]; then
    git clone --no-checkout https://github.com/JimVulkan/radv-xclipse.git "$source_dir"
    git -C "$source_dir" checkout --detach "$revision"
fi
if [[ ! -d "$source_dir/.git" ]] || [[ "$(git -C "$source_dir" rev-parse HEAD)" != "$revision" ]] ||
   [[ -n "$(git -C "$source_dir" status --porcelain --untracked-files=no)" ]]; then
    echo "RADV source must be clean and at $revision: $source_dir" >&2
    exit 1
fi

# The upstream script pins the Meson wraps for libdrm, zlib and expat and builds
# only Android WSI. It uses the NDK API 34 compiler and does not enable Wayland.
"$source_dir/build.sh" "$ndk"
echo "RADV binary: $source_dir/build-android/src/amd/vulkan/libvulkan_radeon.so"
echo "RADV licenses: $source_dir/LICENSE, $source_dir/LICENSES, $source_dir/THIRD_PARTY_NOTICES.md"
