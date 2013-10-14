moc_%.cpp : %.h
	$(MOC) $< -o $@

clean-moc-extra:
	rm -vf moc_*.cpp

clean-am: clean-moc-extra
