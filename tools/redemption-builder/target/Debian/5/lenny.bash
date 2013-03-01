#
# Copyright (c) 2010 WALLIX, SAS. All rights reserved.
# Licensed computer software. Property of WALLIX.
# Product Name: redemption
# Author(s) : Fabien Boucher <fbo@wallix.com>, Sofian Brabez <sbz@wallix.com>
# Id: $Id: lenny.bash 491 2010-09-03 09:38:56Z sbrabez $
# URL: $URL: svn+ssh://mpotier@zoo.ifr.lan/srv/svn/redemption/trunk/build/target/Debian/5/lenny.bash $
# Module description:
# Compile with:
#

#set -x

# var BUILDPREFIX is defined in consts.var
# var distdir is set to $BUILDPREFIX/build/build/target/dist/version/

function _prepPackagesBuildStore() {
    # After build of each package copy it in packagedir
    [ -d ${distdir}/packagesdir ] && rm -Rf ${distdir}/packagesdir
    mkdir ${distdir}/packagesdir
    
    # Remove all previous build stuff is exists
    rm -Rf $BUILDPREFIX/../${REDEMPTIONNAME}*.deb
    rm -Rf $BUILDPREFIX/../${REDEMPTIONNAME}*.tar.gz
    rm -Rf $BUILDPREFIX/../${REDEMPTIONNAME}*.dsc
    rm -Rf $BUILDPREFIX/../${REDEMPTIONNAME}*.changes
}

function updateBuildArch() {
    echo "In updateBuildArch."
    aptitude update
    aptitude -y safe-upgrade

    # install build dependencies
    aptitude -y install libboost-dev libssl-dev libpam0g-dev boost-build lftp

    # cleaning
    _prepPackagesBuildStore
}

function _deleteExtraBuildInstalled() {
    # remove installed deb from a previous build process

    apt-get -y remove --purge libboost-dev libssl-dev libpam0g-dev boost-build
}

