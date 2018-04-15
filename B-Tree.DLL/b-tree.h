#pragma once
#include "b-tree-node.h"
#include <fstream>

template <typename Key, typename Record, int order>
class BTree {
public:
	//constructor

	//main functions
	bool search(Record &target);
	void insert(const Record &new_entry);
	void remove(const Record &target);
protected:
private:
	bool recursive_search(BTreeNode<Key, Record, order> *current, Record &target);
	void push_down(BTreeNode<Key, Record, order> *current, const Record &new_entry, Record &median, BTreeNode<Key, Record, order> * &right_branch);
	void push_in(BTreeNode<Key, Record, order> *current, const Record &entry, BTreeNode<Key, Record, order> *right_branch, int position);
	void split_node(
		BTreeNode<Key, Record, order> *current, // node to be split
		const Record &extra_entry, // new entry to insert
		BTreeNode<Key, Record, order> *extra_branch, // subtree on right of extra_entry
		int position, // index in node where extra_entry goes
		BTreeNode<Key, Record, order> * &right_half, // new node for right half of entries
		Record &median); // median entry (in neither half)
	void recursive_remove(BTreeNode<Key, Record, order> *current, const Record &target);
	void remove_data(BTreeNode<Key, Record, order> *current, int position);
	void copy_in_predecessor(BTreeNode<Key, Record, order> *current, int position);
	void restore(BTreeNode<Key, Record, order> *current, int position);
	void move_left(BTreeNode<Key, Record, order> *current, int position);
	void move_right(BTreeNode<Key, Record, order> *current, int position);
	void combine(BTreeNode<Key, Record, order> *current, int position);

	BTreeNode<Key, Record, order> *root;
};