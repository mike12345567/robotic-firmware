PARTICLE_BIN=node_modules/.bin/particle

APPDIR=../../../src

PLATFORM?=photon

TARGET_FILE=robotic-firmware
TARGET_DIR=../../../build/

APP=robotic-firmware

ifeq ("$(NO_MOVEMENT)", "y")
CPPFLAGS_EX+=-DNO_MOVEMENT
endif

ifeq ("$(WHEEL_CASTER)", "y")
CPPFLAGS_EX+=-DWHEEL_CASTER
endif

ifeq ("$(ONE_WHEEL_ROTATION)", "y")
CPPFLAGS_EX+=-DONE_WHEEL_ROTATION
endif

export CPPFLAGS=$(CPPFLAGS_EX)

all elf bin hex size clean:
	@$(MAKE) -C particle/firmware/main PLATFORM=$(PLATFORM) APP=$(APP) APPDIR=$(APPDIR) TARGET_DIR=$(TARGET_DIR) TARGET_FILE=$(TARGET_FILE) v=1 $@

program:
	@$(MAKE) -C particle/firmware/main PLATFORM=$(PLATFORM) APP=$(APP) APPDIR=$(APPDIR) TARGET_DIR=$(TARGET_DIR) TARGET_FILE=$(TARGET_FILE) v=1 $(NEED_TO_CLEAN) all program-dfu

upgrade:
	@$(MAKE) -C particle/firmware/modules PLATFORM=$(PLATFORM) APP=$(APP) APPDIR=$(APPDIR) TARGET_DIR=$(TARGET_DIR) TARGET_FILE=$(TARGET_FILE) v=1 clean all program-dfu
