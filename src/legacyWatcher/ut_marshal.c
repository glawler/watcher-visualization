//
// marshal.h unit tests.
//
#include "marshal.h"

#include "../hcunit/hcunit.h"
#include <stdlib.h>
#include <limits.h>

HCUNIT_ASSEMBLY_SETUP(marshalUnitTestSetup)
{
	/* called once before anything else */
}

HCUNIT_ASSEMBLY_TEARDOWN(marshalUnitTestTeardown)
{
	/* called once after everything else */
}

HCUNIT_TEST_SETUP(marshalTestSetup)
{
	/* called before each test */
}

HCUNIT_TEST_TEARDOWN(marshalTestTeardown)
{
	/* called after each test */
}

static unsigned long someulongs[] = {
	0,
	1,
	2,
	10,
	100,
	1000,
	10000,
	100000,
	1000000,
	0x0badbeefUL,
	(unsigned long)LONG_MAX,
	(unsigned long)LONG_MAX+1,
	0xfabbfabb,
	ULONG_MAX-1,
	ULONG_MAX
};
#define NULONGS (sizeof(someulongs)/sizeof(someulongs[0]))

static long somelongs[] = {
	0,
	1, -1,
	2, -2,
	10, -10,
	100, -100,
	1000, -1000,
	10000, -10000,
	100000, -100000,
	1000000, -1000000,
	LONG_MIN,
	LONG_MIN+1,
	LONG_MAX,
	LONG_MAX-1,
};
#define NLONGS (sizeof(somelongs)/sizeof(somelongs[0]))

HCUNIT_TEST(marshalTestLong)
{
	size_t i, j;
	unsigned char buf[NLONGS*4];
	unsigned char *p;
	for (i = 0, p = buf; i < NLONGS; i++)
	{
		HCUNIT_ASSERT_TRUE(p == buf + i*4,"ah: p-buf=%u, i=%u", p-buf, i);
		MARSHALLONG(p,somelongs[i]);
	}
	HCUNIT_ASSERT_TRUE(p == buf + NLONGS*4, "p-buf=%u, NLONGS*4=%u", p-buf, NLONGS*4);
	for (j = 0; j < 2; j++)
	{
		for (i = 0, p = buf; i < NLONGS; i++)
		{
			long l;
			UNMARSHALLONG(p,l);
			HCUNIT_ASSERT_TRUE(l == somelongs[i], "z");
		}
		HCUNIT_ASSERT_TRUE(p == buf + NLONGS*4, "z");
	}
}



HCUNIT_TEST(marshalTestULong)
{
	size_t i, j;
	unsigned char buf[NULONGS*4];
	unsigned char *p;
	for (i = 0, p = buf; i < NULONGS; i++)
	{
		HCUNIT_ASSERT_TRUE(p == buf + i*4,"ah: p-buf=%u, i=%u", p-buf, i);
		MARSHALLONG(p,someulongs[i]);
	}
	HCUNIT_ASSERT_TRUE(p == buf + NULONGS*4, "p-buf=%u, NULONGS*4=%u", p-buf, NULONGS*4);
	for (j = 0; j < 2; j++)
	{
		for (i = 0, p = buf; i < NULONGS; i++)
		{
			unsigned long l;
			UNMARSHALLONG(p,l);
			HCUNIT_ASSERT_TRUE(l == someulongs[i], "z");
		}
		HCUNIT_ASSERT_TRUE(p == buf + NULONGS*4, "z");
	}
}

/* rand() returns [0..RAND_MAX], which is 2^31 on x86 linux.
 * these defines assume RAND_MAX >= LONG_MAX (see assert below). */
#define RAND_ULONGLONG() ((((unsigned long long) rand()) << 33)  \
			  | (((unsigned long long) rand()) << 2) \
			  | (((unsigned long long) rand())))
#define RAND_LONGLONG() ((long long)RAND_ULONGLONG())
#define RAND_ULONG() ((((unsigned long) rand()) << 1) \
		      | (((unsigned long) rand())))
