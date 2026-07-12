AC_PREREQ([2.69])
AC_INIT([chatnet], [0.2], [chol.nhial.chol@gmail.com])
AC_CONFIG_SRCDIR([server/chatnet.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CXX
AC_PROG_INSTALL
PKG_PROG_PKG_CONFIG
AC_PATH_PROG([GTK_UPDATE_ICON_CACHE], [gtk-update-icon-cache], [:])
AC_PATH_PROG([UPDATE_DESKTOP_DATABASE], [update-desktop-database], [:])

AC_CHECK_HEADERS([arpa/inet.h netdb.h netinet/in.h sys/select.h sys/socket.h unistd.h], [],
  [AC_MSG_ERROR([a required POSIX networking header is missing])])
AC_SEARCH_LIBS([pthread_create], [pthread], [],
  [AC_MSG_ERROR([POSIX threads are required])])

PKG_CHECK_MODULES([GTKMM], [gtkmm-2.4 >= 2.4], [],
  [AC_MSG_ERROR([gtkmm-2.4 development files are required])])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
