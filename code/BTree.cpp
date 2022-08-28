#include "BTree.h"
#include <iostream>

// constructor, initialize class variables and pointers here if need.
BTree::BTree(){
  // Your code here
}

//deconstructor,
BTree::~BTree(){
}

void BTree::insert(shared_ptr<btree> root, int key){
  Set_Parents(root, NULL); //give each node their proper parent pointer for use in other functions
  shared_ptr<btree> targetNode = find(root, key); //locate correct node for placement of key
  Add_Key(targetNode, key); //add key to correct leaf node

  //fix any issues by splitting overfull nodes
  shared_ptr<btree> fullNode = Get_Full_Node(root);
  while(fullNode != NULL){
    shared_ptr<btree> postSplit = Split_Node(fullNode);
    if (fullNode == root){
      *root = *postSplit;
    }
    fullNode = Get_Full_Node(root);
  }
}

void BTree::remove(shared_ptr<btree> root, int key){
  if (root == NULL || root->num_keys == 0 && root->is_leaf){
    return;
  }
  Set_Parents(root, NULL); //give each node their proper parent pointer for use in other functions
  shared_ptr<btree> targetNode = find(root, key); //locate correct node for placement of key
  Remove_Key(targetNode, key); //remove key from appropriate node 

  //now we have to search for underfull nodes and fix them
  shared_ptr<btree> underfullNode = Get_Underfull_Node(root);
  while (underfullNode != NULL){ //TODO: Change to while loop
    //shared_ptr<btree> postSplit = Fix_Underfull_Node(underfullNode);
    //cout << "Identified underfull node. Begins with key: " << underfullNode->keys[0] << endl;
    int fix_type = Determine_Fix(underfullNode);
    //cout << "Type of fix required: " << fix_type << endl;
    if (fix_type == 1){
      Rotate_Left(underfullNode);
      underfullNode = Get_Underfull_Node(root);
    }
    else if (fix_type == 2){
      Rotate_Right(underfullNode);
      underfullNode = Get_Underfull_Node(root);
    }
    else if (fix_type == 3){
      Merge_Right(underfullNode);
      underfullNode = Get_Underfull_Node(root);
    }
    else if (fix_type == 4){
      Merge_Left(underfullNode);
      underfullNode = Get_Underfull_Node(root);
    }
    else if (fix_type == 5){
      shared_ptr<btree> postSplit = Fuse_Right(underfullNode);
      if (postSplit->parent == NULL){
        *root = *postSplit;
      }
      underfullNode = Get_Underfull_Node(root);
    }
    else if (fix_type == 6){
      shared_ptr<btree> postSplit = Fuse_Left(underfullNode);
      if (postSplit->parent == NULL){
        *root = *postSplit;
      }
      underfullNode = Get_Underfull_Node(root);
    }
    else{
      underfullNode = NULL;//underfull node is root; do nothing
    }
  }
}

shared_ptr<btree> BTree::find(shared_ptr<btree> root, int key){
  if (root->is_leaf){//recursive base case; if node is a leaf, return this node
    return root;
  }
  for (int i = 0; i < root->num_keys; i++){ //iterate through keys looking for match; if found, return current node
    if (root->keys[i] == key){
      return root;
    }
  }
  for (int i = 0; i < root->num_keys; i++){ //iterate through keys, checking search value against keys
    if (root->num_keys == i || key < root->keys[i]){ //if iteration reaches last key or is less than next key, search that child
      return find(root->children[i], key);
    }
  }
  return find(root->children[root->num_keys], key); //otherwise search the child just beyond the last key
}

int BTree::count_nodes(shared_ptr<btree> root){
  int node_count = 1; //count the root
  if (root->is_leaf){//recursive base case; if node is a leaf, return 1
    return 1;
  }
  for (int i = 0; i <= root->num_keys; i++){ //iterate through each child, recursively counting nodes
    node_count = node_count + count_nodes(root->children[i]);
  }
  return node_count;
}

