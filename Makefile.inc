#-----------------------------------------------------------------------------
#
#  TSDuck - The MPEG Transport Stream Toolkit
#  Copyright (c) 2005-2021, Thierry Lelegard
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
#  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
#  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
#  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
#  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
#  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
#  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
#  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGE.
#
#-----------------------------------------------------------------------------
#
#  Common makefile definitions for the TSDuck project.
#  To be included in all makefiles in the project.
#
#  General note: To speed up make recursion, some variables are exported when
#  their definition take some time.
#
#-----------------------------------------------------------------------------

# If no target precedes the inclusion of this file, use "default" as target.

.PHONY: first default
first: default
	@true

# Some nerds define exotic shells as default. Stay to a known shell.
# Skip initialization files to speed up and reproduceability.

SHELL := /usr/bin/env bash --noprofile --norc

# The directory which contains the currently included Makefile is the project root.

ROOTDIR := $(abspath $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))

# Project specific directories.

INSTALLERDIR = $(ROOTDIR)/installers
SCRIPTSDIR   = $(ROOTDIR)/scripts
SRCROOT      = $(ROOTDIR)/src
LIBTSDUCKDIR = $(SRCROOT)/libtsduck
TSTOOLSDIR   = $(SRCROOT)/tstools
TSPLUGINSDIR = $(SRCROOT)/tsplugins

# Enforce English locale by default for predictible output.

export LANGUAGE = en_US.UTF-8
export LC_ALL = en_US.UTF-8
export LANG = en_US.UTF-8

# Representation of a space and a comma character (make oddities...)

EMPTY =
SPACE = $(EMPTY) $(EMPTY)
COMMA = ,

# The function F_SEARCH searches a command in $PATH and in a few other predefined
# directories. Return the full path of the first found, empty if not found.
# Example: $(call F_SEARCH,ls)

F_SEARCH = $(firstword $(wildcard $(addsuffix /$(1),$(subst :, ,$(PATH)) /usr/bin /usr/sbin /usr/local/bin /opt/homebrew/bin $(HOME)/bin)))

# The funtion F_COMMAND searches a command with F_SEARCH. If the command is not
# found, return the second parameter as default value. The following example
# searches for a command named "gsed" and, if not found, use command "sed" as
# fallback: $(call F_COMMAND,gsed,sed)

F_COMMAND = $(eval _tmp := $(call F_SEARCH,$(1)))$(if $(_tmp),$(_tmp),$(2))

# Find GNU alternatives to sed, grep. fgrep, chmod.

ifndef SED
    export SED   := $(call F_COMMAND,gsed,sed)
    export GREP  := $(call F_COMMAND,ggrep,grep)
    export FGREP := $(call F_COMMAND,gfgrep,fgrep)
    export CHMOD := $(call F_COMMAND,gchmod,chmod)
endif

# Recursive invocations of make should be silent.

MAKEFLAGS += --no-print-directory

# Check if "make -k" is specified (ie. continue on error).

CONTINUE := $(if $(findstring k,$(filter-out --%,$(MAKEFLAGS))),true,)

# The function F_RECURSE recurses the current make targets in the specified subdirectories.

F_RECURSE = for dir in $(1); do if [[ -d $$dir ]]; then $(MAKE) -C $$dir $@ $(if $(CONTINUE),,|| exit $$?); fi; done

# Operating system, architecture, host name.
# $(LOCAL_ARCH) is the local processor exact architecture ("i686", "x86_64", "armv7l", etc.)
# $(MAIN_ARCH) is the local processor architecture family ("i386", "x86_64", "arm", etc.)

ifndef LOCAL_OS
    export LOCAL_OS   := $(shell uname -s | tr A-Z a-z)
    export LOCAL_ARCH := $(shell uname -m)
    export HOSTNAME   := $(shell hostname | $(SED) -e 's/\..*//')
endif
MACOS := $(if $(findstring darwin,$(LOCAL_OS)),true)
MAC := $(MACOS)

# Logical CPU and physical core count.
# $(CORE_COUNT) contains the number of physical CPU cores in the machine.
# $(CPU_COUNT) contains the number of logical CPU's in the machine.
# With hyperthreaded CPU cores, $(CPU_COUNT) is typically twice $(CORE_COUNT).

