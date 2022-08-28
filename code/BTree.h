#ifndef BTREE_H__
#define BTREE_H__

#include <string>
#include <memory>
#include <vector>

using namespace std;

// The BTREE_ORDER definition sets the B-tree order (using the Knuth
// definition). This is the number of children the node can have. The
// number of keys is one less than this value.
#define BTREE_ORDER 5

// Note that the keys and children arrays are OVERSIZED to allow for
// some approaches to work, where nodes are allowed to temporarily
// have too many keys or children. You do not have to use the extra
// slots if you don't want to. Don't take this as a subtle hint that I
// want you to do it this way.
// 
// A valid btree node can have at most:
//   BTREE_ORDER-1 keys.
//   BTREE_ORDER children.
struct btree {
  // num_keys is the number of in keys array that are currently valid.
  int num_keys;

  // keys is an array of values. valid indexes are in [0..num_keys)
  int keys[BTREE_ORDER];

  // is_leaf is true if this is a leaf, false otherwise
  bool is_leaf;

  // children is an array of pointers to b-tree subtrees. valid
  // indexes are in [0..num_keys].
  shared_ptr<btree> children[BTREE_ORDER + 1];

  //Parent node pointer
  shared_ptr<btree> parent;
};

class BTree {
public:
  // constructor, initialize class variables and pointers here if need.
  BTree();

  // deconstructor, if you are using shared_pointers you don't need to keep track of
  // freeing memory and you can leave this blank
  ~BTree();

  // insert adds the given key into a b-tree rooted at 'root'.  If the
  // key is already contained in the btree this should do nothing.
  // 
  // On exit: 
  // -- the 'root' pointer should refer to the root of the
  //    tree. (the root may change when we insert or remove)
  // -- the btree pointed to by 'root' is valid.
  void insert(shared_ptr<btree> root, int key);

  // remove deletes the given key from a b-tree rooted at 'root'. If the
  // key is not in the btree this should do nothing.
  //
  // On exit:
  // -- the 'root' pointer should refer to the root of the
  //    tree. (the root may change when we insert or delete)
  // -- the btree pointed to by 'root' is valid.
  void remove(shared_ptr<btree> root, int key);

  // find locates the node that either: (a) currently contains this key,
  // or (b) the node that would contain it if we were to try to insert
  // it.  Note that this always returns a non-null node.
  shared_ptr<btree> find(shared_ptr<btree> root, int key);

  // count_nodes returns the number of nodes referenced by this
  // btree. If this node is NULL, count_nodes returns zero; if it is a
  // root, it returns 1; otherwise it returns 1 plus however many nodes
  // are accessable via any valid child links.
  int count_nodes(shared_ptr<btree> root);

  // count_keys returns the total number of keys stored in this
  // btree. If the root node is null it returns zero; otherwise it
  // returns the number of keys in the root plus however many keys are
  // contained in valid child links.
  int count_keys(shared_ptr<btree> root);

  // you can add add more public member variables and member functions here if you need
  shared_ptr<btree> build_node(int size, int* keys);
  shared_ptr<btree> build_small();
  shared_ptr<btree> init_node();
  
  //move to private after testing
  shared_ptr<btree> Build_Node(int size, int *key_array); //function to build new node with one provided key
  

private:
  // you can add add more private member variables and member functions here if you need
  shared_ptr<btree> tree_root;
  
  //Helper function that returns parent
  shared_ptr<btree> Get_Parent(shared_ptr<btree> node);

  //Helper function that returns child at index
  shared_ptr<btree> Get_Child(shared_ptr<btree> node, int child_index);

  //helper function that returns left sibling of node
  shared_ptr<btree> Get_Left_Sibling(shared_ptr<btree> node);

  //helper function that returns right sibling of node
  shared_ptr<btree> Get_Right_Sibling(shared_ptr<btree> node);

  //helper function that returns true if node is full (BTREE_ORDER - 1 keys)
  bool Test_Full(shared_ptr<btree> node);

  //helper function that returns value of successor key
  int Get_Successor(shared_ptr<btree> node, int key_index);
 
  //helper function that adds a key to a passed node; node must be leaf
  int Add_Key(shared_ptr<btree> node, int key);

  //helper function that removes a key from a passed node; node must be leaf
  void Remove_Key(shared_ptr<btree> node, int key);

  //helper function that finds overfull nodes to identify for splitting
  shared_ptr<btree> Get_Full_Node(shared_ptr<btree> root);

  //helper function that searches tree for underfull nodes for merging
  shared_ptr<btree> Get_Underfull_Node(shared_ptr<btree> root);

  //Function that populates each node with parents to allow Get_Parent to work
  void Set_Parents(shared_ptr<btree> root, shared_ptr<btree> parentNode);

  //Function that splits node into two
  shared_ptr<btree> Split_Node(shared_ptr<btree> node);

  //Function that splits node into two without merging into parent--for remove() funciton
  shared_ptr<btree> Remove_Split_Node(shared_ptr<btree> node);
  
  //Function that corrects an underfull node by merging or rotation
  shared_ptr<btree> Fix_Underfull_Node(shared_ptr<btree> node);

  /*function that returns the appropriate method of fixing an underfull node
  0: root node; no fix required
  1: left rotation
  2: right rotation
  3: merge right
  4: merge left
  5: fusion (right sibling and parent) 
  6: fusion (left sibling and parent)*/
  int Determine_Fix(shared_ptr<btree> node);
  
  //function that retrieves the adjacent key index in the parent node
  //LR will be passed to determine left or right adjacency
  int Get_Adjacent_Parent_Key(shared_ptr<btree> node, char LR);

  //function that returns the int index of the key_value in the passed node
  int Get_Key_Index(shared_ptr<btree> node, int key_value);

  //function that removes a key without replacement without the successor
  void Remove_Key_Without_Replacement(shared_ptr<btree> node, int key);

  //function that rotates the btree left at the node
  void Rotate_Left(shared_ptr<btree> node);

  //function that rotates the btree right at the node
  void Rotate_Right(shared_ptr<btree> node);

  //function that merges underfull node with its right sibling and adjacent parent key
  void Merge_Right(shared_ptr<btree> node);

  //function that merges underfull node with its left sibling and adjacent parent key
  void Merge_Left(shared_ptr<btree> node);

  //function that fuses node with its right sibling and two-key parent
  shared_ptr<btree> Fuse_Right(shared_ptr<btree> node);

  //function that fuses node with its left sibling and two-key parent
  shared_ptr<btree> Fuse_Left(shared_ptr<btree> node);

  //Function that fixes an overly tall subtree on the left after fusion
  void Fix_Tall_Node_Left(shared_ptr<btree> parentNode);

  //Function that fixes an overly tall subtree on the right after fusion
  void Fix_Tall_Node_Right(shared_ptr<btree> parentNode);
};

#endif  // BTREE_H__