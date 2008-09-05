/*
 * ut_BufferPair.c - unit tests for BufferPair
 */

#include <ctype.h> // for isprint()
#include <netinet/in.h>
#include "../hcunit/hcunit.h"
#include "../protection/parse.h"
#include "idsCommunications.h"
#include "transDuo.h"
#include "bufferPair.h"
#include "transformSign.h"
#include "untransformSign.h"

static struct TransDuoManager *m = 0;
static struct Transform *ts = 0;


///////////////////////////////////////////////////////////////////////
//  The keys (private but can be read as public too).
///////////////////////////////////////////////////////////////////////
static char const *key = 
   "192.168.1.1 p9qe7ao5kCy75B2feymy5cv5v0liA7ga6jD8yDCC06l0obzvrwqz6g"
   "4clwfkp1eg5A657n7f1fD8pyewc8yiAlv32xyhfcfu|000003|jes1l0snq9eg|vhk"
   "vwwiD4lva2k3b9Bel43r2ebaEAhf8ko9Ci2zw8e70brmlyt2xkzplmr0cnux57g6r6"
   "3A71jtosegbmf31rp2q1f0rrpDs|bqop4fq4w7vkrxol4E77wmC3m9Bcd8tdcozsbg"
   "4gznzlBm71|4AcEydwcbjrnAj25jsgxB9jkm7odmgqBvaqpswqup8zBha5C|xfw8jq"
   "49zEa29tuusx61ugsu7xc57cxwyym82wl0skd2v8aD|k51xymhgz57dimge3d4wlsD"
   "22jc4vyjmm2Bjwcsi4Ensbew0|fp7ur7llyEifAre73vb8CjEdr61q5n5o83r46yiu"
   "q7nCp6vc|83yzaalCCEwj";

static ManetAddr addr;


#if 0
// for debugging
static void dump(char const *title, uint8_t const *buf, size_t len)
{
    size_t i = 1;
    uint8_t const *c = buf;
    uint8_t const *cend;
    char line[40];
    char line2[20];
    char *p = line;
    char *p2 = line2;
    cend = c + len;
    printf("%s: dumping %d byte buffer\n", title, len);
    while(c != cend)
    {
        p += sprintf(p, "%02x", *c);
        p2 += sprintf(p2, "%c", isprint(*c) ? *c : '.');
        if((i % 16) == 0)
        {
            printf("%s: %s %s\n", title, line, line2);
            p = line;
            p2 = line2;
        } else if((i % 4) == 0) {
            p += sprintf(p, " ");
            if((i % 8) == 0) {
                p2 += sprintf(p2, "-");
            }
        }
        ++i;
        ++c;
    }
    if((i - 1) % 16)
    {
        printf("%s: %-35.35s %s\n", title, line, line2);
    }
    return;
} /* dump */
#endif


//
// Setup the transform and untransform
//
HCUNIT_ASSEMBLY_SETUP(bufferPairUnitTestSetup)
{
    static char const *fname = "bufferPairUnitTest.tmp";
    FILE *fp;
    struct in_addr a;
    HCUNIT_ASSERT_TRUE(
            fp = fopen(fname, "w"), 
            "Failed to open %s for writing", fname);
    fprintf(fp, "%s\n", key);
    fclose(fp);
    HCUNIT_ASSERT_TRUE(ts = transformSignCreate(fname), 
            "Failed transformSignCreate()");
    parseIPv4(&a, key);
    addr = (ManetAddr)ntohl(a.s_addr);
    HCUNIT_ASSERT_TRUE(fp = fopen(fname, "w"), 
            "Failed to open %s for writing", fname);
    transDuoInit();
    HCUNIT_ASSERT_TRUE(m = transDuoManagerCreate(), 
            "Failed to create TransDuo manager");
    transDuoUntransformRegister(m, untransformSignCreate(fname));
    unlink(fname);
    return;
}

HCUNIT_ASSEMBLY_TEARDOWN(bufferPairUnitTestTeardown)
{
    ts->transformDestroy(ts);
    transDuoManagerDestroy(m);
    transDuoFini();
}

