//
// TransDuo unit tests.
//
#include <string.h>

#include "../hcunit/hcunit.h"
#include "../protection/parse.h"
#include "transDuo.h"
#include "transformSign.h"
#include "transformNull.h"
#include "untransformSign.h"
#include "untransformNull.h"

typedef Transform *tptr;
#define TYPE tptr
#include "../protection/VEC.h"
#undef TYPE 

#define IPQUAD(a) \
    ((uint8_t const *)&a)[3], \
    ((uint8_t const *)&a)[2], \
    ((uint8_t const *)&a)[1], \
    ((uint8_t const *)&a)[0]

static struct TransDuoManager *m = 0;
//
// All the transforms I want to test land in "t"
//
static tptr_vec t = { 0, 0, 0 };

static char const *fname = "tranDuoUnitTest.tmp";

///////////////////////////////////////////////////////////////////////
//  The keys (private but can be read as public too).
///////////////////////////////////////////////////////////////////////
static char const *key[] = 
{
   "192.168.1.1 p9qe7ao5kCy75B2feymy5cv5v0liA7ga6jD8yDCC06l0obzvrwqz6g"
   "4clwfkp1eg5A657n7f1fD8pyewc8yiAlv32xyhfcfu|000003|jes1l0snq9eg|vhk"
   "vwwiD4lva2k3b9Bel43r2ebaEAhf8ko9Ci2zw8e70brmlyt2xkzplmr0cnux57g6r6"
   "3A71jtosegbmf31rp2q1f0rrpDs|bqop4fq4w7vkrxol4E77wmC3m9Bcd8tdcozsbg"
   "4gznzlBm71|4AcEydwcbjrnAj25jsgxB9jkm7odmgqBvaqpswqup8zBha5C|xfw8jq"
   "49zEa29tuusx61ugsu7xc57cxwyym82wl0skd2v8aD|k51xymhgz57dimge3d4wlsD"
   "22jc4vyjmm2Bjwcsi4Ensbew0|fp7ur7llyEifAre73vb8CjEdr61q5n5o83r46yiu"
   "q7nCp6vc|83yzaalCCEwj",
    "192.168.1.2 Avzompfmz2xojtCn2s3vyd1zwb7ie2gkax1Ah9v2iwn0nBplj769f"
    "1emyvrabpqhtqhwsx2x029aEql6e7l4eqey6xwf6jon|000003|ibBAheorypjE|4"
    "lAtpi1Ehf8cA1j5n7g0jx5ki27l54rotgzfils5zvzm2ths3811mE2hjiCt84vbkv"
    "95CkAwimdyd9m2lk358pfrBi5g7Bhv|dBhBDc8aE930zCced1a0ozdBg6eprj6x0a"
    "wym084wqz0gcAn|wtEo7rwiazytr3f6mC8B5eynj0ljzqthpdqcwe73s4ybd2e3|i"
    "aCCt0dsDuupffuzBpzByBejtaDttf1uchqqhl3vz2702qs5|9bbCDzhzpAxqnD89m"
    "ej1fqEBavnh4q4m0789r14kcgndC8of|9j0qej9b86wli2a4fc5Ch9AtcrrEnvvE2"
    "clyln4twgmyA1n2|e8peqzz7ltC3",
    "192.168.1.3 A03uBnzyzj3bfbo252zdimBddCuisbiazgplvgxa7p2ra2ug04lEn"
    "1zqmprhqCe6obzidcE2uCvmlcrkp1pkzy59jfsj6xae|000003|owpa64tl13po|8"
    "3lp8wegdbv1vkpiBxz1ohwye4m5tt9sg22clejAky84ymaqcfgh1cojmAvl4k2evf"
    "iazjaE56iBBtn4ADofus5zwd4ulq1t|0k9y9bwdoio7i7dknhAk9AA04yk9td0b98"
    "mq9t9gEru1uq4q|o2pjtose51Aqul5bB6b19n4pzqx5nbxE6gez4wja3qz59C0o|u"
    "iiBzmjkxAwhvzhvDCk9j51mwDab77lsB0ogqts619e6xe9x|0dk967lmtDtwAy76B"
    "AAs5s0DfnDBa707jx1hv7vvztk16v33|sgg2BsiAusoh7Ei4kj7ek1uhbgAroAz0h"
    "0rAfmpny2nh6pe2|0yrg17gwiBs1"
};

