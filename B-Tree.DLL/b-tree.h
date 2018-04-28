#pragma once

#include "b-tree-node.h"
#include "error-code.h"

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

	void insert(const R &new_entry){
		R median;
		Node *right_branch, *new_root;
		Error_code result = push_down(root, new_entry, median, right_branch);
		if (result == overflow) { // The whole tree grows in height.
								  // Make a brand new root for the whole B-tree.
			new_root = new B_node<Record, order>;
			new_root->count = 1;
			new_root->data[0] = median;
			new_root->branch[0] = root;
			new_root->branch[1] = right_branch;
			root = new_root;
			result = success;
		}
		return result;
	}

	Error_code remove(const R &target){
		Error_code result;
		result = recursive_remove(root, target);
		if (root != NULL && root->count == 0) { // root is now empty.
			Node *old_root = root;
			root = root->branch[0];
			delete old_root;
		}
		return result;
	}

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
		return position < current->count && target == current->data[position] ? success : not_present;
	}
	
	Error_code push_down(Node *current, const R &new_entry, R &median, Node * &right_branch){
		Error_code result;
		int position;
		if (current == NULL) {
			// Since we cannot insert in an empty tree, the recursion terminates.
			median = new_entry;
			right_branch = NULL;
			result = overflow;
		}
		else { // Search the current node.
			if (search_node(current, new_entry, position) == success)
				result = duplicate_error;
			else {
				R extra_entry;
				Node *extra_branch;
				result = push_down(current->branch[position], new_entry, extra_entry, extra_branch);
				if (result == overflow) {
					// Record extra_entry now must be added to current
					if (current->count < order − 1) {
						result = success;
						push_in(current, extra_entry, extra_branch, position);
					}
					else 
						split_node(current, extra_entry, extra_branch, position, right_branch, median);
					// Record median and its right_branch will go up to a higher node.
				}
			}
		}
		return result;
	}
	
	void push_in(Node *current, const R &entry, Node *right_branch, int position){
		for (int i = current->count; i > position; i−−) {
			// Shift all later data to the right.
			current->data[i] = current->data[i − 1];
			current->branch[i + 1] = current->branch[i];
		}
		current->data[position] = entry;
		current->branch[position + 1] = right_branch;
		current->count++;
	}
	
	void split_node(
		Node *current, // node to be split
		const R &extra_entry, // new entry to insert
		Node *extra_branch, // subtree on right of extra_entry
		int position, // index in node where extra_entry goes
		Node * &right_half, // new node for right half of entries
		R &median) // median entry (in neither half)
	{
		right_half = new Node;
		int mid = order / 2; // The entries from mid on will go to right_half.
		if (position <= mid) { // First case: extra_entry belongs in left half.
			for (int i = mid; i < order − 1; i++) { // Move entries to right_half.
				right_half->data[i − mid] = current->data[i];
				right_half->branch[i + 1 − mid] = current->branch[i + 1];
			}
			current->count = mid;
			right_half->count = order − 1 − mid;
			push_in(current, extra_entry, extra_branch, position);
		}
		else { // Second case: extra_entry belongs in right half.
			mid++; // Temporarily leave the median in left half.
			for (int i = mid; i < order − 1; i++) { // Move entries to right_half.
				right_half->data[i − mid] = current->data[i];
				right_half->branch[i + 1 − mid] = current->branch[i + 1];
			}
			current->count = mid;
			right_half->count = order − 1 − mid;
			push_in(right_half, extra_entry, extra_branch, position − mid);
		}
		median = current->data[current->count − 1];
		// Remove median from left half.
		right_half->branch[0] = current->branch[current->count];
		current->count−−;
	}
	
	Error_code recursive_remove(Node *current, const R &target){
		Error_code result;
		int position;
		if (current == NULL) result = not_present;
		else {
			if (search_node(current, target, position) == success) {
				// The target is in the current node.
				result = success;
				if (current->branch[position] != NULL) { // not at a leaf node
					copy_in_predecessor(current, position);
					recursive_remove(current->branch[position],	current->data[position]);
				}
				else 
					remove_data(current, position); // Remove from a leaf node.
			}
			else 
				result = recursive_remove(current->branch[position], target);
			if (current->branch[position] != NULL)
				if (current->branch[position]->count < (order − 1) / 2)
					restore(current, position);
		}
		return result;
	}
	
	void remove_data(Node *current, int position){
		for (int i = position; i < current->count − 1; i++)
			current->data[i] = current->data[i + 1];
		current->count−−;
	}
	
	void copy_in_predecessor(Node *current, int position){
		B_node<Record, order> *leaf = current->branch[position];
		// First go left from the current entry.
		while (leaf->branch[leaf->count] != NULL)
			leaf = leaf->branch[leaf->count]; // Move as far rightward as possible.
		current->data[position] = leaf->data[leaf->count − 1];
	}
	
	void restore(Node *current, int position){
		if (position == current->count) // case: rightmost branch
			if (current->branch[position − 1]->count > (order − 1) / 2)
				move_right(current, position − 1);
			else
				combine(current, position);
		else if (position == 0) // case: leftmost branch
			if (current->branch[1]->count > (order − 1) / 2)
				move_left(current, 1);
			else
				combine(current, 1);
		else // remaining cases: intermediate branches
			if (current->branch[position − 1]->count > (order − 1) / 2)
				move_right(current, position − 1);
			else if (current->branch[position + 1]->count > (order − 1) / 2)
				move_left(current, position + 1);
			else
				combine(current, position);
	}
	
	void move_left(Node *current, int position){
		Node *left_branch = current->branch[position − 1],
			*right_branch = current->branch[position];
		left_branch->data[left_branch->count] = current->data[position − 1];
		// Take entry from the parent.
		left_branch->branch[++left_branch->count] = right_branch->branch[0];
		current->data[position − 1] = right_branch->data[0];
		// Add the right-hand entry to the parent.
		right_branch->count−−;
		for (int i = 0; i < right_branch->count; i++) {
			// Move right-hand entries to fill the hole.
			right_branch->data[i] = right_branch->data[i + 1];
			right_branch->branch[i] = right_branch->branch[i + 1];
		}
		right_branch->branch[right_branch->count] = right_branch->branch[right_branch->count + 1];
	}
	
	void move_right(Node *current, int position){
		Node *right_branch = current->branch[position + 1],
			*left_branch = current->branch[position];
		right_branch->branch[right_branch->count + 1] =
			right_branch->branch[right_branch->count];
		for (int i = right_branch->count; i > 0; i−−) { // Make room for new entry.
			right_branch->data[i] = right_branch->data[i − 1];
			right_branch->branch[i] = right_branch->branch[i − 1];
		}
		right_branch->count++;
		right_branch->data[0] = current->data[position];
		// Take entry from parent.
		right_branch->branch[0] = left_branch->branch[left_branch->count−−];
		current->data[position] = left_branch->data[left_branch->count];
	}
	
	void combine(Node *current, int position){
		int i;
		B_node<Record, order> *left_branch = current->branch[position − 1],
			*right_branch = current->branch[position];
		left_branch->data[left_branch->count] = current->data[position − 1];
		left_branch->branch[++left_branch->count] = right_branch->branch[0];
		for (i = 0; i < right_branch->count; i++) {
			left_branch->data[left_branch->count] = right_branch->data[i];
			left_branch->branch[++left_branch->count] =
				right_branch->branch[i + 1];
		}
		current->count−−;
		for (i = position − 1; i < current->count; i++) {
			current->data[i] = current->data[i + 1];
			current->branch[i + 1] = current->branch[i + 2];
		}
		delete right_branch;
	}
	
	Node *root;
	rsize_t order;
};