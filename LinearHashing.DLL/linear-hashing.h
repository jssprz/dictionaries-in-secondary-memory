#pragma once

#include "../common_headers/error-code.h"
#include "../common_headers/data-types.h"
#include "../common_headers/file-manager.h"
#include "../common_headers/cache-memory.h"
#include "linear-hashing-page.h"
#include <numeric>

using namespace common;

namespace linear_hashing {
	enum Maintenance_criteria {
		maximum_average_search_cost = 1,
		minimum_filled_percent = 2
	};

	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class LinearHashing { };

	template <typename K, typename R, typename TK, typename TR>
	class LinearHashing<K, R, TK, TR, true, true> {
	public:
		typedef LinearHashingPage<K, R, TK, TR> Page;
		typedef CacheMemory<Page> Cache;
		typedef int isSpecialized;

		// file manager
		static FileManager *fm;
		static long pageSize;
		static int keySize;   //K.Size
		static int valueSize; //R.Size

		// data for tests
		static int writes_count;
		static int reads_count;

		LinearHashing(size_t max_page_count, Maintenance_criteria criterion, double criteria_param, FileManager *handler, int keySize, int valueSize)
			: n(0), p(1), t(0), criterion(criterion), all_pages_count(1){

			LinearHashing<K, R, TK, TR>::fm = handler;
			LinearHashing<K, R, TK, TR>::keySize = keySize;
			LinearHashing<K, R, TK, TR>::valueSize = valueSize;
			LinearHashing<K, R, TK, TR>::max_page_count = max_page_count;

			switch (criterion)
			{
			case maximum_average_search_cost:
				search_cost_thresh = criteria_param;
			case minimum_filled_percent:
				filled_percent_thresh = criteria_param;
			default:
				break;
			}

			this->RAM = new Cache(100);

			auto initial_page = alloc();
			first_pages.push_back(initial_page->file_pos);
			last_pages.push_back(initial_page->file_pos);

			calculate_page_size();

			writes_count = reads_count = 0;
		}

		LinearHashing(long file_pos, FileManager *handler)
			: n(0), p(1), t(0), criterion(criterion), all_pages_count(1) {

			LinearHashing<K, R, TK, TR>::fm = handler;
			LinearHashing<K, R, TK, TR>::keySize = keySize;
			LinearHashing<K, R, TK, TR>::valueSize = valueSize;
			LinearHashing<K, R, TK, TR>::max_page_count = max_page_count;

			this->RAM = new Cache(100);

			calculate_page_size();

			read_hashing_info(file_pos);
		}

		~LinearHashing() {
		}

		//main functions
		Error_code search(K &target) {
			int max = pow(2, t + 1);
			auto hash = target.hash();
			size_t index = !(hash % max) ? max - 1 : hash % max - 1;
			long file_pos;
			if (index >= p)
				file_pos = first_pages[index - max / 2];
			else
				file_pos = first_pages[index];
			return search_in_pages_list(file_pos, target);
		}

		Error_code insert(K &new_entry) {
			int max = pow(2, t + 1);
			auto hash = new_entry.hash();
			size_t index = !(hash % max) ? max - 1 : hash % max - 1;
			Error_code result;
			if (index >= p)
				result = insert_in_pages_list(new_entry, index - max / 2);
			else
				result = insert_in_pages_list(new_entry, index);

			switch (criterion)
			{
			case maximum_average_search_cost:
				if (search_cost() > 3 * search_cost_thresh / 2) //case: the average search cost is greater than the maximum permitted
					expand_page();
				break;
			case minimum_filled_percent:
				if (filled_percent() > 3 * filled_percent_thresh / 2) //case: the filled percent of the pages is letter than the minimum permitted
					expand_page();
				break;
			}

			return result;
		}

		Error_code remove(K &entry) {
			auto hash = entry.hash();
			int max = pow(2, t + 1);
			size_t index = !(hash % max) ? max - 1 : hash % max - 1;
			Error_code result;
			if (index >= p)
				result = remove_from_page_list(entry, index - max / 2);
			else
				result = remove_from_page_list(entry, index);

			switch (criterion)
			{
			case maximum_average_search_cost:
				if (p > 1 && search_cost() < search_cost_thresh / 2) //case: the average search cost is greater than the maximum permitted
					contract_page();
				break;
			case minimum_filled_percent:
				if (p > 1 && filled_percent() < filled_percent_thresh / 2) //case: the filled percent of the pages is letter than the minimum permitted
					contract_page();
				break;
			}

			return result;
		}

