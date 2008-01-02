
SUFFIXES=.mm
# Added for automake (at least version 1.5) - Frodo Looijaard (frodol@dds.nl)
.mm.lo:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(AM_OBJCFLAGS) $(OBJCFLAGS) $<
.mm.o:
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(AM_OBJCFLAGS) $(OBJCFLAGS) $<

