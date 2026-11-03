#!/usr/bin/env python3
"""Verify APK native libraries against the NDK's public API 34 libraries."""
import argparse
import pathlib
import re
import subprocess
import sys

parser = argparse.ArgumentParser()
parser.add_argument("--ndk", required=True, type=pathlib.Path)
parser.add_argument("--libdir", required=True, type=pathlib.Path)
args = parser.parse_args()
toolchain = args.ndk / "toolchains/llvm/prebuilt/linux-x86_64"
readelf = toolchain / "bin/llvm-readelf"
public = toolchain / "sysroot/usr/lib/aarch64-linux-android/34"
libraries = sorted(args.libdir.glob("*.so"))
if not libraries or not readelf.is_file() or not public.is_dir():
    sys.exit("Native libraries, readelf or NDK API 34 stubs missing")
ndk_only = {"libc++.so", "libc++_shared.so", "libunwind.so"}
available = {p.name for p in libraries} | ({p.name for p in public.glob("*.so")} - ndk_only)
for lib in libraries:
    def read(*options):
        return subprocess.check_output([str(readelf), *options, str(lib)], text=True)
    if not re.search(r"Machine:\s+AArch64\b", read("-h")):
        sys.exit(f"{lib}: not AArch64")
    loads = [line.split() for line in read("-lW").splitlines() if line.strip().startswith("LOAD ")]
    if not loads or any(int(line[-1], 16) < 16384 or int(line[-1], 16) % 16384 for line in loads):
        sys.exit(f"{lib}: LOAD segments are not aligned for 16 KB pages")
    needed = re.findall(r"\(NEEDED\).*?\[(.*?)\]", read("-d"))
    missing = sorted(set(needed) - available)
    if lib.name == "libvulkan_radeon.so" and "libhardware.so" in missing:
        # RADV's WSI links this platform-private library via the targeted adrenotools namespace patch.
        # Its availability on the phone cannot be established by an NDK-only check.
        hook = args.libdir / "libhook_impl.so"
        if not hook.is_file() or b"RADV Xclipse: cannot link system libhardware.so" not in hook.read_bytes():
            sys.exit(f"{lib}: libhardware.so requires the Xclipse namespace hook")
        missing.remove("libhardware.so")
        print("libvulkan_radeon.so: requires system-private libhardware.so via Xclipse hook; verify on device")
    if missing:
        sys.exit(f"{lib}: unresolved Android dependencies: {', '.join(missing)}")
    print(f"{lib.name}: ARM64, 16 KB LOAD alignment; NEEDED: {', '.join(needed) or '(none)'}")
