AC_INIT(watcher, 1.0.0)
PACKAGE_RELEASE=1
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
PKG_PROG_PKG_CONFIG

AC_SUBST(PACKAGE_VERSION)
AC_SUBST(PACKAGE_RELEASE)

# Boost 1.36 is when asio appeared
AX_BOOST_BASE(1.36)
AX_BOOST_SYSTEM
AX_BOOST_FILESYSTEM
AX_BOOST_ASIO
AX_BOOST_REGEX
AX_BOOST_THREAD
AX_BOOST_UNIT_TEST_FRAMEWORK
AX_BOOST_DATE_TIME
AX_BOOST_PROGRAM_OPTIONS

# handle DEBUG enabled builds.
AC_MSG_CHECKING(whether to enable debugging)
debug_default="yes"
AC_ARG_ENABLE(debug, [  --enable-debug=[no/yes] turn on debugging [default=$debug_default]],, enable_debug=$debug_default)
if test "x$enable_debug" = "xyes"; then
    CFLAGS="$CXXFLAGS -ggdb -O0 -DDEBUG -pthread"
    CXXFLAGS="$CFLAGS -ggdb -O0 -DDEBUG -pthread"
    AC_MSG_RESULT(yes)
else
    CFLAGS=${CFLAGS=""};
    CXXFLAGS=${CXXFLAGS=""}; 
    AC_MSG_RESULT(no)
fi

PKG_CHECK_MODULES(LOGGER, logger)

# allow optional 'test node only' build which builds just test node components.
AC_MSG_CHECKING(whether to enable a test node only build)
AC_ARG_ENABLE(testnodeonly,
          [AS_HELP_STRING([--enable-testnodeonly],[only configure and build test node components])],
          [enable_testnodeonly=$enableval],
          [enable_testnodeonly="no"])
AC_MSG_RESULT($enable_testnodeonly)
AM_CONDITIONAL([ENABLE_TESTNODEONLY], [test x$enable_testnodeonly != xno])

# Things we always want
PKG_CHECK_MODULES(LOG4CXX, [liblog4cxx >= 0.10.0])
PKG_CHECK_MODULES(LIBCONFIG, [libconfig++ >= 1.3.1])
PKG_CHECK_MODULES(LIBYAML, [yaml-cpp >= 0.2.6])

# Not entirely correct. Will break if 3rd party libs not installed in standard locations
# relative to that lib's PREFIX. Will also break when doing 64/32 bit cross compilation.
local_libsubdir="lib"
if test "$(uname -m)" = "x86_64"; then
    local_libsubdir="lib64"
fi

