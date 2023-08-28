# --------------------------------------------------------------------------------------------------
# Config
# --------------------------------------------------------------------------------------------------
# this has to be defined by the makefile that includes this one
#PROJECT_NAME =

MAJOR_VER ?= 1
MINOR_VER ?= 0
BUILD_NUM ?= 0

COMPLETE_VERSION := $(MAJOR_VER).$(MINOR_VER).b$(BUILD_NUM)

RM := rm -f
RMDIR := rm -rf
MKDIR := mkdir -p
CP := cp
DD := dd

CXXVERSION := $(shell $(CXX) --version | grep g++ | sed 's/^.* //g')
DOXYGEN_VERSION := $(shell doxygen -version)


# --------------------------------------------------------------------------------------------------
# Compile and link flags
# --------------------------------------------------------------------------------------------------
CFG ?= release

ifeq ($(CFG),release)
	OPTIMIZATION_FLAGS = -O3
else ifeq ($(CFG),debug)
	OPTIMIZATION_FLAGS = -O0 -g3
else
	$(error error: CFG must be 'release' or 'debug', it was '$(CFG)')
endif


POSTCOMPILE = @mv -f $(DEPS_DIR)/$*.Td $(DEPS_DIR)/$*.d && touch $@

PREPROCESSOR_DEFINES += \
-DMAJOR_VER=$(MAJOR_VER) \
-DMINOR_VER=$(MINOR_VER) \
-DBUILD_NUM=$(BUILD_NUM) 

WARNING_FLAGS = -Wall -Wextra -pedantic -Werror -Wno-psabi
OTHER_FLAGS = -c -std=c++14 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections

# OPTIMIZATION_FLAGS:
# -O0 / -O2: optimization level (0: no optimization, good for debug builds, 2: level 2 optimization)
# -g3: compile with level 3 debug information
#
# WARNING_FLAGS:
# -Wall: enable a lot of warnings
# -Wextra: enable even more warnings than those enabled with -Wall 
# -pedantic: issue all the warnings demanded by strict ISO c and ISO c++
# -Werror: consider all warnings as errors
# -Wno-psabi: used to remove an annoying warning about an abi change regarding the implmentation of 
#		functions that handle var_args (maybe this flag disappeared in more recent versions of gcc...)
#
# OTHER_FLAGS:
# -c: compile to object files without linking
# -std=c++14: uses c++14 features
# -fmessage-length=n: Try to format error messages so that they fit on lines of about n characters. The default is 72 characters 
#		for g++ and 0 for the rest of the front ends supported by GCC. If n is zero, then no line-wrapping is done; each error 
#		message appears on a single line.
# -fsigned-char: is needed in OTHER_FLAGS because we usually make the (wrong) assumption that 'char' is signed.
# 		anyway, the sign of 'char' depends on the compiler implementation, for example with msvc it is signed by default
# 		while with gcc it is unsigned by default
# -ffunction-sections and -fdata-sections: used to indicate to the linker where are functions and data sections 
#		in object files, so that it will be able to remove unused code and data



CXXFLAGS = $(OPTIMIZATION_FLAGS) $(WARNING_FLAGS) $(PREPROCESSOR_DEFINES) $(OTHER_FLAGS)


LDFLAGS = -Wl,--gc-sections # --gc-sections used to remove unused code and data


# --------------------------------------------------------------------------------------------------
# Directories
# --------------------------------------------------------------------------------------------------
DOCS_DIR = ../docs

BUILD_ROOT_DIR = ./build
BUILD_DIR := $(BUILD_ROOT_DIR)/$(CFG)

BIN_ROOT_DIR = ./bin
BIN_DIR := $(BIN_ROOT_DIR)/$(CFG)


DEPS_DIR := $(BUILD_DIR)/deps

DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPS_DIR)/$*.Td

# add all directories that contain a .c/.cpp file here
 # this has to be defined by the makefile that includes this one
SRC_DIRS +=


# add additional include directories here
 # this has to be defined by the makefile that includes this one
INC_DIRS +=

# add additional library paths here
# this has to be defined by the makefile that includes this one
LIB_DIRS +=

vpath %.cpp $(SRC_DIRS)

# --------------------------------------------------------------------------------------------------
# Objects and sources
# --------------------------------------------------------------------------------------------------
CPP_SOURCES += $(foreach dir,$(SRC_DIRS),$(notdir $(wildcard $(dir)/*.cpp)))

OBJS += $(patsubst %.cpp,%.o,$(filter %.cpp,$(CPP_SOURCES)))

 # this has to be defined by the makefile that includes this one
LIBS +=

BINARY_NAME := $(BIN_DIR)/$(PROJECT_NAME)


# --------------------------------------------------------------------------------------------------
# Targets
# --------------------------------------------------------------------------------------------------
.PHONY: all clean cleandeps cleanall cleandriver documentation dos2unix

.DELETE_ON_ERROR:


all: $(BINARY_NAME)


# App


$(BUILD_DIR)/%.o: %.cpp ./$(DEPS_DIR)/%.d | $(BUILD_DIR) $(DEPS_DIR)
	@echo 'Building file: $<'
	@echo 'Invoking: G++ Compiler v$(CXXVERSION)'
	$(CXX) $(CXXFLAGS) $(INC_DIRS) $(DEPFLAGS) -o "$@" "$<"
	$(POSTCOMPILE)
	@echo 'Finished building: $<'
	@echo ' '


$(BINARY_NAME): $(addprefix $(BUILD_DIR)/,$(OBJS)) | $(BIN_DIR)
	@echo 'Building target: $@'
	@echo 'Updated prerequisites: $?'
	@echo 'Invoking: G++ Linker v$(CXXVERSION)'
	$(CXX) $(LDFLAGS) -o "$@" $^ $(LIB_DIRS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '


documentation:
	@echo 'Generating documentation...'
	$(MAKE) -C $(DOCS_DIR)
	@echo 'Done.'
	@echo ' '


dos2unix:
	@echo 'Converting newline characters to LF in source files...'
	dos2unix $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.h))
	dos2unix $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.cpp))
	@echo 'Done converting newline characters'
	@echo ' '



$(BUILD_DIR):
	@echo 'Creating $@ directory...'
	$(MKDIR) $@
	@echo ' '


$(BIN_DIR):
	@echo 'Creating $@ directory...'
	$(MKDIR) $@
	@echo ' '


$(DEPS_DIR):
	@echo 'Creating $@ directory...'
	$(MKDIR) $@
	@echo ' '



$(DEPS_DIR)/%.d: ;
.PRECIOUS: $(DEPS_DIR)/%.d


include $(wildcard $(patsubst %,$(DEPS_DIR)/%.d,$(basename $(CPP_SOURCES))))


clean:
	@echo 'Removing all files for CFG=$(CFG)'
	-$(RMDIR) $(BUILD_DIR)/* $(BIN_DIR)/*
	@echo 'Finished $@ for CFG=$(CFG)'
	@echo ' '

cleandeps:
	@echo 'Removing all dependency files for CFG=$(CFG)'
	-$(RM) $(DEPS_DIR)/*
	@echo 'Finished $@ for CFG=$(CFG)'
	@echo ' '

cleandocs:
	@echo 'Removing all documentation files'
	$(MAKE) -C $(DOCS_DIR) clean
	@echo 'Finished $@'
	@echo ' '

cleanall: cleandocs
	@echo 'Removing all files for all configurations'
	-$(RMDIR) $(BUILD_ROOT_DIR)/* $(BIN_ROOT_DIR)/*
	@echo 'Finished $@'
	@echo ' '
