
# Process this file with autoconf to produce a configure script.

#########################
#    Initializations    #
#########################

# Initialize autoconf & define package name, version and bug-report address 
AC_INIT(Spectral Analysis, 3.3.0, tools@bsc.es)

# GNU Autotools intermediate files are stored in the following directory
AC_CONFIG_AUX_DIR(config)

# Loads some shell variables like host_cpu and host_os, to get the host information 
AC_CANONICAL_SYSTEM

case "${target_os}" in
  freebsd* ) OperatingSystem="freebsd"
             AC_DEFINE([OS_FREEBSD], [1], [Define if operating system is FreeBSD]) ;;
esac

# Initialize automake
AM_INIT_AUTOMAKE
AC_PROG_MKDIR_P

# Specify the output configuration header file
AC_CONFIG_HEADER(config.h)

# Search for libtool support
AC_LIBTOOL_DLOPEN
AC_ENABLE_SHARED
AC_PROG_LIBTOOL

############################
#    Checks for options    #
############################
AX_CHECK_DEBUG

#############################
#    Checks for programs    #
#############################

AC_PROG_CC(gcc xlc cc)
AC_PROG_CXX(g++ xlC CC)
AC_PROG_CPP
AC_PROG_INSTALL

# Automake 1.10 reports a problem if this is not defined
AM_PROG_CC_C_O

##############################
#    Checks for libraries    #
##############################

AX_OPENMP
AX_PROG_FFT
AX_PROG_EXTRAE
AX_LIBTOOLS

#############################
#    Checks for headers     #
#############################

###########################
#    Checks for types     #
###########################

AC_C_CONST
AC_TYPE_SIZE_T

CFLAGS="-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE $CFLAGS"

AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
