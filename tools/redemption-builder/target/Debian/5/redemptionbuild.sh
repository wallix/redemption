#!/bin/bash
#
# Copyright (c) 2010 WALLIX, SAS. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: redemption
# Author(s) : Fabien Boucher <fbo@wallix.com>, Sofian Brabez <sbz@wallix.com>
# Id: $Id: redemptionbuild.sh 484 2010-08-27 13:09:26Z xdunat $
# URL: $URL: svn+ssh://mpotier@zoo.ifr.lan/srv/svn/redemption/trunk/build/target/Debian/5/redemptionbuild.sh $
# Module description: Script to create package
# Compile with:
#

#set -x

DEBPREFIX=$1

PACKAGENAME=<!redemptionname!>
BUILDDIR=<!builddir!>
REDEMPTIONPREFIX=<!redemptioninstallprefix!>
REDEMPTIONINSTALLPREFIX=$DEBPREFIX/$REDEMPTIONPREFIX

PYTHON_VER=python2.5
PYTHON=/usr/bin/$PYTHON_VER

REDEMPTION=$BUILDDIR

cd $REDEMPTION
echo "Building $PACKAGENAME ..."
ETC_PREFIX=/etc/opt/wab/rdp PREFIX=$REDEMPTIONPREFIX bjam
echo "Installing $PACKAGENAME ..."
ETC_PREFIX=$DEBPREFIX/etc/opt/wab/rdp PREFIX=$REDEMPTIONINSTALLPREFIX bjam install

# quick fix because ETC_PREFIX doesn't work
mkdir $DEBPREFIX/etc/opt/wab/rdp
mv $DEBPREFIX/etc/opt/wab/*.ini $DEBPREFIX/etc/opt/wab/rdp/

cd -