#define RAND_LONG() ((long)RAND_ULONG())
#define RAND_UINT() ((((unsigned int) rand()) << 1) \
		     | (((unsigned int) rand())))
#define RAND_INT()  ((int)RAND_UINT())
#define RAND_USHORT() ((unsigned short) rand())
#define RAND_SHORT()  ((short)RAND_USHORT())
#define RAND_UCHAR() ((unsigned char)rand())

//#undef HCUNIT_ASSERT_TRUE
//#define HCUNIT_ASSERT_TRUE(a,m) do {} while (0);

HCUNIT_TEST(marshalTestRand)
{
	static unsigned char bigbuf[50000000]; /* 50 Mbyte */
	unsigned char *p;
	size_t i;
	int unmarshal;

	/* rand() returns [0..RAND_MAX], which is 2^31 on x86 linux */
	HCUNIT_ASSERT_TRUE(1, "checking rand range");
	HCUNIT_ASSERT_TRUE(RAND_MAX >= LONG_MAX, "checking rand() range");

	for (unmarshal = 0; unmarshal < 2; unmarshal++)
	{
		printf("%s...\n", unmarshal ? "unmarshaling" : "marshaling");
		srand(0);
		for (i = 0, p = bigbuf; p < bigbuf + sizeof(bigbuf); i++)
		{
			switch (rand() % 12)
			{
			case 0:
			{
				unsigned long long w, v = RAND_ULONGLONG();
				if (unmarshal)
				{
					UNMARSHALLONGLONG(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "unsigned long long");
					/* printf("unmarshaled unsigned long long: %llu\n", w); */
				}
				else
				{
					MARSHALLONGLONG(p, v);
				}
				break;
			}
			case 1:
				/* signed longlong */
			{
				long long w, v = RAND_LONGLONG();
				if (unmarshal)
				{
					UNMARSHALLONGLONG(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "long long");
					/* printf("unmarshaled long long: %lld\n", w); */
				}
				else
				{
					MARSHALLONGLONG(p, v);
				}
				break;
			}
			case 2:
				/* unsigned long */
			{
				unsigned long w, v = RAND_ULONG();
				if (unmarshal)
				{
					UNMARSHALLONG(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "unsigned long");
					/* printf("unmarshaled unsigned long: %lu\n", w); */
				}
				else
				{
					MARSHALLONG(p, v);
				}
				break;
			}
			case 3:
				/* signed long */
			{
				long w, v = RAND_LONG();
				if (unmarshal)
				{
					UNMARSHALLONG(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "long");
					/* printf("unmarshaled long: %ld\n", w); */
				}
				else
				{
					MARSHALLONG(p, v);
				}
				break;
			}
			case 4:
				/* unsigned short */
			{
				unsigned short w, v = RAND_USHORT();
				if (unmarshal)
				{
					UNMARSHALSHORT(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "unsigned short");
					/* printf("unmarshaled unsigned short: %u\n", w); */
				}
				else
				{
					MARSHALSHORT(p, v);
				}
				break;
			}
			case 5:
				/* signed short */
			{
				short w, v = RAND_SHORT();
				if (unmarshal)
				{
					UNMARSHALSHORT(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "short");
					/* printf("unmarshaled short: %d\n", w); */
				}
				else
				{
					MARSHALSHORT(p, v);
				}
				break;
			}
			case 6:
				/* byte */
			{
				unsigned char w, v = RAND_UCHAR();
				if (unmarshal)
				{
					UNMARSHALBYTE(p, w);
					HCUNIT_ASSERT_TRUE(w == v, "unsigned char");
					/* printf("unmarshaled unsigned char: %u\n", w); */
				}
				else
				{
					MARSHALBYTE(p, v);
				}
				break;
			}
			case 7:
				/* short string */
				break;
			case 8:
				/* medium string */
				break;
			case 9:
				/* buffer */
				break;
			case 10:
				/* network long */
				break;
			case 11:
				/* network short */
				break;
			}
		}
		printf("did %d values\n", i);
	}
}
