#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"

echo "Building Dragonfly Hall Reverb..."
docker build --output type=local,dest="$ROOT/dist/dragonfly-hall" "$ROOT"
echo "Build complete: dist/dragonfly-hall/dragonfly-hall.so"
