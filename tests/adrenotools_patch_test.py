"""Check that the dependency patch is repeatable and rejects an unexpected source revision."""
import pathlib
import subprocess
import tempfile

patch = pathlib.Path(__file__).resolve().parents[1] / "tools/patch_adrenotools.cmake"
anchor = "    // Preload ourself, a new instance will be created since we have different linker ancestory"
with tempfile.TemporaryDirectory(prefix="xclipse-hook-test-") as tmp:
    root = pathlib.Path(tmp)
    source = root / "src/hook/hook_impl.cpp"
    source.parent.mkdir(parents=True)
    source.write_text(anchor + "\n")
    command = ["cmake", f"-DADRENO_SOURCE={root}", "-P", str(patch)]
    subprocess.run(command, check=True)
    once = source.read_bytes()
    assert b'customDriverName == "libvulkan_radeon.so"' in once
    assert b'android_link_namespaces(driverNs, systemNs, "libhardware.so")' in once
    subprocess.run(command, check=True)
    assert source.read_bytes() == once
    source.write_text("unexpected source\n")
    result = subprocess.run(command, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    assert result.returncode != 0
print("adrenotools_patch_test: all passed")
