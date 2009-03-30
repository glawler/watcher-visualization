AC_INIT(watcher, 1.0)
AM_INIT_AUTOMAKE
AC_PROG_CXX
AC_PROG_RANLIB
CXXFLAGS="$CXXFLAGS -Wall -Weffc++ -pthread -ggdb"
AX_BOOST_BASE(1.37)
AX_BOOST_SYSTEM
AX_BOOST_ASIO
AX_BOOST_SERIALIZATION
AX_BOOST_THREAD
AX_BOOST_UNIT_TEST_FRAMEWORK
AC_CONFIG_FILES([Makefile logger/Makefile util/Makefile libwatcher/Makefile libwatcher/test/Makefile watcherd/Makefile])
AC_OUTPUT
