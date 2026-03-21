import { MoveKnob1, MoveKnob2, MoveKnob3, MoveKnob4,
         MoveKnob5, MoveKnob6, MoveKnob7, MoveKnob8,
         MoveBack } from '/data/UserData/schwung/shared/constants.mjs';
import { decodeDelta } from '/data/UserData/schwung/shared/input_filter.mjs';
import { drawMenuHeader } from '/data/UserData/schwung/shared/menu_layout.mjs';

const PARAMS = [
    { key: "dry_level",   name: "Dry",    fmt: v => Math.round(v)+'%'       },
    { key: "early_level", name: "Early",  fmt: v => Math.round(v)+'%'       },
    { key: "late_level",  name: "Late",   fmt: v => Math.round(v)+'%'       },
    { key: "size",        name: "Size",   fmt: v => v.toFixed(1)+'m'        },
    { key: "decay",       name: "Decay",  fmt: v => v.toFixed(1)+'s'        },
    { key: "diffuse",     name: "Diff",   fmt: v => Math.round(v)+'%'       },
    { key: "delay",       name: "Pre",    fmt: v => Math.round(v)+'ms'      },
    { key: "width",       name: "Width",  fmt: v => Math.round(v)+'%'       },
];

const STEPS = [1, 1, 1, 0.5, 0.1, 1, 1, 1];
const KNOB_CCS = [MoveKnob1, MoveKnob2, MoveKnob3, MoveKnob4,
                  MoveKnob5, MoveKnob6, MoveKnob7, MoveKnob8];

let vals = [80, 10, 20, 24, 1.3, 90, 4, 90];
let needsRedraw = true;

function fetchParams() {
    for (let i = 0; i < PARAMS.length; i++) {
        const v = host_module_get_param(PARAMS[i].key);
        if (v !== null && v !== undefined) vals[i] = parseFloat(v) || vals[i];
    }
}

function setParam(i, v) {
    vals[i] = v;
    host_module_set_param(PARAMS[i].key, v.toFixed(4));
}

function drawUI() {
    clear_screen();
    drawMenuHeader("Dragonfly Hall");
    const cols = [0, 32, 64, 96];
    const rows = [16, 38];
    for (let i = 0; i < 8; i++) {
        const x = cols[i % 4];
        const y = rows[Math.floor(i / 4)];
        print(x, y,      PARAMS[i].name, 1);
        print(x, y + 10, PARAMS[i].fmt(vals[i]), 1);
    }
    needsRedraw = false;
}

globalThis.init = function() {
    fetchParams();
    needsRedraw = true;
};

globalThis.tick = function() {
    if (needsRedraw) drawUI();
};

globalThis.onMidiMessageInternal = function(data) {
    if (data[1] < 10) return; // filter knob touch
    if ((data[0] & 0xF0) !== 0xB0) return;
    const idx = KNOB_CCS.indexOf(data[1]);
    if (idx < 0) return;
    const delta = decodeDelta(data[2]);
    if (delta === 0) return;
    setParam(idx, vals[idx] + delta * STEPS[idx]);
    needsRedraw = true;
};
