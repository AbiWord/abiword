#ifndef XAP_UNIXENCMGR_H
#define XAP_UNIXENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_UnixEncodingManager: public XAP_EncodingManager
{
public:
    XAP_UnixEncodingManager();
    ~XAP_UnixEncodingManager();    

    const char* getNativeEncodingName() 	const;
    const char* getLanguageISOName() 		const;
    const char* getLanguageISOTerritory() 	const;
    
    void 	initialize();    
};

#endif /* XAP_UNIXENCMGR_H */