static ManetAddr addr[sizeof(key)/sizeof(key[0])];

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
HCUNIT_ASSEMBLY_SETUP(transDuoUnitTestSetup)
{
    size_t i;
    FILE *fp;
    Transform *tmp;
    for(i = 0; i < sizeof(key)/sizeof(key[0]); ++i)
    {
        struct in_addr a;
        HCUNIT_ASSERT_TRUE(
                fp = fopen(fname, "w"), 
                "Failed to open %s for writing", fname);
        fprintf(fp, "%s\n", key[i]);
        fclose(fp);
        HCUNIT_ASSERT_TRUE(tmp = transformSignCreate(fname), 
                "Failed transformSignCreate()");
        push_back_tptr(&t, &tmp);
        parseIPv4(&a, key[i]);
        addr[i] = (ManetAddr)ntohl(a.s_addr);
    }
    HCUNIT_ASSERT_TRUE(fp = fopen(fname, "w"), 
            "Failed to open %s for writing", fname);
    for(i = 0; i < sizeof(key)/sizeof(key[0]); ++i)
    {
        fprintf(fp, "%s\n", key[i]);
    }
    fclose(fp);
    tmp = transformNullCreate(); 
    push_back_tptr(&t, &tmp);
    tmp = transformNullCreate(); 
    push_back_tptr(&t, &tmp);
    tmp = transformNullCreate(); 
    push_back_tptr(&t, &tmp);
    transDuoInit();
    m = transDuoManagerCreate();
    transDuoUntransformRegister(m, untransformSignCreate(fname));
    transDuoUntransformRegister(m, untransformNullCreate());
    return;
}

HCUNIT_ASSEMBLY_TEARDOWN(transDuoUnitTestTeardown)
{
    while(t.end != t.beg)
    {
        --t.end;
        (*t.end)->transformDestroy(*t.end);
    }
    transDuoManagerDestroy(m);
    transDuoFini();
    unlink(fname);
    clean_tptr_vec(&t);
}

HCUNIT_TEST(transDuoTestCreateDestroy)
{
    struct TransDuo *td = transDuoCreate(m);
    transDuoDestroy(td);
}

HCUNIT_TEST(transDuoTestAdds)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 55, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td = transDuoCreate(m);
        transDuoAdd(td, buf, sizeof(buf), *cur);
        HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), transDuoRawLengthGet(td),
                "Expected %d bytes, got %d",
                sizeof(buf), transDuoRawLengthGet(td));
        HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, transDuoRawGet(td), sizeof(buf)), 
                "Raw data doesn't match added data");
        transDuoDestroy(td);
    }
}

HCUNIT_TEST(transDuoTestRoundTrips)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td_src = transDuoCreate(m);
        struct TransDuo *td_dst;
        char const *mbuf;
        size_t mbuflen;
        transDuoAdd(td_src, buf, sizeof(buf), *cur);
        mbuf = (char const*)transDuoBufferGet(td_src);
        mbuflen = transDuoBufferLengthGet(td_src);
        td_dst = transDuoFromBuffer(m, mbuf, mbuflen);
        HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), transDuoRawLengthGet(td_dst),
                "Expected %d bytes, got %d",
                sizeof(buf), transDuoRawLengthGet(td_dst));
        HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, transDuoRawGet(td_dst), sizeof(buf)), 
                "Raw data doesn't match added data");

        transDuoDestroy(td_dst);
        transDuoDestroy(td_src);
    }
}

HCUNIT_TEST(transDuoTestTagGets)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td = transDuoCreate(m);
        HCUNIT_ASSERT_ARE_EQUAL(-1, transDuoTagGet(td),
                "Expected tag=-1, got %d", transDuoTagGet(td));
        transDuoAdd(td, buf, sizeof(buf), *cur);
        HCUNIT_ASSERT_ARE_EQUAL((*cur)->tag, (uint32_t)transDuoTagGet(td),
                "Expected tag=%d, got %d", (*cur)->tag, transDuoTagGet(td));
        transDuoDestroy(td);
    }
}

