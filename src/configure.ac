AC_INIT(watcher, 0.6)
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

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

PKG_CHECK_MODULES(LOGGER, logger)
PKG_CHECK_MODULES(LIBIDMEF, libidmef)
PKG_CHECK_MODULES(IDSCOMMUNICATIONS, idsCommunications)

# allow optional 'test node only' build which builds just test node components.
AC_MSG_CHECKING(--enable-testnodeonly)
AC_ARG_ENABLE(testnodeonly,
	      [AS_HELP_STRING([--enable-testnodeonly],[only configure and build test node components])],
	      [enable_testnodeonly=$enableval],
          [enable_testnodeonly="no"])
AC_MSG_RESULT($enable_testnodeonly)
AM_CONDITIONAL([ENABLE_TESTNODEONLY], [test x$enable_testnodeonly != xno])

if test x$enable_testnodeonly == xno; then
    #
    # Normal, full build section
    #
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
    AC_MSG_CHECKING([for sqlite3 >= 3.5.9])
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[@%:@include <sqlite3.h>
		]],
        [[#if SQLITE_VERSION_NUMBER >= 3005009
    //OK
    #else
    #  error sqlite version is too old
    #endif
    ]])],
		[AC_MSG_RESULT(ok)],
		[AC_MSG_ERROR(sqlite is too old)])

    # allow optional disabling of ogreWatcher
    AC_MSG_CHECKING(--enable-ogreWatcher)
    AC_ARG_ENABLE(ogreWatcher,
	      [AS_HELP_STRING([--enable-ogreWatcher],[enable compilation of ogreWatcher])],
	      [enable_ogreWatcher=$enableval],
          [enable_ogreWatcher="no"])
    AC_MSG_RESULT($enable_ogreWatcher)
    AM_CONDITIONAL([ENABLE_OGREWATCHER], [test x$enable_ogreWatcher != xno])

    # allow optional disabling of watcher3d
    AC_MSG_CHECKING(--enable-watcher3d)
    AC_ARG_ENABLE(watcher3d,
	      [AS_HELP_STRING([--enable-watcher3d],[enable compilation of watcher3d GUI])],
	      [enable_watcher3d=$enableval],
          [enable_watcher3d="no"])
    AC_MSG_RESULT($enable_watcher3d)
    AM_CONDITIONAL([ENABLE_WATCHER3D], [test x$enable_watcher3d != xno])

    # allow optional disabling of earthWatcher
    AC_MSG_CHECKING(--enable-earthWatcher)
    AC_ARG_ENABLE(earthWatcher,
	      [AS_HELP_STRING([--enable-earthWatcher],[enable compilation of earthWatcher])],
	      [enable_earthWatcher=$enableval],
          [enable_earthWatcher="no"])
    AC_MSG_RESULT($enable_earthWatcher)
    AM_CONDITIONAL([ENABLE_EARTHWATCHER], [test x$enable_earthWatcher != xno])

    ac_cv_kml=no
    AC_ARG_WITH([kml], 
                [AS_HELP_STRING([--with-kml=PREFIX],[use the libkml installed in the PREFIX directory])],
    	    	[ac_cv_kml=$withval])

    if test x$enable_earthWatcher != xno; then
	    AC_MSG_CHECKING(for libkml)
	    if test x$ac_cv_kml = xno; then
		    #not specified by the user, look in the standard locations
		    for d in /usr /usr/local; do
			    if test -d $d/include/kml; then
				    ac_cv_kml=$d
			    fi
		    done
	    fi
	    if test x$ac_cv_kml = xno; then
		    AC_MSG_ERROR("unable to locate libkml")
	    fi
	    AC_MSG_RESULT($ac_cv_kml)
	    KML_INCLUDES="-I$ac_cv_kml/include"
	    KML_LDFLAGS="-L$ac_cv_kml/lib"
	    KML_LDADD="-lkmlconvenience -lkmlengine -lkmldom"
	    AC_SUBST(KML_INCLUDES)
	    AC_SUBST(KML_LDFLAGS)
	    AC_SUBST(KML_LDADD)
    fi

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
    # if we can't find it, assume that it is already set in CXXFLAGS, LDFLAGS, etc...
	AC_MSG_CHECKING(for qt4)
	ac_cv_qt=
	if test -d /usr/local/include/qt; then
	    AC_MSG_RESULT(/usr/local/include/qt)
	elif test -d /usr/include/qt4; then
	    AC_MSG_RESULT(/usr/include/qt4)
	    ac_cv_qt='-qt4'
	elif test -d /usr/include/qt; then
	    AC_MSG_RESULT(/usr/include/qt)
	elif test -d /usr/include/Qt; then
	    AC_MSG_RESULT(/usr/include)
	else
        AC_MSG_WARN([Did not find QT4 in "standard" locations. QT4 LDFLAGS and CFLAGS must be given in environment variables.])
	fi
	
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
        AC_MSG_WARN([Did not find QWT in "standard" locations. QWT LDFLAGS and CFLAGS must be given in environment variables.])
    fi

    AC_SUBST(QWT_INCLUDES)
    AC_SUBST(QWT_LIB)

else
    #
    # testnodeonly build section
    #
    AM_CONDITIONAL([ENABLE_OGREWATCHER], [test 1 == 0])
    AM_CONDITIONAL([ENABLE_WATCHER3D], [test 1 == 0])
fi

# allow optional disabling of watcherHierarchyClient
AC_MSG_CHECKING(--enable-hierarchy_client)
AC_ARG_ENABLE(hierarchy_client,
    [AS_HELP_STRING([--enable-hierarchy_client],[enable compilation of hierarchy_client])],
    [enable_hierarchy_client=$enableval],
    [enable_hierarchy_client="no"])
AM_CONDITIONAL([ENABLE_HIERARCHY_CLIENT], [test x$enable_hierarchy_client != xno])
AC_MSG_RESULT($enable_hierarchy_client)

if test x$enable_hierarchy_client != xno; then
    PKG_CHECK_MODULES(idsCommunications, idsCommunications >= 1.0)
fi

AC_CONFIG_FILES([ \
    Makefile \
    util/Makefile \
    libwatcher/Makefile \
    libwatcher/watcher.pc \
    libwatcher/watchermsg.pc \
    libwatcher/test/Makefile \
    clients/Makefile \
    clients/routeFeeder/Makefile \
    clients/commandlineFeeders/Makefile 
])

if test x$enable_testnodeonly == xno; then
    AC_CONFIG_FILES([ \
       sqlite_wrapper/Makefile \
       watcherd/Makefile \
       clients/messageStream2Text/Makefile \
       clients/connectivity2dot/Makefile \
       clients/legacyWatcher/legacyWatcher.pro
    ])

    if test x$enable_watcher3d != xno; then
        AC_CONFIG_FILES([clients/watcher3D/Makefile])
    fi

    if test x$enable_ogreWatcher != xno; then
        AC_CONFIG_FILES([clients/ogreWatcher/Makefile])
    fi
fi

if test x$enable_hierarchy_client != xno; then
    AC_CONFIG_FILES([clients/watcherHierarchyClient/Makefile])
fi

if test x$enable_earthWatcher != xno; then
    AC_CONFIG_FILES([clients/earthWatcher/Makefile])
fi

AC_OUTPUT
