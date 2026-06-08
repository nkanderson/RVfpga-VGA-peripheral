<!--
Build to LaTeX/PDF with:
  pandoc docs/NoteFeller-Design.md -o NoteFeller-Design.pdf \
    -V geometry:margin=1in -V fontsize=11pt --toc
-->

# Note Feller — Design and Implementation

## 1. Overview

Note Feller is a rhythm-based music game implemented as a System-on-Chip
(SoC) design on the Digilent Nexys A7 (Xilinx Artix-7) FPGA platform. The
system scrolls note targets down a VGA monitor while the player matches note
timing using the onboard pushbuttons or a USB keyboard. Correct,
well-timed inputs raise the score and extend a combo multiplier; mistimed or
missed notes reset the combo.

The project follows a hardware/software co-design split:

- **Custom SystemVerilog peripherals** handle the real-time work — VGA
  scan-out, audio synthesis, and input sampling — exposing their state to the
  CPU through memory-mapped Wishbone registers.
- **Embedded C firmware** running on the VeeR EL2 RISC-V core
  (`NoteFeller-App/`) owns all gameplay logic: note sequencing, movement,
  hit detection, scoring, audio cues, and menu/state control.

Development proceeded incrementally, validating each subsystem independently
(VGA and input first, then audio and gameplay integration) before full system
bring-up. The codebase started from the Project 2 VGA peripheral and was
extended with new audio, input, and USB address space.

## 2. System Architecture

### 2.1 Wishbone interconnect and address space

The peripherals attach to the existing VeeRwolf Wishbone I/O bus. Adding the
new hardware required extending `wb_intercon.v` / `wb_intercon.vh` with slave
slots for audio, USB, and a second GPIO bank, plus an enlarged region for VGA.
The AXI-to-Wishbone bridge address window in `axi_intercon.sv` was widened
from `0x80004000` to `0x80010000` to cover the new slaves.

Sizing rationale: the input-controller bank reuses the compact 64-byte
`gpio2` slot, while audio, VGA, and USB each receive a full 4 KiB page. The
generous per-page allocation leaves room for register-map growth — the VGA
sprite table alone consumes 64 sprites × 2 words = 512 bytes.

### 2.2 Memory map

All peripherals live in the I/O region based at `0x80000000`.

| Peripheral            | Base address  | Size   | Module                  |
|-----------------------|---------------|--------|-------------------------|
| Boot ROM              | `0x80000000`  | 4 KiB  | `wb_mem_wrapper`        |
| System controller     | `0x80001000`  | 64 B   | `veerwolf_syscon` (7-seg) |
| SPI flash             | `0x80001040`  | 64 B   | `simple_spi_top`        |
| SPI accelerometer     | `0x80001100`  | 64 B   | `simple_spi_top`        |
| PIT/timer (PTC)       | `0x80001200`  | 64 B   | `ptc_top`               |
| GPIO                  | `0x80001400`  | 64 B   | `gpio_top`              |
| **Input controller**  | `0x80001500`  | 64 B   | `wb_input_controller`   |
| UART                  | `0x80002000`  | 4 KiB  | `uart_top`              |
| **VGA**               | `0x80003000`  | 4 KiB  | `wb_vga`                |
| **Audio**             | `0x80004000`  | 4 KiB  | `wb_audio`              |
| **USB/PS-2**          | `0x80008000`  | 4 KiB  | reserved                |

Bold rows are subsystems built or extended for Note Feller. Each custom slave
implements the same minimal Wishbone handshake: a registered one-cycle
`wb_ack_o` asserted when `wb_cyc_i & wb_stb_i` is seen, with `wb_err_o` and
`wb_rty_o` tied low.

## 3. Audio Subsystem

### 3.1 `wb_audio` — direct digital synthesis with delta-sigma output

The Nexys A7 exposes only a single-bit PWM audio pin (`aud_pwm`) plus a
shutdown/mute pin (`aud_sd`). The audio peripheral therefore synthesizes tones
digitally and reduces them to a 1-bit stream.

