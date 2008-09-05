//
// Unit tests for the sign transform
//
#include <string.h>
#include "../hcunit/hcunit.h"
#include "../protection/parse.h"
#include "transformSign.h"
#include "untransformSign.h"

//
// Three signers so we can test beginning, middle, and end for lookup of
// public keys.
//
static Transform *t1;
static Transform *t2;
static Transform *t3;
static ManetAddr a1;
static ManetAddr a2;
static ManetAddr a3;
//
// To hold three public keys.
//
static Untransform *u;

///////////////////////////////////////////////////////////////////////
//  The keys (private but can be read as public too).
///////////////////////////////////////////////////////////////////////
static char const *fname = "transformSignUnitTest.tmp";

static char const *key1 = 
   "192.168.1.1 p9qe7ao5kCy75B2feymy5cv5v0liA7ga6jD8yDCC06l0obzvrwqz6g"
   "4clwfkp1eg5A657n7f1fD8pyewc8yiAlv32xyhfcfu|000003|jes1l0snq9eg|vhk"
   "vwwiD4lva2k3b9Bel43r2ebaEAhf8ko9Ci2zw8e70brmlyt2xkzplmr0cnux57g6r6"
   "3A71jtosegbmf31rp2q1f0rrpDs|bqop4fq4w7vkrxol4E77wmC3m9Bcd8tdcozsbg"
   "4gznzlBm71|4AcEydwcbjrnAj25jsgxB9jkm7odmgqBvaqpswqup8zBha5C|xfw8jq"
   "49zEa29tuusx61ugsu7xc57cxwyym82wl0skd2v8aD|k51xymhgz57dimge3d4wlsD"
   "22jc4vyjmm2Bjwcsi4Ensbew0|fp7ur7llyEifAre73vb8CjEdr61q5n5o83r46yiu"
   "q7nCp6vc|83yzaalCCEwj";

static char const *key2 = 
    "192.168.1.2 Avzompfmz2xojtCn2s3vyd1zwb7ie2gkax1Ah9v2iwn0nBplj769f"
    "1emyvrabpqhtqhwsx2x029aEql6e7l4eqey6xwf6jon|000003|ibBAheorypjE|4"
    "lAtpi1Ehf8cA1j5n7g0jx5ki27l54rotgzfils5zvzm2ths3811mE2hjiCt84vbkv"
    "95CkAwimdyd9m2lk358pfrBi5g7Bhv|dBhBDc8aE930zCced1a0ozdBg6eprj6x0a"
    "wym084wqz0gcAn|wtEo7rwiazytr3f6mC8B5eynj0ljzqthpdqcwe73s4ybd2e3|i"
    "aCCt0dsDuupffuzBpzByBejtaDttf1uchqqhl3vz2702qs5|9bbCDzhzpAxqnD89m"
    "ej1fqEBavnh4q4m0789r14kcgndC8of|9j0qej9b86wli2a4fc5Ch9AtcrrEnvvE2"
    "clyln4twgmyA1n2|e8peqzz7ltC3";

static char const *key3 = 
    "192.168.1.3 A03uBnzyzj3bfbo252zdimBddCuisbiazgplvgxa7p2ra2ug04lEn"
    "1zqmprhqCe6obzidcE2uCvmlcrkp1pkzy59jfsj6xae|000003|owpa64tl13po|8"
    "3lp8wegdbv1vkpiBxz1ohwye4m5tt9sg22clejAky84ymaqcfgh1cojmAvl4k2evf"
    "iazjaE56iBBtn4ADofus5zwd4ulq1t|0k9y9bwdoio7i7dknhAk9AA04yk9td0b98"
    "mq9t9gEru1uq4q|o2pjtose51Aqul5bB6b19n4pzqx5nbxE6gez4wja3qz59C0o|u"
    "iiBzmjkxAwhvzhvDCk9j51mwDab77lsB0ogqts619e6xe9x|0dk967lmtDtwAy76B"
    "AAs5s0DfnDBa707jx1hv7vvztk16v33|sgg2BsiAusoh7Ei4kj7ek1uhbgAroAz0h"
    "0rAfmpny2nh6pe2|0yrg17gwiBs1";

