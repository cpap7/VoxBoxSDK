import subprocess
import shutil
import os
import stat
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
PIPER_DIR = ROOT_DIR / "VoxBox-PiperAPI" / "Vendor" / "piper"
BUILD_DIR = ROOT_DIR / "ext-bin" / "piper"  # Short path for CMake build (avoids MAX_PATH)

def _force_rmtree(path: Path):
    """Remove a directory tree, forcing removal of read-only files (e.g., .git pack files on Windows)."""
    def _on_rm_error(func, filepath, _exc_info):
        os.chmod(filepath, stat.S_IWRITE)
        func(filepath)
    shutil.rmtree(path, onerror=_on_rm_error)

def clean():
    """Remove cached build directories from both locations."""
    for config in ["debug", "release"]:
        for parent in [PIPER_DIR, BUILD_DIR]:
            cache_dir = parent / config
            if cache_dir.exists():
                print(f"  Removing {cache_dir}")
                _force_rmtree(cache_dir)
    # Remove build cache dir itself if empty
    if BUILD_DIR.exists() and not any(BUILD_DIR.iterdir()):
        BUILD_DIR.rmdir()

def build(config: str, force_rebuild: bool = False):
    print(f"\n{'='*50}")
    print(f"\tBuilding Piper ({config})")
    print('='*50)

    preset = f"vb-x64-windows-{config.lower()}"
    cache_dir = BUILD_DIR / config.lower()
    final_dir = PIPER_DIR / config.lower() # Where premake links from
    output_dir = ROOT_DIR / "VoxBox-PiperAPI" / "bin" / f"{config}-x86_64"

    # Force rebuild if requested
    if force_rebuild:
        for d in [cache_dir, final_dir]:
            if d.exists():
                print(f"  Cleaning {d}...")
                _force_rmtree(d)

    # If final output exists, just copy and skip build
    if final_dir.exists() and (final_dir / config / "piper.exe").exists():
        print(f"  Found cached {config} build, copying...")
        _copy_from_cache(final_dir, output_dir, config)
        print(f"[SETUP] Piper ({config}) complete (from cache)")
        return

    # Configure and build using presets (builds to ext-bin/ at solution root)
    print(f"  Configuring with preset: {preset}")    
    subprocess.run(["cmake", "--preset", preset], cwd=PIPER_DIR, check=True)

    print(f"  Building...")
    subprocess.run(["cmake", "--build", "--preset", preset], cwd=PIPER_DIR, check=True)

    # Relocate build output into Vendor/piper/<config>/
    print(f"  Relocating build output to {final_dir}...")
    if final_dir.exists():
        _force_rmtree(final_dir)
    shutil.move(str(cache_dir), str(final_dir))

    # Clean up build_cache if empty
    if BUILD_DIR.exists() and not any(BUILD_DIR.iterdir()):
        BUILD_DIR.rmdir()
    
    # Copy to final output
    _copy_from_cache(final_dir, output_dir, config)
    print(f"[SETUP] Piper ({config}) complete")

def _copy_from_cache(cache_dir: Path, output_dir: Path, config: str):
    """Copy from preset build directory to final output."""
    output_dir.mkdir(parents=True, exist_ok=True)
    (output_dir / "espeak").mkdir(exist_ok=True)

    exe_src = cache_dir / config / "piper.exe"
    if exe_src.exists():
        print(f"  Copying piper.exe")
        shutil.copy(exe_src, output_dir / "piper.exe")

    for dll in ["espeak-ng.dll", "piper_phonemize.dll"]:
        src = cache_dir / "pi/bin" / dll
        if src.exists():
            print(f"  Copying {dll}")
            shutil.copy(src, output_dir / dll)

    onnx_src = cache_dir / "pi/lib/onnxruntime.dll"
    if onnx_src.exists():
        print(f"  Copying onnxruntime.dll")
        shutil.copy(onnx_src, output_dir / "onnxruntime.dll")

    espeak_src = cache_dir / "pi/share/espeak-ng-data"
    espeak_dst = output_dir / "espeak/espeak-ng-data"
    if espeak_src.exists():
        print(f"  Copying espeak-ng-data/")
        shutil.copytree(espeak_src, espeak_dst, dirs_exist_ok=True)

if __name__ == "__main__":
    import sys
    force = "--force" in sys.argv
    build("Debug", force)
    build("Release", force)