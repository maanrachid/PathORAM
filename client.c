#include<stdio.h> //printf
#include <stdlib.h>
#include <time.h>
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <unistd.h>   //close
#include <stdlib.h>
#include <time.h>
#include "ext.h"
#include "MyMath.h"
#include "Encryptor.h"
//#include "wolfssl/wolfcrypt/aes.h"

//#define ARRSIZE 1000

void encrypt(Block message[], int size);
//void encrypt_v2(Block message[], int size,int op);
//void encrypt_v3(Block message[], int size,int op);
void checkContent(Block message[],int size, char * mess);
void show_usage();
int random_item(int i);
int initialize_pos_recursive(int sock,int i,int level,int pos);
void send_and_recieve(int sock,int treeid,int x,int command, char * buf,int *bufsize);
void add_to_stach(int treeID,vect b);
void add_to_stach_item(Block v);
void read_path_aux(int sock,int x,int treeID);
void write_path_aux(int sock,int x,int treeID);
int find_pos_recursive(int sock,int i,int level,int pos,int final,int write_val);
void read_path(int sock,int x);
void write_path(int sock,int x);
int get_position_map(int sock,int i);
void set_position_map(int sock,int i,int newval);
Block request_an_item(int sock,int index,int newval);


//void init_Enc();

long long com_key;
Block *stach;
vect *stach_aux[TREES];
int stach_counter=0;
int stach_counter_aux[TREES];
int stach_max=0;
int stach_max_aux[TREES];
int stach_sizes[TREES];
int *positionmap;
int trees_count=1;
int tree_sizes[TREES];
int heights[TREES];
int ARRSIZE;
int finalpos;

int main(int argc , char *argv[])
{

  
  int sock;
  struct sockaddr_in middle;
  Block message[3];
  Block server_reply[3];
  if (argc!=5) {show_usage(); exit(0);}
  ARRSIZE = atoi(argv[3]);
  int requests = atoi(argv[4]);
  
  com_key=10000;

  if (ENCRYPT_LIB== 2)
    printf("Encryption is done using IPP.\n");
  else if  (ENCRYPT_LIB== 3)
    printf("Encryption is done using wolfssl.\n"); 
  
  
  //Create socket
  sock = socket(AF_INET , SOCK_STREAM , 0);
  if (sock == -1)
    {
      printf("Could not create socket");
    }
  puts("Socket created. Trying to connect to server");
     
  middle.sin_addr.s_addr = inet_addr(argv[1]); //127.0.0.1
  middle.sin_family = AF_INET;
  middle.sin_port = htons( atoi(argv[2]) ); //8887
  
  
  //Connect to Middle 
  while (connect(sock , (struct sockaddr *)&middle , sizeof(middle)) < 0)
    {
      //perror("connect failed. Error");
      //return 1;
    }
     
  puts("Connected\n");
  srand(time(0));


  printf("Initialization....\n");
  printf("Encrypting tree..\n");


  printf("Sending Encrypted Block\n");
  Block b[Z]; vect v[Z];
  int TotalSize = Z*(sizeof(vect)+sizeof(Block)) + sizeof(long long); 
  for(int i=0;i<Z;i++){
    b[i]=-1; // all values are dummies.
    v[i].value=-1;
  }
  //  Encrypt((char*)b,Z*sizeof(Block),1,com_key);
  //Encrypt((char*)v,Z*sizeof(vect),1,com_key);
  
 
  char *Buf=(char*)malloc(TotalSize);
  memcpy(Buf,b,Z*sizeof(Block));
  memcpy(&Buf[Z*sizeof(Block)],v,Z*sizeof(vect));
  memcpy(&Buf[TotalSize-sizeof(long long)],&com_key,sizeof(long long));

  
  if( send(sock , Buf , TotalSize , 0) < 0)
    {
      puts("Send failed");
      
    }


  int ans;
  if( recv(sock , &ans , sizeof(int) , 0) < 0)
    {
      puts("recv failed");
      exit(0);;
    }
  if (ans==1)
    printf("Encrypted successfullly\n");

  
  srand(time(NULL));
  
  if (ARRSIZE<=POS_MAP_LIMIT){
    positionmap = (int*)malloc(ARRSIZE*sizeof(int));
    for(int i=0;i<ARRSIZE;i++){
      positionmap[i]=rand()%ARRSIZE;
      //printf("%d\n",positionmap[i]);
    }
  }else {
    int temp1 = find_closet_power_of_two(ARRSIZE);// find closet size of power of 2
    heights[0]=log(temp1)+1;
    printf("Item Tree : height %d\n",heights[0]);
    temp1/=2;
    do{
      tree_sizes[trees_count]=temp1;
      heights[trees_count++]=log(temp1)+1;
      printf("Tree %d : %d height : %d \n",trees_count-1,tree_sizes[trees_count-1],heights[trees_count-1]);
      temp1/=2;
    }while(temp1>POS_MAP_LIMIT);
    positionmap = (int*)malloc(tree_sizes[trees_count-1]*sizeof(int));
   // should only be called once
    for(int i=0;i<tree_sizes[trees_count-1];i++)
      positionmap[i]=rand()%tree_sizes[trees_count-1];
  }
  

  // prepare stash
  stach_sizes[0]=STACH_LIMIT;
  stach = (Block*) malloc(sizeof(Block)*stach_sizes[0]);
  
  if (trees_count>1){
    //stach_aux[1]=(vect*)malloc(sizeof(vect)*STACH_LIMIT);
    for(int i=1;i<trees_count;i++){
      stach_sizes[i] = STACH_LIMIT;
      stach_aux[i]=(vect*)malloc(sizeof(vect)*(stach_sizes[i]));
    }
  }


  /* Testing only
  read_path_aux(sock,10,1);
  vect vec1; vec1.index=1;vec1.position=1;vec1.value=2;
  vect vec2; vec2.index=2;vec2.position=2;vec2.value=1;
  add_to_stach(1,vec1);
  add_to_stach(1,vec2);

  write_path_aux(sock,10,1); */

  

  
  if (trees_count>1){
    printf("Creating position map using auxillary trees..\n");
    for(int i=0;i<tree_sizes[trees_count-1];i++){
      //printf("processing %d -------------- \n",i); 
      positionmap[i]= initialize_pos_recursive(sock,i,trees_count-1,positionmap[i]);   
      //printf("processed %d %d -------------- \n",i,positionmap[i]); 
    }
    printf("Position map is ready\n");
  }




  
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
   
    set_position_map(sock,i,newval);
    read_path(sock,x);
    write_path(sock,x);
    //printf("Done i=%d-------------------------\n",i);
  }


  //end initialization


  
 

  
  //keep communicating with Middle
  for(int i=0;i<requests;i++)
     //    while(1)
  
    {

  
    

      int request = rand()%ARRSIZE;
      printf("Requesting item %d\n",request);
      printf("The value of item %d is %d\n",request,(int)request_an_item(sock,request,-1));


      

      
      //encrypt(message,3);  
      //Encrypt((char*)message,3,1,com_key); // encrypt
      
      //encrypt(message,3);  // decrypt for testing
      
      //checkContent(message,3,"After encryption from client:\n"); 

      //encrypt_v3(message,3,2);
      //checkContent(message,3,"After decryption from client:\n");
  
    // encrypt then decrypt then change 
    }
  
  char dummy[2];int dumm=2;
  send_and_recieve(sock,1,1,-1,dummy,&dumm);
  close(sock);
  return 0;
}


