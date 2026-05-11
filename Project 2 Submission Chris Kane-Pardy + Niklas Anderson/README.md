# ECE 540 Project 2 – VGA Peripheral Design (Nexys A7)
### By Chris Kane-Pardy & Niklas Anderson

## LLM Acknowledgement

Part 3 application code was generated with the assistance of a large language model (LLM) after initial manual planning and design. The LLM code generation was guided by the specifications and requirements provided by the authors, and the final code was reviewed and edited by the authors to ensure correctness and adherence to the intended functionality.

## Overview

This project implements a custom RGB LED peripheral for the VeeRwolf (VeeR-EL2 RISC-V) SoC on the Nexys A7-DDR FPGA. The design extends the baseline system from earlier labs by integrating pushbutton inputs (Part 2) and a new PWM-based RGB LED controller with Wishbone bus support (Part 3).

## Submission Details

This submission is organized to clearly separate the development stages of the project and to make it easy to locate all required deliverables. **Part 2** contains the GPIO2 pushbutton integration. **Part 3** builds on this foundation by introducing a custom RGB PWM peripheral. The included design log "**Project 2 Design Log.pdf**" provides a explanation of the system architecture, design decisions, debugging process, and how the hardware and software components interact to achieve the final functionality.

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


#### HDL/
  - Updated `veerwolf_core.v`
  - Updated `wb_intercon.v` and `wb_intercon.vh`
  - Updated `rvfpganexys.sv` and `rvfpganexys.xdc`
  - Update 'CLOCK FILE HERE'
  - Implemented 'wb_vga.sv'

#### rvfpganexys.bit
//TODO description here

### Part 3 – RGB PWM Peripheral

This directory contains all deliverables for the final project implementation.

    Part 3/
    ├── Source Code/
    └── Video/

#### Source Code/
- Application1.c
- Application2.c

#### Video/
- Application1.mov
  - Demonstration of 
- Application2.mov
  - Demonstration of
