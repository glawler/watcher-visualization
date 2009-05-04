AC_INIT(watcher, 1.0)
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_RANLIB
# Boost 1.36 is when asio appeared
AX_BOOST_BASE(1.36)
AX_BOOST_SYSTEM
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
])

#override default $CXXFLAGS
CXXFLAGS="-Wall -Wextra -pthread -ggdb -O0"

AC_OUTPUT
