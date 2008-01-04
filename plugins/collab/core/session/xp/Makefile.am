
noinst_LTLIBRARIES = libxp.la

libxp_la_CPPFLAGS = \
	$(COLLAB_CFLAGS) \
	-I$(srcdir)/.. \
	-I$(srcdir)/../../.. \
	-I$(srcdir)/../../dialogs \
	-I$(srcdir)/../../packet \
	-DABI_BUILD_VERSION=\"$(VERSION)\"

libxp_la_SOURCES = \
	AbiCollab_Command.cpp \
	AbiCollab_Command.h \
	AbiCollab.cpp \
	AbiCollab_Export.cpp \
	AbiCollab_Export.h \
	AbiCollab.h \
	AbiCollab_Import.cpp \
	AbiCollab_Import.h \
	AbiCollab_ImportRuleSet.cpp \
	AbiCollab_ImportRuleSet.h \
	AbiCollab_Interface.cpp \
	AbiCollab_Interface.h \
	AbiCollab_Plugin.cpp \
	AbiCollab_Plugin.h \
	AbiCollab_Regression.cpp \
	AbiCollab_Regression.h \
	AbiCollabSessionManager.cpp \
	AbiCollabSessionManager.h \
	DiskSessionRecorder.cpp \
	DiskSessionRecorder.h

