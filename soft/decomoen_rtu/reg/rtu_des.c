
#include "rtu_des.h"

typedef struct {
    uint32_t esk[32];
    uint32_t dsk[32];
} rtu_des_context;

#define GET_UINT32_BE(n,b,i) \
{ \
    (n) = ( (uint32_t) (b)[(i) ] << 24 ) \
    | ( (uint32_t) (b)[(i) + 1] << 16 ) \
    | ( (uint32_t) (b)[(i) + 2] << 8 ) \
    | ( (uint32_t) (b)[(i) + 3] ); \
}

#define PUT_UINT32_BE(n,b,i) \
{ \
    (b)[(i) ] = (unsigned char) ( (n) >> 24 ); \
    (b)[(i) + 1] = (unsigned char) ( (n) >> 16 ); \
    (b)[(i) + 2] = (unsigned char) ( (n) >> 8 ); \
    (b)[(i) + 3] = (unsigned char) ( (n) ); \
}

static const uint32_t SB1[64] =
{
    0x01010400, 0x00000000, 0x00010000, 0x01010404,
    0x01010004, 0x00010404, 0x00000004, 0x00010000,
    0x00000400, 0x01010400, 0x01010404, 0x00000400,
    0x01000404, 0x01010004, 0x01000000, 0x00000004,
    0x00000404, 0x01000400, 0x01000400, 0x00010400,
    0x00010400, 0x01010000, 0x01010000, 0x01000404,
    0x00010004, 0x01000004, 0x01000004, 0x00010004,
    0x00000000, 0x00000404, 0x00010404, 0x01000000,
    0x00010000, 0x01010404, 0x00000004, 0x01010000,
    0x01010400, 0x01000000, 0x01000000, 0x00000400,
    0x01010004, 0x00010000, 0x00010400, 0x01000004,
    0x00000400, 0x00000004, 0x01000404, 0x00010404,
    0x01010404, 0x00010004, 0x01010000, 0x01000404,
    0x01000004, 0x00000404, 0x00010404, 0x01010400,
    0x00000404, 0x01000400, 0x01000400, 0x00000000,
    0x00010004, 0x00010400, 0x00000000, 0x01010004
};

static const uint32_t SB2[64] =
{
    0x80108020, 0x80008000, 0x00008000, 0x00108020,
    0x00100000, 0x00000020, 0x80100020, 0x80008020,
    0x80000020, 0x80108020, 0x80108000, 0x80000000,
    0x80008000, 0x00100000, 0x00000020, 0x80100020,
    0x00108000, 0x00100020, 0x80008020, 0x00000000,
    0x80000000, 0x00008000, 0x00108020, 0x80100000,
    0x00100020, 0x80000020, 0x00000000, 0x00108000,
    0x00008020, 0x80108000, 0x80100000, 0x00008020,
    0x00000000, 0x00108020, 0x80100020, 0x00100000,
    0x80008020, 0x80100000, 0x80108000, 0x00008000,
    0x80100000, 0x80008000, 0x00000020, 0x80108020,
    0x00108020, 0x00000020, 0x00008000, 0x80000000,
    0x00008020, 0x80108000, 0x00100000, 0x80000020,
    0x00100020, 0x80008020, 0x80000020, 0x00100020,
    0x00108000, 0x00000000, 0x80008000, 0x00008020,
    0x80000000, 0x80100020, 0x80108020, 0x00108000
};

static const uint32_t SB3[64] =
{
    0x00000208, 0x08020200, 0x00000000, 0x08020008,
    0x08000200, 0x00000000, 0x00020208, 0x08000200,
    0x00020008, 0x08000008, 0x08000008, 0x00020000,
    0x08020208, 0x00020008, 0x08020000, 0x00000208,
    0x08000000, 0x00000008, 0x08020200, 0x00000200,
    0x00020200, 0x08020000, 0x08020008, 0x00020208,
    0x08000208, 0x00020200, 0x00020000, 0x08000208,
    0x00000008, 0x08020208, 0x00000200, 0x08000000,
    0x08020200, 0x08000000, 0x00020008, 0x00000208,
    0x00020000, 0x08020200, 0x08000200, 0x00000000,
    0x00000200, 0x00020008, 0x08020208, 0x08000200,
    0x08000008, 0x00000200, 0x00000000, 0x08020008,
    0x08000208, 0x00020000, 0x08000000, 0x08020208,
    0x00000008, 0x00020208, 0x00020200, 0x08000008,
    0x08020000, 0x08000208, 0x00000208, 0x08020000,
    0x00020208, 0x00000008, 0x08020008, 0x00020200
};

