#!/bin/bash

BRANCH="ABI-3-1-0-STABLE"
RELEASE="3.1.0"
RELEASE_BASE_DIR=~/tmp
RELEASE_DIR="$RELEASE_BASE_DIR/abiword-release-dir-$RELEASE"

TAG=release-$RELEASE

# check for a git checkout
if [ `git branch | grep $BRANCH | wc -l` -ne 1 ] ; then
    echo "Must be in a git checkout"
    exit 1
fi

if [ `git tag -l $TAG | wc -l` -ne 0 ] ; then
    echo "Already found a tag for $RELEASE"
    exit 1
fi

if [ -d "$RELEASE_DIR" ] ; then
    echo "Unclean release. Directory $RELEASE_DIR exists."
    exit 2
fi

if [ ! -d "$RELEASE_BASE_DIR" ] ; then
    echo "I am about to create ~/tmp."
    echo "Continue [y/n]?"
    read c
    if [ x$c != "xy" ] ; then
        echo "Cancelled"
        exit 3
    fi
fi

mkdir -p $RELEASE_DIR

echo "About to tag. Your GPG passphrase will be requested"
git tag -s -m "release $RELEASE" $TAG $BRANCH || exit 4
# For now we'll skip the other modules.
# There was no change from 3.0.2
# svn copy -m "Tag release $RELEASE" ^/abiword-docs/$BRANCH ^/abiword-docs/tags/release-$RELEASE
# svn copy -m "Tag release $RELEASE" ^/abiword-msvc2008/$BRANCH ^/abiword-msvc2008/tags/release-$RELEASE

mkdir $RELEASE_DIR/abiword-$RELEASE
git archive --format=tar $TAG | tar -x -C $RELEASE_DIR/abiword-$RELEASE
#svn export ^/abiword-docs/tags/release-$RELEASE $RELEASE_DIR/abiword-docs-$RELEASE

cd $RELEASE_DIR

cd abiword-$RELEASE
./autogen.sh && make distcheck

if [ -d ../abiword-docs-$RELEASE ] ; then
    cd ../abiword-docs-$RELEASE
    ./autogen.sh && make dist
fi

echo "Will sign the tarball. Your GPG passphrase will be requested"
sha256sum abiword-*$RELEASE.tar.gz > SHA256SUM
sha1sum abiword-*$RELEASE.tar.gz > SHA1SUM
gpg -ba abiword-*$RELEASE.tar.gz

echo "Tarball is ready in $RELEASE_DIR/abiword-$RELEASE/abiword-$RELEASE.tar.gz."
echo "After everything is OK don't forget to git push --tags."
