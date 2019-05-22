#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.50)
AC_INIT(sucrack, 1.2.3, nico@leidecker.info)

AM_INIT_AUTOMAKE([subdir-objects foreign])

AC_CANONICAL_TARGET

AC_CONFIG_SRCDIR([src/sucrack.c])

AM_CONFIG_HEADER(config.h)

# Checks for programs.
AC_PROG_CC

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h sys/ioctl.h unistd.h pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM


SYSTEM_OS=unknown

case $target in
    *-linux*) SYSTEM_OS=LINUX;;
    *-darwin*) SYSTEM_OS=MACOS;;
    *-freebsd*) SYSTEM_OS=FREEBSD;;
    *-netbsd*) SYSTEM_OS=NETBSD;;
esac

# Extra opts

BUFFER_TYPE="-DSTATIC_BUFFER"

AC_ARG_WITH(static-buffer, 
	[  --with-static-buffer    use a static dictionary buffer], BUFFER_TYPE="-DSTATIC_BUFFER")
	
AC_ARG_WITH(dynamic-list, 
	[  --with-dynamic-list     use a dynamic list dictionary buffer], BUFFER_TYPE="-DDYNAMIC_LIST")

	
AC_ARG_ENABLE(statistics,
	[  --enable-statistics	   calculate statistics during runtime], SUCRACK_COMPILE_FLAGS="$SUCRACK_COMPILE_FLAGS -DSTATISTICS")



SUCRACK_LINK_FLAGS="-pthread"
AC_ARG_ENABLE(static-linked,
	[  --enable-static-linked	   link sucrack staticly], SUCRACK_LINK_FLAGS="-static -pthread")

SUCRACK_VERSION=`cat VERSION`

SUCRACK_COMPILE_FLAGS="$BUFFER_TYPE $SUCRACK_COMPILE_FLAGS -D$SYSTEM_OS -DSUCRACK_TITLE=\"\\\"sucrack "`cat VERSION`" ($SYSTEM_OS)\\\"\""

AC_SUBST(SUCRACK_COMPILE_FLAGS)
AC_SUBST(SUCRACK_LINK_FLAGS)


# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_FORK
AC_CHECK_FUNCS([dup2 memset strdup strstr])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT

echo "
sucrack configuration
---------------------
sucrack version		: $SUCRACK_VERSION
target system           : $SYSTEM_OS
sucrack link flags      : $SUCRACK_LINK_FLAGS
sucrack compile flags	: $SUCRACK_COMPILE_FLAGS
"
