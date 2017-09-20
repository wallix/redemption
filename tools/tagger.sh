#!/bin/bash

set -e

version_file=include/main/version.hpp
current_version=
progname="$0"

load_version ()
{
    current_version=$(sed -E '/#d/!d;s/.* VERSION "([^"]+)".*/\1/' "$version_file")
}

usage ()
{
    echo 'usage:' >&2
    echo "  $progname -u,--update-version new-version [-f,--force-version] [-p,--push] [-r,--repo-path path]" >&2
    echo "  $progname -g,--get-version [-r,--repo-path path]" >&2
    [ -z "$current_version" ] && cd "$(dirname "$progname")"/.. && load_version
    echo "current version: $current_version"
    exit 1
}


cd "$(dirname "$0")"/..

TEMP=`getopt -o 'hgpfu:r:' -l 'help,get-version,push,force-version,update-version:,repo-path:' -- "$@"`

if [ $? != 0 ] ; then usage >&2 ; fi

eval set -- "$TEMP"

new_version=
gver=0
push=0
packager=modules/packager/packager.py
force_vers=0
repo_path=''
while true; do
  case "$1" in
    -h|--help) usage ; exit 0 ;;
    -g|--get-version) gver=1; shift ;;
    -p|--push) push=1; shift ;;
    -f|--force-version) force_vers=1; shift ;;
    -r|--repo-path)
        [ -z "$2" ] && usage
        cd "$2"
        load_version
        shift 2 ;;
    -u|--update-version)
        [ -z "$2" ] && usage
        new_version="$2"
        shift 2 ;;
    --) shift; break ;;
    * ) usage; exit 1 ;;
  esac
done

if [ -z "$current_version" ] ; then
    cd "$(dirname "$progname")"/..
    load_version
fi


if [ $gver = 1 ] ; then
    echo "$current_version"
    exit
fi


if [ -z "$new_version" ] ; then
    echo missing --new-version
    usage
fi

if [ $force_vers = 0 ] && [ "$current_version" = "$new_version" ] ; then
    echo version is already to "$new_version"
    exit 2
fi

gdiff=$(GIT_PAGER=cat git diff --shortstat)

if [ $? != 0 ] || [ "$gdiff" != '' ] ; then
    echo -e "your repository has uncommited changes:\n$gdiff\nPlease commit before packaging." >&2
    exit 2
fi

check_tag ()
{
    grep -m1 -o "^$new_version$" && {
        echo "tag $new_version already exists ("$1")."
        exit 2
    }
}

git tag --list | check_tag locale
git ls-remote --tags origin | sed 's#.*/##' | check_tag remote

sed s/'VERSION .*/VERSION "'"$new_version"'"/' -i "$version_file"

git commit -am "Version $new_version" && git tag "$new_version" && {
    [ $push = 1 ] && git push && git push --tags
}

exit $?
