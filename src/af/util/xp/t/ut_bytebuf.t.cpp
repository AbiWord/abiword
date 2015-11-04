


#include <stdlib.h>
#include <string.h>

#include "tf_test.h"
#include "ut_bytebuf.h"

#define TFSUITE "core.af.util.bytebuf"

TFTEST_MAIN("UT_ByteBuf")
{
	UT_ByteBuf *mybuf = new UT_ByteBuf();

	UT_Byte *buf = (UT_Byte *)g_try_malloc(1000);
	memset(buf, 0, 1000);

	TFPASS(mybuf->append(buf, 1000));
	TFPASS(mybuf->getLength() == 1000);


	UT_Byte *buf2 = (UT_Byte *)g_try_malloc(500);
	memset(buf2, 0xef, 500);

	TFPASS(mybuf->ins(0, buf2, 500));
	TFPASS(mybuf->getLength() == 1500);

	UT_Byte c = 42;

	TFPASS(mybuf->append(&c, 1));
	TFPASS(mybuf->getLength() == 1501);

	// check that the layout of the buffer is OK

	const UT_Byte * ptr = mybuf->getPointer(0);
	TFPASS(ptr);
	TFPASS(memcmp(ptr, buf2, 500) == 0);

	ptr = mybuf->getPointer(500);
	TFPASS(ptr);
	TFPASS(memcmp(ptr, buf, 1000) == 0);

	ptr = mybuf->getPointer(1500);
	TFPASS(ptr);
	TFPASS(*ptr == c);

	FREEP(buf);
	FREEP(buf2);
	delete mybuf;
}