ifndef CPU_COUNT
    ifdef MACOS
        export CPU_COUNT  := $(shell sysctl -n hw.logicalcpu 2>/dev/null)
        export CPU_COUNT  := $(if $(CPU_COUNT),$(CPU_COUNT),1)
        export CORE_COUNT := $(shell sysctl -n hw.physicalcpu 2>/dev/null)
        export CORE_COUNT := $(if $(CORE_COUNT),$(CORE_COUNT),$(CPU_COUNT))
    else
        export CPU_COUNT  := $(shell nproc 2>/dev/null)
        export CPU_COUNT  := $(if $(CPU_COUNT),$(CPU_COUNT),1)
        export CORE_COUNT := $(shell $(FGREP) -m1 'cpu cores' /proc/cpuinfo 2>/dev/null | cut -d ' ' -f3)
        export CORE_COUNT := $(if $(CORE_COUNT),$(CORE_COUNT),$(CPU_COUNT))
    endif
endif

# Compute best multiprocessor make option if unspecified.
# Warning, this must be a delayed eval ("=", not ":=").

MAKEFLAGS_SMP = $(if $(filter -j%,$(MAKEFLAGS)),$(filter -j%,$(MAKEFLAGS)),-j$(CPU_COUNT))

# Detect Cygwin platform. Impacts:
# - The executables are started by the Windows loader which uses distinct environment
#   variables for shared library resolution.
# - The option -fPIC generates a stupid error:
#   "-fPIC ignored for target (all code is position independent)"
# - $(LD_LIBRARY_PATH_NAME) is the name of the environment variable containing
#   the search path for the shareable libraries. Typically LD_LIBRARY_PATH on
#   native unices and PATH on Windows-based environments such as Cygwin.

ifneq ($(findstring cygwin,$(LOCAL_OS)),)
    CYGWIN = true
    LOCAL_OS := cygwin
    LD_LIBRARY_PATH_NAME = PATH
    CXXFLAGS_FPIC =
else
    CYGWIN =
    LD_LIBRARY_PATH_NAME = LD_LIBRARY_PATH
    CXXFLAGS_FPIC = -fPIC
endif

# Executable files and shared objects suffix.

EXE_SUFFIX = $(if $(CYGWIN),.exe,)
SO_SUFFIX = $(if $(CYGWIN),.dll,$(if $(MACOS),.dylib,.so))

# Define compilation flags for 32-bit cross-compilation.
# Target architecture is skipped on ARM because of numerous variants.

ifdef M32
    override MAIN_ARCH := i386
    override TARGET_FLAGS := -march=i686
    CXXFLAGS_M32 := -m32
    LDFLAGS_M32  := -m32
else ifndef MAIN_ARCH
    MAIN_ARCH    := $(shell uname -m | $(SED) -e 's/i.86/i386/' -e 's/arm.*/arm/')
    TARGET_FLAGS :=
    CXXFLAGS_M32 :=
    LDFLAGS_M32  :=
endif
export MAIN_ARCH TARGET_FLAGS

# Use $(CXX) for compilation. Use $(GCC) to explicitly reference GCC.

GCC ?= gcc

# Cross-compilation support.
# If $(CROSS) is defined (any non-empty value), perform a cross-compilation build.
# $(CROSS_TARGET) is the target name of the cross-compilation (e.g. arm-unknown-linux-gnueabi).
# If undefined but $(CROSS) is defined, we locate the first GCC executable with path
# $(CROSS_PREFIX)/TARGET/bin/TARGET-gcc.

ifeq ($(CROSS)$(CROSS_TARGET),)
    CXXFLAGS_CROSS =
    LDFLAGS_CROSS =
