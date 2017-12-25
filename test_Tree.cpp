#include <iostream>
#include "Tree.h"

using namespace std;


int main(){
  cout<<"Enter N :";
  int N;
  cin >> N;
  tree_node_aux *l= create_tree_aux(N,1);
  //l->parent=NULL;
     traverse(l);
  //cout<<l->parent->temp<<endl;



  
  for(int i=0;i<leaf_count_aux[1];i++){

    tree_node_aux* temp = links_to_leaves_aux[1][i];
    while (temp->temp!=1){
      cout << temp->temp<<"->";
      //if (temp->left!=NULL) cout<<temp->left->temp<<",";
      //if (temp->right!=NULL) cout <<temp->right->temp<<"->";
      
      temp=temp->parent;
    }
    cout <<temp->left->temp<<","<<temp->right->temp<<endl;
  }

    
  //delete(l);
  return 0;
}
