JM Reference Software README
============================

The latest version of this software can be obtained from:

  http://iphome.hhi.de/suehring/tml

For reporting bugs please use the JM bug tracking system located at:

  https://ipbt.hhi.fraunhofer.de


Please send comments and additions to Karsten.Suehring (at) hhi.fraunhofer.de and alexis.tourapis@dolby.com

======================================================================================
NOTE: This file contains only a quick overview.

      More detailed information can be found the "JM Reference Software Manual" in the
      doc/ subdirectory of this package.
======================================================================================

1. Compilation
2. Command line parameters
3. Input/Output file format
4. Configuration files
5. Platform specific notes


1. Compilation
--------------

1.1 Windows
-----------
  
  Workspaces for MS Visual C++ 2003/2005/2008/2010 are provided with the names 

    jm_vc7.sln   - MS Visual C++ 2003
    jm_vc8.sln   - MS Visual C++ 2005
    jm_vc9.sln   - MS Visual C++ 2008
    jm_vc10.sln  - MS Visual C++ 2010

  These contain encoder and decoder projects.


1.2 Unix
--------

  Before compiling in a UNIX environment please run the "unixprep.sh" script which
  will remove the DOS LF characters from the files and create object directories.

  Makefiles for GNU make are provided at the top level and in the lencod and ldecod directories.


1.3 MacOS X
-----------

  A workspace for XCode can be found in the main directory. The project can also be build 
  using the UNIX build process (make).


2. Command line parameters
--------------------------

2.1 Encoder
-----------

    lencod.exe [-h] [-d default-file] [-f file] [-p parameter=value]

  All Parameters are initially taken from DEFAULTCONFIGFILENAME, defined in 
  configfile.h (typically: "encoder.cfg")

  -h
             Show help on parameters.

  -d default-file    
             Use the specified file as default configuration instead of the file in 
             DEFAULTCONFIGFILENAME.  

  -f file    
             If an -f parameter is present in the command line then 
             this file is used to update the defaults of DEFAULTCONFIGFILENAME.  
             There can be more than one -f parameters present.  

  -p parameter=value 

             If -p <ParameterName = ParameterValue> parameters are present then 
             these overide the default and the additional config file's settings, 
             and are themselfes overridden by future -p parameters.  There must 
             be whitespace between -f and -p commands and their respecitive 
             parameters.
  -v
             Show short version info.
  -V
             Show long version info.

2.2 Decoder
-----------

    ldecod.exe [-h] [-d default-file] [-f file] [-p parameter=value]

  All Parameters are initially taken from DEFAULTCONFIGFILENAME, defined in 
  configfile.h (typically: "encoder.cfg")

  -h
             Show help on parameters.

  -d default-file    
             Use the specified file as default configuration instead of the file in 
             DEFAULTCONFIGFILENAME.  

  -f file    
             If an -f parameter is present in the command line then 
             this file is used to update the defaults of DEFAULTCONFIGFILENAME.  
             There can be more than one -f parameters present.  

  -p parameter=value 

             If -p <ParameterName = ParameterValue> parameters are present then 
             these overide the default and the additional config file's settings, 
             and are themselfes overridden by future -p parameters.  There must 
             be whitespace between -f and -p commands and their respecitive 
             parameters.
  -v
             Show short version info.
  -V
             Show long version info.



3. Input/Output file format
---------------------------

  The source video material is read from raw YUV 4:2:0 data files.
  For output the same format is used.


4. Configuration files
----------------------

  Sample encoder and decode configuration files are provided in the bin/ directory.
  These contain explanatory comments for each parameter.
  
  The generic structure is explained here.

4.1 Encoder
-----------
  <ParameterName> = <ParameterValue> # Comments

  Whitespace is space and \t

  <ParameterName>  are the predefined names for Parameters and are case sensitive.
                   See configfile.h for the definition of those names and their 
                   mapping to configinput->values.

 <ParameterValue> are either integers [0..9]* or strings.
                  Integers must fit into the wordlengths, signed values are generally 
                  assumed. Strings containing no whitespace characters can be used directly.
                  Strings containing whitespace characters are to be inclosed in double 
                  quotes ("string with whitespace")
                  The double quote character is forbidden (may want to implement something 
                  smarter here).

  Any Parameters whose ParameterName is undefined lead to the termination of the program
  with an error message.

  Known bug/Shortcoming:  zero-length strings (i.e. to signal an non-existing file
                          have to be coded as "".
 
4.2 Decoder
-----------
  Beginning with JM 17.0 the decoder uses the same config file style like the encoder.


5. Platform specific notes
--------------------------
  This section contains hints for compiling and running the JM software on different 
  operating systems.

5.1 MacOS X
-----------
  MacOs X has a UNIX core so most of the UNIX compile process will work. You might need 
  the following modifications:

  a) Before Leopard (MacOS 10.5): in Makefile change "CC = $(shell which gcc)" to "CC = gcc"
     (it seems "which" doesn't work)

  b) MacOS "Tiger" (MacOS 10.4) doesn't come with ftime. We suggest using a third party ftime 
     implementation, e.g. from:

     http://darwinsource.opendarwin.org/10.3.4/OpenSSL096-3/openssl/crypto/ftime.c

5.2 FreeBSD
-----------
  You might need to add "-lcompat" to LIBS in the Makefiles for correct linking.
#!/bin/csh -f

#--------------------------------------------------------------------
# (C) 2011-2014 Renesas Electronics Corporation. All rights reserved.
#--------------------------------------------------------------------
##============================================================================##
##  Project Name   : Belize Simulation                                        ##
##  Script name    : RUN_VPX                                                  ##
##  Author         : Tetsuya Shibayama(Renesas)                               ##
##                                                                            ##
##  History                                                                   ##
##  Version  Date        Author(Company)   Description                        ##
##  v0.0     2011.12.27  T.Shibayama(Renesas)  New Entry                      ##
##  v1.0     2013.10.02  H.Ueda(Renesas)       Modify for Belize              ##
##  v1.1     2014.05.29  R.Hashimoto(REL)      clean-up, add coverage         ##
##============================================================================##

echo "============================================================"
echo " RUN_VPX : Simulation Scripts for VCP/VDP Simulation"
echo ""
echo " (C) Renesas Electronics Corp., 2011. All rights reserved."
echo "============================================================"
echo "Executing host: `hostname` at `date`"

unsetenv LD_LIBRARY_PATH
setenv LD_LIBRARY_PATH

#-------------------------------------
# Environment Variable Setting(Global)
#-------------------------------------

set SYSC_DEC_DIR = ../../sim/dec/gcc
set SYSC_ENC_DIR = ../../sim/enc/gcc
set SYSC_DIR     = ../../sim/dec/gcc
set GEN_DIR  = ../../../gen/hevc_dec/build/linux
set UM_DIR  = ../../../model

set GEN_NAME = TAppDecoderStatic
set UM_NAME = ./belize_model.dbg
#set ANALYZER = /design01/VCP/Belize/WORK/akieka/Belize_work/working/jm12.2/bin/ldecod.exe
set TOOL_DIR  = ../../../tools/bin/linux
set SYSC_BIN = bel_dec
set SYSC_BIN_EXE = bel_dec
set FPGA_BIN = ./Decode
set IP_NAME = bel1
set TOP_DIR = TOP

set REF_YUV = ref.yuv
# golden answer
set GA_DIR    = hev_sample_ok
set GA_YUV    = ${REF_YUV}
set GA_YUV_GZ = ${REF_YUV}.gz

set CETEST_FLAG = 0
set SYSC_FLAG = 0

set REBUILD_FLAG = 0
set CLEAN_BUILD  = 1
set CLEAN_INS  = 0
set GEN_BUILD = 1
set REMOVE_DIR_FLAG = 0
set FORCE_RM_DIR_FLAG = 0
set CP_FLAG = 0
set LINK_FLAG = 0
set DUMP_FLAG = 0
set MEMC_ENV_FLAG = 0
set FPGA_ENV_FLAG = 0
set CMP_MD5_FLAG = 0
set PARAM_OUT = 0
set WAVE     = 0
set WAVE_SYSC = 0
set WAVE_ST_FR = ""
set WAVE_ED_FR = ""
set GZIP_FLAG = 0
set CLEAN_FLAG = 0
set TOP_SIM = 0
set VLCS_SIM = 0
set MODE = top
set DEBUG_MODE=DEBUG
set USER=`whoami`
set FIXED_NAME = 0
set CODEC_DISABLE = 0
set USE_CTL = 0
set MW_DEBUG = 0
set UM_DEBUG = 0
set AVC_DEBUG = 0
set DUT_RES = "DUT_RES"
set SYSC_OPT = ""
set RST_OPT = ""
set UM_OPT = "-E -l PHEVE.lst"
set SVA_FLAG = 0
set SVA = ""
set REGRW_DIR   = reg_rw/reg_rw/reg_rw
set LD_MODE = 0
set FI_MODE = 0
set FR_MODE = 0
set GCOV = 0 ## GCOV
set ORG = "" ## not INS
set SKIP_INS_FLAG = 0 ## skip generating INS.cpp
set ONLY_INS_FLAG = 0 ## generating INS.cpp
set USE_DISP = 0
set BUSMONITOR = "" ## VIP
# add for AMBA Random
set APB_RND = "" ## VIP
set AXI_RND = "" ## VIP
set FORCE_DEB_THRU = 0 ## for debugging
set RVC = ""
set NO_BS = 0
set DEBUG_SC = "" ## "-g" option
set DIPF = 0
set LCV_ENABLE = 0
set LCV_FR = 0
set TOP_NAME = ""

set STIM_DIR    = NULL
set WORK_DIR    = `pwd`
set TEST_DIR    = ""
set WORK_001    = ""
set ASSERT_OPT  = ""
set OS_SET      = "REDHATE5_0" # default os
set OS_GEN      = "" # default os
set MEM         = "1000"
set CE_NUM      = 1
set FORCE_CE    = 0
set CE_AUTO_START = 0

set SKIP_SIM = 0
set SET_VAR = 0
set NO_CYCLE_CHK = 0
set SKIP_GEN_OPT = 0
set SKIP_GEN_FLAG = 0
set REAL_STB_FLAG = 0
set VPC_SIM_FLAG = 0
set PING = 0
set DAO_MODE = 0    #tamnguyen2 add Mar 11 2012

set SKIP_DIR_FLAG = 0  ## skip already passed patterns
set CFG_FILE = ""

set VLC_INSERT_SIM_FLAG = 0

set SW_MAX_CYCLE = 260000000
set SBUS_LOAD_FLAG = 0
set SIM_ALL_STAT = 0
set PSNR_LOG = "psnr_check.log"

set BIN_SUFFIX = ""
set AC_TYPE = 0
set SC_MAIN_FILE_NAME = "sc_main"

set GEN_FCPC_DIR  = ../../../TOP/sim/tb/fcpc/tb/fcl/build/gcc
set GEN_FCPC_NAME = FCL.exe

# config parameters
set VLC_PEDT_CNT_MAX = 0
set CE_PEDT_CNT_MAX = 0

set VLC_SKIP = 65535
set CE_SKIP  = 65535

##### set umask #####
umask 002
#####################

onintr  catch_interrupt

foreach WORK_002 ( `echo $WORK_DIR | sed -e 's/\// /g'` )
    if ( $WORK_002 == "logic" ) then
        set TEST_DIR = $TEST_DIR$WORK_001:h
        set WORK_001    = ""
    endif
    set WORK_001 = $WORK_001/$WORK_002
end
set TMP_DIR = ./tmp.$$
set SIM_DIR = ../sim
set LOG_DIR = ../log
set TB_DIR  = ../tb
set PAT_DIR = ../pat
set SCR_DIR = ../scripts
set COV_DIR = ../cov ## gcov
set COV_RESULT = cov

set DECODE_FLAG = 0
set ENCODE_FLAG = 0
set BATCH_FLAG = 0
set REGTEST = 0
set CTOP_FLAG = 0
set SEQ_FLAG = 0

set RESET_SIM   = 0
set RESET_PIC   = 0
set RESET_CYC   = 0
set RESET_NUME  = 0
set RESET_DENOM = 0

set CYCLE_FILE  = 0

set PARA_DEC     = 0
set PARA_ENC     = 0
set PARA_SIMUL   = 0

set RTG_SIM = 0
set CODEC = 0
set CODEC_PREFIX = "hev"
set EXTENSION = "265"
set SIM_MD = "2ce"
set GEN_EXT = ""

set SIM_EXEC_OPT = ""
set LD_OPT = ""
set IMR_OPT = ""

set TMlist  = ""

set FCPC_SIM = 0
set FCPC_MODE = 0
set FCPC_COMP_EN = 0
set DISABLE_FCP_RESET = 0
set FCPC_DUMP_FRM = 0
#### CODEC ####
set H265 = 1
set H264 = 0
set VP9  = 0


