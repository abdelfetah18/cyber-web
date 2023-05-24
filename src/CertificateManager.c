#include "headers/CertificateManager.h"

void createRootCertificate(){
    EVP_PKEY *pkey = EVP_PKEY_new();
    X509 *x509 = X509_new();

    // Generate private key
    EVP_PKEY_keygen(pkey, EVP_PKEY_RSA);

    // Set certificate version and serial number
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

    // Set issuer and subject names
    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"Cyber Web", -1, -1, 0);

    // Set validity period
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

    // Set public key
    X509_set_pubkey(x509, pkey);

    // Sign the certificate with private key
    X509_sign(x509, pkey, EVP_sha256());

    // Save certificate to file
    FILE *file = fopen("hosts/CyberWeb_RootCA.crt", "wb");
    PEM_write_X509(file, x509);
    fclose(file);

    // Save private key to file
    FILE *keyFile = fopen("hosts/CyberWeb_RootCA.key", "wb");
    PEM_write_PrivateKey(keyFile, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(keyFile);

    // Clean up
    X509_free(x509);
    EVP_PKEY_free(pkey);
}

void createCertificate(char* host){
    X509 *cert = NULL;
    EVP_PKEY *pkey = NULL;
    X509_NAME *subj = NULL;
    int days = 365;
    int serial = 1;

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    // Generate a new private key
    pkey = EVP_PKEY_new();
    if (!pkey) {
        fprintf(stderr, "Error generating private key.\n");
        exit(1);
    }

    // Generate a new certificate
    cert = X509_new();
    if (!cert) {
        fprintf(stderr, "Error generating certificate.\n");
        exit(1);
    }

    // Set the certificate's serial number
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);

    // Set the certificate's validity period
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), days * 24 * 60 * 60);

    // Set the certificate's subject
    subj = X509_NAME_new();
    X509_NAME_add_entry_by_txt(subj, "CN", MBSTRING_ASC, (unsigned char*) host, -1, -1, 0);
    X509_set_subject_name(cert, subj);

    // Set the certificate's issuer (self-signed)
    X509_set_issuer_name(cert, subj);

    // Set the certificate's public key
    X509_set_pubkey(cert, pkey);

    // Sign the certificate with the private key
    if (!X509_sign(cert, pkey, EVP_sha256())) {
        fprintf(stderr, "Error signing certificate.\n");
        exit(1);
    }

    // Write the certificate to a file
    FILE *fp = fopen("hosts/wildcard.crt", "wb");
    if (!fp) {
        fprintf(stderr, "Error opening certificate file for writing.\n");
        exit(1);
    }
    PEM_write_X509(fp, cert);
    fclose(fp);

    // Save private key to file
    FILE *keyFile = fopen("hosts/wildcard.key", "wb");
    PEM_write_PrivateKey(keyFile, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(keyFile);

    // Clean up
    X509_free(cert);
    EVP_PKEY_free(pkey);
    X509_NAME_free(subj);
}

