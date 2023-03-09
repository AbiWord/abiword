You WILL need additional work to make this go.

make MXE_PLUGIN_DIRS=plugins/apps/abiword abiword

notes:

* need to remove the manpages to get Enchant to build
* need to remove the -Werror flags to get libgoffice to build
