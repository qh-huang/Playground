#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>
#include <openssl/pem.h>

// I'm not using BIO for base64 encoding/decoding.  It is difficult to use.
// Using superwills' Nibble And A Half instead 
// https://github.com/superwills/NibbleAndAHalf/blob/master/NibbleAndAHalf/base64.h
//#include "base64.h"
#include <stdio.h>
#include <stdlib.h>

const static char* b64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;

// maps A=>0,B=>1..
const static unsigned char unb64[]={
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //10 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //20 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //30 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //40 
  0,   0,   0,  62,   0,   0,   0,  63,  52,  53, //50 
 54,  55,  56,  57,  58,  59,  60,  61,   0,   0, //60 
  0,   0,   0,   0,   0,   0,   1,   2,   3,   4, //70 
  5,   6,   7,   8,   9,  10,  11,  12,  13,  14, //80 
 15,  16,  17,  18,  19,  20,  21,  22,  23,  24, //90 
 25,   0,   0,   0,   0,   0,   0,  26,  27,  28, //100 
 29,  30,  31,  32,  33,  34,  35,  36,  37,  38, //110 
 39,  40,  41,  42,  43,  44,  45,  46,  47,  48, //120 
 49,  50,  51,   0,   0,   0,   0,   0,   0,   0, //130 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //140 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //150 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //160 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //170 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //180 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //190 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //200 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //210 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //220 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //230 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //240 
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0, //250 
  0,   0,   0,   0,   0,   0, 
}; // This array has 256 elements

// Converts binary data of length=len to base64 characters.
// Length of the resultant string is stored in flen
// (you must pass pointer flen).
char* base64( const void* binaryData, int len, int *flen )
{
  const unsigned char* bin = (const unsigned char*) binaryData ;
  char* res ;
  
  int rc = 0 ; // result counter
  int byteNo ; // I need this after the loop
  
  int modulusLen = len % 3 ;
  int pad = ((modulusLen&1)<<1) + ((modulusLen&2)>>1) ; // 2 gives 1 and 1 gives 2, but 0 gives 0.
  
  *flen = 4*(len + pad)/3 ;
  res = (char*) malloc( *flen + 1 ) ; // and one for the null
  if( !res )
  {
    puts( "ERROR: base64 could not allocate enough memory." ) ;
    puts( "I must stop because I could not get enough" ) ;
    return 0;
  }
  
  for( byteNo = 0 ; byteNo <= len-3 ; byteNo+=3 )
  {
    unsigned char BYTE0=bin[byteNo];
    unsigned char BYTE1=bin[byteNo+1];
    unsigned char BYTE2=bin[byteNo+2];
    res[rc++]  = b64[ BYTE0 >> 2 ] ;
    res[rc++]  = b64[ ((0x3&BYTE0)<<4) + (BYTE1 >> 4) ] ;
    res[rc++]  = b64[ ((0x0f&BYTE1)<<2) + (BYTE2>>6) ] ;
    res[rc++]  = b64[ 0x3f&BYTE2 ] ;
  }
  
  if( pad==2 )
  {
    res[rc++] = b64[ bin[byteNo] >> 2 ] ;
    res[rc++] = b64[ (0x3&bin[byteNo])<<4 ] ;
    res[rc++] = '=';
    res[rc++] = '=';
  }
  else if( pad==1 )
  {
    res[rc++]  = b64[ bin[byteNo] >> 2 ] ;
    res[rc++]  = b64[ ((0x3&bin[byteNo])<<4)   +   (bin[byteNo+1] >> 4) ] ;
    res[rc++]  = b64[ (0x0f&bin[byteNo+1])<<2 ] ;
    res[rc++] = '=';
  }
  
  res[rc]=0; // NULL TERMINATOR! ;)
  return res ;
}

