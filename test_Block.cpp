#include <stdio.h>
#include <stdlib.h>
#include "ext.h"


int main(){
  //  printf("%d\n",sizeof(int));
  //printf("%d\n",sizeof(uint8_t));

  Block b1[3];
  Block b2[3];
  b1[0]=10;
  b1[1]=b1[0];
  printf("%d\n",b1[1].element[0]);
  printf("%d\n",b1[1].element[0]);
  printf("%d\n",sizeof(Block));
  b1[1]= 20;
  printf("%d\n",b1[1].element[0]);
  b1[2] = b1[1] ^ b1[0];
  printf("%d\n",(int)(b1[2]));
  memset(b2,0,3*sizeof(Block));
  printf("%d\n",b2[1].element[0]);
  memcpy(b2,b1,2*sizeof(Block));
  printf("%d\n",(int)b2[0]);
  return 0;

}
