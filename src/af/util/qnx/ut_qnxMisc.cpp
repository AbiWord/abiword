
#include "ut_misc.h"
#include "ut_assert.h"
#include <sys/time.h>
/*!
    UT_gettimeofday() fills in the timeval structure with current
    time; the platform implementation needs to be as accurate as
    possible since this function is used in the UT_UUID class.
 */
void UT_gettimeofday(struct timeval *tv)
{
	gettimeofday(tv,NULL);
}

/*!
    retrieve the 6-byte address of the network card; returns true on success
*/
bool UT_getEthernetAddress(UT_EthernetAddress &a)
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return false;
}

