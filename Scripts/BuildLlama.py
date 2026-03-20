import subprocess
import shutil
import os
import stat
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
LLAMA_DIR = ROOT_DIR / "VoxBox-LlamaAPI" / "Vendor" / "llama.cpp"
BUILD_DIR = Path("C:/VoxBoxSDK-Cache/llama.cpp")   # Short path for CMake build (avoids MAX_PATH)

def _force_rmtree(path: Path):
    """Remove a directory tree, forcing removal of read-only files (e.g., .git pack files on Windows)."""
    def _on_rm_error(func, filepath, _exc_info):
        os.chmod(filepath, stat.S_IWRITE)
        func(filepath)
    shutil.rmtree(path, onerror=_on_rm_error)

def clean():
    """Remove cached build directories from both locations."""
    for config in ["debug", "release"]:
        for parent in [LLAMA_DIR, BUILD_DIR]:
            cache_dir = parent / config
            if cache_dir.exists():
                print(f"  Removing {cache_dir}")
                _force_rmtree(cache_dir)
    # Remove build_cache dir itself if empty
    if BUILD_DIR.exists() and not any(BUILD_DIR.iterdir()):
        BUILD_DIR.rmdir()

def build(config: str, force_rebuild: bool = False):
    print(f"\n{'='*50}")
    print(f"\tBuilding llama.cpp ({config})")
    print('='*50)

    preset = f"vb-x64-win-{config.lower()}-static-vulkan"
    cache_dir = BUILD_DIR / config.lower()       # Where CMake builds (short path)
    final_dir = LLAMA_DIR / config.lower()          # Where premake links from

    # Force rebuild if requested
    if force_rebuild:
        for d in [cache_dir, final_dir]:
            if d.exists():
                print(f"  Cleaning {d}...")
                _force_rmtree(d)

    # Check if already relocated (VS multi-config: src/<Config>/llama.lib)
    llama_lib = final_dir / "src" / config / "llama.lib"
    if llama_lib.exists():
        print(f"  Found cached {config} build, skipping...")
        print(f"[SETUP] llama.cpp ({config}) complete (from cache)")
        return

    # Configure CMake (builds to ext-bin/ at solution root)
    print(f"  Configuring with preset: {preset}")
    subprocess.run(["cmake", "--preset", preset], cwd=LLAMA_DIR, check=True)

    # Build
    print(f"  Building...")
    subprocess.run(["cmake", "--build", "--preset", preset], cwd=LLAMA_DIR, check=True)

    # Relocate build output into Vendor/llama.cpp/<config>/
    print(f"  Relocating build output to {final_dir}...")
    if final_dir.exists():
        _force_rmtree(final_dir)
    shutil.move(str(cache_dir), str(final_dir))

    # Clean up build_cache if empty
    if BUILD_DIR.exists() and not any(BUILD_DIR.iterdir()):
        BUILD_DIR.rmdir()

    # Verify output
    if llama_lib.exists():
        print(f"[SETUP] llama.cpp ({config}) complete")
    else:
        print(f"[ERROR] llama.lib not found at expected path: {llama_lib}")
        print(f"  Contents of {final_dir / 'src'}:")
        if (final_dir / "src").exists():
            for item in (final_dir / "src").rglob("*.lib"):
                print(f"    {item}")

if __name__ == "__main__":
    import sys
    force = "--force" in sys.argv
    build("Debug", force)
    build("Release", force)