		long save() {
			RAM->flush(&disk_write);
			long file_pos = fm->get_heap_end();
			write_hashing_info(file_pos);
			return file_pos;
		}

		long count() {
			return this->n;
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

			writes_count++;
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

			reads_count++;

			return result;
		}


	//private: //comment this for tests

		Error_code search_in_pages_list(long file_pos, K &target) {
			auto current = get_page(file_pos);
			while (current != nullptr && search_in_page(current, target) != success)
				current = get_page(current->next_page_file_pos);
			return current != nullptr ? success : not_present;
		}

		Error_code search_in_page(shared_ptr<Page> &page, K &target) {
			for (size_t i = 0; i < page->count; i++) {
				if (page->data[i] == target)
					return success;
			}
			return not_present;
		}

		Error_code insert_in_pages_list(K &new_entry, int hash_table_pos) {
			auto page = get_page(first_pages[hash_table_pos]);
			return insert_in_page(page, new_entry, hash_table_pos);
		}
		
		Error_code insert_in_page(shared_ptr<Page> &page, K &new_entry, int hash_table_pos) {
			shared_ptr<Page> prev_page;
			while (page != nullptr && page->count == max_page_count){
				prev_page = page;
				page = get_page(page->next_page_file_pos);
			}
			if (page == nullptr) {
				auto new_page = alloc();
				new_page->prev_page_file_pos = prev_page->file_pos;
				prev_page->next_page_file_pos = last_pages[hash_table_pos] = new_page->file_pos;
				all_pages_count++;
				page = new_page;
			}
			
			page->data[page->count++] = new_entry;
			n++;

			return success;
		}

		void realloc_in_page(shared_ptr<Page> &page, K &entry, int hash_table_pos) {
			shared_ptr<Page> prev_page;
			while (page != nullptr && page->count == max_page_count) {
				prev_page = page;
				page = get_page(page->next_page_file_pos);
			}
			if (page == nullptr) {
				auto new_page = alloc();
				new_page->prev_page_file_pos = prev_page->file_pos;
				prev_page->next_page_file_pos = last_pages[hash_table_pos] = new_page->file_pos;
				all_pages_count++;
				page = new_page;
			}

			page->data[page->count++] = entry;
		}

		void expand_page() {
			int max = pow(2, t + 1);
			auto current = get_page(first_pages[p - max / 2]);

			auto realloc_page = alloc();
			first_pages[p - max / 2] = last_pages[p - max / 2] = realloc_page->file_pos;
			all_pages_count++;

			// expansion
			auto new_page = alloc();
			first_pages.push_back(new_page->file_pos);
			last_pages.push_back(new_page->file_pos);
			all_pages_count++;

			while (current != nullptr) {
				for (size_t i = 0; i < current->count; i++) {
					auto hash = current->data[i].hash(); //calculate hash
					size_t index = !(hash % max) ? max - 1 : hash % max - 1;
					if (index == p - max / 2)
						realloc_in_page(realloc_page, current->data[i], p - max / 2);
					else
						realloc_in_page(new_page, current->data[i], p);
				}
				//free memory of current->file_pos
				//fm->free(current->file_pos);
				all_pages_count--;
				current = get_page(current->next_page_file_pos);
			}
			
			if (++p == max) //case: expansion completed
				t++;
		}

		Error_code remove_from_page_list(K &entry, int hash_table_pos) {
			auto page = get_page(first_pages[hash_table_pos]);
			return remove_from_page(page, entry, hash_table_pos);
		}

