#pragma once

template <typename K, typename R, int B>
struct BTreeNode {
	// data members:
	int count;
	K data[B-1];
	BTreeNode<K, R, B> *branch[B];
	// constructor:
	BTreeNode();
};