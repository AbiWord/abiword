In order to build the deb package for hildon do the following:

change into the abi directory, and then

ln -s src/pkg/hildon/debian debian
dpkg-buildpackage -rfakeroot
