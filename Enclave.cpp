#include "Enclave_t.h"

#include "sgx_trts.h"
#include "sgx_tcrypto.h"
#include "ext.h"
#include "Tree.h"
#include "Encryptor.h"

#include <string.h>
//#define ARRSIZE 1000

void checkContent(Block message[],int size, char * mess);
void encrypt_node(tree_node *n);
void decrypt_node(tree_node *n);
void encrypt_node_aux(tree_node_aux *n);
void decrypt_node_aux(tree_node_aux *n);
void read_path(int x);
void write_path(int x);
void read_path_aux(int x,int treeID);
void write_path_aux(int x,int treeID);
void encrypt_tree(tree_node *root);
void encrypt_tree_aux(tree_node_aux *root);
void do_enclave_work();
void encrypt(Block message[], int size);
void printf(const char *fmt, ...);
void path_level(int leaf,int level,int *min,int *max);
void random_array(int arr[],int size);
int random_item(int size);
int get_position_map(int i,bool obliv);
void set_position_map(int i,int newval,bool obliv);
int initialize_pos_recursive(int i,int level,int pos);
void obliv_write_int(int *Arr,int i,int size,int new_val);
int obliv_read_int(int *Arr,int i,int size);
int *  MyMallocInt(int size,char **ptr);
void MyFree(char *ptr);
int find_pos_recursive(int i,int treeID,int pos,int final,int write_val);
void add_to_stach(int treeID,vect b);
void add_to_stach_item(Block v);


Block *A=NULL;
sgx_aes_ctr_128bit_key_t key;
InitVectorStr initVector;
Block com_key;
Block clientmessage[3];
int ARRSIZE;
int Mid_Enc_Com,SizeSoFarEnc,SizeSoFarDec,initcounter,currclientmessageindex;
bool enteredfirsttime=false;
bool clientmessagemodified=false;
tree_node *tr0;
tree_node_aux **tr_aux;
int *positionmap;
tree_node **links;
tree_node_aux ***links_aux;
Block *stach;
vect *stach_aux[TREES];
int stach_counter=0;
int stach_counter_aux[TREES];
int stach_max=0;
int stach_max_aux[TREES];
int stach_sizes[TREES];
Block Bluff[2];
int tree_sizes[TREES];
int trees_count=0;
int Obliv_int[2];
int inc_int;
char *real_pos_map_ptr;
int finalpos;


