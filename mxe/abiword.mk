# This file is part of MXE. See LICENSE.md for licensing information.

# TOOLKIT target for windows must be forced to GTK

PKG             := abiword
$(PKG)_DEPS     := cc libgsf libxslt enchant wv goffice

define $(PKG)_BUILD
    cd '$(BUILD_DIR)' && '/media/modus/External2/abiword/abiword/configure' \
        $(MXE_CONFIGURE_OPTS) \
        --infodir='$(BUILD_DIR)/sink'
    $(MAKE) -C $(BUILD_DIR) -j '$(JOBS)'
    $(MAKE) -C $(BUILD_DIR) -j 1 install $(MXE_DISABLE_DOCS)
endef