int BTree::count_keys(shared_ptr<btree> root){
  int key_count = root->num_keys; //initialize count
  if (root->is_leaf){
    return key_count; //if leaf, return number of keys (num_keys)
  }
  else{
    for (int i = 0; i <= root->num_keys; i++){ //if not leaf, iterate through children recursively, returning key counts
      key_count = key_count + count_keys(root->children[i]);
    }
  }
  return key_count;
}

shared_ptr<btree> BTree::Build_Node(int size, int *key_array){ //to build a new node with supplied keys
    shared_ptr<btree> newNode = shared_ptr<btree> (new btree);
    newNode->num_keys = 0; //initialize num_keys to zero to prepare for Add_Key()
    for (int i = 0; i < BTREE_ORDER; i++){ //initialize keys to zero to prepare for Add_Key()
      newNode->keys[i] = 0;
    }
    for (int i = 0; i < size; i++){
      Add_Key(newNode, key_array[i]);
    }
    for (int i = 0; i < BTREE_ORDER; i++){
      newNode->children[i] = NULL;
    }
    newNode->parent = NULL;
    newNode->is_leaf = true;
    return newNode;
  }

//Helper function that returns parent
shared_ptr<btree> BTree::Get_Parent(shared_ptr<btree> node){
  return node->parent;
}

shared_ptr<btree> BTree::Get_Child(shared_ptr<btree> node, int child_index){
  return node->children[child_index];
}

shared_ptr<btree> BTree::Get_Left_Sibling(shared_ptr<btree> node){
  shared_ptr<btree> parent = Get_Parent(node);
  if (parent->children[0] == node){ //if this is the leftmost child, then there is no left sibling, return NULL
    return NULL;
  }
  for (int i = 1; i < parent->num_keys + 1; i++){ //iterate through parent's children
    if (parent->children[i] == node){ //when node found, left sibling is the previous child
      return parent->children[i-1];
    }
  }
  return NULL;
}

shared_ptr<btree> BTree::Get_Right_Sibling(shared_ptr<btree> node){
  shared_ptr<btree> parent = Get_Parent(node);
  if (parent->children[parent->num_keys] == node){ //if this is the rightmost child, then there is no right sibling, return NULL
    return NULL;
  }
  for (int i = 0; i < parent->num_keys; i++){ //iterate through parent's children
    if (parent->children[i] == node){ //when node found, right sibling is the next child
      return parent->children[i+1];
    }
  }
  return NULL;
}

bool BTree::Test_Full(shared_ptr<btree> node){
  if (node == NULL){
    return false;
  }
  else if (node->num_keys >= BTREE_ORDER){ 
    return true;
  }
  return false;
}

int BTree::Get_Successor(shared_ptr<btree> node, int key_index){
  shared_ptr<btree> current = node;
  current = current->children[key_index+1]; //set current to child to right of key
  while (!current->is_leaf){ //follow leftmost child until reaching leaf
    current = current->children[0];
  }
  int successor_key = current->keys[0];
  Remove_Key(current, successor_key); //remove successor key from leaf
  return successor_key; //return minimum key from leftmost leaf on rightmost path
}

int BTree::Add_Key(shared_ptr<btree> node, int key){
  if (node->num_keys == 0){ //if node is empty, add the key to index 0
    node->keys[0] = key;
    node->num_keys++;
    return 0;
  }
  else{ //if node isn't empty, more checks are required
    for (int i = 0; i < node->num_keys; i++){ //iterate through keys to find correct slot
      if (node->keys[i] == key){
        return i; //if key exists, don't add it
      } 
      else if (node->keys[i] > key){
        for (int j = node->num_keys; j > i; j--){ //starting from back end, copy keys right
          node->keys[j] = node->keys[j-1];
        }
        node->keys[i] = key; //place key in correct slot
        node->num_keys++; //increment num_keys to track available slots
        return i;
      }
    }
    if (key > node->keys[node->num_keys-1]){
      node->keys[node->num_keys] = key;
      node->num_keys++;
      return node->num_keys - 1;
    }
  }
}