unsigned char* unbase64( const char* ascii, int len, int *flen )
{
  const unsigned char *safeAsciiPtr = (const unsigned char*)ascii ;
  unsigned char *bin ;
  int cb=0;
  int charNo;
  int pad = 0 ;

  if( len < 2 ) { // 2 accesses below would be OOB.
    // catch empty string, return NULL as result.
    puts( "ERROR: You passed an invalid base64 string (too short). You get NULL back." ) ;
    *flen=0;
    return 0 ;
  }
  if( safeAsciiPtr[ len-1 ]=='=' )  ++pad ;
  if( safeAsciiPtr[ len-2 ]=='=' )  ++pad ;
  
  *flen = 3*len/4 - pad ;
  bin = (unsigned char*)malloc( *flen ) ;
  if( !bin )
  {
    puts( "ERROR: unbase64 could not allocate enough memory." ) ;
    puts( "I must stop because I could not get enough" ) ;
    return 0;
  }
  
  for( charNo=0; charNo <= len - 4 - pad ; charNo+=4 )
  {
    int A=unb64[safeAsciiPtr[charNo]];
    int B=unb64[safeAsciiPtr[charNo+1]];
    int C=unb64[safeAsciiPtr[charNo+2]];
    int D=unb64[safeAsciiPtr[charNo+3]];
    
    bin[cb++] = (A<<2) | (B>>4) ;
    bin[cb++] = (B<<4) | (C>>2) ;
    bin[cb++] = (C<<6) | (D) ;
  }
  
  if( pad==1 )
  {
    int A=unb64[safeAsciiPtr[charNo]];
    int B=unb64[safeAsciiPtr[charNo+1]];
    int C=unb64[safeAsciiPtr[charNo+2]];
    
    bin[cb++] = (A<<2) | (B>>4) ;
    bin[cb++] = (B<<4) | (C>>2) ;
  }
  else if( pad==2 )
  {
    int A=unb64[safeAsciiPtr[charNo]];
    int B=unb64[safeAsciiPtr[charNo+1]];
    
    bin[cb++] = (A<<2) | (B>>4) ;
  }
  
  return bin ;
}

// The PADDING parameter means RSA will pad your data for you
// if it is not exactly the right size
//#define PADDING RSA_PKCS1_OAEP_PADDING
#define PADDING RSA_PKCS1_PADDING
//#define PADDING RSA_NO_PADDING

RSA* loadPUBLICKeyFromString( const char* publicKeyStr )
{
  // A BIO is an I/O abstraction (Byte I/O?)
  
  // BIO_new_mem_buf: Create a read-only bio buf with data
  // in string passed. -1 means string is null terminated,
  // so BIO_new_mem_buf can find the dataLen itself.
  // Since BIO_new_mem_buf will be READ ONLY, it's fine that publicKeyStr is const.
  BIO* bio = BIO_new_mem_buf( (void*)publicKeyStr, -1 ) ; // -1: assume string is null terminated
  
  BIO_set_flags( bio, BIO_FLAGS_BASE64_NO_NL ) ; // NO NL
  
  // Load the RSA key from the BIO
  RSA* rsaPubKey = PEM_read_bio_RSA_PUBKEY( bio, NULL, NULL, NULL ) ;
  if( !rsaPubKey )
    printf( "ERROR: Could not load PUBLIC KEY!  PEM_read_bio_RSA_PUBKEY FAILED: %s\n", ERR_error_string( ERR_get_error(), NULL ) ) ;
  
  BIO_free( bio ) ;
  return rsaPubKey ;
}

RSA* loadPRIVATEKeyFromString( const char* privateKeyStr )
{
  BIO *bio = BIO_new_mem_buf( (void*)privateKeyStr, -1 );
  //BIO_set_flags( bio, BIO_FLAGS_BASE64_NO_NL ) ; // NO NL
  RSA* rsaPrivKey = PEM_read_bio_RSAPrivateKey( bio, NULL, NULL, NULL ) ;
  
  if ( !rsaPrivKey )
    printf("ERROR: Could not load PRIVATE KEY!  PEM_read_bio_RSAPrivateKey FAILED: %s\n", ERR_error_string(ERR_get_error(), NULL));
  
  BIO_free( bio ) ;
  return rsaPrivKey ;
}

