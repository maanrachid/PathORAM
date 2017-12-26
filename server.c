#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include<stdlib.h>
#include <time.h>

#define ENCLAVE_FILE "enclave.signed.so"
//#define ARRSIZE 1000
//#define BUFFERSIZE 10000
#include "ext.h"
#include "MyMath.h"
#include "Tree.h"
#include "Encryptor.h"


void checkContents(Block* Arr1,char *message);
void show_usage();
void encrypt_tree(tree_node *root,Block *B,long long IV);
void encrypt_tree_aux(tree_node_aux *root,vect *v,long long IV);
int recieve_and_send(int sock);


int senttimes=0;



int main(int argc , char *argv[])
{
  int socket_desc , client_sock , c , read_size, sock;
  struct sockaddr_in middle , client, server;
  Block client_message[3];  // operation , position , newvalue
  //char message[2000];
  //char  server_reply[2000];

  if (argc!=3){
    show_usage();
    exit(0);
  }
  const int ARRSIZE = atoi(argv[2]);

  //Block* A= new Block[ARRSIZE];
  //Block* B= new Block[ARRSIZE];

  int updated = 0;
  int pacsize = ARRSIZE*sizeof(Block);
  
 
  
  //printf("Buffer Size:%d\n",BUFFERSIZE*sizeof(Block));
  printf("Number of Blocks:%d\n",ARRSIZE);
  printf("Size of a Block: %d\n",sizeof(Block));
  printf("Position map threshold: %d\n",POS_MAP_LIMIT);
  printf("Z: %d\n",Z);




   
  //Create socket to let Client connect to it -------------------------
  socket_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (socket_desc == -1)
    {
      printf("Could not create socket");
    }
  puts("Socket created to receive from Client");
     
  //Prepare the sockaddr_in structure
  middle.sin_family = AF_INET;
  middle.sin_addr.s_addr = INADDR_ANY;
  middle.sin_port = htons( atoi(argv[1]) );  //8887
     
  //Bind
  if( bind(socket_desc,(struct sockaddr *)&middle , sizeof(middle)) < 0)
    {
      //print the error message
      perror("bind failed. Error");
      return 1;
    }
  puts("bind done");
     
  //Listen
  listen(socket_desc , 3);
     
  //Accept and incoming connection
  puts("Waiting for incoming connections from Client...");
  c = sizeof(struct sockaddr_in);
     
  //accept connection from an incoming client
  client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
  if (client_sock < 0)
    {
      perror("accept failed");
      return 1;
    }
  puts("Connection accepted");
  // end connecting to client ----------------------------------------------------------

  // receiving encrypted vect and block and the IV 
  int TotalSize = Z*(sizeof(vect)+sizeof(Block)) + sizeof(long long);

  Block b[Z];vect v[Z];
  char *Buf=(char *) malloc(TotalSize);
  if ( (read_size = recv(client_sock , Buf , TotalSize , 0)) > 0 ){
   
    memcpy(b,Buf,Z*sizeof(Block));  
    memcpy(v,&Buf[Z*sizeof(Block)],Z*sizeof(vect));
    long long received_IV;
    memcpy(&received_IV,&Buf[TotalSize-sizeof(long long)],sizeof (long long));

    
    //printf("%d \n",received_IV);
    //printf("%d\n",b[Z-1]);
    //printf("%d\n",v[Z-1].value);


    
    // Creating tree and auxiliary trees
    clock_t begin_time = clock();
    printf("Creating Items Binary tree....\n");
    tree_node *tr0 = create_tree(ARRSIZE);
    tree_node_aux **tr_aux=new tree_node_aux*[TREES];
    //traverse(tr0);
    tr0->parent=NULL;
    encrypt_tree(tr0,b,received_IV);
    printf("Creating and Encrypting Items Binary tree is completed.\n");
    printf("Time for creating item trees: %f\n",
	   float( clock () - begin_time ) /  CLOCKS_PER_SEC);

    //traverse(tr0);
    
    
    int temp = ARRSIZE;
    int temp1 = find_closet_power_of_two(temp);// find closet size of power of 2
    int trees_count=1;

    if (temp>POS_MAP_LIMIT){
      begin_time =clock();
      temp1/=2;
      do{
	printf("Creating tr%d size:%d...\n",trees_count,temp1);
	tr_aux[trees_count] = create_tree_aux(temp1,trees_count);
	tr_aux[trees_count]->parent=NULL;
	encrypt_tree_aux(tr_aux[trees_count],v,received_IV);
	//printf("%d %d\n",tr_aux[trees_count]->right->temp,links_to_leaves_aux[trees_count][0]->temp); 
	temp1/=2;
	trees_count++;
      } while (temp1>POS_MAP_LIMIT);
      printf("Time for Creating auxiliary trees: %f\n",
	     float( clock () - begin_time ) /  CLOCKS_PER_SEC);
    }
    
    int ans=1;
    write(client_sock,&ans,sizeof(int));
  }



  while (1){
    int v = recieve_and_send(client_sock);
    if (v==-1)
      break;
  }

  


  close(sock); // disconnect from server

  //freeMem(eid);
  //delete [] A;
  //delete [] B;

  
     
  return 0;
}


