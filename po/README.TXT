These scripts can generate valid GNU gettext PO files from the existing AbiWord Strings implementation. This is some excellent work done by Kenneth Christiansen <kenneth@ripen.dk>. Use abi-update-all.sh to recreate all of the relevant potfiles.

Dom


In order to work on language xx-YY and to update the xx-YY translation,

  run:  update.pl xx-YY
  edit: xx-YY.po

The make utility will run 'ui-backport.pl xx-YY.po xx-YY.strings' at build time then which creates the AbiWord Strings implementation.


In order to provide a new translation for language xx-YY,

  run:    update.pl --pot
  rename: abiword.pot xx-YY.po
  edit:   xx-YY.po

and add xx-YY.strings to strings_DATA in Makefile.am.

ib