unsigned char* rsaEncrypt( RSA *pubKey, const unsigned char* str, int dataSize, int *resultLen )
{
  int rsaLen = RSA_size( pubKey ) ;
  unsigned char* ed = (unsigned char*)malloc( rsaLen ) ;
  
  // RSA_public_encrypt() returns the size of the encrypted data
  // (i.e., RSA_size(rsa)). RSA_private_decrypt() 
  // returns the size of the recovered plaintext.
  *resultLen = RSA_public_encrypt( dataSize, (const unsigned char*)str, ed, pubKey, PADDING ) ; 
  if( *resultLen == -1 )
    printf("ERROR: RSA_public_encrypt: %s\n", ERR_error_string(ERR_get_error(), NULL));

  return ed ;
}

unsigned char* rsaDecrypt( RSA *privKey, const unsigned char* encryptedData, int *resultLen )
{
  int rsaLen = RSA_size( privKey ) ; // That's how many bytes the decrypted data would be
  
  unsigned char *decryptedBin = (unsigned char*)malloc( rsaLen ) ;
  *resultLen = RSA_private_decrypt( RSA_size(privKey), encryptedData, decryptedBin, privKey, PADDING ) ;
  if( *resultLen == -1 )
    printf( "ERROR: RSA_private_decrypt: %s\n", ERR_error_string(ERR_get_error(), NULL) ) ;
    
  return decryptedBin ;
}

unsigned char* makeAlphaString( int dataSize )
{
  unsigned char* s = (unsigned char*) malloc( dataSize ) ;
  
  int i;
  for( i = 0 ; i < dataSize ; i++ )
    s[i] = 65 + i ;
  s[i-1]=0;//NULL TERMINATOR ;)
  
  return s ;
}

// You may need to encrypt several blocks of binary data (each has a maximum size
// limited by pubKey).  You shoudn't try to encrypt more than
// RSA_LEN( pubKey ) bytes into some packet.
// returns base64( rsa encrypt( <<binary data>> ) )
// base64OfRsaEncrypted()
// base64StringOfRSAEncrypted
// rsaEncryptThenBase64
char* rsaEncryptThenBase64( RSA *pubKey, unsigned char* binaryData, int binaryDataLen, int *outLen )
{
  int encryptedDataLen ;
  
  // RSA encryption with public key
  unsigned char* encrypted = rsaEncrypt( pubKey, binaryData, binaryDataLen, &encryptedDataLen ) ;
  
  // To base 64
  int asciiBase64EncLen ;
  char* asciiBase64Enc = base64( encrypted, encryptedDataLen, &asciiBase64EncLen ) ;
  
  // Destroy the encrypted data (we are using the base64 version of it)
  free( encrypted ) ;
  
  // Return the base64 version of the encrypted data
  return asciiBase64Enc ;
}

// rsaDecryptOfUnbase64()
// rsaDecryptBase64String()
// unbase64ThenRSADecrypt()
// rsaDecryptThisBase64()
unsigned char* rsaDecryptThisBase64( RSA *privKey, char* base64String, int *outLen )
{
  int encBinLen ;
  unsigned char* encBin = unbase64( base64String, (int)strlen( base64String ), &encBinLen ) ;
  
  // rsaDecrypt assumes length of encBin based on privKey
  unsigned char *decryptedBin = rsaDecrypt( privKey, encBin, outLen ) ;
  free( encBin ) ;
  
  return decryptedBin ;
}
  