else
    CROSS ?= true
    # Cross-compilation tools are in /usr/local by default.
    CROSS_PREFIX ?= /usr/local
    # If cross target undefined, find the first one.
    # We look for a file pattern: PREFIX/TARGET/bin/TARGET-gcc
    CROSS_TARGET := $(if $(CROSS_TARGET),$(CROSS_TARGET),$(shell \
        ls $(CROSS_PREFIX)/*/bin/*-gcc 2>/dev/null | \
        $(GREP) '$(CROSS_PREFIX)//*\([^/]*\)/bin/\1-gcc' | \
        $(SED) -e 's|^$(CROSS_PREFIX)//*||' -e 's|/.*$$||' | \
        head -1))
    $(if $(CROSS_TARGET),,$(error CROSS is defined but no cross-compilation tool-chain was found))
    # Adjust target. Use "arm" as main target if CROSS_TARGET starts with "arm".
    override MAIN_ARCH := $(if $(findstring @arm,@$(CROSS_TARGET)),arm,$(CROSS_TARGET))
    override TARGET_FLAGS :=
    override GCC_VERSION_DONE :=
    # Redirect build tools.
    CXX = $(CROSS_PREFIX)/$(CROSS_TARGET)/bin/$(CROSS_TARGET)-g++
    GCC = $(CROSS_PREFIX)/$(CROSS_TARGET)/bin/$(CROSS_TARGET)-gcc
    LD  = $(CROSS_PREFIX)/$(CROSS_TARGET)/bin/$(CROSS_TARGET)-ld
    # Add options. The layout can be different, so use them all.
    CXXFLAGS_CROSS = -I$(CROSS_PREFIX)/$(CROSS_TARGET)/include \
                     -I$(CROSS_PREFIX)/$(CROSS_TARGET)/$(CROSS_TARGET)/include \
                     -I$(CROSS_PREFIX)/$(CROSS_TARGET)/$(CROSS_TARGET)/libc/include
    LDFLAGS_CROSS  = -L$(CROSS_PREFIX)/$(CROSS_TARGET)/lib \
                     -L$(CROSS_PREFIX)/$(CROSS_TARGET)/$(CROSS_TARGET)/lib \
                     -L$(CROSS_PREFIX)/$(CROSS_TARGET)/$(CROSS_TARGET)/libc/lib
endif

# Check if we are on a x86_64 Linux distro using /usr/lib64 as library path.
# Define USELIB64 as "true" or empty. Linuxbrew does not use lib64.

USELIB64 := $(if $(subst linux-x86_64,,$(LOCAL_OS)-$(MAIN_ARCH)),,$(if $(LINUXBREW),,$(if $(wildcard /usr/lib64/libc.so*),true,)))

# $(SYSPREFIX) is the installation prefix. If not set on input, it is
# defined as /usr/local or /opt/homebrew on macOS and /usr on Linux.
# On macOS, the Homebrew root is used without system privileges.
#
# $(USRLIBDIR) is the library directory for the target architecture.
# This is $(SYSPREFIX)/lib64 for Linux x86_64 where /usr/lib64/libc.so* exists
# and $(SYSPREFIX)/lib elsewhere.
#
# $(UDEVDIR) is the directory for system-defined udev rules. $(ETCDIR) is
# the system configuration directory. This is /etc if $(SYSPREFIX) is /usr
# and $(SYSPREFIX)/etc otherwise.

SYSPREFIX ?= $(if $(MACOS)$(LINUXBREW),$(ALTDEVROOT),/usr)
ETCDIR    ?= $(if $(subst /usr,,$(SYSPREFIX)),$(SYSPREFIX)/etc,/etc)
USRLIBDIR ?= $(if $(USELIB64),$(SYSPREFIX)/lib64,$(SYSPREFIX)/lib)
UDEVDIR   ?= $(if $(subst /usr,,$(SYSPREFIX)),$(SYSPREFIX)/etc/udev/rules.d,/lib/udev/rules.d)

# All binaries are built into one single root directory named
# bin/release|debug-arch-hostname where bin is located in the top-level directory.
# The variable $(BINDIR) points to this directory. The variable $(OBJDIR) points
# to the directory where object files are stored
#
# $(BINDIR_SUFFIX) is a fixed suffix to add to generated $(BINDIR) subdirectory name.
# It is ignored if $(BINDIR) is defined as input variable. Duplicate the output tree
# using $(BINDIR_SUFFIX) for specific compilers or profiling tools.

ifdef GCOV
    BINDIR_SUFFIX += -gcov
endif
ifdef GPROF
    BINDIR_SUFFIX += -gprof
endif
ifdef LLVM
    BINDIR_SUFFIX += -clang
endif
ifdef STATIC
    BINDIR_SUFFIX += -static
endif

ifneq ($(BINDIR),)
    # BINDIR is specified in input, transform it into an absolute path for recursion.
    INBINDIR := $(BINDIR)
    override BINDIR := $(abspath $(INBINDIR))
    MAKEOVERRIDES := $(subst BINDIR=$(INBINDIR),BINDIR=$(BINDIR),$(MAKEOVERRIDES))
else ifneq ($(DEBUG),)
    BINDIR = $(ROOTDIR)/bin/debug-$(MAIN_ARCH)$(if $(HOSTNAME),-$(HOSTNAME),)$(BINDIR_SUFFIX)
else
    BINDIR = $(ROOTDIR)/bin/release-$(MAIN_ARCH)$(if $(HOSTNAME),-$(HOSTNAME),)$(BINDIR_SUFFIX)
endif

OBJDIR = $(BINDIR)/objs-$(notdir $(CURDIR))

# TSDuck library files.

STATIC_LIBTSDUCK = $(BINDIR)/libtsduck.a
SHARED_LIBTSDUCK = $(BINDIR)/libtsduck$(SO_SUFFIX)

# Get current compiler version, usually in form x.y.z

ifeq ($(GCC_VERSION_DONE),)
    export GCC_VERSION_DONE = true
    export GCC_VERSION := $(shell $(GCC) -dumpversion 2>/dev/null)
    export GCC_MAJOR := $(firstword $(subst .,$(SPACE),$(GCC_VERSION)))
    export LLVM_VERSION := $(shell clang -dumpversion 2>/dev/null)
    export LLVM_MAJOR := $(firstword $(subst .,$(SPACE),$(LLVM_VERSION)))
    ifeq ($(shell test 0$(GCC_MAJOR) -ge 5; echo $$?),0)
        export GCC_5_OR_MORE := true
    else
        GCC_5_OR_MORE :=
        unexport GCC_5_OR_MORE
    endif
    ifeq ($(shell test 0$(GCC_MAJOR) -ge 6; echo $$?),0)
        export GCC_6_OR_MORE := true
    else
        GCC_6_OR_MORE :=
        unexport GCC_6_OR_MORE
    endif
    ifeq ($(shell test 0$(GCC_MAJOR) -ge 8; echo $$?),0)
        export GCC_8_OR_MORE := true
    else
        GCC_8_OR_MORE :=
        unexport GCC_8_OR_MORE
    endif 
    ifeq ($(shell test 0$(LLVM_MAJOR) -ge 10; echo $$?),0)
        export LLVM_10_OR_MORE := true
    else
        LLVM_10_OR_MORE :=
        unexport LLVM_10_OR_MORE
    endif
    ifeq ($(shell test 0$(LLVM_MAJOR) -ge 12; echo $$?),0)
        export LLVM_12_OR_MORE := true
    else
        LLVM_12_OR_MORE :=
        unexport LLVM_12_OR_MORE
    endif
    export USE_GCC := $(if $(shell $(CXX) --version 2>/dev/null | grep -i -e gcc -e g++),true,)
    export USE_LLVM := $(if $(shell $(CXX) --version 2>/dev/null | grep -i clang),true,)
endif

# Forced usage of LLVM (clang).

ifdef LLVM
    export USE_GCC :=
    export USE_LLVM := true
    CXX = clang++
endif

# Compilation flags for various types of optimization.
# $(CXXFLAGS_FULLSPEED) contains the C/C++ optimization options for maximum
# execution speed, at the expense of code size. Can be used in the makefile
# to specify that some selected modules should be compiled for full speed. Example:
# $(OBJDIR)/fast1.o $(OBJDIR)/fast2.o: CXXFLAGS_OPTIMIZE = $(CXXFLAGS_FULLSPEED)
# $(CXXFLAGS_OPTSIZE) contains the C/C++ optimization options for minimum code size.

CXXFLAGS_OPTIMIZE ?= -O2 -fno-strict-aliasing
CXXFLAGS_FULLSPEED = -O3 -fno-strict-aliasing -funroll-loops -fomit-frame-pointer
CXXFLAGS_OPTSIZE   = -Os -fno-strict-aliasing

# On macOS, Homebrew development packages are installed in /usr/local (Intel) or /opt/homebrew (Arm).
# On Linux, when Homebrew is forced, use it as alternative development root.
# if $(LINUXBREW) is defined (any non-empty value), force a compilation in a Homebrew
# environment on Linux. Homebrew on Linux (aka "Linuxbrew") installs a non-standard environment
# where libraries are fetched from $(HOMEBREW_PREFIX) instead of /usr.
# By default, ignore the Homebrew environment on Linux and build using system libraries.
# $(ALTDEVROOT) is the alternative root for development tools and library. This can be
# the Homebrew root or "/usr" if no other value is suitable so that $(ALTDEVROOT) can
# always be safely used to check files.

ifneq ($(MACOS)$(LINUXBREW),)
    $(if $(LINUXBREW),$(if $(HOMEBREW_PREFIX),,$(error LINUXBREW is defined but not HOMEBREW_PREFIX)),)
    ALTDEVROOT := $(if $(HOMEBREW_PREFIX),$(HOMEBREW_PREFIX),$(if $(wildcard /opt/homebrew/bin),/opt/homebrew,/usr/local))
    CXXFLAGS_INCLUDES += -I$(ALTDEVROOT)/include
    LDLIBS := -L$(ALTDEVROOT)/lib $(LDLIBS)
else
    ALTDEVROOT := /usr
endif

# Always use maximal or even paranoid warning mode.
# With clang, the option -Weverything turns everything on. There is no such option with GCC.

CXXFLAGS_WARNINGS = -Werror
ifdef USE_LLVM
    CXXFLAGS_WARNINGS += -Weverything -Wno-c++98-compat-pedantic
    ifdef MACOS
        # On macOS, it is normal to include from /usr/local/include since some libraries come from Homebrew.
        # Starting with clang 12, this generates a warning we need to disable. However, this disable option
        # generates an error with previous versions. And we cannot disable this warning inside the code since
        # this is a command-line-level error. So, we must check the version here...
        ifdef LLVM_12_OR_MORE
            CXXFLAGS_WARNINGS += -Wno-poison-system-directories
        endif
    endif
else
    CXXFLAGS_WARNINGS += \
        -Wall -Wextra -Wformat-nonliteral -Wformat-security -Wswitch-default -Wuninitialized \
        -Wfloat-equal -Wundef -Wpointer-arith -Woverloaded-virtual -Wctor-dtor-privacy \
        -Wnon-virtual-dtor -Wsign-promo -Wzero-as-null-pointer-constant -Wstrict-null-sentinel
    ifdef GCC_5_OR_MORE
        CXXFLAGS_WARNINGS += -Wshadow -Wpedantic -Weffc++
    endif
    ifdef GCC_6_OR_MORE
        CXXFLAGS_WARNINGS += -Wsuggest-override
    endif
    ifeq ($(MAIN_ARCH),arm)
        CXXFLAGS_WARNINGS += -Wno-psabi
    endif
endif

# Language levels.

CXXFLAGS_STANDARD ?= -std=c++11

# Compilation flags for security.

CXXFLAGS_SECURITY = -fstack-protector-all

# Compilation flags in debug mode.

ifdef DEBUG
    CXXFLAGS_DEBUG = -g -DDEBUG=1
    LDFLAGS_DEBUG =
else
    CXXFLAGS_DEBUG = $(CXXFLAGS_OPTIMIZE)
    LDFLAGS_DEBUG =
endif

# Compilation flags for code coverage using gcov.

ifdef GCOV
    CXXFLAGS_GCOV = --coverage -DGCOV=1
    LDFLAGS_GCOV = --coverage
else
    CXXFLAGS_GCOV =
    LDFLAGS_GCOV =
endif

# Compilation flags for code profiling using gprof.

ifdef GPROF
    CXXFLAGS_GPROF = -g -pg -DGPROF=1
    LDFLAGS_GPROF = -g -pg
else
    CXXFLAGS_GPROF =
    LDFLAGS_GPROF =
endif

# Compilation flags for posix threads.

CXXFLAGS_PTHREAD = -pthread
LDFLAGS_PTHREAD = $(if $(MACOS),,-pthread)

# External libraries

LDLIBS += -lstdc++ -lpthread $(if $(MACOS),,-lrt) -lm

# Global compilation flags.
# Additional flags can be passed on the "make" command line using xxFLAGS_EXTRA.

CXXFLAGS = $(CXXFLAGS_DEBUG) $(CXXFLAGS_M32) $(CXXFLAGS_GCOV) $(CXXFLAGS_GPROF) $(CXXFLAGS_WARNINGS) \
           $(CXXFLAGS_SECURITY) $(CXXFLAGS_INCLUDES) $(TARGET_FLAGS) $(CXXFLAGS_FPIC) $(CXXFLAGS_STANDARD) \
           $(CXXFLAGS_CROSS) $(CXXFLAGS_PTHREAD) $(CXXFLAGS_EXTRA)
LDFLAGS  = $(LDFLAGS_DEBUG) $(LDFLAGS_M32) $(LDFLAGS_GCOV) $(LDFLAGS_GPROF) $(TARGET_FLAGS) \
           $(LDFLAGS_CROSS) $(LDFLAGS_PTHREAD) $(LDFLAGS_EXTRA)
ARFLAGS  = rc$(if $(MACOS),,U) $(ARFLAGS_EXTRA)

# $(SOFLAGS) specifies options when creating shared objects (.so or .dylib files).

ifeq ($(MACOS),)
    # Link flags on Linux
    CXXFLAGS_INCLUDES += -I$(LIBTSDUCKDIR)/linux
    LDFLAGS_EXTRA += -Wl,-rpath,'$$ORIGIN',-z,noexecstack
    SOFLAGS = -Wl,-soname=$(notdir $@),-z,noexecstack
else
    # Link flags on macOS
    CXXFLAGS_INCLUDES += -I$(LIBTSDUCKDIR)/mac
    LDFLAGS_EXTRA += -Wl,-rpath,@executable_path -Wl,-rpath,@executable_path/../lib
    SOFLAGS = -install_name '@rpath/$(notdir $@)'
endif

# Compilation rules

$(OBJDIR)/%.o: %.cpp
	@echo '  [CXX] $<'; \
	mkdir -p $(OBJDIR); \
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<
$(BINDIR)/%: $(OBJDIR)/%.o
	@echo '  [LD] $@'; \
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@
$(BINDIR)/%$(SO_SUFFIX): $(OBJDIR)/%.o
	@echo '  [LD] $@'; \
	$(CXX) $(CXXFLAGS) $(SOFLAGS) $(LDFLAGS) $^ $(LDLIBS) -shared -o $@

# Default installation root.
# RPM packagers should override this in the "make install" command line
# to build a temporary system root where TSDuck is installed.

SYSROOT =

# A shortcut-target to rebuild with static linking.
# Not meaningfull everywhere:
# - Static linking with system libraries is not supported on macOS.
# - On Linux, all used libraries must be installed. This is not supported
#   on all distros. On Fedora, you may install "glibc-static libstdc++-static"
#   but there is no static package for curl and pcsclite.

.PHONY: static
static:
	+@$(MAKE) STATIC=true

ifeq ($(STATIC),)
    # Dynamic (default) link
    LDLIBS := -ldl $(LDLIBS)
else ifdef MACOS
    $(error static linking is not supported on macOS)
else
    NOCURL     = true
    NOPCSC     = true
    NODTAPI    = true
    NOSRT      = true
    NORIST     = true
    NOEDITLINE = true
    NOTEST     = true
    CXXFLAGS_INCLUDES += -DTSDUCK_STATIC=1
endif

# External include directories and libraries.

ifneq ($(CROSS)$(CROSS_TARGET),)
    # Some libraries are bypassed in cross-compilation.
    NOCURL     = true
    NOPCSC     = true
    NODTAPI    = true
    NOSRT      = true
    NORIST     = true
    NOEDITLINE = true
endif

ifdef NOTELETEXT
    CXXFLAGS_INCLUDES += -DTS_NOTELETEXT=1
endif

ifdef ASSERTIONS
    CXXFLAGS_INCLUDES += -DTS_KEEP_ASSERTIONS=1
endif

ifeq ($(NOSRT),)
    # SRT not disabled, check if libsrt is available.
    NOSRT := $(if $(wildcard /usr/include/srt/*.h $(ALTDEVROOT)/include/srt/*.h),,true)
endif

ifneq ($(NOSRT),)
    CXXFLAGS_INCLUDES += -DTS_NO_SRT=1
else
    LDLIBS := -lsrt $(LDLIBS)
endif

ifeq ($(NORIST),)
    # RIST not disabled, check if librist is available.
    NORIST := $(if $(wildcard /usr/include/librist/*.h $(ALTDEVROOT)/include/librist/*.h),,true)
endif

ifneq ($(NORIST),)
    CXXFLAGS_INCLUDES += -DTS_NO_RIST=1
else
    LDLIBS := -lrist $(LDLIBS)
endif

ifneq ($(NOEDITLINE),)
    CXXFLAGS_INCLUDES += -DTS_NO_EDITLINE=1
else
    LDLIBS := -ledit $(LDLIBS)
endif

ifeq ($(NOCURL),)
    # curl not disabled, check if available.
    export NOCURL := $(if $(call F_COMMAND,curl-config,),,true)
endif

ifneq ($(NOCURL),)
    CXXFLAGS_INCLUDES += -DTS_NO_CURL=1
else
    ifndef CXXFLAGS_CURL
        export CXXFLAGS_CURL := $(shell curl-config --cflags)
        export LDLIBS_CURL := $(shell curl-config --libs)
    endif
    CXXFLAGS_INCLUDES += $(CXXFLAGS_CURL)
    LDLIBS := $(LDLIBS_CURL) $(LDLIBS)
endif

ifeq ($(NOPCSC)$(MACOS),)
    # PCSC not disabled and not on macOS, check if available. On macOS, it is always available.
    NOPCSC := $(if $(wildcard /usr/include/PCSC/*.h $(ALTDEVROOT)/include/PCSC/*.h),,true)
endif

ifneq ($(NOPCSC),)
    CXXFLAGS_INCLUDES += -DTS_NO_PCSC=1
else ifneq ($(MACOS),)
    # On macOS, use PCSC.framework
    LDLIBS := -framework PCSC $(LDLIBS)
else ifneq ($(LINUXBREW),)
    # PCSC on Linuxbrew
    CXXFLAGS_INCLUDES += -I$(ALTDEVROOT)/include/PCSC
    LDLIBS := -lpcsclite $(LDLIBS)
else
    # PCSC on Linux
    CXXFLAGS_INCLUDES += -I/usr/include/PCSC
    LDLIBS := -lpcsclite $(LDLIBS)
endif

ifneq ($(NODEKTEC),)
    # NODEKTEC is an alternative name for NODTAPI
    NODTAPI = true
endif

ifeq ($(NODTAPI)$(NODTAPI_CHECKED),)
    # Disable DTAPI on platforms where it is not supported.
    NODTAPI := $(if $(shell $(ROOTDIR)/scripts/dtapi-config.sh --support),,true)
    export NODTAPI_CHECKED := true
endif

ifneq ($(NODTAPI),)
    CXXFLAGS_INCLUDES += -DTS_NO_DTAPI=1
endif

ifneq ($(NOGITHUB),)
    CXXFLAGS_INCLUDES += -DTS_NO_GITHUB=1
endif

ifeq ($(CXXFLAGS_JNI),)
    export CXXFLAGS_JNI := $(shell $(SCRIPTSDIR)/java-config.sh --cflags)
endif
CXXFLAGS_INCLUDES += $(CXXFLAGS_JNI)

ifdef BITRATE_FRACTION
    CXXFLAGS_INCLUDES += -DTS_BITRATE_FRACTION=1
endif

ifdef BITRATE_INTEGER
    CXXFLAGS_INCLUDES += -DTS_BITRATE_INTEGER=1
endif

ifdef BITRATE_FLOAT
    CXXFLAGS_INCLUDES += -DTS_BITRATE_FLOAT=1
endif

ifdef BITRATE_FIXED
    CXXFLAGS_INCLUDES += -DTS_BITRATE_FIXED=1
endif

ifdef BITRATE_DECIMALS
    CXXFLAGS_INCLUDES += -DTS_BITRATE_DECIMALS=$(BITRATE_DECIMALS)
endif

# List of libtsduck directories containing private and public headers.

ifndef PUBLIC_INCLUDES
    export ALL_INCLUDES := $(filter-out %/windows %/$(if $(MACOS),linux,mac),$(patsubst %/,%,$(sort $(dir $(wildcard $(LIBTSDUCKDIR)/*.h $(LIBTSDUCKDIR)/*/*.h $(LIBTSDUCKDIR)/*/*/*.h)))))
    export PRIVATE_INCLUDES := $(filter %/private,$(ALL_INCLUDES))
    export PUBLIC_INCLUDES := $(filter-out %/private,$(ALL_INCLUDES))