**Phase accumulators (DDS).** Each voice owns a 24-bit phase accumulator that
is incremented every clock by a per-note phase-increment constant drawn from a
note look-up table (`NOTE_LUT`). A larger increment wraps the accumulator
sooner, producing a higher frequency. The accumulator's most-significant bit
(bit 23) is taken as a square wave at the target pitch.

**Polyphony.** To sound chords, the single-voice design was widened to eight
parallel voices (notes C4–C5). The phase accumulators, square-wave MSBs,
per-voice volume/amplitude, and sample values are all 8-element arrays. Each
voice's square wave is scaled to a signed amplitude (`±volume`), gated by its
on/off bit, and the eight contributions are summed.

**Delta-sigma 1-bit DAC.** The summed sample is right-shifted (averaged),
biased to a positive value, and fed into a delta-sigma modulator accumulator.
The carry/MSB of that accumulator drives `aud_pwm` directly, so the time-
average of the PWM stream reconstructs the mixed analog waveform while pushing
quantization noise out of the audible band.

### 3.2 Register map (`wb_audio`, base `0x80004000`)

| Offset | Name           | Field encoding                                              |
|--------|----------------|-------------------------------------------------------------|
| `0x00` | `AUDIO_CTRL`   | `[0]` amp enable (drives `aud_sd`); `[7:4]` reserved master gain |
| `0x04` | `AUDIO_VOICES` | `[7:0]` per-voice on/off mask — bit *i* = voice *i* sounding |
| `0x08` | `AUDIO_VOL`    | Eight packed 4-bit volumes: voice *i* in bits `[i*4 +: 4]`   |

The `AUDIO_VOL` packing places C4 in `[3:0]`, D4 in `[7:4]`, … C5 in
`[31:28]`, giving independent per-note balance within a chord.

### 3.3 Worked example: note frequency → phase increment

The output frequency of a phase accumulator of width *N* clocked at
*f*<sub>clk</sub> with increment *P* is:

> *f*<sub>out</sub> = *P* · *f*<sub>clk</sub> / 2<sup>*N*</sup>

Solving for the increment stored in the LUT (here *N* = 24,
*f*<sub>clk</sub> = 25 MHz):

> *P* = round( *f*<sub>note</sub> · 2<sup>24</sup> / 25 000 000 )

**Example — A4 = 440 Hz:**

> *P* = round(440 · 16 777 216 / 25 000 000) = round(295.28) = **295 = `0x127`**

Reconstructing the realized pitch: 295 · 25 MHz / 2²⁴ = **439.6 Hz**, within
0.1 % of concert A. The full table the hardware ships with:

| Voice | Note | Freq (Hz) | Computed *P* | LUT value |
|-------|------|-----------|--------------|-----------|
| 0 | C4 | 261.63 | 175.6 | `0x0000B0` |
| 1 | D4 | 293.66 | 197.1 | `0x0000C5` |
| 2 | E4 | 329.63 | 221.2 | `0x0000DD` |
| 3 | F4 | 349.23 | 234.4 | `0x0000EA` |
| 4 | G4 | 392.00 | 263.1 | `0x000107` |
| 5 | A4 | 440.00 | 295.3 | `0x000127` |
| 6 | B4 | 493.88 | 331.5 | `0x00014C` |
| 7 | C5 | 523.25 | 351.2 | `0x00015F` |

### 3.4 `audio.c` / `audio.h` driver

The software driver hides register packing behind a small voice-oriented API:
`audio_init()` silences all voices, loads a uniform default volume, and enables
the amplifier; `audio_set_voice(idx, on)` and `audio_set_voices(mask)` toggle
notes; `audio_set_voice_volume()` performs a read-modify-write of the packed
`AUDIO_VOL` register. Voice indices map directly to scale notes
(`AUDIO_VOICE_C4 … AUDIO_VOICE_C5`), so the game can play a lane's note with a
single call.

## 4. Video Subsystem (VGA)

### 4.1 Display timing

