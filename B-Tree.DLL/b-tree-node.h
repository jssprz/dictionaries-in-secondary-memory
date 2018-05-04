#pragma once

#include "../common_headers/data-types.h"
#include <vector>

using namespace std;
using namespace common;

namespace btree {
	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class BTreeNode { };

	template <typename K, typename R, typename TK, typename TR>
	class BTreeNode<K, R, TK, TR, true, true> {
	public:
		BTreeNode(size_t branchingFactor, long file_pos)
			: file_pos(file_pos), count(0), order(branchingFactor) {
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
		long file_pos;
		size_t order;
	};
}