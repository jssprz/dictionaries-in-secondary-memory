#pragma once

#include "../common_headers/error-code.h"
#include "../common_headers/data-types.h"
#include "../common_headers/file-manager.h"
#include "../common_headers/cache-memory.h"
#include "ext-hashing-node.h"
#include "ext-hashing-page.h"
#include <memory>

using namespace common;

namespace extensible_hashing {
	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class ExtensibleHashing { };

	template <typename K, typename R, typename TK, typename TR>
	class ExtensibleHashing<K, R, TK, TR, true, true> {
	public:
		typedef ExtensibleHashingNode Node;
		typedef ExtensibleHashingPage<K, R, TK, TR> Page;
		typedef CacheMemory<Page> Cache;
		typedef int isSpecialized;

		// file manager
		static FileManager *fm;
		static long pageSize;
		static int keySize;   //K.Size
		static int valueSize; //R.Size

		ExtensibleHashing(size_t max_page_count, size_t hash_code_length, FileManager *handler, int keySize, int valueSize)
			: n(0), hash_code_length(hash_code_length), max_keys_merge(2 * max_page_count / 3) {

			ExtensibleHashing<K, R, TK, TR>::fm = handler;
			ExtensibleHashing<K, R, TK, TR>::keySize = keySize;
			ExtensibleHashing<K, R, TK, TR>::valueSize = valueSize;
			ExtensibleHashing<K, R, TK, TR>::max_page_count = max_page_count;

			this->root = make_shared<Node>();

			this->RAM = new Cache(100);

			calculate_page_size();
		}

		ExtensibleHashing(size_t max_page_count, size_t hash_code_length, FileManager *handler, long root_file_pos, int keySize, int valueSize)
			: n(0), hash_code_length(hash_code_length) {
			
			ExtensibleHashing<K, R, TK, TR>::fm = handler;
			ExtensibleHashing<K, R, TK, TR>::keySize = keySize;
			ExtensibleHashing<K, R, TK, TR>::valueSize = valueSize;
			
			this->root = new Node();

			this->RAM = new Cache(100);
			
			calculate_page_size();

			this->root = disk_read(root_file_pos);
		}

		~ExtensibleHashing() {
		}

		//main functions
		Error_code search(K &target) {
			//calculate hash
			auto hash = target.hash();

			int height;
			auto leaf = move_to_leaf(hash, height);

			return search_in_pages_list(leaf->first_page_file_pos, target);
		}

		Error_code insert(K &new_entry) {
			auto hash = new_entry.hash();//calculate hash
			int height;
			auto leaf = move_to_leaf(hash, height);
			auto page = get_page(leaf->first_page_file_pos);
			return recursive_insert(leaf, height, page, new_entry);
		}

		Error_code remove(K &target) {
			auto hash = target.hash();
			return recursive_remove(root, target, hash, 0);
		}

		void save() {
			/*if (root != nullptr) {
				disk_write(root);
				RAM->flush();
			}*/
		}

		long count() {
			return this->n;
		}

		bool empty() {
			return n == 0;
		}

		shared_ptr<Page> get_page(long file_pos) {
			if (file_pos == -1)
				return nullptr;

			auto page = RAM->find(file_pos);
			if (page == nullptr) { //if it is not in the cache
				page = disk_read(file_pos);
				RAM->add(page, &disk_write);
			}

			return page;
		}

		static void disk_write(shared_ptr<Page> &x) {
			//1-count
			//2-data
			//3-next_page_file_pos
			//4-prev_page_file_pos

			fm->fileStream.seekp(x->file_pos);

			// Write the count of keys the node has
			fm->fileStream.write((char *)&(x->count), sizeof(size_t));

			// Write the keys (K*)
			for (size_t i = 0; i < x->count; i++) {
				auto key = x->data[i].save();
				fm->fileStream.write(key, keySize);
				delete[] key;
			}

			// Reserve space for other keys, it is necesary beacause we attempt to read all pageSize
			int empty_size = (max_page_count - x->count) * keySize;
			char *empty = new char[empty_size];
			fm->fileStream.write(empty, empty_size);
			delete[] empty;

			// Write the next_page_file_pos
			fm->fileStream.write((char *)&(x->next_page_file_pos), sizeof(long));

			// Write the prev_page_file_pos
			fm->fileStream.write((char *)&(x->prev_page_file_pos), sizeof(long));

			// Write info in the file
			fm->fileStream.flush();
		}

