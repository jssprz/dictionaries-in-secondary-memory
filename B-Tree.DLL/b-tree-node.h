#pragma once

template <typename K, typename R>
struct BTreeNode {
	// data members:
	int count;
	K *data;
	BTreeNode<K, R> **branch;
	// constructor:
	BTreeNode(int branchingFactor);
	// destructor
	~BTreeNode();
};