SDK_DEMO_PATH ?= .
BL_SDK_BASE ?= $(SDK_DEMO_PATH)/../../bl61x_SDK/os/

export BL_SDK_BASE

# NOW_REAL_PATH:= $(realpath ./)

CHIP ?= bl616
# BOARD ?= m61_evb_s1
BOARD ?= bl616dk
CROSS_COMPILE ?= riscv64-unknown-elf-
# BOARD_DIR = ${NOW_REAL_PATH}

# add custom cmake definition
#cmake_definition+=-Dxxx=sss

include $(BL_SDK_BASE)/project.build