#if 0
// for debugging
static void dump(char const *title, uint8_t const *buf, size_t len)
{
    size_t i = 1;
    uint8_t const *c = buf;
    uint8_t const *cend;
    char line[40];
    char *p = line;
    cend = c + len;
    printf("%s: dumping %d byte buffer\n", title, len);
    while(c != cend)
    {
        p += sprintf(p, "%02x", *c);
        if((i % 16) == 0)
        {
            printf("%s: %s\n", title, line);
            p = line;
        } else if((i % 4) == 0) {
            p += sprintf(p, " ");
        }
        ++i;
        ++c;
    }
    if((i - 1) % 16)
    {
        printf("%s: %s\n", title, line);
    }
    return;
} 
#endif

//
// Setup the three transforms and the one untransform
//
HCUNIT_ASSEMBLY_SETUP(transformSignUnitTestAssemblySetup)
{
    FILE *fp = fopen(fname, "w");
    if(fp)
    {
        fprintf(fp, "%s\n", key1);
        fclose(fp);
        HCUNIT_ASSERT_TRUE(t1 = transformSignCreate(fname), 
                "Failed transformSignCreate()");
        fp = fopen(fname, "w");
        fprintf(fp, "%s\n", key2);
        fclose(fp);
        t2 = transformSignCreate(fname);
        fp = fopen(fname, "w");
        fprintf(fp, "%s\n", key3);
        fclose(fp);
        t3 = transformSignCreate(fname);
        fp = fopen(fname, "w");
        fprintf(fp, "%s\n", key1);
        fprintf(fp, "%s\n", key2);
        fprintf(fp, "%s\n", key3);
        fclose(fp);
        parseIPv4((struct in_addr*)&a1, key1);
        parseIPv4((struct in_addr*)&a2, key2);
        parseIPv4((struct in_addr*)&a3, key3);
        a1 = ntohl(a1);
        a2 = ntohl(a2);
        a3 = ntohl(a3);
    }
    else
    {
        HCUNIT_ASSERT_FAIL("Couldn't open \"%s\" for writing", fname);
    }
    return;
}

//
// Clean up
//
HCUNIT_ASSEMBLY_TEARDOWN(transformSignUnitTestAssemblyTeardown)
{
    t1->transformDestroy(t1);
    t2->transformDestroy(t2);
    t3->transformDestroy(t3);
    unlink(fname);
}

HCUNIT_TEST_SETUP(transformSignUnitTestSetup)
{
    // Make new untransform for each test to keep spurious inter-test 
    // replay events from happening.
    u = untransformSignCreate(fname);
}

HCUNIT_TEST_TEARDOWN(transformSignUnitTestTeardown)
{
    u->untransformDestroy(u);
}

//
// Test the length passed in to the transform is what we expected.
//
HCUNIT_TEST(transformSignTestRawLength)
{
    uint8_t buf[200];
    TransformDataHandle tdh = t1->dataHandleCreate(t1);
    memset(buf, 77, sizeof(buf));
    t1->setRaw(tdh, buf, sizeof(buf));
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), t1->rawLength(tdh), 
            "expected %d got %d", sizeof(buf), t1->rawLength(tdh));
    t1->dataHandleDestroy(tdh);
}


HCUNIT_TEST(transformSignTestToUntransform)
{
    uint8_t buf[100];
    size_t i;
    TransformDataHandle tdh = t1->dataHandleCreate(t1);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t1->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t1->transformed(tdh), t1->transformedLength(tdh));
    HCUNIT_ASSERT_ARE_EQUAL(
            t1->transformedLength(tdh),
            u->transformedLength(udh),
            "expected transformed length %d got %d", 
            sizeof(buf), u->rawLength(udh));
    HCUNIT_ASSERT_ARE_EQUAL(
            0, 
            memcmp(
                t1->transformed(tdh), 
                u->transformed(udh), 
                t1->transformedLength(tdh)),
            "The untransform->transformed() data is different that the "
            "transform->transformed() data");
    u->dataHandleDestroy(udh);
    t1->dataHandleDestroy(tdh);
}

//
// Test roundtrip using key1
//
HCUNIT_TEST(transformSignTestRoundtrip1)
{
    uint8_t buf[100];
    size_t i;
    TransformDataHandle tdh = t1->dataHandleCreate(t1);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t1->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t1->transformed(tdh), t1->transformedLength(tdh));
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), u->rawLength(udh), 
            "expected %d got %d", sizeof(buf), u->rawLength(udh));
    HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, u->raw(udh), sizeof(buf)),
            "round trip failed");
    u->dataHandleDestroy(udh);
    t1->dataHandleDestroy(tdh);
}