static const uint32_t SB4[64] =
{
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802080, 0x00800081, 0x00800001, 0x00002001,
    0x00000000, 0x00802000, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00800080, 0x00800001,
    0x00000001, 0x00002000, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002001, 0x00002080,
    0x00800081, 0x00000001, 0x00002080, 0x00800080,
    0x00002000, 0x00802080, 0x00802081, 0x00000081,
    0x00800080, 0x00800001, 0x00802000, 0x00802081,
    0x00000081, 0x00000000, 0x00000000, 0x00802000,
    0x00002080, 0x00800080, 0x00800081, 0x00000001,
    0x00802001, 0x00002081, 0x00002081, 0x00000080,
    0x00802081, 0x00000081, 0x00000001, 0x00002000,
    0x00800001, 0x00002001, 0x00802080, 0x00800081,
    0x00002001, 0x00002080, 0x00800000, 0x00802001,
    0x00000080, 0x00800000, 0x00002000, 0x00802080
};

static const uint32_t SB5[64] =
{
    0x00000100, 0x02080100, 0x02080000, 0x42000100,
    0x00080000, 0x00000100, 0x40000000, 0x02080000,
    0x40080100, 0x00080000, 0x02000100, 0x40080100,
    0x42000100, 0x42080000, 0x00080100, 0x40000000,
    0x02000000, 0x40080000, 0x40080000, 0x00000000,
    0x40000100, 0x42080100, 0x42080100, 0x02000100,
    0x42080000, 0x40000100, 0x00000000, 0x42000000,
    0x02080100, 0x02000000, 0x42000000, 0x00080100,
    0x00080000, 0x42000100, 0x00000100, 0x02000000,
    0x40000000, 0x02080000, 0x42000100, 0x40080100,
    0x02000100, 0x40000000, 0x42080000, 0x02080100,
    0x40080100, 0x00000100, 0x02000000, 0x42080000,
    0x42080100, 0x00080100, 0x42000000, 0x42080100,
    0x02080000, 0x00000000, 0x40080000, 0x42000000,
    0x00080100, 0x02000100, 0x40000100, 0x00080000,
    0x00000000, 0x40080000, 0x02080100, 0x40000100
};

static const uint32_t SB6[64] =
{
    0x20000010, 0x20400000, 0x00004000, 0x20404010,
    0x20400000, 0x00000010, 0x20404010, 0x00400000,
    0x20004000, 0x00404010, 0x00400000, 0x20000010,
    0x00400010, 0x20004000, 0x20000000, 0x00004010,
    0x00000000, 0x00400010, 0x20004010, 0x00004000,
    0x00404000, 0x20004010, 0x00000010, 0x20400010,
    0x20400010, 0x00000000, 0x00404010, 0x20404000,
    0x00004010, 0x00404000, 0x20404000, 0x20000000,
    0x20004000, 0x00000010, 0x20400010, 0x00404000,
    0x20404010, 0x00400000, 0x00004010, 0x20000010,
    0x00400000, 0x20004000, 0x20000000, 0x00004010,
    0x20000010, 0x20404010, 0x00404000, 0x20400000,
    0x00404010, 0x20404000, 0x00000000, 0x20400010,
    0x00000010, 0x00004000, 0x20400000, 0x00404010,
    0x00004000, 0x00400010, 0x20004010, 0x00000000,
    0x20404000, 0x20000000, 0x00400010, 0x20004010
};

static const uint32_t SB7[64] =
{
    0x00200000, 0x04200002, 0x04000802, 0x00000000,
    0x00000800, 0x04000802, 0x00200802, 0x04200800,
    0x04200802, 0x00200000, 0x00000000, 0x04000002,
    0x00000002, 0x04000000, 0x04200002, 0x00000802,
    0x04000800, 0x00200802, 0x00200002, 0x04000800,
    0x04000002, 0x04200000, 0x04200800, 0x00200002,
    0x04200000, 0x00000800, 0x00000802, 0x04200802,
    0x00200800, 0x00000002, 0x04000000, 0x00200800,
    0x04000000, 0x00200800, 0x00200000, 0x04000802,
    0x04000802, 0x04200002, 0x04200002, 0x00000002,
    0x00200002, 0x04000000, 0x04000800, 0x00200000,
    0x04200800, 0x00000802, 0x00200802, 0x04200800,
    0x00000802, 0x04000002, 0x04200802, 0x04200000,
    0x00200800, 0x00000000, 0x00000002, 0x04200802,
    0x00000000, 0x00200802, 0x04200000, 0x00000800,
    0x04000002, 0x04000800, 0x00000800, 0x00200002
};

