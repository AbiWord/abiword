These scripts can generate valid GNU gettext PO files from the existing AbiWord Strings implementation. This is some excellent work done by Kenneth Christiansen <kenneth@ripen.dk>. Use abi-port-all.sh to recreate all of the relevant potfiles.

Dom

In order to work on language XY and to update the XY translation,

  run:  abi-extract.pl XY
  edit: XY.po
  run:  ui-backport.pl XY

ib
