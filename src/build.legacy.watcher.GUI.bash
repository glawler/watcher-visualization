#/usr/bin/env bash

#
# This is a really stupid script for building the legacyWatcher and the libs that it needs. It is 
# good for rebuilding all libs needed for the GUI. That being said it is bad because:
# There is no error checking.
# It will break somewhere.
# It doesn't use system libs.
# It reconfigures all the time.
# It doesn't look at dependencies.
# iIf all that sounds good to you, knock yourself out.
#

TOPDIR=$(dirname $(pwd))
SRCDIR=$TOPDIR/src

# 
# Tell the ./configures below where to find the locally installed libraries.
#
export CFLAGS="-I $TOPDIR/include"
export CXXFLAGS="-I $TOPDIR/include"
# export LDFLAGS="-L $TOPDIR/lib"

# IDMEF
cd $SRCDIR
tar zkxvf ../tars/libidmef-0.7.3_beta_McAfee20050325.tar.gz 
cd $SRCDIR/libidmef-0.7.3_beta_McAfee20050325
./configure --prefix=$TOPDIR
make
make install

# libconfig (for F9+, just yum install it)
cd $SRCDIR
tar zkxvf ../tars/libconfig-1.3.1.tar.gz
cd $SRCDIR/libconfig-1.3.1/
./configure --prefix=$TOPDIR
make && make install

cd $SRCDIR/watcher/legacyWatcher
make libidsCommunications.a
cp libidsCommunications.a $TOPDIR/lib


cd $SRCDIR/watcher/legacyWatcher
for f in floatinglabeltest routingdetector watcherpropertytest watchergraphtest edgetest labeltest
do
    make $f
    cp $f $TOPDIR/bin
done

cd $SRCDIR
tar zkxvf ../tars/apr-1.3.3.tar.gz
cd apr-1.3.3
./configure --prefix=$TOPDIR
make  
make install

# install apr-utils
cd $SRCDIR
tar zkxvf ../tars/apr-util-1.3.4.tar.gz
cd apr-util-1.3.4
./configure --prefix=$TOPDIR --with-apr=$TOPDIR
make  
make install

# install log4cxx
cd $SRCDIR
tar zkxvf ../tars/apache-log4cxx-0.10.0.tar.gz
cd apache-log4cxx-0.10.0
./configure --prefix=$TOPDIR --with-apr=$TOPDIR --with-apr-util=$TOPDIR --disable-doxygen
make  
make install

cd $SRCDIR
tar jkxvf ../tars/qwt-5.1.1.tar.bz2
cd qwt-5.1.1/
# replace install dir with $TOPDIR in config file
perl -pi -e " s|/usr/local/qwt-5.1.1|$TOPDIR| " qwtconfig.pri
qmake-qt4
make  
make install

cd $SRCDIR/logger
make 
make install

cd $SRCDIR/util
make 
make install

cd $SRCDIR/watcher
qmake-qt4
make

