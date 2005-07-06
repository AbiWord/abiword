In order to build the deb package for hildon do the following:

change into the abi directory, and then

ln -s src/pkg/hildon/debian debian
ln -s src/pkg/hildon/debian/misc/clean_dir_struct.sh clean_dir_struct.sh
dpkg-buildpackage -rfakeroot