void BTree::Remove_Key(shared_ptr<btree> node, int key){
  //Find key in node
  int key_index = -1;
  for (int i = 0; i < node->num_keys; i++){
    if (node->keys[i] == key){
      key_index = i;
    }
  }
  if (key_index < 0){ //if key not found
    return;
  }
  
  //Case 1: node is a leaf
  else if(node->is_leaf){ 
    for (int j = key_index; j < node->num_keys - 1; j++){ //iterate through keys starting from target key, setting each to subsequent key
      node->keys[j] = node->keys[j+1];
    }
    node->keys[node->num_keys-1] = 0; //set last key to 0 for debugging purposes
    node->num_keys--; //decrement number of keys
  }
  //Case 2: node is internal
  else{
    int successor_key = Get_Successor(node, key_index);
    node->keys[key_index] = successor_key;
  }
}

shared_ptr<btree> BTree::Get_Full_Node(shared_ptr<btree> root){
  shared_ptr<btree> currentNode = NULL;
  if (Test_Full(root)){
    currentNode = root;
  }
  else if (root != NULL){
    for (int i = 0; i <= root->num_keys; i++){ //iterate through keys within node
      currentNode = Get_Full_Node(root->children[i]); //recursively search for full nodes
      if (Test_Full(currentNode)){
        return currentNode;
      }
      } 
  }
  return currentNode;
}

shared_ptr<btree> BTree::Get_Underfull_Node(shared_ptr<btree> root){
  shared_ptr<btree> currentNode = NULL;
  if (root && root->parent && root->num_keys < (BTREE_ORDER/2)){ //if node is underfull
    currentNode = root;
  }
  else if (root != NULL){
    for (int i = 0; i <= root->num_keys; i++){ //iterate through keys within node
      currentNode = Get_Underfull_Node(root->children[i]); //recursively search for underfull nodes
      if (currentNode && currentNode->num_keys < (BTREE_ORDER/2)){
        return currentNode;
      }
      } 
  }
  return currentNode;
}

void BTree::Set_Parents(shared_ptr<btree> root, shared_ptr<btree> parentNode){
  if (root == NULL){ //skip if node is NULL
    return;
  }
  if (root->is_leaf){//recursive base case; if node is a leaf, set parent and return
    root->parent = parentNode;
    return;
  }
  for (int i = 0; i < root->num_keys + 1; i++){ //iterate through children
      Set_Parents(root->children[i], root);
  }
  root->parent = parentNode;
}


shared_ptr<btree> BTree::Split_Node(shared_ptr<btree> node){ //will only be used on a known overfull node (5 keys)
  //TO DO: Build this so it works for any value of BTREE_ORDER
  int leftKeys[2]; //copy left keys
  leftKeys[0] = node->keys[0];
  leftKeys[1] = node->keys[1];
  int rightKeys[2]; //copy right keys
  rightKeys[0] = node->keys[3];
  rightKeys[1] = node->keys[4];
  
  shared_ptr<btree> newLeft = Build_Node(2, leftKeys); //build new left node
  shared_ptr<btree> newRight = Build_Node(2, rightKeys); //build new right node

  //transfer children if node is not leaf
  if (!node->is_leaf){
    bool leafTest = true;
    newLeft->children[0] = node->children[0];
    newLeft->children[1] = node->children[1];
    newLeft->children[2] = node->children[2];
    for (int i = 0; i < 3; i++){ //iterate through children sent to newLeft node; if any non-NULL, set is_leaf to false
      if (newLeft->children[i] != NULL){
        leafTest = false;
      }
    }
    newLeft->is_leaf = leafTest;
    newRight->children[0] = node->children[3];
    newRight->children[1] = node->children[4];
    newRight->children[2] = node->children[5];
    leafTest = true;
    for (int i = 0; i < 3; i++){ //iterate through children sent to newRight node; if any non-NULL, set is_leaf to false
      if (newRight->children[i] != NULL){
        leafTest = false;
      }
    }
    newRight->is_leaf = leafTest;
  }
  
  shared_ptr<btree> parent = node->parent; //for legibility
  if (parent == NULL){ //if this is the root node, make a new node
    int add_key[1];
    add_key[0] = node->keys[2];
    parent = Build_Node(1, add_key);
    parent->children[0] = newLeft; //set split nodes to children
    parent->children[1] = newRight;
    parent->is_leaf = false;
  }
  else{
    int key_index = Add_Key(parent, node->keys[2]); //add central key to parent node and return index
    for (int i = parent->num_keys; i > key_index + 1; i--){ //copy children over until reaching child to right of new key
      parent->children[i] = parent->children[i-1]; 
    }
    parent->children[key_index + 1] = newRight;
    parent->children[key_index] = newLeft;
  }
  return parent;
}