if test x$enable_testnodeonly == xno; then
    #
    # Normal, full build section
    #
    PKG_CHECK_MODULES(SQLITE3, [sqlite3 >= 3.5.9])

    # GTL - ogreWatcher is not buildable right now. 
    # # allow optional disabling of ogreWatcher
    # AC_MSG_CHECKING(whether to build ogreWatcher)
    # AC_ARG_ENABLE(ogreWatcher,
    #       [AS_HELP_STRING([--enable-ogreWatcher],[enable compilation of ogreWatcher])],
    #       [enable_ogreWatcher=$enableval],
    #       [enable_ogreWatcher="no"])
    # AC_MSG_RESULT($enable_ogreWatcher)
    # AM_CONDITIONAL([ENABLE_OGREWATCHER], [test x$enable_ogreWatcher != xno])

    # allow optional disabling of watcher3d
    AC_MSG_CHECKING(whether to build watcher3d)
    AC_ARG_ENABLE(watcher3d,
          [AS_HELP_STRING([--enable-watcher3d],[enable compilation of watcher3d GUI])],
          [enable_watcher3d=$enableval],
          [enable_watcher3d="no"])
    AC_MSG_RESULT($enable_watcher3d)
    AM_CONDITIONAL([ENABLE_WATCHER3D], [test x$enable_watcher3d != xno])

    # allow optional disabling of earthWatcher
    AC_MSG_CHECKING(whether to build earthWatcher)
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

    ac_cv_kml_libdir=no
    AC_ARG_WITH([kml-libdir], 
                [AS_HELP_STRING([--with-kml-libdir=LIBDIR],[use the libdir installed in LIBDIR/libkml when looking for kml libs])],
                [ac_cv_kml_libdir=$withval])

    if test x$enable_earthWatcher != xno; then
        PKG_CHECK_MODULES(LIBURIPARSER, liburiparser)
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
            AC_MSG_WARN("unable to locate libkml. You will have to specify CFLAGS and LDFLAGS by hand...")
        else
            AC_MSG_RESULT($ac_cv_kml)
            KML_INCLUDES="-I$ac_cv_kml/include"
            if test x$ac_cv_kml_libdir = xno; then
                # ARL puts it in ./lib or ./lib64 not lib/libkml or lib64libkml 
                KML_LDFLAGS="-L$ac_cv_kml/$local_libsubdir/libkml -L$ac_cv_kml/$local_libsubdir -Wl,-rpath=$ac_cv_kml/$local_libsubdir/libkml"
            else 
                # ARL puts it in ./lib or ./lib64 not lib/libkml or lib64libkml 
                KML_LDFLAGS="-L$ac_cv_kml_libdir/libkml -L$ac_cv_kml_libdir -Wl,-rpath=$ac_cv_kml_libdir/libkml"
            fi
            AC_SUBST(KML_INCLUDES)
            AC_SUBST(KML_LDFLAGS)
        fi
        
        KML_LDADD="-lkmlbase -lkmlconvenience -lkmlengine -lkmldom"
        AC_SUBST(KML_LDADD)
    fi
    
    # GTL - OgreWatcher is not buildable right now, so we don't need these checks.
    # # OGRE, OIS, and CEGUI
    # if test x$enable_ogreWatcher != xno; then
    #     AC_DEFINE(FEAT_OGREWATCHER)

    #     PKG_CHECK_MODULES(OGRE, [OGRE >= 1.2])
    #     AC_SUBST(OGRE_CFLAGS)
    #     AC_SUBST(OGRE_LIBS)
    # 
    #     PKG_CHECK_MODULES(OIS, [OIS >= 1.0])
    #     AC_SUBST(OIS_CFLAGS)
    #     AC_SUBST(OIS_LIBS)
    # 
    #     PKG_CHECK_MODULES(CEGUI, [CEGUI >= 0.5.0])
    #     AC_SUBST(CEGUI_CFLAGS)
    #     AC_SUBST(CEGUI_LIBS)
    # 
    #     # PKG_CHECK_MODULES(CEGUI-OGRE, [CEGUI-OGRE >= 1.4.5])
    #     # AC_SUBST(CEGUI-OGRE_CFLAGS)
    #     # AC_SUBST(CEGUI-OGRE_LIBS)
    # fi

    # allow optional disabling of legacyWatcher
    AC_MSG_CHECKING(whether to build legacyWatcher)
    AC_ARG_ENABLE(legacyWatcher,
          [AS_HELP_STRING([--enable-legacyWatcher],[enable compilation of legacyWatcher GUI])],
          [enable_legacyWatcher=$enableval],
          [enable_legacyWatcher="no"])
    AC_MSG_RESULT($enable_legacyWatcher)
    AM_CONDITIONAL([ENABLE_LEGACYWATCHER], [test x$enable_legacyWatcher != xno])

    # allow optional disabling of dataWatcher
    AC_MSG_CHECKING(whether to build dataWatcher)
    AC_ARG_ENABLE(dataWatcher,
          [AS_HELP_STRING([--enable-dataWatcher],[enable compilation of dataWatcher GUI])],
          [enable_legacyWatcher=$enableval],
          [enable_legacyWatcher="no"])
    AC_MSG_RESULT($enable_dataWatcher)
    AM_CONDITIONAL([ENABLE_DATAWATCHER], [test x$enable_dataWatcher != xno])

    if test x$enable_legacyWatcher = xyes -o x$enable_dataWatcher = xyes ; then
        # Check for Qt, even if we use qmake to find/link to it.
        PKG_CHECK_MODULES(QTGUI, [QtGui >= 4.3.0])
        PKG_CHECK_MODULES(QTCORE, [QtCore >= 4.3.0])
        PKG_CHECK_MODULES(QTOPENGL, [QtOpenGL >= 4.3.0])
		PKG_CHECK_MODULES(SDL, sdl)
		PKG_CHECK_MODULES(SDLIMAGE, SDL_image)
    fi

    # Looking for qwt is error prone and a pain. Submitted a bugfix to add a pkg-config .pc to it, 
    # but it has yet to get into the tree. 
    if test x$enable_dataWatcher = xyes; then
        ac_cv_qwt=no
        AC_ARG_WITH([qwt], 
                    [AS_HELP_STRING([--with-qwt=PREFIX],[use the Qwt lib installed in the PREFIX directory])],
                    [ac_cv_qwt=$withval])

        ac_cv_qwt_libdir=no
        AC_ARG_WITH([qwt-libdir], 
                    [AS_HELP_STRING([--with-qwt-libdir=LIBDIR],[use the libdir installed in LIBDIR when looking for qwt libs])],
                    [ac_cv_qwt_libdir=$withval])

        # ubuntu 10.04 is /usr/include/qwt-qt4/ and -lqwt-qt4
        ac_cv_qwt_ext=''
        AC_ARG_WITH([qwt-ext],
            [AS_HELP_STRING([--with-qwt-ext=STRING],[use the Qwt library named qwt-STRING])],
            [ac_cv_qwt_ext=$withval])

        AC_MSG_CHECKING(for qwt)
        if test x$ac_cv_qwt = xno; then
            AC_MSG_RESULT(/usr/local/qwt)
            AC_MSG_WARN("QWT prefix not defined via --with-qwt.")
            AC_MSG_WARN(["Assuming /usr/local/qwt, which is almost certainly incorrect..."])
            QWT_INCLUDES="/usr/local/qwt/include"
            if test x$ac_cv_qwt_libdir = xno; then
                QWT_LIB="-L/usr/local/qwt/$local_libsubdir -lqwt"
            else
                QWT_LIB="-L$ac_cv_qwt_libdir -lqwt"
            fi
        else 
            AC_MSG_RESULT($ac_cv_qwt)
            if test x$ac_cv_qwt_ext != x; then
                ac_cv_qwt_ext="-$ac_cv_qwt_ext"
            fi
            QWT_INCLUDES="$ac_cv_qwt/include/qwt$ac_cv_qwt_ext $ac_cv_qwt/include"
            if test x$ac_cv_qwt_libdir = xno; then
                QWT_LIB="-L/$ac_cv_qwt/$local_libsubdir -lqwt$ac_cv_qwt_ext"
            else
                QWT_LIB="-L$ac_cv_qwt_libdir -lqwt$ac_cv_qwt_ext"
            fi
        fi

        AC_SUBST(QWT_INCLUDES)
        AC_SUBST(QWT_LIB)
    fi 

