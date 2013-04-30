#ifndef XAP_QT_ENCMGR_H
#define XAP_QT_ENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_QtEncodingManager: public XAP_EncodingManager
{
protected:
    XAP_QtEncodingManager();
    virtual ~XAP_QtEncodingManager();    

    void 	initialize();    

public:
    const char* getNativeEncodingName() 	const;
    const char* getNativeSystemEncodingName() 	const;
    const char* getNative8BitEncodingName()	const;
    const char* getNativeNonUnicodeEncodingName() 	const;
    const char* getNativeUnicodeEncodingName() 	const;
    const char* getLanguageISOName() 		const;
    const char* getLanguageISOTerritory() 	const;

	friend class XAP_EncodingManager;
};

#endif /* XAP_QTENCMGR_H */