endif
CXXFLAGS_INCLUDES += $(addprefix -I,$(PUBLIC_INCLUDES))

# List of plugins and tools to build.

ifndef TSPLUGINS
    # All plugins and tools, one source file per plugin or executable.
    TSPLUGINS := $(sort $(notdir $(basename $(wildcard $(TSPLUGINSDIR)/tsplugin_*.cpp))))
    TSTOOLS := $(sort $(notdir $(basename $(wildcard $(TSTOOLSDIR)/ts*.cpp))))
    # Obsolete plugins, were in separate shared libraries, now in libtsduck.so.
    # Maintenance: also update scripts/tsduck.nsi (Windows).
    NO_TSPLUGINS := $(addprefix tsplugin_,dektec drop file fork hls http ip null psi skip srt tables)
    # Build a list of tools and plugins to not build or deinstall from the system tree.
    NO_TSTOOLS :=
    ifdef NODTAPI
        NO_TSTOOLS += tsdektec
    endif
    ifdef MACOS
        # Do not compile HiDes tools and plugins on macOS. Physical tuners are
        # not supported either but the related code (dvb plugin, tslsdvb, tscan) shall
        # be compiled because the tuner emulator facility is available everywhere.
        NO_TSPLUGINS += tsplugin_hides
        NO_TSTOOLS += tshides
    endif
    ifdef NOTELETEXT
        NO_TSPLUGINS += tsplugin_teletext
    endif
    ifdef NORIST
        NO_TSPLUGINS += tsplugin_rist
    endif
    ifdef NOPCSC
        NO_TSTOOLS += tssmartcard
    endif
    TSTOOLS := $(filter-out $(NO_TSTOOLS),$(TSTOOLS))
    TSPLUGINS := $(filter-out $(NO_TSPLUGINS),$(TSPLUGINS))
    export TSPLUGINS TSTOOLS NO_TSTOOLS NO_TSPLUGINS