static const uint32_t SB8[64] =
{
    0x10001040, 0x00001000, 0x00040000, 0x10041040,
    0x10000000, 0x10001040, 0x00000040, 0x10000000,
    0x00040040, 0x10040000, 0x10041040, 0x00041000,
    0x10041000, 0x00041040, 0x00001000, 0x00000040,
    0x10040000, 0x10000040, 0x10001000, 0x00001040,
    0x00041000, 0x00040040, 0x10040040, 0x10041000,
    0x00001040, 0x00000000, 0x00000000, 0x10040040,
    0x10000040, 0x10001000, 0x00041040, 0x00040000,
    0x00041040, 0x00040000, 0x10041000, 0x00001000,
    0x00000040, 0x10040040, 0x00001000, 0x00041040,
    0x10001000, 0x00000040, 0x10000040, 0x10040000,
    0x10040040, 0x10000000, 0x00040000, 0x10001040,
    0x00000000, 0x10041040, 0x00040040, 0x10000040,
    0x10040000, 0x10001000, 0x10001040, 0x00000000,
    0x10041040, 0x00041000, 0x00041000, 0x00001040,
    0x00001040, 0x00040040, 0x10000000, 0x10041000
};


static const uint32_t LHs[16] =
{
    0x00000000, 0x00000001, 0x00000100, 0x00000101,
    0x00010000, 0x00010001, 0x00010100, 0x00010101,
    0x01000000, 0x01000001, 0x01000100, 0x01000101,
    0x01010000, 0x01010001, 0x01010100, 0x01010101
};

static const uint32_t RHs[16] =
{
    0x00000000, 0x01000000, 0x00010000, 0x01010000,
    0x00000100, 0x01000100, 0x00010100, 0x01010100,
    0x00000001, 0x01000001, 0x00010001, 0x01010001,
    0x00000101, 0x01000101, 0x00010101, 0x01010101,
};


