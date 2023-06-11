#include "headers/CertificateManager.h"

bool doesRootCertificateExists(){
    FILE* file = fopen("hosts/CyberWeb.crt", "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

void createRootCertificate(){
    // Generate a new private key
    EVP_PKEY *pkey = EVP_PKEY_new();
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

    X509 *x509 = X509_new();

    // Set certificate version and serial number
    X509_set_version(x509, 2);
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

    // Set issuer and subject names
    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"Cyber Web", -1, -1, 0);
    X509_set_issuer_name(x509, name);

    // Set validity period
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

    BASIC_CONSTRAINTS* bc = BASIC_CONSTRAINTS_new();
    bc->ca = 1;
    bc->pathlen = 0;
    unsigned char* bcData = NULL;

    int bcDataLength = i2d_BASIC_CONSTRAINTS(bc, &bcData);
    
    printf("len: %d\n", bcDataLength);
    ASN1_OCTET_STRING* extData = ASN1_OCTET_STRING_new();
    ASN1_OCTET_STRING_set(extData, bcData, bcDataLength);

    X509_EXTENSION* ext__ = X509_EXTENSION_new();
    X509_EXTENSION_set_data(ext__, extData);
    X509_EXTENSION_set_object(ext__, OBJ_nid2obj(NID_basic_constraints));
    X509_add_ext(x509, ext__, -1);


    ASN1_OCTET_STRING* constraint = ASN1_OCTET_STRING_new();
    ASN1_OCTET_STRING_set(constraint, "CA:TRUE", 7);
    // Create a basic constraints extension with CA:TRUE
    X509_EXTENSION* ext = X509_EXTENSION_create_by_NID(NULL, NID_basic_constraints, 0, constraint);
    X509_EXTENSION_set_critical(ext, 1);
    
    // Add the extension to the certificate
    X509_add_ext(x509, ext, -1);

    

    // Create Authority Key.
    unsigned char AUTHORITY_KEY[20];
    int alen = 20;
    memset(AUTHORITY_KEY, 0, 20);
    AUTHORITY_KEYID* auth_key = AUTHORITY_KEYID_new();
    auth_key->keyid = X509_pubkey_digest(x509, EVP_sha1(), AUTHORITY_KEY, &alen);
    unsigned char AUTHORITY_KEY_BUFFER[40];
    memset(AUTHORITY_KEY_BUFFER, 0, 40);
    for(int i = 0; i < 20; i++){
        sprintf(AUTHORITY_KEY_BUFFER+(i*2),"%02x", AUTHORITY_KEY[i]);
    }
    printf("%s\n", AUTHORITY_KEY_BUFFER);
    ASN1_OCTET_STRING* authority_keyid = ASN1_OCTET_STRING_new();
    ASN1_OCTET_STRING_set(authority_keyid, AUTHORITY_KEY_BUFFER, 40);
    
    // Create the extension with the authority key identifier structure
    X509_EXTENSION* ext_1 = X509_EXTENSION_create_by_NID(NULL, NID_authority_key_identifier, 0, authority_keyid);
    X509_EXTENSION* ext_2 = X509_EXTENSION_create_by_NID(NULL, NID_subject_key_identifier, 0, authority_keyid);

    // Add the extension to the certificate
    X509_add_ext(x509, ext_1, -1);
    X509_add_ext(x509, ext_2, -1);

    // Add Key Usage
    ASN1_OCTET_STRING* key_usage = ASN1_OCTET_STRING_new();
    ASN1_OCTET_STRING_set(key_usage, "Certificate Sign, CRL Sign", 26);
    // Create a basic constraints extension with CA:TRUE
    X509_EXTENSION* ext_3 = X509_EXTENSION_create_by_NID(NULL, NID_key_usage, 0, key_usage);
    X509_EXTENSION_set_critical(ext_3, 1);
    X509_add_ext(x509, ext_3, -1);

    // Set public key
    X509_set_pubkey(x509, pkey);

    // Sign the certificate with private key
    X509_sign(x509, pkey, EVP_sha256());

    
    int is_ca = X509_check_ca(x509);
    printf("CA: %d\n", is_ca);

    // Save certificate to file
    FILE *file = fopen("hosts/CyberWeb.crt", "wb");
    PEM_write_X509(file, x509);
    fclose(file);

    // Save private key to file
    FILE *keyFile = fopen("hosts/CyberWeb.key", "wb");
    PEM_write_PrivateKey(keyFile, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(keyFile);

    // Clean up
    X509_free(x509);
    EVP_PKEY_free(pkey);
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