		static shared_ptr<Page> disk_read(long file_pos) {
			//1-count
			//2-data
			//3-next_page_file_pos
			//4-prev_page_file_pos

			int index = 0;
			auto result = make_shared<Page>(max_page_count, keySize, file_pos);

			// Move to the riquired position in file
			fm->fileStream.seekg(file_pos);

			// Read the node info
			char* bufferNode = new char[pageSize];
			fm->fileStream.read(bufferNode, pageSize);

			// Read the count of keys of node
			result->count = *((size_t*)(bufferNode));
			index += sizeof(size_t);

			// Read data (K*)
			char* subBuffer = new char[keySize];
			for (int i = 0; i < result->count; i++)
			{
				strncpy(subBuffer, bufferNode + index, keySize);
				K key;
				key.load(subBuffer, keySize);
				result->data[i] = key;
				index += keySize;
			}
			index += (max_page_count - result->count) * keySize;

			// Read the next_page_file_pos
			result->next_page_file_pos = *((long*)(bufferNode + index));
			index += sizeof(long);

			// Read the prev_page_file_pos
			result->prev_page_file_pos = *((long*)(bufferNode + index));
			index += sizeof(long);

			delete[] subBuffer;
			delete[] bufferNode;

			return result;
		}
		

	private:

		int get_ith_bit(long long hash, int i) {
			long long mask = 1L << i;
			long long masked_n = hash & mask;
			return !(masked_n >> i) ? 0 : 1;
		}

		shared_ptr<Node> move_to_leaf(long long hash, int &height) {
			height = 0;
			auto current = root;
			while (!current->is_leaf) {
				//get ith bit
				int thebit = get_ith_bit(hash, height++);

				//get the next node
				current = current->branch[thebit];
			}
			return current;
		}

		Error_code search_in_pages_list(long file_pos, K &target) {
			auto current = get_page(file_pos);
			while (current != nullptr && search_in_page(current, target) != success)
				current = get_page(current->next_page_file_pos);
			return current != nullptr ? success : not_present;
		}

		Error_code search_in_page(shared_ptr<Page> &page, K &target) {
			for (size_t i = 0; i < page->count; i++)
				if (page->data[i] == target)
					return success;
			return not_present;
		}

		Error_code recursive_insert(shared_ptr<Node> &current, int height, shared_ptr<Page> &page, K &new_entry) {
			if (page != nullptr) {
				if (page->count < max_page_count) { //case: the new entry can be added to leaf page
					page->data[page->count++] = new_entry;
					n++;
				}
				else if (height < hash_code_length) {//case: there is more bits in the hash to increase the tree
					auto result = split_page_and_insert(current, page, height, new_entry);
					if (result == overflow) {
						//insert in the leaf that is full now
						recursive_insert(current, height + 1, page, new_entry);
					}
				}
				else { //case: the entry must be inserted at the end of the pages linked-list
					shared_ptr<Page> last_page = get_page(current->last_page_file_pos);
					if (last_page->count < max_page_count) //case: the last page is not full
						last_page->data[last_page->count++] = new_entry;
					else { //case: the last page is full
						shared_ptr<Page> new_last = alloc();
						new_last->data[0] = new_entry;
						new_last->count = 1;
						new_last->prev_page_file_pos = last_page->file_pos;
						last_page->next_page_file_pos = new_last->file_pos;
						current->last_page_file_pos = new_last->file_pos;
					}
					n++;
				}
			}
			else {
				shared_ptr<Page> new_page = alloc();
				new_page->data[0] = new_entry;
				new_page->count = 1;
				current->first_page_file_pos = current->last_page_file_pos = new_page->file_pos;
				n++;
			}
			return success;
		}

		Error_code split_page_and_insert(shared_ptr<Node> &leaf, shared_ptr<Page> &full_page, int new_height, K &new_entry) {
			leaf->is_leaf = false;
			
			auto data0 = new K[max_page_count], data1 = new K[max_page_count];
			size_t data0_count = 0, data1_count = 0;

			for (size_t i = 0; i < full_page->count; i++) {
				//get the bit
				int thebit = get_ith_bit(full_page->data[i].hash(), new_height);

				if (!thebit)
					data0[data0_count++] = full_page->data[i];
				else
					data1[data1_count++] = full_page->data[i];
			}

			if (data0_count == max_page_count) {
				delete[] data0, data1;
				leaf->branch[0] = make_shared<Node>(true, full_page->file_pos, full_page->file_pos);
				leaf->branch[1] = make_shared<Node>();
				leaf = leaf->branch[0];
				return overflow;
			}
			else if (data1_count == max_page_count) {
				delete[] full_page->data, data0;
				full_page->data = data1;
				full_page->count = data1_count;

				leaf->branch[0] = make_shared<Node>();
				leaf->branch[1] = make_shared<Node>(true, full_page->file_pos, full_page->file_pos);
				leaf = leaf->branch[1];
				return overflow;
			}

			//get the bit
			int thebit = get_ith_bit(new_entry.hash(), new_height);

			if (!thebit)
				data0[data0_count++] = new_entry;
			else
				data1[data1_count++] = new_entry;
			n++;

			auto extra_page = alloc();
			leaf->branch[0] = make_shared<Node>(true, full_page->file_pos, full_page->file_pos);
			leaf->branch[1] = make_shared<Node>(true, extra_page->file_pos, extra_page->file_pos);

			delete[] full_page->data, extra_page->data;

			full_page->data = data0;
			full_page->count = data0_count;

			extra_page->data = data1;
			extra_page->count = data1_count;

			return success;
		}