#define DES_IP(X,Y) \
{ \
    T = ((X >> 4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T << 4); \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16); \
    T = ((Y >> 2) ^ X) & 0x33333333; X ^= T; Y ^= (T << 2); \
    T = ((Y >> 8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T << 8); \
    Y = ((Y << 1) | (Y >> 31)) & 0xFFFFFFFF; \
    T = (X ^ Y) & 0xAAAAAAAA; Y ^= T; X ^= T; \
    X = ((X << 1) | (X >> 31)) & 0xFFFFFFFF; \
}


#define DES_FP(X,Y) \
{ \
    X = ((X << 31) | (X >> 1)) & 0xFFFFFFFF; \
    T = (X ^ Y) & 0xAAAAAAAA; X ^= T; Y ^= T; \
    Y = ((Y << 31) | (Y >> 1)) & 0xFFFFFFFF; \
    T = ((Y >> 8) ^ X) & 0x00FF00FF; X ^= T; Y ^= (T << 8); \
    T = ((Y >> 2) ^ X) & 0x33333333; X ^= T; Y ^= (T << 2); \
    T = ((X >> 16) ^ Y) & 0x0000FFFF; Y ^= T; X ^= (T << 16); \
    T = ((X >> 4) ^ Y) & 0x0F0F0F0F; Y ^= T; X ^= (T << 4); \
}


#define DES_ROUND(X,Y) \
{ \
    T = *SK++ ^ X; \
    Y ^= SB8[ (T ) & 0x3F ] ^ \
    SB6[ (T >> 8) & 0x3F ] ^ \
    SB4[ (T >> 16) & 0x3F ] ^ \
    SB2[ (T >> 24) & 0x3F ]; \
    \
    T = *SK++ ^ ((X << 28) | (X >> 4)); \
    Y ^= SB7[ (T ) & 0x3F ] ^ \
    SB5[ (T >> 8) & 0x3F ] ^ \
    SB3[ (T >> 16) & 0x3F ] ^ \
    SB1[ (T >> 24) & 0x3F ]; \
}

static void _des_main_ks( uint32_t SK[32], unsigned char key[8] )
{
	int i;
	uint32_t X, Y, T;

	GET_UINT32_BE( X, key, 0 );
	GET_UINT32_BE( Y, key, 4 );


	T = ((Y >> 4) ^ X) & 0x0F0F0F0F; X ^= T; Y ^= (T << 4);
	T = ((Y ) ^ X) & 0x10101010; X ^= T; Y ^= (T );

	X = (LHs[ (X ) & 0xF] << 3) | (LHs[ (X >> 8) & 0xF ] << 2)
	| (LHs[ (X >> 16) & 0xF] << 1) | (LHs[ (X >> 24) & 0xF ] )
	| (LHs[ (X >> 5) & 0xF] << 7) | (LHs[ (X >> 13) & 0xF ] << 6)
	| (LHs[ (X >> 21) & 0xF] << 5) | (LHs[ (X >> 29) & 0xF ] << 4);

	Y = (RHs[ (Y >> 1) & 0xF] << 3) | (RHs[ (Y >> 9) & 0xF ] << 2)
	| (RHs[ (Y >> 17) & 0xF] << 1) | (RHs[ (Y >> 25) & 0xF ] )
	| (RHs[ (Y >> 4) & 0xF] << 7) | (RHs[ (Y >> 12) & 0xF ] << 6)
	| (RHs[ (Y >> 20) & 0xF] << 5) | (RHs[ (Y >> 28) & 0xF ] << 4);

	X &= 0x0FFFFFFF;
	Y &= 0x0FFFFFFF;


	for( i = 0; i < 16; i++ )
	{
		if( i < 2 || i == 8 || i == 15 )
		{
		X = ((X << 1) | (X >> 27)) & 0x0FFFFFFF;
		Y = ((Y << 1) | (Y >> 27)) & 0x0FFFFFFF;
		}
		else
		{
		X = ((X << 2) | (X >> 26)) & 0x0FFFFFFF;
		Y = ((Y << 2) | (Y >> 26)) & 0x0FFFFFFF;
		}

		*SK++ = ((X << 4) & 0x24000000) | ((X << 28) & 0x10000000)
		| ((X << 14) & 0x08000000) | ((X << 18) & 0x02080000)
		| ((X << 6) & 0x01000000) | ((X << 9) & 0x00200000)
		| ((X >> 1) & 0x00100000) | ((X << 10) & 0x00040000)
		| ((X << 2) & 0x00020000) | ((X >> 10) & 0x00010000)
		| ((Y >> 13) & 0x00002000) | ((Y >> 4) & 0x00001000)
		| ((Y << 6) & 0x00000800) | ((Y >> 1) & 0x00000400)
		| ((Y >> 14) & 0x00000200) | ((Y ) & 0x00000100)
		| ((Y >> 5) & 0x00000020) | ((Y >> 10) & 0x00000010)
		| ((Y >> 3) & 0x00000008) | ((Y >> 18) & 0x00000004)
		| ((Y >> 26) & 0x00000002) | ((Y >> 24) & 0x00000001);

		*SK++ = ((X << 15) & 0x20000000) | ((X << 17) & 0x10000000)
		| ((X << 10) & 0x08000000) | ((X << 22) & 0x04000000)
		| ((X >> 2) & 0x02000000) | ((X << 1) & 0x01000000)
		| ((X << 16) & 0x00200000) | ((X << 11) & 0x00100000)
		| ((X << 3) & 0x00080000) | ((X >> 6) & 0x00040000)
		| ((X << 15) & 0x00020000) | ((X >> 4) & 0x00010000)
		| ((Y >> 2) & 0x00002000) | ((Y << 8) & 0x00001000)
		| ((Y >> 14) & 0x00000808) | ((Y >> 9) & 0x00000400)
		| ((Y ) & 0x00000200) | ((Y << 7) & 0x00000100)
		| ((Y >> 7) & 0x00000020) | ((Y >> 3) & 0x00000011)
		| ((Y << 2) & 0x00000004) | ((Y >> 21) & 0x00000002);
	}
}

static void _des_set_key( rtu_des_context *ctx, unsigned char key[8] )
{
	int i;
	_des_main_ks( ctx->esk, key );

	for( i = 0; i < 32; i += 2 )
	{
		ctx->dsk[i ] = ctx->esk[30 - i];
		ctx->dsk[i + 1] = ctx->esk[31 - i];
	}
}

static void _des_crypt( uint32_t SK[32],unsigned char input[8],unsigned char output[8] )
{
	uint32_t X, Y, T;

	GET_UINT32_BE( X, input, 0 );
	GET_UINT32_BE( Y, input, 4 );

	DES_IP( X, Y );

	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );
	DES_ROUND( Y, X ); DES_ROUND( X, Y );

	DES_FP( Y, X );

	PUT_UINT32_BE( Y, output, 0 );
	PUT_UINT32_BE( X, output, 4 );
}


static void _des_encrypt( rtu_des_context *ctx,unsigned char input[8],unsigned char output[8] )
{
	_des_crypt( ctx->esk, input, output );
}


static void _des_decrypt( rtu_des_context *ctx,unsigned char input[8],unsigned char output[8] )
{
	_des_crypt( ctx->dsk, input, output );
}

int rtu_des_encrypt(unsigned char *out,unsigned char *in, long datalen,unsigned char key[8])
{
	rtu_des_context ctx ;
	_des_set_key(&ctx, key);
	while(datalen > 0)
	{
        _des_encrypt(&ctx, in, out);

        in  += 8;
        out += 8;
        datalen -= 8;
	}
	return datalen;
}


int rtu_des_decrypt(unsigned char *out,unsigned char *in, long datalen,unsigned char key[8])
{
	rtu_des_context ctx ;
	_des_set_key(&ctx, key);
        
	while(datalen > 0)
	{
        _des_decrypt(&ctx, in, out);

        in  += 8;
        out += 8;
        datalen -= 8;
	}
	return datalen;
}