void initialize(int arrsize,int trees_count_aux){

  for(int i=0;i<16;i++){
    key[i] = random_item(256);
  }
  

  
  ARRSIZE = arrsize;
  inc_int =  PAGESIZE/sizeof(int);
  printf("Count of Auxilary trees: %d\n",trees_count_aux);
  trees_count=trees_count_aux;

  //Calculate tree sizes;
  int v = find_closet_power_of_two(ARRSIZE);
  v/=2;
  for(int i=1;i<=trees_count;i++){
    tree_sizes[i]=v;
    printf("Tree %d : %d\n",i,tree_sizes[i]);
    v/=2;
  }

  stach_sizes[0]=STACH_LIMIT;
  stach = (Block*) malloc(sizeof(Block)*stach_sizes[0]);
  
  if (trees_count>=1){
    //stach_aux[1]=(vect*)malloc(sizeof(vect)*STACH_LIMIT);
    for(int i=1;i<=trees_count;i++){
      stach_sizes[i] = STACH_LIMIT;
      stach_aux[i]=(vect*)malloc(sizeof(vect)*(stach_sizes[i]));
    }
  }
			

  
  if (trees_count_aux==0){ // if no auxillary trees, just use the normal size
    positionmap = MyMallocInt(arrsize*sizeof(int),&real_pos_map_ptr);
   
    if (positionmap==NULL){
      printf("Malloc fail inside the enclave.\n");
    }

    //for(int i=0;i<ARRSIZE;i++)
    //obliv_write_int(positionmap,i,ARRSIZE,random_item(ARRSIZE));
    // printf("created...\n");
    random_array(positionmap,ARRSIZE);
  }else {
    positionmap = MyMallocInt(tree_sizes[trees_count_aux]*sizeof(int),&real_pos_map_ptr);
    if (positionmap==NULL){
      printf("Malloc fail inside the enclave.\n");
    }
    printf("size of position map : %d\n",tree_sizes[trees_count_aux]);
    random_array(positionmap,tree_sizes[trees_count_aux]);
  }

  encrypt_tree(tr0);
  //printf("%d\n",tr_aux[1]==NULL);

  for(int i=1;i<=trees_count_aux;i++)
    encrypt_tree_aux(tr_aux[i]);



  //printf("%d %d %d\n",tr_aux[1]->left->temp,tr_aux[1]->right->temp,(int)tr_aux[1]->value[0].value);
  //printf("%d %d %d %d\n",tr0->left->temp,tr0->right->temp,(int)tr0->value[0],links[0]->temp);

  if (trees_count>0){
    printf("Creating position map using auxillary trees..\n");
    for(int i=0;i<tree_sizes[trees_count];i++){
      //printf("processing %d -------------- \n",i); 
      positionmap[i]= initialize_pos_recursive(i,trees_count,positionmap[i]);   
      //printf("processed %d %d -------------- \n",i,positionmap[i]); 
    }
    printf("Position map is ready\n");
  }

 
  //return;
  
  //Start : initialize elements
  printf("Initializing elements...\n");
  for(int i=0;i<ARRSIZE;i++){
    Block z;
    z=i;
    z.setPos(i);
    //printf("processing %d counter %d\n",i,stach_counter);
    int newval = random_item(ARRSIZE);
    z.setMap(newval);
    add_to_stach_item(z);
    int x=random_item(ARRSIZE);//get_position_map(i,false);
    //printf("Position map for final i %d : %d\n",i,x);
   
    set_position_map(i,newval,false);
    read_path(x);
    write_path(x);
    //printf("Done i=%d-------------------------\n",i);
  }
  //End 


  for(int i=1;i<=trees_count;i++)
    printf("Maximum stach tree %d  :%d\n",i,stach_max_aux[i]);

  printf("Initialization is done\n");
  printf("Maximum stach :%d\n",stach_max);
  com_key=10000;
  

}

void passAddress(void *input,char c,size_t len){
  if (c=='T'){
    tr0 = *((tree_node**)input);
    //printf("tr %d\n",tr0->temp);
  }
  else if (c=='L'){
    links = *((tree_node***)input);
    //printf("links %d\n",links[0]->temp);
  }else if (c=='X'){
    tr_aux = *((tree_node_aux***)input);
    //printf("tr_aux %d\n",tr_aux[1]->temp);
  }else if (c=='A'){
    links_aux = *((tree_node_aux****)input);
    //printf("links aux %d\n",links_aux[1][0]->temp);
  }
}




void passMessageFromClient(Block *input,size_t len){
  memcpy(clientmessage,input,3*sizeof(Block));
  /*
  for(int i=0;i<3;i++){
      clientmessage[i]=input[i];
      }*/
  do_enclave_work();
}



void getMessageToClient(Block *input,size_t len){
  memcpy(input,clientmessage,3*sizeof(Block));
  /* for(int i=0;i<3;i++){
    input[i]=clientmessage[i];
    }*/
}


void do_enclave_work(){
  //register Block r1;
  //register Block r2;
  

  //encrypt(clientmessage,3);  // decrypt message
  Encrypt(clientmessage,3,2,com_key);

  
  int index = (int)clientmessage[1];
  Block newvalue;
  
  int x = get_position_map(index,true);
  int newval = random_item(ARRSIZE);
  set_position_map(index,newval,true);
  read_path(x);

  for(int i=0;i<stach_counter;i++){
    //printf("stach[i] %d\n",stach[i].getPos());
    if (stach[i].getPos() == index){
      stach[i].setMap(newval);
      if ((int)clientmessage[0]==2){
	clientmessage[0]=4;
	stach[i] = (int)clientmessage[2];
      }else{
	clientmessage[0]=3;
	clientmessage[2] = stach[i];
      }
    }
  }

  write_path(x);
    
  //encrypt(clientmessage,3);
  Encrypt(clientmessage,3,1,com_key);
  com_key++; // decrypt then encrypt then change

  Mid_Enc_Com++;
}

