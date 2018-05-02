#pragma once

#include "key.h"
#include "record.h"
#include <vector>

using namespace std;

namespace btree {
	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class BTreeNode { };

	template <typename K, typename R, typename TK, typename TR>
	class BTreeNode<K, R, TK, TR, true, true> {
	public:
		BTreeNode(size_t branchingFactor, long file_position)
			: file_position(file_position), count(0), order(branchingFactor) {
			data = vector<K>(branchingFactor - 1);
			branch = vector<long>(branchingFactor, -1);
		}

		~BTreeNode() {
			//delete[] this->data;
			//for (size_t i = 0; i < count; i++) {
			//	delete this->branch[i];
			//}
			//delete[] this->branch;
		}

		// data members:
		size_t count;
		vector<K> data;
		vector<long> branch;
		long file_position;
		size_t order;
	};
}