pack:
	rm document.odt
	cd package_odt ; zip -r ../document.odt .

install: pack
	mkdir -p /tmp/odt
	cat document.odt >| /tmp/odt/document.odt

unpack:
	mkdir -p package_odt
	cd package_odt; unzip -o  ../document.odt
	echo "all:" >| package_odt/Makefile
	echo "	cd .. && make install" >> package_odt/Makefile
	cd package_odt; mv content.xml content.xml.original; xmllint --format content.xml.original >| content.xml
