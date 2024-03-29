#------------------------------------------------------------------------------
# Clear the implicit built in rules
#------------------------------------------------------------------------------
.SUFFIXES:
#------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules
#include $(DEVKITPPC)/gamecube_rules

#------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#------------------------------------------------------------------------------
TARGET		:=	$(notdir $(CURDIR))
BUILD		:=	build
SOURCES		:=	source
DATA		:=	data  
INCLUDES	:=

#------------------------------------------------------------------------------
# Specify whether to use the following libraries
#------------------------------------------------------------------------------
USE_FREETYPE	:=	Y
USE_JPEG	:=	N

#------------------------------------------------------------------------------
# Set FREETYPE to the location of the Freetype libarary
#------------------------------------------------------------------------------
FREETYPE	:=	/opt/devkitpro/libfreetype
FREETYPE_INC	:=	$(FREETYPE)/include
FREETYPE_LIB	:=	$(FREETYPE)/lib
ifeq ($(USE_FREETYPE),Y)
    FREETYPE_L	:=	-lfreetype
endif

#------------------------------------------------------------------------------
# Set JPEG to the location of the JPEG libarary
#------------------------------------------------------------------------------
JPEG		:=	/opt/devkitpro/libjpeg
JPEG_INC	:=	$(JPEG)/include
JPEG_LIB	:=	$(JPEG)/lib/wii
ifeq ($(USE_JPEG),Y)
    JPEG_L	:=	-ljpeg
endif

#------------------------------------------------------------------------------
# options for code generation
#------------------------------------------------------------------------------
CFLAGS		:=	-g -O2 -mrvl -Wall $(MACHDEP) $(INCLUDE)
#CFLAGS		:=	-g -O2 -mogc -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS	:=	$(CFLAGS)
LDFLAGS		:=	-g $(MACHDEP) -mrvl -Wl,-Map,$(notdir $@).map

#------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#------------------------------------------------------------------------------
LIBS	:=	-logc -lm $(FREETYPE_L) $(JPEG_L)

#------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#------------------------------------------------------------------------------
LIBDIRS	:=

#------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#------------------------------------------------------------------------------
# automatically build a list of object files for our project
#------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(addsuffix .o,$(BINFILES)) \
					$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#------------------------------------------------------------------------------
# build a list of include paths
#------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC) \
					-I$(FREETYPE_INC) -I$(JPEG_INC) 

#------------------------------------------------------------------------------
# build a list of library paths
#------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB) \
					-L$(FREETYPE_LIB) -L$(JPEG_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol source/tags

#------------------------------------------------------------------------------
run:
	psoload $(TARGET).dol

#------------------------------------------------------------------------------
reload:
	psoload -r $(TARGET).dol


#------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#------------------------------------------------------------------------------
# main targets
#------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#------------------------------------------------------------------------------
%.jpg.o	:	%.jpg
#------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#------------------------------------------------------------------------------
endif
#------------------------------------------------------------------------------

tags:
	@( cd source; ctags *.c *.h /opt/devkitpro/libogc/include/*.h \
	    /opt/devkitpro/libogc/include/mad/*.h \
	    /opt/devkitpro/libogc/include/ogc/*.h \
	    /opt/devkitpro/libogc/include/modplay/*.h \
	    /opt/devkitpro/libogc/include/sdcard/*.h \
	    /opt/devkitpro/libjpeg/include/jpeg/*.h \
	    /opt/devkitpro/libfreetype/include/*.h \
	    /opt/devkitpro/libfreetype/include/freetype/*.h )
