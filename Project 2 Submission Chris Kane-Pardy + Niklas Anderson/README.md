# ECE 540 Project 1 – GPIO Expansion & RGB PWM Peripheral (Nexys A7)
### By Chris Kane-Pardy

## Overview

This project implements a custom RGB LED peripheral for the VeeRwolf (VeeR-EL2 RISC-V) SoC on the Nexys A7-DDR FPGA. The design extends the baseline system from earlier labs by integrating pushbutton inputs (Part 2) and a new PWM-based RGB LED controller with Wishbone bus support (Part 3).

## Submission Details

This submission is organized to clearly separate the development stages of the project and to make it easy to locate all required deliverables. **Part 2** contains the GPIO2 pushbutton integration. **Part 3** builds on this foundation by introducing a custom RGB PWM peripheral. The included design log "**Project 1 Design Log.pdf**" provides a explanation of the system architecture, design decisions, debugging process, and how the hardware and software components interact to achieve the final functionality.

    Project 1 Submission Chris Kane-Pardy
    ├── Part 2/
    ├── Part 3/
    ├── Project 1 Design Log.pdf
    └── README.md

### Part 2 – GPIO2 Pushbutton Integration

This directory contains all deliverables for Part 2 of the project.

    Part 2/
    ├── Source Code/
    ├── HDL/
    └── Video/


#### Source Code/
- Main.S
- rvfpganexys.bit

#### HDL/
  - Updated `veerwolf_core.v`
  - Updated `wb_intercon.v` and `wb_intercon.vh`
  - Updated `rvfpganexys.sv` and `rvfpganexys.xdc`

#### Video/
- Demonstration of working Part 2 functionality

### Part 3 – RGB PWM Peripheral

This directory contains all deliverables for the final project implementation.

    Part 3/
    ├── Source Code/
    ├── HDL/
    └── Video/

#### Source Code/
- rvfpganexys.bit
- main.c

#### HDL/
This folder contains both **modified system files** and **newly created modules**.

    HDL/
    ├── Modified RVFPGA Files/
    └── RGB-PWM Files/

##### Modified RVFPGA Files/
  - Updated `veerwolf_core.v`
  - Updated `wb_intercon.v`
  - Updated `wb_intercon.vh`
  - Updated `rvfpganexys.sv`
  - Updated `rvfpganexys.xdc`

##### RGB-PWM Files/
Contains new modules created for the RGB peripheral:

- `pwm_channel.sv`  → Single-channel PWM generator (parameterized, counter-based)
- `rgb_pwm.sv`      → 3-channel RGB controller (instantiates 3 PWM modules)
- `wb_rgb_pwm.sv`   → Wishbone slave wrapper exposing two RGB LEDs via memory-mapped registers

#### Video/
- Demonstration of full Part 3 functionality