//
//
// Test roundtrip using key2
HCUNIT_TEST(transformSignTestRoundtrip2)
{
    uint8_t buf[200];
    size_t i;
    TransformDataHandle tdh = t2->dataHandleCreate(t2);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t2->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t2->transformed(tdh), t2->transformedLength(tdh));
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), u->rawLength(udh), 
            "expected %d got %d", sizeof(buf), u->rawLength(udh));
    HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, u->raw(udh), sizeof(buf)),
            "round trip failed");
    u->dataHandleDestroy(udh);
    t2->dataHandleDestroy(tdh);
}

//
// Test roundtrip using key3
//
HCUNIT_TEST(transformSignTestRoundtrip3)
{
    uint8_t buf[300];
    size_t i;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t3->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t3->transformed(tdh), t3->transformedLength(tdh));
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), u->rawLength(udh), 
            "expected %d got %d", sizeof(buf), u->rawLength(udh));
    HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, u->raw(udh), sizeof(buf)),
            "round trip failed");
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test copy transformed data from transform using
// TRANSFORM_COPY_STRAIGHT.
//
HCUNIT_TEST(transformSignTestCopyFromTransformStraight)
{
    uint8_t buf[300];
    size_t i;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    TransformDataHandle tdhc = t3->dataHandleCreate(t3);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t3->setRaw(tdh, buf, sizeof(buf));
    t3->copy(tdhc, t3->transformed(tdh), t3->transformedLength(tdh),
            TRANSFORM_COPY_STRAIGHT);
    HCUNIT_ASSERT_ARE_EQUAL(
            t3->transformedLength(tdh),
            t3->transformedLength(tdhc),
            "Expected %d got %d after copy", 
            t3->transformedLength(tdh),
            t3->transformedLength(tdhc));
    HCUNIT_ASSERT_FALSE(
            t3->raw(tdhc), "How did the copy get raw data?");
    HCUNIT_ASSERT_ARE_EQUAL(
            0,
            memcmp(
                t3->transformed(tdh),
                t3->transformed(tdhc),
                t3->transformedLength(tdh)),
            "The original transformed data is not the same as the copy");
    t3->dataHandleDestroy(tdhc);
    t3->dataHandleDestroy(tdh);
}

//
// Test copy transformed data from transform using
// TRANSFORM_COPY_AND_AMEND. The transformSign shouldn't show any
// amending.
//
HCUNIT_TEST(transformSignTestCopyFromTransformAmmend)
{
    uint8_t buf[300];
    size_t i;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    TransformDataHandle tdhc = t3->dataHandleCreate(t3);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t3->setRaw(tdh, buf, sizeof(buf));
    t3->copy(tdhc, t3->transformed(tdh), t3->transformedLength(tdh),
            TRANSFORM_COPY_AND_AMEND);
    HCUNIT_ASSERT_ARE_EQUAL(
            t3->transformedLength(tdh),
            t3->transformedLength(tdhc),
            "Expected %d got %d after copy", 
            t3->transformedLength(tdh),
            t3->transformedLength(tdhc));
    HCUNIT_ASSERT_FALSE(
            t3->raw(tdhc), "How did the copy get raw data?");
    HCUNIT_ASSERT_ARE_EQUAL(
            0,
            memcmp(
                t3->transformed(tdh),
                t3->transformed(tdhc),
                t3->transformedLength(tdh)),
            "The original transformed data is not the same as the copy");
    t3->dataHandleDestroy(tdhc);
    t3->dataHandleDestroy(tdh);
}

