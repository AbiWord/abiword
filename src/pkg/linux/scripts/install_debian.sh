#!/bin/sh

chown -R root.root $1
dpkg-deb -b $1 $2
