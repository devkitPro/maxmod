#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

ifeq ($(SYSTEM),)
$(error "Please specify SYSTEM, valid systems are: GBA" )
endif

ifeq ($(SYSTEM),GBA)

BUILD		:=	build_gba

SOURCES		:=	source source_gba
TARGET		:=	$(TOPDIR)/lib/libmm.a
include $(DEVKITARM)/gba_rules
GOODSYSTEM	:= YES

endif

ifneq ($(GOODSYSTEM),YES)
$(error "Invalid SYSTEM, valid systems are: GBA" )
endif

INCLUDES	:=	asm_include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

ifeq ($(SYSTEM),GBA)

DEFS	:=	-DSYS_GBA -DUSE_IWRAM
ARCH	:=	$(XMEM) -march=armv4t -mthumb -mthumb-interwork

endif

ASFLAGS	:=	$(DEFS) $(ARCH) $(INCLUDE)
LDFLAGS	=	$(ARCH) -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export TOPDIR	:=	$(CURDIR)
export DEPSDIR	:=	$(CURDIR)/$(BUILD)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

SFILES			:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export OFILES	:=	$(SFILES:.s=.o)

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LD	:=	$(CC)

.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@[ -d lib ] || mkdir -p lib
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/maxmod.mak

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) *.elf

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

$(TARGET)	:	$(OFILES)
	@rm -f "$(TARGET)"
	@$(AR) -rc "$(TARGET)" $(OFILES)
	@$(PREFIX)strip -x "$(TARGET)"
	@echo built ... $(SYSTEM) lib

-include $(DEPENDS)

#---------------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------------