endif

# Dependency (.dep) files: For each xxx.cpp source file, there is one
# xxx.dep file in the same directory as xxx.o. The file xxx.dep lists
# all .h dependencies for xxx.cpp.
#
# When make is invoked, the .dep files are automatically updated. This is
# required for most targets, those which build files. Some targets are
# informational or do not need to build object files. In this case,
# rebuilding the .dep files is useless but harmless. However, in some
# cases such as the standard "clean" targets, the .dep files should
# not be rebuilt. In fact, they are deleted by the target. In these
# cases, the "clean" targets must not implicitly rebuild the .dep
# files. The standard "clean" targets are automatically added in NODEPS,
# there is no need to set them. However, if a makefile defines some
# non-standard "clean" targets, it should set them in NODEPS before
# inclusion of this make file.
#
# If $(DONT_BUILD_DEPS) is defined, do not attempt to build the header
# dependency files. Useful when the source file depend on dynamically
# generated header files (initially non-existent).
#
# $(NODEPS) lists the make targets in the parent makefile which shall
# not enforce the generation of header dependency files. This is a bit
# complicated and, most of the time, there is no need to set this variable.

$(OBJDIR)/%.dep: %.cpp
	@echo '  [DEP] $<'; \
	mkdir -p $(OBJDIR); \
	$(CXX) -MM $(CPPFLAGS) $(CXXFLAGS_INCLUDES) $(CXXFLAGS_STANDARD) $(CXXFLAGS_NO_WARNINGS) -MT $(OBJDIR)/$*.o -MT $@ $< >$@ || \
	rm -f $@

