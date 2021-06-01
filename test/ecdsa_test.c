/* This program is released under the Common Public License V1.0
 *
 * You should have received a copy of Common Public License V1.0 along with
 * with this program.
 */

/* Copyright IBM Corp. 2017 */

#include <fcntl.h>
#include <memory.h>
#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "ica_api.h"
#include <sys/time.h>
#include "testcase.h"
#include <openssl/obj_mac.h>
#include <sys/time.h>


#define NUM_HW_SW_TESTS		2
#define NUM_ECDSA_TESTS		(sizeof(ecdsa_kats)/sizeof(ecdsa_kat_t))
#define NUM_HASH_LENGTHS	(sizeof(hash_length)/sizeof(int))

#define MAX_ECC_PRIV_SIZE	66 /* 521 bits */
#define MAX_ECDSA_SIG_SIZE	132
#define MAX_HASH_LENGTH		64

typedef struct {
    unsigned int nid;
    unsigned char d[MAX_ECC_PRIV_SIZE];
    unsigned char x[MAX_ECC_PRIV_SIZE];
    unsigned char y[MAX_ECC_PRIV_SIZE];
} ecdsa_kat_t;

static unsigned char hash[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
};

static unsigned int hash_length[] = {
	20, 28, 32, 48, 64,
};

/**
 * From: http://csrc.nist.gov/groups/STM/cavp/digital-signatures.html#test-vectors
 */
