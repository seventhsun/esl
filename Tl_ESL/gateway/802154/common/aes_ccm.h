#pragma  once

#define AES_BLOCK_SIZE     16
#define CCM_ASTR_MAX_SIZE  64

enum {
    AES_SUCC = 0x00,
    AES_NO_BUF,               
    AES_FAIL,
};


struct CCM_FLAGS_TAG {
    union {
        struct {
            unsigned char L : 3;
            unsigned char M : 3;
            unsigned char aData :1;
            unsigned char reserved :1;            
        } bf;
        unsigned char val;
    };
};

typedef struct CCM_FLAGS_TAG ccm_flags_t;

enum AES_OPT {
    AES_ENCRYPTION = 0,
    AES_DECRYPTION,
};

typedef struct {
    union {
        unsigned char A[AES_BLOCK_SIZE];
        unsigned char B[AES_BLOCK_SIZE];
    } bf;
    
    unsigned char tmpResult[AES_BLOCK_SIZE];
    unsigned char newAstr[CCM_ASTR_MAX_SIZE];
} aes_enc_t;

unsigned char aes_ccmAuthTran(unsigned char M, unsigned char *key, unsigned char *iv, unsigned char *mStr, unsigned short mStrLen, unsigned char *aStr, unsigned char aStrLen, unsigned char *result);
unsigned char aes_ccmEncTran(unsigned char M, unsigned char *key, unsigned char *iv, unsigned char *mStr, unsigned short mStrLen, unsigned char *aStr, unsigned char aStrLen, unsigned char *result);
unsigned char aes_ccmDecTran(unsigned char micLen, unsigned char *key, unsigned char *iv, unsigned char *mStr, unsigned short mStrLen, unsigned char *aStr, unsigned char aStrLen, unsigned char *mic);
unsigned char aes_ccmDecAuthTran(unsigned char micLen, unsigned char *key, unsigned char *iv, unsigned char *mStr, unsigned short mStrLen, unsigned char *aStr, unsigned char aStrLen, unsigned char *mic);