NODEPS += clean cmacros coverity cppcheck cppcheck-xml cxxmacros distclean doxygen \
          flawfinder cloc listvars scan-build unixify

ifeq ($(DONT_BUILD_DEPS),)
    ifneq ($(MAKECMDGOALS),)
        DONT_BUILD_DEPS := $(if $(filter-out $(NODEPS),$(MAKECMDGOALS)),,true)
    endif
endif

ifeq ($(DONT_BUILD_DEPS),)
    -include $(addprefix $(OBJDIR)/,$(addsuffix .dep,$(notdir $(basename $(CURCPP) $(wildcard *.cpp)))))
endif

# Cleanup Windows oddities in source files.
# Many IDE's indent with tabs, and tabs are 4 chars wide.
# Tabs shall not be expanded in Makefiles.

.PHONY: unixify
unixify:
	for f in $$(find . -name \*.c -o -name \*.cpp -o -name \*.h -o -name \*.sh -o -name \*.dox -o -name \*.md -o -name \*.xml -o -name \*.txt); do \
	  expand -t 4 $$f >$$f.tmp; \
	  $(CHMOD) --reference=$$f $$f.tmp; \
	  mv -f $$f.tmp $$f; \
	done
	for f in $$(find . -name \*.c -o -name \*.cpp -o -name \*.h -o -name Makefile\* -o -name \*.sh -o -name \*.dox -o -name \*.md -o -name \*.xml -o -name \*.txt); do \
	  dos2unix -q $$f; \
	  $(SED) -i -e 's/  *$$//' $$f; \
	done

# Display make variables for debug purposes.

.PHONY: listvars
listvars:
	@true
	$(foreach v, \
	  $(sort $(filter-out .% ^% @% _% *% \%% <% +% ?% BASH% LS_COLORS SSH% VTE% XDG%,$(.VARIABLES))), \
	  $(info $(v) = "$($(v))"))