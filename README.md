# Dragonfly Hall Reverb for Schwung

A port of [Dragonfly Hall Reverb](https://github.com/michaelwillis/dragonfly-reverb) by Michael Willis for [Schwung](https://github.com/charlesvestal/move-anything) on Ableton Move.

## Features

- 25 factory presets across 5 categories (Rooms, Studios, Small Halls, Medium Halls, Large Halls)
- 8 knob-mapped parameters: Dry, Early, Late, Size, Decay, Diffuse, Predelay, Width
- Full parameter access via jog wheel menu: Modulation, Wander, High Cut, High Cross, High Mult, Low Cut, Low Cross, Low Mult
- Signal Chain compatible — works as an Audio FX in Shadow Mode patches

## Installation

### Via Schwung Module Store (coming soon)

### Manual Installation

```bash
./scripts/install.sh
```

Or copy manually via SSH:
```bash
rsync -av src/ ableton@move.local:/data/UserData/schwung/modules/audio_fx/dragonfly-hall/
```

## Building

Requires Docker.

```bash
./scripts/build.sh
```

Output: `dist/dragonfly-hall/dragonfly-hall.so`

## Controls

| Knob | Parameter | Range |
|------|-----------|-------|
| 1 | Dry Level | 0–100% |
| 2 | Early Level | 0–100% |
| 3 | Late Level | 0–100% |
| 4 | Size | 10–60m |
| 5 | Decay | 0.1–10s |
| 6 | Diffuse | 0–100% |
| 7 | Predelay | 0–100ms |
| 8 | Width | 50–150% |

Jog wheel menu provides access to all remaining parameters plus preset browser.

## Credits

- [Dragonfly Reverb](https://github.com/michaelwillis/dragonfly-reverb) by Michael Willis & Rob van den Berg (GPL-3.0)
- [Schwung](https://github.com/charlesvestal/move-anything) by Charles Vestal
- Port by Brad Coomber, developed with Claude (Anthropic)

## License

GPL-3.0 — see [LICENSE](LICENSE)
