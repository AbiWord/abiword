#ifndef XAP_UNIXENCMGR_H
#define XAP_UNIXENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_UnixEncodingManager: public XAP_EncodingManager
{
protected:
    XAP_UnixEncodingManager();
    virtual ~XAP_UnixEncodingManager();    

    void 	initialize();    

public:
    const char* getNativeEncodingName() 	const;
    const char* getNative8BitEncodingName()	const;
    const char* getNativeUnicodeEncodingName() 	const;
    const char* getLanguageISOName() 		const;
    const char* getLanguageISOTerritory() 	const;

	friend class XAP_EncodingManager;
};

#endif /* XAP_UNIXENCMGR_H */
