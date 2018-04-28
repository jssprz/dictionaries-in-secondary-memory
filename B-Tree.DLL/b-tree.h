#pragma once

#include "b-tree-node.h"
#include "error-code.h"
#include <fstream>

template <typename K, typename R>
class BTree {
public:
	typedef typename K::Key K;
	typedef typename R::Record R;
	typedef BTreeNode<K, R> Node;

	//constructor
	BTree(rsize_t branchingFactor):order(branchingFactor) {
	}

	//main functions
	Error_code search(R &target){
		return recursive_search_tree(root, target);
	}
	void insert(const R &new_entry){}
	void remove(const R &target){}

protected:
private:
	Error_code recursive_search(Node *current, R &target){
		Error_code result = not_present;
		size_t position;
		if (current != NULL) {
			result = search_node(current, target, position);
			if (result == not_present)
				result = recursive_search_tree(current->branch[position], target);
			else
				target = current->data[position];
		}
		return result;
	}

	Error_code search_node(Node *current, R &target, size_t &position) {
		position = 0;
		while (position < current->count && target > current->data[position])
			position++; // Perform a sequential search through the keys.
		position < current->count && target == current->data[position] ? return success : not_present;
	}
	
	void push_down(Node *current, const R &new_entry, R &median, Node * &right_branch){}
	
	void push_in(Node *current, const R &entry, Node *right_branch, int position){}
	
	void split_node(
		Node *current, // node to be split
		const R &extra_entry, // new entry to insert
		Node *extra_branch, // subtree on right of extra_entry
		int position, // index in node where extra_entry goes
		Node * &right_half, // new node for right half of entries
		R &median) // median entry (in neither half)
	{}
	
	void recursive_remove(Node *current, const R &target){}
	
	void remove_data(Node *current, int position){}
	
	void copy_in_predecessor(Node *current, int position){}
	
	void restore(Node *current, int position){}
	
	void move_left(Node *current, int position){}
	
	void move_right(Node *current, int position){}
	
	void combine(Node *current, int position){}
	
	Node *root;
	rsize_t order;
};