shared_ptr<btree> BTree::Remove_Split_Node(shared_ptr<btree> node){
  int leftKeys[2]; //copy left keys
  leftKeys[0] = node->keys[0];
  leftKeys[1] = node->keys[1];
  int rightKeys[2]; //copy right keys
  rightKeys[0] = node->keys[3];
  rightKeys[1] = node->keys[4];
  
  shared_ptr<btree> newLeft = Build_Node(2, leftKeys); //build new left node
  shared_ptr<btree> newRight = Build_Node(2, rightKeys); //build new right node

  //transfer children
  newLeft->children[0] = node->children[0];
  newLeft->children[1] = node->children[1];
  newLeft->children[2] = node->children[2];
  
  newRight->children[0] = node->children[3];
  newRight->children[1] = node->children[4];
  newRight->children[2] = node->children[5];

  //test for leafiness and fix
  for (int i = 0; i < 3; i++){
    if (newLeft->children[i] != NULL){
      newLeft->is_leaf = false;
    }
    if (newRight->children[i] != NULL){
      newRight->is_leaf = false;
    }
  }
  
  //remove keys that are now part of children
  Remove_Key_Without_Replacement(node, node->keys[4]);
  Remove_Key_Without_Replacement(node, node->keys[3]);
  Remove_Key_Without_Replacement(node, node->keys[0]);
  Remove_Key_Without_Replacement(node, node->keys[0]);

  //fix child pointers
  node->children[0] = newLeft;
  node->children[1] = newRight;

  return node;
}

shared_ptr<btree> BTree::Fix_Underfull_Node(shared_ptr<btree> node){
  //case 1: node is a leaf
}

int BTree::Determine_Fix(shared_ptr<btree> node){
  if (node->parent == NULL){ //if this is the root, no need to fix
    return 0;
  }
  //determine number of keys on left sibling
  int num_keys_left = 0; //initialize to 0
  if (Get_Left_Sibling(node)){ //if left sibling isn't NULL, retrieve number of keys
    num_keys_left = Get_Left_Sibling(node)->num_keys;
  }
  //determine number of keys on right sibling
  int num_keys_right = 0; //initialize to 0
  if (Get_Right_Sibling(node)){
    num_keys_right = Get_Right_Sibling(node)->num_keys;
  }
  //determine number of keys on parent sibling
  int num_keys_parent = 0;
  if (node->parent){
    num_keys_parent = node->parent->num_keys;
  }

  //Use these values to determine the appropriate fix
  
  if (num_keys_right > 2){ //if there are extra keys to the right, perform left rotation
    return 1;
  }
  else if (num_keys_left > 2){ //if there are extra keys to the left, perform right rotation
    return 2;
  }
  else if (num_keys_parent > 2){ //if insufficient keys to left and right, but extra in parent
    if (num_keys_right > 0){ //there is a node to the right to merge with
      return 3; //merge with right sibling
    }
    else{
      return 4; //merge with left sibling
    }
  }
  else{ //failing that, the only possibility is a 1- or 2- key parent and 2-key sibling
    if (num_keys_right > 0){
      return 5; //fuse with parent and right sibling
    }
    else {
      return 6; //fuse with parent and left sibling
    }

  }
} //closing bracket for Determine_Fix

