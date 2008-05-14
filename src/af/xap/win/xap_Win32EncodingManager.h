#ifndef XAP_WIN32ENCMGR_H
#define XAP_WIN32ENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class ABI_EXPORT XAP_Win32EncodingManager: public XAP_EncodingManager
{
protected:
    XAP_Win32EncodingManager();
    virtual ~XAP_Win32EncodingManager();    

    void 	initialize();    

public:

    const char* getNativeEncodingName() 	const;
    const char* getNativeSystemEncodingName() 	const;
    const char* getNative8BitEncodingName() const;
    const char* getNativeNonUnicodeEncodingName() const;
    const char* getNativeUnicodeEncodingName() const;
	inline virtual bool isUnicodeLocale()	const {return m_bIsUnicodeLocale;}
    const char* getLanguageISOName() 		const;
    const char* getLanguageISOTerritory() 	const;
    
	friend class XAP_EncodingManager;

private:
	bool m_bIsUnicodeLocale;
};

#endif /* XAP_WIN32ENCMGR_H */