A display-timing generator (`dtg.sv`) produces 640×480 @ 60 Hz sync signals
from a 25 MHz pixel clock, emitting `pixel_row`, `pixel_column`, and
`video_on`. The Wishbone clock doubles as the pixel clock, so VGA timing and
register access share one domain.

### 4.2 Sprite engine and scan-line prefetch FSM

Rather than a full frame buffer, `wb_vga` implements a **64-sprite scan-line
engine**. The CPU programs up to 64 sprite descriptors (type, color, position,
ROM id) ahead of time; the hardware composites them live during scan-out.

During each row's horizontal blanking interval, a prefetch FSM
time-multiplexes the single-port sprite ROM to load the 1-bpp row bitmap for
the *next* scan line of every sprite that overlaps it. With one-cycle BRAM read
latency the fetch uses ~65 of the ~160 available blanking cycles. A
two-layer combinational compositor then paints each visible pixel: the
background-color register forms the bottom layer, and sprites are applied in
priority order (sprite 0 wins). A `0` bitmap bit is transparent; `1` bits paint
the sprite's foreground color.

Each descriptor selects a **32×32** or **16×16** sprite, allowing both chunky
note/chord circles and compact text glyphs from the same engine.

> **Resource note.** The original goal was a full 12-bpp 640×480 @ 60 Hz frame
> buffer in DDR2 with a DMA path, but the BRAM/DMA implementation could not be
> finished in the available time, so the sprite engine was adopted as a
> resource-efficient partial solution.

### 4.3 Sprite ROM

A unified Vivado Block-Memory ROM (32-bit × 4096, init from
`guitarSprites.coe`) holds both worlds: IDs **0–94** are bigfont glyphs (ASCII
32–126, `sprite_id = ascii − 32`) and IDs **95–111** are the guitar/game
sprites (chord circles, note circles, squiggles, borders). The ROM address is
`{sprite_id[6:0], row_offset[4:0]}`. Unifying the text font and game art into
one ROM is what let the score and menus move off the seven-segment display and
onto the VGA screen.

### 4.4 Register map (`wb_vga`, base `0x80003000`)

| Offset           | Name            | Field encoding                                              |
|------------------|-----------------|-------------------------------------------------------------|
| `0x000`          | `REG_MODE`      | `[0]` display mode (0 = graphics)                           |
| `0x014`          | `REG_BG_COLOR`  | `[11:0]` background `{R[11:8],G[7:4],B[3:0]}`               |
| `0x080 + n·8`    | `SPRITE_POS[n]` | `[9:0]` x position; `[19:10]` y position                    |
| `0x084 + n·8`    | `SPRITE_CFG[n]` | `[31:25]` sprite id; `[15:4]` color RGB444; `[1]` type (0=32×32, 1=16×16); `[0]` visible |

### 4.5 `vga_sprite.c` / `vga_sprite.h` driver

The driver presents a `Sprite` descriptor struct (slot, id, type, color, x, y)
and packs it into the two hardware words. Helpers include `VGA_COLOR(r,g,b)`
for 12-bit colors, named `SPRITE_FORM_*` constants for every ROM entry,
`vga_set_sprite()`, `vga_clear_sprite()`, and `vga_set_bg_color()`. Higher-level
game modules (keys, notes, menu, score) build entirely on this API.

## 5. Input / I-O Subsystem

### 5.1 `wb_input_controller`

The input controller exposes player input as memory-mapped registers and adds
two features the raw GPIO lacks:

- **Edge detection.** Physical buttons are first passed through a two-flop
  synchronizer; a rising edge (`current & ~previous`) sets a *latched* event
  bit. This separates a freshly pressed key from one being held — essential for
  rhythm scoring — and lets software poll at its own pace without missing
  presses. Edge bits are write-1-to-clear.
- **Selectable input source.** An input-mode register chooses between the
  onboard pushbuttons and the PS-2/USB keyboard, so the same five gameplay
  signals (four lanes + Enter) can be driven by either device. This was
  invaluable during development and debugging.

### 5.2 `ps2_receiver`

