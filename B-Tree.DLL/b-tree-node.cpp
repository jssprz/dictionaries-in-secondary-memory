#include "b-tree-node.h"
#include "key.h"
#include "record.h"

BTreeNode<Key, Record>::BTreeNode(int branchingFactor) {
	this->data = new Key[branchingFactor - 1];
	this->branch = new BTreeNode<Key, Record>*[branchingFactor];
}

BTreeNode<Key, Record>::~BTreeNode() {
	delete[] this->data;
	for (size_t i = 0; i < count; i++) {
		delete this->branch[i];
	}
	delete[] this->branch;
}