HCUNIT_TEST(transDuoTestDataHandle)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td_src = transDuoCreate(m);
        struct TransDuo *td_dst;
        TransformDataHandle tdh;
        UntransformDataHandle udh;
        tdh = transDuoTransformDataHandleGet(td_src);
        HCUNIT_ASSERT_ARE_EQUAL(0, tdh, 
                "Unset transform expects 0 value handle, got %p", tdh);
        transDuoAdd(td_src, buf, sizeof(buf), *cur);
        tdh = transDuoTransformDataHandleGet(td_src);
        HCUNIT_ASSERT_ARE_NOT_EQUAL(0, tdh, 
                "Set transform expects non-0 value handle, got %p", tdh);
        td_dst = transDuoFromBuffer(
                m,
                transDuoBufferGet(td_src),
                transDuoBufferLengthGet(td_src));
        udh = transDuoUntransformDataHandleGet(td_dst);
        HCUNIT_ASSERT_ARE_NOT_EQUAL(0, tdh, 
                "Set untransform expects non-0 value handle, got %p", tdh);
        transDuoDestroy(td_dst);
        transDuoDestroy(td_src);
    }
}

HCUNIT_TEST(transDuoTestRawGet)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td_src = transDuoCreate(m);
        struct TransDuo *td_dst;
        void const *raw = transDuoRawGet(td_src);
        size_t rawLength = transDuoRawLengthGet(td_src);
        HCUNIT_ASSERT_FALSE(raw, 
                "Unset transform expects null raw, got %p", raw);
        HCUNIT_ASSERT_FALSE(rawLength, 
                "Unset transform expects 0 raw length, got %u", rawLength);
        transDuoAdd(td_src, buf, sizeof(buf), *cur);
        raw = transDuoRawGet(td_src);
        rawLength = transDuoRawLengthGet(td_src);
        HCUNIT_ASSERT_TRUE(raw, 
                "Set transform expects non-null raw, got %p", raw);
        HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), rawLength, 
                "Set transform expects %d raw length, got %u", sizeof(buf), rawLength);
        td_dst = transDuoFromBuffer(
                m,
                transDuoBufferGet(td_src),
                transDuoBufferLengthGet(td_src));
        raw = transDuoRawGet(td_dst);
        rawLength = transDuoRawLengthGet(td_dst);
        HCUNIT_ASSERT_TRUE(raw, 
                "Set transform expects non-null raw, got %p", raw);
        HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), rawLength, 
                "Set transform expects %d raw length, got %u", sizeof(buf), rawLength);
        transDuoDestroy(td_dst);
        transDuoDestroy(td_src);
    }
}

HCUNIT_TEST(transDuoTestTransformedGet)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td_src = transDuoCreate(m);
        struct TransDuo *td_dst;
        void const *transformed = transDuoTransformedGet(td_src);
        size_t transformedLength = transDuoTransformedLengthGet(td_src);
        HCUNIT_ASSERT_FALSE(transformed, 
                "Unset transform expects null transformed, got %p", transformed);
        HCUNIT_ASSERT_FALSE(transformedLength, 
                "Unset transform expects 0 transformed length, got %u", transformedLength);
        transDuoAdd(td_src, buf, sizeof(buf), *cur);
        transformed = transDuoTransformedGet(td_src);
        transformedLength = transDuoTransformedLengthGet(td_src);
        HCUNIT_ASSERT_TRUE(transformed, 
                "Set transform expects non-null transformed, got %p", transformed);
        HCUNIT_ASSERT_TRUE(sizeof(buf) <= transformedLength, 
                "Set transform expects transformed length >= %d, got %u", sizeof(buf), transformedLength);
        td_dst = transDuoFromBuffer(
                m,
                transDuoBufferGet(td_src),
                transDuoBufferLengthGet(td_src));
        transformed = transDuoTransformedGet(td_dst);
        transformedLength = transDuoTransformedLengthGet(td_dst);
        HCUNIT_ASSERT_TRUE(transformed, 
                "Set transform expects non-null transformed, got %p", transformed);
        HCUNIT_ASSERT_TRUE(sizeof(buf) <= transformedLength, 
                "Set transform expects transformed length >= %d, got %u", 
                sizeof(buf), transformedLength);
        transDuoDestroy(td_dst);
        transDuoDestroy(td_src);
    }
}

