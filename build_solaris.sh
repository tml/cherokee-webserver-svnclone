#!/usr/bin/bash

# Copyright (C) 2001, 2002, 2003, 2004, 2005 Alvaro Lopez Ortega
#
# Authors:
#      Alvaro Lopez Ortega <alvaro@alobbs.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of version 2 of the GNU General Public
# License as published by the Free Software Foundation.

# Configuration parameter
#
INSTALL_DIR=/tmp/cheinst

# Read some data from the system
#
if test `uname -p` = "i386"; then 
 ARCH="i386"
 SOLARIS_ARCH="intel"
else
 ARCH="sparc"
 SOLARIS_ARCH="sparc"
fi
INIT_DIR=`pwd`
SOLARIS_RELEASE=`uname -r | sed s/5\.//`
TARGET_FILE=/tmp/cherokee-0.4.25-sol${SOLARIS_RELEASE}-${SOLARIS_ARCH}-local

# Build and install cherokee
#
rm -rf $INSTALL_DIR
make all || exit 1
make DESTDIR=$INSTALL_DIR install || exit 1
cd $INSTALL_DIR/usr/local

# Build the prototype file with the package content
#
echo "i pkginfo=./pkginfo" > prototype
find . -print | pkgproto | grep -v pkginfo | sed -e "s/root root/bin bin/" >> prototype

# Build the pkginfo file to describe the package
#
cat << EOF >pkginfo
PKG="Cherokee"
NAME="cherokee"
VERSION="0.4.25"
CATEGORY="application"
VENDOR="Alvaro Lopez Ortega"
EMAIL="alvaro@alobbs.com"
PSTAMP="Alvaro Lopez Ortega"
BASEDIR="/usr/local"
CLASSES="none"
EOF
echo "ARCH=\"$ARCH\"" >>pkginfo

# Build the binary package
#
rm -rf /var/spool/pkg/Cherokee/
pkgmk -r `pwd`

# Convert to stream2
#
rm -f $TARGET_FILE $TARGET_FILE.gz
cd /var/spool/pkg
echo | pkgtrans -s `pwd` $TARGET_FILE
cd $INIT_DIR
gzip -9 $TARGET_FILE

# Clean up
#
rm -rf $INSTALL_DIR
rm -rf /var/spool/pkg/Cherokee/

# It's done!
#
echo "Congratulations! Here it is:"
ls -l $TARGET_FILE.gz