void path_level(int leaf,int level,int *min,int *max){
  int power_of_2=1;
  for(int i=0;i<level;i++)
    power_of_2 *=2;
  *min = leaf/power_of_2 * power_of_2;
  *max = *min + power_of_2 -1;
}


void write_path(int x){

  tree_node *t = links[x];
  int level=1;
  tree_node temp;
  while (1){
    int min,max;
    int node_counter=0;
    //printf("writing path for leaf %d, node %d\n",x,t->temp);
    path_level(x,level-1,&min,&max);
    for(int i=0;i<Z;i++) temp.value[i]=-1;
    //printf("Write:Calculate min %d and max %d and prepare temp\n",min,max);

    
    for(int i=0;i<stach_counter;i++){
      int y= stach[i].getPos();
      int y_pos = stach[i].getMap();//get_position_map(y,true);
      //printf("Write:inside loop i %d counter %d y=%d y_pos=%d level %d\n",
      //  i,stach_counter,y,y_pos,level);
      if (y_pos>=min && y_pos<=max){
	temp.value[node_counter++]=stach[i];
	//printf("removing %d frm stach at level %d\n",stach[i].getPos(),level);
	stach[i]=stach[stach_counter-1];   // move last one to the one
	i--;
	stach_counter--;
      }
      if (node_counter==Z) break;
    }

    encrypt_node(&temp);
    memcpy(t->value, temp.value,sizeof(Block)*Z);
    t->IV=temp.IV;
    level++;
    if (t->parent==NULL)break;
    t = t->parent;
    //printf("Stach count %d\n",stach_counter);
  }
}


void read_path(int x){
  //printf("Entering read_path %d\n",i);
  //printf("x : %d\n",x);
  
  tree_node t = *links[x];
  int level=1;
  
  while (1){
    //printf("%d->",t.temp);
    decrypt_node (&t);
    for(int i=0;i<Z;i++){
      if (t.value[i]!=-1){
	add_to_stach_item(t.value[i]);
	//printf("Read:storing %d in stach at level %d\n",stach[stach_counter-1].getPos(),level);
      }
    }
    if (t.parent==NULL) break;
    t = *(t.parent);
    level++;
  }

  //printf("%d\n",stach_counter);
  /*
    printf("%d %d \n",t.IV,(int)t.value[0]);
    decrypt_node (&t);
    printf("%d %d \n",t.IV,(int)t.value[0]);
  */
}

void encrypt_tree(tree_node *root){
  if (root==NULL)
    return;
  else {
    for (int i=0;i<Z;i++)  // dummitize a node
      root->value[i]=-1;
    encrypt_node(root);
    encrypt_tree(root->left);
    encrypt_tree(root->right);   
  }
}

void encrypt_tree_aux(tree_node_aux *root){
  if (root==NULL)
    return;
  else {
    for (int i=0;i<Z;i++)  // dummitize a node
      root->value[i].value=-1;
    encrypt_node_aux(root);
    encrypt_tree_aux(root->left);
    encrypt_tree_aux(root->right);   
  }
}

void encrypt_node_aux(tree_node_aux *n){
  const sgx_aes_ctr_128bit_key_t *p_key= &key;  //key 
  const vect *p_src = n->value;  // input array
  const int src_len= sizeof(vect)*Z; //size of array
  uint8_t *p_ctr;   // initialization vector
  const int ctr_inc_bits=8;
  vect p_dst[Z];


  InitVectorStr s= initVector;
  p_ctr = (uint8_t*)&s;
  
  sgx_aes_ctr_encrypt(p_key,(uint8_t*)p_src,src_len,p_ctr,ctr_inc_bits,(uint8_t*)p_dst);
  n->IV= (int)initVector;
  initVector = initVector+1;
  
  memcpy(n->value,p_dst,sizeof(vect)*Z);
  //printf("Encode with IV %d\n",n->IV);
  
  //free(p_dst);
}