function _buildFfmpeg() {
    # actual packaged version of ffmpeg
    FFMPEG_VERSION="0.svn20080206"
    FFMPEG_VERSION_SUFFIX="$FFMPEG_VERSION-18+wallix1"

    [ ! -d ${distdir}/extra/ffmpeg ] && {
        echo "Unable to find ffmpeg package source directory. Exit"
        exit 1
    }
    cd ${distdir}/extra/ffmpeg
    dpkg-source -x ffmpeg-debian_$FFMPEG_VERSION_SUFFIX.dsc
    cd - >> $LOGFILE 2>&1
    cd ${distdir}/extra/ffmpeg/ffmpeg-debian-$FFMPEG_VERSION
    dpkg-buildpackage >> $LOGFILE 2>&1
    cd - >> $LOGFILE 2>&1
    [ ! -d ${distdir}/packagesdir/ ] && mkdir -p ${distdir}/packagesdir/
    cp -fr ${distdir}/extra/ffmpeg/*.deb ${distdir}/packagesdir/
    [ "$?" != "0" ] && {
        echo "Unable to copy ${distdir}/extra/ffmpeg/*.deb in ${distdir}/packagesdir/"
        exit 1
    }
}

function extraBuild() {
    echo "In extraBuild."

    # clean build machine
    _deleteExtraBuildInstalled

    # need to repackage libssh2-1
    echo -n "Building libssh2 package ..."
    _buildFfmpeg
    echo "Done."
}

function printBanner() {
    echo '#############################################'
    echo "# $1"
    echo '#############################################'
}

# https://trac.ifr.lan/sphinx/browser/trunk/build/doc/WAB-PackageNamingConvention.txt
# This function from the name (assuming the covention is followed)
# of tags and branches cut the tagname and return to ver the correct version

function fromTagNameFormVersion() {
    # pass the tags name or the branches name as first argument
    tagname=$1
    testprerelease=$(echo $tagname | awk 'BEGIN {FS="-"} {print $3}')
    if [ -n "$testprerelease" ]; then
        # We want to build a prerelease (beta1, beta2) ...
        tagver=$(echo $tagname | awk 'BEGIN {FS="-"} {print $2}')
        prerelease=$(echo $tagname | awk 'BEGIN {FS="-"} {print $3}')
        ver="$tagver~$prerelease"
     else
        # We want to build a final release
        tagver=$(echo $tagname | awk 'BEGIN {FS="-"} {print $2}')
        ver="$tagver"
     fi
}

function redemptionBuild() {
    # force to remove $BUILDPREFIX/debian to avoid conflict with
    # $distdir/debian
    rm -Rf $BUILDPREFIX/debian/

    # Now system is ok to build and create WAB packages

    [ -d $distdir/debian ] && {
        cp -Rf $distdir/debian $BUILDPREFIX/
        [ -f $BUILDPREFIX/debian/tmp ] && mkdir $BUILDPREFIX/debian/tmp
        cp $distdir/redemptionbuild.sh $BUILDPREFIX/

        redemptioninstallprefix=/opt/wab

        # name like redemption-2.0.<svnbranchesrevision>
        # name like redemption-2.1.<svnbranchesrevision>
        [ $action == "branches" ] && ver="$ver.$branchesver"

        # Now substitute some consts
        sed -i "s|<!redemptionname!>|$REDEMPTIONNAME|g" $BUILDPREFIX/debian/control
        sed -i "s|<!redemptionver!>|$ver|g" $BUILDPREFIX/debian/control

        sed -i "s|<!redemptionname!>|$REDEMPTIONBNAME|" $BUILDPREFIX/debian/rules
        sed -i "s|<!builddir!>|$BUILDPREFIX|" $BUILDPREFIX/debian/rules
        sed -i "s|<!redemptioninstallprefix!>|$redemptioninstallprefix|" \
            $BUILDPREFIX/debian/rules
        sed -i "s|<!redemptionname!>|$REDEMPTIONNAME|" $BUILDPREFIX/redemptionbuild.sh
        sed -i "s|<!builddir!>|$BUILDPREFIX|" $BUILDPREFIX/redemptionbuild.sh
        sed -i "s|<!redemptioninstallprefix!>|$redemptioninstallprefix|" \
            $BUILDPREFIX/redemptionbuild.sh
    }

    # rename package
    [ $REDEMPTIONNAME == "redemption" ] && {
        cd $BUILDPREFIX/debian
        mv redemption.install ${REDEMPTIONNAME}.install
        mv redemption.postinst ${REDEMPTIONNAME}.postinst
        mv redemption.postrm ${REDEMPTIONNAME}.postrm
        cd -
    }

    cd $BUILDPREFIX/
    version=${ver}-lenny.$REDEMPTIONRELEASESUFFIX
    export DEBFULLNAME="WAB Dev Team"
    export DEBEMAIL="wab@wallix.com"
    dch --create -v $version --package $REDEMPTIONNAME --empty 
    sed -i "/urgency/ a\    *  Initial Release" debian/changelog
    # Build packages based on debian/rules makefile
    # Use -b to only create binary packages and -tc to clean build files.
    dpkg-buildpackage -b -tc | tee -a $LOGFILE
    cd - | tee -a $LOGFILE

    # Copy deb in packagedir
    cp $BUILDPREFIX/../${REDEMPTIONNAME}*.deb ${distdir}/packagesdir/

    printBanner "BUILT PACKAGES ARE STORED IN ${distdir}/packagesdir/ BEFORE BEING EXPORTED TO REPOSITORY"
}

function exportRedemptionPackagesToRepo() {
    # export redemption packages into wab $exportversion repository
    local exportversion=$1

    # Now if $action is trunk then push package to wab2-dev upload folder
    # If the $action is tags so upload in wab2-<tagversion> upload folder
    # If the $ action is branches so upload in wab2-<brancheversion>-dev

    mainfolder="upload/dists/lenny/wab2"

    [ "$action" == "trunk" ] && folder="$mainfolder/wab2-dev"
    [ "$action" == "tags" ] && folder="$mainfolder/wab2-$exportversion"
    [ "$action" == "branches" ] && folder="$mainfolder/$tagname-dev"

    echo "Start export all packages in upload $folder ... "
    lftp -c "open -e \"mput ${distdir}/packagesdir/*.deb -O ./${folder}/\" $DEPOTBASEURL"

    touch ${distdir}/packagesdir/done
    lftp -c "open -e \"mput ${distdir}/packagesdir/done -O ./${folder}/\" $DEPOTBASEURL"
    return
}
