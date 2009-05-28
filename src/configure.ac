AC_INIT(watcher, 1.0)
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB

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
AX_BOOST_THREAD
AX_BOOST_UNIT_TEST_FRAMEWORK
AC_CONFIG_FILES([ \
    Makefile \
    logger/Makefile \
    util/Makefile \
    sqlite_wrapper/Makefile \
    libwatcher/Makefile \
    libwatcher/test/Makefile \
    watcherd/Makefile 
    watcherd/test/Makefile \
    watcherHierarchyClient/Makefile \
    feeders/Makefile \
    feeders/routeFeeder/Makefile \
])

#override default $CXXFLAGS
CXXFLAGS="-Wall -Wextra -pthread -ggdb -O0"

AC_OUTPUT
