overrides_dir = `python -c "import gi; print gi._overridesdir"`

install-data-local:
	$(INSTALL_DATA) $(top_srcdir)/src/gi-overrides/Abi.py $(overrides_dir)/Abi.py

uninstall-local:
	rm -f $(overrides_dir)/Abi.py
