#include <iostream>
#include "../code/BTree.h"
#include "../tests/BTreeTestHelp.h"

using namespace std;

int main(){

  BTree testTree;

  shared_ptr<btree> root = testTree.build_small();
  /*testTree.insert(root, 9);
  testTree.insert(root, 9);
  testTree.insert(root, 1);
  testTree.insert(root, 7);
  testTree.insert(root, 26);
  testTree.insert(root, 29);
  testTree.insert(root, 23);
  testTree.insert(root, 30);
  testTree.insert(root, 45);*/
  
  testTree.remove(root, 2);
  return 0;
}
