#ifdef DEBUG
#include <stdlib.h>
#endif

#include <glade/glade.h>
#include "ut_string.h"
#include "xap_App.h"
#include "xap_UnixDialogBuilder.h"

XAP_UnixDialogBuilder *XAP_UnixDialogBuilder::_instance = 0;

XAP_UnixDialogBuilder::XAP_UnixDialogBuilder(void)
{
}

XAP_UnixDialogBuilder::~XAP_UnixDialogBuilder(void)
{
}

XAP_UnixDialogBuilder *XAP_UnixDialogBuilder::instance(void)
{
	if (_instance == 0)
		_instance = new XAP_UnixDialogBuilder();

	return _instance;
}

GladeXML *XAP_UnixDialogBuilder::build(const char *name, XAP_App *app)
{
	char *tmp1 = UT_catPathname(app->getAbiSuiteLibDir(), _getGladeDirectory());
	char *tmp2 = UT_catPathname(tmp1, name);
	GladeXML *xml = glade_xml_new(tmp2, NULL);

	free(tmp1);
	free(tmp2);

	return xml;
}

const char *XAP_UnixDialogBuilder::_getGladeDirectory(void)
{
#ifdef HAVE_GNOME
	return "glade/gnome";
#else
	return "glade";
#endif
}