/*
void encrypt(Block message[], int size){
  for(int i=0;i<size;i++){
    message[i]= (message[i] ^ com_key);
  }

  }*/



void show_usage(){
  printf("client <middle ip> <middle port> <Array size> <number of requests>  \n");
  
}

void checkContent(Block message[],int size, char * mess){
  printf("%s\n",mess);
  for(int i=0;i<size;i++){
    printf("%d\n",(int)message[i]);
  }

}


int get_position_map(int sock,int i){
  if (trees_count==1)
    return positionmap[i];
  else {
    int index=i;
    for(int z=1;z<trees_count;z++)
      index/=2;
    
    int pos=positionmap[index];
    positionmap[index] = find_pos_recursive(sock,index,trees_count-1,pos,i,-1);
    //printf("get pos map of index %d is %d\n",index,positionmap[index]);
    return finalpos;
  }
}

void set_position_map(int sock,int i,int newval){
  if (trees_count==1){
      positionmap[i]=newval;
  }else {
    int index=i;
    for(int z=1;z<trees_count;z++)
      index/=2;
    int pos=positionmap[index];
    //printf("Set position for i %d index %d oldpos %d\n",i, index,pos);
    positionmap[index] = find_pos_recursive(sock,index,trees_count-1,pos,i,newval);
   
  }
}

int find_pos_recursive(int sock,int i,int level,int pos,int final,int write_val){
  int i1=i*2;
  int i2=i*2+1;
  int newval;

  newval = random_item(tree_sizes[level]);

  if (level>1){
    read_path_aux(sock,pos,level);
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
    int v1 = find_pos_recursive(sock,i1,level-1,pos1,final,write_val);
    int v2 = find_pos_recursive(sock,i2,level-1,pos2,final,write_val);

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
    
    write_path_aux(sock,pos,level);
  }else {
    read_path_aux(sock,pos,level); 
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

    write_path_aux(sock,pos,level);
  }

  return newval;
}





