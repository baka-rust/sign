# Terminology #

Some definitions that will be useful for reading this document:

* Signing: We say a piece of data is "signed" when it is encrypted using a *private* key. Encryption using a public key is just called encryption.

* Signature algorithm: A combination of a hashing algorithm (such as SHA) and a signing algorithm (such as RSA) for generating a digital signature. A signature algorithm performs the following transformation:

	     hash function      signing algorithm
	data ------------> hash ----------------> signature


* Certificate: A certificate is the combination of a subject's public key and identifying information (e.g. name). The certificate is usually issued by a certificate authority, which signs the certificate using its own private key. A certificate is used to verify a digital signature.


# TL;DR #

Instead of inventing our own signature algorithm and certificate format, we are going to tack a CMS signature onto the end of a binary and verify it upon binary load. Why? It turns out that the kernel already supports loading SIGNED MODULES (yay!). Signed modules have a CMS signature tacked onto the end of them, which the kernel verifies upon module load. The kernel also validates that the certificate used for verification has been signed by a trusted authority. The kernel maintains a list of trusted public keys that it uses for trust validation. This is all explained [here](https://www.kernel.org/doc/Documentation/module-signing.txt). Also, see system_keyring.c in the certs/ folder.

Basically, all of the code signing logic is already implemented. Signed modules is a kernel configuration option that is not enabled by default. When building our version of the kernel, it must be enabled, as our feature builds on top of it.


# What is X.509? #

X.509 is a standard for digital certificates issued by certificate authorities. As a certificate, it contains a subject's public key and identifying information, and it is signed by a certificate authority. X.509 certificates are ASN.1 data structures that can be encoded in various formats, including DER and PEM.

Usually, X.509 certificates are obtained by creating a certificate signing request (CSR) and sending it to the CA, which will then issue the certificate. However, it is also possible to generate a self-signed certificate using OpenSSL, as follows.

## Generating a 4096-bit RSA public/private key pair and self-signed X.509 certificate: ##

openssl req -x509 -newkey rsa:4096 -keyout privatekey.pem -out certificate.pem -days 365


# What is PKCS#7/CMS? #

PCKS#7 is a standard for encrypting/signing messages using a certificate from a CA (e.g. an X.509 certificate). It is superseded by CMS (Cryptographic Message Syntax), although the two are largely identical. PKCS#7 defines an ASN.1 data structure that contains a chunk of data, as well as signatures for that data. It is possible to decouple the data from the signatures, such that the data structure does not contain the actual data that is signed.

## Generating a CMS signature of an arbitrary piece of data using the private key + X.509 certificate ##

openssl cms -sign -signer certificate.pem -inkey privatekey.pem -binary -in <data to sign> -outform der -out signature -noattr



