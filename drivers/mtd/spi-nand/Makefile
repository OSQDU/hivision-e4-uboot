
include $(TOPDIR)/config.mk

LIB	:= $(obj)libspi_nandflash.a

COBJS-$(CONFIG_FH_SPI_NAND) += spi-nand-base.o spi-nand-bbt.o spi-nand-device.o
COBJS-$(CONFIG_FH_SPI_NAND) += spi-nand-ids.o spi-nand-util.o

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_SPINAND_SUPPORT)	+= spi-nand-spl-load.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