HCUNIT_TEST(testRoundTripBufferPairFromTransDuo)
{
    struct TransDuo *td = transDuoCreate(m);
    HCUNIT_ASSERT_TRUE(td, "Failed to make TransDuo");
    HCUNIT_ASSERT_FALSE(transDuoAdd(td, "hello", 5, ts), "Failed to make TransDuo");
    struct BufferPair *bp = bufferPairFromTransDuo(td);
    HCUNIT_ASSERT_TRUE(bp, "Failed to make BufferPair");
    void const *buf = bufferPairMarshal(bp);
    size_t buflen = bufferPairMarshalLength(bp);
    HCUNIT_ASSERT_TRUE(buf && buflen, 
            "Failed to make marshaled BufferPair, %s==0", 
            buf ? "buflen" : buflen ? "buf" : "buf and buflen");
    struct BufferPair *check = bufferPairFromMarshaled(buf, buflen);
    HCUNIT_ASSERT_ARE_EQUAL(transDuoTransformedLengthGet(td), 
            bufferPairFirstLength(check), 
            "Expected %d got %d bytes on the transformed side",
            transDuoTransformedLengthGet(td), 
            bufferPairFirstLength(check));
    HCUNIT_ASSERT_FALSE(
            memcmp(
                transDuoTransformedGet(td),
                bufferPairFirst(check),
                bufferPairFirstLength(check)),
                "Transformed buffer doesn't match");
    HCUNIT_ASSERT_ARE_EQUAL(transDuoRawLengthGet(td), 
            bufferPairSecondLength(check), 
            "Expected %d got %d bytes on the raw side",
            transDuoRawLengthGet(td), 
            bufferPairSecondLength(check));
    HCUNIT_ASSERT_FALSE(
            memcmp(
                transDuoRawGet(td),
                bufferPairSecond(check),
                bufferPairSecondLength(check)),
                "Raw buffer doesn't match");
    bufferPairDestroy(check);
    bufferPairDestroy(bp);
    transDuoDestroy(td);
}

HCUNIT_TEST(testRoundTripBufferPairBuffers)
{
    char const buf1[] = "hello";
    char const buf2[] = "goodbye";
    struct BufferPair *bp = 
        bufferPairFromBuffers(
                buf1, sizeof(buf1),
                buf2, sizeof(buf2));
    HCUNIT_ASSERT_TRUE(bp, "Failed to make BufferPair");
    void const *buf = bufferPairMarshal(bp);
    size_t buflen = bufferPairMarshalLength(bp);
    HCUNIT_ASSERT_TRUE(buf && buflen, 
            "Failed to make marshaled BufferPair, %s==0", 
            buf ? "buflen" : buflen ? "buf" : "buf and buflen");
    struct BufferPair *check = bufferPairFromMarshaled(buf, buflen);
    HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf1),
            bufferPairFirstLength(check), 
            "Expected %d got %d bytes on the transformed side",
            sizeof(buf1),
            bufferPairFirstLength(check));
    HCUNIT_ASSERT_FALSE(
            memcmp(
                buf1,
                bufferPairFirst(check),
                bufferPairFirstLength(check)),
                "Transformed buffer doesn't match");
    HCUNIT_ASSERT_ARE_EQUAL(
            sizeof(buf2),
            bufferPairSecondLength(check), 
            "Expected %d got %d bytes on the raw side",
            sizeof(buf2),
            bufferPairSecondLength(check));
    HCUNIT_ASSERT_FALSE(
            memcmp(
                buf2,
                bufferPairSecond(check),
                bufferPairSecondLength(check)),
                "Raw buffer doesn't match");
    bufferPairDestroy(check);
    bufferPairDestroy(bp);
}

HCUNIT_TEST(testBufferPairDups)
{
    char const buf1[] = "hello";
    char const buf2[] = "goodbye";
    char *check;
    struct BufferPair *bp = 
        bufferPairFromBuffers(
                buf1, sizeof(buf1),
                buf2, sizeof(buf2));
    HCUNIT_ASSERT_TRUE(bp, "Failed to make BufferPair");
    check = bufferPairFirstDup(bp);
    HCUNIT_ASSERT_FALSE(
            memcmp(buf1, check, sizeof(buf1)),
            "duplicated first doesn't match");
    free(check);
    check = bufferPairSecondDup(bp);
    HCUNIT_ASSERT_FALSE(
            memcmp(buf2, check, sizeof(buf2)),
            "duplicated second doesn't match");
    free(check);
    check = bufferPairMarshalDup(bp);
    HCUNIT_ASSERT_FALSE(
            memcmp(bufferPairMarshal(bp), check,
                bufferPairMarshalLength(bp)),
            "duplicated marshaled doesn't match");
    free(check);
    bufferPairDestroy(bp);
}