//
// Test copy transformed data from untransform to transform
//
HCUNIT_TEST(transformSignTestCopyThroughUntransform)
{
    uint8_t buf[300];
    size_t i;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    TransformDataHandle tdh_copy = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    for(i = 0; i < sizeof(buf); ++i)
    {
        buf[i] = (uint8_t)i;
    }
    t3->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t3->transformed(tdh), t3->transformedLength(tdh));
    t3->copy(tdh_copy, u->transformed(udh), u->transformedLength(udh),
            TRANSFORM_COPY_STRAIGHT);
    HCUNIT_ASSERT_ARE_EQUAL(
            t3->transformedLength(tdh),
            t3->transformedLength(tdh_copy),
            "Expected %d got %d after copy", 
            t3->transformedLength(tdh),
            t3->transformedLength(tdh_copy));
    HCUNIT_ASSERT_FALSE(
            t3->raw(tdh_copy), "How did the copy get raw data?");
    HCUNIT_ASSERT_ARE_EQUAL(
            0,
            memcmp(
                t3->transformed(tdh),
                t3->transformed(tdh_copy),
                t3->transformedLength(tdh)),
            "The original transformed data is not the same as the copy");
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh_copy);
    t3->dataHandleDestroy(tdh);
}

#define IPQUAD(a) \
    ((uint8_t const *)&a)[3], \
    ((uint8_t const *)&a)[2], \
    ((uint8_t const *)&a)[1], \
    ((uint8_t const *)&a)[0]

//
// Test id of sender1
//
HCUNIT_TEST(transformSignTestIdOfSender1)
{
    uint8_t buf[100];
    ManetAddr ma;
    TransformDataHandle tdh = t1->dataHandleCreate(t1);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t1->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t1->transformed(tdh), t1->transformedLength(tdh));
    ma = untransformSignSigner(udh);
    HCUNIT_ASSERT_ARE_EQUAL(a1, ma, "Expected %d.%d.%d.%d, got %d.%d.%d.%d", 
            IPQUAD(a1), IPQUAD(ma));
    u->dataHandleDestroy(udh);
    t1->dataHandleDestroy(tdh);
}

//
// Test id of sender2
//
HCUNIT_TEST(transformSignTestIdOfSender2)
{
    uint8_t buf[100];
    ManetAddr ma;
    TransformDataHandle tdh = t2->dataHandleCreate(t2);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t2->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t2->transformed(tdh), t2->transformedLength(tdh));
    ma = untransformSignSigner(udh);
    HCUNIT_ASSERT_ARE_EQUAL(a2, ma, "Expected %d.%d.%d.%d, got %d.%d.%d.%d", 
            IPQUAD(a2), IPQUAD(ma));
    u->dataHandleDestroy(udh);
    t2->dataHandleDestroy(tdh);
}

//
// Test id of sender3
//
HCUNIT_TEST(transformSignTestIdOfSender3)
{
    uint8_t buf[100];
    ManetAddr ma;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t3->transformed(tdh), t3->transformedLength(tdh));
    ma = untransformSignSigner(udh);
    HCUNIT_ASSERT_ARE_EQUAL(a3, ma, "Expected %d.%d.%d.%d, got %d.%d.%d.%d", 
            IPQUAD(a3), IPQUAD(ma));
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignVerified result
//
HCUNIT_TEST(transformSignTestUntransformSignVerified)
{
    uint8_t buf[100];
    UntransformSignResult res;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    u->setTransformed(udh, t3->transformed(tdh), t3->transformedLength(tdh));
    res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignVerified, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignVerified),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignVerificationFailure result
//
HCUNIT_TEST(transformSignTestUntransformSignVerificationFailureOnTruncation)
{
    uint8_t buf[100];
    uint8_t tbuf[500]; // big enough to hold signed buffer
    size_t tbuflen;
    UntransformSignResult res;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    tbuflen = t3->transformedLength(tdh);
    memcpy(tbuf, t3->transformed(tdh), tbuflen);
    // truncate
    u->setTransformed(udh, tbuf, tbuflen - 1);
    res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignVerificationFailure, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignVerificationFailure),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignVerificationFailure result
//
HCUNIT_TEST(transformSignTestUntransformSignVerificationFailureOnOtherIP)
{
    uint8_t buf[100];
    uint8_t tbuf[500]; // big enough to hold signed buffer
    size_t tbuflen;
    UntransformSignResult res;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    tbuflen = t3->transformedLength(tdh);
    memcpy(tbuf, t3->transformed(tdh), tbuflen);
    // change IP to another valid one
    HCUNIT_ASSERT_ARE_EQUAL(3, tbuf[3], "Expected 3rd byte to be 3");
    tbuf[3] = 1; // last octet of IP address at byte 3
    u->setTransformed(udh, tbuf, tbuflen);
    res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignVerificationFailure, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignVerificationFailure),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignVerificationFailure result
