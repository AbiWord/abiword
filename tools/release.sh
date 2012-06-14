#!/bin/bash

RELEASE="2.9.3"
SVNUSER="hub"

mkdir abiword-release-dir-$RELEASE
cd abiword-release-dir-$RELEASE

svn copy -m "Tag release $RELEASE" svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword/trunk svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword/tags/release-$RELEASE
svn copy -m "Tag release $RELEASE" svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword-docs/trunk svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword-docs/tags/release-$RELEASE
svn copy -m "Tag release $RELEASE" svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword-msvc2008/trunk svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword-msvc2008/tags/release-$RELEASE

svn export svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword/tags/release-$RELEASE abiword-$RELEASE
svn export svn+ssh://$SVNUSER@svn.abisource.com/svnroot/abiword-docs/tags/release-$RELEASE abiword-docs-$RELEASE

cd abiword-$RELEASE
./autogen.sh && make distcheck

cd ../abiword-docs-$RELEASE
./autogen.sh && make dist