int initialize_pos_recursive(int sock,int i,int level,int pos){
  int i1=i*2;
  int i2=i*2+1;
  int newval;

  
  if (level>1){
    newval = random_item(tree_sizes[level]); // to be returned
    //  if (i1<tree_sizes[level-1]){
    //printf("read value %d path %d tree %d\n",i,pos,level);
    read_path_aux(sock,pos,level);
    
    int v1 = initialize_pos_recursive(sock,i1,level-1,random_item(tree_sizes[level-1]));
    //printf("value 1 First store to stach i1=%d v=%d from tree %d\n",i1,v1,level);

    int v2 = initialize_pos_recursive(sock,i2,level-1,random_item(tree_sizes[level-1]));
    //printf("value 2 First store to stach i2=%d  v=%d from tree %d\n",i2,v2,level);
      
    vect vec1; vec1.index=i1;vec1.position=newval;vec1.value=v1;
    vect vec2; vec2.index=i2;vec2.position=newval;vec2.value=v2;
    add_to_stach(level,vec1);
    add_to_stach(level,vec2);
    write_path_aux(sock,pos,level);
    return newval;
  }else {
    newval = random_item(tree_sizes[level]);
    //if (i<tree_sizes[level]){
    //printf("reading in tree %d\n",level);
    read_path_aux(sock,pos,level);     
    int v1 =  random_item(ARRSIZE);
    int v2 = random_item(ARRSIZE);
    vect vec1; vec1.index=i1;vec1.position=newval;vec1.value=v1;
    vect vec2; vec2.index=i2;vec2.position=newval;vec2.value=v2;
    add_to_stach(level,vec1);
    add_to_stach(level,vec2);
   
    write_path_aux(sock,pos,level);
   
    return newval;
  }
}  



int random_item(int size){
  return rand()%size;
}

void read_path(int sock,int x){
  

  char buf[BUFFERSIZE];
  int size=0;

  //printf("x : %d Tree %d\n",x,treeID);
  send_and_recieve(sock,0,x,1,buf,&size);

  //printf("finish send and received a read for path %d size  %d\n",x,size);
  
  int counter=4;
  while (size>0){
    long long *IV_p= (long long *)&buf[counter];
    long long IV= *IV_p;
    counter+= sizeof(long long);
    size-= sizeof(long long);
    Block *t = (Block*)&buf[counter];
    //Encrypt((char*) t, sizeof(Block)*Z ,2, IV); // decrypting the received node
    for(int i=0;i<Z;i++){
      if (t[i]!=-1){
	//printf("adding to the stach - read path %d value %d\n",x,(int)t[i]);
	add_to_stach_item(t[i]);
      }
    }
    counter +=  sizeof(Block)*Z;
    size-= sizeof(Block)*Z;
  }


}




void read_path_aux(int sock,int x,int treeID){
  

  char buf[BUFFERSIZE];
  int size=0;

  //printf("x : %d Tree %d\n",x,treeID);
  send_and_recieve(sock,treeID,x,1,buf,&size);

  //printf("finish send and received a read for path %d size  %d\n",x,size);
  
  int counter=4;
  while (size>0){
    long long *IV_p= (long long *)&buf[counter];
    long long IV= *IV_p;
    counter+= sizeof(long long);
    size-= sizeof(long long);
    vect *t = (vect*)&buf[counter];
    //Encrypt((char*) t, sizeof(vect)*Z ,2, IV); // decrypting the received node
    for(int i=0;i<Z;i++){
     
      if (t[i].value!=-1){
	//printf("adding to the stach - read path %d value %d\n",x,t[i].value);
	add_to_stach(treeID,t[i]);
      }
    }
    counter +=  sizeof(vect)*Z;
    size-= sizeof(vect)*Z;
  }


}

