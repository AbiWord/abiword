These scripts can generate valid GNU gettext PO files from the existing AbiWord Strings implementation. This is some excellent work done by Kenneth Christiansen <kenneth@ripen.dk>. Use abi-port-all.sh to recreate all of the relevant potfiles.

Dom

Now, to create the files, use make in any UNIX-ish environment:
make update-po
	Update the .po files to contain new to-be
	translated strings.
make translate
	Create a new translation
make
	Convert the .po files to .mo files
make install
	Install the .mo files; necessary before you can see
	the existing or updated translated messages
