#ifndef XAP_QNXENCMGR_H
#define XAP_QNXENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_QNXEncodingManager: public XAP_EncodingManager
{
protected:
    XAP_QNXEncodingManager();
    virtual ~XAP_QNXEncodingManager();    

    void 	initialize();    

public:

    const char* getNativeEncodingName() 	const;
    const char* getNative8BitEncodingName() const;
    const char* getNativeUnicodeEncodingName() const;
	inline virtual bool isUnicodeLocale()	const {return m_bIsUnicodeLocale;}
    const char* getLanguageISOName() 		const;
    const char* getLanguageISOTerritory() 	const;
    
	friend class XAP_EncodingManager;

private:
	bool m_bIsUnicodeLocale;
};

#endif /* XAP_WIN32ENCMGR_H */