A small PS-2 receiver synchronizes the `ps2_clk` line, detects falling edges,
and shifts in the 11-bit frame (start, 8 data LSB-first, parity, stop),
emitting a scan-code byte plus a one-cycle valid pulse. The controller decodes
Set-2 scan codes for A/S/D/F/Enter and tracks make/break (`0xF0`) prefixes to
maintain per-key state.

### 5.3 Register map (`wb_input_controller`, base `0x80001500`)

| Offset | Name           | Field encoding                                        |
|--------|----------------|-------------------------------------------------------|
| `0x00` | `INPUT_STATUS` | `[4:0]` current held inputs (4 lanes + Enter)         |
| `0x04` | `INPUT_EDGE`   | `[4:0]` latched new presses; write 1s to clear        |
| `0x08` | `INPUT_CTRL`   | `[0]` write 1 to clear all edge bits                  |
| `0x0C` | `INPUT_MODE`   | `[1:0]` source select (0 = buttons, 1 = USB/keyboard) |

### 5.4 `input_controller.c` / `input_controller.h` driver

The driver offers `input_get_status()` (held state), `input_get_edges()` /
`input_clear_edges()`, `input_set_mode()`, and the convenience
`input_poll_new_presses()` — which reads the edge register, clears the bits it
saw, and returns them. That single call is the primary hit-detection primitive
for the game loop.

## 6. Seven-Segment Display (legacy score output)

Before the score was rendered on the VGA screen, it was shown on the board's
eight-digit seven-segment display, driven through the System Controller. The
`seven_segment.c` driver writes two registers: an enables register
(`0x80001038`) and a packed-digit register (`0x8000103C`).
`sevenseg_display_score()` converts the decimal score into packed BCD nibbles.
The module is still updated in parallel with the VGA score for redundancy, but
the VGA bigfont text (Section 4.3) is now the primary readout.

| Offset       | Name              | Encoding                                  |
|--------------|-------------------|-------------------------------------------|
| `0x80001038` | `SEVENSEG_ENABLES`| Per-digit enable (bit = 1 disables digit) |
| `0x8000103C` | `SEVENSEG_DIGITS` | Eight 4-bit hex nibbles, one per digit    |

## 7. Game Engine (`NoteFeller-App`)

### 7.1 Main loop and state machine

`main.c` runs a three-state machine — `START`, `PLAYING`, `END` — each tick
polling new presses via the input driver. Enter (lane 4) starts the game from
the title screen, ends a run, and restarts from the game-over screen.
`system_init()` initializes every driver and draws the start menu;
`reset_gameplay()` clears sprites, resets score, and re-seeds notes between
runs.

### 7.2 Lanes, keys, and notes

The game uses four lanes, each bound to an input bit, an audio voice (C4–F4),
a screen column (`globals.h`), and a lane color. `key.c` owns the stationary
target sprites at the bottom of each lane; `note.c` owns a fixed pool of
falling notes (`notes[4][6]`, 24 total). Notes move at a fixed speed driven by
per-note tick counters (`TICK_THRESHOLD`, `INCREMENT_Y`), and are recycled when
they fall off the bottom of the screen.

**Sprite register budget (worked allocation of the 64 slots):**

| Slots   | Count | Purpose                                  |
|---------|-------|------------------------------------------|
| 0–3     | 4     | Lane key/target sprites (`KEY_SPRITE_OFFSET`) |
| 4–27    | 24    | Falling notes, 6 per lane (`NOTE_SPRITE_OFFSET`) |
| 28–29   | 2     | Combo multiplier glyphs (`COMBO_SPRITE_OFFSET`) |
| 30+     | rest  | Menu/score text glyphs                    |

### 7.3 Spawn cadence and pattern

`note_spawn_routine()` uses two free-running counters to create spawn
opportunities: a chord counter (fires at `SPAWN_THRESHOLD`) and a note counter
(fires at half that). When a counter fires, `rand()` decides *how many* notes
to drop that wave (0–4 for chords, 0–2 for single notes) and a second `rand()`
picks *which* distinct lanes receive them, skipping any lane already full.
`srand()` is seeded from the build timestamp (`__TIME__`) so runs vary without
depending on a cycle-counter CSR.