void decrypt_node_aux(tree_node_aux *n){
  const sgx_aes_ctr_128bit_key_t *p_key= &key;  //key 
  const vect *p_src = n->value;  // input array
  const int src_len= sizeof(vect)*Z; //size of array
  uint8_t *p_ctr;   // initialization vector
  const int ctr_inc_bits=8;
  vect p_dst[Z];

  InitVectorStr s;s=s+ n->IV;
  //printf("Decoding with IV=%d\n",n->IV);
  p_ctr = (uint8_t*)&s;
  sgx_aes_ctr_decrypt(p_key,(uint8_t*)p_src,src_len,p_ctr,ctr_inc_bits,(uint8_t*)p_dst);
  
  memcpy(n->value,p_dst,sizeof(vect)*Z);
  //free(p_dst);
}




void encrypt_node(tree_node *n){
  const sgx_aes_ctr_128bit_key_t *p_key= &key;  //key 
  const Block *p_src = n->value;  // input array
  const int src_len= sizeof(Block)*Z; //size of array
  uint8_t *p_ctr;   // initialization vector
  const int ctr_inc_bits=8;
  Block p_dst[Z];


  InitVectorStr s= initVector;
  p_ctr = (uint8_t*)&s;
  
  sgx_aes_ctr_encrypt(p_key,(uint8_t*)p_src,src_len,p_ctr,ctr_inc_bits,(uint8_t*)p_dst);
  n->IV= (int)initVector;
  initVector = initVector+1;
  
  memcpy(n->value,p_dst,sizeof(Block)*Z);
  //printf("Encode with IV %d\n",n->IV);
  
  //free(p_dst);
}

void decrypt_node(tree_node *n){
  const sgx_aes_ctr_128bit_key_t *p_key= &key;  //key 
  const Block *p_src = n->value;  // input array
  const int src_len= sizeof(Block)*Z; //size of array
  uint8_t *p_ctr;   // initialization vector
  const int ctr_inc_bits=8;
  Block p_dst[Z];

  InitVectorStr s;s=s+ n->IV;
  //printf("Decoding with IV=%d\n",n->IV);
  p_ctr = (uint8_t*)&s;
  sgx_aes_ctr_decrypt(p_key,(uint8_t*)p_src,src_len,p_ctr,ctr_inc_bits,(uint8_t*)p_dst);
  
  memcpy(n->value,p_dst,sizeof(Block)*Z);
  //free(p_dst);

}



void encrypt(Block message[], int size){
  for(int i=0;i<size;i++){
    Block b = (message[i] ^ com_key);
    message[i]=b;
    //message[i]= (message[i] ^ com_to_client_key);
  }
}

void freeMem(){
  free(A);
}

void reSetComCounter(){
  Mid_Enc_Com=0;
  enteredfirsttime=false;
  clientmessagemodified=false;
}

/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
{
  char buf[BUFSIZ] = {'\0'};
  va_list ap;
  va_start(ap, fmt);
  vsnprintf(buf, BUFSIZ, fmt, ap);
  va_end(ap);
  ocall_print_string(buf);
}


void random_array(int arr[],int size){
  for(int i=0;i<size;i++){
    uint32_t rand;
    sgx_read_rand((unsigned char *)&rand,4);
    arr[i]= rand%size;
    //obliv_write_int(arr,i,size,rand%size);
  }
}

int random_item(int size){
  uint32_t rand;
  sgx_read_rand((unsigned char *)&rand,4);
  return rand%size;
}


