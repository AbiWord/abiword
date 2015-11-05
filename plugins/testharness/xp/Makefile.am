
noinst_LTLIBRARIES = libxp.la

AM_CPPFLAGS = \
	$(COMMAND_CFLAGS) \
	-I$(top_srcdir)/src/af/tf/xp/ \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiTest.cpp \
	AbiTest.h \
	$(top_srcdir)/src/af/util/xp/t/ut_bytebuf.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_misc.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_string_class.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_std_string.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_types.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_vector.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_string.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_uuid.t.cpp \
	$(top_srcdir)/src/af/util/xp/t/ut_locale.t.cpp \
	$(top_srcdir)/src/text/ptbl/xp/t/pt_PieceTable.t.cpp \
	$(top_srcdir)/src/text/ptbl/xp/t/pp_PropertyMap.t.cpp \
	$(top_srcdir)/src/text/ptbl/xp/t/pf_Fragments.t.cpp \
	$(NULL)

