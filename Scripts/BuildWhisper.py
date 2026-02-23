import subprocess
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
ROOT_DIR = SCRIPT_DIR.parent
WHISPER_DIR = ROOT_DIR / "VoxBox-WhisperAPI" / "Vendor" / "whisper.cpp"

def clean():
    """Remove cached build directories."""
    for folder in ["debug", "release"]:
        cache_dir = WHISPER_DIR / folder
        if cache_dir.exists():
            print(f"  Removing {cache_dir}")
            shutil.rmtree(cache_dir)

def build(config: str, force_rebuild: bool = False):
    print(f"\n{'='*50}")
    print(f"\tBuilding Whisper.cpp ({config})")
    print('='*50)

    preset = f"vb-x64-win-{config.lower()}-static-vulkan"
    cache_dir = WHISPER_DIR / config.lower()

    # Force rebuild if requested
    if force_rebuild and cache_dir.exists():
        print(f"  Cleaning {config} cache...")
        shutil.rmtree(cache_dir)

    # Check if already built
    whisper_lib = cache_dir / "src" / config / "whisper.lib"
    if whisper_lib.exists():
        print(f"  Found cached {config} build, skipping...")
        print(f"[SETUP] Whisper.cpp ({config}) complete (from cache)")
        return

    # Configure and build using presets
    subprocess.run(["cmake", "--preset", preset], cwd=WHISPER_DIR, check=True)
    subprocess.run(["cmake", "--build", "--preset", preset], cwd=WHISPER_DIR, check=True)

    print(f"[SETUP] Whisper.cpp ({config}) complete")