HCUNIT_TEST(transDuoTestBufferDup)
{
    tptr *cur;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        struct TransDuo *td = transDuoCreate(m);
        void const *obuf = transDuoBufferGet(td);
        size_t len = transDuoBufferLengthGet(td);
        void  *dupBuf = transDuoBufferDup(td);
        HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(obuf, dupBuf, len), 
                "transDuoBufferGet and transDuoBufferDup returned differing values");
        free(dupBuf);
        transDuoDestroy(td);
    }
}

HCUNIT_TEST(transDuoTestSigner)
{
    tptr *cur;
    uint8_t buf[100];
    int i = 0;
    memset(buf, 57, sizeof(buf));
    for(cur = t.beg; cur != t.end; ++cur)
    {
        if((*cur)->tag == 2) // transformSign
        {
            struct TransDuo *td_src = transDuoCreate(m);
            struct TransDuo *td_dst;
            ManetAddr check;
            HCUNIT_ASSERT_ARE_EQUAL(-1, transDuoTagGet(td_src),
                    "Expected tag=-1, got %d", transDuoTagGet(td_src));
            transDuoAdd(td_src, buf, sizeof(buf), *cur);
            td_dst = transDuoFromBuffer(
                    m,
                    transDuoBufferGet(td_src),
                    transDuoBufferLengthGet(td_src));
            check = untransformSignSigner(transDuoUntransformDataHandleGet(td_dst));
            HCUNIT_ASSERT_ARE_EQUAL(
                    addr[i], check,
                    "Expected %d.%d.%d.%d, got %d.%d.%d.%d\n",
                    IPQUAD(addr[i]), 
                    IPQUAD(check));
            ++i;
            transDuoDestroy(td_dst);
            transDuoDestroy(td_src);
        }
    }
}


HCUNIT_TEST(transDuoTestDoubleRoundTripsWithCopy)
{
    tptr *cur1;
    uint8_t buf[100];
    memset(buf, 57, sizeof(buf));
    for(cur1 = t.beg; cur1 != t.end; ++cur1)
    {
        tptr *cur2;
        for(cur2 = t.beg; cur2 != t.end; ++cur2)
        {
            // can only copy between same transforms
            if((*cur1)->tag == (*cur2)->tag)
            {
                struct TransDuo *td_src1 = transDuoCreate(m);
                struct TransDuo *td_dst1;
                struct TransDuo *td_src2 = transDuoCreate(m);
                struct TransDuo *td_dst2;
                transDuoAdd(td_src1, buf, sizeof(buf), *cur1);
                td_dst1 = transDuoFromBuffer(
                        m,
                        transDuoBufferGet(td_src1), 
                        transDuoBufferLengthGet(td_src1));
                transDuoCopy(td_src2, td_dst1, *cur2);
                // reregister the signature stuff to keep from replay
                // (otherwise its like we got the same buffer twice in the
                // "transDuoFromBuffer()" below
                transDuoDestroy(td_dst1); // reregister invalidates this
                transDuoUntransformRegister(m, untransformSignCreate(fname));
                td_dst2 = transDuoFromBuffer(
                        m,
                        transDuoBufferGet(td_src2), 
                        transDuoBufferLengthGet(td_src2));

                HCUNIT_ASSERT_ARE_EQUAL(sizeof(buf), transDuoRawLengthGet(td_dst2),
                        "Expected %d bytes, got %d",
                        sizeof(buf), transDuoRawLengthGet(td_dst2));
                HCUNIT_ASSERT_ARE_EQUAL(0, memcmp(buf, transDuoRawGet(td_dst2), sizeof(buf)), 
                        "Raw data doesn't match added data");
                transDuoDestroy(td_dst2);
                transDuoDestroy(td_src2);
                transDuoDestroy(td_src1);
            }
        }
    }
}

