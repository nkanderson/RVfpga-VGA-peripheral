# ============================================================================
# File: sprite_rom_ip.tcl
# Author: Jacob Burtenshaw
# Date Created: 2026-05-23
# Description: Vivado TCL script to add the sprite_rom Block Memory Generator
#              IP to the project. Uses the PROJECT_ROOT environment variable
#              so any user can run it without editing hardcoded paths.
#
#              Configuration:
#                Module name  : sprite_rom
#                Memory type  : Single Port ROM
#                Width        : 32 bits
#                Depth        : 4096 entries  (12-bit address)
#                Output reg   : NONE  (1-cycle read latency — required by
#                               the scanline prefetch FSM in wb_vga.sv)
#                Init file    : guitarSprites.coe
#
#              Addresses 0..3039 are bigfont glyphs (IDs 0-94).
#              Addresses 3040..3583 are guitar sprites (IDs 95-111).
#              Addresses 3584..4095 default to 0 (reserved).
#
# ============================================================================
#
# SETUP — Before sourcing this script, set PROJECT_ROOT to the absolute path
#         of the cloned RVfpga-VGA-peripheral repository on your machine.
#
#         Also set VIVADO_PROJECT to the absolute path of the Vivado .xpr file.
#
#         From the Vivado Tcl Console (substitute your actual paths):
#
#           set ::env(PROJECT_ROOT)    {/your/path/to/RVfpga-VGA-peripheral}
#           set ::env(VIVADO_PROJECT)  {/your/path/to/VIVADO_WORK_AREA/final_project/final_project.xpr}
#           source {/your/path/to/RVfpga-VGA-peripheral/src/VeeRwolf/Peripherals/vga/sprite_rom_ip.tcl}
#
#         Typical values for the original developer:
#           PROJECT_ROOT   = /home/jburtens/masters_program/ECE_540/FINAL_PROJECT/RVfpga-VGA-peripheral
#           VIVADO_PROJECT = /home/jburtens/masters_program/ECE_540/FINAL_PROJECT/VIVADO_WORK_AREA/final_project/final_project.xpr
#
# ============================================================================

# Validate required environment variables
foreach req_var {PROJECT_ROOT VIVADO_PROJECT} {
    if {![info exists ::env($req_var)] || $::env($req_var) eq ""} {
        error "Environment variable $req_var is not set.\nSee the SETUP section at the top of this script."
    }
}

set project_root [file normalize $::env(PROJECT_ROOT)]
set project_xpr  [file normalize $::env(VIVADO_PROJECT)]

# sprite_rom.xci lives inside the Vivado project tree (sibling of the repo)
set vivado_dir  [file dirname $project_xpr]
set sprite_xci  [file join $vivado_dir "final_project.srcs" "sources_1" "ip" "sprite_rom" "sprite_rom.xci"]

# Open project if not already open
if {[catch {current_project}]} {
    puts "Opening project: $project_xpr"
    open_project $project_xpr
} else {
    puts "Using already-open project: [current_project]"
}

if {![file exists $sprite_xci]} {
    error "sprite_rom.xci not found at:\n  $sprite_xci\nEnsure the sprite_rom IP has been added to the Vivado project."
}

# Remove the old IP from the project fileset if it was previously added
if {[llength [get_ips -quiet sprite_rom]] > 0} {
    puts "Removing old sprite_rom from project fileset..."
    catch {export_ip_user_files -of_objects [get_ips sprite_rom] -no_script -reset -force -quiet}
    catch {remove_files [get_files $sprite_xci]}
}

puts "Adding sprite_rom.xci to project..."
add_files -norecurse $sprite_xci

# Generate output products
generate_target {instantiation_template} [get_ips sprite_rom]
generate_target all                       [get_ips sprite_rom]

puts ""
puts "sprite_rom IP added and output products generated successfully."