//
HCUNIT_TEST(transformSignTestUntransformSignVerificationFailureOnModified)
{
    uint8_t buf[100];
    uint8_t tbuf[500]; // big enough to hold signed buffer
    size_t tbuflen;
    UntransformSignResult res;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    tbuflen = t3->transformedLength(tdh);
    memcpy(tbuf, t3->transformed(tdh), tbuflen);
    // modified
    tbuf[10] ^= 1;
    u->setTransformed(udh, tbuf, tbuflen);
    tbuf[10] ^= 1; // restore
    res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignVerificationFailure, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignVerificationFailure),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignNoPeer result
//
HCUNIT_TEST(transformSignTestUntransformSignVerificationNoPeer)
{
    uint8_t buf[100];
    uint8_t tbuf[500]; // big enough to hold signed buffer
    size_t tbuflen;
    UntransformSignResult res;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    tbuflen = t3->transformedLength(tdh);
    memcpy(tbuf, t3->transformed(tdh), tbuflen);
    // change IP to an unknown one
    HCUNIT_ASSERT_ARE_EQUAL(3, tbuf[3], "Expected 3rd byte to be 3");
    tbuf[3] = 7; // last octet of IP address at byte 3
    u->setTransformed(udh, tbuf, tbuflen);
    res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignNoPeer, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignNoPeer),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignReplay result
//
HCUNIT_TEST(transformSignTestUntransformSignReplay)
{
    uint8_t buf[100];
    uint8_t tbuf[500]; // big enough to hold signed buffer
    size_t tbuflen;
    UntransformSignResult res1;
    UntransformSignResult res2;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh1 = u->dataHandleCreate(u);
    UntransformDataHandle udh2 = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    tbuflen = t3->transformedLength(tdh);
    memcpy(tbuf, t3->transformed(tdh), tbuflen);
    u->setTransformed(udh1, tbuf, tbuflen);
    u->setTransformed(udh2, tbuf, tbuflen);
    res1 = untransformSignResult(udh1);
    res2 = untransformSignResult(udh2);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignVerified, res1,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignVerified),
            UNTRANSFORM_SIGN_RESULT_TEXT(res1)); 
    // replay has been turned off so expect verified
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignVerified, res2,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignVerified),
            UNTRANSFORM_SIGN_RESULT_TEXT(res2)); 
    u->dataHandleDestroy(udh2);
    u->dataHandleDestroy(udh1);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignInternalError result
//
HCUNIT_TEST(transformSignTestUntransformSignInternalErrorOnShortData)
{
    uint8_t buf[100];
    uint8_t tbuf[500]; // big enough to hold signed buffer
    size_t tbuflen;
    UntransformSignResult res;
    TransformDataHandle tdh = t3->dataHandleCreate(t3);
    UntransformDataHandle udh = u->dataHandleCreate(u);
    memset(buf, 39, sizeof(buf));
    t3->setRaw(tdh, buf, sizeof(buf));
    tbuflen = t3->transformedLength(tdh);
    memcpy(tbuf, t3->transformed(tdh), tbuflen);
    // change IP to an unknown one
    HCUNIT_ASSERT_ARE_EQUAL(3, tbuf[3], "Expected 3rd byte to be 3");
    tbuf[3] = 7; // last octet of IP address at byte 3
    u->setTransformed(udh, tbuf, 3);
    res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignInternalError, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignInternalError),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
    t3->dataHandleDestroy(tdh);
}

//
// Test getting UntransformSignInternalError result
//
HCUNIT_TEST(transformSignTestUntransformSignInternalErrorOnUnsetHandle)
{
    UntransformDataHandle udh = u->dataHandleCreate(u);
    UntransformSignResult res = untransformSignResult(udh);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignInternalError, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignInternalError),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
    u->dataHandleDestroy(udh);
}

//
// Test getting UntransformSignInternalError result
//
HCUNIT_TEST(transformSignTestUntransformSignInternalErrorOnBadHandle)
{
    UntransformSignResult res = untransformSignResult(0);
    HCUNIT_ASSERT_ARE_EQUAL(UntransformSignInternalError, res,
            "Expected %s, got %s",
            UNTRANSFORM_SIGN_RESULT_TEXT(UntransformSignInternalError),
            UNTRANSFORM_SIGN_RESULT_TEXT(res)); 
}