int BTree::Get_Adjacent_Parent_Key(shared_ptr<btree> node, char LR){
  shared_ptr<btree> parentNode = Get_Parent(node);
  for (int i = 0; i <= parentNode->num_keys; i++){ //iterate through children in parent
    if (parentNode->children[i] == node){
      if (LR == 'L'){ //if looking for left adjacent key, return i-1
        return parentNode->keys[i - 1];
      }
      else{
        return parentNode->keys[i];
      }
    }
  }
}

int BTree::Get_Key_Index(shared_ptr<btree> node, int key_value){
  for (int i = 0; i < node->num_keys; i++){ //iterate through keys; once key value is found, return the index
    if (node->keys[i] == key_value){
      return i;
    }
  }
  return -1; //return -1 if not found for debugging purposes
}

void BTree::Remove_Key_Without_Replacement(shared_ptr<btree> node, int key){
  //Find key in node
  int key_index = Get_Key_Index(node, key);
 
  //Remove key
  for (int j = key_index; j < node->num_keys - 1; j++){ //iterate through keys starting from target key, setting each to subsequent key
    node->keys[j] = node->keys[j+1];
    node->children[j+1] = node->children[j+2]; //also shift children down to close gap
  }
  node->keys[node->num_keys - 1] = 0; //set last key to 0 for debugging purposes
  node->children[node->num_keys] = NULL; //set last child to NULL
  node->num_keys--; //decrement number of keys
}

void BTree::Rotate_Left(shared_ptr<btree> node){
  shared_ptr<btree> parentNode = node->parent;
  int parentKey = Get_Adjacent_Parent_Key(node, 'R'); //retrieve key to be rotated from parent and index
  int parentKeyIndex = Get_Key_Index(parentNode, parentKey);

  shared_ptr<btree> rightSibling = Get_Right_Sibling(node);
  int siblingKey = rightSibling->keys[0]; //retrieve key to be rotated into parent node

  //add parent key to rotation node
  Add_Key(node, parentKey);

  //add left child of right sibling to rotation node
  node->children[node->num_keys] = rightSibling->children[0];

  //replace parent key with sibling key
  parentNode->keys[parentKeyIndex] = siblingKey;

  //remove sibling key
  Remove_Key_Without_Replacement(rightSibling, siblingKey);

  //restore parentage to subtree
  Set_Parents(parentNode, parentNode->parent);
}

void BTree::Rotate_Right(shared_ptr<btree> node){
  shared_ptr<btree> parentNode = node->parent;
  int parentKey = Get_Adjacent_Parent_Key(node, 'L'); //retrieve key to be rotated from parent and index
  int parentKeyIndex = Get_Key_Index(parentNode, parentKey);

  shared_ptr<btree> leftSibling = Get_Left_Sibling(node);
  int siblingKey = leftSibling->keys[leftSibling->num_keys-1]; //retrieve key to be rotated into parent node

  //add parent key to rotation node
  Add_Key(node, parentKey);
  
  //shift children right
  for (int i = node->num_keys; i > 0; i--){
    node->children[i] = node->children[i-1];
  }

  //add right child of left sibling to rotation node
  node->children[0] = leftSibling->children[leftSibling->num_keys];

  //replace parent key with sibling key
  parentNode->keys[parentKeyIndex] = siblingKey;

  //remove sibling key
  Remove_Key_Without_Replacement(leftSibling, siblingKey);

  //restore parentage to subtree
  Set_Parents(parentNode, parentNode->parent);
}

void BTree::Merge_Right(shared_ptr<btree> node){
  //identify parent and adjacent key
  shared_ptr<btree> parentNode = Get_Parent(node);
  int parentKey = Get_Adjacent_Parent_Key(node, 'R'); //retrieve key to be rotated from parent and index
  int parentKeyIndex = Get_Key_Index(parentNode, parentKey);

  //identify right sibling (which will always have two keys)
  shared_ptr<btree> rightSibling = Get_Right_Sibling(node);
  
  //make room for two additional children in right sibling
  rightSibling->children[4] = rightSibling->children[2];
  rightSibling->children[3] = rightSibling->children[1];
  rightSibling->children[2] = rightSibling->children[0];

  //move parent key into right sibling
  Add_Key(rightSibling, parentKey);

  //move left sibling key and children into right sibling
  Add_Key(rightSibling, node->keys[0]);
  rightSibling->children[1] = node->children[1];
  rightSibling->children[0] = node->children[0];

  //remove parent key from parent
  Remove_Key_Without_Replacement(parentNode, parentKey);

  //make parent child point to right sibling
  parentNode->children[parentKeyIndex] = rightSibling;

  //restore parentage to subtree
  Set_Parents(parentNode, parentNode->parent);
}

