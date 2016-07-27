
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(COMMAND_CFLAGS) \
	-I$(top_srcdir)/src/af/tf/xp/ \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiTest.cpp \
	AbiTest.h \
	../../../src/af/util/xp/t/ut_bytebuf.t.cpp \
	../../../src/af/util/xp/t/ut_misc.t.cpp \
	../../../src/af/util/xp/t/ut_string_class.t.cpp \
	../../../src/af/util/xp/t/ut_std_string.t.cpp \
	../../../src/af/util/xp/t/ut_types.t.cpp \
	../../../src/af/util/xp/t/ut_vector.t.cpp \
	../../../src/af/util/xp/t/ut_string.t.cpp \
	../../../src/af/util/xp/t/ut_uuid.t.cpp \
	../../../src/af/util/xp/t/ut_locale.t.cpp \
	../../../src/text/ptbl/xp/t/pt_PieceTable.t.cpp \
	../../../src/text/ptbl/xp/t/pp_PropertyMap.t.cpp \
	../../../src/text/ptbl/xp/t/pp_AttrProp.t.cpp \
	../../../src/text/ptbl/xp/t/pf_Fragments.t.cpp \
	../../../src/wp/impexp/xp/t/ie_mailmerge.t.cpp \
	../../../src/wp/impexp/xp/t/ie_Table.t.cpp \
	$(NULL)

