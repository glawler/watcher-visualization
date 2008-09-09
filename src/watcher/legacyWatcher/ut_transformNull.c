//
// Unit tests for the null transform
//
#include <string.h>

#include "../hcunit/hcunit.h"
#include "transformNull.h"
#include "untransformNull.h"


static Transform *t;
static Untransform *u;


HCUNIT_ASSEMBLY_SETUP(transformNullUnitTestSetup)
{
    t = transformNullCreate();
    u = untransformNullCreate();
}

HCUNIT_ASSEMBLY_TEARDOWN(transformNullUnitTestTeardown)
{
    t->transformDestroy(t);
    u->untransformDestroy(u);
}

HCUNIT_TEST(transformNullRoundtrip)
{
    uint8_t buf[100];
    size_t i;
    TransformDataHandle tdh = t->dataHandleCreate(t);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t->transformed(tdh), t->transformedLength(tdh));
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), t->rawLength(tdh), 
            "expected %d got %d", sizeof(buf), t->rawLength(tdh));
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), u->rawLength(udh), 
            "expected %d got %d", sizeof(buf), u->rawLength(udh));
    HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, u->raw(udh), sizeof(buf)),
            "round trip failed");
    u->dataHandleDestroy(udh);
    t->dataHandleDestroy(tdh);
}
