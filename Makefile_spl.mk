#
# (C) Copyright 2000-2011
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2011
# Daniel Schwierzeck, daniel.schwierzeck@googlemail.com.
#
# (C) Copyright 2011
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
# Based on top-level Makefile.
#
CONFIG_SPL_BUILD := y  
export CONFIG_SPL_BUILD

src := $(obj)

ifeq "$(BUILD_DIR)" ""
BUILD_DIR=.
endif

# Create output directory if not already present
_dummy := $(shell [ -d $(obj) ] || mkdir -p $(obj))

-include include/config/auto.conf
-include $(obj)/include/autoconf.mk

KBUILD_CPPFLAGS += -DCONFIG_SPL_BUILD
ifeq ($(CONFIG_TPL_BUILD),y)
KBUILD_CPPFLAGS += -DCONFIG_TPL_BUILD
endif

ifeq ($(CONFIG_TPL_BUILD),y)
SPL_BIN := u-boot-tpl
else
SPL_BIN := u-boot-spl
endif

include config.mk

# Enable garbage collection of un-used sections for SPL
KBUILD_CFLAGS += -ffunction-sections -fdata-sections
LDFLAGS_FINAL += --gc-sections

# FIX ME
cpp_flags := $(KBUILD_CPPFLAGS) $(PLATFORM_CPPFLAGS) $(UBOOTINCLUDE) \
							$(NOSTDINC_FLAGS)

HAVE_VENDOR_COMMON_LIB = $(if $(wildcard $(srctree)/board/$(VENDOR)/common/Makefile),y,n)

libs-y += $(CPUDIR)/lib$(CPU).a
ifdef SOC
libs-y += $(CPUDIR)/$(SOC)/lib$(SOC).a
endif
libs-y += arch/$(ARCH)/lib/lib$(ARCH).a
libs-y += $(if $(BOARDDIR),board/$(BOARDDIR)/lib$(BOARD).a)
libs-$(CONFIG_SPL_LIBCOMMON_SUPPORT) += common/libcommon.a
libs-$(CONFIG_SPL_LIBGENERIC_SUPPORT) += lib/libgeneric.a
libs-$(CONFIG_SPL_SERIAL_SUPPORT) += drivers/serial/libserial.a
libs-$(CONFIG_SPL_SPI_SUPPORT) += drivers/gpio/libgpio.a
libs-$(CONFIG_SPL_SPI_SUPPORT) += drivers/spi/libspi.a
libs-$(CONFIG_SPL_SPI_SUPPORT) += drivers/mtd/spi/libspi_flash.a
libs-$(CONFIG_SPL_SPI_SUPPORT) += drivers/mtd/libmtd.a
libs-$(CONFIG_SPL_SPINAND_SUPPORT) += drivers/gpio/libgpio.a
libs-$(CONFIG_SPL_SPINAND_SUPPORT) += drivers/spi/libspi.a
libs-$(CONFIG_SPL_SPINAND_SUPPORT) += drivers/mtd/spi-nand/libspi_nandflash.a
libs-$(CONFIG_SPL_SPINAND_SUPPORT) += drivers/mtd/libmtd.a

head-y		:= $(addprefix ,$(obj)/$(head-y))
libs-y		:= $(addprefix ,$(obj)/$(libs-y))

u-boot-spl-dirs	:= $(patsubst %/,%,$(filter %/, $(libs-y)))

libs-y := $(patsubst %/, %/built-in.o, $(libs-y))

spl-libs := $(notdir $(libs-y))

# Add GCC lib
ifeq ($(CONFIG_USE_PRIVATE_LIBGCC),y)
PLATFORM_LIBGCC = arch/$(ARCH)/lib/lib.a
PLATFORM_LIBS := $(filter-out %/lib.a, $(filter-out -lgcc, $(PLATFORM_LIBS))) $(PLATFORM_LIBGCC)
endif

u-boot-spl-init := $(head-y)
u-boot-spl-main := $(libs-y)

# Linker Script
LDSCRIPT :=