void createAndSignACertificate(char* hostname){
    X509 *cert = NULL;
    EVP_PKEY *pkey = NULL;
    X509 *rootCert = NULL;
    EVP_PKEY *rootKey = NULL;
    X509_NAME *subj = NULL;
    int days = 365;
    int serial = 1;

    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    ERR_load_crypto_strings();

    // Load the Root CA's private key
    FILE *rootKeyFile = fopen("hosts/CyberWeb.key", "rb");
    if (!rootKeyFile) {
        fprintf(stderr, "Error opening Root CA private key file.\n");
        exit(1);
    }
    rootKey = PEM_read_PrivateKey(rootKeyFile, NULL, NULL, NULL);
    fclose(rootKeyFile);
    if (!rootKey) {
        fprintf(stderr, "Error loading Root CA private key.\n");
        exit(1);
    }

    // Load the Root CA's certificate
    FILE *rootCertFile = fopen("hosts/CyberWeb.crt", "rb");
    if (!rootCertFile) {
        fprintf(stderr, "Error opening Root CA certificate file.\n");
        exit(1);
    }
    rootCert = PEM_read_X509(rootCertFile, NULL, NULL, NULL);
    fclose(rootCertFile);
    if (!rootCert) {
        fprintf(stderr, "Error loading Root CA certificate.\n");
        exit(1);
    }

    // Generate a new private key
    pkey = EVP_PKEY_new();
    if (!pkey) {
        fprintf(stderr, "Error generating private key.\n");
        exit(1);
    }

    // Generate a new private key
    RSA *rsa = RSA_new();
    BIGNUM *bn = BN_new();
    if (!rsa || !bn) {
        fprintf(stderr, "Error generating private key.\n");
        exit(1);
    }
    BN_set_word(bn, RSA_F4);
    RSA_generate_key_ex(rsa, 2048, bn, NULL);
    EVP_PKEY_assign_RSA(pkey, rsa);


    // Generate a new certificate
    cert = X509_new();
    if (!cert) {
        fprintf(stderr, "Error generating certificate.\n");
        exit(1);
    }

    X509_set_version(cert, 2);

    // Set the certificate's serial number
    ASN1_INTEGER_set(X509_get_serialNumber(cert), serial);

    // Set the certificate's validity period
    X509_gmtime_adj(X509_get_notBefore(cert), 0);
    X509_gmtime_adj(X509_get_notAfter(cert), days * 24 * 60 * 60);

    // Set the certificate's subject
    subj = X509_NAME_new();
    X509_NAME_add_entry_by_txt(subj, "CN", MBSTRING_ASC, (unsigned char*) hostname, -1, -1, 0);
    X509_set_subject_name(cert, subj);

    // Set the certificate's issuer (Root CA)
    X509_set_issuer_name(cert, X509_get_subject_name(rootCert));

    // Set the certificate's public key
    X509_set_pubkey(cert, pkey);

    // Add Subject Alternative Names (SANs)
    GENERAL_NAMES *san_names = sk_GENERAL_NAME_new_null();
    if (!san_names) {
        fprintf(stderr, "Error creating SANs.\n");
        exit(1);
    }

    // Add DNS names to SANs
    GENERAL_NAME *dns_name = GENERAL_NAME_new();
    ASN1_IA5STRING *ia5str = ASN1_IA5STRING_new();
    ASN1_STRING_set(ia5str, hostname, -1);
    GENERAL_NAME_set0_value(dns_name, GEN_DNS, ia5str);
    sk_GENERAL_NAME_push(san_names, dns_name);

    // Set SANs extension in the certificate
    X509_EXTENSION *ext = X509V3_EXT_i2d(NID_subject_alt_name, 0, san_names);
    X509_add_ext(cert, ext, -1);
    X509_EXTENSION_free(ext);

    // Sign the certificate with the Root CA's private key
    if (!X509_sign(cert, rootKey, EVP_sha256())) {
        fprintf(stderr, "Error signing certificate.\n");
        
        unsigned long err = ERR_peek_error();
        printf("err: %lu\n", err);
        printf("err_hex: 0x%lx\n", err);
        printf("err_hex: 0x%p\n", err);
        printf("err_hex: 0x%x\n", err);
        
        exit(1);
    }

    char* path = "hosts/";
    char* target_host_file = malloc((sizeof(char) * (strlen(hostname) + 11)));
    strcpy(target_host_file, path);
    strcpy(target_host_file+6, hostname);
    strcpy(target_host_file+6+strlen(hostname), ".crt");

    // Write the signed certificate to a file
    FILE *fp = fopen(target_host_file, "wb");
    if (!fp) {
        fprintf(stderr, "Error opening certificate file for writing.\n");
        exit(1);
    }
    PEM_write_X509(fp, cert);
    fclose(fp);

    strcpy(target_host_file+6+strlen(hostname), ".key");
    // Save private key to file
    FILE *keyFile = fopen(target_host_file, "wb");
    PEM_write_PrivateKey(keyFile, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(keyFile);


    // Clean up
    X509_free(cert);
    EVP_PKEY_free(pkey);
    X509_NAME_free(subj);
    X509_free(rootCert);
    EVP_PKEY_free(rootKey);
}

char* getCertificate(char* hostname){
    DIR *dir;
    struct dirent *entry;

    dir = opendir("./hosts");
    if (dir == NULL) {
        perror("opendir");
        exit(EXIT_FAILURE);
    }
    
    char* target_host_file = malloc((sizeof(char) * strlen(hostname)) + 5);
    strcpy(target_host_file, hostname);
    strcpy(target_host_file+strlen(target_host_file), ".crt");

    bool does_exist = false;
    while((entry = readdir(dir)) != NULL){
        if(strcmp(entry->d_name, target_host_file) == 0){
            printf("[*] Already exists: %s\n", entry->d_name);
            does_exist = true;
            break;
        }
    }
    closedir(dir);

    if(does_exist == false){
        createAndSignACertificate(hostname);
    }
    
    return target_host_file;
}