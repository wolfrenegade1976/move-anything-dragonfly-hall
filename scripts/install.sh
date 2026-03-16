#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$SCRIPT_DIR")"

MOVE_HOST="${MOVE_HOST:-move.local}"
MOVE_USER="${MOVE_USER:-ableton}"
DEST="/data/UserData/move-anything/modules/audio_fx/dragonfly-hall"

echo "Installing to $MOVE_USER@$MOVE_HOST:$DEST"
rsync -av "$ROOT/src/" "$MOVE_USER@$MOVE_HOST:$DEST/"
echo "Done. Reload the module on your Move."
