pack:
	rm external.odt
	cd package_odt ; zip -r ../external.odt .

install: pack
	cat external.odt >| /tmp/odt/external.odt
