#!/bin/bash

BRANCH="branches/ABI-3-0-0-STABLE"
RELEASE="3.0.1"

# check for a svn checkout
svn info > /dev/null 2>&1 || echo "Must be in a SVN checkout"

mkdir abiword-release-dir-$RELEASE
cd abiword-release-dir-$RELEASE

svn copy -m "Tag release $RELEASE" ^/abiword/$BRANCH ^/abiword/tags/release-$RELEASE
svn copy -m "Tag release $RELEASE" ^/abiword-docs/$BRANCH ^/abiword-docs/tags/release-$RELEASE
svn copy -m "Tag release $RELEASE" ^/abiword-msvc2008/$BRANCH ^/abiword-msvc2008/tags/release-$RELEASE

svn export ^/abiword/tags/release-$RELEASE abiword-$RELEASE
svn export ^/abiword-docs/tags/release-$RELEASE abiword-docs-$RELEASE

cd abiword-$RELEASE
./autogen.sh && make distcheck

cd ../abiword-docs-$RELEASE
./autogen.sh && make dist

