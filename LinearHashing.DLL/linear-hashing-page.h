#pragma once

#include "../common_headers/data-types.h"
#include <vector>

using namespace std;
using namespace common;

namespace linear_hashing {
	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class LinearHashingPage { };

	template <typename K, typename R, typename TK, typename TR>
	class LinearHashingPage<K, R, TK, TR, true, true> {
	public:
		LinearHashingPage(size_t capacity, size_t keySize, long file_pos)
			: file_pos(file_pos), count(0), next_page_file_pos(-1), prev_page_file_pos(-1) {
			data = new K[capacity];
		}

		~LinearHashingPage() {
			delete[] data;
		}

		// data members:
		size_t count;
		K *data;
		long file_pos;
		long next_page_file_pos;
		long prev_page_file_pos;
	};
}