#ifndef Tree_h
#define Tree_h
#include "ext.h"
#include "MyMath.h"
#include <iostream>



using namespace std;



int gg = 0;
tree_node** links_to_leaves;
tree_node_aux*** links_to_leaves_aux=new tree_node_aux**[TREES];
int leaf_count=0;
int leaf_count_aux[TREES];
int stop_rec=false;

void  create_tree_rec(int level,tree_node* root,int n){
  if (stop_rec) return;
  if (level==0){
    //cout <<"create a link to a leaf " << root->temp<<endl;
     links_to_leaves[leaf_count++]=root;
     if (leaf_count==n) stop_rec=true;
  }
   
  if (level>0){    
    root->left = new tree_node();
    //root->left->temp=gg++;
    root->left->parent = root;
    root->right=new tree_node();
    //root->right->temp=gg++;
    root->right->parent=root;
    //    if (root->temp==1) cout << "root "<< endl;
    //cout <<"node "<<root->temp<<endl;
    //cout<<"creating left "<<root->left->temp<<" level:"<<level<<endl;
    //cout<<"creating right "<<root->right->temp<<" level:"<< level <<endl;
    //if (root->parent!=NULL) cout<<"parent "<<root->parent->temp<<endl;
    create_tree_rec(level-1,root->left,n);
    create_tree_rec(level-1,root->right,n);
  }
}

void  create_tree_rec_aux(int level,tree_node_aux* root,int n,int index){
  if (stop_rec) return;
  if (level==0){
    //cout <<"create a link to a leaf " << root->temp<<endl;
     links_to_leaves_aux[index][leaf_count_aux[index]++]=root;
     if (leaf_count_aux[index]==n) stop_rec=true;
  }
   
  if (level>0){    
    root->left = new tree_node_aux();
    //root->left->temp=gg++;
    root->left->parent = root;
    root->right=new tree_node_aux();
    //root->right->temp=gg++;
    root->right->parent=root;
    //    if (root->temp==1) cout << "root "<< endl;
    //cout <<"node "<<root->temp<<endl;
    //cout<<"creating left "<<root->left->temp<<" level:"<<level<<endl;
    //cout<<"creating right "<<root->right->temp<<" level:"<< level <<endl;
    //if (root->parent!=NULL) cout<<"parent "<<root->parent->temp<<endl;
    create_tree_rec_aux(level-1,root->left,n,index);
    create_tree_rec_aux(level-1,root->right,n,index);
  }
}





void traverse(tree_node *root){
  if (root==NULL)
    return;
  else{
    cout<<root->IV<<endl;
    traverse(root->left);
    traverse(root->right);
  }
}



tree_node *create_tree(int n){
  int log_n = log(n);
  //cout<<log_n<<endl;
  links_to_leaves = new tree_node*[n];
  tree_node* ptr = new tree_node();
  gg=1;
  //ptr->temp=gg++;
  ptr->parent=NULL;
  stop_rec=false;

  create_tree_rec(log_n,ptr,n);
  //cout<<ptr->left->temp<<endl;
  //cout<<ptr->right->temp<<endl;
  //cout<<ptr->temp<<endl;
  //cout<<ptr->parent->temp<<endl;
  return ptr;
}

tree_node_aux *create_tree_aux(int n,int index){
  int log_n = log(n);
  //cout<<log_n<<endl;
  links_to_leaves_aux[index] = new tree_node_aux*[n];
  tree_node_aux* ptr = new tree_node_aux();
  gg=1;
  //ptr->temp=gg++;
  ptr->parent=NULL;
  leaf_count_aux[index]=0;
  stop_rec=false;
  create_tree_rec_aux(log_n,ptr,n,index);
  //cout<<ptr->left->temp<<endl;
  //cout<<ptr->right->temp<<endl;
  //cout<<ptr->temp<<endl;
  //cout<<ptr->parent->temp<<endl;
  return ptr;
}




#endif 
