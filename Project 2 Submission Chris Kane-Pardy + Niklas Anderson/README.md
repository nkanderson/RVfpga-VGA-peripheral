# ECE 540 Project 2 – VGA Peripheral Design (Nexys A7)
### By Chris Kane-Pardy & Niklas Anderson

## LLM Acknowledgement

Part 3 application code was generated with the assistance of a large language model (LLM) after initial manual planning and design. The LLM code generation was guided by the specifications and requirements provided by the authors, and the final code was reviewed and edited by the authors to ensure correctness and adherence to the intended functionality.

## Overview

This project implements a custom VGA display peripheral for the VeeRwolf (VeeR-EL2 RISC-V) SoC on the Nexys A7-DDR FPGA. The design extends the baseline system from earlier labs by integrating a Wishbone-controlled VGA controller, BRAM-based font ROM rendering, and both text and graphics display modes. The final implementation supports software-controlled VGA animation and ASCII character rendering through memory-mapped hardware registers.

## Submission Details

This submission is organized to clearly separate the major hardware and software components of the project and to make all required deliverables easy to locate.

The hardware portion (Part 2) includes:
- Wishbone VGA peripheral integration
- VGA signal routing
- BRAM/font ROM configuration
- Text and graphics rendering support

The software portion includes:
- A text-mode VGA application
- A graphics-mode VGA screensaver application

The included design log, **"Project 2 Design Log.pdf"**, explains the system architecture, VGA rendering pipeline, hardware/software interaction, design decisions, debugging process, and testing performed throughout development.

    Project 2 Submission Chris Kane-Pardy + Niklas Anderson
    ├── Part 2/
    ├── Part 3/
    ├── Project 2 Design Log.pdf
    └── README.md

### Part 2 – VGA Wishbone Integration

This directory contains all deliverables for Part 2 of the project.

    Part 2/
    ├── HDL/
    └── rvfpanexys.bit

The VGA peripheral supports:
- Wishbone memory-mapped control
- Text rendering using BRAM-based font ROM
- Graphics rendering mode
- Foreground/background color control
- VGA synchronization generation
- 640x480 VGA output

#### HDL/
This directory contains the modified and newly created hardware source files required for VGA support.

Included files:
- Updated `veerwolf_core.sv`
- Updated `wb_intercon.v`
- Updated `wb_intercon.vh`
- Updated `rvfpganexys.sv`
- Updated `rvfpganexys.xdc`
- Updated `clk_gen_nexys.v`
- Generated `font_rom.vhd` BRAM configuration
  
**vga/**
- Implemented `wb_vga.sv`
- Included `dtg.sv`
- Included `bigfont.coe`

#### rvfpganexys.bit

This file contains the final synthesized and implemented FPGA bitstream for the complete VGA-enabled RVfpga system.

The bitstream includes:
- Modified Wishbone interconnect
- Integrated VGA peripheral
- BRAM-based font ROM
- VGA synchronization logic
- Text and graphics display support

### Part 3 – C Apps

This directory contains the software applications used to demonstrate the VGA peripheral functionality.

    Part 3/
    ├── Source Code/
    └── Video/

#### Source Code/
**Application1.c**

Text-mode VGA demonstration application.

Features:
- Displays ASCII digits using the hardware font ROM
- Demonstrates memory-mapped Wishbone register control
- Displays even values as `0`
- Uses programmable foreground/background colors
- Demonstrates software-controlled text rendering

**Application2.c**

Graphics-mode VGA screensaver application.

Features:
- Animated bouncing square
- Dynamic color cycling
- Software-controlled VGA animation
- Real-time coordinate updates
- Demonstrates graphics rendering mode

#### Video/

**Application1.mov**

Demonstration of:
- VGA text-mode rendering
- ASCII character display
- Foreground/background color control
- Software-driven register updates

**Application2.mov**

Demonstration of:
- VGA graphics-mode rendering
- Real-time animation
- Dynamic color transitions
- Bouncing screensaver behavior