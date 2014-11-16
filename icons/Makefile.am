public_icons_themes = \
        hicolor \
        $(NULL)

public_icons = \
        hicolor_apps_16x16_abiword.png \
        hicolor_apps_22x22_abiword.png \
        hicolor_apps_32x32_abiword.png \
        hicolor_apps_48x48_abiword.png \
        hicolor_apps_256x256_abiword.png \
        hicolor_apps_512x512_abiword.png \
        hicolor_apps_scalable_abiword.svg \
        $(NULL)

icondir= $(ABIWORD_ICONDIR)

gtk_update_icon_cache = gtk-update-icon-cache -f -t

install-data-local: install-icons update-icon-cache
uninstall-local: uninstall-icons update-icon-cache

update-icon-cache:
	@-if test -z "$(DESTDIR)"; then \
		echo "Updating Gtk icon cache."; \
		for theme in $(public_icons_themes); do \
			$(gtk_update_icon_cache) $(icondir)/$$theme; \
		done; \
	else \
		echo "*** Icon cache not updated.  After (un)install, run this:"; \
		for theme in $(public_icons_themes); do \
			echo "***   $(gtk_update_icon_cache) $(icondir)/$$theme"; \
		done; \
	fi

install-icons:
	for icon in $(public_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		mkdir -p $(DESTDIR)$(datadir)/icons/$$THEME/$$SIZE/$$CONTEXT; \
		$(INSTALL_DATA) $(srcdir)/$$icon $(DESTDIR)$(icondir)/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done

uninstall-icons:
	-for icon in $(public_icons); do \
		THEME=`echo $$icon | cut -d_ -f1`; \
		CONTEXT=`echo $$icon | cut -d_ -f2`; \
		SIZE=`echo $$icon | cut -d_ -f3`; \
		ICONFILE=`echo $$icon | cut -d_ -f4`; \
		rm -f $(DESTDIR)$(icondir)/$$THEME/$$SIZE/$$CONTEXT/$$ICONFILE; \
	done

EXTRA_DIST = \
        $(public_icons)         \
	$(NULL)