else
    #
    # testnodeonly build section
    #
    # AM_CONDITIONAL([ENABLE_OGREWATCHER], [test 1 == 0]) GTL - disable OgreWatcher build for now.
    AM_CONDITIONAL([ENABLE_WATCHER3D], [test 1 == 0])
    AM_CONDITIONAL([ENABLE_EARTHWATCHER], [test 1 == 0])
    AM_CONDITIONAL([ENABLE_DATAWATCHER], [test 1 == 0])
    AM_CONDITIONAL([ENABLE_LEGACYWATCHER], [test 1 == 0])
fi

# allow optional disabling of watcherHierarchyClient
AC_MSG_CHECKING(whether to build a hierarchy client enabled build)
AC_ARG_ENABLE(hierarchy_client,
    [AS_HELP_STRING([--enable-hierarchy_client],[enable compilation of hierarchy_client])],
    [enable_hierarchy_client=$enableval],
    [enable_hierarchy_client="no"])
AM_CONDITIONAL([ENABLE_HIERARCHY_CLIENT], [test x$enable_hierarchy_client != xno])
AC_MSG_RESULT($enable_hierarchy_client)

if test x$enable_hierarchy_client != xno; then
    PKG_CHECK_MODULES(idsCommunications, idsCommunications >= 1.0)
    PKG_CHECK_MODULES(LIBIDMEF, libidmef = 0.7.3-beta-McAfee20050325)
fi

AC_CONFIG_FILES([ \
    Makefile \
    util/Makefile \
    libwatcher/Makefile \
    libwatcher/watcher.pc \
    libwatcher/watchermsg.pc \
    util/watcherutils.pc \
    libwatcher/test/Makefile \
    clients/Makefile \
    clients/routeFeeder/Makefile \
    clients/commandlineFeeders/Makefile \
])

if test x$enable_testnodeonly == xno; then
    AC_CONFIG_FILES([ \
       sqlite_wrapper/Makefile \
       watcherd/Makefile \
       clients/messageStream2Text/Makefile \
       clients/connectivity2dot/Makefile \
       clients/randomScenario/Makefile \
       clients/gps2eventdb/Makefile \
       clients/gps2eventdb/test/Makefile
    ])

    if test x$enable_legacyWatcher != xno; then
        AC_CONFIG_FILES([clients/legacyWatcher/legacyWatcher.pro])
    fi

    if test x$enable_dataWatcher != xno; then
       AC_CONFIG_FILES([clients/dataWatcher/Makefile.config])
    fi 

    if test x$enable_watcher3d != xno; then
        AC_CONFIG_FILES([clients/watcher3D/Makefile])
    fi

    # GTL - OgreWatcher is not currently buildable
    # if test x$enable_ogreWatcher != xno; then
    #     AC_CONFIG_FILES([clients/ogreWatcher/Makefile])
    # fi
fi

if test x$enable_hierarchy_client != xno; then
    AC_CONFIG_FILES([clients/watcherHierarchyClient/Makefile])
fi

if test x$enable_earthWatcher != xno; then
    AC_CONFIG_FILES([clients/earthWatcher/Makefile])
fi

AC_OUTPUT
