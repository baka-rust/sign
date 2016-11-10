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

static int crypto_init(void)
{
	printk(KERN_ALERT "crypto_init called!\n");
	
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

	SHASH_DESC_ON_STACK(hashDesc, algoHandle);
	hashDesc->tfm = algoHandle;
	hashDesc->flags = 0;
	
	unsigned int hashSize = crypto_shash_digestsize(algoHandle);
	printk("Hash size: %u", hashSize);

	u8 hash[hashSize];

	unsigned bufferSize = strlen(buffer);
	printk("Buffer size: %u", bufferSize);
	
	if (crypto_shash_digest(hashDesc, buffer, strlen(buffer), hash) == 0) {
		printk("Successfully hashed %s:", buffer);

		print_hex_dump(KERN_ALERT, "", DUMP_PREFIX_NONE, 16, 1, hash, hashSize, 1);
	} else {
		printk("Hashing failed!");
	}

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