int get_position_map(int i,bool obliv){
  if (trees_count==0){
    if (obliv)
      return obliv_read_int(positionmap,i,ARRSIZE);
    else
      return positionmap[i];
  }else {
    int index=i;
    for(int z=1;z<=trees_count;z++)
      index/=2;

    if (obliv){
      int size = tree_sizes[trees_count];
      int pos=obliv_read_int(positionmap,index,size);
      //printf("get pos map of i %d index %d oldpos %d\n",i, index,pos);
      obliv_write_int(positionmap,index,size, find_pos_recursive(index,trees_count,pos,i,-1));
    }else{
      int pos=positionmap[index];
      positionmap[index] = find_pos_recursive(index,trees_count,pos,i,-1);
    //printf("get pos map of index %d is %d\n",index,positionmap[index]);
    }
    return finalpos;
  }
}

void set_position_map(int i,int newval,bool obliv){
  if (trees_count==0){
    if (obliv)
      obliv_write_int(positionmap,i,ARRSIZE,newval);
    else
      positionmap[i]=newval;
  }else {
    int index=i;
    for(int z=1;z<=trees_count;z++)
      index/=2;

    if (obliv){
      int size = tree_sizes[trees_count];
      int pos=obliv_read_int(positionmap,index,size);
      obliv_write_int(positionmap,index,size,find_pos_recursive(index,trees_count,pos,i,newval));
    }else {
      int pos=positionmap[index];
      //printf("Set position for i %d index %d oldpos %d\n",i, index,pos);
      positionmap[index] = find_pos_recursive(index,trees_count,pos,i,newval);
    }
  }
}

int initialize_pos_recursive(int i,int level,int pos){
  int i1=i*2;
  int i2=i*2+1;
  int newval;

  
  if (level>1){
    newval = random_item(tree_sizes[level]); // to be returned
    //  if (i1<tree_sizes[level-1]){
    read_path_aux(pos,level);     
    int v1 = initialize_pos_recursive(i1,level-1,random_item(tree_sizes[level-1]));
      //printf("value 1 First store to stach i1=%d v=%d from tree %d\n",i1,v,level);

      int v2 = initialize_pos_recursive(i2,level-1,random_item(tree_sizes[level-1]));
      //printf("value 2 First store to stach i2=%d  v=%d from tree %d\n",i2,v,level);
      
      vect vec1; vec1.index=i1;vec1.position=newval;vec1.value=v1;
      vect vec2; vec2.index=i2;vec2.position=newval;vec2.value=v2;
      add_to_stach(level,vec1);
      add_to_stach(level,vec2);
      write_path_aux(pos,level);
      //printf("wrote path 1\n");
      // }

  /*
    if (i2<tree_sizes[level-1]){    
      read_path_aux(pos,level);
      //printf("read path 2\n");
     
     
    
      write_path_aux(pos,level);
      }*/
    return newval;
  }else {
    newval = random_item(tree_sizes[level]);
    //if (i<tree_sizes[level]){
    read_path_aux(pos,level);     
      int v1 =  random_item(ARRSIZE);
      int v2 = random_item(ARRSIZE);
      //printf("Last step in recursion for first store stach i=%d v=%d from tree %d stach_counter_aux %d pos %d\n",i,v,level,stach_counter_aux[level],pos);
      //vect vec; vec.index=i;vec.position=newval;vec.value=v;
      //add_to_stach(level,vec);
      vect vec1; vec1.index=i1;vec1.position=newval;vec1.value=v1;
      vect vec2; vec2.index=i2;vec2.position=newval;vec2.value=v2;
      add_to_stach(level,vec1);
      add_to_stach(level,vec2);


      
      /*
      for(int i=0;i<stach_counter_aux[level];i++)
	printf("%d-", stach_aux[level][i].index);
	printf("Tree %d\n",level);*/
      write_path_aux(pos,level);
      //printf("wrote a path in last round\n");
      //}
    return newval;
  }
}

