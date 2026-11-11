# Native builds use the official devkitPPC platform rules and libogc.
# make PLATFORM=wii   /   make PLATFORM=gamecube
.SUFFIXES:
PLATFORM ?= wii
ifeq ($(strip $(DEVKITPPC)),)
$(error DEVKITPPC is not set. Open the devkitPro MSYS2 terminal)
endif
ifeq ($(PLATFORM),wii)
include $(DEVKITPPC)/wii_rules
PLATFORM_DEFINE := -DPLATFORM_WII
PLATFORM_LIBS := -lwiiuse -lbte
else ifeq ($(PLATFORM),gamecube)
include $(DEVKITPPC)/gamecube_rules
PLATFORM_DEFINE := -DPLATFORM_GAMECUBE
else
$(error PLATFORM must be wii or gamecube)
endif
TARGET := bernas-career-$(PLATFORM)
BUILD := build-$(PLATFORM)
SOURCES := source
INCLUDES := include
CXXFLAGS := -std=gnu++17 -O2 -Wall -Wextra -Wno-misleading-indentation -ffunction-sections -fdata-sections $(MACHDEP) $(PLATFORM_DEFINE) $(INCLUDE)
LDFLAGS := $(MACHDEP) -Wl,--gc-sections,-Map,$(TARGET).map
LIBS := -lfat $(PLATFORM_LIBS) -logc -lm
ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/source
export DEPSDIR := $(CURDIR)/$(BUILD)
export LD := $(CXX)
export OFILES := main.o
export INCLUDE := -I$(CURDIR)/include -I$(LIBOGC_INC)
export LIBPATHS := -L$(LIBOGC_LIB)
.PHONY: all clean $(BUILD)
all: $(BUILD)
$(BUILD):
	@mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile PLATFORM=$(PLATFORM)
clean:
	rm -rf build-wii build-gamecube bernas-career-wii.elf bernas-career-gamecube.elf bernas-career-wii.dol bernas-career-gamecube.dol
else
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)
-include $(OFILES:.o=.d)
endif
