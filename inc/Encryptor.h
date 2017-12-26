#ifndef _Encryptor_h
#define _Encryptor_h

//#include "../ext.h"

#if ENCRYPT_LIB==2 
  #include "ippcp.h"
  #include "ippcore.h"
#elif ENCRYPT_LIB==3
  #include "wolfssl/wolfcrypt/aes.h"
#endif






long long arr_256[] = {256,65536,16777216
		     ,4294967296, 1099511627776,281474976710656};




#if ENCRYPT_LIB==3

void incr_pIV_wolf(byte pIV [],long long IV_counter){

  long long num = IV_counter;
  for(int i=5;i>=0;i--){
    pIV[i+1]= num / arr_256[i];
    num = num % arr_256[i];
  }

  pIV[0]=num;
}


void encrypt_wolfssl(char message[], int size,int op,long long IV_counter){
  Aes enc;
  int ret = 0;

    // initialize enc with AesSetKey, using direction AES_ENCRYPTION
  byte wkey[] =  "\x00\x01\x02\x03\x04\x05\x06\x07"
    "\x08\x09\x10\x11\x12\x13\x14\x15";


  byte iv [] =    "\x00\x00\x01\x02\x03\x04\x05\x06\x07"
    "\x08\x09\x10\x11\x12\x13\x14\x15";


  incr_pIV_wolf(iv,IV_counter);

  
  char cipher[size]; // Some multiple of 16 bytes
  
  if (op==1){
    if (ret = wc_AesSetKey(&enc, wkey, 16, iv, AES_ENCRYPTION) != 0) {
        // failed to set aes key
      //printf("Failed to set key\n");
    }
    if ((ret = wc_AesCbcEncrypt(&enc, (byte*)cipher, (byte*)message, size)) != 0 ) {
    // block align error
      //printf("Fail to encryt CBC\n");      
    }
  }else {
    if (ret = wc_AesSetKey(&enc, wkey, 16, iv, AES_DECRYPTION) != 0) {
      // failed to set aes key
      //printf("Failed to set key\n");
    }
    
    if ((ret = wc_AesCbcDecrypt(&enc, (byte*)cipher, (byte*)message, size)) != 0 ) {
    // block align error
      //printf("Fail to deccryt CBC\n");      
    }
  }

  memcpy(message,cipher,size);
}

#endif


#if ENCRYPT_LIB==2

void incr_pIV_ipp( Ipp8u pIV [],long long IV_counter){

  long long num = IV_counter;
  for(int i=5;i>=0;i--){
    pIV[i+1]= num / arr_256[i];
    num = num % arr_256[i];
  }

  pIV[0]=num;
}


void encrypt_ipp(char message[], int size,int op,long long IV_counter){
  // op=1 is encryption, op=2 is decryption

  Ipp8u key[] = "\x00\x01\x02\x03\x04\x05\x06\x07"
    "\x08\x09\x10\x11\x12\x13\x14\x15";
  // define and setup AES cipher
  int ctxSize;
  ippsAESGetSize(&ctxSize);
  IppsAESSpec* pCtx = (IppsAESSpec*)( new Ipp8u [ctxSize] );  
  ippsAESInit(key, sizeof(key)-1, pCtx, ctxSize);  



  
  Ipp8u pIV[] = "\xff\xee\xdd\xcc\xbb\xaa\x99\x88"
    "\x77\x66\x55\x44\x33\x22\x11\x00"; 

  incr_pIV_ipp(pIV,IV_counter);

  
  Ipp8u* pSrc= (Ipp8u*)message;
  int msgsize = size;
  Ipp8u* pDst = (Ipp8u*) malloc(msgsize);
  
  
  if (op==1){
    if (ippsAESEncryptCBC(pSrc,pDst,msgsize,pCtx,pIV)!=ippStsNoErr){
      //printf("Error Encrypting client\n");
      //exit(0);
    }
  }else{
    if (ippsAESDecryptCBC(pSrc,pDst,msgsize,pCtx,pIV)!=ippStsNoErr){
      //printf("Error Decrypting client\n");
      //exit(0);
    }
  }
  
  memcpy(message,pDst,msgsize);
}

#endif




void Encrypt(char message[], int size,int op,long long IV_counter){

  #if ENCRYPT_LIB==2
    encrypt_ipp(message,size,op,IV_counter);
  #elif ENCRYPT_LIB==3
    encrypt_wolfssl(message,size,op,IV_counter);
  #endif
}





#endif 