void read_path_aux(int x,int treeID){
  
  //printf("x : %d Tree %d\n",x,treeID);
  
  tree_node_aux t = *links_aux[treeID][x];
  int level=1;
  
  while (1){
    //printf("%d->",t.temp);
    decrypt_node_aux (&t);
    
    for(int i=0;i<Z;i++){
      if (t.value[i].value!=-1){
	//printf("value %d index %d position %d\n",t.value[i].value,t.value[i].index,t.value[i].position);
	
	for(int z=0;z<stach_counter_aux[treeID];z++){
	  if (t.value[i].index==stach_aux[treeID][z].index)
	    printf("Problem. already in stach index %d !!!!!! %d level %d tree %d counter %d\n",z,stach_aux[treeID][z].index,
		   level,treeID,stach_counter_aux[treeID]);
	}
	add_to_stach(treeID,t.value[i]);
      }
    }
    if (t.parent==NULL) break;
    t = *(t.parent);
    level++;
  }

  //printf("stach counter after read %d level %d\n",stach_counter_aux[treeID],treeID);
  /*
    printf("%d %d \n",t.IV,(int)t.value[0]);
    decrypt_node (&t);
    printf("%d %d \n",t.IV,(int)t.value[0]);
  */
}


void write_path_aux(int x,int treeID){

  tree_node_aux *t = links_aux[treeID][x];
  int level=1;
  tree_node_aux temp;
  while (1){
    int min,max;
    int node_counter=0;
    //printf("writing path for leaf %d, node %d\n",x,t->temp);
    path_level(x,level-1,&min,&max);
    for(int i=0;i<Z;i++) temp.value[i].value=-1;
    //printf("Write:Calculate min %d and max %d and prepare temp\n",min,max);

    
    for(int i=0;i<stach_counter_aux[treeID];i++){
      int y_pos= stach_aux[treeID][i].position;
      //printf("Write:inside loop i %d counter %d y_pos=%d level %d\n",
      //i,stach_counter_aux[treeID],y_pos,level);
      if (y_pos>=min && y_pos<=max){
	temp.value[node_counter++]=stach_aux[treeID][i];
	//printf("Removing %d frm stach index %d at level %d tree %d counter %d \n",stach_aux[treeID][i].index,i,
	//  level,treeID,stach_counter_aux[treeID]);
	stach_aux[treeID][i]=stach_aux[treeID][stach_counter_aux[treeID]-1];   // move last one to the one
	i--;
	stach_counter_aux[treeID]--;

	/*
	for(int z=0;z<stach_counter_aux[treeID];z++)
	  printf("%d-", stach_aux[treeID][z].index);
	printf("Tree %d\n",treeID);
	*/
      }
      if (node_counter==Z) break;
    }

    encrypt_node_aux(&temp);
    memcpy(t->value, temp.value,sizeof(vect)*Z);
    t->IV=temp.IV;
    level++;
    if (t->parent==NULL)break;
    t = t->parent;
   
  }

  //printf("Stach count in tree %d after write %d\n",treeID,stach_counter_aux[treeID]);
  //printf("out\n");
}






void obliv_write_int(int *Arr,int i,int size,int new_val){
  //Arr[i]=new_val;return;
  Obliv_int[1]=new_val;
  for(int z=0;z<size;z+=inc_int){
    Obliv_int[0]=Arr[z];
    //if (z==i)
    if (i>=z && i<z+inc_int)
      Arr[i]=Obliv_int[1];
    else
      Arr[z]=Obliv_int[0];
  }
}

int obliv_read_int(int *Arr,int i,int size){
  //return Arr[i];
  for(int z=0;z<size;z+=inc_int){
    //if (i==z)
    if (i>=z && i<z+inc_int)
      Obliv_int[1]=Arr[i];
    else
      Obliv_int[0]=Arr[z];
  }
  
  return Obliv_int[1];
}