ifdef CONFIG_SPL_LDSCRIPT
# need to strip off double quotes
LDSCRIPT := $(addprefix $(SRCTREE)/,$(CONFIG_SPL_LDSCRIPT:"%"=%))
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(SRCTREE)/board/$(BOARDDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(SRCTREE)/$(CPUDIR)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(SRCTREE)/arch/$(ARCH)/cpu/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
	LDSCRIPT := $(SRCTREE)/u-boot-spl.lds
endif
ifeq ($(wildcard $(LDSCRIPT)),)
$(error could not find linker script)
endif

# Special flags for CPP when processing the linker script.
# Pass the version down so we can handle backwards compatibility
# on the fly.
LDPPFLAGS += \
	-include $(srctree)/include/u-boot/u-boot.lds.h \
	-include $(objtree)/include/config.h \
	-DCPUDIR=$(CPUDIR) \
	$(shell $(LD) --version | \
	  sed -ne 's/GNU ld version \([0-9][0-9]*\)\.\([0-9][0-9]*\).*/-DLD_MAJOR=\1 -DLD_MINOR=\2/p')

quiet_cmd_mkimage = MKIMAGE $@
cmd_mkimage = $(objtree)/tools/mkimage $(MKIMAGEFLAGS_$(@F)) -d $< $@ \
	$(if $(KBUILD_VERBOSE:1=), >/dev/null)

MKIMAGEFLAGS_MLO = -T omapimage -a $(CONFIG_SPL_TEXT_BASE)

MKIMAGEFLAGS_MLO.byteswap = -T omapimage -n byteswap -a $(CONFIG_SPL_TEXT_BASE)

MLO MLO.byteswap: $(obj)/u-boot-spl.bin
	$(call if_changed,mkimage)

ifeq ($(CONFIG_SYS_SOC),"at91")
MKIMAGEFLAGS_boot.bin = -T atmelimage

ifeq ($(CONFIG_SPL_GENERATE_ATMEL_PMECC_HEADER),y)
MKIMAGEFLAGS_boot.bin += -n $(shell $(obj)/../tools/atmel_pmecc_params)

boot.bin: $(obj)/../tools/atmel_pmecc_params
endif

boot.bin: $(obj)/u-boot-spl.bin
	$(call if_changed,mkimage)
else
MKIMAGEFLAGS_boot.bin = -T zynqimage

spl/boot.bin: $(obj)/u-boot-spl-dtb.bin
	$(call if_changed,mkimage)
endif

ALL-y	+= $(obj)/$(SPL_BIN).bin

ifdef CONFIG_SPL_OF_CONTROL
ALL-$(CONFIG_OF_SEPARATE) += $(obj)/$(SPL_BIN)-pad.bin $(obj)/$(SPL_BIN)-dtb.bin
endif

ifdef CONFIG_SAMSUNG
ALL-y	+= $(obj)/$(BOARD)-spl.bin
endif

ifdef CONFIG_ARCH_SOCFPGA
ALL-y	+= $(obj)/$(SPL_BIN)-dtb.sfp
endif

ifdef CONFIG_SUNXI
ALL-y	+= $(obj)/sunxi-spl.bin
endif

ifeq ($(CONFIG_SYS_SOC),"at91")
ALL-y	+= boot.bin
endif

ifdef CONFIG_ARCH_ZYNQ
ALL-y	+= $(obj)/boot.bin
endif

all:	$(ALL-y)

quiet_cmd_cat = CAT     $@
cmd_cat = cat $(filter-out $(PHONY), $^) > $@

$(obj)/$(SPL_BIN)-dtb.bin: $(obj)/$(SPL_BIN).bin $(obj)/$(SPL_BIN)-pad.bin \
		$(obj)/$(SPL_BIN).dtb FORCE
	$(call if_changed,cat)

# Create a file that pads from the end of u-boot-spl.bin to bss_end
$(obj)/$(SPL_BIN)-pad.bin: $(obj)/$(SPL_BIN)
	@bss_size_str=$(shell $(NM) $< | awk 'BEGIN {size = 0} /__bss_size/ \
	{size = $$1} END {print "ibase=16; " toupper(size)}' | bc); \
	dd if=/dev/zero of=$@ bs=1 count=$${bss_size_str} 2>/dev/null;

# Pass the original device tree file through fdtgrep twice. The first pass
# removes any unwanted nodes (i.e. those which don't have the
# 'u-boot,dm-pre-reloc' property and thus are not needed by SPL. The second
# pass removes various unused properties from the remaining nodes.
# The output is typically a much smaller device tree file.
quiet_cmd_fdtgrep = FDTGREP $@
      cmd_fdtgrep = $(objtree)/tools/fdtgrep -b u-boot,dm-pre-reloc -RT $< \
		-n /chosen -O dtb | \
	$(objtree)/tools/fdtgrep -r -O dtb - -o $@ \
		$(addprefix -P ,$(subst $\",,$(CONFIG_OF_SPL_REMOVE_PROPS)))

$(obj)/$(SPL_BIN).dtb: dts/dt.dtb
	$(call cmd,fdtgrep)

quiet_cmd_cpp_cfg = CFG     $@
cmd_cpp_cfg = $(CPP) -Wp,-MD,$(depfile) $(cpp_flags) $(LDPPFLAGS) -ansi \
	-DDO_DEPS_ONLY -D__ASSEMBLY__ -x assembler-with-cpp -P -dM -E -o $@ $<

$(obj)/$(SPL_BIN).cfg:	include/config.h
	$(call if_changed,cpp_cfg)

ifdef CONFIG_SAMSUNG
ifdef CONFIG_VAR_SIZE_SPL
VAR_SIZE_PARAM = --vs
else
VAR_SIZE_PARAM =
endif
$(obj)/$(BOARD)-spl.bin: $(obj)/u-boot-spl.bin
	$(if $(wildcard $(objtree)/spl/board/samsung/$(BOARD)/tools/mk$(BOARD)spl),\
	$(objtree)/spl/board/samsung/$(BOARD)/tools/mk$(BOARD)spl,\
	$(objtree)/tools/mkexynosspl) $(VAR_SIZE_PARAM) $< $@
endif

quiet_cmd_objcopy = OBJCOPY $@
cmd_objcopy = $(OBJCOPY) $(OBJCOPYFLAGS) $(OBJCOPYFLAGS_$(@F)) $< $@

OBJCOPYFLAGS_$(SPL_BIN).bin = $(SPL_OBJCFLAGS) -O binary

$(obj)/$(SPL_BIN).bin: $(obj)/$(SPL_BIN) FORCE
	$(call if_changed,objcopy)

LDFLAGS_$(SPL_BIN) += -T $(LDSCRIPT) $(LDFLAGS_FINAL)
ifneq ($(TEXT_BASE),)
LDFLAGS_$(SPL_BIN) += -Ttext $(TEXT_BASE)
endif

ifdef CONFIG_ARCH_SOCFPGA
MKIMAGEFLAGS_$(SPL_BIN)-dtb.sfp = -T socfpgaimage
$(obj)/$(SPL_BIN)-dtb.sfp: $(obj)/$(SPL_BIN)-dtb.bin FORCE
	$(call if_changed,mkimage)
endif

ifdef CONFIG_SUNXI
quiet_cmd_mksunxiboot = MKSUNXI $@
cmd_mksunxiboot = $(objtree)/tools/mksunxiboot $< $@
$(obj)/sunxi-spl.bin: $(obj)/$(SPL_BIN).bin
	$(call if_changed,mksunxiboot)
endif

quiet_cmd_u-boot-spl = LD      $@
      cmd_u-boot-spl = (cd $(obj) && $(LD) $(LDFLAGS_$(@F)) \
		       $(patsubst $(obj)/%,%,$(u-boot-spl-init)) --start-group \
		       $(spl-libs) \
		       --end-group \
		       $(PLATFORM_LIBS) -Map $(SPL_BIN).map -o $(SPL_BIN))

$(obj)/$(SPL_BIN): $(u-boot-spl-init) $(u-boot-spl-main) $(obj)/u-boot-spl.lds FORCE
	$(call if_changed,u-boot-spl)
	$(cmd_u-boot-spl)

$(libs-y):	depend $(SUBDIRS)
	$(MAKE) -C $(dir $(subst $(obj)/,,$@))

$(SUBDIRS):	depend
	$(warning subdir $@)
	$(MAKE) -C $@ all

depend dep:	$(TIMESTAMP_FILE) $(VERSION_FILE) $(BUILD_DIR)/include/autoconf.mk
	for dir in $(SUBDIRS) $(CPUDIR) $(dir $(LDSCRIPT)) ; do \
		$(MAKE) -C $$dir _depend ; done
		
#$(sort $(u-boot-spl-init) $(u-boot-spl-main)): $(u-boot-spl-dirs) ;

PHONY += $(u-boot-spl-dirs)
$(u-boot-spl-dirs):
	$(Q)$(MAKE) $(build)=$@

quiet_cmd_cpp_lds = LDS     $@
cmd_cpp_lds = $(CPP) -Wp,-MD,$(depfile) $(cpp_flags) $(LDPPFLAGS) -ansi \
		-D__ASSEMBLY__ -x assembler-with-cpp -P -o $@ $<

$(obj)/u-boot-spl.lds: $(LDSCRIPT) FORCE
	$(call if_changed_dep,cpp_lds)

# read all saved command lines

targets := $(wildcard $(sort $(targets)))
cmd_files := $(wildcard $(obj)/.*.cmd $(foreach f,$(targets),$(dir $(f)).$(notdir $(f)).cmd))

ifneq ($(cmd_files),)
  $(cmd_files): ;	# Do not try to update included dependency files
  include $(cmd_files)
endif

PHONY += FORCE
FORCE:

# Declare the contents of the .PHONY variable as phony.  We keep that
# information in a variable so we can use it in if_changed and friends.
.PHONY: $(PHONY)
