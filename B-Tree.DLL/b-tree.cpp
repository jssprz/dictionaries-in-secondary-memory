#include "b-tree.h"
#include "record.h"
#include "key.h"

//main functions
bool BTree<Key, Record, 20>::search(Record &target) {
	return false;
}
void BTree<Key, Record, 20>::insert(const Record &new_entry) {

}
void BTree<Key, Record, 20>::remove(const Record &target) {

}

//auxiliar functions
bool BTree<Key, Record, 20>::recursive_search(BTreeNode<Key, Record, 20> *current, Record &target) {
	return false;
}
void BTree<Key, Record, 20>::push_down(BTreeNode<Key, Record, 20> *current, const Record &new_entry, Record &median, BTreeNode<Key, Record, 20> * &right_branch) {}
void BTree<Key, Record, 20>::push_in(BTreeNode<Key, Record, 20> *current, const Record &entry, BTreeNode<Key, Record, 20> *right_branch, int position) {}
void BTree<Key, Record, 20>::split_node(
	BTreeNode<Key, Record, 20> *current, // node to be split
	const Record &extra_entry, // new entry to insert
	BTreeNode<Key, Record, 20> *extra_branch, // subtree on right of extra_entry
	int position, // index in node where extra_entry goes
	BTreeNode<Key, Record, 20> * &right_half, // new node for right half of entries
	Record &median) { // median entry (in neither half)
}
void BTree<Key, Record, 20>::recursive_remove(BTreeNode<Key, Record, 20> *current, const Record &target) {}
void BTree<Key, Record, 20>::remove_data(BTreeNode<Key, Record, 20> *current, int position) {}
void BTree<Key, Record, 20>::copy_in_predecessor(BTreeNode<Key, Record, 20> *current, int position) {}
void BTree<Key, Record, 20>::restore(BTreeNode<Key, Record, 20> *current, int position) {}
void BTree<Key, Record, 20>::move_left(BTreeNode<Key, Record, 20> *current, int position) {}
void BTree<Key, Record, 20>::move_right(BTreeNode<Key, Record, 20> *current, int position) {}
void BTree<Key, Record, 20>::combine(BTreeNode<Key, Record, 20> *current, int position) {}