
#include <vector>
#include <algorithm>

#include "tf_test.h"
#include "ut_uuid.h"

#define TFSUITE "core.af.util.uuid"


//#include "ut_endian.h"
#include "ut_rand.h"

struct test_record
{
  UT_uint32 val;
  UT_uint32 indx;
};

static int s_cmp_hash(const test_record &i1, const test_record &i2)
{
  return (i1.val < i2.val);
}

void UT_UUIDGenerator__test(UT_UUIDGenerator* self)
{
  // test hashes ...
  UT_DEBUGMSG(("------------------------- Testing uuid hash() ---------------------------\n"));
  std::vector<test_record> v;
  const UT_uint32 iMax = 512000;
  const UT_uint32 iMsg = 5000;
  const UT_uint32 iTest = 10;
  UT_uint32 iColHTotal = 0;
  UT_uint32 iDeltaMinH = 0xffffffff;

  // create a dummy uuid instance ...
  if(!self->m_pUUID)
    self->m_pUUID = new UT_UUID;

  for (UT_uint32 k = 0; k < iTest; ++k)
  {
    UT_uint32 j;
    UT_uint32 iColH = 0;
    UT_uint32 iDMinH = 0xffffffff;

    TF_Test::pulse();

    for(j = 0; j < iMax; ++j)
    {
      //makeUUID();

      // on similar strings, the glib hash performs much better;
      // let's test it on random strings
      UT_uint32 * p = (UT_uint32 *)&(self->m_pUUID->m_uuid);

      for(UT_uint32 n = 0; n < 4; n++)
        p[n] = UT_rand();

      test_record t;
      t.val = self->m_pUUID->hash32();
      t.indx = j;
      v.push_back(t);

//      if(0 == j % iMsg)
//        UT_DEBUGMSG(("Round %d: Generating rand %d of %d\n", k, j, iMax));
    }

    TF_Test::pulse();

    std::sort(v.begin(), v.end(), s_cmp_hash);

    for(j = 0; j < iMax - 1; ++j)
    {
      const test_record &t1 = v.at(j);
      const test_record &t2 = v.at(j+1);

      if(t1.val == t2.val)
      {
        UT_DEBUGMSG(("Round %04d: uuid hash() collision (value: %u)\n", k, t1.val));
        UT_uint32 i1 = t1.indx > t2.indx ? t1.indx : t2.indx;
        UT_uint32 i2 = t1.indx < t2.indx ? t1.indx : t2.indx;
        iDMinH = iDMinH < (UT_uint32)(i1-i2) ? iDMinH : (UT_uint32)i1-i2;
        iColH++;
      }

//      if(0 == j % iMsg)
//        UT_DEBUGMSG(("Round %04d: testing %d of %d\n", k, j, iMax));
    }

    UT_DEBUGMSG(("RESULTS: round %04u: %u hash collisions (min distance %u)\n",
                 k, iColH, iDMinH));

    iColHTotal += iColH;
    iDeltaMinH = iDeltaMinH < iDMinH ? iDeltaMinH : iDMinH;
    v.clear();
  }

  UT_DEBUGMSG(("CUMULATIVE RESULTS (of %d): %d hash collisions (min distance %d)\n",
               iMax*iTest, iColHTotal, iDeltaMinH));

  // delete the dummy uuid instance so that any genuine calls to the
  // hash functions allocate a proper derived instance
  if(self->m_pUUID)
  {
    delete self->m_pUUID;
    self->m_pUUID = NULL;
  }

  UT_DEBUGMSG(("---------------------- Testing uuid hash END --------------------------\n"));
}

TFTEST_MAIN("UUID")
{
  {
    UT_UUIDGenerator generator;

//    UT_UUIDGenerator__test(&generator);
  }
  {
    UT_UUIDGenerator generator;

    UT_UUID *uuid = generator.createUUID();

    TFPASS(uuid->isValid());
    TFPASS(!uuid->isNull());

    delete uuid;

  }
}