int main( int argc, const char* argv[] )
{
  ERR_load_crypto_strings();  
  
  puts( "We are going to: rsa_decrypt( unbase64( base64( rsa_encrypt( <<binary data>> ) ) ) )" );
  // public key
  // http://srdevspot.blogspot.ca/2011/08/openssl-error0906d064pem.html
  //1. The file must contain:
  //-----BEGIN CERTIFICATE-----
  //on a separate line (i.e. it must be terminated with a newline).
  //2. Each line of "gibberish" must be 64 characters wide.
  //3. The file must end with:
  //-----END CERTIFICATE-----
  // YOUR PUBLIC KEY MUST CONTAIN NEWLINES.  If it doesn't (ie if you generated it with
  // something like
  // ssh-keygen -t rsa -C "you@example.com"
  // ) THEN YOU MUST INSERT NEWLINES EVERY 64 CHRS (just line it up with how I have it here
  // or with how the ssh-keygen private key is formatted by default)
  const char *b64_pKey = "-----BEGIN PUBLIC KEY-----\n"
  "MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQCp2w+8HUdECo8V5yuKYrWJmUbL\n"
  "tD6nSyVifN543axXvNSFzQfWNOGVkMsCo6W4hpl5eHv1p9Hqdcf/ZYQDWCK726u6\n"
  "hsZA81AblAOOXKaUaxvFC+ZKRJf+MtUGnv0v7CrGoblm1mMC/OQI1JfSsYi68Epn\n"
  "aOLepTZw+GLTnusQgwIDAQAB\n"
  "-----END PUBLIC KEY-----\n";
  
  // private key
  const char *b64priv_key = "-----BEGIN RSA PRIVATE KEY-----\n"
  "MIICXAIBAAKBgQCp2w+8HUdECo8V5yuKYrWJmUbLtD6nSyVifN543axXvNSFzQfW\n"
  "NOGVkMsCo6W4hpl5eHv1p9Hqdcf/ZYQDWCK726u6hsZA81AblAOOXKaUaxvFC+ZK\n"
  "RJf+MtUGnv0v7CrGoblm1mMC/OQI1JfSsYi68EpnaOLepTZw+GLTnusQgwIDAQAB\n"
  "AoGBAKDuq3PikblH/9YS11AgwjwC++7ZcltzeZJdGTSPY1El2n6Dip9ML0hUjeSM\n"
  "ROIWtac/nsNcJCnvOnUjK/c3NIAaGJcfRPiH/S0Ga6ROiDfFj2UXAmk/v4wRRUzr\n"
  "5lsA0jgEt5qcq2Xr/JPQVGB4wUgL/yQK0dDhW0EdrJ707e3BAkEA1aIHbmcVfCP8\n"
  "Y/uWuK0lvWxrIWfR5MlHhI8tD9lvkot2kyXiV+jB6/gktwk1QaFsy7dCXn7w03+k\n"
  "xrjEGGN+kQJBAMuKf55lDtU9K2Js3YSStTZAXP+Hz7XpoLxmbWFyGvBx806WjgAD\n"
  "624irwS+0tBxkERbRcisfb2cXmAx8earT9MCQDZuVCpjBWxd1t66qYpgQ29iAmG+\n"
  "jBIY3qn9uOOC6RSTiCCx1FvFqDMxRFmGdRVFxeyZwsVE3qNksF0Zko0MPKECQCEe\n"
  "oDV97DP2iCCz5je0R5hUUM2jo8DOC0GcyR+aGZgWcqjPBrwp5x08t43mHxeb4wW8\n"
  "dFZ6+trnntO4TMxkA9ECQB+yCPgO1zisJWYuD46KISoesYhwHe5C1BQElQgi9bio\n"
  "U39fFo88w1pok23a2CZBEXguSvCvexeB68OggdDXvy0=\n"
  "-----END RSA PRIVATE KEY-----\n";

  const char* eavpriv_key = "-----BEGIN RSA PRIVATE KEY-----\n"
  "MIICdAIBADANBgkqhkiG9w0BAQEFAASCAl4wggJaAgEAAoGBAMFFU2p/iSbAmx5V\n"
  "xQFoVPY2rzHvn2OIJbflFqaIvwndiOuVzkLMRBzxg3WueWKWmBREHPPymZQHgx8B\n"
  "0O0xoF6VfLQd9a41++Nz5DTKJVNJIPokbF7UznGQgH7IvHUMvl0+rcqToOO7y9KU\n"
  "Ly/dp2IhAN1sgHCdwsfxs8ih6UdnAgMBAAECgYANDWuGw7o1fch8EfrOD4e9YRAP\n"
  "5Uj0Zdt2Qeh3ssCEIAwBLQrTBAyNj9n69p+IAveeVsNFzlqE3sUCMRM1Eabs3hVB\n"
  "7/P/WafZWrm0JZ1c6MkuK4yV9I0juwEYLx8fMSuwqCGvOnyVdR+8M2/o7cnmNAG7\n"
  "FlfBgZbcQaoiDv+4kQJBAPqrURowSJHsPDqww+46MzBM3CJPujjp4grWmq59i+j6\n"
  "f4aAHgGEDmcPY/UU6PUhluW7X5awvxVsclqS2TyB6hsCQQDFYYXhTVyFrwB+x3X9\n"
  "CAPcyRKDg6B+j/txfNKfUWbAWEspwap6hlY3Ky1PZIW9cRZt09SXS5h3xhcXvgtw\n"
  "3mylAkBRt8nfxL/TunTEGV42aojHhTb6hI1MAosrR1zz53/V+tvaCChADEH5vR20\n"
  "BxYO291qlVQLNyGEJW9+MYNaY2vjAkALQHv5VAeqN0e38nbvbb2KoIvFo4NKs7qJ\n"
  "xavcGS/aVy4xuzCUu4O+sV9r3zcgjYnRyPmdhlmyukAmwla0oHlhAj96U73bpEiN\n"
  "pda5mZmzmv3mSDPCwAClGTtvjPE/1c0ujSXWyTE9/f33NczZVia8nfnLErTQLtp/\n"
  "W601dt1cJQA=\n"
  "-----END RSA PRIVATE KEY-----\n";

  // String to encrypt, INCLUDING NULL TERMINATOR:
  int dataSize=37 ; // 128 for NO PADDING, __ANY SIZE UNDER 128 B__ for RSA_PKCS1_PADDING
  unsigned char *str = makeAlphaString( dataSize ) ;
  printf( "\nThe original data is:\n%s\n\n", (char*)str ) ;
  
  // LOAD PUBLIC KEY
  RSA *pubKey = loadPUBLICKeyFromString( b64_pKey ) ;
  
  int asciiB64ELen ;
  char* asciiB64E = rsaEncryptThenBase64( pubKey, str, dataSize, &asciiB64ELen ) ;
   
  RSA_free( pubKey ) ; // free the public key when you are done all your encryption
  
  printf( "Sending base64_encoded ( rsa_encrypted ( <<binary data>> ) ):\n%s\n", asciiB64E ) ;
  puts( "<<----------------  SENDING DATA ACROSS INTERWEBS  ---------------->>" ) ;
  
  char* rxOverHTTP = asciiB64E ; // Simulate Internet connection by a pointer reference
  printf( "\nRECEIVED some base64 string:\n%s\n", rxOverHTTP ) ;
  puts( "\n * * * What could it be?" ) ;
  
  // Now decrypt this very string with the private key
  RSA *privKey = loadPRIVATEKeyFromString( b64priv_key ) ;
  
  // Now we got the data at the server.  Time to decrypt it.
  int rBinLen ;
  unsigned char* rBin = rsaDecryptThisBase64( privKey, rxOverHTTP, &rBinLen ) ;
  printf("Decrypted %d bytes, the recovered data is:\n%.*s\n\n", rBinLen, rBinLen, rBin ) ; // rBin is not necessarily NULL
  // terminated, so we only print rBinLen chrs

  RSA *eavprivKey = loadPRIVATEKeyFromString( eavpriv_key ) ;
  char* eav_encrypt = "So0NH7b2uJZ7KApmBq98CzV7MSUu1XmleaDIqYC1OzUfih5gCvsvd+jChSEiqal8E69ghl+MhQ6+yoOqqwwQ7JtvQqPJBWLE4aOdjO4codrmhH9zzgdwKuQHR7BUwjJSBjyZ85QeV51/ipY3gyqNnMSnsVfKxVv9xAZTE+fP5pg=";
  unsigned char* eavrBin = rsaDecryptThisBase64(eavprivKey, eav_encrypt, &rBinLen);  
  printf("Decrypted %d bytes, the recovered data is:\n%.*s\n\n", rBinLen, rBinLen, eavrBin) ; // rBin is not necessarily NULL

  RSA_free(privKey) ;
  RSA_free(eavprivKey) ;
  
  bool allEq = true ;
  for( int i = 0 ; i < dataSize ; i++ )
    allEq &= (str[i] == rBin[i]) ;
  
  if( allEq ) puts( "DATA TRANSFERRED INTACT!" ) ;
  else puts( "ERROR, recovered binary does not match sent binary" ) ;
  free( str ) ; 
  free( asciiB64E ) ; // rxOverHTTP  
  free( rBin ) ;
  ERR_free_strings();
}
