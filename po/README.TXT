These scripts can generate valid GNU gettext PO files from the existing AbiWord Strings implementation. This is some excellent work done by Kenneth Christiansen <kenneth@ripen.dk>. Use abi-update-all.sh to recreate all of the relevant potfiles.

Dom

In order to work on language XY and to update the XY translation,

  run:  update.pl XY
  edit: XY.po

The make utility will run 'ui-backport.pl XY.po XY.strings' at build time then which creates the AbiWord Strings implementation.

ib