static ecdsa_kat_t ecdsa_kats[] = {
	/* Keys from RFC 5114 */
	{NID_X9_62_prime192v1,
	 {0x32,0x3F,0xA3,0x16,0x9D,0x8E,0x9C,0x65,0x93,0xF5,0x94,0x76,0xBC,0x14,0x20,0x00,
	  0xAB,0x5B,0xE0,0xE2,0x49,0xC4,0x34,0x26,},
	 {0xCD,0x46,0x48,0x9E,0xCF,0xD6,0xC1,0x05,0xE7,0xB3,0xD3,0x25,0x66,0xE2,0xB1,0x22,
	  0xE2,0x49,0xAB,0xAA,0xDD,0x87,0x06,0x12,},
	 {0x68,0x88,0x7B,0x48,0x77,0xDF,0x51,0xDD,0x4D,0xC3,0xD6,0xFD,0x11,0xF0,0xA2,0x6F,
	  0x8F,0xD3,0x84,0x43,0x17,0x91,0x6E,0x9A,},
	},
	{NID_secp224r1,
	 {0xB5,0x58,0xEB,0x6C,0x28,0x8D,0xA7,0x07,0xBB,0xB4,0xF8,0xFB,0xAE,0x2A,0xB9,0xE9,
	  0xCB,0x62,0xE3,0xBC,0x5C,0x75,0x73,0xE2,0x2E,0x26,0xD3,0x7F,},
	 {0x49,0xDF,0xEF,0x30,0x9F,0x81,0x48,0x8C,0x30,0x4C,0xFF,0x5A,0xB3,0xEE,0x5A,0x21,
	  0x54,0x36,0x7D,0xC7,0x83,0x31,0x50,0xE0,0xA5,0x1F,0x3E,0xEB,},
	 {0x4F,0x2B,0x5E,0xE4,0x57,0x62,0xC4,0xF6,0x54,0xC1,0xA0,0xC6,0x7F,0x54,0xCF,0x88,
	  0xB0,0x16,0xB5,0x1B,0xCE,0x3D,0x7C,0x22,0x8D,0x57,0xAD,0xB4,},
	 },
	{NID_X9_62_prime256v1,
	 {0x81,0x42,0x64,0x14,0x5F,0x2F,0x56,0xF2,0xE9,0x6A,0x8E,0x33,0x7A,0x12,0x84,0x99,
	  0x3F,0xAF,0x43,0x2A,0x5A,0xBC,0xE5,0x9E,0x86,0x7B,0x72,0x91,0xD5,0x07,0xA3,0xAF,},
	 {0x2A,0xF5,0x02,0xF3,0xBE,0x89,0x52,0xF2,0xC9,0xB5,0xA8,0xD4,0x16,0x0D,0x09,0xE9,
	  0x71,0x65,0xBE,0x50,0xBC,0x42,0xAE,0x4A,0x5E,0x8D,0x3B,0x4B,0xA8,0x3A,0xEB,0x15,},
	 {0xEB,0x0F,0xAF,0x4C,0xA9,0x86,0xC4,0xD3,0x86,0x81,0xA0,0xF9,0x87,0x2D,0x79,0xD5,
	  0x67,0x95,0xBD,0x4B,0xFF,0x6E,0x6D,0xE3,0xC0,0xF5,0x01,0x5E,0xCE,0x5E,0xFD,0x85,},
	},
	{NID_secp384r1,
	 {0xD2,0x73,0x35,0xEA,0x71,0x66,0x4A,0xF2,0x44,0xDD,0x14,0xE9,0xFD,0x12,0x60,0x71,
	  0x5D,0xFD,0x8A,0x79,0x65,0x57,0x1C,0x48,0xD7,0x09,0xEE,0x7A,0x79,0x62,0xA1,0x56,
	  0xD7,0x06,0xA9,0x0C,0xBC,0xB5,0xDF,0x29,0x86,0xF0,0x5F,0xEA,0xDB,0x93,0x76,0xF1,},
	 {0x79,0x31,0x48,0xF1,0x78,0x76,0x34,0xD5,0xDA,0x4C,0x6D,0x90,0x74,0x41,0x7D,0x05,
	  0xE0,0x57,0xAB,0x62,0xF8,0x20,0x54,0xD1,0x0E,0xE6,0xB0,0x40,0x3D,0x62,0x79,0x54,
	  0x7E,0x6A,0x8E,0xA9,0xD1,0xFD,0x77,0x42,0x7D,0x01,0x6F,0xE2,0x7A,0x8B,0x8C,0x66,},
	 {0xC6,0xC4,0x12,0x94,0x33,0x1D,0x23,0xE6,0xF4,0x80,0xF4,0xFB,0x4C,0xD4,0x05,0x04,
	  0xC9,0x47,0x39,0x2E,0x94,0xF4,0xC3,0xF0,0x6B,0x8F,0x39,0x8B,0xB2,0x9E,0x42,0x36,
	  0x8F,0x7A,0x68,0x59,0x23,0xDE,0x3B,0x67,0xBA,0xCE,0xD2,0x14,0xA1,0xA1,0xD1,0x28,},
	},
	{NID_secp521r1,
	 {0x01,0x13,0xF8,0x2D,0xA8,0x25,0x73,0x5E,0x3D,0x97,0x27,0x66,0x83,0xB2,0xB7,0x42,
	  0x77,0xBA,0xD2,0x73,0x35,0xEA,0x71,0x66,0x4A,0xF2,0x43,0x0C,0xC4,0xF3,0x34,0x59,
	  0xB9,0x66,0x9E,0xE7,0x8B,0x3F,0xFB,0x9B,0x86,0x83,0x01,0x5D,0x34,0x4D,0xCB,0xFE,
	  0xF6,0xFB,0x9A,0xF4,0xC6,0xC4,0x70,0xBE,0x25,0x45,0x16,0xCD,0x3C,0x1A,0x1F,0xB4,
	  0x73,0x62,},
	 {0x01,0xEB,0xB3,0x4D,0xD7,0x57,0x21,0xAB,0xF8,0xAD,0xC9,0xDB,0xED,0x17,0x88,0x9C,
	  0xBB,0x97,0x65,0xD9,0x0A,0x7C,0x60,0xF2,0xCE,0xF0,0x07,0xBB,0x0F,0x2B,0x26,0xE1,
	  0x48,0x81,0xFD,0x44,0x42,0xE6,0x89,0xD6,0x1C,0xB2,0xDD,0x04,0x6E,0xE3,0x0E,0x3F,
	  0xFD,0x20,0xF9,0xA4,0x5B,0xBD,0xF6,0x41,0x3D,0x58,0x3A,0x2D,0xBF,0x59,0x92,0x4F,
	  0xD3,0x5C,},
	 {0x00,0xF6,0xB6,0x32,0xD1,0x94,0xC0,0x38,0x8E,0x22,0xD8,0x43,0x7E,0x55,0x8C,0x55,
	  0x2A,0xE1,0x95,0xAD,0xFD,0x15,0x3F,0x92,0xD7,0x49,0x08,0x35,0x1B,0x2F,0x8C,0x4E,
	  0xDA,0x94,0xED,0xB0,0x91,0x6D,0x1B,0x53,0xC0,0x20,0xB5,0xEE,0xCA,0xED,0x1A,0x5F,
	  0xC3,0x8A,0x23,0x3E,0x48,0x30,0x58,0x7B,0xB2,0xEE,0x34,0x89,0xB3,0xB4,0x2A,0x5A,
	  0x86,0xA4,},
	},
	/* Keys from RFC 5903 */
	{NID_X9_62_prime256v1,
	 {0xC8,0x8F,0x01,0xF5,0x10,0xD9,0xAC,0x3F,0x70,0xA2,0x92,0xDA,0xA2,0x31,0x6D,0xE5,
	  0x44,0xE9,0xAA,0xB8,0xAF,0xE8,0x40,0x49,0xC6,0x2A,0x9C,0x57,0x86,0x2D,0x14,0x33,},
	 {0xDA,0xD0,0xB6,0x53,0x94,0x22,0x1C,0xF9,0xB0,0x51,0xE1,0xFE,0xCA,0x57,0x87,0xD0,
	  0x98,0xDF,0xE6,0x37,0xFC,0x90,0xB9,0xEF,0x94,0x5D,0x0C,0x37,0x72,0x58,0x11,0x80,},
	 {0x52,0x71,0xA0,0x46,0x1C,0xDB,0x82,0x52,0xD6,0x1F,0x1C,0x45,0x6F,0xA3,0xE5,0x9A,
	  0xB1,0xF4,0x5B,0x33,0xAC,0xCF,0x5F,0x58,0x38,0x9E,0x05,0x77,0xB8,0x99,0x0B,0xB3,},
	},
	{NID_secp384r1,
	 {0x09,0x9F,0x3C,0x70,0x34,0xD4,0xA2,0xC6,0x99,0x88,0x4D,0x73,0xA3,0x75,0xA6,0x7F,
	  0x76,0x24,0xEF,0x7C,0x6B,0x3C,0x0F,0x16,0x06,0x47,0xB6,0x74,0x14,0xDC,0xE6,0x55,
	  0xE3,0x5B,0x53,0x80,0x41,0xE6,0x49,0xEE,0x3F,0xAE,0xF8,0x96,0x78,0x3A,0xB1,0x94,},
	 {0x66,0x78,0x42,0xD7,0xD1,0x80,0xAC,0x2C,0xDE,0x6F,0x74,0xF3,0x75,0x51,0xF5,0x57,
	  0x55,0xC7,0x64,0x5C,0x20,0xEF,0x73,0xE3,0x16,0x34,0xFE,0x72,0xB4,0xC5,0x5E,0xE6,
	  0xDE,0x3A,0xC8,0x08,0xAC,0xB4,0xBD,0xB4,0xC8,0x87,0x32,0xAE,0xE9,0x5F,0x41,0xAA,},
	 {0x94,0x82,0xED,0x1F,0xC0,0xEE,0xB9,0xCA,0xFC,0x49,0x84,0x62,0x5C,0xCF,0xC2,0x3F,
	  0x65,0x03,0x21,0x49,0xE0,0xE1,0x44,0xAD,0xA0,0x24,0x18,0x15,0x35,0xA0,0xF3,0x8E,
	  0xEB,0x9F,0xCF,0xF3,0xC2,0xC9,0x47,0xDA,0xE6,0x9B,0x4C,0x63,0x45,0x73,0xA8,0x1C,},
	},
	{NID_secp521r1,
	 {0x00,0x37,0xAD,0xE9,0x31,0x9A,0x89,0xF4,0xDA,0xBD,0xB3,0xEF,0x41,0x1A,0xAC,0xCC,
	  0xA5,0x12,0x3C,0x61,0xAC,0xAB,0x57,0xB5,0x39,0x3D,0xCE,0x47,0x60,0x81,0x72,0xA0,
	  0x95,0xAA,0x85,0xA3,0x0F,0xE1,0xC2,0x95,0x2C,0x67,0x71,0xD9,0x37,0xBA,0x97,0x77,
	  0xF5,0x95,0x7B,0x26,0x39,0xBA,0xB0,0x72,0x46,0x2F,0x68,0xC2,0x7A,0x57,0x38,0x2D,
	  0x4A,0x52,},
	 {0x00,0x15,0x41,0x7E,0x84,0xDB,0xF2,0x8C,0x0A,0xD3,0xC2,0x78,0x71,0x33,0x49,0xDC,
	  0x7D,0xF1,0x53,0xC8,0x97,0xA1,0x89,0x1B,0xD9,0x8B,0xAB,0x43,0x57,0xC9,0xEC,0xBE,
	  0xE1,0xE3,0xBF,0x42,0xE0,0x0B,0x8E,0x38,0x0A,0xEA,0xE5,0x7C,0x2D,0x10,0x75,0x64,
	  0x94,0x18,0x85,0x94,0x2A,0xF5,0xA7,0xF4,0x60,0x17,0x23,0xC4,0x19,0x5D,0x17,0x6C,
	  0xED,0x3E,},
	 {0x01,0x7C,0xAE,0x20,0xB6,0x64,0x1D,0x2E,0xEB,0x69,0x57,0x86,0xD8,0xC9,0x46,0x14,
	  0x62,0x39,0xD0,0x99,0xE1,0x8E,0x1D,0x5A,0x51,0x4C,0x73,0x9D,0x7C,0xB4,0xA1,0x0A,
	  0xD8,0xA7,0x88,0x01,0x5A,0xC4,0x05,0xD7,0x79,0x9D,0xC7,0x5E,0x7B,0x7D,0x5B,0x6C,
	  0xF2,0x26,0x1A,0x6A,0x7F,0x15,0x07,0x43,0x8B,0xF0,0x1B,0xEB,0x6C,0xA3,0x92,0x6F,
	  0x95,0x82,},
	},
#if OPENSSL_VERSION_NUMBER >= 0x010002000
	/* Keys from RFC 7027 */
	{NID_brainpoolP256r1,
	 {0x81,0xDB,0x1E,0xE1,0x00,0x15,0x0F,0xF2,0xEA,0x33,0x8D,0x70,0x82,0x71,0xBE,0x38,
	  0x30,0x0C,0xB5,0x42,0x41,0xD7,0x99,0x50,0xF7,0x7B,0x06,0x30,0x39,0x80,0x4F,0x1D,},
	 {0x44,0x10,0x6E,0x91,0x3F,0x92,0xBC,0x02,0xA1,0x70,0x5D,0x99,0x53,0xA8,0x41,0x4D,
	  0xB9,0x5E,0x1A,0xAA,0x49,0xE8,0x1D,0x9E,0x85,0xF9,0x29,0xA8,0xE3,0x10,0x0B,0xE5,},
	 {0x8A,0xB4,0x84,0x6F,0x11,0xCA,0xCC,0xB7,0x3C,0xE4,0x9C,0xBD,0xD1,0x20,0xF5,0xA9,
	  0x00,0xA6,0x9F,0xD3,0x2C,0x27,0x22,0x23,0xF7,0x89,0xEF,0x10,0xEB,0x08,0x9B,0xDC,},
	},
	{NID_brainpoolP384r1,
	 {0x1E,0x20,0xF5,0xE0,0x48,0xA5,0x88,0x6F,0x1F,0x15,0x7C,0x74,0xE9,0x1B,0xDE,0x2B,
	  0x98,0xC8,0xB5,0x2D,0x58,0xE5,0x00,0x3D,0x57,0x05,0x3F,0xC4,0xB0,0xBD,0x65,0xD6,
	  0xF1,0x5E,0xB5,0xD1,0xEE,0x16,0x10,0xDF,0x87,0x07,0x95,0x14,0x36,0x27,0xD0,0x42,},
	 {0x68,0xB6,0x65,0xDD,0x91,0xC1,0x95,0x80,0x06,0x50,0xCD,0xD3,0x63,0xC6,0x25,0xF4,
	  0xE7,0x42,0xE8,0x13,0x46,0x67,0xB7,0x67,0xB1,0xB4,0x76,0x79,0x35,0x88,0xF8,0x85,
	  0xAB,0x69,0x8C,0x85,0x2D,0x4A,0x6E,0x77,0xA2,0x52,0xD6,0x38,0x0F,0xCA,0xF0,0x68,},
	 {0x55,0xBC,0x91,0xA3,0x9C,0x9E,0xC0,0x1D,0xEE,0x36,0x01,0x7B,0x7D,0x67,0x3A,0x93,
	  0x12,0x36,0xD2,0xF1,0xF5,0xC8,0x39,0x42,0xD0,0x49,0xE3,0xFA,0x20,0x60,0x74,0x93,
	  0xE0,0xD0,0x38,0xFF,0x2F,0xD3,0x0C,0x2A,0xB6,0x7D,0x15,0xC8,0x5F,0x7F,0xAA,0x59,},
	},
	{NID_brainpoolP512r1,
	 {0x16,0x30,0x2F,0xF0,0xDB,0xBB,0x5A,0x8D,0x73,0x3D,0xAB,0x71,0x41,0xC1,0xB4,0x5A,
	  0xCB,0xC8,0x71,0x59,0x39,0x67,0x7F,0x6A,0x56,0x85,0x0A,0x38,0xBD,0x87,0xBD,0x59,
	  0xB0,0x9E,0x80,0x27,0x96,0x09,0xFF,0x33,0x3E,0xB9,0xD4,0xC0,0x61,0x23,0x1F,0xB2,
	  0x6F,0x92,0xEE,0xB0,0x49,0x82,0xA5,0xF1,0xD1,0x76,0x4C,0xAD,0x57,0x66,0x54,0x22,},
	 {0x0A,0x42,0x05,0x17,0xE4,0x06,0xAA,0xC0,0xAC,0xDC,0xE9,0x0F,0xCD,0x71,0x48,0x77,
	  0x18,0xD3,0xB9,0x53,0xEF,0xD7,0xFB,0xEC,0x5F,0x7F,0x27,0xE2,0x8C,0x61,0x49,0x99,
	  0x93,0x97,0xE9,0x1E,0x02,0x9E,0x06,0x45,0x7D,0xB2,0xD3,0xE6,0x40,0x66,0x8B,0x39,
	  0x2C,0x2A,0x7E,0x73,0x7A,0x7F,0x0B,0xF0,0x44,0x36,0xD1,0x16,0x40,0xFD,0x09,0xFD,},
	 {0x72,0xE6,0x88,0x2E,0x8D,0xB2,0x8A,0xAD,0x36,0x23,0x7C,0xD2,0x5D,0x58,0x0D,0xB2,
	  0x37,0x83,0x96,0x1C,0x8D,0xC5,0x2D,0xFA,0x2E,0xC1,0x38,0xAD,0x47,0x2A,0x0F,0xCE,
	  0xF3,0x88,0x7C,0xF6,0x2B,0x62,0x3B,0x2A,0x87,0xDE,0x5C,0x58,0x83,0x01,0xEA,0x3E,
	  0x5F,0xC2,0x69,0xB3,0x73,0xB6,0x07,0x24,0xF5,0xE8,0x2A,0x6A,0xD1,0x47,0xFD,0xE7,},
	},
#endif
};


