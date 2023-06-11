#ifndef CERTIFICATE_MANAGER
#define CERTIFICATE_MANAGER

#include <dirent.h>
#include <stdbool.h>
#include <openssl/ssl.h>
#include <openssl/err.h> 
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/asn1.h>
#include <openssl/asn1t.h>


void createRootCertificate();
void createAndSignACertificate(char* hostname);
char* getCertificate(char* host);
bool doesRootCertificateExists();

#endif