### 7.4 Hit detection and lane locking

Each tick, `process_note_hits()` combines the latched new-press edges with the
current held state. A per-lane state (`LANE_IDLE` → `LANE_HIT_DETECTED`) acts as
a hit-flag handshake that locks the lane after one press and only releases it
when the key is let go — preventing a held key from repeatedly scoring or
missing. `note_process_hit()` deactivates the first *hittable* note in the lane
(a note is hittable when its center overlaps the target band). A hit shows a
solid circle and plays the lane's note; a press with no hittable note flashes a
miss color and breaks the combo.

### 7.5 Scoring and combo

`score.c` awards `SCORE_POINTS_PER_HIT × multiplier` per hit. The combo count
rises with consecutive hits and the multiplier steps up in tiers (≥10 → ×2,
≥20 → ×3, ≥30 → ×4); any miss resets combo and multiplier to 1. Each update
refreshes both the seven-segment display and the on-screen VGA text/multiplier
glyph.

### 7.6 Audio sustain

To make notes ring rather than click, a hit arms a per-lane **sustain
countdown** (`AUDIO_SUSTAIN_TICKS`). The voice stays on until the counter
expires, decoupling note duration from how briefly the key was tapped and
producing more natural instrumentation.

### 7.7 Menu and on-screen text

`menu.c` renders the title, prompts, and score using the bigfont glyph sprites,
mapping each character to `sprite_id = ascii − ' '`. It draws the start screen,
the in-game score label, and the game-over screen with a right-aligned final
score.

## 8. Key Design Decisions

| Decision | Reason |
|----------|--------|
| 64-sprite scan-line engine instead of a frame buffer | A 12-bpp 640×480 frame buffer exceeded available BRAM; the DDR2 + DMA path could not be finished in time. |
| Direct digital synthesis (phase accumulators) for tones | One adder per voice generates any pitch from a small LUT; trivially scales to polyphony. |
| Delta-sigma 1-bit DAC on `aud_pwm` | The board provides only a 1-bit PWM audio pin; noise-shaping yields acceptable tone quality. |
| Sum-and-bias voice mixing | Lets eight independent voices share the single PWM output as a chord. |
| Hardware edge detection + latched event register | Cleanly distinguishes a new press from a held key and decouples input timing from CPU poll rate. |
| Selectable button / PS-2 input source | Either device can drive the same five gameplay signals, easing development and enabling the keyboard stretch goal. |
| Reuse the 64 B `gpio2` slot for input | Minimal interconnect change for a register set that needs little space. |
| 4 KiB pages for audio/VGA/USB | Headroom for register-map growth (e.g., the 512 B sprite table) and future features. |
| Unified font + sprite ROM | One ROM serves both game art and text, allowing score/menus to render on VGA. |
| Per-lane hit-flag lock state | Prevents a held key from double-scoring; one press = one scoring event. |
| Sustain countdown for note-off | Decouples audio duration from key-hold time for more realistic sound. |
| `__TIME__`-seeded `srand` | Varies spawn patterns without depending on a cycle-counter CSR. |

## 9. Summary

Note Feller demonstrates a complete hardware/software co-designed SoC on the
Nexys A7. Three custom Wishbone peripherals carry the real-time load: a
polyphonic DDS audio engine that mixes eight voices into a single delta-sigma
PWM pin, a 64-sprite scan-line VGA engine that composites game art and text
from a unified ROM without a frame buffer, and an input controller that adds
edge detection and a selectable button/keyboard source on top of raw GPIO. A
thin driver layer maps each peripheral to a clean C API, and the firmware game
engine layers note spawning, fixed-speed movement, lane-locked hit detection,
and a tiered combo-scoring system on top. The design's recurring theme is
trading ideal-but-costly approaches (frame buffer, multi-bit DAC) for
resource-efficient FPGA-friendly ones (sprite engine, sigma-delta), delivering
a responsive, musical game within the platform's BRAM and I/O constraints.
