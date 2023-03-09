# This file is part of MXE. See LICENSE.md for licensing information.

# need to remove the -Werror flags to get this to work

PKG             := goffice
$(PKG)_DEPS     := cc libgsf librsvg

define $(PKG)_BUILD
    cd '$(BUILD_DIR)' && PKG_CONFIG=/media/modus/External2/mxe/usr/bin/i686-w64-mingw32.static-pkg-config '/media/modus/External2/abiword/goffice-0.10.55/configure' \
        $(MXE_CONFIGURE_OPTS) \
        --infodir='$(BUILD_DIR)/sink'
    $(MAKE) -C $(BUILD_DIR) -j '$(JOBS)'
    $(MAKE) -C $(BUILD_DIR) -j 1 install $(MXE_DISABLE_DOCS)
endef
