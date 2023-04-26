m4_define([package_version_major],[1])
m4_define([package_version_minor],[0])
m4_define([package_version_micro],[0])
m4_define([api_version],[1])

AC_PREREQ([2.10])
AC_INIT([libtenma], [package_version_major.package_version_minor.package_version_micro], [matt@majenko.co.uk], [], [https://majenko.co.uk])

PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

LT_INIT
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects no-dependencies foreign])
AC_CONFIG_MACRO_DIR([m4])

AM_SILENT_RULES([yes])

AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")

PKG_CHECK_MODULES([libusb],
                     [libusb-1.0],
                     [])
PKG_CHECK_MODULES([libgd],
                     [gdlib],
                     [])

AC_SUBST([PACKAGE_VERSION_MAJOR],package_version_major)
AC_SUBST([PACKAGE_VERSION_MINOR],package_version_minor)
AC_SUBST([PACKAGE_VERSION_MICRO],package_version_micro)
AC_SUBST([API_VERSION],api_version)

#AC_CHECK_HEADER(libusb-1.0/libusb.h, [], [
#    AC_MSG_FAILURE(libusb not found)
#])
#AC_CHECK_LIB(usb-1.0, libusb_init, [], [
#    AC_MSG_FAILURE(libusb not found)
#])

#AC_CHECK_HEADER(gd.h, [], [
#    AC_MSG_FAILURE(gd not found)
#])
#AC_CHECK_LIB(gd, gdImageSetPixel, [], [
#    AC_MSG_FAILURE(gd not found)
#])

# Checks for programs.
AC_PROG_CC

AC_CONFIG_FILES([Makefile src/Makefile src/libtenma.pc])
AC_OUTPUT

