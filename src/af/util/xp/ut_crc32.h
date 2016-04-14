#ifndef UT_CRC32_H
#define UT_CRC32_H
#include "ut_types.h"

const UT_uint32 CRC32_NEGL = 0xffffffffL;
#ifdef UT_LITTLE_ENDIAN
#define CRC32_INDEX(c) (c & 0xff)
#define CRC32_SHIFTED(c) (c >> 8)
#else
#define CRC32_INDEX(c) (c >> 24)
#define CRC32_SHIFTED(c) (c << 8)
#endif

//! CRC Checksum Calculation
class ABI_EXPORT UT_CRC32
{
public:
        enum {DIGESTSIZE = 4};
        UT_CRC32();
        void Fill(const char *input);
        void Fill(const char *input, UT_uint32 length);
        void Fill(const unsigned char *input, UT_uint32 length);
        UT_uint32 DigestSize() const {return DIGESTSIZE;}

        void UpdateByte(unsigned char b) {m_crc = m_tab[CRC32_INDEX(m_crc) ^ b] ^ CRC32_SHIFTED(m_crc);}
        unsigned char GetCrcByte(UT_uint32 i) const {return ((const unsigned char *)&(m_crc))[i];}
        UT_uint32 GetCRC32(void) const { return m_crc;}
private:
        void Reset() {m_crc = CRC32_NEGL;}

        static const UT_uint32 m_tab[256];
        UT_uint32  m_crc;
};


#endif /* UT_CRC32_H */
