#pragma once

template <typename K, typename R>
class BTreeNode {

public:
	BTreeNode(int branchingFactor) {
		this->data = new K[branchingFactor - 1];
		this->branch = new BTreeNode<K, R>*[branchingFactor];
	}

	~BTreeNode() {
		delete[] this->data;
		for (size_t i = 0; i < count; i++) {
			delete this->branch[i];
		}
		delete[] this->branch;
	}

	// data members:
	size_t count;
	K *data;
	BTreeNode<K, R> **branch;
};