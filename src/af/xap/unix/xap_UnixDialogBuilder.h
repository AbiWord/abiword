#ifndef _XAP_UNIXDIALOGBUILDER_H_
#define _XAP_UNIXDIALOGBUILDER_H_

#include <glade/glade.h>
#include "xap_App.h"

class XAP_UnixDialogBuilder
{
public:
	static XAP_UnixDialogBuilder *instance(void);
	GladeXML *build(const char *name, XAP_App *app);
	virtual ~XAP_UnixDialogBuilder(void);

protected:
	const char *_getGladeDirectory(void);

private:
	XAP_UnixDialogBuilder(void);
	static XAP_UnixDialogBuilder *_instance;
};

#endif // _XAP_UNIXDIALOGBUILDER_H_
