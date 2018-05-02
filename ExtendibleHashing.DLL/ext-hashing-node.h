#pragma once

#include <vector>

using namespace std;

namespace extensible_hashing {
	template<typename K>
	class ExtensibleHashingNode
	{
		typedef ExtensibleHashingNode<K> Node;
	public:
		ExtensibleHashingNode(bool is_leaf) {
			this->is_leaf = is_leaf;
			this->branch = vector<Node*>(2, NULL);
			file_page_position = -1;
		}
		~ExtensibleHashingNode() {

		}

		bool is_leaf;
		vector<Node*> branch;
		long file_page_position;

	private:

	};
}