void add_to_stach(int treeID,vect b){
  stach_aux[treeID][stach_counter_aux[treeID]++]=b;
       
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


void send_and_recieve(int sock,int treeid,int x,int command, char * buf,int *bufsize){
  char buffer[BUFFERSIZE];
  int counter=0;

  memcpy(&buffer[counter],&treeid,sizeof(int));
  counter+=4;
  memcpy(&buffer[counter],&x,sizeof(int));
  counter+=4;
  memcpy(&buffer[counter], &command,sizeof(int));
  counter+=4;
  memcpy(&buffer[counter], bufsize,sizeof(int));
  counter+=4;

  memcpy(&buffer[counter],buf,*bufsize);
  
  if( send(sock , buffer , BUFFERSIZE , 0) < 0){
  }


  
  recv(sock , buffer , BUFFERSIZE , 0);

  *bufsize = *((int*)buffer);
  memcpy(buf,buffer,*bufsize);


  /*********************new code
  
  if( send(sock , buffer , 16 , 0) < 0)   // send meta data
    {
      puts("Send to server failed");
      exit(0);
    }


  int bufsize2; int dummy;
  if (command==1){
    if ( recv(sock , &bufsize2 , 4 , 0) < 0){ // received the size of data
    }

   
    send(sock , &dummy , 4 , 0); 

    recv(sock , buf , bufsize2 , 0);
    *bufsize = bufsize2;
  }else { //command==2
    recv(sock,&dummy,4,0);
    if( send(sock , buf , *bufsize , 0) < 0)   // send  the data
      {
	puts("Send to server failed");
	exit(0);
      }
  }
  
 

  /*
  printf("%d\n",*bufsize);
  printf("%lld\n",*((int *)&buf[4]));
  Encrypt(&buf[12],sizeof(vect)*Z,2,10000);
  printf("%d\n",((vect *)&buf[12])->value);
  printf("%lld\n",*((int *)&buf[12+sizeof(vect)*Z]));
  printf("%lld\n",*((int *)&buf[20+2*sizeof(vect)*Z]));*/
}

void write_path(int sock,int x){
  char buffer[BUFFERSIZE];
  int counter=0;

  int level=1;
  tree_node temp;
  while (level<=heights[0]){
    int min,max;
    int node_counter=0;
    path_level(x,level-1,&min,&max);
    for(int i=0;i<stach_counter;i++){    
      int y_pos= stach[i].getMap();
      if (y_pos>=min && y_pos<=max){
	memcpy(&buffer[counter],&stach[i],sizeof(Block));
	counter+=sizeof(Block);
	//printf("writing from the item stach to the buffer level %d stach count %d \n",
	//       level,	stach_counter);
	stach[i]=stach[stach_counter-1];   // move last one to the one
	i--;
	stach_counter--;
	node_counter++;
      }

      if (node_counter==Z) {
	break;
      }
    }

    level++;
    com_key++;
    for(int i=0;i<Z-node_counter;i++){
      Block z; z=-1;
      memcpy(&buffer[counter],&z,sizeof(Block));
      counter+=sizeof(Block);
    }
    //Encrypt(&buffer[counter-sizeof(Block)*Z],sizeof(Block)*Z,1,com_key); // encrypt
    memcpy(&buffer[counter],&com_key,sizeof(long long));
    counter+=sizeof(long long);
  }

  int v=counter;
  send_and_recieve(sock,0,x,2,buffer,&counter);
  //printf("finish send and received a write size %d\n",v);
}


void write_path_aux(int sock,int x,int treeID){

  char buffer[BUFFERSIZE];
  int counter=0;

  int level=1;
  tree_node_aux temp;
  while (level<=heights[treeID]){
    int min,max;
    int node_counter=0;
    path_level(x,level-1,&min,&max);
    //printf("level %d stach count %d max %d min %d \n",
    //   level,stach_counter_aux[treeID],max,min);
    for(int i=0;i<stach_counter_aux[treeID];i++){    
      int y_pos= stach_aux[treeID][i].position;
      if (y_pos>=min && y_pos<=max){
	memcpy(&buffer[counter],&stach_aux[treeID][i],sizeof(vect));
	counter+=sizeof(vect);
	//printf("writing from the stach to the buffer level %d stach count %d \n",
	//       level,	stach_counter_aux[treeID]);
	stach_aux[treeID][i]=stach_aux[treeID][stach_counter_aux[treeID]-1];   // move last one to the one
	i--;
	stach_counter_aux[treeID]--;
	node_counter++;
      }

      if (node_counter==Z) {
	break;
      }
    }

    level++;
    com_key++;
    for(int i=0;i<Z-node_counter;i++){
      vect z; z.value=-1;
      memcpy(&buffer[counter],&z,sizeof(vect));
      counter+=sizeof(vect);
    }
    //Encrypt(&buffer[counter-sizeof(vect)*Z],sizeof(vect)*Z,1,com_key);
    memcpy(&buffer[counter],&com_key,sizeof(long long));
    counter+=sizeof(long long);
  }

  
  int v=counter;
  send_and_recieve(sock,treeID,x,2,buffer,&counter);
  //printf("finish send and received a write size %d\n",v);
  
}



Block request_an_item(int sock,int index,int newval2){
  int x = get_position_map(sock,index);
  int newval = random_item(ARRSIZE);
  set_position_map(sock,index,newval);
  read_path(sock,x);
  Block ret;
  
  for(int i=0;i<stach_counter;i++){
    printf("stach[i] %d %d\n",stach[i].getPos(),(int)stach[i]);
    if (stach[i].getPos() == index){
      stach[i].setMap(newval);
      if (newval2!=-1){
	stach[i] = newval2;
      }else{
	ret = stach[i];
      }
    }
  }
  write_path(sock,x);
  return ret;
}