		Error_code recursive_remove(shared_ptr<Node> &current, K &target, long long hash, int height) {
			if (current->is_leaf) {
				//find the target in the pages
				auto current_page = get_page(current->first_page_file_pos);
				while (current_page != nullptr) {
					for (size_t i = 0; i < current_page->count; i++) {
						if (current_page->data[i] == target) {
							if (current_page->file_pos != current->last_page_file_pos) {//case: can get key of the last page
								auto last_page = get_page(current->last_page_file_pos);
								current_page->data[i] = last_page->data[--last_page->count];//move key of th last page, and update the last page count
								if (last_page->count == 0) {//case: the last page was left empty, must be updated the linked list links and the last_page link of the node
									auto prev_page = get_page(last_page->prev_page_file_pos);
									prev_page->next_page_file_pos = -1;
									current->last_page_file_pos = prev_page->file_pos;
									RAM->remove(last_page); //so that this page is not written later
								}
							}
							else if (current_page->count == 1) {
								if (current->first_page_file_pos == current->last_page_file_pos) //case: the only page was left empty
									current->first_page_file_pos = current->last_page_file_pos = -1;
								else { //case: the last page was left empty, must be updated the linked list links and the last_page link of the node
									auto prev_page = get_page(current_page->prev_page_file_pos);
									prev_page->next_page_file_pos = -1;
									current->last_page_file_pos = prev_page->file_pos;
								}
								RAM->remove(current_page); //so that this page is not written later
							}
							else //case: the key to delete is in the last page
								current_page->data[i] = current_page->data[--current_page->count];//move last key of the page, and update count
							n--;
							return success;
						}
					}
					current_page = get_page(current_page->next_page_file_pos);
				}
				return not_present;
			}
			else {
				int thebit = get_ith_bit(hash, height);
				auto child = current->branch[thebit];
				auto result = recursive_remove(child, target, hash, height + 1);
				if (result == success) {
					auto brother = current->branch[-(thebit - 1)];
					if (child->is_empty()) {
						if (brother->is_empty()) {//case: child is empty & bro is empty
							current->is_leaf = true;
							current->first_page_file_pos = current->last_page_file_pos = -1;
						}
						else if (brother->is_leaf) {//case: child is empty & bro is leaf
							current->is_leaf = true;
							current->first_page_file_pos = brother->first_page_file_pos;
							current->last_page_file_pos = brother->last_page_file_pos;
						}
					}
					else if (child->is_leaf && brother->is_empty()) {//case: child is a not empty leaf & bro is empty
						current->is_leaf = true;
						current->first_page_file_pos = child->first_page_file_pos;
						current->last_page_file_pos = child->last_page_file_pos;
					}
					else if (child->has_only_page() && brother->has_only_page()) {//case: the brother
						auto page0 = get_page(child->first_page_file_pos);
						auto page1 = get_page(brother->first_page_file_pos);
						if (page0->count + page1->count < max_keys_merge) {//case: the brother is leaf too and all keys fit in one only page
							current->is_leaf = true;
							for (size_t i = 0; i < page1->count; i++)
								page0->data[page0->count++] = page1->data[i];
							current->first_page_file_pos = current->last_page_file_pos = page0->file_pos;
						}
					}
				}
				return result;
			}
		}

		// create and return a new page
		shared_ptr<Page> alloc() {
			long freeMemory = fm->alloc();
			auto result = make_shared<Page>(max_page_count, keySize, freeMemory);

			//Adds node to cache
			RAM->add(result, &disk_write);

			return result;
		}

		void calculate_page_size() {
			pageSize = sizeof(size_t) +					//count of keys (count).       
				sizeof(long) +							//reference to the prev page.
				sizeof(long) +							//reference to the next page.
				valueSize * (max_page_count) +			//satellite info (satellite).
				keySize * (max_page_count);				//keys of the node(Keys).
		}

		// root of the btree
		shared_ptr<Node> root;
		// count of keys
		long n;
		// maximum position in the hash code
		size_t hash_code_length;
		// maximum count of keys in a page
		static int max_page_count;
		// maximum count of keys needed to merge 2 pages
		int max_keys_merge;
		// nodes in ram
		Cache *RAM;
	};

	template<typename K, typename R, typename TK, typename TR>
	FileManager* ExtensibleHashing<K, R, TK, TR, true, true>::fm = nullptr;
	template<typename K, typename R, typename TK, typename TR>
	long ExtensibleHashing<K, R, TK, TR, true, true>::pageSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int ExtensibleHashing<K, R, TK, TR, true, true>::keySize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int ExtensibleHashing<K, R, TK, TR, true, true>::valueSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int ExtensibleHashing<K, R, TK, TR, true, true>::max_page_count = 10;
}