
#ifndef ext_h
#define ext_h

#include <string.h>
#include <stdio.h>
#define BUFFERSIZE 5000
#define ELEMENTNUM 4
#define POS_MAP_LIMIT 1000000 // 2^22
#define TREES 50
#define STACH_LIMIT 1000000
#define Z 8
#define PAGESIZE 512
//#define ENCRYPT_LIB 3
//typedef long long Block;





typedef struct Bloc {

  Bloc(){memset(this,0,sizeof(struct Bloc));}
  struct Bloc& operator= (const struct Bloc& B){
    for(int i=0;i<ELEMENTNUM;i++)
      this->element[i] = B.element[i];return *this;
  }
    struct Bloc& operator^ (const struct Bloc& B){
      int v = this-> element[0];
      this->element[0] = v ^ B.element[0];return *this;
    }
  struct Bloc& operator= (const int i){this->element[0] = i;return *this;}
    operator int(){ return this->element[0];}
  struct Bloc& operator++ (int i){ this->element[0]+=1;return *this;}
  bool operator== (const int i){return this->element[0]==i;}
  int getPos(){return this ->element[1];}
  void setPos(int i){this->element[1]=i;}

  int getMap(){return this ->element[2];}
  void setMap(int i){this->element[2]=i;}
  //int getIV(){int i; i=this->element[ELEMENTNUM-1];return i;}
  //void setIV(int i){this->element[ELEMENTNUM-1]=i;}
  int element[ELEMENTNUM];
} Block;



typedef struct tr_n {
  Block value[Z];
  tr_n *left;
  tr_n *right;
  tr_n *parent;
 
  //int temp;
  long long IV;
}tree_node;


struct vect {
  int index;
  int value;
  int position;
  int some_thing_to_make_16;
};

typedef struct tr_n_aux {
  vect value[Z];
  tr_n_aux *left;
  tr_n_aux *right;
  tr_n_aux *parent;
  
  //int temp;
  long long IV;
}tree_node_aux;




#endif
