
################################################################
# This is a generated script based on design: MSS_8S0L
#
# Though there are limitations about the generated script,
# the main purpose of this utility is to make learning
# IP Integrator Tcl commands easier.
################################################################

namespace eval _tcl {
proc get_script_folder {} {
   set script_path [file normalize [info script]]
   set script_folder [file dirname $script_path]
   return $script_folder
}
}
variable script_folder
set script_folder [_tcl::get_script_folder]

################################################################
# Check if script is running in correct Vivado version.
################################################################
set scripts_vivado_version 2024.1
set current_vivado_version [version -short]

if { [string first $scripts_vivado_version $current_vivado_version] == -1 } {
   puts ""
   if { [string compare $scripts_vivado_version $current_vivado_version] > 0 } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2042 -severity "ERROR" " This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Sourcing the script failed since it was created with a future version of Vivado."}

   } else {
     catch {common::send_gid_msg -ssname BD::TCL -id 2041 -severity "ERROR" "This script was generated using Vivado <$scripts_vivado_version> and is being run in <$current_vivado_version> of Vivado. Please run the script in Vivado <$scripts_vivado_version> then open the design in Vivado <$current_vivado_version>. Upgrade the design by running \"Tools => Report => Report IP Status...\", then run write_bd_tcl to create an updated script."}

   }

   return 1
}

################################################################
# START
################################################################

# To test this script, run the following commands from Vivado Tcl console:
# source MSS_8S0L_script.tcl

# If there is no project opened, this script will create a
# project, but make sure you do not have an existing project
# <./myproj/project_1.xpr> in the current working folder.

set list_projs [get_projects -quiet]
if { $list_projs eq "" } {
   create_project project_1 myproj -part xczu49dr-ffvf1760-2-e
   set_property BOARD_PART xilinx.com:zcu216:part0:2.0 [current_project]
}


# CHANGE DESIGN NAME HERE
variable design_name
set design_name MSS_8S0L

# If you do not already have an existing IP Integrator design open,
# you can create a design using the following command:
#    create_bd_design $design_name

# Creating design if needed
set errMsg ""
set nRet 0

set cur_design [current_bd_design -quiet]
set list_cells [get_bd_cells -quiet]

if { ${design_name} eq "" } {
   # USE CASES:
   #    1) Design_name not set

   set errMsg "Please set the variable <design_name> to a non-empty value."
   set nRet 1

} elseif { ${cur_design} ne "" && ${list_cells} eq "" } {
   # USE CASES:
   #    2): Current design opened AND is empty AND names same.
   #    3): Current design opened AND is empty AND names diff; design_name NOT in project.
   #    4): Current design opened AND is empty AND names diff; design_name exists in project.

   if { $cur_design ne $design_name } {
      common::send_gid_msg -ssname BD::TCL -id 2001 -severity "INFO" "Changing value of <design_name> from <$design_name> to <$cur_design> since current design is empty."
      set design_name [get_property NAME $cur_design]
   }
   common::send_gid_msg -ssname BD::TCL -id 2002 -severity "INFO" "Constructing design in IPI design <$cur_design>..."

} elseif { ${cur_design} ne "" && $list_cells ne "" && $cur_design eq $design_name } {
   # USE CASES:
   #    5) Current design opened AND has components AND same names.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 1
} elseif { [get_files -quiet ${design_name}.bd] ne "" } {
   # USE CASES: 
   #    6) Current opened design, has components, but diff names, design_name exists in project.
   #    7) No opened design, design_name exists in project.

   set errMsg "Design <$design_name> already exists in your project, please set the variable <design_name> to another value."
   set nRet 2

} else {
   # USE CASES:
   #    8) No opened design, design_name not in project.
   #    9) Current opened design, has components, but diff names, design_name not in project.

   common::send_gid_msg -ssname BD::TCL -id 2003 -severity "INFO" "Currently there is no design <$design_name> in project, so creating one..."

   create_bd_design $design_name

   common::send_gid_msg -ssname BD::TCL -id 2004 -severity "INFO" "Making design <$design_name> as current_bd_design."
   current_bd_design $design_name

}

common::send_gid_msg -ssname BD::TCL -id 2005 -severity "INFO" "Currently the variable <design_name> is equal to \"$design_name\"."

if { $nRet != 0 } {
   catch {common::send_gid_msg -ssname BD::TCL -id 2006 -severity "ERROR" $errMsg}
   return $nRet
}

set bCheckIPsPassed 1
##################################################################
# CHECK IPs
##################################################################
set bCheckIPs 1
if { $bCheckIPs == 1 } {
   set list_check_ips "\ 
xilinx.com:ip:zynq_ultra_ps_e:3.5\
xilinx.com:ip:axi_gpio:2.0\
xilinx.com:ip:smartconnect:1.0\
xilinx.com:ip:xlconcat:2.1\
xilinx.com:ip:xlslice:1.0\
xilinx.com:ip:axi_dma:7.1\
xilinx.com:ip:axis_data_fifo:2.0\
xilinx.com:ip:aurora_64b66b:12.0\
xilinx.com:ip:proc_sys_reset:5.0\
xilinx.com:ip:dfx_decoupler:1.0\
xilinx.com:hls:conv_pool_I:1.0\
"

   set list_ips_missing ""
   common::send_gid_msg -ssname BD::TCL -id 2011 -severity "INFO" "Checking if the following IPs exist in the project's IP catalog: $list_check_ips ."

   foreach ip_vlnv $list_check_ips {
      set ip_obj [get_ipdefs -all $ip_vlnv]
      if { $ip_obj eq "" } {
         lappend list_ips_missing $ip_vlnv
      }
   }

   if { $list_ips_missing ne "" } {
      catch {common::send_gid_msg -ssname BD::TCL -id 2012 -severity "ERROR" "The following IPs are not found in the IP Catalog:\n  $list_ips_missing\n\nResolution: Please add the repository containing the IP(s) to the project." }
      set bCheckIPsPassed 0
   }

}

if { $bCheckIPsPassed != 1 } {
  common::send_gid_msg -ssname BD::TCL -id 2023 -severity "WARNING" "Will not continue with creation of design due to the error(s) above."
  return 3
}

##################################################################
# DESIGN PROCs
##################################################################


# Hierarchical cell: slot_8
proc create_hier_cell_slot_8 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_8() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_7
proc create_hier_cell_slot_7 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_7() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_6
proc create_hier_cell_slot_6 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_6() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_5
proc create_hier_cell_slot_5 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_5() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_4
proc create_hier_cell_slot_4 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_4() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_3
proc create_hier_cell_slot_3 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_3() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_2
proc create_hier_cell_slot_2 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_2() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net Conn3 [get_bd_intf_pins dfx_decoupler_0/rp_intf_0] [get_bd_intf_pins rp_intf_0]
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}

# Hierarchical cell: slot_1
proc create_hier_cell_slot_1 { parentCell nameHier } {

  variable script_folder

  if { $parentCell eq "" || $nameHier eq "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2092 -severity "ERROR" "create_hier_cell_slot_1() - Empty argument(s)!"}
     return
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj

  # Create cell and set as current instance
  set hier_obj [create_bd_cell -type hier $nameHier]
  current_bd_instance $hier_obj

  # Create interface pins
  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_control

  create_bd_intf_pin -mode Slave -vlnv xilinx.com:interface:aximm_rtl:1.0 s_axi_ctrl

  create_bd_intf_pin -mode Master -vlnv xilinx.com:interface:aximm_rtl:1.0 rp_intf_0


  # Create pins
  create_bd_pin -dir O decouple_status
  create_bd_pin -dir I decouple
  create_bd_pin -dir I -type clk ap_clk
  create_bd_pin -dir I -type rst ap_rst_n

  # Create instance: dfx_decoupler_0, and set properties
  set dfx_decoupler_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:dfx_decoupler:1.0 dfx_decoupler_0 ]
  set_property -dict [list \
    CONFIG.ALL_PARAMS {ALWAYS_HAVE_AXI_CLK 1 INTF {intf_0 {ID 0 VLNV xilinx.com:interface:aximm_rtl:1.0 MODE slave PROTOCOL AXI4 SIGNALS {ARVALID {WIDTH 1 PRESENT 1} ARREADY {WIDTH 1 PRESENT 1} AWVALID\
{WIDTH 1 PRESENT 1} AWREADY {WIDTH 1 PRESENT 1} BVALID {WIDTH 1 PRESENT 1} BREADY {WIDTH 1 PRESENT 1} RVALID {WIDTH 1 PRESENT 1} RREADY {WIDTH 1 PRESENT 1} WVALID {WIDTH 1 PRESENT 1} WREADY {WIDTH 1 PRESENT\
1} AWID {WIDTH 1 PRESENT 1} AWADDR {WIDTH 64 PRESENT 1} AWLEN {WIDTH 8 PRESENT 1} AWSIZE {WIDTH 3 PRESENT 1} AWBURST {WIDTH 2 PRESENT 0} AWLOCK {WIDTH 1 PRESENT 1} AWCACHE {WIDTH 4 PRESENT 1} AWPROT {WIDTH\
3 PRESENT 1} AWREGION {WIDTH 4 PRESENT 1} AWQOS {WIDTH 4 PRESENT 1} AWUSER {WIDTH 0 PRESENT 0} WID {WIDTH 1 PRESENT 1} WDATA {WIDTH 64 PRESENT 1} WSTRB {WIDTH 8 PRESENT 1} WLAST {WIDTH 1 PRESENT 1} WUSER\
{WIDTH 0 PRESENT 0} BID {WIDTH 1 PRESENT 1} BRESP {WIDTH 2 PRESENT 1} BUSER {WIDTH 0 PRESENT 0} ARID {WIDTH 1 PRESENT 1} ARADDR {WIDTH 64 PRESENT 1} ARLEN {WIDTH 8 PRESENT 1} ARSIZE {WIDTH 3 PRESENT 1}\
ARBURST {WIDTH 2 PRESENT 0} ARLOCK {WIDTH 1 PRESENT 1} ARCACHE {WIDTH 4 PRESENT 1} ARPROT {WIDTH 3 PRESENT 1} ARREGION {WIDTH 4 PRESENT 1} ARQOS {WIDTH 4 PRESENT 1} ARUSER {WIDTH 0 PRESENT 0} RID {WIDTH\
1 PRESENT 1} RDATA {WIDTH 64 PRESENT 1} RRESP {WIDTH 2 PRESENT 1} RLAST {WIDTH 1 PRESENT 1} RUSER {WIDTH 0 PRESENT 0}}}}} \
    CONFIG.GUI_SELECT_MODE {slave} \
    CONFIG.GUI_SELECT_VLNV {xilinx.com:interface:aximm_rtl:1.0} \
  ] $dfx_decoupler_0


  # Create instance: conv_pool_I_0, and set properties
  set conv_pool_I_0 [ create_bd_cell -type ip -vlnv xilinx.com:hls:conv_pool_I:1.0 conv_pool_I_0 ]

  # Create interface connections
  connect_bd_intf_net -intf_net conv_pool_I_0_m_axi_data [get_bd_intf_pins conv_pool_I_0/m_axi_data] [get_bd_intf_pins dfx_decoupler_0/s_intf_0]
  connect_bd_intf_net -intf_net dfx_decoupler_0_rp_intf_0 [get_bd_intf_pins rp_intf_0] [get_bd_intf_pins dfx_decoupler_0/rp_intf_0]
  connect_bd_intf_net -intf_net s_axi_control_1 [get_bd_intf_pins s_axi_control] [get_bd_intf_pins conv_pool_I_0/s_axi_control]
  connect_bd_intf_net -intf_net s_axi_ctrl_1 [get_bd_intf_pins s_axi_ctrl] [get_bd_intf_pins conv_pool_I_0/s_axi_ctrl]

  # Create port connections
  connect_bd_net -net ap_clk_1 [get_bd_pins ap_clk] [get_bd_pins dfx_decoupler_0/intf_0_aclk] [get_bd_pins conv_pool_I_0/ap_clk]
  connect_bd_net -net ap_rst_n_1 [get_bd_pins ap_rst_n] [get_bd_pins dfx_decoupler_0/intf_0_arstn] [get_bd_pins conv_pool_I_0/ap_rst_n]
  connect_bd_net -net decouple_1 [get_bd_pins decouple] [get_bd_pins dfx_decoupler_0/decouple]
  connect_bd_net -net dfx_decoupler_0_decouple_status [get_bd_pins dfx_decoupler_0/decouple_status] [get_bd_pins decouple_status]

  # Restore current instance
  current_bd_instance $oldCurInst
}


