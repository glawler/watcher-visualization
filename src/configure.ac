AC_INIT(watcher, 1.0)
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
AC_PROG_LIBTOOL

AC_DEFUN([AX_PATH_LIB],
	[AC_MSG_CHECKING(for $1)
	ac_cv_sqlite_path=false
	# check std locations
	for d in /usr/local /usr; do
		if test -e $d/include/$1; then
			ac_cv_sqlite_path=$d
		fi
	done
	# if not found try in $(TOPDIR)/{include,lib}
	if test $ac_cv_sqlite_path = false; then
		if test -e "include/$1"; then
			ac_cv_sqlite_path='$(TOPDIR)'
			LDFLAGS="$LDFLAGS -L${ac_cv_sqlite_path}/lib"
			CPPFLAGS="$CPPFLAGS -I${ac_cv_sqlite_path}/include"
		else
			AC_MSG_ERROR(could not locate $1)
		fi
	fi
	AC_MSG_RESULT($ac_cv_sqlite_path)])
AX_PATH_LIB(sqlite3.h)
AX_PATH_LIB(log4cxx)
AX_PATH_LIB(libconfig.h)

# check for recent version of sqlite3
AC_MSG_CHECKING([for sqlite3 >= 3.6.10])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <sqlite3.h>
				   ]],
                 [[#if SQLITE_VERSION_NUMBER >= 3006010
//OK
#else
#  error sqlite version is too old
#endif
]])],
		[AC_MSG_RESULT(ok)],
		[AC_MSG_ERROR(sqlite is too old)])

# Boost 1.36 is when asio appeared
AX_BOOST_BASE(1.36)
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_ASIO
AX_BOOST_SERIALIZATION
AX_BOOST_REGEX
AX_BOOST_THREAD
AX_BOOST_UNIT_TEST_FRAMEWORK

# allow optional disabling of watcher3d
AC_ARG_ENABLE(watcher3d,
	      [AS_HELP_STRING([--disable-watcher3d],[disable compilation of watcher3d])],
	      [want_watcher3d=$enableval])
AM_CONDITIONAL([WANT_WATCHER3D], [test x$want_watcher3d != xno])

# allow optional disabling of watcherHierarchyClient
AC_ARG_ENABLE(hierarchy_client,
	      [AS_HELP_STRING([--disable-hierarchy_client],[disable compilation of hierarchy_client])],
	      [want_hierarchy_client=$enableval])
AM_CONDITIONAL([WANT_HIERARCHY_CLIENT], [test x$want_hierarchy_client != xno])

AC_CONFIG_FILES([ \
    Makefile \
    logger/Makefile \
    util/Makefile \
    sqlite_wrapper/Makefile \
    libwatcher/Makefile \
    libwatcher/test/Makefile \
    watcherd/Makefile 
    clients/Makefile \
    clients/routeFeeder/Makefile \
    clients/messageStream2Text/Makefile \
    clients/connectivity2dot/Makefile \
    clients/commandlineFeeders/Makefile \
    clients/watcherHierarchyClient/Makefile \
    clients/watcher3D/Makefile \
])

#override default $CXXFLAGS
CXXFLAGS="-Wall -Wextra -pthread -ggdb -O0"

AC_OUTPUT