		Error_code remove_from_page(shared_ptr<Page> &current_page, K &entry, int hash_table_pos) {
			while (current_page !=nullptr) {
				for (size_t i = 0; i < current_page->count; i++) {
					if (current_page->data[i] == entry) {
						if (current_page->file_pos != last_pages[hash_table_pos]) {//case: can get key of the last page
							shared_ptr<Page> last_page = get_page(last_pages[hash_table_pos]);
							current_page->data[i] = last_page->data[--last_page->count];//move key of th last page, and update the last page count
							n--;
							if (last_page->count == 0) {//case: last page was left empty, must be updated the linked list links and the last_page link of the node
								shared_ptr<Page> prev_page = get_page(last_page->prev_page_file_pos);
								prev_page->next_page_file_pos = -1;
								last_pages[hash_table_pos] = prev_page->file_pos;
								all_pages_count--;
								//free space of old last_page in file
								//fm->free(last_page->file_pos);
							}
						}
						else {//case: the key to delete is in the last page
							current_page->data[i] = current_page->data[--current_page->count];//move last key of the page, and update count
							n--;
							if (current_page->count == 0) {
								if (first_pages[hash_table_pos] == last_pages[hash_table_pos]) //case: the only page was left empty
									first_pages[hash_table_pos] = last_pages[hash_table_pos] = -1;
								else { //case: the last page was left empty, must be updated the linked list links and the last_page link of the node
									auto prev_page = get_page(current_page->prev_page_file_pos);
									prev_page->next_page_file_pos = -1;
									last_pages[hash_table_pos] = prev_page->file_pos;
								}
								all_pages_count--;
							}
						}
						return success;
					}
				}
				current_page = get_page(current_page->next_page_file_pos);
			}
			return not_present;
		}

		void contract_page() {
			auto page = get_page(first_pages.back());
			int min = pow(2, t);

			int realloc_index = p - min - 1;
			auto realloc_page = get_page(first_pages[realloc_index]);
			if (realloc_page != nullptr) {
				while (page != nullptr) {
					for (size_t i = 0; i < page->count; i++)
						realloc_in_page(realloc_page, page->data[i], realloc_index);
					//free space of page in the file
					//fm->free(page->file_pos);
					all_pages_count--;
					page = get_page(page->next_page_file_pos);
				}
			}
			else {
				first_pages[realloc_index] = first_pages.back();
				last_pages[realloc_index] = last_pages.back();
			}

			// contraction
			first_pages.pop_back();
			last_pages.pop_back();

			if (--p == min) //case: contraction completed
				t--;
		}

		double filled_percent() {
			return double(n) / double(all_pages_count * max_page_count);
		}

		double search_cost() {
			return double(all_pages_count) / (p - pow(2, t));
		}

		// create and return a new page
		shared_ptr<Page> alloc() {
			long freeMemory = fm->alloc();
			auto result = make_shared<Page>(max_page_count, keySize, freeMemory);
			reads_count++;

			//Adds page to cache
			RAM->add(result, &disk_write);

			return result;
		}

		void calculate_page_size() {
			pageSize = sizeof(size_t) +					//count of keys (count).       
				sizeof(long) +							//reference to the prev page.
				sizeof(long) +							//reference to the next page.
				valueSize * (max_page_count)+			//satellite info (satellite).
				keySize * (max_page_count);				//keys of the node(Keys).
		}

		void write_hashing_info(long file_pos) {

		}

		void read_hashing_info(long file_pos) {

		}

		// file_pos of the first page of each pages list
		vector<long> first_pages;
		// file_pos of the last page of each pages list
		vector<long> last_pages;
		// total count of pages
		long all_pages_count;
		// count of keys
		long n;
		// pages lists count
		int p;
		// pow of 2
		int t;
		// criterion by which the expansions and contractions will be regulated
		Maintenance_criteria criterion;
		// maximum average search cost threshold
		double search_cost_thresh;
		// minimum filled pages percent threshold
		double filled_percent_thresh;
		// maximum count of keys in a page
		static int max_page_count;
		// nodes in ram
		Cache *RAM;
	};

	template<typename K, typename R, typename TK, typename TR>
	FileManager* LinearHashing<K, R, TK, TR, true, true>::fm = nullptr;
	template<typename K, typename R, typename TK, typename TR>
	long LinearHashing<K, R, TK, TR, true, true>::pageSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int LinearHashing<K, R, TK, TR, true, true>::keySize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int LinearHashing<K, R, TK, TR, true, true>::valueSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int LinearHashing<K, R, TK, TR, true, true>::max_page_count = 10;
	template<typename K, typename R, typename TK, typename TR>
	int LinearHashing<K, R, TK, TR, true, true>::reads_count = 0;
	template<typename K, typename R, typename TK, typename TR>
	int LinearHashing<K, R, TK, TR, true, true>::writes_count = 0;
}