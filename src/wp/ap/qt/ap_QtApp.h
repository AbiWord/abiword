/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */


#include "ap_App.h"

#ifndef __AP_QT_APP_H__
#define __AP_QT_APP_H__

class ABI_EXPORT AP_QtApp : public AP_App
{
public:
	AP_QtApp(const char * szAppName);
	virtual ~AP_QtApp();

        virtual bool                                    initialize(bool has_display);
	virtual bool					shutdown(void);

	virtual XAP_Frame *				newFrame(void);
	virtual GR_Graphics *           newDefaultScreenGraphics() const;

	virtual const XAP_StringSet *	                getStringSet(void) const;
	virtual const char *			        getAbiSuiteAppDir(void) const;

	virtual void					copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard = true);
	virtual void					pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting = true);
	virtual bool					canPasteFromClipboard(void);

	virtual void					cacheCurrentSelection(AV_View *);

	static int main (const char * szAppName, int argc, char ** argv);

};

#endif
