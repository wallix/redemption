
#ifdef  __cplusplus
extern "C" {
#endif


typedef pthread_t CRYPTO_THREAD_ID;
typedef void CRYPTO_RWLOCK;

typedef struct ec_key_method_st EC_KEY_METHOD;
typedef struct ec_method_st EC_METHOD;
typedef struct ec_point_st EC_POINT;
typedef struct ec_group_st EC_GROUP;

typedef struct asn1_pctx_st ASN1_PCTX;


typedef struct evp_md_st EVP_MD;
typedef struct evp_md_ctx_st EVP_MD_CTX;
typedef struct evp_pkey_st EVP_PKEY;


typedef struct evp_pkey_method_st EVP_PKEY_METHOD;
typedef struct evp_pkey_ctx_st EVP_PKEY_CTX;

typedef struct evp_pkey_asn1_method_st EVP_PKEY_ASN1_METHOD;
typedef struct asn1_object_st ASN1_OBJECT;
typedef struct asn1_type_st ASN1_TYPE;

typedef struct x509_st X509;
typedef struct X509_pubkey_st X509_PUBKEY;
typedef struct X509_algor_st X509_ALGOR;
typedef struct x509_store_ctx_st X509_STORE_CTX;
typedef struct X509_POLICY_CACHE_st X509_POLICY_CACHE;
typedef struct X509_name_st X509_NAME;





typedef struct bio_st BIO;


typedef struct pkcs8_priv_key_info_st PKCS8_PRIV_KEY_INFO;

typedef struct ASN1_ITEM_st ASN1_ITEM;

/* ASN1_ITEM pointer exported type */
typedef const ASN1_ITEM ASN1_ITEM_EXP;

typedef struct ASN1_TEMPLATE_st ASN1_TEMPLATE;

typedef struct ui_st UI;
typedef struct ui_method_st UI_METHOD;

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;


typedef struct AUTHORITY_KEYID_st AUTHORITY_KEYID;

typedef struct NAME_CONSTRAINTS_st NAME_CONSTRAINTS;

typedef struct ctlog_st CTLOG;
typedef struct ctlog_store_st CTLOG_STORE;


#ifdef  __cplusplus
}
#endif


