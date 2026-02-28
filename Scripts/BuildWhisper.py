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

    # Configure CMake
    print(f"  Configuring with preset: {preset}")
    subprocess.run(["cmake", "--preset", preset], cwd=WHISPER_DIR, check=True)

    # Build using presets
    print(f" Building...")
    subprocess.run(["cmake", "--build", "--preset", preset], cwd=WHISPER_DIR, check=True)
    
    # Verify output
    if whisper_lib.exists():
        print(f"[SETUP] Whisper.cpp ({config}) complete")
    else:
        print(f"[ERROR] whisper.lib not found at expected path: {whisper_lib}")
        # List what was actually created
        print(f"  Contents of {cache_dir / 'src'}:")
        if (cache_dir / "src").exists():
            for item in (cache_dir / "src").rglob("*.lib"):
                print(f"    {item}")

if __name__ == "__main__":
    import sys
    force = "--force" in sys.argv
    build("Debug", force)
    build("Release", force)    
    