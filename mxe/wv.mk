# This file is part of MXE. See LICENSE.md for licensing information.

PKG             := wv
$(PKG)_DEPS     := cc libgsf

define $(PKG)_BUILD
    cd '$(BUILD_DIR)' && PKG_CONFIG=/media/modus/External2/mxe/usr/bin/i686-w64-mingw32.static-pkg-config '/media/modus/External2/abiword/wv/configure' \
        $(MXE_CONFIGURE_OPTS) \
        --infodir='$(BUILD_DIR)/sink'
    $(MAKE) -C $(BUILD_DIR) -j '$(JOBS)'
    $(MAKE) -C $(BUILD_DIR) -j 1 install $(MXE_DISABLE_DOCS)
endef
