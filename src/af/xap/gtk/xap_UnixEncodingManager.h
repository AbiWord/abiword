#ifndef XAP_UNIXENCMGR_H
#define XAP_UNIXENCMGR_H

#include "xap_EncodingManager.h"

/* it's assumed that only one instance of this class will exist */
class XAP_UnixEncodingManager: public XAP_EncodingManager
{
protected:
    XAP_UnixEncodingManager();
    virtual ~XAP_UnixEncodingManager();

    virtual void initialize() override;

public:
    const char* getNativeEncodingName() const override;
    const char* getNativeSystemEncodingName() const override;
    const char* getNative8BitEncodingName() const override;
    const char* getNativeNonUnicodeEncodingName() const override;
    const char* getNativeUnicodeEncodingName() const override;
    const char* getLanguageISOName() const override;
    const char* getLanguageISOTerritory() const override;

	friend class XAP_EncodingManager;
};

#endif /* XAP_UNIXENCMGR_H */