int main(int argc, char **argv)
{
	ica_adapter_handle_t adapter_handle;
	unsigned int i, j, k, rc;
	unsigned int errors=0, test_failed=0;
	unsigned char signature[MAX_ECDSA_SIG_SIZE];
	unsigned int privlen = 0;
	ICA_EC_KEY *eckey;
	char *icapath;

	set_verbosity(argc, argv);

	if (!ecc_available()) {
		printf("Skipping ECDSA test, because the required HW"
		       " is not available on this machine.\n");
		return TEST_SKIP;
	}

	rc = ica_open_adapter(&adapter_handle);
	if (rc != 0) {
		V_(printf("ica_open_adapter failed and returned %d (0x%x).\n", rc, rc));
	}

	/* set ICAPATH default value */
	icapath = getenv("ICAPATH");
	if ((icapath == NULL) || (atoi(icapath) == 0)) {
		icapath = "1";
		setenv("ICAPATH", icapath, 1);
	}

	/* Iterate over curves */
	for (i = 0; i < NUM_ECDSA_TESTS; i++) {
		setenv("ICAPATH", icapath, 1);

		V_(printf("Testing curve %d \n", ecdsa_kats[i].nid));

		test_failed = 0;
		memset(signature, 0, MAX_ECDSA_SIG_SIZE);

		eckey = ica_ec_key_new(ecdsa_kats[i].nid, &privlen);
		rc = ica_ec_key_init(ecdsa_kats[i].x, ecdsa_kats[i].y, ecdsa_kats[i].d, eckey);

		for (j = 0; j<NUM_HASH_LENGTHS; j++) {

			for (k = 0; k < NUM_HW_SW_TESTS; k++) {

				if (is_supported_openssl_curve(ecdsa_kats[i].nid) || getenv_icapath() == 2)
					toggle_env_icapath();

				/* calculate ECDSA signature */
				VV_(printf("  performing sign with ICAPATH=%d \n", getenv_icapath()));
				rc = ica_ecdsa_sign(adapter_handle, eckey, hash, hash_length[j],
						signature, MAX_ECDSA_SIG_SIZE);

				if (rc) {
					V_(printf("Signature could not be created, rc=%i.\n",rc));
					test_failed = 1;
					break;
				} else {

					if (is_supported_openssl_curve(ecdsa_kats[i].nid))
						toggle_env_icapath();

					/* verify ECDSA signature */
					VV_(printf("  performing verify with ICAPATH=%d \n", getenv_icapath()));
					rc = ica_ecdsa_verify(adapter_handle, eckey, hash, hash_length[j],
								  signature, MAX_ECDSA_SIG_SIZE);

					if (rc) {
						V_(printf("Signature could not be verified, rc=%i.\n",rc));
						test_failed = 1;
						break;
					}
				}
			}
		}

		if (test_failed)
			errors++;

		ica_ec_key_free(eckey);
		unset_env_icapath();
	}

	ica_close_adapter(adapter_handle);

	if (errors) {
		printf("%i of %li ECDSA tests failed.\n", errors, NUM_ECDSA_TESTS);
		return TEST_FAIL;
	}

	printf("All ECDSA tests passed.\n");
	return TEST_SUCC;
}