void BTree::Merge_Left(shared_ptr<btree> node){
  //identify parent and adjacent key
  shared_ptr<btree> parentNode = Get_Parent(node);
  int parentKey = Get_Adjacent_Parent_Key(node, 'L'); //retrieve key to be rotated from parent and index
  int parentKeyIndex = Get_Key_Index(parentNode, parentKey);

  ///identify left sibling (which will always have two keys)
  shared_ptr<btree> leftSibling = Get_Left_Sibling(node);

  //move parent key into left sibling
  Add_Key(leftSibling, parentKey);

  //move node key and children into left sibling
  Add_Key(leftSibling, node->keys[0]);
  leftSibling->children[3] = node->children[0];
  leftSibling->children[4] = node->children[1];

  //remove parent key from parent
  Remove_Key_Without_Replacement(parentNode, parentKey);

  //restore parentage to subtree
  Set_Parents(parentNode, parentNode->parent);
}

shared_ptr<btree> BTree::Fuse_Right(shared_ptr<btree> node){
  //identify parent and adjacent key
  shared_ptr<btree> parentNode = Get_Parent(node);
  int parentKey = Get_Adjacent_Parent_Key(node, 'R'); //retrieve key to be rotated from parent and index
  int parentKeyIndex = Get_Key_Index(parentNode, parentKey);

  //identify right sibling (which will always have two keys)
  shared_ptr<btree> rightSibling = Get_Right_Sibling(node);

  //Add node key to parent node
  Add_Key(parentNode, node->keys[0]);

  //Add keys from right sibling to parent
  Add_Key(parentNode, rightSibling->keys[0]);
  Add_Key(parentNode, rightSibling->keys[1]);

  //fix children of parent node
  if (parentKeyIndex == 0){ //case 1: node is left child of parent
    parentNode->children[5] = parentNode->children[2]; //push rightmost child to new rightmost child slot
    parentNode->children[4] = rightSibling->children[2]; //copy right sibling's children into parent node
    parentNode->children[3] = rightSibling->children[1];
    parentNode->children[2] = rightSibling->children[0];
    parentNode->children[1] = node->children[1]; //copy node's children into parent node
    parentNode->children[0] = node->children[0];
  }
  else { //case 2: node is middle child of parent
    //child 0 remains in place
    parentNode->children[1] = node->children[0]; //copy children from node
    parentNode->children[2] = node->children[1];
    parentNode->children[3] = rightSibling->children[0]; //copy children from right sibling
    parentNode->children[4] = rightSibling->children[1]; 
    parentNode->children[5] = rightSibling->children[2];
  }

  ///If the parent node is overfull, split it
  if (parentNode->num_keys == 5){
    parentNode = Remove_Split_Node(parentNode);
    
    //fix erroneous leaf designation

    //restore parentage to subtree
    Set_Parents(parentNode, parentNode->parent);

    //there is now an extra level on the far left or right subtree
    if (parentKeyIndex == 0){
      Fix_Tall_Node_Right(parentNode);
    }
    else{
      Fix_Tall_Node_Left(parentNode);
    }

  }

  else{
    //restore parentage to subtree in case the 
    Set_Parents(parentNode, parentNode->parent);
  }
  return parentNode;
}

