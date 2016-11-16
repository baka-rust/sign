/*
 * Experimenting with the kernel's cryptographic API
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/crypto.h>
#include <linux/kern_levels.h>
#include <crypto/hash.h>
#include <linux/delay.h>
#include <linux/scatterlist.h>
#include <crypto/akcipher.h>
#include <crypto/pkcs7.h>

struct sdesc {
    struct shash_desc shash;
    char ctx[];
};

unsigned char example_2048_rsa_pubkey[] = {
  0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
  0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xfb, 0x46, 0xbe,
  0x39, 0x51, 0xc7, 0x35, 0x38, 0x2a, 0x7b, 0x74, 0x66, 0x2a, 0xba, 0xbe,
  0x07, 0x2a, 0x3f, 0x4f, 0x9b, 0x9e, 0x42, 0xe2, 0x35, 0x4c, 0xca, 0x82,
  0xe0, 0x73, 0x8f, 0xf7, 0x6d, 0x38, 0x85, 0x9e, 0x2b, 0x39, 0x1b, 0x9e,
  0x03, 0x60, 0x08, 0x15, 0xf1, 0x00, 0xd0, 0x64, 0xa2, 0x8a, 0xed, 0x15,
  0xef, 0xcc, 0xa2, 0x4f, 0x54, 0x64, 0x9e, 0xee, 0xce, 0xca, 0x96, 0x2b,
  0xf2, 0x9e, 0x47, 0x60, 0x01, 0xed, 0x71, 0xe7, 0x1f, 0x55, 0xc5, 0x41,
  0x0b, 0x59, 0x79, 0x80, 0x17, 0xa8, 0xd6, 0x99, 0xd9, 0x45, 0x24, 0x7f,
  0x08, 0xae, 0xb3, 0x61, 0xab, 0xf1, 0xa1, 0x77, 0xa3, 0x30, 0x46, 0xf9,
  0xd1, 0x55, 0xf8, 0xd0, 0xb8, 0x68, 0xc7, 0xd6, 0x23, 0x33, 0xda, 0x42,
  0x9a, 0x00, 0x15, 0xf8, 0xed, 0x1c, 0xd5, 0xb6, 0x99, 0xb1, 0x77, 0xbc,
  0x23, 0xca, 0xd6, 0xd2, 0xb2, 0x4c, 0xd4, 0xf0, 0xe9, 0x89, 0x5f, 0xc1,
  0x1c, 0x15, 0xd9, 0xa1, 0xcb, 0xd2, 0x25, 0xb6, 0x89, 0xac, 0xaa, 0x4a,
  0xd2, 0xb8, 0x4f, 0x1c, 0xf4, 0x19, 0xba, 0x5a, 0x92, 0x38, 0x48, 0x56,
  0x4e, 0x8c, 0x7b, 0x1a, 0xbb, 0x91, 0x36, 0x40, 0x86, 0x01, 0x9f, 0xeb,
  0x60, 0x23, 0x00, 0xae, 0x34, 0xd6, 0x97, 0x31, 0x4c, 0x5c, 0x44, 0xf2,
  0x40, 0x6b, 0x60, 0x4f, 0x3a, 0x6a, 0x60, 0xe6, 0x62, 0xf9, 0xf7, 0x19,
  0xf9, 0xd7, 0xf0, 0xf4, 0x9c, 0xda, 0xea, 0xb5, 0xa7, 0x98, 0xad, 0x32,
  0x13, 0x3a, 0x54, 0xa9, 0x76, 0xbc, 0xd3, 0xdf, 0x6e, 0xb0, 0xa0, 0x26,
  0x8d, 0x18, 0xb3, 0xf2, 0xed, 0xd7, 0x15, 0xa1, 0x07, 0x5b, 0x65, 0x49,
  0xa9, 0x83, 0x26, 0x44, 0x7b, 0x70, 0x6c, 0x78, 0x3b, 0xf4, 0xde, 0xd7,
  0xe9, 0x45, 0xc7, 0xf4, 0x55, 0x7a, 0x3b, 0xd7, 0x0c, 0x81, 0x07, 0x25,
  0xff, 0x02, 0x03, 0x01, 0x00, 0x01
};

static const unsigned int pubkey_size = 294;

// The string "Hello, world!" signed using the private key associated with example_2048_rsa_pubkey
unsigned char test_sig[] = {
  0xf3, 0x6f, 0xe0, 0xc2, 0x07, 0xbb, 0xf8, 0x03, 0x1f, 0x57, 0x0d, 0x51,
  0x24, 0xd8, 0x1d, 0xf1, 0xa4, 0xbe, 0x20, 0xb2, 0x46, 0x42, 0xb0, 0x89,
  0x61, 0x39, 0x6b, 0x43, 0x1e, 0x94, 0xe7, 0xa0, 0x46, 0xea, 0xe9, 0xee,
  0x65, 0x50, 0x59, 0x0c, 0x52, 0x06, 0xdc, 0xc3, 0x0c, 0x5c, 0x40, 0x59,
  0x86, 0x02, 0x32, 0x33, 0xa2, 0x5e, 0x1a, 0xc3, 0x59, 0x96, 0xc4, 0x6b,
  0x15, 0x4f, 0x2c, 0x76, 0x26, 0x4c, 0xe3, 0xd9, 0x0d, 0x46, 0x68, 0xe0,
  0x34, 0xbb, 0x34, 0x7d, 0x17, 0x49, 0xf2, 0x55, 0xd4, 0x1c, 0x6e, 0x68,
  0x8b, 0xd2, 0x16, 0xd8, 0x50, 0x01, 0xea, 0x5e, 0xf8, 0x00, 0x5d, 0x7c,
  0x6a, 0xed, 0xa7, 0xdd, 0xce, 0xae, 0x34, 0x8d, 0x46, 0x10, 0x48, 0xe0,
  0xfe, 0x0c, 0xa9, 0xb3, 0x68, 0x2c, 0xd0, 0x8b, 0xed, 0xea, 0x99, 0xbe,
  0x33, 0x95, 0x71, 0x65, 0x38, 0xf6, 0x5d, 0x18, 0xaf, 0xdf, 0x8a, 0xee,
  0x94, 0x18, 0x0f, 0x3d, 0x12, 0x7d, 0x4a, 0x33, 0xca, 0x82, 0x8d, 0x54,
  0x39, 0xc1, 0xe0, 0x0e, 0x54, 0xf3, 0xf2, 0xb3, 0xb6, 0x87, 0xd7, 0x0b,
  0x31, 0xcf, 0x3c, 0x94, 0xe8, 0xfc, 0x33, 0x63, 0x3f, 0x6d, 0x97, 0x60,
  0xec, 0xcd, 0x69, 0x27, 0x48, 0x4c, 0xce, 0x7a, 0x2f, 0x45, 0x55, 0xd1,
  0x1d, 0x49, 0x9a, 0x3c, 0x03, 0xa4, 0xa3, 0xb1, 0xfb, 0x90, 0x04, 0xe6,
  0x5e, 0x76, 0x5e, 0x39, 0x13, 0x26, 0xd9, 0x54, 0xc1, 0xe4, 0xd3, 0x7e,
  0x0f, 0xf4, 0xcb, 0x11, 0x5f, 0x42, 0x95, 0x1b, 0x5e, 0xdd, 0xa7, 0xf4,
  0x52, 0x10, 0xe5, 0x6e, 0x3c, 0x80, 0x0b, 0x25, 0xa1, 0x84, 0x6d, 0x5f,
  0x23, 0xde, 0xb6, 0x62, 0x33, 0xf1, 0xed, 0x74, 0x35, 0x2d, 0x18, 0xaf,
  0xdd, 0x66, 0x62, 0x54, 0x56, 0x85, 0xd1, 0x71, 0x53, 0x46, 0x60, 0x17,
  0xad, 0xb2, 0xad, 0x81
};

static const unsigned int test_sig_size = 256;

unsigned char testsignature[] = {
  0x30, 0x82, 0x08, 0xb3, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d,
  0x01, 0x07, 0x02, 0xa0, 0x82, 0x08, 0xa4, 0x30, 0x82, 0x08, 0xa0, 0x02,
  0x01, 0x01, 0x31, 0x0d, 0x30, 0x0b, 0x06, 0x09, 0x60, 0x86, 0x48, 0x01,
  0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0b, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x07, 0x01, 0xa0, 0x82, 0x05, 0xc9, 0x30, 0x82,
  0x05, 0xc5, 0x30, 0x82, 0x03, 0xad, 0xa0, 0x03, 0x02, 0x01, 0x02, 0x02,
  0x09, 0x00, 0xb0, 0x83, 0x9e, 0xb3, 0xf9, 0xae, 0x1d, 0x90, 0x30, 0x0d,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05,
  0x00, 0x30, 0x79, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06,
  0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04,
  0x08, 0x0c, 0x02, 0x4d, 0x4f, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55,
  0x04, 0x07, 0x0c, 0x08, 0x4f, 0x27, 0x46, 0x61, 0x6c, 0x6c, 0x6f, 0x6e,
  0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x05, 0x57,
  0x55, 0x53, 0x54, 0x4c, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04,
  0x03, 0x0c, 0x0d, 0x45, 0x74, 0x68, 0x61, 0x6e, 0x20, 0x56, 0x61, 0x75,
  0x67, 0x68, 0x61, 0x6e, 0x31, 0x22, 0x30, 0x20, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x13, 0x65, 0x6a, 0x76,
  0x61, 0x75, 0x67, 0x68, 0x61, 0x6e, 0x40, 0x67, 0x6d, 0x61, 0x69, 0x6c,
  0x2e, 0x63, 0x6f, 0x6d, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x36, 0x31, 0x31,
  0x31, 0x36, 0x30, 0x37, 0x30, 0x38, 0x30, 0x38, 0x5a, 0x17, 0x0d, 0x31,
  0x37, 0x31, 0x31, 0x31, 0x36, 0x30, 0x37, 0x30, 0x38, 0x30, 0x38, 0x5a,
  0x30, 0x79, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13,
  0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x08,
  0x0c, 0x02, 0x4d, 0x4f, 0x31, 0x11, 0x30, 0x0f, 0x06, 0x03, 0x55, 0x04,
  0x07, 0x0c, 0x08, 0x4f, 0x27, 0x46, 0x61, 0x6c, 0x6c, 0x6f, 0x6e, 0x31,
  0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x05, 0x57, 0x55,
  0x53, 0x54, 0x4c, 0x31, 0x16, 0x30, 0x14, 0x06, 0x03, 0x55, 0x04, 0x03,
  0x0c, 0x0d, 0x45, 0x74, 0x68, 0x61, 0x6e, 0x20, 0x56, 0x61, 0x75, 0x67,
  0x68, 0x61, 0x6e, 0x31, 0x22, 0x30, 0x20, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16, 0x13, 0x65, 0x6a, 0x76, 0x61,
  0x75, 0x67, 0x68, 0x61, 0x6e, 0x40, 0x67, 0x6d, 0x61, 0x69, 0x6c, 0x2e,
  0x63, 0x6f, 0x6d, 0x30, 0x82, 0x02, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a,
  0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82,
  0x02, 0x0f, 0x00, 0x30, 0x82, 0x02, 0x0a, 0x02, 0x82, 0x02, 0x01, 0x00,
  0xbd, 0x51, 0xf3, 0xc7, 0x68, 0x19, 0xe4, 0xc6, 0xaf, 0x88, 0xbc, 0x67,
  0x0d, 0x6e, 0xde, 0xe3, 0xcd, 0x38, 0x9a, 0x61, 0xe9, 0x99, 0x19, 0x02,
  0xfe, 0x94, 0x5a, 0x5a, 0x44, 0x60, 0x10, 0xb7, 0xfc, 0xb3, 0x85, 0xe1,
  0xb6, 0x6d, 0x03, 0x1a, 0xd7, 0xa7, 0xbe, 0xab, 0xf2, 0x27, 0x9b, 0x33,
  0x92, 0xfc, 0xb3, 0x2d, 0x00, 0xc1, 0x8c, 0xbf, 0x2d, 0x6c, 0x91, 0x76,
  0x40, 0x7e, 0x9c, 0xf1, 0x77, 0x9b, 0x2e, 0x77, 0xbb, 0x22, 0xfc, 0x49,
  0xc5, 0xa7, 0xf8, 0xcf, 0x4e, 0x7d, 0xdb, 0x66, 0xff, 0xbb, 0xf0, 0x0a,
  0x4c, 0x0d, 0xc4, 0xe9, 0x31, 0x03, 0x37, 0x9d, 0x86, 0x07, 0x64, 0xa5,
  0x67, 0xfa, 0x57, 0x87, 0x73, 0x78, 0x71, 0xbc, 0x5b, 0xfa, 0x36, 0x91,
  0x53, 0x02, 0x5a, 0x72, 0x4e, 0x64, 0x42, 0xb2, 0xd1, 0xb4, 0x4e, 0x0b,
  0xc8, 0xc9, 0x6c, 0xf6, 0x6e, 0xfd, 0xe7, 0xb6, 0x39, 0x13, 0x0a, 0xcb,
  0xc0, 0xf5, 0xc7, 0x5b, 0x8f, 0x7d, 0x31, 0x8a, 0xf9, 0xbe, 0x4a, 0xff,
  0xca, 0xbd, 0x04, 0x64, 0x90, 0x22, 0x59, 0x0e, 0x0a, 0x9e, 0x7d, 0xd9,
  0x0a, 0x3e, 0xb4, 0x47, 0x80, 0xb1, 0xd9, 0x7b, 0x1b, 0xb8, 0x2a, 0xd9,
  0x01, 0x39, 0xf1, 0x7a, 0xf3, 0x94, 0x7f, 0xde, 0xe2, 0x76, 0x43, 0x5c,
  0x15, 0x0a, 0xce, 0xcc, 0x2c, 0x22, 0xfa, 0xe0, 0xad, 0x9a, 0x59, 0xf5,
  0xc4, 0xc7, 0x87, 0x5d, 0xb4, 0x6d, 0x2d, 0xad, 0x1f, 0x21, 0xc3, 0xdc,
  0xd6, 0x90, 0x3f, 0x24, 0x19, 0x85, 0x37, 0x69, 0x4f, 0x9d, 0x7d, 0x6b,
  0xb7, 0xd7, 0x53, 0x8b, 0x5f, 0xd8, 0xb5, 0x8c, 0x5c, 0x58, 0x9f, 0x18,
  0x7f, 0xbb, 0x98, 0x94, 0x0b, 0xf4, 0x61, 0xfa, 0x14, 0xf2, 0xcd, 0x56,
  0xd8, 0x21, 0x46, 0xcc, 0x91, 0x15, 0xcb, 0xc3, 0x89, 0xc8, 0xe5, 0xd8,
  0x0f, 0xa4, 0xc4, 0x5b, 0x15, 0x84, 0xb4, 0xe2, 0x92, 0x44, 0xfa, 0x7c,
  0x67, 0x98, 0xa6, 0x56, 0x46, 0xc1, 0x64, 0xf0, 0x77, 0xdf, 0x9a, 0x3d,
  0xbd, 0xb4, 0xb9, 0x57, 0x13, 0xf4, 0xb9, 0xeb, 0x7b, 0xc2, 0x83, 0x0a,
  0x4c, 0xa9, 0x2f, 0x20, 0xd7, 0xc8, 0x7d, 0xf5, 0x58, 0x59, 0xff, 0xf9,
  0xe4, 0x2b, 0xa0, 0x73, 0x4d, 0x8b, 0x01, 0x03, 0x3a, 0x15, 0xbd, 0xca,
  0xbf, 0x52, 0x0c, 0x07, 0x71, 0xc9, 0x8d, 0x4e, 0x5d, 0x37, 0xd8, 0x18,
  0x1f, 0x65, 0x8b, 0xf6, 0xe6, 0x00, 0x9a, 0x88, 0xd2, 0x20, 0x4c, 0xba,
  0xd8, 0xc6, 0xd9, 0x73, 0x12, 0xae, 0x15, 0x9f, 0x22, 0xbb, 0x17, 0x92,
  0x3b, 0xf9, 0x27, 0x5f, 0xe3, 0xcd, 0x55, 0x60, 0xc4, 0x42, 0x10, 0xda,
  0x90, 0x63, 0x38, 0x46, 0x7a, 0x90, 0x9b, 0x0d, 0x95, 0x71, 0xe7, 0xad,
  0xf6, 0xb6, 0x20, 0xda, 0xc5, 0x3b, 0x29, 0x9c, 0x51, 0xc3, 0x24, 0xee,
  0x1b, 0x43, 0x81, 0x4c, 0xb9, 0x05, 0x07, 0x07, 0x90, 0xfc, 0x08, 0x6d,
  0x40, 0xdb, 0x52, 0x7c, 0xdd, 0x19, 0xf0, 0x74, 0x42, 0x75, 0xb6, 0x14,
  0xa3, 0x24, 0x8f, 0x81, 0x79, 0xbd, 0xa5, 0x3f, 0x57, 0xd9, 0x68, 0x92,
  0x73, 0x09, 0x3a, 0x91, 0xdc, 0x37, 0xaa, 0x39, 0xb1, 0xa1, 0x07, 0x7a,
  0xcc, 0x2e, 0xf1, 0xad, 0xf4, 0x7b, 0x20, 0x41, 0xdd, 0xb6, 0x00, 0xdc,
  0xc9, 0x48, 0x90, 0x3c, 0x25, 0x26, 0xa7, 0x9e, 0x15, 0x74, 0x76, 0x55,
  0x44, 0xfc, 0x29, 0xd2, 0xdf, 0xc3, 0x2a, 0x24, 0x61, 0xa4, 0x7d, 0x3a,
  0x77, 0x61, 0xee, 0x90, 0x49, 0x11, 0xdf, 0xb7, 0x1e, 0x9c, 0x2b, 0x3f,
  0xfb, 0x25, 0x5b, 0x15, 0x97, 0xea, 0x41, 0xce, 0x8c, 0x46, 0x61, 0x40,
  0xc5, 0xaf, 0x8f, 0x5b, 0x0c, 0x77, 0xcd, 0xfc, 0x0f, 0xa6, 0xd7, 0x95,
  0xab, 0x22, 0x38, 0xb7, 0xbc, 0x42, 0xe9, 0x53, 0x02, 0x03, 0x01, 0x00,
  0x01, 0xa3, 0x50, 0x30, 0x4e, 0x30, 0x1d, 0x06, 0x03, 0x55, 0x1d, 0x0e,
  0x04, 0x16, 0x04, 0x14, 0x1e, 0xbd, 0xc2, 0x24, 0x6d, 0x14, 0x50, 0x84,
  0x60, 0xff, 0xff, 0xf8, 0xa7, 0xc9, 0x78, 0x0a, 0xcc, 0x20, 0x11, 0x77,
  0x30, 0x1f, 0x06, 0x03, 0x55, 0x1d, 0x23, 0x04, 0x18, 0x30, 0x16, 0x80,
  0x14, 0x1e, 0xbd, 0xc2, 0x24, 0x6d, 0x14, 0x50, 0x84, 0x60, 0xff, 0xff,
  0xf8, 0xa7, 0xc9, 0x78, 0x0a, 0xcc, 0x20, 0x11, 0x77, 0x30, 0x0c, 0x06,
  0x03, 0x55, 0x1d, 0x13, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xff, 0x30,
  0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b,
  0x05, 0x00, 0x03, 0x82, 0x02, 0x01, 0x00, 0x95, 0x8e, 0x29, 0xed, 0x4f,
  0x31, 0x7f, 0x9b, 0xd3, 0xc9, 0x56, 0xfc, 0x4c, 0xa4, 0xbb, 0x91, 0x00,
  0x87, 0xfa, 0x8e, 0x95, 0x48, 0xbe, 0x7b, 0x6b, 0x6c, 0xeb, 0x1d, 0x5b,
  0xde, 0xf6, 0x5b, 0xf7, 0xfa, 0x38, 0x03, 0xc1, 0xa5, 0xea, 0x89, 0xcb,
  0x15, 0xc0, 0x16, 0xae, 0x5a, 0x44, 0x4c, 0x47, 0x8b, 0x5c, 0x68, 0x99,
  0x23, 0xa3, 0xd5, 0xec, 0xb2, 0xd6, 0x89, 0xa0, 0xe9, 0xd9, 0xd9, 0x59,
  0x0b, 0x80, 0x8c, 0x22, 0x54, 0x6b, 0x2f, 0x76, 0x50, 0xf0, 0xbc, 0xa0,
  0xec, 0x8d, 0x10, 0xcb, 0x29, 0x54, 0x88, 0x5c, 0xcd, 0x15, 0x6c, 0x76,
  0x91, 0xac, 0x1d, 0x75, 0xae, 0xc5, 0x3f, 0x7d, 0xaf, 0x05, 0x3d, 0x9e,
  0x3a, 0x86, 0x63, 0x6c, 0x79, 0xfe, 0x4a, 0x84, 0x92, 0x02, 0x83, 0xf7,
  0x95, 0x56, 0x4a, 0xbb, 0xb4, 0x6c, 0xf3, 0x68, 0x7d, 0x0d, 0x1c, 0xa2,
  0xbf, 0xb3, 0x23, 0x06, 0xe4, 0x11, 0x4d, 0x5a, 0xda, 0xdb, 0xb7, 0x65,
  0xa0, 0xd8, 0xb0, 0x97, 0xbe, 0xbb, 0xc7, 0xc4, 0x1b, 0xda, 0xd6, 0x18,
  0x6c, 0xc5, 0x0d, 0xa7, 0x3f, 0x14, 0x63, 0x5f, 0x55, 0x53, 0xae, 0x13,
  0x74, 0xd1, 0xd6, 0xde, 0x25, 0x73, 0x7c, 0x30, 0xc1, 0xce, 0x2a, 0x56,
  0xd4, 0xf6, 0x63, 0x17, 0x37, 0xa3, 0xb1, 0x13, 0x07, 0xcc, 0xc8, 0xe6,
  0xb6, 0x64, 0x7e, 0x10, 0xcc, 0x93, 0x91, 0xc2, 0x60, 0xb4, 0x2b, 0xbe,
  0xcd, 0x87, 0x13, 0xec, 0x2a, 0x93, 0xc9, 0xda, 0xe2, 0x2d, 0xbf, 0x3c,
  0x80, 0xb2, 0xc1, 0x89, 0xf6, 0xac, 0x44, 0xc8, 0x3b, 0x29, 0x2a, 0x88,
  0x46, 0x14, 0x5f, 0x83, 0x6e, 0xf1, 0xc6, 0xf8, 0xfd, 0x3d, 0x17, 0x9a,
  0x04, 0x3c, 0x08, 0x1a, 0x56, 0x78, 0xfa, 0x7b, 0xa8, 0x0f, 0x01, 0x8f,
  0x91, 0x14, 0xaa, 0xe5, 0xba, 0xda, 0x89, 0xfd, 0xca, 0x0b, 0xfb, 0x4c,
  0x79, 0x80, 0x61, 0x9d, 0xac, 0xec, 0x1c, 0xc0, 0x41, 0x44, 0x43, 0x57,
  0x39, 0xe7, 0x6a, 0x16, 0xc2, 0x1b, 0xd2, 0x6c, 0x64, 0x9f, 0x00, 0x2b,
  0xad, 0xab, 0x95, 0x87, 0x93, 0xd3, 0xfb, 0x2a, 0xcb, 0x5b, 0x9b, 0x3c,
  0xa8, 0xfc, 0xac, 0xb1, 0x47, 0x39, 0x5f, 0x49, 0x2f, 0x12, 0x0a, 0xd6,
  0x28, 0x18, 0x5f, 0x6b, 0x69, 0xd5, 0x02, 0x2f, 0x99, 0xd6, 0x3a, 0x35,
  0x4d, 0xe6, 0x71, 0x62, 0xb8, 0xab, 0xbc, 0xe7, 0xe8, 0x20, 0x28, 0x57,
  0x64, 0x90, 0x0d, 0xda, 0xa2, 0xa6, 0xb2, 0x35, 0x0b, 0x62, 0xb0, 0x7d,
  0x16, 0x4a, 0x5b, 0x66, 0xf5, 0x89, 0x9b, 0xc1, 0x61, 0x54, 0x55, 0x07,
  0xbd, 0x02, 0xbf, 0xf5, 0x49, 0xe8, 0x77, 0xda, 0x54, 0x62, 0x44, 0x4e,
  0x7f, 0x62, 0x82, 0x07, 0x36, 0x6a, 0xe5, 0x62, 0x9e, 0x2d, 0x2e, 0x98,
  0x44, 0x2a, 0x27, 0xbc, 0xef, 0x70, 0xb4, 0xa1, 0x89, 0x0c, 0x2b, 0x86,
  0x68, 0x15, 0x26, 0x39, 0xcc, 0x46, 0x61, 0xea, 0x86, 0x67, 0x0e, 0xa4,
  0xeb, 0x19, 0x71, 0x7d, 0x89, 0xf7, 0xb2, 0xba, 0xbc, 0xd5, 0xb2, 0xe2,
  0xb5, 0x5d, 0xf3, 0xfd, 0x40, 0x5b, 0x7e, 0x97, 0x78, 0x0d, 0xd7, 0x12,
  0xa0, 0x9e, 0x86, 0xd6, 0x2e, 0x9e, 0x17, 0xef, 0x3e, 0x7d, 0x00, 0x26,
  0x5f, 0xd8, 0xf1, 0x7b, 0xc7, 0x2e, 0xa2, 0xa1, 0xb4, 0x97, 0x8e, 0x25,
  0xea, 0xb1, 0xbd, 0x65, 0xee, 0x1b, 0xb6, 0xae, 0x42, 0xf2, 0x87, 0x06,
  0xb1, 0xab, 0x54, 0x92, 0xd4, 0xb7, 0x56, 0x61, 0xbf, 0x9f, 0x7f, 0x09,
  0xa4, 0x2f, 0x1f, 0x6a, 0x71, 0x01, 0x49, 0x4a, 0x24, 0x59, 0x90, 0xe4,
  0x26, 0xe3, 0x25, 0xc2, 0x61, 0x06, 0x57, 0xf2, 0xd9, 0x6d, 0x54, 0xfb,
  0xa9, 0xa0, 0xd2, 0xe7, 0x3c, 0x6e, 0x96, 0xa2, 0x21, 0xc2, 0xea, 0x25,
  0x02, 0x26, 0xac, 0x31, 0x82, 0x02, 0xb0, 0x30, 0x82, 0x02, 0xac, 0x02,
  0x01, 0x01, 0x30, 0x81, 0x86, 0x30, 0x79, 0x31, 0x0b, 0x30, 0x09, 0x06,
  0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x55, 0x53, 0x31, 0x0b, 0x30, 0x09,
  0x06, 0x03, 0x55, 0x04, 0x08, 0x0c, 0x02, 0x4d, 0x4f, 0x31, 0x11, 0x30,
  0x0f, 0x06, 0x03, 0x55, 0x04, 0x07, 0x0c, 0x08, 0x4f, 0x27, 0x46, 0x61,
  0x6c, 0x6c, 0x6f, 0x6e, 0x31, 0x0e, 0x30, 0x0c, 0x06, 0x03, 0x55, 0x04,
  0x0a, 0x0c, 0x05, 0x57, 0x55, 0x53, 0x54, 0x4c, 0x31, 0x16, 0x30, 0x14,
  0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0d, 0x45, 0x74, 0x68, 0x61, 0x6e,
  0x20, 0x56, 0x61, 0x75, 0x67, 0x68, 0x61, 0x6e, 0x31, 0x22, 0x30, 0x20,
  0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x09, 0x01, 0x16,
  0x13, 0x65, 0x6a, 0x76, 0x61, 0x75, 0x67, 0x68, 0x61, 0x6e, 0x40, 0x67,
  0x6d, 0x61, 0x69, 0x6c, 0x2e, 0x63, 0x6f, 0x6d, 0x02, 0x09, 0x00, 0xb0,
  0x83, 0x9e, 0xb3, 0xf9, 0xae, 0x1d, 0x90, 0x30, 0x0b, 0x06, 0x09, 0x60,
  0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01, 0x30, 0x0d, 0x06, 0x09,
  0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x04,
  0x82, 0x02, 0x00, 0x9e, 0x01, 0xf3, 0x68, 0xaa, 0x15, 0x41, 0x2e, 0x71,
  0x6d, 0x8d, 0xb5, 0xdf, 0x0e, 0x33, 0x95, 0xc6, 0x33, 0x6d, 0x6f, 0x7e,
  0x7e, 0x02, 0xf0, 0x5b, 0xed, 0xe2, 0xe4, 0x3f, 0xee, 0x76, 0xb5, 0x7e,
  0x49, 0xbe, 0x64, 0xd9, 0xa3, 0x7f, 0x2e, 0xcb, 0x16, 0xbb, 0x2f, 0x29,
  0x7f, 0x8c, 0x8f, 0x23, 0x84, 0x1b, 0x36, 0x66, 0xdc, 0xd5, 0x83, 0x3d,
  0x26, 0xd4, 0xf5, 0x71, 0xf7, 0xc4, 0xb6, 0x97, 0xb7, 0xab, 0x76, 0xd6,
  0x24, 0x7b, 0x46, 0x9b, 0x0d, 0xea, 0xbe, 0x40, 0x91, 0xdf, 0xcf, 0xb3,
  0xd0, 0x2d, 0x87, 0xd4, 0xaf, 0x8a, 0xb9, 0xe3, 0xcf, 0x8c, 0xaf, 0xb8,
  0xeb, 0x71, 0x90, 0x43, 0x03, 0x8e, 0x8b, 0x26, 0xb9, 0xb0, 0x63, 0x97,
  0xb4, 0xa6, 0xd8, 0x6d, 0x7c, 0xad, 0xb1, 0xcd, 0x96, 0xa4, 0x54, 0xd1,
  0x67, 0x4c, 0x14, 0x11, 0x4e, 0xc0, 0x3a, 0x58, 0x02, 0xfa, 0xa2, 0x8c,
  0x03, 0x1b, 0x71, 0xe2, 0x06, 0xc3, 0x3e, 0x63, 0x8b, 0x27, 0x5b, 0x25,
  0xe6, 0x78, 0xb3, 0xba, 0x03, 0x08, 0x54, 0x43, 0xc8, 0x74, 0xb3, 0xab,
  0xbc, 0x35, 0xe5, 0x4c, 0xf1, 0xc6, 0xb5, 0xd2, 0x32, 0x73, 0x31, 0x52,
  0xa6, 0xf6, 0x2d, 0xbd, 0x14, 0xb9, 0x7a, 0xac, 0x62, 0xbe, 0x5f, 0x87,
  0xb7, 0x54, 0x86, 0xed, 0x97, 0x99, 0x87, 0x50, 0x16, 0xb2, 0x5b, 0x22,
  0x33, 0xc5, 0x6e, 0x2a, 0xfd, 0xdd, 0xf7, 0xf7, 0x16, 0x08, 0x38, 0x0f,
  0xfb, 0x48, 0x50, 0x2a, 0x31, 0x88, 0x73, 0xef, 0x77, 0x0e, 0xfc, 0x14,
  0x47, 0x26, 0xee, 0xb3, 0x8d, 0x0c, 0x38, 0x9e, 0x9a, 0x39, 0x30, 0x6d,
  0xdf, 0xe9, 0x4c, 0x12, 0xdb, 0x91, 0x91, 0x6b, 0xc3, 0xf1, 0xa3, 0xe2,
  0xeb, 0x4e, 0x35, 0x23, 0xce, 0x8d, 0x2e, 0x55, 0x76, 0x05, 0x5f, 0x56,
  0xd0, 0x49, 0x75, 0x50, 0xb2, 0xe5, 0x98, 0xe8, 0xcb, 0x35, 0xb1, 0xef,
  0x2d, 0xaa, 0x79, 0x4a, 0xe6, 0x8a, 0x23, 0x18, 0x22, 0xad, 0x69, 0x69,
  0xe3, 0xe7, 0x2b, 0x4c, 0xd5, 0x20, 0xbe, 0xba, 0x9d, 0xb8, 0x66, 0x25,
  0x47, 0x9b, 0x38, 0x2c, 0xc5, 0xc3, 0x86, 0xfa, 0xeb, 0x25, 0x0b, 0x38,
  0x7b, 0x09, 0x66, 0x97, 0x5f, 0xb4, 0xf4, 0xc5, 0xc3, 0xe9, 0x52, 0x5b,
  0xd8, 0x34, 0xd1, 0x11, 0xe8, 0xd8, 0x8e, 0xf8, 0x1e, 0xf3, 0x13, 0x3c,
  0x50, 0x9e, 0x73, 0x55, 0x8c, 0x80, 0xab, 0x49, 0x90, 0xa4, 0x44, 0x09,
  0xf8, 0x4f, 0x71, 0xc7, 0x84, 0x69, 0x7d, 0x0b, 0x09, 0x6f, 0x6b, 0xf5,
  0xa8, 0xdd, 0x4c, 0x36, 0x56, 0x48, 0x56, 0x19, 0xa5, 0xa2, 0x1c, 0x31,
  0x7e, 0x65, 0x8e, 0x4d, 0x45, 0x1b, 0x33, 0x9e, 0xd0, 0x42, 0xa4, 0xe8,
  0xc6, 0x0a, 0x3e, 0x3a, 0x43, 0x71, 0x5c, 0x05, 0x7d, 0x5b, 0xac, 0x35,
  0x1a, 0x79, 0xc7, 0x6a, 0xcb, 0x90, 0x64, 0x2c, 0xe0, 0xc3, 0x02, 0x0d,
  0x53, 0x6b, 0xef, 0xaf, 0x75, 0xa9, 0xc3, 0xba, 0x8c, 0xfe, 0xe1, 0xef,
  0x65, 0x2c, 0x3c, 0x8e, 0x1b, 0x46, 0xbf, 0xe3, 0x4d, 0x71, 0x26, 0xf2,
  0xae, 0xb1, 0x00, 0x8a, 0x6e, 0x12, 0xda, 0x4b, 0x54, 0x4c, 0x37, 0x38,
  0xbc, 0x14, 0x59, 0x0c, 0x0c, 0x10, 0xbd, 0x7f, 0xa1, 0x37, 0xf1, 0x51,
  0xd7, 0x60, 0xc3, 0xe5, 0x05, 0xc0, 0x55, 0x6c, 0x6b, 0x6e, 0x8e, 0x87,
  0xb8, 0x11, 0xe0, 0xa2, 0x3a, 0x3c, 0x30, 0x87, 0xc9, 0xb3, 0xed, 0x7f,
  0x79, 0xef, 0x35, 0x02, 0xec, 0xb3, 0x49, 0x92, 0x06, 0xad, 0xb5, 0x18,
  0x60, 0xa8, 0x3f, 0xf2, 0x73, 0x3e, 0x37, 0x14, 0x93, 0x66, 0x38, 0xd7,
  0xd9, 0x1a, 0x19, 0xaa, 0x3a, 0x42, 0x28, 0xba, 0x67, 0xfa, 0x85, 0xa3,
  0x0e, 0xa6, 0xfa, 0x01, 0x24, 0xf1, 0x82, 0xfe, 0xf0, 0x65, 0xc9
};
unsigned int testsignature_len = 2231;

static int crypto_init(void)
{
	printk(KERN_ALERT "crypto_init called!\n");

	/* SHA */
	
	// The data we want to hash
	const char *buffer = "Hello, world!";

	// A handle to the hashing algorithm
	struct crypto_shash *algoHandle = crypto_alloc_shash("sha256", 0, 0);
	
	if (IS_ERR(algoHandle)) {
		printk("Error creating handle for hashing algorithm!\n");
		return -1;
	} else {
		printk("Successfully created hashing algorithm!\n");
	}

	int size = sizeof(struct shash_desc) + crypto_shash_descsize(algoHandle);

	struct sdesc *hashDesc = kmalloc(size, GFP_KERNEL);
	hashDesc->shash.tfm = algoHandle;
	hashDesc->shash.flags = 0;	
	
	unsigned int hashSize = crypto_shash_digestsize(algoHandle);
	printk("Hash size: %u", hashSize);

	u8 hash[hashSize];

	unsigned bufferSize = strlen(buffer);
	printk("Buffer size: %u", bufferSize);
	
	if (crypto_shash_digest(&hashDesc->shash, buffer, strlen(buffer), hash) == 0) {
		printk(KERN_ALERT "Successfully hashed %s:", buffer);

		print_hex_dump(KERN_ALERT, "", DUMP_PREFIX_NONE, 16, 1, hash, hashSize, 1);
	} else {
		printk("Hashing failed!");
	}

	kfree(hashDesc);
	
	crypto_free_shash(algoHandle);

	/* PKCS#7 */
	
	const char *unsignedData = "Hello, world!\n";

	struct pkcs7_message *sig = pkcs7_parse_message(testsignature, testsignature_len);
	
	if (IS_ERR(sig)) {
		printk("Error parsing PKCS#7 signature: %d\n", PTR_ERR(sig));
		return 0;
	}

	if (pkcs7_supply_detached_data(sig, unsignedData, strlen(unsignedData)) != 0) {
		printk("Error supplying detached data for signature\n");
		return 0;
	}

	int verifyResult;
	if ((verifyResult = pkcs7_verify(sig, VERIFYING_UNSPECIFIED_SIGNATURE)) != 0) {
		printk("Error verifying signature: %d\n", verifyResult);
	}

	

	return 0;		

	/* RSA */

	// Get a handle to the algorithm
	struct crypto_akcipher *rsaHandle = crypto_alloc_akcipher("pkcs1pad(rsa, sha)", 0, 0);	

	if (IS_ERR(rsaHandle)) {
		printk(KERN_ALERT "Error allocating rsa algorithm handle!\n");
		return -1;
	}

	// Set public key
	if (crypto_akcipher_set_pub_key(rsaHandle, example_2048_rsa_pubkey, pubkey_size) != 0) {
		printk(KERN_ALERT "Error setting public key!\n");
		return -1;
	}

	// Create a verify request
	struct akcipher_request *verifyRequest = akcipher_request_alloc(rsaHandle, 0);

	if (verifyRequest == NULL) {
		printk(KERN_ALERT "Error creating verify request!\n");
		return -1;
	}

	// Allocate destination buffer
	struct scatterlist dest;
	int resultSize = crypto_akcipher_maxsize(rsaHandle);
	char *result = kmalloc(resultSize, GFP_KERNEL);
	sg_init_one(&dest, result, resultSize); 

	struct scatterlist src;
	sg_init_one(&src, test_sig, test_sig_size);	

	// Set request data
	akcipher_request_set_crypt(verifyRequest, &src, &dest, test_sig_size, resultSize);	

	if (crypto_akcipher_verify(verifyRequest) == 0) {
		printk(KERN_ALERT "Successfully verified signature!");
		print_hex_dump(KERN_ALERT, "", DUMP_PREFIX_NONE, 16, 1, result, resultSize, 1);	
	} else {
		printk(KERN_ALERT "Error verifying signature!\n");
	}

	// Free the request
	akcipher_request_free(verifyRequest);

	// Free the handle
	crypto_free_akcipher(rsaHandle);	

	return 0;
}

static void crypto_exit(void)
{
	printk(KERN_ALERT "crypto_exit called\n");
}

module_init(crypto_init);
module_exit(crypto_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ethan Vaughan");
MODULE_DESCRIPTION("A Crypto Module");
