# ============================================================================
# File: sprite_rom_ip.tcl
# Author: Jacob Burtenshaw
# Date Created: 2026-05-23
# Description: Vivado TCL script to configure the sprite_rom Block Memory
#              Generator IP. If sprite_rom.xci already exists it is
#              re-registered and the COE path is refreshed. If the XCI is
#              absent the IP is created from scratch. Uses the PROJECT_ROOT
#              and VIVADO_PROJECT environment variables so any user can run
#              it without editing hardcoded paths.
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

# COE file is in the VGA peripheral directory inside the repo
set coe_file    [file join $project_root "src" "VeeRwolf" "Peripherals" "vga" "guitarSprites.coe"]

# sprite_rom.xci lives inside the Vivado project tree (sibling of the repo)
set vivado_dir  [file dirname $project_xpr]
set sprite_xci  [file join $vivado_dir "final_project.srcs" "sources_1" "ip" "sprite_rom" "sprite_rom.xci"]

# Validate COE file exists before doing anything
if {![file exists $coe_file]} {
    error "guitarSprites.coe not found at:\n  $coe_file\nEnsure PROJECT_ROOT points to the repository root."
}

# Open project if not already open
if {[catch {current_project}]} {
    puts "Opening project: $project_xpr"
    open_project $project_xpr
} else {
    puts "Using already-open project: [current_project]"
}

# Remove any stale IP registration before proceeding
if {[llength [get_ips -quiet sprite_rom]] > 0} {
    puts "Removing existing sprite_rom from project fileset..."
    catch {export_ip_user_files -of_objects [get_ips sprite_rom] -no_script -reset -force -quiet}
    catch {remove_files [get_files $sprite_xci]}
}

if {![file exists $sprite_xci]} {
    # XCI does not exist — create the IP from scratch
    puts "sprite_rom.xci not found. Creating IP from scratch..."

    set ip_dir [file dirname $sprite_xci]
    file mkdir $ip_dir

    create_ip \
        -name blk_mem_gen \
        -vendor xilinx.com \
        -library ip \
        -version 8.4 \
        -module_name sprite_rom \
        -dir $ip_dir

    set_property -dict [list \
        CONFIG.Memory_Type                              {Single_Port_ROM} \
        CONFIG.Write_Width_A                            {32}              \
        CONFIG.Write_Depth_A                            {4096}            \
        CONFIG.Enable_A                                 {Always_Enabled}  \
        CONFIG.Register_PortA_Output_of_Memory_Primitives {false}         \
        CONFIG.Register_PortA_Output_of_Memory_Core    {false}            \
        CONFIG.Load_Init_File                           {true}            \
        CONFIG.Coe_File                                 $coe_file         \
    ] [get_ips sprite_rom]

    puts "sprite_rom IP created with COE: $coe_file"
} else {
    # XCI exists — re-add it and update the COE path in case it changed
    puts "Found existing sprite_rom.xci. Re-adding and updating COE path..."
    add_files -norecurse $sprite_xci
    set_property CONFIG.Coe_File $coe_file [get_ips sprite_rom]
}

# Generate output products
generate_target {instantiation_template} [get_ips sprite_rom]
generate_target all                       [get_ips sprite_rom]

puts ""
puts "sprite_rom IP ready. COE: $coe_file"
puts "sprite_rom IP output products generated successfully."
