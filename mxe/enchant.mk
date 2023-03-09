# This file is part of MXE. See LICENSE.md for licensing information.

# need to remove the manpages to get this to work!

PKG             := enchant
$(PKG)_DEPS     := cc

define $(PKG)_BUILD
    cd '$(BUILD_DIR)' && '/media/modus/External2/abiword/enchant/configure' \
        $(MXE_CONFIGURE_OPTS) \
        --infodir='$(BUILD_DIR)/sink'
    $(MAKE) -C $(BUILD_DIR) -j '$(JOBS)'
    $(MAKE) -C $(BUILD_DIR) -j 1 install $(MXE_DISABLE_DOCS)
endef
