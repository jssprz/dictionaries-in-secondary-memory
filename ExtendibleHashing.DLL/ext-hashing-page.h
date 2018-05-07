#pragma once

#include "../common_headers/data-types.h"
#include <vector>

using namespace std;
using namespace common;

namespace extensible_hashing {
	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class ExtensibleHashingPage { };

	template <typename K, typename R, typename TK, typename TR>
	class ExtensibleHashingPage<K, R, TK, TR, true, true> {
	public:
		ExtensibleHashingPage(size_t capacity, size_t keySize, long file_pos)
			: file_pos(file_pos), count(0), next_page_file_pos(-1), prev_page_file_pos(-1){
			data = new K[capacity];
			//data = (K *)malloc(capacity*keySize);
		}

		~ExtensibleHashingPage() {
			delete[] data;
			//delete[] this->data;
			//for (size_t i = 0; i < count; i++) {
			//	delete this->branch[i];
			//}
			//delete[] this->branch;
		}

		// data members:
		size_t count;
		K *data;
		long file_pos;
		long next_page_file_pos;
		long prev_page_file_pos;
	};
}