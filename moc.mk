moc_%.cpp : %.h
	moc $< -o $@

clean-moc-extra:
	rm -vf moc_*.cpp

clean-am: clean-moc-extra
