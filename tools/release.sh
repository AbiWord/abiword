#!/bin/bash

BRANCH="branches/ABI-3-1-0-STABLE"
RELEASE="3.1.0"
RELEASE_DIR="abiword-release-dir-$RELEASE"

# check for a svn checkout
svn info > /dev/null 2>&1 
if [ $? -ne 0 ] ; then
	echo "Must be in a SVN checkout" 
	exit 1
fi

if [ -d "$RELEASE_DIR" ] ; then
	echo "Unclean release. Directory $RELEASE_DIR exists."
	exit 2
fi

mkdir $RELEASE_DIR

svn copy -m "Tag release $RELEASE" ^/abiword/$BRANCH ^/abiword/tags/release-$RELEASE
svn copy -m "Tag release $RELEASE" ^/abiword-docs/$BRANCH ^/abiword-docs/tags/release-$RELEASE
svn copy -m "Tag release $RELEASE" ^/abiword-msvc2008/$BRANCH ^/abiword-msvc2008/tags/release-$RELEASE

svn export ^/abiword/tags/release-$RELEASE $RELEASE_DIR/abiword-$RELEASE
svn export ^/abiword-docs/tags/release-$RELEASE $RELEASE_DIR/abiword-docs-$RELEASE

cd $RELEASE_DIR

cd abiword-$RELEASE
./autogen.sh && make distcheck

cd ../abiword-docs-$RELEASE
./autogen.sh && make dist