#----------------
# Argument Check
#----------------
if($#argv == 0 ) then
    goto usage
endif

while($#argv > 0)
    switch($1)
        case -help:
            goto usage
        breaksw
        case -vlcs:
            set VLCS_SIM = 1
            set MODE = vlcs
            shift
        breaksw
        case -ce:
            set CETEST_FLAG = 1
            shift
        breaksw
        case -top:
            set TOP_SIM = 1
            set MODE = top
            shift
        breaksw
        case -dec:
            set DECODE_FLAG = 1
            shift
        breaksw
        case -enc:
            set ENCODE_FLAG = 1
            shift
        breaksw
        case -bat:
            set BATCH_FLAG = 1
            shift
        breaksw
        case -reg:
            set REGTEST = 1
            shift
        breaksw
        case -link:
            set LINK_FLAG = 1
            shift
        breaksw
        case -cp:
            set CP_FLAG = 1
            shift
        breaksw
        case -sctop:
            set SYSC_FLAG = 1
            set CTOP_FLAG = 1
            set MODE = sc_htop
            shift
        breaksw
        case -sysc:
            set SYSC_FLAG = 1
            set MODE = sc_top
            shift
        breaksw
        case -rebuild:
            set REBUILD_FLAG = 1
            shift
        breaksw
        case -incremental
            set CLEAN_BUILD = 0
            shift
        breaksw
        case -nogenbuild
            set GEN_BUILD = 0
            shift
        breaksw
        case -clean_ins
            set CLEAN_INS = 1
            shift
        breaksw
        case -wave:
            set WAVE = 1
            shift
        breaksw
        case -wave_sysc:
            set WAVE_SYSC = 1
            shift
        breaksw
        case -vcd:
            set WAVE = 1
            shift
        breaksw
        case -vcdcst:
            set WAVE_ST_FR = "-vcdcst ${2}"
            shift;shift
        breaksw
        case -vcdced:
            set WAVE_ED_FR = "-vcdced ${2}"
            shift;shift
        breaksw
        case -vcdvst:
            set WAVE_ST_FR = "-vcdvst ${2}"
            shift;shift
        breaksw
        case -vcdved:
            set WAVE_ED_FR = "-vcdved ${2}"
            shift;shift
        breaksw
        case -os:
            set OS_SET = "${2}"
            shift;shift
        breaksw
        case -os_gen:
            set OS_GEN = "${2}"
            shift;shift
        breaksw
        case -clean:
            set CLEAN_FLAG = 1
            shift
        breaksw
        case -rm:
            set REMOVE_DIR_FLAG = 1
            shift
        breaksw
        case -force_rm:
            set REMOVE_DIR_FLAG = 1
            set FORCE_RM_DIR_FLAG = 1
            shift
        breaksw
        case -skip:
            set SKIP_DIR_FLAG = 1
            shift
        breaksw
        case -par_out:
            set PARAM_OUT = 1
            shift
        breaksw
        case -force_deb_thru:
            set FORCE_DEB_THRU = 1
            shift
        breaksw
        case -vreset:
            set RESET_SIM = 1
            set RESET_PIC = "${2}"
            set RESET_CYC = "${3}"
            shift;shift;shift;
        breaksw
        case -creset:
            set RESET_SIM = 2
            set RESET_PIC = "${2}"
            set RESET_CYC = "${3}"
            shift;shift;shift;
        breaksw
        case -vfresetfrm:
                set RESET_SIM = 6
                set RESET_NUME = "${2}"
                set RESET_DENOM = "${3}"
                shift;shift;shift;
        breaksw
        case -cfresetfrm:
                set RESET_SIM = 7
                set RESET_NUME = "${2}"
                set RESET_DENOM = "${3}"
                shift;shift;shift;
        breaksw
        case -nohung:
            set SYSC_OPT = "${SYSC_OPT} -nohung"
            shift
        breaksw
        case -ld:
            set LD_MODE = 1
            set LD_OPT = "-ld"
            if ( "${2}" =~ [0-9]* ) then
                set LD_OPT = "${LD_OPT} ${2}"
                shift
            endif
            if ( "${2}" =~ [0-9]* ) then
                set LD_OPT = "${LD_OPT} ${2}"
                shift
            endif
            echo "LD_OPT = ${LD_OPT}"
            set SYSC_OPT = "${SYSC_OPT} ${LD_OPT}"
            shift
        breaksw
        case -dao:
            set DAO_MODE = 1    #tamnguyen2 add Mar 11 2012
            shift
        breaksw
        case -use_imsb_only:
            set SYSC_OPT = "${SYSC_OPT} -use_imsb_only"
            shift
        breaksw
        case -imr_stall:
            set IMR_OPT = "-imr_stall"
            if ( "${2}" =~ [0-9]* ) then
                set IMR_OPT = "${IMR_OPT} ${2}"
                shift
            endif
            if ( "${2}" =~ [0-9]* ) then
                set IMR_OPT = "${IMR_OPT} ${2}"
                shift
            endif
            echo "IMR_OPT = ${IMR_OPT}"
            set SYSC_OPT = "${SYSC_OPT} ${IMR_OPT}"
            shift
        breaksw
        case -sysc_opt:
            if ( ( "${2}" == "-ld" ) ) then
                set LD_MODE = 1
            endif
            if ( ( "${2}" == "-fi" ) ) then
                set FI_MODE = 1
            endif
            if ( ( "${2}" == "-fr" ) ) then
                set FR_MODE = 1
            endif
            if ( ( "${2}" == "-vcd_st" )        || ( "${2}" == "-vcd_ed" )       || \
                 ( "${2}" == "-chpad_seed" )    || ( "${2}" == "-vhpad_seed" )   || \
                 ( "${2}" == "-crhpad_seed" )   || ( "${2}" == "-vrhpad_seed" )  || \
                 ( "${2}" == "-cwhpad_seed" )   || ( "${2}" == "-vwhpad_seed" )  || \
                 ( "${2}" == "-vpcrhpad_seed" ) || \
                 ( "${2}" == "-chpad_frame" )   || ( "${2}" == "-vhpad_frame" )  || \
                 ( "${2}" == "-crhpad_frame" )  || ( "${2}" == "-vrhpad_frame" ) || \
                 ( "${2}" == "-cwhpad_frame" )  || ( "${2}" == "-vwhpad_frame" ) || \
                 ( "${2}" == "-vpcrhpad_frame" )|| \
                 ( "${2}" == "-cfifo_seed" )    || ( "${2}" == "-vfifo_seed" )   || \
                 ( "${2}" == "-crfifo_seed" )   || ( "${2}" == "-vrfifo_seed" )  || \
                 ( "${2}" == "-cwfifo_seed" )   || ( "${2}" == "-vwfifo_seed" )  || \
                 ( "${2}" == "-vpcrfifo_seed" ) || \
                 ( "${2}" == "-cfifo_frame" )   || ( "${2}" == "-vfifo_frame" )  || \
                 ( "${2}" == "-crfifo_frame" )  || ( "${2}" == "-vrfifo_frame" ) || \
                 ( "${2}" == "-cwfifo_frame" )  || ( "${2}" == "-vwfifo_frame" ) || \
                 ( "${2}" == "-vpcrfifo_frame" ) ) then
                set SYSC_OPT = "${SYSC_OPT} ${2} ${3}"
                shift
            else
                set SYSC_OPT = "${SYSC_OPT} ${2}"
            endif
            if ( "${2}" == "-sbus_stop" ) then
                set NO_CYCLE_CHK = 1
            endif
            shift;shift
        breaksw
        case -sysc_opt2:
            set SYSC_OPT = "${SYSC_OPT} ${2} ${3}"
            if ( "${2}" == "-sbus_stop" ) then
                set NO_CYCLE_CHK = 1
            endif
            shift;shift;shift
        breaksw
        case -clkp:
            set SYSC_OPT = "${SYSC_OPT} -clkp ${2}"
            shift;shift
        breaksw
        case -clkm:
            set SYSC_OPT = "${SYSC_OPT} -clkm ${2}"
            shift;shift
        breaksw
        case -mb:
            set SYSC_OPT = "${SYSC_OPT} -mb"
            shift
        breaksw
        case -mbl:
            set SYSC_OPT = "${SYSC_OPT} -mbl"
            shift
        breaksw
        case -vltest:
            set SYSC_OPT = "${SYSC_OPT} -vltest"
            shift
        breaksw
        case -cetest:
            set SYSC_OPT = "${SYSC_OPT} -cetest"
            shift
        breaksw
        case -debug:
            set MW_DEBUG = 1
            shift
        breaksw
        case -um_debug:
            set UM_DEBUG = 1
            shift
        breaksw
        case -avc_debug:
            set AVC_DEBUG = 1
            shift
        breaksw
        case -ctb32:
            set SYSC_OPT = "${SYSC_OPT} -ctb32"
            set UM_OPT = "${UM_OPT} -ctb32"
            shift
        breaksw
        case -ctb64:
            set SYSC_OPT = "${SYSC_OPT} -ctb64"
            set UM_OPT = "${UM_OPT} -ctb64"
            shift
        breaksw
        case -ref_acm:
            set SYSC_OPT = "${SYSC_OPT} -ref_acm"
            shift
        breaksw
        case -vlhung:
            set SYSC_OPT = "${SYSC_OPT} -vlhung ${2}"
            shift
            shift
        breaksw
        case -cehung:
            set SYSC_OPT = "${SYSC_OPT} -cehung ${2}"
            shift
            shift
        breaksw
        case -dec_acm:
            set SYSC_OPT = "${SYSC_OPT} -dec_acm"
            shift
        breaksw
        case -debug_sc:
            set DEBUG_SC = "debug" ## "-g" option
            shift
        breaksw
        case -endian:
            set SYSC_OPT = "${SYSC_OPT} -endian ${2}"
            shift;shift
        breaksw
        case -dump:
            ### intermidiate dump ###
            set DUMP_FLAG = 1
            shift
        breaksw
        case -memc:
            set MEMC_ENV_FLAG = 1
            set MODE = memc_top
            shift
        breaksw
        case -fpga:
            set FPGA_ENV_FLAG = 1
            set MODE = fpga_top
            set GEN_BUILD = 0
            set NO_BS = 1
            shift
        breaksw
        case -sva:
            set SVA_FLAG = 1
            set SVA      = "sva"
            shift
        breaksw
        case -gcov:
            set GCOV = 1 ## GCOV
            shift
        breaksw
        case -org:
            set ORG = "org" ## not INS.cpp
            shift
        breaksw
        case -skip_ins:
            set SKIP_INS_FLAG = 1 ## skip generating INS.cpp
            shift
        breaksw
        case -ins:
            set ONLY_INS_FLAG = 1 ## generating INS.cpp
            shift
        breaksw
        case -busmonitor:
            set BUSMONITOR = "busmonitor" ## VIP
            shift
        breaksw
        # add for AMBA Random
        case -apb_rnd:
            set APB_RND = "apb_rnd" ## VIP
            shift
        breaksw
        case -axi_rnd:
            set AXI_RND = "axi_rnd" ## VIP
            shift
        breaksw
        case -gz:
            set GZIP_FLAG = 1
            shift
        breaksw
        case -vhpad:
            set SYSC_OPT = "${SYSC_OPT} -vhpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vhpad_fixed:
            set SYSC_OPT = "${SYSC_OPT} -vhpad ${2} ${3} ${4} ${5}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift;shift;shift;
        breaksw
        case -vrhpad:
            set SYSC_OPT = "${SYSC_OPT} -vrhpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vwhpad:
            set SYSC_OPT = "${SYSC_OPT} -vwhpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -chpad:
            set SYSC_OPT = "${SYSC_OPT} -chpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -chpad_fixed:
            set SYSC_OPT = "${SYSC_OPT} -chpad ${2} ${3} ${4} ${5}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift;shift;shift;
        breaksw
        case -crhpad:
            set SYSC_OPT = "${SYSC_OPT} -crhpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -cwhpad:
            set SYSC_OPT = "${SYSC_OPT} -cwhpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vpcrhpad:
            set SYSC_OPT = "${SYSC_OPT} -vpcrhpad"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vfifo:
            set SYSC_OPT = "${SYSC_OPT} -vfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vrfifo:
            set SYSC_OPT = "${SYSC_OPT} -vrfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vwfifo:
            set SYSC_OPT = "${SYSC_OPT} -vwfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -cfifo:
            set SYSC_OPT = "${SYSC_OPT} -cfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -crfifo:
            set SYSC_OPT = "${SYSC_OPT} -crfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -cwfifo:
            set SYSC_OPT = "${SYSC_OPT} -cwfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vpcrfifo:
            set SYSC_OPT = "${SYSC_OPT} -vpcrfifo"
            set NO_CYCLE_CHK = 1
            shift
        breaksw
        case -vfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -vfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -vrfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -vrfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -vwfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -vwfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -cfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -cfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -crfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -crfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -cwfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -cwfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -vpcrfifo_fixed:
            set SYSC_OPT = "${SYSC_OPT} -vpcrfifo ${2} ${2} ${3} ${3}"
            set NO_CYCLE_CHK = 1
            shift;shift;shift
        breaksw
        case -skip_sim:
            set SKIP_SIM = 1
            shift
        breaksw
        case -set_var:
            set SET_VAR = 1
            shift
        breaksw
        case -rload:
            set SYSC_OPT = "${SYSC_OPT} -rload ${2}"
            set NO_CYCLE_CHK = 1
            set SBUS_LOAD_FLAG = 1
            shift;shift
        breaksw
        case -lload
            set SYSC_OPT = "${SYSC_OPT} -lload ${2}"
            set NO_CYCLE_CHK = 1
            set SBUS_LOAD_FLAG = 1
            shift;shift
        breaksw
        case -cdef:
            set DEFINE_OPTC = "${DEFINE_OPTC} -D${2}"
            shift;shift
        breaksw
        case -vdef:
            set DEFINE_OPTV = "${DEFINE_OPTV} -D${2}"
            shift;shift
        breaksw
        case -h265:
            set H265 = 1
            set CODEC = 15
            set EXTENSION = "265"
            shift
        breaksw
        case -h264:
            set H264 = 1
            set H265 = 0
            set CODEC = 2
            set EXTENSION = "264"
            set GEN_DIR  = "../../../gen/avc_dec"
            set GEN_NAME = avc_dec
            set CODEC_PREFIX = "avc"
            set GA_DIR = avc_sample_ok
            shift
        breaksw
        case -vp9:
            set VP9 = 1
            set CODEC = 14
            set EXTENSION = "webm"
            set GEN_DIR  = "../../../gen/vp9_dec"
            set GEN_NAME = vpxdec
            set CODEC_PREFIX = "vp9"
            set GA_DIR = vp9_sample_ok
            shift
        breaksw
        case -codec_dis:
            set CODEC_DISABLE = 1
            shift
        breaksw
        case -pat:
            set FIXED_NAME = 1
            set STREAM = "${2}"
            shift;shift
        breaksw
        case -skip_gen:
            set SKIP_GEN_OPT = 1
            shift;
        breaksw
        case -cfg:
            set CFG_FILE = ${2}
            shift;shift
        breaksw
        case -no_cyc_chk:
            set NO_CYCLE_CHK = 1
            shift;
        breaksw
        case -stb:
            set REAL_STB_FLAG = 1
            shift;
        breaksw
        case -vpc:
            set VPC_SIM_FLAG = 1
            shift;
        breaksw
        case -rvc:
            set RVC = "rvc"
            shift;
        breaksw
        case -png:
            set PING = 1
            shift;
        breaksw
        case -no_bs:
            set NO_BS = 1
            shift;
        breaksw
        case -dipf:
            set DIPF = 1
            shift;
        breaksw
        case -lcv:
            set LCV_ENABLE = 1
            shift
        breaksw
        case -lcv_fr:
            set LCV_FR = 1
            shift
        breaksw
        case -use_cdmac_refbuf:
            set SYSC_OPT = "${SYSC_OPT} -use_cdmac_refbuf"
            shift;
        breaksw
        case -cdmac_wcmd_sep:
            set SYSC_OPT = "${SYSC_OPT} -cdmac_wcmd_sep"
            shift;
        breaksw
        case -cdmac_16x2_addr:
            set SYSC_OPT = "${SYSC_OPT} -cdmac_16x2_addr"
            shift;
        breaksw
        case -caxi2_disable:
            set SYSC_OPT = "${SYSC_OPT} -caxi2_disable"
            shift;
        breaksw
        case -trid_min0:
            set SYSC_OPT = "${SYSC_OPT} -trid_min0"
            shift;
        breaksw
        case -trid_sp:
            set SYSC_OPT = "${SYSC_OPT} -trid_sp"
            shift;
        breaksw
        case -mem:
            set MEM      = "${2}"
            shift;shift
        breaksw
        case -ce_num:
            set CE_NUM   = "${2}"
            shift;shift
        breaksw
        case -force_ce:
            set FORCE_CE    = 1
            shift;
        breaksw
        case -ac_type:
            set AC_TYPE = 1
            shift;
        breaksw
        case -ref_am:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift
        breaksw
        case -dec_am:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift
        breaksw
        case -tmp_am:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift
        case -ref_acm:
            set SYSC_OPT = "${SYSC_OPT} -ref_acm"
            shift;
        case -dec_acm:
            set SYSC_OPT = "${SYSC_OPT} -dec_acm"
            shift;
        breaksw
        case -md5:
            set CMP_MD5_FLAG = 1
            shift;
        breaksw
        case -fcpc_dump_frm:
            set SYSC_OPT = "${SYSC_OPT} ${1}"
            set FCPC_DUMP_FRM = 1
            shift;
        breaksw
        case -randerr:
            set SYSC_OPT = "${SYSC_OPT} -randerr ${2}"
            shift; shift;
        breaksw
        case -fcpc:
            set FCPC_SIM = 1
            set FCPC_MODE = "${2}"
            @ FCPC_COMP_EN = $FCPC_MODE % 2
            shift;shift
        breaksw
        case -direct_input
            set SYSC_OPT = "${SYSC_OPT} ${1}"
            shift;
            breaksw
        case -ce_auto:
            set CE_AUTO_START = 1;
            set SYSC_OPT = "${SYSC_OPT} -ce_wait_cycle ${2} 1"
            shift;shift;
            breaksw
        case -ce_reg_dump:
            set SYSC_OPT = "${SYSC_OPT} -ce_reg_dump"
            shift;
            breaksw
        case -no_irq:
            set SYSC_OPT = "${SYSC_OPT} -no_irq"
            shift;
            breaksw
        case -stride:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift
            breaksw
        case -vlc_ce_serial:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift
            breaksw
        case -skip_nal_bits:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift
            breaksw
        case -disable_ce_line_info_out:
            set SYSC_OPT = "${SYSC_OPT} -disable_ce_line_info_out"
            shift;
            breaksw
        case -disable_vlc_line_info_out:
            set SYSC_OPT = "${SYSC_OPT} -disable_vlc_line_info_out"
            shift;
            breaksw
        case -disable_md5_hw:
            set SYSC_OPT = "${SYSC_OPT} -disable_md5_hw"
            shift;
            breaksw
        case -cycle_file:
            set SYSC_OPT = "${SYSC_OPT} ${1}"
            set CYCLE_FILE = 1
            shift;
            breaksw
        case -wait_2t_rst:
            set SYSC_OPT = "${SYSC_OPT} ${1} ${2}"
            shift;shift;
            breaksw
        case -ce_pedt_cnt_max:
            set CE_PEDT_CNT_MAX = $2
            shift; shift
            breaksw
        case -vlc_pedt_cnt_max:
            set VLC_PEDT_CNT_MAX = $2
            shift; shift
            breaksw
        case -disable_fcp_rst:
            set DISABLE_FCP_RESET = 1
            shift
            breaksw
        case -wostd_max_num:
            set SYSC_OPT = "${SYSC_OPT} $1 $2"
            shift; shift
            breaksw
        case -rostd_max_num:
            set SYSC_OPT = "${SYSC_OPT} $1 $2"
            shift; shift
            breaksw
        case -vlc_skip :
            set VLC_SKIP = $2
            shift; shift
            breaksw
        case -ce_skip :
            set CE_SKIP = $2
            shift; shift
            breaksw
        case -seq_mode:
            set SEQ_FLAG = 1
            set SYSC_OPT = "${SYSC_OPT} ${1}"
            set UM_OPT = "${UM_OPT} -qmap_repeat"
            shift;
            breaksw
        case -qmap_repeat:
            set SYSC_OPT = "${SYSC_OPT} ${1}"
            set UM_OPT = "${UM_OPT} ${1}"
            shift
            breaksw
        default:
            if ( $TMlist == "" ) then
                set TMlist = $1
            else
                echo "Unknown option: $argv[1].  ignored."
                goto usage
            endif
        shift
        breaksw
    endsw
end

#-----------------
# set suffix
#-----------------
if ( ${GCOV} == 1 ) then
    set BIN_SUFFIX = "_ins"
    if ( ${AC_TYPE} == 1 ) then
      echo "Warning : ac type is not allowed when you get coverage"
      echo "          ac type is forced to off"
      set AC_TYPE = 0
    endif
else if ( ${AC_TYPE} == 1 ) then
    set BIN_SUFFIX = "_sim"
endif
#--------------------------
# Extension (default: 265)
#--------------------------

if ($GEN_EXT == "") then
    set GEN_EXT = $EXTENSION
endif


#-----------------
# Error Check
#-----------------
if ( ($CETEST_FLAG == 1) && ($VLCS_SIM == 1) ) then
    goto usage
else if ( $CETEST_FLAG == 1 && ($TOP_SIM == 1) ) then
    goto usage
else if ( ($TOP_SIM == 1) && ($VLCS_SIM == 1) ) then
    goto usage
endif

#----------------------
# Setting for FPGA
#----------------------
if( $FPGA_ENV_FLAG == 1 ) then
    set NO_BS = 1
    set TOOL_DIR = ../../../tools/bin/arm_linux

    set FPGA_VLC_DEV_NAME = "/dev/uio_bel_vlc"
    set FPGA_CE_DEV_NAME = "/dev/uio_bel_ce"
    set FPGA_NFS_PATH = "/mnt_fpga_env"
    set FPGA_IMG_PATH = "$FPGA_NFS_PATH/images"
    if ( $FCPC_SIM == 1 ) then
      set FPGA_BIN = ./Decode_fcpc
      set SYSC_OPT = "${SYSC_OPT} -ref_am 4"
    else
      set SYSC_OPT = "${SYSC_OPT} -ref_am 4 -dec_am 4"
    endif
endif

#----------------------
# Error Check for FPGA
#----------------------
if( $FPGA_ENV_FLAG == 1 ) then
    if( ! -e $FPGA_VLC_DEV_NAME ) then
        echo "$FPGA_VLC_DEV_NAME does not exist."
        goto usage
    else if( ! -e $FPGA_CE_DEV_NAME ) then
        echo "$FPGA_CE_DEV_NAME does not exist."
        goto usage
    else if( ( $ENCODE_FLAG == 1 ) && ( ! -e $FPGA_IMG_PATH ) ) then
        echo "$FPGA_IMG_PATH does not exist."
        goto usage
    endif
endif

set CPU_TYPE = `uname -m`
if ( $CPU_TYPE == "armv7l" ) then
    if ( $FPGA_ENV_FLAG != 1 ) then
        echo "-fpga option is necessary"
        goto usage
    endif
else
    if ( $FPGA_ENV_FLAG == 1 ) then
        echo "-fpag option is not allowed"
        goto usage
    endif
endif

#---------------------
# Image path setting
#---------------------
if( $FPGA_ENV_FLAG == 1 ) then
    #FPGA env ('/' is the root of zynq-linux)
    set IMG_PATH = "$FPGA_IMG_PATH"
else
    if( "$RVC" == "rvc" ) then
        #RVC env
        set IMG_PATH = /shsv/IP1/VPU/VPU5HA/work/image
    else
        #REL env
        set IMG_PATH = /design01/VPU5/DaVinci/WORK/images
    endif
endif


#-----------------
# set BS command
#-----------------

if ( $NO_BS == 1 ) then
    set BS_CMD = ""
else
    set BS_CMD = "bs -os ${OS_SET} -M ${MEM}"
endif

if ( $OS_GEN == "" ) then
    set BS_GEN = "${BS_CMD}"
else
    set BS_GEN = "bs -os ${OS_GEN} -M ${MEM}"
endif

#-----------------------
# Setting Revision File
#-----------------------

#-- make log directory
if (! -d ${WORK_DIR}/${LOG_DIR} ) then
    mkdir ${WORK_DIR}/${LOG_DIR}
endif

if ( $CETEST_FLAG == 1 ) then
    set REVFILE = ${WORK_DIR}/${LOG_DIR}/rebuild_ver_ce.log
else
    if ( $ENCODE_FLAG == 1 ) then
        set REVFILE   = ${WORK_DIR}/${LOG_DIR}/rebuild_ver_enc.log
    else
        set REVFILE   = ${WORK_DIR}/${LOG_DIR}/rebuild_ver_dec.log
    endif
endif


#---------------------------
# Create Coverage Directory
#---------------------------
set C_DIR = ${COV_DIR} ## GCOV
if ( $GCOV == 1 ) then
   mkdir -p ${C_DIR} ## GCOV
endif


#------------------
# Setting SYSC Binary
#------------------
if ( $ENCODE_FLAG == 1 ) then
    set SYSC_BIN = ./bel_enc
    set SYSC_DIR = $SYSC_ENC_DIR
else
    set SYSC_BIN = ./bel_dec
    set SYSC_DIR = $SYSC_DEC_DIR
endif

#-----------------
# Check Rebuild flag
#-----------------
if ( $REBUILD_FLAG == 1 ) then

    echo "start making bel_dec/enc"
    hg parent > rebuild_${MODE}.log
    hg parent | tee ${REVFILE}

    pushd $SYSC_DIR > /dev/null
    if ( $CLEAN_BUILD == 1 ) then
         make dist-clean AC_TYPE=${AC_TYPE} COVERAGE=${GCOV}
    endif
    rm -f obj/${SC_MAIN_FILE_NAME}${BIN_SUFFIX}.o
    ${BS_CMD} make SIMDEFS="BELIZE_SBUS_LOAD_VERIFY _DEBUG_SIM_CE_PERFORM _DEBUG_SIM_VLCS_PERFORM" BIN=${SYSC_BIN}_sbld AC_TYPE=${AC_TYPE} COVERAGE=${GCOV}
    rm -f obj/${SC_MAIN_FILE_NAME}${BIN_SUFFIX}.o
    ${BS_CMD} make SIMDEFS="_DEBUG_SIM_CE_PERFORM _DEBUG_SIM_VLCS_PERFORM" AC_TYPE=${AC_TYPE} COVERAGE=${GCOV}
    if ($status != 0) then
        exit 1
    endif

    popd > /dev/null

endif

#----------------
# make generator
#----------------
if ( ${GEN_BUILD} == 1 ) then
    echo "start making generator"
    pushd ${GEN_DIR} > /dev/null
    if ( $VP9 == 1 ) then
    ./configure --disable-multithread --enable-coefficient-range-checking --enable-vp9 --enable-experimental --enable-emulate_hardware
    endif
    ${BS_GEN} make
    if ($status != 0) then
        exit 1
    endif

    popd > /dev/null

    if( -d ${GEN_FCPC_DIR} ) then
        echo "start making FCPC model"
        pushd ${GEN_FCPC_DIR} > /dev/null
        ${BS_GEN} make
        if ($status != 0) then
            exit 1
        endif

        popd > /dev/null
    endif
endif

#----------------
# make UM
#----------------
if ( $ENCODE_FLAG == 1 && ${FPGA_ENV_FLAG} == 0 && $H265 == 1 ) then
    echo "start making Untimed Model"
    pushd ${UM_DIR} > /dev/null
    ${BS_CMD} make
    if ($status != 0) then
        exit 1
    endif

    popd > /dev/null

endif

if (( $TMlist == "" ) && ($REGTEST == 0)) then
    if ( ${REBUILD_FLAG} == 0 && ${GEN_BUILD} == 0 ) then
        printf "[ERROR] TMlist not specified."
        printf "\n\n\n"
        goto usage
    endif
    exit 1
endif

#-----------------
# Check List file
#-----------------
if ( (${REGTEST} == 0) && (! -e ${TMlist}) ) then
    printf '[ERROR] Specified list file: "'
    printf ${TMlist}
    printf '" does not exist.'
    printf "\n\n\n"
    goto usage
endif

set ABS_HDIR = `pwd | sed -e 's/\// /g' | awk '{ for(i=1;i<=NF-3;i=i+1){printf "/%s",$i}}'`
set SIM_EXDIR = `pwd | sed -e 's/\// /g' | awk '{print $NF}'`
setenv LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:${ABS_HDIR}/tools/linux


#--------------------
# Set M/W Option
#--------------------
if ( $REGTEST == 1 ) then
    set MW_OPT = "-REGTEST -ce_num ${CE_NUM}"
else if ( $BATCH_FLAG == 1 ) then
    set MW_OPT = "-B"
else if ( $ENCODE_FLAG == 1 ) then
    if ( $H264 == 1 ) then
        set MW_OPT = "-E -l PAVCE.lst"
    else
        set MW_OPT = "-E -l PHEVE.lst"
    endif
else
    if ( $VP9 == 1 ) then
        set MW_OPT = "-D -l PVP9D.lst"
    else if ( $H264 == 1 ) then
        set MW_OPT = "-D -l PAVCD.lst"
    else
        set MW_OPT = "-D -l PHEVD.lst"
    endif
endif

if ( $RESET_SIM == 1 ) then
    set RST_OPT = "-VR ${RESET_PIC} ${RESET_CYC}"
else if ( $RESET_SIM == 2 ) then
    set RST_OPT = "-CR ${RESET_PIC} ${RESET_CYC}"
else if ( $RESET_SIM == 6 ) then
    set RST_OPT = "-VFR_FRMS ${RESET_NUME} ${RESET_DENOM}"
else if ( $RESET_SIM == 7 ) then
    set RST_OPT = "-CFR_FRMS ${RESET_NUME} ${RESET_DENOM}"
endif

if ( $MW_DEBUG ) then
    set MW_OPT = "${MW_OPT} -debug -d 0xFFFFFFF0"
endif

if ( $DAO_MODE ) then
        set MW_OPT = "$MW_OPT -dao"
endif

if ( $DIPF ) then
    set MW_OPT = "$MW_OPT -dipf"
endif

if ( $LCV_ENABLE ) then
    set MW_OPT = "$MW_OPT -lcv"
endif

if ( $LCV_FR ) then
    set MW_OPT = "$MW_OPT -lcv_fr"
endif


#--------------------
# Set SystemC Option
#--------------------
if ($CODEC_DISABLE) then
    set SYSC_OPT = "${SYSC_OPT} -codec_dis"
endif

if (( $WAVE_SYSC ) || ( $WAVE )) then
    set SYSC_OPT = "${SYSC_OPT} -vcd ${WAVE_ST_FR} ${WAVE_ED_FR}"
endif

if ( $PARAM_OUT ) then
    set SYSC_OPT = "${SYSC_OPT} -par_out"
endif

if ( $FORCE_DEB_THRU ) then
    set SYSC_OPT = "${SYSC_OPT} -force_deb_thru"
endif


#-------------------------------------------
# Remove Comment & Redundant Line From List
#-------------------------------------------

set DATEDATA = ""

mkdir $TMP_DIR

if ( $REGTEST == 1 ) then
    echo $REGRW_DIR > ${TMP_DIR}/ss_tmp
else
    sed -e 's/	//g' -e '/^#/d' -e '/^$/d' -e '/^ *$/d' -e 's/#.*//' ${TMlist}  > ${TMP_DIR}/ss_tmp
endif

set NUM = `wc -l ${TMP_DIR}/ss_tmp | awk '{print $1}'`
set STIMS = (`awk '{printf "%s ",$1;}' ${TMP_DIR}/ss_tmp`)
set PPS_TMP   = (`awk '{printf "%s ",$1;}' ${TMP_DIR}/ss_tmp`)
set PPS = `echo "$PPS_TMP" | sed -e 's/\// /g' -e 's/stim_//g' | awk '{print $NF}'`
rm -rf ${TMP_DIR}

set S_DIR = ${SIM_DIR}
set L_DIR = ${LOG_DIR}
set R_DIR = ${LOG_DIR}

echo "NUM     : $NUM"
echo "STIMS   : $STIMS"
echo "PPS_TMP : $PPS_TMP"
echo "PPS     : $PPS"
echo "S_DIR   : $S_DIR"
echo "L_DIR   : $L_DIR"
echo "R_DIR   : $R_DIR"
echo "SIM_DIR : $SIM_DIR"

######################################################

#------------------------------
# Create Simulation Directory
#------------------------------
if( ! -e ${S_DIR} ) then
    mkdir ${S_DIR}
endif

#----------------------
# Create LOG Directory
#----------------------
if( ! -e ${L_DIR} ) then
    mkdir ${L_DIR}
endif

#--------------------------
# Create Results Directory
#--------------------------
if( ! -e ${R_DIR} ) then
    mkdir ${R_DIR}
endif


#-----------------
# Simulation Loop
#-----------------
@ i=1

if( ${RTG_SIM} != 1 ) then
    foreach list (${STIMS})
else
    foreach list (${STIMS[1]})
endif
    set SIM_ERR_FLAG = 0

    #set vfifo/cfifo parameter from pattern name (only when SET_VAR = 1 for safety)
    set PAT_NAM = ${STIMS[$i]}
    if ($SET_VAR == 1) then
        echo ${PAT_NAM} | grep -e "vfifo" > /dev/null
        if ($status == 0) then
            set FIFO = `echo ${PAT_NAM} | sed -e 's/^.*vfifo//' | sed -e 's/_.*$//'`
            set SYSC_OPT = "${SYSC_OPT} -vfifo ${FIFO} ${FIFO} ${FIFO} ${FIFO}"
            set NO_CYCLE_CHK = 1
        endif

        echo ${PAT_NAM} | grep -e "cfifo" > /dev/null
        if ($status == 0) then
            set FIFO = `echo ${PAT_NAM} | sed -e 's/^.*cfifo//' | sed -e 's/_.*$//'`
            set SYSC_OPT = "${SYSC_OPT} -cfifo ${FIFO} ${FIFO} ${FIFO} ${FIFO}"
            set NO_CYCLE_CHK = 1
        endif
        set PAT_NAM = `echo ${PAT_NAM} | sed -e 's/_vfifo[0-9]*//' | sed -e 's/_cfifo[0-9]*//'`
    endif

    #------------------
    # Checking Pat dir
    #------------------
    echo "PAT_NAM : $PAT_NAM"

    if ( $REGTEST != 1 ) then
        if( -d $PAT_DIR/$PAT_NAM ) then
        else
            echo "ERROR : ${PAT_NAM} does not exist."
            set SIM_ALL_STAT = 1
            goto next_pat
        endif
    endif

    #----------------------------
    # Skip already passed pattern
    #----------------------------
    if ( $SKIP_DIR_FLAG == 1 ) then
        set PREV_RESFILE   = ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/results.log
        if( -e ${PREV_RESFILE} ) then
            set SKIP_STAT = `grep FAIL $PREV_RESFILE | grep -c -v CYCLE`

            if ( ${SKIP_STAT} == 0 ) then
                echo ""
                echo "Already passed pattern ${STIMS[$i]}.${EXTENSION} is skipped."
                echo ""
                goto next_pat
            endif
        endif
    endif


    #------------------
    # Create directory
    #------------------
    #--  Create directory for simulation
    if( $CLEAN_FLAG == 1 ) then
        if( -e ${S_DIR}/${STIMS[$i]} ) then
            rm -rf ${S_DIR}/${STIMS[$i]}
            rm -rf ${L_DIR}/${STIMS[$i]}
        endif
    endif

    #-- Create directory for sim
    mkdir -p ${S_DIR}/${STIMS[$i]}
    #--  Create directory for log
    mkdir -p ${L_DIR}/${STIMS[$i]}
    #--  Create directory for result
    mkdir -p ${R_DIR}/${STIMS[$i]}
    #--  Create directory for coverage
    if ( ${GCOV} == 1 ) then
        mkdir -p ${C_DIR}/${STIMS[$i]} ## GCOV
    endif

    #----------------------
    # Check error pattern
    #----------------------
    set SKIP_GEN_FLAG = 0
    set ERROR_FLAG    = 0
    set EXCEPT_CE_ERR_DETECT = 0
    set EXCEPT_VL_ERR_DETECT = 0

    if (( ${SYSC_FLAG} == 1 ) || ( ${TOP_SIM} == 1 ) || ( ${VLCS_SIM} == 1 ) || ( ${FPGA_ENV_FLAG} == 1 )) then
        echo ${STIMS[$i]} | grep -e "stim_error" > /dev/null
        if ($status == 0) then
            set ERROR_FLAG    = 1
        endif

        echo ${STIMS[$i]} | grep -e "dec_264" -e "dec_264ld" | grep -e "stim_interoperability" > /dev/null
        if ($status == 0) then
            set ERROR_FLAG = `echo ${STIMS[$i]} | awk '/_error|e$/ {print 1;}'`
        endif

        ## Enc function
        echo ${STIMS[$i]} | grep -e "enc_" | grep -e "stim_function" > /dev/null
        if ($status == 0) then
            set ERROR_FLAG = `echo ${STIMS[$i]} | awk '/dve2_15$|dve2_17$|dve2_147$|dve2_271$|fna1_3$|fna1_135$|fna6_9$|fna6_11$|fna0_7$|fna0_9$|fna12_3$/ {print 1;}'`
        endif

        #----- Except pattern for ERROR Detect check
        ## H.265 Dec
        set EXCEPT_CE_ERR_DETECT = `echo ${STIMS[$i]} | awk '/it_range_over|dic2_137_d$/ {print 1;}'`
        set EXCEPT_VL_ERR_DETECT = `echo ${STIMS[$i]} | awk '/mvmax$|mvmin$|stim_mv_max0$/ {print 1;}'`
    endif

    if (( ${SYSC_FLAG} == 1 ) || ( ${VLCS_SIM} == 1 ) || ( ${FPGA_ENV_FLAG} == 1 )) then
        ## h264 enc nal_unit_header_extension
        echo ${STIMS[$i]} | grep -e "enc_264" | grep -e "stim_vlc_insert" > /dev/null
        if ($status == 0) then
            set SKIP_GEN_FLAG = 1
            set VLC_INSERT_SIM_FLAG = 1
            set SYSC_OPT = "${SYSC_OPT} -vltest"
        endif
    endif

###########################################################################

    #------------------
    # Setting for gen
    #------------------
    if ($FPGA_ENV_FLAG == 0) then
        set GEN_OPT = ""
        set GEN_CE_NUM = `echo 2^${CE_NUM} | bc -l`

        if ( $ENCODE_FLAG == 1 ) then
            if ( $H264 == 1 ) then
                set GEN_OPT = "${GEN_OPT} -enc -uv"
            else
                set GEN_OPT = "${GEN_OPT} --ISOutput=2 -c ${GEN_CE_NUM} --Nocrop"
            endif
        else
            if ( $VP9 == 1 ) then
                set GEN_OPT = "${GEN_OPT} --dump --rawvideo"
            else if ( $H264 == 1 ) then
                set GEN_OPT = "${GEN_OPT} -uv"
            else
                set GEN_OPT = "${GEN_OPT} --ISOutput=1 -c ${GEN_CE_NUM} --Nocrop"
            endif

        endif

        if ($LD_MODE) then
#            set GEN_OPT = "${GEN_OPT} -sb"
#            if (($SKIP_GEN_FLAG == 1) || ($SKIP_GEN_OPT == 1)) then
#                cp -rf ${SCR_DIR}/gen_imr_info.sh ${S_DIR}/${STIMS[$i]}/.
#            endif
#            if ($FI_MODE) then
#                set GEN_OPT = "${GEN_OPT} -fi"
#            endif
#            if ($FR_MODE) then
#                set GEN_OPT = "${GEN_OPT} -fr"
#            endif
        endif

        if ($FORCE_DEB_THRU) then
            set GEN_OPT = "${GEN_OPT} -force_deb_thru"
        endif
    endif


    #------------------
    # Setting LOG FILE
    #------------------
    set LOGFILE_V = ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/sysc.log
    set LOGFILE_A = ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/assert_summary.log
    set LOGFILE_G = ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/gen.log
    set RESFILE   = ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/results.log
    #set REFLOG    = ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/ref.log

    #-------------------
    # Copy for stimulus
    #-------------------
    if( $CP_FLAG == 1 ) then
        echo "cp -rf ${PAT_DIR}/${PAT_NAM}/* ${S_DIR}/${STIMS[$i]}/."
        cp -rf ${PAT_DIR}/${PAT_NAM}/* ${S_DIR}/${STIMS[$i]}/.

        cp -rf ${ABS_HDIR}/gen/bin/*                         ${S_DIR}/${STIMS[$i]}/.
        cp -rf ${SYSC_DIR}/${SYSC_BIN}${BIN_SUFFIX}      ${S_DIR}/${STIMS[$i]}/.
        cp -rf ${SYSC_DIR}/${SYSC_BIN}_sbld${BIN_SUFFIX} ${S_DIR}/${STIMS[$i]}/.

        if ( $CMP_MD5_FLAG == 1 ) then
            cp -rf ${SCR_DIR}/top_md5_verify.csh            ${S_DIR}/${STIMS[$i]}/.
        else if ( $VLCS_SIM == 1 ) then
            cp -rf ${SCR_DIR}/vlcs_verify.csh               ${S_DIR}/${STIMS[$i]}/.
            #cp -rf ${SCR_DIR}/vlcs_verify.pl                ${S_DIR}/${STIMS[$i]}/.
        else if ( $CETEST_FLAG == 1 ) then
            if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
                cp -rf ${SCR_DIR}/ce_fcpc_verify.csh        ${S_DIR}/${STIMS[$i]}/.
            else
                cp -rf ${SCR_DIR}/ce_verify.csh             ${S_DIR}/${STIMS[$i]}/.
            endif
        else if ( $VLC_INSERT_SIM_FLAG == 1 ) then
            cp -rf ${SCR_DIR}/vlc_insert_verify.csh         ${S_DIR}/${STIMS[$i]}/.
        else if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
            cp -rf ${SCR_DIR}/top_fcpc_verify.csh           ${S_DIR}/${STIMS[$i]}/.
        else if ( $ENCODE_FLAG == 1 ) then
            if ( $SEQ_FLAG == 1 ) then
                cp -rf ${SCR_DIR}/top_seq_verify.csh            ${S_DIR}/${STIMS[$i]}/top_enc_verify.csh
            else
                cp -rf ${SCR_DIR}/top_enc_verify.csh            ${S_DIR}/${STIMS[$i]}/.
            endif
#        else if ( $LD_MODE == 1 ) then
#            cp -rf ${SCR_DIR}/top_ld_verify.csh             ${S_DIR}/${STIMS[$i]}/top_verify.csh
        else
            cp -rf ${SCR_DIR}/top_verify.csh                ${S_DIR}/${STIMS[$i]}/.
        endif

        # FCPC SIM
        if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
            cp ${ABS_HDIR}/TOP/sim/tb/fcpc/tb/fcl/build/gcc/${GEN_FCPC_NAME} ${S_DIR}/${STIMS[$i]}/.
        endif

    else
        #----------------------
        #  link for tools/vlcs
        #----------------------

        ln -f -s ${WORK_DIR}/${PAT_DIR}/${PAT_NAM}/* ${S_DIR}/${STIMS[$i]}/.

        if ( -e ${GA_DIR}/${STIMS[$i]}/ref_disp.yuv ) then
            echo "Golden Answer(DISP) exists. Copy GA to Sim dir."
            echo "cp ${GA_DIR}/${STIMS[$i]}/* ${S_DIR}/${STIMS[$i]}/."
            cp ${GA_DIR}/${STIMS[$i]}/* ${S_DIR}/${STIMS[$i]}/.
            set USE_DISP = 1
        else
            set USE_DISP = 0
        endif

        if ( $FPGA_ENV_FLAG == 1 ) then
            ln -sf ${ABS_HDIR}/drv/build/gcc/Decoder/${FPGA_BIN}         ${S_DIR}/${STIMS[$i]}/.
        else
            ln -sf ${WORK_DIR}/${SYSC_DIR}/${SYSC_BIN}${BIN_SUFFIX}      ${S_DIR}/${STIMS[$i]}/.
            ln -sf ${WORK_DIR}/${SYSC_DIR}/${SYSC_BIN}_sbld${BIN_SUFFIX} ${S_DIR}/${STIMS[$i]}/.
            if ( $VP9 == 1 ) then
                ln -sf ${ABS_HDIR}/gen/vp9_dec/${GEN_NAME}               ${S_DIR}/${STIMS[$i]}/.
            else
                ln -sf ${ABS_HDIR}/gen/bin/${GEN_NAME}                   ${S_DIR}/${STIMS[$i]}/.
            endif
            ln -sf ${ABS_HDIR}/model/bin/${UM_NAME}                      ${S_DIR}/${STIMS[$i]}/.
        endif

        if ( $CMP_MD5_FLAG == 1 ) then
            cp -f ${WORK_DIR}/${SCR_DIR}/top_md5_verify.csh             ${S_DIR}/${STIMS[$i]}/.
        else if ( $VLCS_SIM == 1 ) then
            ln -f -s ${WORK_DIR}/${SCR_DIR}/vlcs_verify.csh             ${S_DIR}/${STIMS[$i]}/.
            #ln -f -s ${WORK_DIR}/${SCR_DIR}/vlcs_verify.pl              ${S_DIR}/${STIMS[$i]}/.
        else if ( $CETEST_FLAG == 1 ) then
            if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
                ln -f -s ${WORK_DIR}/${SCR_DIR}/ce_fcpc_verify.csh      ${S_DIR}/${STIMS[$i]}/.
            else
                ln -f -s ${WORK_DIR}/${SCR_DIR}/ce_verify.csh           ${S_DIR}/${STIMS[$i]}/.
            endif

        else if ( $VLC_INSERT_SIM_FLAG == 1 ) then
            ln -f -s ${WORK_DIR}/${SCR_DIR}/vlc_insert_verify.csh       ${S_DIR}/${STIMS[$i]}/.
        else if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
            ln -f -s ${WORK_DIR}/${SCR_DIR}/top_fcpc_verify.csh         ${S_DIR}/${STIMS[$i]}/.
        else if ( $ENCODE_FLAG == 1 ) then
            if ( $SEQ_FLAG == 1 ) then
                ln -f -s ${WORK_DIR}/${SCR_DIR}/top_seq_verify.csh          ${S_DIR}/${STIMS[$i]}/top_enc_verify.csh
            else
                ln -f -s ${WORK_DIR}/${SCR_DIR}/top_enc_verify.csh          ${S_DIR}/${STIMS[$i]}/.
            endif
#        else if ( $LD_MODE == 1 ) then
#            ln -f -s ${WORK_DIR}/${SCR_DIR}/top_ld_verify.csh           ${S_DIR}/${STIMS[$i]}/top_verify.csh
        else
            ln -f -s ${WORK_DIR}/${SCR_DIR}/top_verify.csh              ${S_DIR}/${STIMS[$i]}/.
        endif

        # FCPC SIM
        if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
            ln -fs ${ABS_HDIR}/TOP/sim/tb/fcpc/tb/fcl/build/gcc/${GEN_FCPC_NAME} ${S_DIR}/${STIMS[$i]}/.
        endif

    endif

    ln -f -s ${WORK_DIR}/${TOOL_DIR} ${S_DIR}/${STIMS[$i]}/tools

    #-------------------
    # Execute
    #-------------------

    # clear OPT for each Pattern
    set PAT_OPT = ""

    # move to sim dir
    pushd ${S_DIR}/${STIMS[$i]} > /dev/null

    set PATTEST = `echo ${PAT_NAM} | sed -e 's/\// /g' | awk '{printf "%s ",$NF;}'`

    # Check pattern name
    echo $PATTEST | grep -e "^err.*_900" > /dev/null
    if ($status == 0) then
        echo "$PATTEST : All Picture Conceal (VLC)"
        set PAT_OPT = "${PAT_OPT} -ec_all 1"
    endif

    echo $PATTEST | grep -e "^err.*_901" > /dev/null
    if ($status == 0) then
        echo "$PATTEST : All Picture Conceal (CE)"
        set PAT_OPT = "${PAT_OPT} -ec_all 2"
    endif

    echo $PATTEST | grep -e "^err2_5[0-9][0-9]" > /dev/null
    if ($status == 0) then
        echo "$PATTEST : First Slice Remove"
        set PAT_OPT = "${PAT_OPT} -fst_remove"
    endif

    #------------------------
    # Update EXTENSION
    #------------------------

    if ( $VP9 == 1 ) then # Default .webm ; update .ivf
        if ( -e "${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT}" ) then
            #No update
            echo "VP9 stream format : WEBM"
        else
            if ( -e "${CODEC_PREFIX}_sample_in/${PATTEST}.ivf" ) then
                set EXTENSION = "ivf"
                set GEN_EXT = $EXTENSION
                echo "VP9 stream format : IVF"
            else
                echo "ERROR : wrong VP9 stream format or stream is not existed"
            endif
        endif
    endif

    #------------------------
    # Remove log file
    #------------------------
    rm -rf *.log >& /dev/null

    #------------------------
    # Execute generator to make stimulus
    #------------------------
    if ( $VLCS_SIM && $ENCODE_FLAG ) then
        if ( ! -e no_gen ) then
            echo "Running generator (VLCS encode unit test) ..."

            set GEN_OPT2 = ""

            ${BS_GEN} ./$GEN_NAME -enc ${GEN_OPT} ${GEN_OPT2} -i ${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G

            rm -f dummy.yuv
            ln -s ${REF_YUV} dummy.yuv
            echo $LOGFILE_G
        else
            echo "Skip generator to make stimulus for VLCS eencode sim."
        endif
    endif

    #------------------------
    # Set image directory path
    #------------------------
    if ( $ENCODE_FLAG == 1 ) then

        if( "$RVC" == "rvc" ) then
            #RVC env
            ln -fs $IMG_PATH ./input_cmn
        else
            #REL env
            ln -fs $IMG_PATH ./input_cmn
        endif
        if ( $H264 == 1 ) then
            ln -fs input_cmn images
        endif

        # remove stream link
        rm -rf ${PATTEST}.${EXTENSION} >& /dev/null
        rm -rf out.${EXTENSION} >& /dev/null
    endif

    #------------------------
    # Make output directory
    #------------------------

    mkdir -p ${CODEC_PREFIX}_sample_out

    if ( $PARAM_OUT ) then
        mkdir param_out
    endif

    #------------------
    # Make Config file
    #------------------
    if ( $NO_CYCLE_CHK == 1 ) then
        set SW_MAX_CYCLE = 0x7FFFFFFF
    endif
    set EDT_MAX = 2
    if ( $FPGA_ENV_FLAG ) then
        set EDT_MAX = 128
    endif

    if ( ${DECODE_FLAG} == 1 ) then

        if ( $VP9 == 1 ) then
            set CFG = "PVP9D.cfg"
        else if ( $H264 == 1 ) then
            set CFG = "PAVCD.cfg"
        else
            set CFG = "PHEVD.cfg"
        endif
    else
        if ( $H264 == 1 ) then
            set CFG = "PAVCE.cfg"
        else
            set CFG = "PHEVE.cfg"
        endif
    endif

    if ( $VLCS_SIM == 1 || $CETEST_FLAG == 1 ) then
        rm -f ${CFG}
    endif
    set IS_DUMP_EN = `expr 1 - ${FPGA_ENV_FLAG}`;
    if ( ! -f ${CFG} ) then
        echo "/*-----------------------*/"          >  ${CFG}
        echo "/* Allover ctrl          */"          >> ${CFG}
        echo "/*-----------------------*/"          >> ${CFG}
        echo ""                                     >> ${CFG}
        echo "/* Pseudo settings */"                >> ${CFG}
        echo "app_use_vlc_pseudo = ${CETEST_FLAG};" >> ${CFG}
        echo "app_use_ce_pseudo  = ${VLCS_SIM};"    >> ${CFG}
        echo "force_num_ope_ce   = ${FORCE_CE};"    >> ${CFG}
        echo "log2_num_ope_ce    = ${CE_NUM};"      >> ${CFG}
        echo "app_skip_vlc_start_pic_no = ${VLC_SKIP};" >> ${CFG}
        echo "app_skip_vlc_end_pic_no   = 65535;"       >> ${CFG}
        echo "app_skip_ce_start_pic_no  = ${CE_SKIP};"  >> ${CFG}
        echo "app_skip_ce_end_pic_no    = 65535;"       >> ${CFG}
        echo ""                                     >> ${CFG}
        echo "/* EDT counter */"                    >> ${CFG}
        echo "vlc_edt_cnt_max = ${EDT_MAX};"        >> ${CFG}
        echo "ce_edt_cnt_max  = ${EDT_MAX};"        >> ${CFG}
        if ( ${VLC_PEDT_CNT_MAX} != 0 ) then
            echo "vlc_pedt_cnt_max = $VLC_PEDT_CNT_MAX;" >> ${CFG}
        else if ( ${FPGA_ENV_FLAG} == 1 ) then
            echo "vlc_pedt_cnt_max = 0xffff;"       >> ${CFG}
        endif
        if ( ${CE_PEDT_CNT_MAX} != 0 ) then
            echo "ce_pedt_cnt_max  = $CE_PEDT_CNT_MAX;" >> ${CFG}
        else if ( ${FPGA_ENV_FLAG} == 1 ) then
            echo "ce_pedt_cnt_max  = 0xffff;"       >> ${CFG}
        endif
        echo ""                                     >> ${CFG}
        echo "/* output YUV */"                     >> ${CFG}
        echo "force_output_yuv_disable = 0;"        >> ${CFG}
        echo "force_output_yuv_format  = ${CMP_MD5_FLAG};"        >> ${CFG}
        echo "force_output_diff_yuv_disable = 1;"   >> ${CFG}
        echo "force_output_yuv_cropped = 0;"        >> ${CFG}
        echo ""                                     >> ${CFG}
        echo "/* max cycle */"                      >> ${CFG}
        echo "max_ce_cycle  = ${SW_MAX_CYCLE};"     >> ${CFG}
        echo "max_vlc_cycle = ${SW_MAX_CYCLE};"     >> ${CFG}
        echo ""                                     >> ${CFG}
        echo "/* output Inter Mediate Stream */"                  >> ${CFG}
        echo "imd_store_enable = ${IS_DUMP_EN};"                  >> ${CFG}
        echo "imd_store_path = ./;"                               >> ${CFG}
        echo "fname_ims_store_fmt = vlcs_istream_ce%d_%04d.bin;"  >> ${CFG}
        echo "fname_imc_store_fmt = vlcs_icstream_ce%d_%04d.bin;" >> ${CFG}
        echo "ce_auto_exe_enabled = ${CE_AUTO_START};" >> ${CFG}
        if(${FCPC_SIM} == 1) then
            echo "fcp_config = ${FCPC_MODE};" >> ${CFG}
        endif

        if ( $DECODE_FLAG == 1 ) then
            if ( $CETEST_FLAG == 1 ) then
                echo ""                                                                                   >> ${CFG}
                echo "/* Intermediate stream ctrl */"                                                     >> ${CFG}
                echo "imd_file_enable = 1;                   /* enable reading IS and ICS from file */"   >> ${CFG}
                echo "imd_path        = ./input_cmn;         /* path of IS and ICS file */"               >> ${CFG}
                echo "fname_ims_fmt   = ref_vlcs_istream_ce%d_%04d.bin;   /* file name format for IS */"  >> ${CFG}
                echo "fname_imc_fmt   = ref_vlcs_icstream_ce%d_%04d.bin;  /* file name format for ICS */" >> ${CFG}
                echo "/* IS size input */"                                                                >> ${CFG}
                echo "its_file_enable = 1;                   /* enable reading IS size info from file */" >> ${CFG}
                echo "its_path        = ./input_cmn;         /* path of IS size info file */"             >> ${CFG}
                echo "fname_its_fmt   = ref_vlcs_size_%04d.bin;"                                          >> ${CFG}
            endif
        endif
    else if ( ${FPGA_ENV_FLAG} == 1 || ${FCPC_SIM} == 1 || ${CE_AUTO_START} == 1 || ${FORCE_CE} == 1) then
        mv -f ${CFG} ${CFG}.tmp
        echo "ce_auto_exe_enabled = ${CE_AUTO_START};" > ${CFG}
        if ( ${FPGA_ENV_FLAG} == 1 ) then
            echo "vlc_edt_cnt_max = ${EDT_MAX};"    >> ${CFG}
            echo "ce_edt_cnt_max  = ${EDT_MAX};"    >> ${CFG}
            echo "vlc_pedt_cnt_max = 0xffff;"       >> ${CFG}
            echo "ce_pedt_cnt_max  = 0xffff;"       >> ${CFG}
            echo "force_output_yuv_format  = ${CMP_MD5_FLAG};"  >> ${CFG}
            echo "imd_store_enable = ${IS_DUMP_EN};"             >> ${CFG}
        endif
        if(${FCPC_SIM} == 1) then
            echo "fcp_config = ${FCPC_MODE};" >> ${CFG}
        endif

        if (${FORCE_CE} == 1) then
            echo "force_num_ope_ce   = ${FORCE_CE};"    >> ${CFG}
            echo "log2_num_ope_ce    = ${CE_NUM};"      >> ${CFG}
        endif

        cat ${CFG}.tmp >> ${CFG}
    endif
    #-------------------
    # Coverage
    #-------------------
    if ( ${GCOV} == 1 ) then
        mkdir -p ${COV_RESULT}
        setenv GCOV_PREFIX `pwd`/${COV_RESULT}
        setenv GCOV_PREFIX_STRIP `pwd | sed -e s/"[^\/]"//g | wc -c`
    endif

    if( $H264 == 1 ) then
        if ($ENCODE_FLAG == 1) then
            echo "${PATTEST}.ctl" > PAVCE.lst
            mkdir -p ${CODEC_PREFIX}_sample_out
        else
            if ( ! -e PAVCD.lst ) then
                echo "${PATTEST}.${EXTENSION} avc avc_sample_in avc_sample_out avc_sample_ok PAVCD.avc_sample" > PAVCD.lst
            endif
            mkdir -p ${CODEC_PREFIX}_sample_in
            if ( -e stream.${EXTENSION} ) then
                echo "ln -s stream.${EXTENSION} ${PATTEST}.${EXTENSION}"
                ln -s stream.${EXTENSION} ${PATTEST}.${EXTENSION}
            endif
            ln -s ../${PATTEST}.${EXTENSION} ${CODEC_PREFIX}_sample_in/${PATTEST}.${EXTENSION}
        endif
    endif

  if ( $SKIP_SIM == 1 ) then
    echo "skip_sim"
  else
    #-------------------
    # Execute
    #-------------------
    if ( $FPGA_ENV_FLAG ) then
        set SYSC_BIN_EXE = ${FPGA_BIN}
    else if ( $SBUS_LOAD_FLAG ) then
        set SYSC_BIN_EXE = ${SYSC_BIN}_sbld${BIN_SUFFIX}
        echo ""
        echo "Use SBUS LOAD verification environment"
        echo ""
    else
        set SYSC_BIN_EXE = ${SYSC_BIN}${BIN_SUFFIX}
    endif

    if (( $CETEST_FLAG == 1 ) && ( $DECODE_FLAG == 1 )) then
        mkdir -p input_cmn
        mkdir -p ${GA_DIR}
        pushd input_cmn > /dev/null

        echo "Running generator for IS ... (${PATTEST})"
        if ( $VP9 == 1 ) then
        #    ../$GEN_NAME ${GEN_OPT} -o ../${GA_DIR}/${PATTEST}_ok.yuv ../${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} >&$LOGFILE_G
            ../$GEN_NAME ${GEN_OPT}  --noblit -r  ../${GA_DIR}/${PATTEST}_ok.yuv ../${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} >&$LOGFILE_G
        else if ( $H264 == 1 ) then
            ../$GEN_NAME ${GEN_OPT} -i ../${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ./${REF_YUV} >& $LOGFILE_G
        else
            ../$GEN_NAME ${GEN_OPT} -b ../${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ./${REF_YUV} --LogOutput=4 >& $LOGFILE_G
        endif

        touch IS_FOR_${PATTEST}
        popd

        echo " $SYSC_BIN_EXE ${RST_OPT} ${SYSC_OPT} ${MW_OPT} ${PAT_OPT} ($PATTEST)"
        echo "RST_OPT  : $RST_OPT"
        echo "SYSC_OPT : $SYSC_OPT"
        echo "MW_OPT   : $MW_OPT"
        echo "PAT_OPT  : $PAT_OPT"
        ${BS_CMD} $SYSC_BIN_EXE ${RST_OPT} ${SYSC_OPT} ${MW_OPT} ${PAT_OPT} |& tee $LOGFILE_V
    else
        if ( $SVA_FLAG == 1 ) then
            echo "run_vcs_exe.cmd $SIM_EXEC_OPT -cm assert ($PATTEST)"
            ${BS_CMD} $SYSC_BIN_EXE ${RST_OPT} ${SYSC_OPT} ${MW_OPT} ${PAT_OPT} -cm assert |& tee $LOGFILE_V
        else
            echo " $SYSC_BIN_EXE ${SYSC_OPT} ${MW_OPT} ${PAT_OPT} ($PATTEST)"
            echo "RST_OPT  : $RST_OPT"
            echo "SYSC_OPT : $SYSC_OPT"
            echo "MW_OPT   : $MW_OPT"
            echo "PAT_OPT  : $PAT_OPT"
            if ( $ENCODE_FLAG == 1 && $H265 == 1 ) then
                if ( $SEQ_FLAG != 1 ) then
                    set UM_OPT = "${UM_OPT} -ce_vlc_serial 1"
                endif
                set UM_OPT = "${UM_OPT} -o ${PATTEST}_um.${GEN_EXT}"
                echo "${BS_CMD} ${UM_NAME} ${UM_OPT}"
                ${BS_CMD} ${UM_NAME} ${UM_OPT} >& /dev/null
                #mv ${CODEC_PREFIX}_sample_out/${PATTEST}.${GEN_EXT} ${CODEC_PREFIX}_sample_out/${PATTEST}_um.${GEN_EXT}
                #sleep 10
            endif
            ${BS_CMD} $SYSC_BIN_EXE ${RST_OPT} ${SYSC_OPT} ${MW_OPT} ${PAT_OPT} |& tee $LOGFILE_V
        endif
    endif
  endif

  if ($status != 0) then
    set SIM_ERR_FLAG = 1
  endif

    if ( -e ${REVFILE} ) then
        cat ${REVFILE} >> $LOGFILE_V
    endif

    if ( $REGTEST == 1 ) then
        goto reg_end
    endif
    echo "sim_end"

    #-------------------
    # ref decoder
    #-------------------
    rm -rf $RESFILE

    #-------------------
    # File check & wait
    #-------------------
    if ( $FPGA_ENV_FLAG == 0 ) then
        @ num_wait = 0
        while ( $num_wait < 3 )
          if ( -e $LOGFILE_V ) then
            grep -sq SIZE $LOGFILE_V
            if ( $? == 0 ) then
              break
            endif
          endif
          sleep 30
          @ num_wait++
        end

        if ( $ENCODE_FLAG ) then
          @ num_wait = 0
          while ( $num_wait < 3 )
            if ( $H265 == 1 ) then
              if ( -e ${CODEC_PREFIX}_sample_out/${PATTEST}.${GEN_EXT} ) then
                break
              endif
            else
              if ( -e ./${PATTEST}.${GEN_EXT} ) then
                break
              endif
            endif
            @ num_wait++
            echo "wait encoded stream $num_wait"
            sleep 10
          end
        endif
    endif

    ## Golden Answer check
    set GA_CE_NUM = `echo 2^${CE_NUM} | bc -l`

    if ( -e ${GA_DIR}/${GA_CE_NUM}ce/${GA_YUV} ) then
        set GA_FILE = ${GA_DIR}/${GA_CE_NUM}ce/${GA_YUV}
    else
        set GA_FILE = ${GA_DIR}/${GA_YUV}
    endif

    if ( -e ${GA_DIR}/${GA_CE_NUM}ce/${GA_YUV_GZ} ) then
        set GA_FILE_GZ = ${GA_DIR}/${GA_CE_NUM}ce/${GA_YUV_GZ}
    else
        set GA_FILE_GZ = ${GA_DIR}/${GA_YUV_GZ}
    endif

    if ((( -e ${GA_FILE} ) || ( -e ${GA_FILE_GZ} )) && ( ${FPGA_ENV_FLAG} == 0 )) then
        set SKIP_GEN_FLAG = 1
        cat ${GA_DIR}/gen.log > ${LOGFILE_G}
        cp ${GA_DIR}/${GA_CE_NUM}ce/*.bin ./
        if ( -e ${GA_FILE_GZ} ) then
            gunzip -c ${GA_FILE_GZ} > ${REF_YUV}
        else
            cp ${GA_FILE} ./${REF_YUV}
        endif
    endif

    set wait = 0
    if ( $VLCS_SIM == 1 ) then
        if (($SKIP_GEN_FLAG == 1) || ($SKIP_GEN_OPT == 1)) then
            echo "Skip generator ..."
        else
            if ( $ENCODE_FLAG ) then
                rm -rf DUT_RES
                mkdir DUT_RES
                pushd DUT_RES
                set GEN_OPT_C =

                # run generator to check created stream by VCP3
                ${BS_GEN} ../${GEN_NAME} ${GEN_OPT} ${GEN_OPT_C} -i ../out.${GEN_EXT} -o dut.yuv >& gen.log

                popd
                if ( -e no_gen ) then
                    # Since we did not run generator using input stream, we need to use log from generator output for encoded stream
                    rm -f $LOGFILE_G
                    mv DUT_RES/gen.log $LOGFILE_G
                    ln -s $LOGFILE_G DUT_RES/gen.log
                endif
            else
                if ( $VP9 == 1 ) then
                    echo "${BS_GEN} ${GEN_NAME} ${GEN_OPT} --noblit -r ${REF_YUV} ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} --noblit -r ${REF_YUV} ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} >& $LOGFILE_G
                else if( $H264 == 1 ) then
                    echo "${BS_GEN} $GEN_NAME ${GEN_OPT} -i ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} -i ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G
                    #${ANALYZER} -ll 2 -i ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} >& $REFLOG
                else
                    echo "${BS_GEN} $GEN_NAME ${GEN_OPT} -b ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} -b ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G
                endif
                sed -i "s/COUNT: X/COUNT: 1/" $LOGFILE_G
            endif
        endif
    else if ( ($TOP_SIM == 1) || ($CETEST_FLAG == 1) || ($MEMC_ENV_FLAG == 1) || ($FPGA_ENV_FLAG == 1) ) then
        if (($SKIP_GEN_FLAG == 1) || ($SKIP_GEN_OPT == 1) || ($FPGA_ENV_FLAG == 1)) then
            echo "Skip generator ... ($PATTEST)"
        else
            echo "Running generator ... ($PATTEST)"
            if ( $ENCODE_FLAG ) then
                set wait = 1
                if( $H264 == 1 ) then
                    echo "${BS_GEN} $GEN_NAME ${GEN_OPT} -i ./${PATTEST}.${GEN_EXT} -o ${REF_YUV}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} -i ./${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G
                else
                    echo "${BS_GEN} $GEN_NAME ${GEN_OPT} -b ${CODEC_PREFIX}_sample_out/${PATTEST}.${GEN_EXT} -o ${REF_YUV}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} -b ${CODEC_PREFIX}_sample_out/${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G
                endif
            else
                if ( $VP9 == 1 ) then
                    echo "${BS_GEN} ${GEN_NAME} ${GEN_OPT} --noblit -r ${REF_YUV} ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} --noblit -r ${REF_YUV} ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} >& $LOGFILE_G
                else if( $H264 == 1 ) then
                    echo "${BS_GEN} $GEN_NAME ${GEN_OPT} -i ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} -i ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G
                else
                    echo "${BS_GEN} $GEN_NAME ${GEN_OPT} -b ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV}"
                    ${BS_GEN} ./${GEN_NAME} ${GEN_OPT} -b ${CODEC_PREFIX}_sample_in/${PATTEST}.${GEN_EXT} -o ${REF_YUV} >& $LOGFILE_G
                endif

            endif
            if ($status != 0) then
              set SIM_ERR_FLAG = 1
            endif
            sed -i "s/COUNT: X/COUNT: 1/" $LOGFILE_G
        endif
    endif


    if ($FPGA_ENV_FLAG == 0) then
        set SIZE = `grep "PIC WIDTH" $LOGFILE_G | tail -1 | sed -r "s/\[//" | sed -r 's/\].*$//'`
        set HSIZE = `echo "$SIZE" | sed -r 's/^.*WIDTH://' | sed -r 's/,.*$//'`
        set VSIZE = `echo "$SIZE" | sed -r 's/^.*HEIGHT://' | sed -r 's/,.*$//'`
        if ( -f ${PATTEST}.ctl ) then
            set HSIZE_R = `grep x_pic_size ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g' | tr -d '\r'`
            set VSIZE_R = `grep y_pic_size ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g' | tr -d '\r'`
            set INPUT_YUV = `grep app_input_yuv_file ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g' | tr -d '\r'`
        endif
        set FRAME_NUM = `echo "$SIZE" | sed -r 's/^.*COUNT://' | sed -r 's/,.*$//'`

        if ( $H264 == 1 ) then
            set EXE_NUM   = `echo "$SIZE" | sed -r 's/^.*PICS://' | sed -r 's/,.*$//'`

            set FORMAT = `grep -m 1 -i "Color Format" $LOGFILE_G | sed -e 's/.*\s:\s\(.*\)\s\+(.*/\1/' | sed -e s/://g`
            set DEPTH = `grep -m 1 -i "Color Format" $LOGFILE_G | sed -e 's/.*\s:\s.*\s(\([0-9]*\):.*/\1/'`
            if ( $FORMAT == 400 ) then
                set FORMAT = 420
            endif

            if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
                ## 16x16
                set FCPC_CTB = 0
            endif
        else
            set EXE_NUM   = ${FRAME_NUM}

            if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
                set FCPC_CTB = `grep "CTB" $LOGFILE_G | tail -1 | sed -r "s/\[//" | sed -r 's/^.*CTB SIZE : //' | sed -r 's/\].*$//'`
                if ( ${FCPC_CTB} == "16x16" ) then
                    set FCPC_CTB = 0
                else if ( ${FCPC_CTB} == "32x32" ) then
                    set FCPC_CTB = 1
                else
                    set FCPC_CTB = 2
                endif
            endif
        endif
        
        if (( $FRAME_NUM == "" ) || ( $EXE_NUM == "" )) then
            echo "Can't get information (HSIZE,VSIZE,FRAME_NUM)"
            set SIM_ERR_FLAG = 1

            set HSIZE = 176
            set VSIZE = 144
            set FORMAT = 420
            set DEPTH =  8
            set FRAME_NUM = "-1"
            set EXE_NUM = "-1"
            if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
                set FCPC_CTB = 2
            endif
        endif
    endif
    set wait = 0

    if ( $H264 == 0 ) then
        set FORMAT = 420
        set DEPTH =  8
    endif

    ln -f -s ${LOGFILE_V} .
    #-------------------
    # Compare
    #-------------------
    rm -f verify.csh
    if ( $CMP_MD5_FLAG == 1 ) then
        ln -fs ${CODEC_PREFIX}_sample_out/${PATTEST}.md5 out.md5
        if ( -e ${GA_DIR}/${GA_CE_NUM}ce/${PATTEST}_ok.md5 ) then
          ln -fs ${GA_DIR}/${GA_CE_NUM}ce/${PATTEST}_ok.md5 ref.md5
        else
          ln -fs ${GA_DIR}/${PATTEST}_ok.md5 ref.md5
        endif
    else
        if ( $DECODE_FLAG ) then
            cp ${CODEC_PREFIX}_sample_out/${PATTEST}.yuv out.yuv
        else
        #Encode
            if ( $H265 == 1 ) then
                cp ${CODEC_PREFIX}_sample_out/${PATTEST}_ldec.yuv out.yuv
            else
                ln -fs ../out.yuv ${CODEC_PREFIX}_sample_out/${PATTEST}_ldec.yuv
            endif
        endif
    endif

    if ( $ENCODE_FLAG == 1 ) then
        if ( $wait == 1 ) then
            @ num_wait = 0
            while ( $num_wait < 3 )
                if ( $wait == 0 ) then
                    break
                endif
                echo "wait gen_enc $num_wait"
                sleep 30
                @ num_wait++
            end
        endif
        if ( $H265 == 1 ) then
            mv ${CODEC_PREFIX}_sample_out/${PATTEST}.${EXTENSION} out.${EXTENSION}
            ln -fs ../out.${EXTENSION} ${CODEC_PREFIX}_sample_out/${PATTEST}.${EXTENSION}
            mv ${CODEC_PREFIX}_sample_out/${PATTEST}_um.${EXTENSION} ref.${EXTENSION}
        else
            mv ./${PATTEST}.${EXTENSION} out.${EXTENSION}
            ln -fs ../out.${EXTENSION} ${CODEC_PREFIX}_sample_out/${PATTEST}.${EXTENSION}
        endif
        if ( $UM_DEBUG ) then
            rm -f "hw.log"
            rm -f "model.log"
            ./${GEN_NAME} -b ${WORK_DIR}/${S_DIR}/${STIMS[$i]}/out.${EXTENSION} --LogOutput=7 >& hw.log
            ./${GEN_NAME} -b ${WORK_DIR}/${S_DIR}/${STIMS[$i]}/ref.${EXTENSION} --LogOutput=7 >& model.log
        else if ( $AVC_DEBUG ) then
            rm -f "hw.log"
            #rm -f "model.log"
            ./${GEN_NAME} -i ${WORK_DIR}/${S_DIR}/${STIMS[$i]}/out.${EXTENSION} -lg >& hw.log
            #./${GEN_NAME} -i ${WORK_DIR}/${S_DIR}/${STIMS[$i]}/ref.${EXTENSION} -lg >& model.log
        endif
    endif

    set VLCS_VERIFY_OPT = "0"
    if ( $CMP_MD5_FLAG == 1 ) then
        mv top_md5_verify.csh verify.csh

    else if ( $CETEST_FLAG == 1 ) then
        if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
            cat ce_fcpc_verify.csh | sed -e s/FCPC_CTB/$FCPC_CTB/ | sed -e s/HSIZE/$HSIZE/ | sed -e s/VSIZE/$VSIZE/ | sed -e s/FRAME_NUM/$FRAME_NUM/ | sed -e s/EXE_NUM/$EXE_NUM/ | sed -e s/FORMAT/$FORMAT/ | sed -e s/DEPTH/$DEPTH/ | sed -e s/DISP_MODE/$USE_DISP/ | sed -e s/ENCODE_FLAG/$ENCODE_FLAG/ | sed -e s/CODEC/$CODEC/ > verify.csh
        else
            cat ce_verify.csh | sed -e s/HSIZE/$HSIZE/ | sed -e s/VSIZE/$VSIZE/ | sed -e s/FRAME_NUM/$FRAME_NUM/ | sed -e s/EXE_NUM/$EXE_NUM/ | sed -e s/FORMAT/$FORMAT/ | sed -e s/DEPTH/$DEPTH/ | sed -e s/DISP_MODE/$USE_DISP/ > verify.csh
        endif

        ln -s -f ${LOGFILE_G} .
    else if ( $VLCS_SIM ) then
        if ( $ENCODE_FLAG ) then
            set VLCS_VERIFY_OPT = "${EXE_NUM} -enc"
            if ( -e no_gen ) then
                # We cannot compare YUV file because no input stream
                set VLCS_VERIFY_OPT = "${VLCS_VERIFY_OPT} -no_yuv"
            endif
        else
            cat vlcs_verify.csh | sed -e s/FRAME_NUM/$FRAME_NUM/ | sed -e s/EXE_NUM/$EXE_NUM/ > verify.csh
        endif
        printf "\n\n **** %s RESULTS ****\n\n" ${PATTEST}
    else if ( $VLC_INSERT_SIM_FLAG == 1 ) then
        cat vlc_insert_verify.csh | sed -e s/PAT_OUT/$PATTEST/ > verify.csh
    else if ( ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1 ) then
        cat top_fcpc_verify.csh | sed -e s/FCPC_CTB/$FCPC_CTB/ | sed -e s/HSIZE/$HSIZE/ | sed -e s/VSIZE/$VSIZE/ | sed -e s/FRAME_NUM/$FRAME_NUM/ | sed -e s/EXE_NUM/$EXE_NUM/ | sed -e s/FORMAT/$FORMAT/ | sed -e s/DEPTH/$DEPTH/ | sed -e s/DISP_MODE/$USE_DISP/ | sed -e s/ENCODE_FLAG/$ENCODE_FLAG/ | sed -e s/CODEC/$CODEC/ > verify.csh
        ln -s -f ${LOGFILE_G} .
    else if ( $ENCODE_FLAG == 1 ) then
        cat top_enc_verify.csh | sed -e s/HSIZE_R/$HSIZE_R/ | sed -e s/VSIZE_R/$VSIZE_R/ | sed -e s/INPUT_YUV/$INPUT_YUV/ | sed -e s/HSIZE/$HSIZE/ | sed -e s/VSIZE/$VSIZE/ | sed -e s/FRAME_NUM/$FRAME_NUM/ | sed -e s/EXE_NUM/$EXE_NUM/ | sed -e s/PAT_LOG/$PATTEST.log/ > verify.csh
    else
        cat top_verify.csh | sed -e s/HSIZE/$HSIZE/ | sed -e s/VSIZE/$VSIZE/ | sed -e s/FRAME_NUM/$FRAME_NUM/ | sed -e s/EXE_NUM/$EXE_NUM/ | sed -e s/FORMAT/$FORMAT/ | sed -e s/DEPTH/$DEPTH/ | sed -e s/DISP_MODE/$USE_DISP/ > verify.csh
        ln -s -f ${LOGFILE_G} .
    endif

    #-------------------------------
    # get MAX CLOCK info. and so on
    #-------------------------------
    grep "time out" ${LOGFILE_V} >> $RESFILE
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
    endif
    grep "No more capture frame" ${LOGFILE_V} >> $RESFILE
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
    endif
    grep "DesignWare Model" ${LOGFILE_V} >> $RESFILE
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
    endif
    grep "AXI checker Error" ${LOGFILE_V} >> $RESFILE
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
    endif
    if (($SKIP_GEN_FLAG == 1) || ( $ERROR_FLAG == 1) || ( $EXCEPT_CE_ERR_DETECT == 1 )) then
        grep "HW-ERROR" ${LOGFILE_V} >> $RESFILE
    else
        grep "ERROR_STATUS_FOUND" ${LOGFILE_V} >> $RESFILE
        if ($status == 0) then
            set SIM_ERR_FLAG = 1
        endif
    endif
    grep "Unsupported image size" ${LOGFILE_V} >> $RESFILE
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
    endif
    grep "ERROR" ${LOGFILE_V} | grep "INTERLACE" >> $RESFILE
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
    endif

    grep "Error before VLC" ${LOGFILE_V} | grep "Low delay mode but HEVC tile" > /dev/null
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
        echo "Low Delay Mode but HEVC Tile" >> $RESFILE
    endif

    grep "FCPC parameter error" ${LOGFILE_V} > /dev/null
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
        echo "FCPC parameter error" >> $RESFILE
    endif

    grep "assertion" ${LOGFILE_V} > /dev/null
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
        echo "Assertion FAIL" >> $RESFILE
    endif

    grep "Read/Write access conflict" ${LOGFILE_V} > /dev/null
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
        echo "SRAM RW Conflict" >> $RESFILE
    endif

    grep "WARNING to HW" ${LOGFILE_V} > /dev/null
    if ($status == 0) then
        set SIM_ERR_FLAG = 1
        echo "WARNING to HW" >> $RESFILE
    endif

    if ( ${ERROR_FLAG} == 0 ) then
        if ( ${EXCEPT_CE_ERR_DETECT} == 0 ) then
            grep "CE ERROR" ${LOGFILE_V} >> $RESFILE
            if ($status == 0) then
                set SIM_ERR_FLAG = 1
            endif
        endif
        if ( ${EXCEPT_VL_ERR_DETECT} == 0 ) then
            grep "VLC ERROR" ${LOGFILE_V} >> $RESFILE
            if ($status == 0) then
                set SIM_ERR_FLAG = 1
            endif
        endif
    endif
    #-------------------------------
    # compress log
    #-------------------------------
    if ( ${FPGA_ENV_FLAG} == 1 ) then
        grep -v duration ${LOGFILE_V} | grep -v SDRAM | grep -v VP_VLC_STATUS | sed -e s/"\[EVENT\]\[BELIZE\]"// > ${LOGFILE_V}.tmp
        grep -v endian ${LOGFILE_V}.tmp | grep -v VP_CE_REF_CNT | grep -v refList > ${LOGFILE_V}
        rm -f ${LOGFILE_V}.tmp
    endif

    if ( -e ${REF_YUV} ) then
      chmod 664 ${REF_YUV}
    endif

    if ($VLCS_SIM && $ENCODE_FLAG ) then
        vlcs_verify.pl $VLCS_VERIFY_OPT | tee -a $RESFILE
    else
        if ( $CETEST_FLAG == 1 ) then
            set VERIFY_OPT = ""
        else
            set VERIFY_OPT = "-ce_num ${CE_NUM}"
            if ( $LD_MODE == 1 ) then
                set VERIFY_OPT = "${VERIFY_OPT} -ld"
            endif
        endif

        if ($FCPC_DUMP_FRM == 1 && ${FCPC_SIM} == 1 && $FCPC_COMP_EN == 1) then
            set VERIFY_OPT = "${VERIFY_OPT} -fcpc_dump_frm"
        endif

        chmod +x verify.csh
        ./verify.csh ${VERIFY_OPT} | tee -a $RESFILE
    endif

    if ( ${FPGA_ENV_FLAG} == 0 ) then
        #-------------------------
        # get common results
        #-------------------------
        echo "<<< Cycle Check >>>" >> $RESFILE
        echo "" >> $RESFILE
        grep CYCLE ${LOGFILE_V} >> $RESFILE
        echo ""  >> $RESFILE
        echo ""  >> $RESFILE

        #-----------------------------
        # insert end flag ( tentative )
        #-----------------------------
        if ( $VLCS_SIM == 0 ) then
            echo "*** dump file compare (DUT vs REF) ***"  >> $RESFILE
            echo ""  >> $RESFILE
            echo " <<< SBUS Right Dump Check >>> "  >> $RESFILE
            echo ""  >> $RESFILE
            echo " <<< SBUS Left Dump Check >>> "  >> $RESFILE
            echo ""  >> $RESFILE
            echo ""  >> $RESFILE
        endif

        #-----------------------------
        # PSNR Check for encode
        #-----------------------------
        if ( $ENCODE_FLAG == 1 && $VLCS_SIM == 0 ) then
            echo " <<< PSNR Check >>> "  >> $RESFILE
            echo " "  >> $RESFILE
            egrep "PSNR [YUV]|AVG" $PSNR_LOG  >> $RESFILE
            echo " "  >> $RESFILE
            echo " "  >> $RESFILE
        endif

        if ( $PING == 1 ) then
            set HSIZE = `grep -E -e "x_pic_size\s+" ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g'`
            set VSIZE = `grep -E -e "y_pic_size\s+" ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g'`
            set IPATH = `grep -E -e "app_output_path\s+" ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g'`
            set INAME = `grep -E -e "app_output_ldec_file\s+" ${PATTEST}.ctl | sed -e 's/.*=//g' | sed -e 's/;//g' | sed -e 's/ //g'`
            set ONAME = "${PATTEST}_ld.png"
            #echo "convert -size ${HSIZE}x${VSIZE} -sampling-factor 4:2:0 -depth 8 ${CODEC_PREFIX}_sample_out/${INAME} ${ONAME}"
            convert -size ${HSIZE}x${VSIZE} -sampling-factor 4:2:0 -depth 8 ${IPATH}/${INAME} ${ONAME}
        endif
    endif

    #-----------------------------
    # stream Check for encode
    #-----------------------------
    #if ( $ENCODE_FLAG == 1 && $VLCS_SIM == 0 ) then
    #    echo " <<< Stream Change Check >>> "  >> $RESFILE
    #    echo ""  >> $RESFILE
    #    if ( -e ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/${PATTEST}.${EXTENSION} ) then
    #        \diff ${WORK_DIR}/${S_DIR}/${STIMS[$i]}/${PATTEST}.${EXTENSION} ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/${PATTEST}.${EXTENSION} > prevdiff.txt
    #        if ( -z prevdiff.txt ) then
    #            echo "Compare with previous encoded stream: Equivalent"   >> $RESFILE
    #        else
    #            echo "Compare with previous encoded stream: Changed"   >> $RESFILE
    #        endif
    #      rm -rf prevdiff.txt
    #    else
    #        echo "Compare with previous encoded stream: N/A"   >> $RESFILE
    #    endif
    #    echo ""  >> $RESFILE
    #    echo ""  >> $RESFILE
    #endif

    #if ( $ENCODE_FLAG == 1 && $VLCS_SIM == 0 ) then
    #    echo " <<< Stream Change Check (Golden) >>> "  >> $RESFILE
    #    echo ""  >> $RESFILE
    #    if ( -e ${WORK_DIR}/${R_DIR}_ga/${STIMS[$i]}/${PATTEST}.${EXTENSION} ) then
    #        \diff ${WORK_DIR}/${S_DIR}/${STIMS[$i]}/${PATTEST}.${EXTENSION} ${WORK_DIR}/${R_DIR}_ga/${STIMS[$i]}/${PATTEST}.${EXTENSION} > prevdiff.txt
    #        if ( -z prevdiff.txt ) then
    #            echo "Compare with golden encoded stream: Equivalent"   >> $RESFILE
    #        else
    #            echo "Compare with golden encoded stream: Changed"   >> $RESFILE
    #        endif
    #        rm -rf prevdiff.txt
    #    else
    #        echo "Compare with golden encoded stream: N/A"   >> $RESFILE
    #    endif
    #    echo ""  >> $RESFILE
    #    echo ""  >> $RESFILE
    #endif

    #-----------------------------
    # output executed revision
    #-----------------------------
    if ( -e ${REVFILE} ) then
        cat ${REVFILE} >> $RESFILE
        echo ""  >> $RESFILE
        echo ""  >> $RESFILE
    endif

    #-------------------------
    # Copy logfile in LOG_DIR
    #-------------------------
    if ( $ENCODE_FLAG == 1  && $VLCS_SIM == 0 ) then
        if ( $H265 == 1 ) then
            mv ${CODEC_PREFIX}_sample_out/${PATTEST}.log ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/.
        else
            mv ./${PATTEST}.log ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/.
        endif
        ln -f -s ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/${PATTEST}.log .
    endif
    if ( -e mb_cycle_cnt_ce.log ) then
        mv mb_cycle_cnt_ce.log ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/.
        ln -f -s ${WORK_DIR}/${L_DIR}/${STIMS[$i]}/mb_cycle_cnt_ce.log .
    endif
    # param_out to dat
    if ($PARAM_OUT) then
        foreach s (param_out/*.bin)
            tools/bin2hex $s $s:r.dat
            rm -f $s
        end
    endif

    #-----------------
    # coverage
    #-----------------
    if ( ${GCOV} == 1 ) then
        ${BS_CMD} tar czf ${COV_RESULT}.tar.gz ${COV_RESULT} && rm -rf ${COV_RESULT}
        mv ${COV_RESULT}.tar.gz ${WORK_DIR}/${C_DIR}/${STIMS[$i]}/
    endif

    # return to work
    popd > /dev/null

    #-----------------
    # REMOVE_DIR_FLAG
    #-----------------
    if ( $ENCODE_FLAG == 1 ) then
        if ( $VLCS_SIM ) then
            mv ${S_DIR}/${STIMS[$i]}/out.${EXTENSION} ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/.
            ln -f -s ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/out.${EXTENSION} ${S_DIR}/${STIMS[$i]}/.
        else
            mv ${S_DIR}/${STIMS[$i]}/out.${EXTENSION} ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/out.${EXTENSION}
            ln -f -s ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/out.${EXTENSION} ${S_DIR}/${STIMS[$i]}/.
            if ( $H265 == 1 ) then
                # UM
                mv ${S_DIR}/${STIMS[$i]}/ref.${EXTENSION} ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/ref.${EXTENSION}
                ln -f -s ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/ref.${EXTENSION} ${S_DIR}/${STIMS[$i]}/.
            endif
            if ( $UM_DEBUG || $AVC_DEBUG ) then
                mv ${S_DIR}/${STIMS[$i]}/model.log ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/.
                mv ${S_DIR}/${STIMS[$i]}/hw.log ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/.
                ln -f -s ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/model.log ${S_DIR}/${STIMS[$i]}/.
                ln -f -s ${WORK_DIR}/${R_DIR}/${STIMS[$i]}/hw.log ${S_DIR}/${STIMS[$i]}/.
            endif
        endif
    endif
    ln -f -s ${RESFILE} ${S_DIR}/${STIMS[$i]}/.
    ln -f -s ${LOGFILE_V} ${S_DIR}/${STIMS[$i]}/.
    if ( ${FPGA_ENV_FLAG} == 0 ) then
        ln -sf ${LOGFILE_G} ${S_DIR}/${STIMS[$i]}/.
        #ln -sf ${REFLOG} ${S_DIR}/${STIMS[$i]}/.
    endif
    set SIM_END_STAT = `grep FAIL $RESFILE | grep -c -v CYCLE`
    if ( $SIM_END_STAT != 0 ) then
        set SIM_ALL_STAT = 1
    endif
    if ($SIM_ERR_FLAG != 0) then
        set SIM_ALL_STAT = 1
    endif
    if ( $REMOVE_DIR_FLAG == 1 ) then
        if ($NO_CYCLE_CHK == 1) then
            set SIM_END_STAT = `grep FAIL $RESFILE | grep -c -v CYCLE`
        else
            set SIM_END_STAT = `grep -c FAIL $RESFILE`
        endif
        set SIM_ISICS_RES_STAT = `grep "Stream > FAIL" $RESFILE`
        if (( ${SIM_END_STAT} == 0 ) || ( ${FORCE_RM_DIR_FLAG} == 1 )) then
            echo "remove ${S_DIR}/${STIMS[$i]}"
            rm -rf ${S_DIR}/${STIMS[$i]}/*
        else if ( ${SIM_ISICS_RES_STAT} == 0 ) then
            echo "remove only IS and ICS ${S_DIR}/${STIMS[$i]}"
            rm -f ${S_DIR}/${STIMS[$i]}/*istream*.bin*
            rm -f ${S_DIR}/${STIMS[$i]}/*icstream*.bin*
        endif
    endif

    # Delete dec result if "-v/cfresetfrm"
    if ( ($RESET_SIM == 6) || ($RESET_SIM == 7) ) then
        set SIM_END_STAT = `grep -c FAIL $RESFILE`
        if ( ${SIM_END_STAT} == 0 ) then
            echo "remove ${S_DIR}/${STIMS[$i]}/out.*"
            rm -rf ${S_DIR}/${STIMS[$i]}/out.*
        endif
    endif

    if( $GZIP_FLAG == 1 ) then
        if ( -e ${S_DIR}/${STIMS[$i]}/results.log ) then
            echo "gzip -r ${S_DIR}/${STIMS[$i]} &"
            find ${S_DIR}/${STIMS[$i]} -type f | xargs gzip -r &
        endif
    endif

    ## re-make result link
    if ( $REMOVE_DIR_FLAG == 1 ) then
        if (( ${SIM_END_STAT} == 0 ) || ( ${FORCE_RM_DIR_FLAG} == 1 )) then
            ln -f -s ${RESFILE} ${S_DIR}/${STIMS[$i]}/.
        endif
    endif

    # Delete dec result if "-cycle_file"
    if ( $CYCLE_FILE == 1 ) then
        rm -f ${S_DIR}/${STIMS[$i]}/out.*
    endif

    next_pat:
    @ i++
    if ( ${DISABLE_FCP_RESET} ) then
      set DISABLE_FCP_RESET = 0
      set SYSC_OPT = "${SYSC_OPT} -disable_fcpc_reset"
    endif
end

exit $SIM_ALL_STAT


#-----------------------------------------
# Reg RW sim End
#-----------------------------------------
reg_end:
    # return to work
    popd > /dev/null

    ln -f -s ${RESFILE} ${S_DIR}/${STIMS[$i]}/.
    ln -f -s ${LOGFILE_V} ${S_DIR}/${STIMS[$i]}/.
    ln -f -s ${LOGFILE_G} ${S_DIR}/${STIMS[$i]}/.
    exit
