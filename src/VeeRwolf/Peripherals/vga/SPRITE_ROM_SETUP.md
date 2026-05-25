# sprite_rom IP Generation Guide

Generate the `sprite_rom` Vivado Block Memory Generator IP used by `wb_vga.sv`.

Files referenced here live in `src/VeeRwolf/Peripherals/vga/`.

---

## Option A — TCL Script (Recommended)

### 1. Set environment variables

From the **Vivado Tcl Console**, set the two required variables before sourcing
the script (substitute your actual paths):

```tcl
set ::env(PROJECT_ROOT)    {/your/path/to/RVfpga-VGA-peripheral}
set ::env(VIVADO_PROJECT)  {/your/path/to/VIVADO_WORK_AREA/final_project/final_project.xpr}
```

### 2. Source the script

```tcl
source {/your/path/to/RVfpga-VGA-peripheral/src/VeeRwolf/Peripherals/vga/sprite_rom_ip.tcl}
```

That's it. Skip to **After Generation** below.

---

## Option B — IP Catalog GUI

### 1. Open the IP Catalog
In your Vivado project: **Flow Navigator → IP Catalog**

### 2. Find Block Memory Generator
Search for `Block Memory Generator` → double-click to open the wizard.

### 3. Basic tab
| Setting | Value |
|---------|-------|
| Component Name | `sprite_rom` |
| Interface Type | `Native` |
| Memory Type | `Single Port ROM` |

### 4. Port A Options tab
| Setting | Value |
|---------|-------|
| Port A Width | `32` |
| Port A Depth | `4096` |
| Operating Mode | `Read First` |
| Enable Port Type | `Always Enabled` |
| **Primitives Output Register** | ☐ **unchecked** |
| Core Output Register | ☐ unchecked |
| RSTA Pin | ☐ unchecked |

> ⚠️ **"Primitives Output Register" must be OFF.** This gives 1-cycle read latency,
> matching the timing assumed by the prefetch FSM in `wb_vga.sv`. Enabling it adds
> a second pipeline stage and will cause a 1-row bitmap misalignment.

### 5. Other Options tab
| Setting | Value |
|---------|-------|
| Load Init File | ☑ checked |
| Coefficient File | Browse to `src/VeeRwolf/Peripherals/vga/guitarSprites.coe` |
| Fill Remaining Memory Locations | `0` (default) |

> The COE file defines 3584 entries (IDs 0–94 bigfont glyphs, IDs 95–111 guitar
> sprites). Addresses 3584–4095 default to `0x00000000` (reserved).

### 6. Summary tab
Click **OK**, then **Generate**.

---

## After Generation

Vivado places the generated IP under `<project>.srcs/sources_1/ip/sprite_rom/`.
Add `sprite_rom.xci` to your project source set if it is not added automatically.

Re-run **Generate Bitstream** — synthesis will pick up the new IP automatically.

---

## Verification Checklist

- [ ] `sprite_rom` appears in the IP Sources panel
- [ ] No COE path errors in the Tcl console during generation
- [ ] Synthesis completes without `sprite_rom` unresolved-module errors
- [ ] `blk_mem_gen` utilization shows 1× BRAM36 in the resource report
