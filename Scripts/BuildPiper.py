import subprocess
import shutil
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
PIPER_DIR = ROOT_DIR / "VoxBox-PiperAPI" / "Vendor" / "piper"

def clean():
    """Remove cached build directories."""
    for folder in ["debug", "release", "build"]:
        cache_dir = PIPER_DIR / folder
        if cache_dir.exists():
            print(f"  Removing {cache_dir}")
            shutil.rmtree(cache_dir)

def build(config: str, force_rebuild: bool = False):
    print(f"\n{'='*50}")
    print(f"\tBuilding Piper ({config})")
    print('='*50)

    preset = f"vb-x64-windows-{config.lower()}"
    cache_dir = PIPER_DIR / config.lower()
    output_dir = ROOT_DIR / "VoxBox-PiperAPI" / "bin" / f"{config}-x86_64"

    # Force rebuild if requested
    if force_rebuild and cache_dir.exists():
        print(f"  Cleaning {config} cache...")
        shutil.rmtree(cache_dir)

    # If cache exists, just copy and skip build
    if cache_dir.exists() and (cache_dir / config / "piper.exe").exists():
        print(f"  Found cached {config} build, copying...")
        _copy_from_cache(cache_dir, output_dir, config)
        print(f"[SETUP] Piper ({config}) complete (from cache)")
        return

    # Configure and build using presets
    subprocess.run(["cmake", "--preset", preset], cwd=PIPER_DIR, check=True)
    subprocess.run(["cmake", "--build", "--preset", preset], cwd=PIPER_DIR, check=True)

    # Copy to final output
    _copy_from_cache(cache_dir, output_dir, config)
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