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
AX_BOOST_DATE_TIME

# allow optional disabling of watcher3d
AC_MSG_CHECKING(--enable-watcher3d)
AC_ARG_ENABLE(watcher3d,
	      [AS_HELP_STRING([--enable-watcher3d],[enable compilation of watcher3d GUI])],
	      [want_watcher3d=$enableval],
          [enable_watcher3d="no"])
AC_MSG_RESULT($enable_watcher3d)
AM_CONDITIONAL([WANT_WATCHER3D], [test x$want_watcher3d != xno])

# allow optional disabling of watcherHierarchyClient
AC_MSG_CHECKING(--enable-hierarchy_client)
AC_ARG_ENABLE(hierarchy_client,
	      [AS_HELP_STRING([--enable-hierarchy_client],[enable compilation of hierarchy_client])],
	      [want_hierarchy_client=$enableval],
          [enable_hierarchy_client="no"])
AM_CONDITIONAL([WANT_HIERARCHY_CLIENT], [test x$want_hierarchy_client != xno])
AC_MSG_RESULT($enable_hierarchy_client)

# allow optional disabling of ogreWatcher
AC_MSG_CHECKING(--enable-ogreWatcher)
AC_ARG_ENABLE(ogreWatcher,
	      [AS_HELP_STRING([--enable-ogreWatcher],[enable compilation of ogreWatcher])],
	      [want_ogreWatcher=$enableval],
          [enable_ogreWatcher="no"])
AC_MSG_RESULT($enable_ogreWatcher)
AM_CONDITIONAL([WANT_OGREWATCHER], [test x$want_ogreWatcher != xno])

# OGRE, OIS, and CEGUI
if test x$enable_ogreWatcher != xno; then
    AC_DEFINE(FEAT_OGREWATCHER)

    PKG_CHECK_MODULES(OGRE, [OGRE >= 1.2])
    AC_SUBST(OGRE_CFLAGS)
    AC_SUBST(OGRE_LIBS)

    PKG_CHECK_MODULES(OIS, [OIS >= 1.0])
    AC_SUBST(OIS_CFLAGS)
    AC_SUBST(OIS_LIBS)

    PKG_CHECK_MODULES(CEGUI, [CEGUI >= 0.5.0])
    AC_SUBST(CEGUI_CFLAGS)
    AC_SUBST(CEGUI_LIBS)

    # PKG_CHECK_MODULES(CEGUI-OGRE, [CEGUI-OGRE >= 1.4.5])
    # AC_SUBST(CEGUI-OGRE_CFLAGS)
    # AC_SUBST(CEGUI-OGRE_LIBS)
fi

# qmake-qt4 seems to set the correct includes and libs, we just need to determine where it is for the QWT detection
AC_MSG_CHECKING(for qt4)
ac_cv_qt=
if test -d /usr/local/include/qt; then
    AC_MSG_RESULT(/usr/local/include/qt)
#    QT4_INCLUDES="/usr/local/include/qt"
#    QT4_LIBS="-L/usr/local/lib"
#    ac_cv_qt_dir=/usr/local/lib
elif test -d /usr/include/qt4; then
    AC_MSG_RESULT(/usr/include/qt4)
#    QT4_INCLUDES="/usr/include/qt4"
    ac_cv_qt='-qt4'
#    ac_cv_qt_dir=/usr/lib
elif test -d /usr/include/qt; then
    AC_MSG_RESULT(/usr/include/qt)
#    QT4_INCLUDES="/usr/include/qt4"
#    ac_cv_qt_dir=/usr/lib
elif test -d /usr/include/Qt; then
    AC_MSG_RESULT(/usr/include)
else
    AC_MSG_ERROR(not found)
fi
#if test -f ${ac_cv_qt_dir}/libqt-mt.so; then
#    QT4_LIBS="$QT4_LIBS -lqt-mt"
#else
#    QT4_LIBS="$QT4_LIBS -lqt"
#fi

AC_MSG_CHECKING(for qwt)
if test -d include/qwt; then
    AC_MSG_RESULT(./include/qwt)
    QWT_INCLUDES="../../../include/qwt"
    QWT_LIB="-L../../../lib -lqwt"
elif test -d /usr/local/include/qwt; then
    AC_MSG_RESULT(/usr/local/include/qt)
    QWT_INCLUDES="/usr/local/include/qwt"
    QWT_LIB="-L/usr/local/lib -lqwt"
elif test -d /usr/include/qwt${ac_cv_qt}; then
    AC_MSG_RESULT(/usr/include/qwt${ac_cv_qt})
    QWT_INCLUDES="/usr/include/qwt${ac_cv_qt}"
    QWT_LIB="-lqwt${ac_cv_qt}"
else
    AC_MSG_ERROR(not found)
fi

AC_SUBST(QWT_INCLUDES)
AC_SUBST(QWT_LIB)

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
    clients/ogreWatcher/Makefile \
    clients/legacyWatcher/legacyWatcher.pro
])

AC_OUTPUT