shared_ptr<btree> BTree::Fuse_Left(shared_ptr<btree> node){
  //identify parent and adjacent key
  shared_ptr<btree> parentNode = Get_Parent(node);
  int parentKey = Get_Adjacent_Parent_Key(node, 'L'); //retrieve key to be rotated from parent and index
  int parentKeyIndex = Get_Key_Index(parentNode, parentKey);

  //identify left sibling (which will always have two keys)
  shared_ptr<btree> leftSibling = Get_Left_Sibling(node);

  //Add node key to parent node
  Add_Key(parentNode, node->keys[0]);

  //Add keys from left sibling to parent
  Add_Key(parentNode, leftSibling->keys[0]);
  Add_Key(parentNode, leftSibling->keys[1]);

  //fix children of parent node
  if (parentKeyIndex == 0){ //case 1: node is left child of parent
    parentNode->children[5] = parentNode->children[2]; //push rightmost child to new rightmost child slot
    parentNode->children[4] = node->children[1]; //copy node's children
    parentNode->children[3] = node->children[0];
    parentNode->children[2] = leftSibling->children[2]; //copy left sibling's chidlren into parent node
    parentNode->children[1] = leftSibling->children[1];
    parentNode->children[0] = leftSibling->children[0];
  }
  else { //case 2: node is middle child of parent
    //child 0 remains in place
    parentNode->children[1] = leftSibling->children[0]; //copy children from left sibling
    parentNode->children[2] = leftSibling->children[1];
    parentNode->children[3] = leftSibling->children[2]; 
    parentNode->children[4] = node->children[0]; //copy children from node
    parentNode->children[5] = node->children[1];
  }
  
  //If the parent node is overfull, split it; this will happen unless parentNode was the root
    if (parentNode->num_keys == 5){
    parentNode = Remove_Split_Node(parentNode);
  

    //restore parentage to subtree
    Set_Parents(parentNode, parentNode->parent);

    //there is now an extra level on the far left or right subtree...
    if (parentKeyIndex == 0){
      Fix_Tall_Node_Right(parentNode);
    }
    else{
      Fix_Tall_Node_Left(parentNode);
    }
  }

  else{
    //restore parentage to subtree in case the 
    Set_Parents(parentNode, parentNode->parent);
  }

  return parentNode;
}

void BTree::Fix_Tall_Node_Left(shared_ptr<btree> parentNode){
    shared_ptr<btree> tallNode = parentNode->children[0]->children[0];
    shared_ptr<btree> middleNode = tallNode->parent;
    shared_ptr<btree> rightSibling = Get_Right_Sibling(middleNode);

    //merge that node and the 3-split node to remove the extra level
    if (tallNode->num_keys == 4){ //case 1: tallNode has 4 keys
      Add_Key(parentNode, tallNode->keys[3]); //insert last key from tallNode into parent
      parentNode->children[2] = parentNode->children[1]; //move children right to make room to pull tallNode up
      parentNode->children[1] = parentNode->children[0];
      parentNode->children[0] = tallNode; //tallNode becomes first child
      middleNode->children[0] = tallNode->children[4]; //last child of tallNode replaces tallNode in the now middle node
      Remove_Key_Without_Replacement(tallNode, tallNode->keys[3]); //remove extra key from tallNode
    }
    else if (tallNode->num_keys == 3){ //case 2: tallNode has 3 keys
      Add_Key(parentNode, tallNode->keys[2]); //insert last key from tallNode into parent
      parentNode->children[2] = parentNode->children[1]; //move children right to make room to pull tallNode up
      parentNode->children[1] = parentNode->children[0];
      parentNode->children[0] = tallNode; //tallNode becomes first child
      middleNode->children[0] = tallNode->children[3]; //last child of tallNode replaces tallNode in the now middle node
      Remove_Key_Without_Replacement(tallNode, tallNode->keys[2]); //remove extra key from tallNode
    }
    else { //case 3: tallNode has 2 keys
      Add_Key(rightSibling, parentNode->keys[0]); //add parent node key to right sibling
      rightSibling->children[2] = rightSibling->children[1]; //move children right to make room
      rightSibling->children[1] = rightSibling->children[0];
      rightSibling->children[0] = middleNode->children[2]; //shift last child of left-sibling-to-be to right sibling
      parentNode->keys[0] = middleNode->keys[1]; //copy right key of middleNode and overwrite parent key
      middleNode->keys[1] = middleNode->keys[0]; //shift first key to right
      middleNode->keys[0] = tallNode->keys[1]; //overwrite first key in middleNode with tallNode key 1
      Add_Key(middleNode, tallNode->keys[0]);
      middleNode->children[3] = middleNode->children[1]; //fix all the children
      middleNode->children[2] = tallNode->children[2];
      middleNode->children[1] = tallNode->children[1];
      middleNode->children[0] = tallNode->children[0];
      middleNode->is_leaf = tallNode->is_leaf; //copy leaf status from tallNode to middleNode
    }
}