# Procedure to create entire design; Provide argument to make
# procedure reusable. If parentCell is "", will use root.
proc create_root_design { parentCell } {

  variable script_folder
  variable design_name

  if { $parentCell eq "" } {
     set parentCell [get_bd_cells /]
  }

  # Get object for parentCell
  set parentObj [get_bd_cells $parentCell]
  if { $parentObj == "" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2090 -severity "ERROR" "Unable to find parent cell <$parentCell>!"}
     return
  }

  # Make sure parentObj is hier blk
  set parentType [get_property TYPE $parentObj]
  if { $parentType ne "hier" } {
     catch {common::send_gid_msg -ssname BD::TCL -id 2091 -severity "ERROR" "Parent <$parentObj> has TYPE = <$parentType>. Expected to be <hier>."}
     return
  }

  # Save current instance; Restore later
  set oldCurInst [current_bd_instance .]

  # Set parent object as current
  current_bd_instance $parentObj


  # Create interface ports
  set GT_SERIAL_TX_0 [ create_bd_intf_port -mode Master -vlnv xilinx.com:display_aurora:GT_Serial_Transceiver_Pins_TX_rtl:1.0 GT_SERIAL_TX_0 ]

  set GT_DIFF_REFCLK1_0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:interface:diff_clock_rtl:1.0 GT_DIFF_REFCLK1_0 ]
  set_property -dict [ list \
   CONFIG.FREQ_HZ {156250000} \
   ] $GT_DIFF_REFCLK1_0

  set GT_SERIAL_RX_0 [ create_bd_intf_port -mode Slave -vlnv xilinx.com:display_aurora:GT_Serial_Transceiver_Pins_RX_rtl:1.0 GT_SERIAL_RX_0 ]


  # Create ports

  # Create instance: zynq_ultra_ps_e_0, and set properties
  set zynq_ultra_ps_e_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:zynq_ultra_ps_e:3.5 zynq_ultra_ps_e_0 ]
  set_property -dict [list \
    CONFIG.PSU_BANK_0_IO_STANDARD {LVCMOS18} \
    CONFIG.PSU_BANK_1_IO_STANDARD {LVCMOS18} \
    CONFIG.PSU_BANK_2_IO_STANDARD {LVCMOS18} \
    CONFIG.PSU_DDR_RAM_HIGHADDR {0xFFFFFFFF} \
    CONFIG.PSU_DDR_RAM_HIGHADDR_OFFSET {0x800000000} \
    CONFIG.PSU_DDR_RAM_LOWADDR_OFFSET {0x80000000} \
    CONFIG.PSU_DYNAMIC_DDR_CONFIG_EN {1} \
    CONFIG.PSU_MIO_13_POLARITY {Default} \
    CONFIG.PSU_MIO_20_POLARITY {Default} \
    CONFIG.PSU_MIO_21_POLARITY {Default} \
    CONFIG.PSU_MIO_22_INPUT_TYPE {cmos} \
    CONFIG.PSU_MIO_22_POLARITY {Default} \
    CONFIG.PSU_MIO_23_POLARITY {Default} \
    CONFIG.PSU_MIO_24_POLARITY {Default} \
    CONFIG.PSU_MIO_25_POLARITY {Default} \
    CONFIG.PSU_MIO_26_POLARITY {Default} \
    CONFIG.PSU_MIO_27_POLARITY {Default} \
    CONFIG.PSU_MIO_28_POLARITY {Default} \
    CONFIG.PSU_MIO_29_POLARITY {Default} \
    CONFIG.PSU_MIO_30_POLARITY {Default} \
    CONFIG.PSU_MIO_31_POLARITY {Default} \
    CONFIG.PSU_MIO_38_POLARITY {Default} \
    CONFIG.PSU_MIO_43_POLARITY {Default} \
    CONFIG.PSU_MIO_44_POLARITY {Default} \
    CONFIG.PSU_MIO_TREE_PERIPHERALS {Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Feedback Clk#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad SPI Flash#Quad\
SPI Flash#Quad SPI Flash#GPIO0 MIO#I2C 0#I2C 0#I2C 1#I2C 1#UART 0#UART 0#GPIO0 MIO#GPIO0 MIO#GPIO0 MIO#GPIO0 MIO#GPIO0 MIO#GPIO0 MIO#GPIO1 MIO#GPIO1 MIO#GPIO1 MIO#GPIO1 MIO#GPIO1 MIO#GPIO1 MIO#PMU GPO\
0#PMU GPO 1#PMU GPO 2#PMU GPO 3#PMU GPO 4#PMU GPO 5#GPIO1 MIO#SD 1#SD 1#SD 1#SD 1#GPIO1 MIO#GPIO1 MIO#SD 1#SD 1#SD 1#SD 1#SD 1#SD 1#SD 1#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB 0#USB\
0#USB 0#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#Gem 3#MDIO 3#MDIO 3} \
    CONFIG.PSU_MIO_TREE_SIGNALS {sclk_out#miso_mo1#mo2#mo3#mosi_mi0#n_ss_out#clk_for_lpbk#n_ss_out_upper#mo_upper[0]#mo_upper[1]#mo_upper[2]#mo_upper[3]#sclk_out_upper#gpio0[13]#scl_out#sda_out#scl_out#sda_out#rxd#txd#gpio0[20]#gpio0[21]#gpio0[22]#gpio0[23]#gpio0[24]#gpio0[25]#gpio1[26]#gpio1[27]#gpio1[28]#gpio1[29]#gpio1[30]#gpio1[31]#gpo[0]#gpo[1]#gpo[2]#gpo[3]#gpo[4]#gpo[5]#gpio1[38]#sdio1_data_out[4]#sdio1_data_out[5]#sdio1_data_out[6]#sdio1_data_out[7]#gpio1[43]#gpio1[44]#sdio1_cd_n#sdio1_data_out[0]#sdio1_data_out[1]#sdio1_data_out[2]#sdio1_data_out[3]#sdio1_cmd_out#sdio1_clk_out#ulpi_clk_in#ulpi_dir#ulpi_tx_data[2]#ulpi_nxt#ulpi_tx_data[0]#ulpi_tx_data[1]#ulpi_stp#ulpi_tx_data[3]#ulpi_tx_data[4]#ulpi_tx_data[5]#ulpi_tx_data[6]#ulpi_tx_data[7]#rgmii_tx_clk#rgmii_txd[0]#rgmii_txd[1]#rgmii_txd[2]#rgmii_txd[3]#rgmii_tx_ctl#rgmii_rx_clk#rgmii_rxd[0]#rgmii_rxd[1]#rgmii_rxd[2]#rgmii_rxd[3]#rgmii_rx_ctl#gem3_mdc#gem3_mdio_out}\
\
    CONFIG.PSU_SD1_INTERNAL_BUS_WIDTH {8} \
    CONFIG.PSU_USB3__DUAL_CLOCK_ENABLE {1} \
    CONFIG.PSU__ACT_DDR_FREQ_MHZ {1049.989502} \
    CONFIG.PSU__CAN1__PERIPHERAL__ENABLE {0} \
    CONFIG.PSU__CRF_APB__ACPU_CTRL__ACT_FREQMHZ {1199.988037} \
    CONFIG.PSU__CRF_APB__ACPU_CTRL__FREQMHZ {1200} \
    CONFIG.PSU__CRF_APB__ACPU_CTRL__SRCSEL {APLL} \
    CONFIG.PSU__CRF_APB__APLL_CTRL__SRCSEL {PSS_REF_CLK} \
    CONFIG.PSU__CRF_APB__DBG_FPD_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRF_APB__DBG_FPD_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRF_APB__DBG_FPD_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRF_APB__DBG_TRACE_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRF_APB__DBG_TRACE_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRF_APB__DBG_TSTMP_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRF_APB__DBG_TSTMP_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRF_APB__DBG_TSTMP_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRF_APB__DDR_CTRL__ACT_FREQMHZ {524.994751} \
    CONFIG.PSU__CRF_APB__DDR_CTRL__FREQMHZ {1066} \
    CONFIG.PSU__CRF_APB__DDR_CTRL__SRCSEL {DPLL} \
    CONFIG.PSU__CRF_APB__DPDMA_REF_CTRL__ACT_FREQMHZ {599.994019} \
    CONFIG.PSU__CRF_APB__DPDMA_REF_CTRL__FREQMHZ {600} \
    CONFIG.PSU__CRF_APB__DPDMA_REF_CTRL__SRCSEL {APLL} \
    CONFIG.PSU__CRF_APB__DPLL_CTRL__SRCSEL {PSS_REF_CLK} \
    CONFIG.PSU__CRF_APB__DP_AUDIO_REF_CTRL__SRCSEL {RPLL} \
    CONFIG.PSU__CRF_APB__DP_STC_REF_CTRL__SRCSEL {RPLL} \
    CONFIG.PSU__CRF_APB__DP_VIDEO_REF_CTRL__SRCSEL {VPLL} \
    CONFIG.PSU__CRF_APB__GDMA_REF_CTRL__ACT_FREQMHZ {599.994019} \
    CONFIG.PSU__CRF_APB__GDMA_REF_CTRL__FREQMHZ {600} \
    CONFIG.PSU__CRF_APB__GDMA_REF_CTRL__SRCSEL {APLL} \
    CONFIG.PSU__CRF_APB__GPU_REF_CTRL__FREQMHZ {500} \
    CONFIG.PSU__CRF_APB__GPU_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRF_APB__SATA_REF_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRF_APB__SATA_REF_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRF_APB__SATA_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRF_APB__TOPSW_LSBUS_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRF_APB__TOPSW_LSBUS_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRF_APB__TOPSW_LSBUS_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRF_APB__TOPSW_MAIN_CTRL__ACT_FREQMHZ {524.994751} \
    CONFIG.PSU__CRF_APB__TOPSW_MAIN_CTRL__FREQMHZ {533.33} \
    CONFIG.PSU__CRF_APB__TOPSW_MAIN_CTRL__SRCSEL {DPLL} \
    CONFIG.PSU__CRF_APB__VPLL_CTRL__SRCSEL {PSS_REF_CLK} \
    CONFIG.PSU__CRL_APB__ADMA_REF_CTRL__ACT_FREQMHZ {499.994995} \
    CONFIG.PSU__CRL_APB__ADMA_REF_CTRL__FREQMHZ {500} \
    CONFIG.PSU__CRL_APB__ADMA_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__AMS_REF_CTRL__ACT_FREQMHZ {49.999500} \
    CONFIG.PSU__CRL_APB__CAN1_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__CAN1_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__CPU_R5_CTRL__ACT_FREQMHZ {499.994995} \
    CONFIG.PSU__CRL_APB__CPU_R5_CTRL__FREQMHZ {500} \
    CONFIG.PSU__CRL_APB__CPU_R5_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__DBG_LPD_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRL_APB__DBG_LPD_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRL_APB__DBG_LPD_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__DLL_REF_CTRL__ACT_FREQMHZ {1499.984985} \
    CONFIG.PSU__CRL_APB__GEM3_REF_CTRL__ACT_FREQMHZ {124.998749} \
    CONFIG.PSU__CRL_APB__GEM3_REF_CTRL__FREQMHZ {125} \
    CONFIG.PSU__CRL_APB__GEM3_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__GEM_TSU_REF_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRL_APB__GEM_TSU_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__I2C0_REF_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRL_APB__I2C0_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__I2C0_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__I2C1_REF_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRL_APB__I2C1_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__I2C1_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__IOPLL_CTRL__SRCSEL {PSS_REF_CLK} \
    CONFIG.PSU__CRL_APB__IOU_SWITCH_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRL_APB__IOU_SWITCH_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRL_APB__IOU_SWITCH_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__LPD_LSBUS_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRL_APB__LPD_LSBUS_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__LPD_LSBUS_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__LPD_SWITCH_CTRL__ACT_FREQMHZ {499.994995} \
    CONFIG.PSU__CRL_APB__LPD_SWITCH_CTRL__FREQMHZ {500} \
    CONFIG.PSU__CRL_APB__LPD_SWITCH_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__PCAP_CTRL__ACT_FREQMHZ {187.498123} \
    CONFIG.PSU__CRL_APB__PCAP_CTRL__FREQMHZ {200} \
    CONFIG.PSU__CRL_APB__PCAP_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__PL0_REF_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRL_APB__PL0_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__PL0_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__QSPI_REF_CTRL__ACT_FREQMHZ {124.998749} \
    CONFIG.PSU__CRL_APB__QSPI_REF_CTRL__FREQMHZ {125} \
    CONFIG.PSU__CRL_APB__QSPI_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__RPLL_CTRL__SRCSEL {PSS_REF_CLK} \
    CONFIG.PSU__CRL_APB__SDIO1_REF_CTRL__ACT_FREQMHZ {187.498123} \
    CONFIG.PSU__CRL_APB__SDIO1_REF_CTRL__FREQMHZ {200} \
    CONFIG.PSU__CRL_APB__SDIO1_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__TIMESTAMP_REF_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRL_APB__TIMESTAMP_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__TIMESTAMP_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__UART0_REF_CTRL__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__CRL_APB__UART0_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__UART0_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__UART1_REF_CTRL__FREQMHZ {100} \
    CONFIG.PSU__CRL_APB__UART1_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__USB0_BUS_REF_CTRL__ACT_FREQMHZ {249.997498} \
    CONFIG.PSU__CRL_APB__USB0_BUS_REF_CTRL__FREQMHZ {250} \
    CONFIG.PSU__CRL_APB__USB0_BUS_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__USB3_DUAL_REF_CTRL__ACT_FREQMHZ {19.999800} \
    CONFIG.PSU__CRL_APB__USB3_DUAL_REF_CTRL__FREQMHZ {20} \
    CONFIG.PSU__CRL_APB__USB3_DUAL_REF_CTRL__SRCSEL {IOPLL} \
    CONFIG.PSU__CRL_APB__USB3__ENABLE {1} \
    CONFIG.PSU__CSUPMU__PERIPHERAL__VALID {1} \
    CONFIG.PSU__DDRC__BG_ADDR_COUNT {1} \
    CONFIG.PSU__DDRC__BRC_MAPPING {ROW_BANK_COL} \
    CONFIG.PSU__DDRC__BUS_WIDTH {64 Bit} \
    CONFIG.PSU__DDRC__CL {15} \
    CONFIG.PSU__DDRC__CLOCK_STOP_EN {0} \
    CONFIG.PSU__DDRC__COMPONENTS {UDIMM} \
    CONFIG.PSU__DDRC__CWL {11} \
    CONFIG.PSU__DDRC__DDR4_ADDR_MAPPING {0} \
    CONFIG.PSU__DDRC__DDR4_CAL_MODE_ENABLE {0} \
    CONFIG.PSU__DDRC__DDR4_CRC_CONTROL {0} \
    CONFIG.PSU__DDRC__DDR4_T_REF_MODE {0} \
    CONFIG.PSU__DDRC__DDR4_T_REF_RANGE {Normal (0-85)} \
    CONFIG.PSU__DDRC__DEVICE_CAPACITY {8192 MBits} \
    CONFIG.PSU__DDRC__DM_DBI {DM_NO_DBI} \
    CONFIG.PSU__DDRC__DRAM_WIDTH {16 Bits} \
    CONFIG.PSU__DDRC__ECC {Disabled} \
    CONFIG.PSU__DDRC__FGRM {1X} \
    CONFIG.PSU__DDRC__LP_ASR {manual normal} \
    CONFIG.PSU__DDRC__MEMORY_TYPE {DDR 4} \
    CONFIG.PSU__DDRC__PARITY_ENABLE {0} \
    CONFIG.PSU__DDRC__PER_BANK_REFRESH {0} \
    CONFIG.PSU__DDRC__PHY_DBI_MODE {0} \
    CONFIG.PSU__DDRC__RANK_ADDR_COUNT {0} \
    CONFIG.PSU__DDRC__ROW_ADDR_COUNT {16} \
    CONFIG.PSU__DDRC__SELF_REF_ABORT {0} \
    CONFIG.PSU__DDRC__SPEED_BIN {DDR4_2133P} \
    CONFIG.PSU__DDRC__STATIC_RD_MODE {0} \
    CONFIG.PSU__DDRC__TRAIN_DATA_EYE {1} \
    CONFIG.PSU__DDRC__TRAIN_READ_GATE {1} \
    CONFIG.PSU__DDRC__TRAIN_WRITE_LEVEL {1} \
    CONFIG.PSU__DDRC__T_FAW {30.0} \
    CONFIG.PSU__DDRC__T_RAS_MIN {33} \
    CONFIG.PSU__DDRC__T_RC {46.5} \
    CONFIG.PSU__DDRC__T_RCD {15} \
    CONFIG.PSU__DDRC__T_RP {15} \
    CONFIG.PSU__DDRC__VREF {1} \
    CONFIG.PSU__DDR_HIGH_ADDRESS_GUI_ENABLE {1} \
    CONFIG.PSU__DDR__INTERFACE__FREQMHZ {533.000} \
    CONFIG.PSU__DLL__ISUSED {1} \
    CONFIG.PSU__ENET3__FIFO__ENABLE {0} \
    CONFIG.PSU__ENET3__GRP_MDIO__ENABLE {1} \
    CONFIG.PSU__ENET3__GRP_MDIO__IO {MIO 76 .. 77} \
    CONFIG.PSU__ENET3__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__ENET3__PERIPHERAL__IO {MIO 64 .. 75} \
    CONFIG.PSU__ENET3__PTP__ENABLE {0} \
    CONFIG.PSU__ENET3__TSU__ENABLE {0} \
    CONFIG.PSU__FPDMASTERS_COHERENCY {0} \
    CONFIG.PSU__FPD_SLCR__WDT1__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__FPGA_PL0_ENABLE {1} \
    CONFIG.PSU__GEM3_COHERENCY {0} \
    CONFIG.PSU__GEM3_ROUTE_THROUGH_FPD {0} \
    CONFIG.PSU__GEM__TSU__ENABLE {0} \
    CONFIG.PSU__GPIO0_MIO__IO {MIO 0 .. 25} \
    CONFIG.PSU__GPIO0_MIO__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__GPIO1_MIO__IO {MIO 26 .. 51} \
    CONFIG.PSU__GPIO1_MIO__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__I2C0__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__I2C0__PERIPHERAL__IO {MIO 14 .. 15} \
    CONFIG.PSU__I2C1__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__I2C1__PERIPHERAL__IO {MIO 16 .. 17} \
    CONFIG.PSU__IOU_SLCR__IOU_TTC_APB_CLK__TTC0_SEL {APB} \
    CONFIG.PSU__IOU_SLCR__IOU_TTC_APB_CLK__TTC1_SEL {APB} \
    CONFIG.PSU__IOU_SLCR__IOU_TTC_APB_CLK__TTC2_SEL {APB} \
    CONFIG.PSU__IOU_SLCR__IOU_TTC_APB_CLK__TTC3_SEL {APB} \
    CONFIG.PSU__IOU_SLCR__TTC0__ACT_FREQMHZ {100.000000} \
    CONFIG.PSU__IOU_SLCR__TTC1__ACT_FREQMHZ {100.000000} \
    CONFIG.PSU__IOU_SLCR__TTC2__ACT_FREQMHZ {100.000000} \
    CONFIG.PSU__IOU_SLCR__TTC3__ACT_FREQMHZ {100.000000} \
    CONFIG.PSU__IOU_SLCR__WDT0__ACT_FREQMHZ {99.999001} \
    CONFIG.PSU__LPD_SLCR__CSUPMU__ACT_FREQMHZ {100.000000} \
    CONFIG.PSU__MAXIGP0__DATA_WIDTH {128} \
    CONFIG.PSU__MAXIGP1__DATA_WIDTH {128} \
    CONFIG.PSU__OVERRIDE__BASIC_CLOCK {0} \
    CONFIG.PSU__PL_CLK0_BUF {TRUE} \
    CONFIG.PSU__PMU_COHERENCY {0} \
    CONFIG.PSU__PMU__AIBACK__ENABLE {0} \
    CONFIG.PSU__PMU__EMIO_GPI__ENABLE {0} \
    CONFIG.PSU__PMU__EMIO_GPO__ENABLE {0} \
    CONFIG.PSU__PMU__GPI0__ENABLE {0} \
    CONFIG.PSU__PMU__GPI1__ENABLE {0} \
    CONFIG.PSU__PMU__GPI2__ENABLE {0} \
    CONFIG.PSU__PMU__GPI3__ENABLE {0} \
    CONFIG.PSU__PMU__GPI4__ENABLE {0} \
    CONFIG.PSU__PMU__GPI5__ENABLE {0} \
    CONFIG.PSU__PMU__GPO0__ENABLE {1} \
    CONFIG.PSU__PMU__GPO0__IO {MIO 32} \
    CONFIG.PSU__PMU__GPO1__ENABLE {1} \
    CONFIG.PSU__PMU__GPO1__IO {MIO 33} \
    CONFIG.PSU__PMU__GPO2__ENABLE {1} \
    CONFIG.PSU__PMU__GPO2__IO {MIO 34} \
    CONFIG.PSU__PMU__GPO2__POLARITY {low} \
    CONFIG.PSU__PMU__GPO3__ENABLE {1} \
    CONFIG.PSU__PMU__GPO3__IO {MIO 35} \
    CONFIG.PSU__PMU__GPO3__POLARITY {low} \
    CONFIG.PSU__PMU__GPO4__ENABLE {1} \
    CONFIG.PSU__PMU__GPO4__IO {MIO 36} \
    CONFIG.PSU__PMU__GPO4__POLARITY {low} \
    CONFIG.PSU__PMU__GPO5__ENABLE {1} \
    CONFIG.PSU__PMU__GPO5__IO {MIO 37} \
    CONFIG.PSU__PMU__GPO5__POLARITY {low} \
    CONFIG.PSU__PMU__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__PMU__PLERROR__ENABLE {0} \
    CONFIG.PSU__PRESET_APPLIED {1} \
    CONFIG.PSU__PROTECTION__MASTERS {USB1:NonSecure;0|USB0:NonSecure;1|S_AXI_LPD:NA;0|S_AXI_HPC1_FPD:NA;0|S_AXI_HPC0_FPD:NA;0|S_AXI_HP3_FPD:NA;0|S_AXI_HP2_FPD:NA;0|S_AXI_HP1_FPD:NA;1|S_AXI_HP0_FPD:NA;1|S_AXI_ACP:NA;0|S_AXI_ACE:NA;0|SD1:NonSecure;1|SD0:NonSecure;0|SATA1:NonSecure;1|SATA0:NonSecure;1|RPU1:Secure;1|RPU0:Secure;1|QSPI:NonSecure;1|PMU:NA;1|PCIe:NonSecure;0|NAND:NonSecure;0|LDMA:NonSecure;1|GPU:NonSecure;1|GEM3:NonSecure;1|GEM2:NonSecure;0|GEM1:NonSecure;0|GEM0:NonSecure;0|FDMA:NonSecure;1|DP:NonSecure;0|DAP:NA;1|Coresight:NA;1|CSU:NA;1|APU:NA;1}\
\
    CONFIG.PSU__PROTECTION__SLAVES {LPD;USB3_1_XHCI;FE300000;FE3FFFFF;0|LPD;USB3_1;FF9E0000;FF9EFFFF;0|LPD;USB3_0_XHCI;FE200000;FE2FFFFF;1|LPD;USB3_0;FF9D0000;FF9DFFFF;1|LPD;UART1;FF010000;FF01FFFF;0|LPD;UART0;FF000000;FF00FFFF;1|LPD;TTC3;FF140000;FF14FFFF;1|LPD;TTC2;FF130000;FF13FFFF;1|LPD;TTC1;FF120000;FF12FFFF;1|LPD;TTC0;FF110000;FF11FFFF;1|FPD;SWDT1;FD4D0000;FD4DFFFF;1|LPD;SWDT0;FF150000;FF15FFFF;1|LPD;SPI1;FF050000;FF05FFFF;0|LPD;SPI0;FF040000;FF04FFFF;0|FPD;SMMU_REG;FD5F0000;FD5FFFFF;1|FPD;SMMU;FD800000;FDFFFFFF;1|FPD;SIOU;FD3D0000;FD3DFFFF;1|FPD;SERDES;FD400000;FD47FFFF;1|LPD;SD1;FF170000;FF17FFFF;1|LPD;SD0;FF160000;FF16FFFF;0|FPD;SATA;FD0C0000;FD0CFFFF;1|LPD;RTC;FFA60000;FFA6FFFF;1|LPD;RSA_CORE;FFCE0000;FFCEFFFF;1|LPD;RPU;FF9A0000;FF9AFFFF;1|LPD;R5_TCM_RAM_GLOBAL;FFE00000;FFE3FFFF;1|LPD;R5_1_Instruction_Cache;FFEC0000;FFECFFFF;1|LPD;R5_1_Data_Cache;FFED0000;FFEDFFFF;1|LPD;R5_1_BTCM_GLOBAL;FFEB0000;FFEBFFFF;1|LPD;R5_1_ATCM_GLOBAL;FFE90000;FFE9FFFF;1|LPD;R5_0_Instruction_Cache;FFE40000;FFE4FFFF;1|LPD;R5_0_Data_Cache;FFE50000;FFE5FFFF;1|LPD;R5_0_BTCM_GLOBAL;FFE20000;FFE2FFFF;1|LPD;R5_0_ATCM_GLOBAL;FFE00000;FFE0FFFF;1|LPD;QSPI_Linear_Address;C0000000;DFFFFFFF;1|LPD;QSPI;FF0F0000;FF0FFFFF;1|LPD;PMU_RAM;FFDC0000;FFDDFFFF;1|LPD;PMU_GLOBAL;FFD80000;FFDBFFFF;1|FPD;PCIE_MAIN;FD0E0000;FD0EFFFF;0|FPD;PCIE_LOW;E0000000;EFFFFFFF;0|FPD;PCIE_HIGH2;8000000000;BFFFFFFFFF;0|FPD;PCIE_HIGH1;600000000;7FFFFFFFF;0|FPD;PCIE_DMA;FD0F0000;FD0FFFFF;0|FPD;PCIE_ATTRIB;FD480000;FD48FFFF;0|LPD;OCM_XMPU_CFG;FFA70000;FFA7FFFF;1|LPD;OCM_SLCR;FF960000;FF96FFFF;1|OCM;OCM;FFFC0000;FFFFFFFF;1|LPD;NAND;FF100000;FF10FFFF;0|LPD;MBISTJTAG;FFCF0000;FFCFFFFF;1|LPD;LPD_XPPU_SINK;FF9C0000;FF9CFFFF;1|LPD;LPD_XPPU;FF980000;FF98FFFF;1|LPD;LPD_SLCR_SECURE;FF4B0000;FF4DFFFF;1|LPD;LPD_SLCR;FF410000;FF4AFFFF;1|LPD;LPD_GPV;FE100000;FE1FFFFF;1|LPD;LPD_DMA_7;FFAF0000;FFAFFFFF;1|LPD;LPD_DMA_6;FFAE0000;FFAEFFFF;1|LPD;LPD_DMA_5;FFAD0000;FFADFFFF;1|LPD;LPD_DMA_4;FFAC0000;FFACFFFF;1|LPD;LPD_DMA_3;FFAB0000;FFABFFFF;1|LPD;LPD_DMA_2;FFAA0000;FFAAFFFF;1|LPD;LPD_DMA_1;FFA90000;FFA9FFFF;1|LPD;LPD_DMA_0;FFA80000;FFA8FFFF;1|LPD;IPI_CTRL;FF380000;FF3FFFFF;1|LPD;IOU_SLCR;FF180000;FF23FFFF;1|LPD;IOU_SECURE_SLCR;FF240000;FF24FFFF;1|LPD;IOU_SCNTRS;FF260000;FF26FFFF;1|LPD;IOU_SCNTR;FF250000;FF25FFFF;1|LPD;IOU_GPV;FE000000;FE0FFFFF;1|LPD;I2C1;FF030000;FF03FFFF;1|LPD;I2C0;FF020000;FF02FFFF;1|FPD;GPU;FD4B0000;FD4BFFFF;0|LPD;GPIO;FF0A0000;FF0AFFFF;1|LPD;GEM3;FF0E0000;FF0EFFFF;1|LPD;GEM2;FF0D0000;FF0DFFFF;0|LPD;GEM1;FF0C0000;FF0CFFFF;0|LPD;GEM0;FF0B0000;FF0BFFFF;0|FPD;FPD_XMPU_SINK;FD4F0000;FD4FFFFF;1|FPD;FPD_XMPU_CFG;FD5D0000;FD5DFFFF;1|FPD;FPD_SLCR_SECURE;FD690000;FD6CFFFF;1|FPD;FPD_SLCR;FD610000;FD68FFFF;1|FPD;FPD_DMA_CH7;FD570000;FD57FFFF;1|FPD;FPD_DMA_CH6;FD560000;FD56FFFF;1|FPD;FPD_DMA_CH5;FD550000;FD55FFFF;1|FPD;FPD_DMA_CH4;FD540000;FD54FFFF;1|FPD;FPD_DMA_CH3;FD530000;FD53FFFF;1|FPD;FPD_DMA_CH2;FD520000;FD52FFFF;1|FPD;FPD_DMA_CH1;FD510000;FD51FFFF;1|FPD;FPD_DMA_CH0;FD500000;FD50FFFF;1|LPD;EFUSE;FFCC0000;FFCCFFFF;1|FPD;Display\
Port;FD4A0000;FD4AFFFF;0|FPD;DPDMA;FD4C0000;FD4CFFFF;0|FPD;DDR_XMPU5_CFG;FD050000;FD05FFFF;1|FPD;DDR_XMPU4_CFG;FD040000;FD04FFFF;1|FPD;DDR_XMPU3_CFG;FD030000;FD03FFFF;1|FPD;DDR_XMPU2_CFG;FD020000;FD02FFFF;1|FPD;DDR_XMPU1_CFG;FD010000;FD01FFFF;1|FPD;DDR_XMPU0_CFG;FD000000;FD00FFFF;1|FPD;DDR_QOS_CTRL;FD090000;FD09FFFF;1|FPD;DDR_PHY;FD080000;FD08FFFF;1|DDR;DDR_LOW;0;7FFFFFFF;1|DDR;DDR_HIGH;800000000;87FFFFFFF;1|FPD;DDDR_CTRL;FD070000;FD070FFF;1|LPD;Coresight;FE800000;FEFFFFFF;1|LPD;CSU_DMA;FFC80000;FFC9FFFF;1|LPD;CSU;FFCA0000;FFCAFFFF;1|LPD;CRL_APB;FF5E0000;FF85FFFF;1|FPD;CRF_APB;FD1A0000;FD2DFFFF;1|FPD;CCI_REG;FD5E0000;FD5EFFFF;1|LPD;CAN1;FF070000;FF07FFFF;0|LPD;CAN0;FF060000;FF06FFFF;0|FPD;APU;FD5C0000;FD5CFFFF;1|LPD;APM_INTC_IOU;FFA20000;FFA2FFFF;1|LPD;APM_FPD_LPD;FFA30000;FFA3FFFF;1|FPD;APM_5;FD490000;FD49FFFF;1|FPD;APM_0;FD0B0000;FD0BFFFF;1|LPD;APM2;FFA10000;FFA1FFFF;1|LPD;APM1;FFA00000;FFA0FFFF;1|LPD;AMS;FFA50000;FFA5FFFF;1|FPD;AFI_5;FD3B0000;FD3BFFFF;1|FPD;AFI_4;FD3A0000;FD3AFFFF;1|FPD;AFI_3;FD390000;FD39FFFF;1|FPD;AFI_2;FD380000;FD38FFFF;1|FPD;AFI_1;FD370000;FD37FFFF;1|FPD;AFI_0;FD360000;FD36FFFF;1|LPD;AFIFM6;FF9B0000;FF9BFFFF;1|FPD;ACPU_GIC;F9010000;F907FFFF;1}\
\
    CONFIG.PSU__PSS_REF_CLK__FREQMHZ {33.333} \
    CONFIG.PSU__QSPI_COHERENCY {0} \
    CONFIG.PSU__QSPI_ROUTE_THROUGH_FPD {0} \
    CONFIG.PSU__QSPI__GRP_FBCLK__ENABLE {1} \
    CONFIG.PSU__QSPI__GRP_FBCLK__IO {MIO 6} \
    CONFIG.PSU__QSPI__PERIPHERAL__DATA_MODE {x4} \
    CONFIG.PSU__QSPI__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__QSPI__PERIPHERAL__IO {MIO 0 .. 12} \
    CONFIG.PSU__QSPI__PERIPHERAL__MODE {Dual Parallel} \
    CONFIG.PSU__SATA__LANE0__ENABLE {0} \
    CONFIG.PSU__SATA__LANE1__IO {GT Lane3} \
    CONFIG.PSU__SATA__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__SATA__REF_CLK_FREQ {125} \
    CONFIG.PSU__SATA__REF_CLK_SEL {Ref Clk3} \
    CONFIG.PSU__SAXIGP2__DATA_WIDTH {128} \
    CONFIG.PSU__SAXIGP3__DATA_WIDTH {128} \
    CONFIG.PSU__SD1_COHERENCY {0} \
    CONFIG.PSU__SD1_ROUTE_THROUGH_FPD {0} \
    CONFIG.PSU__SD1__CLK_100_SDR_OTAP_DLY {0x3} \
    CONFIG.PSU__SD1__CLK_200_SDR_OTAP_DLY {0x3} \
    CONFIG.PSU__SD1__CLK_50_DDR_ITAP_DLY {0x3D} \
    CONFIG.PSU__SD1__CLK_50_DDR_OTAP_DLY {0x4} \
    CONFIG.PSU__SD1__CLK_50_SDR_ITAP_DLY {0x15} \
    CONFIG.PSU__SD1__CLK_50_SDR_OTAP_DLY {0x5} \
    CONFIG.PSU__SD1__DATA_TRANSFER_MODE {8Bit} \
    CONFIG.PSU__SD1__GRP_CD__ENABLE {1} \
    CONFIG.PSU__SD1__GRP_CD__IO {MIO 45} \
    CONFIG.PSU__SD1__GRP_POW__ENABLE {0} \
    CONFIG.PSU__SD1__GRP_WP__ENABLE {0} \
    CONFIG.PSU__SD1__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__SD1__PERIPHERAL__IO {MIO 39 .. 51} \
    CONFIG.PSU__SD1__SLOT_TYPE {SD 3.0} \
    CONFIG.PSU__SWDT0__CLOCK__ENABLE {0} \
    CONFIG.PSU__SWDT0__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__SWDT0__RESET__ENABLE {0} \
    CONFIG.PSU__SWDT1__CLOCK__ENABLE {0} \
    CONFIG.PSU__SWDT1__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__SWDT1__RESET__ENABLE {0} \
    CONFIG.PSU__TSU__BUFG_PORT_PAIR {0} \
    CONFIG.PSU__TTC0__CLOCK__ENABLE {0} \
    CONFIG.PSU__TTC0__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__TTC0__WAVEOUT__ENABLE {0} \
    CONFIG.PSU__TTC1__CLOCK__ENABLE {0} \
    CONFIG.PSU__TTC1__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__TTC1__WAVEOUT__ENABLE {0} \
    CONFIG.PSU__TTC2__CLOCK__ENABLE {0} \
    CONFIG.PSU__TTC2__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__TTC2__WAVEOUT__ENABLE {0} \
    CONFIG.PSU__TTC3__CLOCK__ENABLE {0} \
    CONFIG.PSU__TTC3__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__TTC3__WAVEOUT__ENABLE {0} \
    CONFIG.PSU__UART0__BAUD_RATE {115200} \
    CONFIG.PSU__UART0__MODEM__ENABLE {0} \
    CONFIG.PSU__UART0__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__UART0__PERIPHERAL__IO {MIO 18 .. 19} \
    CONFIG.PSU__USB0_COHERENCY {0} \
    CONFIG.PSU__USB0__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__USB0__PERIPHERAL__IO {MIO 52 .. 63} \
    CONFIG.PSU__USB0__REF_CLK_FREQ {26} \
    CONFIG.PSU__USB0__REF_CLK_SEL {Ref Clk2} \
    CONFIG.PSU__USB2_0__EMIO__ENABLE {0} \
    CONFIG.PSU__USB3_0__EMIO__ENABLE {0} \
    CONFIG.PSU__USB3_0__PERIPHERAL__ENABLE {1} \
    CONFIG.PSU__USB3_0__PERIPHERAL__IO {GT Lane2} \
    CONFIG.PSU__USB__RESET__MODE {Boot Pin} \
    CONFIG.PSU__USB__RESET__POLARITY {Active Low} \
    CONFIG.PSU__USE__IRQ0 {1} \
    CONFIG.PSU__USE__M_AXI_GP0 {1} \
    CONFIG.PSU__USE__M_AXI_GP1 {1} \
    CONFIG.PSU__USE__M_AXI_GP2 {0} \
    CONFIG.PSU__USE__S_AXI_GP0 {0} \
    CONFIG.PSU__USE__S_AXI_GP1 {0} \
    CONFIG.PSU__USE__S_AXI_GP2 {1} \
    CONFIG.PSU__USE__S_AXI_GP3 {1} \
    CONFIG.PSU__USE__S_AXI_GP4 {0} \
  ] $zynq_ultra_ps_e_0


  # Create instance: axi_interconnect_0, and set properties
  set axi_interconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_0 ]
  set_property -dict [list \
    CONFIG.ENABLE_ADVANCED_OPTIONS {0} \
    CONFIG.NUM_MI {16} \
    CONFIG.S00_HAS_DATA_FIFO {0} \
    CONFIG.STRATEGY {0} \
  ] $axi_interconnect_0


  # Create instance: slot_1
  create_hier_cell_slot_1 [current_bd_instance .] slot_1

  # Create instance: slot_2
  create_hier_cell_slot_2 [current_bd_instance .] slot_2

  # Create instance: slot_3
  create_hier_cell_slot_3 [current_bd_instance .] slot_3

  # Create instance: slot_4
  create_hier_cell_slot_4 [current_bd_instance .] slot_4

  # Create instance: slot_5
  create_hier_cell_slot_5 [current_bd_instance .] slot_5

  # Create instance: slot_6
  create_hier_cell_slot_6 [current_bd_instance .] slot_6

  # Create instance: slot_7
  create_hier_cell_slot_7 [current_bd_instance .] slot_7

  # Create instance: slot_8
  create_hier_cell_slot_8 [current_bd_instance .] slot_8

  # Create instance: decouple_status, and set properties
  set decouple_status [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 decouple_status ]
  set_property -dict [list \
    CONFIG.C_ALL_INPUTS {1} \
    CONFIG.C_GPIO_WIDTH {8} \
  ] $decouple_status


  # Create instance: smartconnect_0, and set properties
  set smartconnect_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect:1.0 smartconnect_0 ]
  set_property CONFIG.NUM_SI {8} $smartconnect_0


  # Create instance: xlconcat_0, and set properties
  set xlconcat_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlconcat:2.1 xlconcat_0 ]
  set_property CONFIG.NUM_PORTS {8} $xlconcat_0


  # Create instance: axi_interconnect_1, and set properties
  set axi_interconnect_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_interconnect:2.1 axi_interconnect_1 ]
  set_property CONFIG.NUM_MI {4} $axi_interconnect_1


  # Create instance: xlslice_0, and set properties
  set xlslice_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_0 ]
  set_property CONFIG.DIN_WIDTH {8} $xlslice_0


  # Create instance: xlslice_1, and set properties
  set xlslice_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_1 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {1} \
    CONFIG.DIN_TO {1} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_1


  # Create instance: xlslice_2, and set properties
  set xlslice_2 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_2 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {2} \
    CONFIG.DIN_TO {2} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_2


  # Create instance: xlslice_3, and set properties
  set xlslice_3 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_3 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {3} \
    CONFIG.DIN_TO {3} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_3


  # Create instance: xlslice_4, and set properties
  set xlslice_4 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_4 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {4} \
    CONFIG.DIN_TO {4} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_4


  # Create instance: xlslice_5, and set properties
  set xlslice_5 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_5 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {5} \
    CONFIG.DIN_TO {5} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_5


  # Create instance: xlslice_6, and set properties
  set xlslice_6 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_6 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {6} \
    CONFIG.DIN_TO {6} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_6


  # Create instance: xlslice_7, and set properties
  set xlslice_7 [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 xlslice_7 ]
  set_property -dict [list \
    CONFIG.DIN_FROM {7} \
    CONFIG.DIN_TO {7} \
    CONFIG.DIN_WIDTH {8} \
  ] $xlslice_7


  # Create instance: decouple, and set properties
  set decouple [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 decouple ]
  set_property -dict [list \
    CONFIG.C_ALL_INPUTS {0} \
    CONFIG.C_ALL_OUTPUTS {1} \
    CONFIG.C_GPIO_WIDTH {8} \
  ] $decouple


  # Create instance: DMA, and set properties
  set DMA [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_dma:7.1 DMA ]
  set_property -dict [list \
    CONFIG.c_addr_width {32} \
    CONFIG.c_include_sg {0} \
    CONFIG.c_m_axi_mm2s_data_width {64} \
    CONFIG.c_m_axis_mm2s_tdata_width {64} \
    CONFIG.c_mm2s_burst_size {32} \
    CONFIG.c_sg_length_width {26} \
  ] $DMA


  # Create instance: smartconnect_1, and set properties
  set smartconnect_1 [ create_bd_cell -type ip -vlnv xilinx.com:ip:smartconnect:1.0 smartconnect_1 ]

  # Create instance: Recv, and set properties
  set Recv [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:2.0 Recv ]
  set_property -dict [list \
    CONFIG.FIFO_DEPTH {1024} \
    CONFIG.FIFO_MODE {2} \
    CONFIG.IS_ACLK_ASYNC {1} \
  ] $Recv


  # Create instance: Send, and set properties
  set Send [ create_bd_cell -type ip -vlnv xilinx.com:ip:axis_data_fifo:2.0 Send ]
  set_property -dict [list \
    CONFIG.FIFO_DEPTH {1024} \
    CONFIG.FIFO_MODE {2} \
    CONFIG.IS_ACLK_ASYNC {1} \
  ] $Send


  # Create instance: aurora_64b66b_0, and set properties
  set aurora_64b66b_0 [ create_bd_cell -type ip -vlnv xilinx.com:ip:aurora_64b66b:12.0 aurora_64b66b_0 ]
  set_property -dict [list \
    CONFIG.C_REFCLK_SOURCE {MGTREFCLK1_of_Quad_X0Y1} \
    CONFIG.SupportLevel {1} \
    CONFIG.drp_mode {Disabled} \
  ] $aurora_64b66b_0


  # Create instance: Aurora_rst, and set properties
  set Aurora_rst [ create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 Aurora_rst ]
  set_property -dict [list \
    CONFIG.C_ALL_OUTPUTS {1} \
    CONFIG.C_GPIO_WIDTH {2} \
  ] $Aurora_rst


  # Create instance: pma_init, and set properties
  set pma_init [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 pma_init ]
  set_property CONFIG.DIN_WIDTH {2} $pma_init


  # Create instance: reset_pb, and set properties
  set reset_pb [ create_bd_cell -type ip -vlnv xilinx.com:ip:xlslice:1.0 reset_pb ]
  set_property -dict [list \
    CONFIG.DIN_FROM {1} \
    CONFIG.DIN_TO {1} \
    CONFIG.DIN_WIDTH {2} \
  ] $reset_pb


  # Create instance: rst_ps8_0_99M, and set properties
  set rst_ps8_0_99M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_ps8_0_99M ]

  # Create instance: rst_aurora_64b66b_0_161M, and set properties
  set rst_aurora_64b66b_0_161M [ create_bd_cell -type ip -vlnv xilinx.com:ip:proc_sys_reset:5.0 rst_aurora_64b66b_0_161M ]

  # Create interface connections
  connect_bd_intf_net -intf_net DMA_M_AXIS_MM2S [get_bd_intf_pins DMA/M_AXIS_MM2S] [get_bd_intf_pins Send/S_AXIS]
  connect_bd_intf_net -intf_net DMA_M_AXI_MM2S [get_bd_intf_pins DMA/M_AXI_MM2S] [get_bd_intf_pins smartconnect_1/S00_AXI]
  connect_bd_intf_net -intf_net DMA_M_AXI_S2MM [get_bd_intf_pins DMA/M_AXI_S2MM] [get_bd_intf_pins smartconnect_1/S01_AXI]
  connect_bd_intf_net -intf_net GT_DIFF_REFCLK1_0_1 [get_bd_intf_ports GT_DIFF_REFCLK1_0] [get_bd_intf_pins aurora_64b66b_0/GT_DIFF_REFCLK1]
  connect_bd_intf_net -intf_net GT_SERIAL_RX_0_1 [get_bd_intf_ports GT_SERIAL_RX_0] [get_bd_intf_pins aurora_64b66b_0/GT_SERIAL_RX]
  connect_bd_intf_net -intf_net Recv_M_AXIS [get_bd_intf_pins Recv/M_AXIS] [get_bd_intf_pins DMA/S_AXIS_S2MM]
  connect_bd_intf_net -intf_net Send_M_AXIS [get_bd_intf_pins Send/M_AXIS] [get_bd_intf_pins aurora_64b66b_0/USER_DATA_S_AXIS_TX]
  connect_bd_intf_net -intf_net aurora_64b66b_0_GT_SERIAL_TX [get_bd_intf_ports GT_SERIAL_TX_0] [get_bd_intf_pins aurora_64b66b_0/GT_SERIAL_TX]
  connect_bd_intf_net -intf_net aurora_64b66b_0_USER_DATA_M_AXIS_RX [get_bd_intf_pins aurora_64b66b_0/USER_DATA_M_AXIS_RX] [get_bd_intf_pins Recv/S_AXIS]
  connect_bd_intf_net -intf_net axi_interconnect_0_M00_AXI [get_bd_intf_pins axi_interconnect_0/M00_AXI] [get_bd_intf_pins slot_1/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M01_AXI [get_bd_intf_pins axi_interconnect_0/M01_AXI] [get_bd_intf_pins slot_1/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M02_AXI [get_bd_intf_pins axi_interconnect_0/M02_AXI] [get_bd_intf_pins slot_2/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M03_AXI [get_bd_intf_pins axi_interconnect_0/M03_AXI] [get_bd_intf_pins slot_2/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M04_AXI [get_bd_intf_pins axi_interconnect_0/M04_AXI] [get_bd_intf_pins slot_3/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M05_AXI [get_bd_intf_pins axi_interconnect_0/M05_AXI] [get_bd_intf_pins slot_3/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M06_AXI [get_bd_intf_pins axi_interconnect_0/M06_AXI] [get_bd_intf_pins slot_4/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M07_AXI [get_bd_intf_pins axi_interconnect_0/M07_AXI] [get_bd_intf_pins slot_4/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M08_AXI [get_bd_intf_pins axi_interconnect_0/M08_AXI] [get_bd_intf_pins slot_5/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M09_AXI [get_bd_intf_pins axi_interconnect_0/M09_AXI] [get_bd_intf_pins slot_5/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M10_AXI [get_bd_intf_pins axi_interconnect_0/M10_AXI] [get_bd_intf_pins slot_6/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M11_AXI [get_bd_intf_pins axi_interconnect_0/M11_AXI] [get_bd_intf_pins slot_6/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M12_AXI [get_bd_intf_pins axi_interconnect_0/M12_AXI] [get_bd_intf_pins slot_7/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M13_AXI [get_bd_intf_pins axi_interconnect_0/M13_AXI] [get_bd_intf_pins slot_7/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_0_M14_AXI [get_bd_intf_pins axi_interconnect_0/M14_AXI] [get_bd_intf_pins slot_8/s_axi_control]
  connect_bd_intf_net -intf_net axi_interconnect_0_M15_AXI [get_bd_intf_pins axi_interconnect_0/M15_AXI] [get_bd_intf_pins slot_8/s_axi_ctrl]
  connect_bd_intf_net -intf_net axi_interconnect_1_M00_AXI [get_bd_intf_pins axi_interconnect_1/M00_AXI] [get_bd_intf_pins decouple_status/S_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_1_M01_AXI [get_bd_intf_pins decouple/S_AXI] [get_bd_intf_pins axi_interconnect_1/M01_AXI]
  connect_bd_intf_net -intf_net axi_interconnect_1_M02_AXI [get_bd_intf_pins axi_interconnect_1/M02_AXI] [get_bd_intf_pins DMA/S_AXI_LITE]
  connect_bd_intf_net -intf_net axi_interconnect_1_M03_AXI [get_bd_intf_pins Aurora_rst/S_AXI] [get_bd_intf_pins axi_interconnect_1/M03_AXI]
  connect_bd_intf_net -intf_net slot_1_rp_intf_0 [get_bd_intf_pins smartconnect_0/S00_AXI] [get_bd_intf_pins slot_1/rp_intf_0]
  connect_bd_intf_net -intf_net slot_2_rp_intf_0 [get_bd_intf_pins smartconnect_0/S01_AXI] [get_bd_intf_pins slot_2/rp_intf_0]
  connect_bd_intf_net -intf_net slot_3_rp_intf_0 [get_bd_intf_pins smartconnect_0/S02_AXI] [get_bd_intf_pins slot_3/rp_intf_0]
  connect_bd_intf_net -intf_net slot_4_rp_intf_0 [get_bd_intf_pins smartconnect_0/S03_AXI] [get_bd_intf_pins slot_4/rp_intf_0]
  connect_bd_intf_net -intf_net slot_5_rp_intf_0 [get_bd_intf_pins smartconnect_0/S04_AXI] [get_bd_intf_pins slot_5/rp_intf_0]
  connect_bd_intf_net -intf_net slot_6_rp_intf_0 [get_bd_intf_pins smartconnect_0/S05_AXI] [get_bd_intf_pins slot_6/rp_intf_0]
  connect_bd_intf_net -intf_net slot_7_rp_intf_0 [get_bd_intf_pins smartconnect_0/S06_AXI] [get_bd_intf_pins slot_7/rp_intf_0]
  connect_bd_intf_net -intf_net slot_8_rp_intf_0 [get_bd_intf_pins smartconnect_0/S07_AXI] [get_bd_intf_pins slot_8/rp_intf_0]
  connect_bd_intf_net -intf_net smartconnect_0_M00_AXI [get_bd_intf_pins smartconnect_0/M00_AXI] [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HP0_FPD]
  connect_bd_intf_net -intf_net smartconnect_1_M00_AXI [get_bd_intf_pins smartconnect_1/M00_AXI] [get_bd_intf_pins zynq_ultra_ps_e_0/S_AXI_HP1_FPD]
  connect_bd_intf_net -intf_net zynq_ultra_ps_e_0_M_AXI_HPM0_FPD [get_bd_intf_pins zynq_ultra_ps_e_0/M_AXI_HPM0_FPD] [get_bd_intf_pins axi_interconnect_0/S00_AXI]
  connect_bd_intf_net -intf_net zynq_ultra_ps_e_0_M_AXI_HPM1_FPD [get_bd_intf_pins zynq_ultra_ps_e_0/M_AXI_HPM1_FPD] [get_bd_intf_pins axi_interconnect_1/S00_AXI]

  # Create port connections
  connect_bd_net -net Aurora_rst_gpio_io_o [get_bd_pins Aurora_rst/gpio_io_o] [get_bd_pins reset_pb/Din] [get_bd_pins pma_init/Din]
  connect_bd_net -net aurora_64b66b_0_sys_reset_out [get_bd_pins aurora_64b66b_0/sys_reset_out] [get_bd_pins rst_aurora_64b66b_0_161M/ext_reset_in]
  connect_bd_net -net aurora_64b66b_0_user_clk_out [get_bd_pins aurora_64b66b_0/user_clk_out] [get_bd_pins Recv/s_axis_aclk] [get_bd_pins rst_aurora_64b66b_0_161M/slowest_sync_clk] [get_bd_pins Send/m_axis_aclk]
  connect_bd_net -net decouple_gpio_io_o [get_bd_pins decouple/gpio_io_o] [get_bd_pins xlslice_0/Din] [get_bd_pins xlslice_1/Din] [get_bd_pins xlslice_2/Din] [get_bd_pins xlslice_3/Din] [get_bd_pins xlslice_4/Din] [get_bd_pins xlslice_5/Din] [get_bd_pins xlslice_6/Din] [get_bd_pins xlslice_7/Din]
  connect_bd_net -net pma_init_Dout [get_bd_pins pma_init/Dout] [get_bd_pins aurora_64b66b_0/pma_init]
  connect_bd_net -net reset_pb_Dout [get_bd_pins reset_pb/Dout] [get_bd_pins aurora_64b66b_0/reset_pb]
  connect_bd_net -net rst_aurora_64b66b_0_161M_peripheral_aresetn [get_bd_pins rst_aurora_64b66b_0_161M/peripheral_aresetn] [get_bd_pins Recv/s_axis_aresetn]
  connect_bd_net -net rst_ps8_0_99M_peripheral_aresetn [get_bd_pins rst_ps8_0_99M/peripheral_aresetn] [get_bd_pins Aurora_rst/s_axi_aresetn] [get_bd_pins axi_interconnect_1/M03_ARESETN] [get_bd_pins axi_interconnect_0/ARESETN] [get_bd_pins axi_interconnect_0/M00_ARESETN] [get_bd_pins slot_1/ap_rst_n] [get_bd_pins axi_interconnect_0/M01_ARESETN] [get_bd_pins axi_interconnect_0/M02_ARESETN] [get_bd_pins slot_2/ap_rst_n] [get_bd_pins axi_interconnect_0/M03_ARESETN] [get_bd_pins axi_interconnect_0/M04_ARESETN] [get_bd_pins slot_3/ap_rst_n] [get_bd_pins axi_interconnect_0/M05_ARESETN] [get_bd_pins axi_interconnect_0/M06_ARESETN] [get_bd_pins slot_4/ap_rst_n] [get_bd_pins axi_interconnect_0/M07_ARESETN] [get_bd_pins axi_interconnect_0/M08_ARESETN] [get_bd_pins slot_5/ap_rst_n] [get_bd_pins axi_interconnect_0/M09_ARESETN] [get_bd_pins axi_interconnect_0/M10_ARESETN] [get_bd_pins slot_6/ap_rst_n] [get_bd_pins axi_interconnect_0/M11_ARESETN] [get_bd_pins axi_interconnect_0/M12_ARESETN] [get_bd_pins slot_7/ap_rst_n] [get_bd_pins axi_interconnect_0/M13_ARESETN] [get_bd_pins axi_interconnect_0/M14_ARESETN] [get_bd_pins slot_8/ap_rst_n] [get_bd_pins axi_interconnect_0/M15_ARESETN] [get_bd_pins axi_interconnect_0/S00_ARESETN] [get_bd_pins axi_interconnect_1/ARESETN] [get_bd_pins axi_interconnect_1/M00_ARESETN] [get_bd_pins decouple_status/s_axi_aresetn] [get_bd_pins axi_interconnect_1/M01_ARESETN] [get_bd_pins decouple/s_axi_aresetn] [get_bd_pins axi_interconnect_1/M02_ARESETN] [get_bd_pins DMA/axi_resetn] [get_bd_pins axi_interconnect_1/S00_ARESETN] [get_bd_pins Send/s_axis_aresetn]
  connect_bd_net -net slot_1_decouple_status [get_bd_pins slot_1/decouple_status] [get_bd_pins xlconcat_0/In0]
  connect_bd_net -net slot_2_decouple_status [get_bd_pins slot_2/decouple_status] [get_bd_pins xlconcat_0/In1]
  connect_bd_net -net slot_3_decouple_status [get_bd_pins slot_3/decouple_status] [get_bd_pins xlconcat_0/In2]
  connect_bd_net -net slot_4_decouple_status [get_bd_pins slot_4/decouple_status] [get_bd_pins xlconcat_0/In3]
  connect_bd_net -net slot_5_decouple_status [get_bd_pins slot_5/decouple_status] [get_bd_pins xlconcat_0/In4]
  connect_bd_net -net slot_6_decouple_status [get_bd_pins slot_6/decouple_status] [get_bd_pins xlconcat_0/In5]
  connect_bd_net -net slot_7_decouple_status [get_bd_pins slot_7/decouple_status] [get_bd_pins xlconcat_0/In6]
  connect_bd_net -net slot_8_decouple_status [get_bd_pins slot_8/decouple_status] [get_bd_pins xlconcat_0/In7]
  connect_bd_net -net xlconcat_0_dout [get_bd_pins xlconcat_0/dout] [get_bd_pins decouple_status/gpio_io_i]
  connect_bd_net -net xlslice_0_Dout [get_bd_pins xlslice_0/Dout] [get_bd_pins slot_1/decouple]
  connect_bd_net -net xlslice_1_Dout [get_bd_pins xlslice_1/Dout] [get_bd_pins slot_2/decouple]
  connect_bd_net -net xlslice_2_Dout [get_bd_pins xlslice_2/Dout] [get_bd_pins slot_3/decouple]
  connect_bd_net -net xlslice_3_Dout [get_bd_pins xlslice_3/Dout] [get_bd_pins slot_4/decouple]
  connect_bd_net -net xlslice_4_Dout [get_bd_pins xlslice_4/Dout] [get_bd_pins slot_5/decouple]
  connect_bd_net -net xlslice_5_Dout [get_bd_pins xlslice_5/Dout] [get_bd_pins slot_6/decouple]
  connect_bd_net -net xlslice_6_Dout [get_bd_pins xlslice_6/Dout] [get_bd_pins slot_7/decouple]
  connect_bd_net -net xlslice_7_Dout [get_bd_pins xlslice_7/Dout] [get_bd_pins slot_8/decouple]
  connect_bd_net -net zynq_ultra_ps_e_0_pl_clk0 [get_bd_pins zynq_ultra_ps_e_0/pl_clk0] [get_bd_pins aurora_64b66b_0/init_clk] [get_bd_pins Aurora_rst/s_axi_aclk] [get_bd_pins axi_interconnect_1/M03_ACLK] [get_bd_pins rst_ps8_0_99M/slowest_sync_clk] [get_bd_pins axi_interconnect_0/ACLK] [get_bd_pins axi_interconnect_0/M00_ACLK] [get_bd_pins slot_1/ap_clk] [get_bd_pins axi_interconnect_0/M01_ACLK] [get_bd_pins axi_interconnect_0/M02_ACLK] [get_bd_pins slot_2/ap_clk] [get_bd_pins axi_interconnect_0/M03_ACLK] [get_bd_pins axi_interconnect_0/M04_ACLK] [get_bd_pins slot_3/ap_clk] [get_bd_pins axi_interconnect_0/M05_ACLK] [get_bd_pins axi_interconnect_0/M06_ACLK] [get_bd_pins slot_4/ap_clk] [get_bd_pins axi_interconnect_0/M07_ACLK] [get_bd_pins axi_interconnect_0/M08_ACLK] [get_bd_pins slot_5/ap_clk] [get_bd_pins axi_interconnect_0/M09_ACLK] [get_bd_pins axi_interconnect_0/M10_ACLK] [get_bd_pins slot_6/ap_clk] [get_bd_pins axi_interconnect_0/M11_ACLK] [get_bd_pins axi_interconnect_0/M12_ACLK] [get_bd_pins slot_7/ap_clk] [get_bd_pins axi_interconnect_0/M13_ACLK] [get_bd_pins axi_interconnect_0/M14_ACLK] [get_bd_pins slot_8/ap_clk] [get_bd_pins axi_interconnect_0/M15_ACLK] [get_bd_pins axi_interconnect_0/S00_ACLK] [get_bd_pins zynq_ultra_ps_e_0/maxihpm0_fpd_aclk] [get_bd_pins axi_interconnect_1/ACLK] [get_bd_pins axi_interconnect_1/M00_ACLK] [get_bd_pins decouple_status/s_axi_aclk] [get_bd_pins axi_interconnect_1/M01_ACLK] [get_bd_pins decouple/s_axi_aclk] [get_bd_pins axi_interconnect_1/M02_ACLK] [get_bd_pins DMA/s_axi_lite_aclk] [get_bd_pins axi_interconnect_1/S00_ACLK] [get_bd_pins zynq_ultra_ps_e_0/maxihpm1_fpd_aclk] [get_bd_pins DMA/m_axi_mm2s_aclk] [get_bd_pins smartconnect_1/aclk] [get_bd_pins Send/s_axis_aclk] [get_bd_pins DMA/m_axi_s2mm_aclk] [get_bd_pins Recv/m_axis_aclk] [get_bd_pins smartconnect_0/aclk] [get_bd_pins zynq_ultra_ps_e_0/saxihp0_fpd_aclk] [get_bd_pins zynq_ultra_ps_e_0/saxihp1_fpd_aclk]
  connect_bd_net -net zynq_ultra_ps_e_0_pl_resetn0 [get_bd_pins zynq_ultra_ps_e_0/pl_resetn0] [get_bd_pins rst_ps8_0_99M/ext_reset_in]

  # Create address segments
  assign_bd_address -offset 0xB0000000 -range 0x00010000 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs Aurora_rst/S_AXI/Reg] -force
  assign_bd_address -offset 0xB0010000 -range 0x00010000 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs DMA/S_AXI_LITE/Reg] -force
  assign_bd_address -offset 0xA0000000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_1/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA0010000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_1 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_1/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA0020000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_2 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_2/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA0030000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_3 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_2/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA0040000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_4 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_3/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA0050000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_5 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_3/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA0060000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_6 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_4/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA0070000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_7 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_4/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA0080000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_8 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_5/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA0090000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_9 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_5/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA00A0000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_10 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_6/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA00B0000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_11 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_6/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA00C0000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_12 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_7/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA00D0000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_13 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_7/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xA00E0000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_14 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_8/conv_pool_I_0/s_axi_control/Reg] -force
  assign_bd_address -offset 0xA00F0000 -range 0x00010000 -with_name SEG_conv_pool_I_0_Reg_15 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs slot_8/conv_pool_I_0/s_axi_ctrl/Reg] -force
  assign_bd_address -offset 0xB0020000 -range 0x00010000 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs decouple/S_AXI/Reg] -force
  assign_bd_address -offset 0xB0030000 -range 0x00010000 -target_address_space [get_bd_addr_spaces zynq_ultra_ps_e_0/Data] [get_bd_addr_segs decouple_status/S_AXI/Reg] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces DMA/Data_MM2S] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces DMA/Data_MM2S] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces DMA/Data_S2MM] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces DMA/Data_S2MM] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_1/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_1/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_2/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_2/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_3/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_3/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_4/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_4/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_5/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_5/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_6/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_6/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_7/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_7/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force
  assign_bd_address -offset 0x00000000 -range 0x80000000 -target_address_space [get_bd_addr_spaces slot_8/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_LOW] -force
  assign_bd_address -offset 0xC0000000 -range 0x20000000 -target_address_space [get_bd_addr_spaces slot_8/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_QSPI] -force

  # Exclude Address Segments
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces DMA/Data_MM2S] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces DMA/Data_MM2S] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces DMA/Data_S2MM] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces DMA/Data_S2MM] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP3/HP1_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_1/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_1/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_2/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_2/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_3/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_3/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_4/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_4/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_5/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_5/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_6/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_6/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_7/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_7/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]
  exclude_bd_addr_seg -target_address_space [get_bd_addr_spaces slot_8/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_DDR_HIGH]
  exclude_bd_addr_seg -offset 0xFF000000 -range 0x01000000 -target_address_space [get_bd_addr_spaces slot_8/conv_pool_I_0/Data_m_axi_data] [get_bd_addr_segs zynq_ultra_ps_e_0/SAXIGP2/HP0_LPS_OCM]


  # Restore current instance
  current_bd_instance $oldCurInst

  validate_bd_design
  save_bd_design
}
# End of create_root_design()


##################################################################
# MAIN FLOW
##################################################################

create_root_design ""


