import argparse
import BuildPiper
import BuildWhisper
import BuildLlama

def main():
    parser = argparse.ArgumentParser(description="VoxBoxSDK Setup")
    parser.add_argument("--config", choices=["Debug", "Release", "All"], default="All")
    parser.add_argument("--skip-piper", action="store_true")
    parser.add_argument("--skip-whisper", action="store_true")
    parser.add_argument("--skip-llama", action="store_true")
    parser.add_argument("--clean", action="store_true", help="Clean all cached builds before building")
    parser.add_argument("--rebuild", action="store_true", help="Force rebuild (delete cache first)")
    args = parser.parse_args()

    # Clean all caches if requested
    if args.clean:
        print("\nCleaning build caches...")
        if not args.skip_piper:
            BuildPiper.clean()
        if not args.skip_whisper:
            BuildWhisper.clean()
        if not args.skip_llama:
            BuildLlama.clean()
        print("[SETUP] Cleanup complete")
        if not args.rebuild:
            return  # Just clean, don't build

    configs = ["Debug", "Release"] if args.config == "All" else [args.config]

    for config in configs:
        if not args.skip_piper:
            BuildPiper.build(config, force_rebuild=args.rebuild)
        if not args.skip_whisper:
            BuildWhisper.build(config, force_rebuild=args.rebuild)
        if not args.skip_llama:
            BuildLlama.build(config, force_rebuild=args.rebuild)
    print("\n[SETUP] Setup complete!")

if __name__ == "__main__":
    main()