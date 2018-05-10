#pragma once

#include <vector>

using namespace std;

namespace extensible_hashing {
	class ExtensibleHashingNode
	{
	public:
		ExtensibleHashingNode(bool is_leaf = true, long first_page_file_pos = -1, long last_page_file_pos = -1)
			: is_leaf(is_leaf), first_page_file_pos(first_page_file_pos), last_page_file_pos(last_page_file_pos){
			this->branch = vector<shared_ptr<ExtensibleHashingNode>>(2, nullptr);
		}
		~ExtensibleHashingNode() {

		}

		bool is_empty() {
			return is_leaf && first_page_file_pos == -1;
		}

		bool has_only_page() {
			return is_leaf && first_page_file_pos != -1 && first_page_file_pos == last_page_file_pos;
		}

		bool is_leaf;
		vector<shared_ptr<ExtensibleHashingNode>> branch;
		long first_page_file_pos;
		long last_page_file_pos;

	private:

	};
}