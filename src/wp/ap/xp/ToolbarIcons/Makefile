
XPMS = $(shell ls *.xpm)
PNGS    = $(XPMS:.xpm=.png)
CONVERT = convert

all:: png

png: $(PNGS)


clean: 
	rm -fr $(PNGS)

%.png: %.xpm
	$(CONVERT) $< $@

.PHONY: all png clean