int recieve_and_send(int sock){
  int counter=0;
  char buffer[BUFFERSIZE];
  int treeID,command,bufsize,x;
  
  

  // reading the command
  if( recv(sock , buffer , BUFFERSIZE  , 0) > 0)   // send a  request to server
    {
      treeID= *((int*)&buffer[counter]);
      counter+=4;
      x= *((int*)&buffer[counter]);
      counter+=4;
      command=*((int*)&buffer[counter]);
      counter+=4;
      bufsize = *((int *)&buffer[counter]);
      counter+=4;
      //printf("%d,%d,%d,%d\n",treeID,x,command,bufsize);

      int dummy;
	
      // executing the command 
      //if (command==-1)
      //return command;
      if (command==1 && treeID>0){
	counter=4;
	tree_node_aux t = *links_to_leaves_aux[treeID][x];
	int size=0;
	while(1){
	  memcpy(&buffer[counter],&(t.IV),sizeof(long long));
	  counter+=sizeof(long long);
	  size+=sizeof(long long);
	  memcpy(&buffer[counter],t.value, sizeof(vect)*Z);
	  counter+=sizeof(vect)*Z;
	  size+=sizeof(vect)*Z;
	  if (t.parent==NULL) break;
	  t = *(t.parent);
	}
	memcpy(buffer,&size,4);
	//write(sock , buffer , 4);
	//recv(sock,&dummy,4,0);
	//write(sock, &buffer[4],size);
	
      }else if (command==2 && treeID>0){ //write auxiliary
	//write(sock,&dummy,4);
	//recv(sock , buffer , bufsize  , 0);
	//counter=0;
	 
	tree_node_aux *t = links_to_leaves_aux[treeID][x];
	tree_node_aux temp;
	int bufsize2=bufsize;
	while (t!=NULL && bufsize2>0){
	  memcpy(t->value,&buffer[counter],sizeof(vect)*Z);
	  counter+=sizeof(vect)*Z;
	  bufsize2-=sizeof(vect)*Z;
	  memcpy(&(t->IV),&buffer[counter],sizeof(long long));
	  //printf("IV : %d\n",t->IV);
	  //printf("value for first vect %d\n",((vect*)(&buffer[counter-Z*sizeof(vect)]))->value);
	  counter+=sizeof(long long);
	  bufsize2-=sizeof(long long);
	  t=t->parent;
	}
      }else if (command==1 && treeID==0){
	counter=4;
	tree_node t = *links_to_leaves[x];
	int size=0;
	while(1){
	  memcpy(&buffer[counter],&(t.IV),sizeof(long long));
	  counter+=sizeof(long long);
	  size+=sizeof(long long);
	  memcpy(&buffer[counter],t.value, sizeof(Block)*Z);
	  counter+=sizeof(Block)*Z;
	  size+=sizeof(Block)*Z;
	  if (t.parent==NULL) break;
	  t = *(t.parent);
	}
	memcpy(buffer,&size,4);
	//write(sock , buffer , 4);
	//recv(sock,&dummy,4,0);
	//write(sock, &buffer[4],size);

	
      }else if (command==2 && treeID==0){
	//write(sock,&dummy,4);
	//recv(sock , buffer , bufsize  , 0);
	//counter=0;
	
	tree_node *t = links_to_leaves[x];
	tree_node temp;
	int bufsize2=bufsize;
	while (t!=NULL && bufsize2>0){
	  memcpy(t->value,&buffer[counter],sizeof(Block)*Z);
	  counter+=sizeof(Block)*Z;
	  bufsize2-=sizeof(Block)*Z;
	  memcpy(&(t->IV),&buffer[counter],sizeof(long long));
	  //printf("IV : %d\n",t->IV);
	  //printf("value for first block %d\n",((vect*)(&buffer[counter-Z*sizeof(Block)]))->value);
	  counter+=sizeof(long long);
	  bufsize2-=sizeof(long long);
	  t=t->parent;
	}

      }

      
      // sending results to client
      write(sock , buffer , BUFFERSIZE);
    }

  return command;
}





void checkContents(Block* Arr1,char * message){
  puts(message);
  for(int i=0;i<10;i++)
    printf("%d\n",(int)Arr1[i]);
}


void show_usage(){
  printf("middle \[server port\] \[Array size\]  \n");
  //printf("Oram type: 1 for linear, 2 for sqrt\n");
}


/* OCall functions */
void ocall_print_string(const char *str)
{
    /* Proxy/Bridge will check the length and null-terminate 
     * the input string to prevent buffer overflow. 
     */
    printf("%s", str);
}


void encrypt_tree(tree_node *root,Block *B,long long IV){
  if (root==NULL)
    return;
  else {
    //    for (int i=0;i<Z;i++)  // dummitize a node
    //  root->value[i]=-1;
    //encrypt_node(root);
    memcpy(root->value,B,Z*sizeof(Block));
    root->IV= IV;
    encrypt_tree(root->left,B,IV);
    encrypt_tree(root->right,B,IV);   
  }
}

void encrypt_tree_aux(tree_node_aux *root,vect *v,long long IV){
   if (root==NULL)
    return;
  else {
    //    for (int i=0;i<Z;i++)  // dummitize a node
    //  root->value[i]=-1;
    //encrypt_node(root);
    memcpy(root->value,v,Z*sizeof(vect));
    root->IV=IV;
    encrypt_tree_aux(root->left,v,IV);
    encrypt_tree_aux(root->right,v,IV);
  }
}