void BTree::Fix_Tall_Node_Right(shared_ptr<btree> parentNode){
    shared_ptr<btree> tallNode = parentNode->children[1]->children[2];
    shared_ptr<btree> middleNode = tallNode->parent;
    shared_ptr<btree> leftSibling = Get_Left_Sibling(middleNode);
  
    //merge that node and the 3-split node to remove the extra level
    if (tallNode->num_keys == 4){ //case 1: tallNode has 4 keys
      Add_Key(parentNode, tallNode->keys[0]);
      middleNode->children[2] = tallNode->children[0]; //move leftmost child to middleNode
      Remove_Key_Without_Replacement(tallNode, tallNode->keys[0]);
      parentNode->children[2] = tallNode;
      tallNode->parent = parentNode;
    }
    else if (tallNode->num_keys == 3){ //case 2: tallNode has 3 keys
      Add_Key(parentNode, tallNode->keys[0]);
      middleNode->children[2] = tallNode->children[0]; //move leftmost child to middleNode
      Remove_Key_Without_Replacement(tallNode, tallNode->keys[0]);
      parentNode->children[2] = tallNode;
      tallNode->parent = parentNode;
    }
    else { //case 3: tallNode has 2 keys
      Add_Key(leftSibling, parentNode->keys[0]); //move parent key into left node
      parentNode->keys[0] = middleNode->keys[0]; //move least key of right node into parent
      leftSibling->children[2] = middleNode->children[0]; //move least child of right node to left
      Remove_Key_Without_Replacement(middleNode, middleNode->keys[0]); //remove key that is now in parent node
      Add_Key(middleNode, tallNode->keys[0]); //move both keys from tallNode to right child
      Add_Key(middleNode, tallNode->keys[1]);
      //move children from tallNode into the new right node
      middleNode->children[1] = tallNode->children[0];
      middleNode->children[2] = tallNode->children[1];
      middleNode->children[3] = tallNode->children[2];
      middleNode->is_leaf = tallNode->is_leaf;
    }
}

/* For testing purposes ONLY */

shared_ptr<btree> BTree::build_small() {
  //       root
  //      10   20
  //    /    |    \
  //  2,8  13,17  24,28
  // left   mid    right
  
  int vals[] = { 10, 20 };
  shared_ptr<btree> root = build_node(2, vals);
  // now we need three children
  int vals2[] = { 2, 8 };
  shared_ptr<btree> left = build_node(2, vals2);
  int vals3[] = { 13, 17 };
  shared_ptr<btree> mid =  build_node(2, vals3);
  int vals4[] = { 24, 28 };
  shared_ptr<btree> right = build_node(2, vals4);
  root->is_leaf = false;
  root->children[0] = left;
  root->children[1] = mid;
  root->children[2] = right;
  return root;
}/*Tree testTree;
    int vals[2] = {15, 30};
    testTree.build_node(2, vals);*/


shared_ptr<btree> BTree::build_node(int size, int* keys) {
  shared_ptr<btree> node = init_node();
  node->num_keys = size;
  for (int i=0; i < node->num_keys; i++) {
    node->keys[i] = keys[i];
  }
  return node;
}

shared_ptr<btree> BTree::init_node() {
  shared_ptr<btree> ret = shared_ptr<btree>(new btree);
  ret->num_keys = 0;
  ret->is_leaf = true;
  for (int i=0; i <= BTREE_ORDER; i++) {
    ret->children[i] = NULL;
  }
  return ret;
}

