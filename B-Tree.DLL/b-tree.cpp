#include "b-tree.h"
#include "record.h"
#include "key.h"

//main functions
bool BTree<Key, Record>::search(Record &target) {
	return this->recursive_search(this->root, target);
}
void BTree<Key, Record>::insert(const Record &new_entry) {

}
void BTree<Key, Record>::remove(const Record &target) {

}

//auxiliar functions
template<> bool BTree<Key, Record>::recursive_search(BTreeNode<Key, Record> *current, Record &target) {
	return false;
}
template<> void BTree<Key, Record>::push_down(BTreeNode<Key, Record> *current, const Record &new_entry, Record &median, BTreeNode<Key, Record> * &right_branch) {}
template<> void BTree<Key, Record>::push_in(BTreeNode<Key, Record> *current, const Record &entry, BTreeNode<Key, Record> *right_branch, int position) {}
template<> void BTree<Key, Record>::split_node(
	BTreeNode<Key, Record> *current, // node to be split
	const Record &extra_entry, // new entry to insert
	BTreeNode<Key, Record> *extra_branch, // subtree on right of extra_entry
	int position, // index in node where extra_entry goes
	BTreeNode<Key, Record> * &right_half, // new node for right half of entries
	Record &median) { // median entry (in neither half)
}
template<> void BTree<Key, Record>::recursive_remove(BTreeNode<Key, Record> *current, const Record &target) {}
template<> void BTree<Key, Record>::remove_data(BTreeNode<Key, Record> *current, int position) {}
template<> void BTree<Key, Record>::copy_in_predecessor(BTreeNode<Key, Record> *current, int position) {}
template<> void BTree<Key, Record>::restore(BTreeNode<Key, Record> *current, int position) {}
template<> void BTree<Key, Record>::move_left(BTreeNode<Key, Record> *current, int position) {}
template<> void BTree<Key, Record>::move_right(BTreeNode<Key, Record> *current, int position) {}
template<> void BTree<Key, Record>::combine(BTreeNode<Key, Record> *current, int position) {}