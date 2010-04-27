#ifndef IE_IMP_STAROFFICE_ENCODINGS_H
#define IE_IMP_STAROFFICE_ENCODINGS_H

/** Given the charset ID from a staroffice file, returns an iconv
 * handle, or (UT_iconv_t)(-1) if none is found.
 * The handle must be closed with UT_iconv_close when it's no longer
 * used. */
extern UT_iconv_t findConverter(UT_uint8 id);

#endif  /* IE_IMP_STAROFFICE_ENCODINGS_H */