int*  MyMallocInt(int size,char **ptr){
  //return (int*)malloc(size);
  char *Arr;
  Arr = (char*)malloc(size+PAGESIZE);
  *ptr=Arr;
 
  
  if ((long int)Arr%PAGESIZE==0)
    return (int *)Arr;
  else{
    int i = ((((long int)Arr/PAGESIZE)+1)*PAGESIZE);
    int addr_Arr = (long int) Arr;
    int diff = i-addr_Arr;
    return (int*)(&Arr[diff]);
  }
  
}

void MyFree(char *ptr){
  free(ptr);
}

int find_pos_recursive(int i,int level,int pos,int final,int write_val){
  int i1=i*2;
  int i2=i*2+1;
  int newval;

  newval = random_item(tree_sizes[level]);

  if (level>1){
    read_path_aux(pos,level);
    int pos1,pos2;
    for(int z=0;z<stach_counter_aux[level];z++){
      if (stach_aux[level][z].index==i1){
	//printf("Found first %d val %d tree %d\n",
	//     i1,stach_aux[level][z].value,level);
	pos1= stach_aux[level][z].value;
      }if (stach_aux[level][z].index==i2){
	//printf("Found Second %d val %d tree %d\n",
	//     i2,stach_aux[level][z].value,level);
	pos2= stach_aux[level][z].value;
      }
    }
    int v1 = find_pos_recursive(i1,level-1,pos1,final,write_val);
    int v2 = find_pos_recursive(i2,level-1,pos2,final,write_val);

    for(int z=0;z<stach_counter_aux[level];z++){
      if (stach_aux[level][z].index==i1){
	//printf("Found first back %d val %d tree %d\n",
	//     i1,stach_aux[level][z].value,level);
	stach_aux[level][z].value=v1;
	stach_aux[level][z].position=newval;
      }if (stach_aux[level][z].index==i2){
	//printf("Found Second back %d val %d tree %d\n",
	//     i2,stach_aux[level][z].value,level);
	stach_aux[level][z].value=v2;
	stach_aux[level][z].position=newval;
      }
    }
    
    write_path_aux(pos,level);
  }else {
    read_path_aux(pos,level); 
    for(int z=0;z<stach_counter_aux[level];z++){
      if (stach_aux[level][z].index==i1 || stach_aux[level][z].index==i2){
	stach_aux[level][z].position=newval;
	if (stach_aux[level][z].index==final){
	  //printf("Found final %d val %d\n",i,stach_aux[level][z].value);
	  if (write_val==-1)
	    finalpos =stach_aux[level][z].value;
	  else
	    stach_aux[level][z].value= write_val;
	}
      }
    }

    write_path_aux(pos,level);
  }

  return newval;
}


void add_to_stach(int treeID,vect b){
  stach_aux[treeID][stach_counter_aux[treeID]++]=b;
	//printf("Read:storing %d in stach at level %d tree %d counter %d\n",
	//   stach_aux[treeID][stach_counter_aux[treeID]-1].index,level,treeID,stach_counter_aux[treeID]);

	/*
	for(int z=0;z<stach_counter_aux[treeID];z++)
	  printf("%d-", stach_aux[treeID][z].index);
	printf("Tree %d\n",treeID);
	*/
  if (stach_max_aux[treeID]<stach_counter_aux[treeID]) stach_max_aux[treeID]=stach_counter_aux[treeID];

  if (stach_counter_aux[treeID]==stach_sizes[treeID]-1){
    stach_sizes[treeID]+=STACH_LIMIT;
    stach_aux[treeID]= (vect*)realloc( stach_aux[treeID],stach_sizes[treeID]*sizeof(vect));
    printf("Reallocating is done to tree %d\n",treeID);
  }
}

void add_to_stach_item(Block v){
  stach[stach_counter++]=v;
    //printf("initial storing pos %d in stach\n",stach[stach_counter-1].getPos());
  if (stach_max<stach_counter) stach_max=stach_counter;
  if (stach_counter==stach_sizes[0]-1){
    stach_sizes[0]+=STACH_LIMIT;
    stach = (Block*)realloc(stach,stach_sizes[0]*sizeof(Block));
    printf("Reallocating is done to tree 0\n");
  }  
}
