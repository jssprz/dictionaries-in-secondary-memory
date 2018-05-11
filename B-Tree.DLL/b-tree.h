#pragma once

#include "b-tree-node.h"
#include "../common_headers/data-types.h"
#include "../common_headers/error-code.h"
#include "../common_headers/file-manager.h"
#include "../common_headers/cache-memory.h"
#include <type_traits>
#include <memory>

using namespace common;

namespace btree {

	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class BTree { };

	template <typename K, typename R, typename TK, typename TR>
	class BTree<K, R, TK, TR, true, true> {
	public:
		typedef BTreeNode<K, R, TK, TR> Node;
		typedef CacheMemory<Node> Cache;
		typedef int isSpecialized;

		// file manager
		static FileManager *fm;
		static long nodeSize;
		static int keySize;   //K.Size
		static int valueSize; //R.Size

		// data for tests
		static int writes_count;
		static int reads_count;

		BTree(size_t branchingFactor, FileManager *handler, int keySize, int valueSize)
			: root(nullptr), n(0) {
			BTree<K, R, TK, TR>::order = branchingFactor;
			BTree<K, R, TK, TR>::fm = handler;
			BTree<K, R, TK, TR>::keySize = keySize;
			BTree<K, R, TK, TR>::valueSize = valueSize;
			this->RAM = new Cache(100);
			calculate_node_size();
		}

		BTree(size_t branchingFactor, FileManager *handler, long root_file_pos, int keySize, int valueSize)
			: root(nullptr), n(0) {
			BTree<K, R, TK, TR>::order = branchingFactor;
			BTree<K, R, TK, TR>::fm = handler;
			BTree<K, R, TK, TR>::keySize = keySize;
			BTree<K, R, TK, TR>::valueSize = valueSize;
			this->RAM = new Cache(100);
			calculate_node_size();

			this->root = disk_read(root_file_pos);
		}

		~BTree() {

		}

		//main functions
		Error_code search(K &target) {
			return recursive_search(root, target);
		}

		Error_code insert(const K &new_entry) {
			K median;
			shared_ptr<Node> right_branch, new_root;
			Error_code result = push_down(root, new_entry, median, right_branch);
			if (result == overflow) { // The whole tree grows in height.
									  // Make a brand new root for the whole B-tree.

				//allocate new node on disk
				new_root = alloc();
				//new_root = new Node(this->order);

				new_root->count = 1;
				new_root->data[0] = median;
				new_root->branch[0] = root == nullptr ? -1 : root->file_pos;
				new_root->branch[1] = right_branch == nullptr ? -1 : right_branch->file_pos;

				if (root != nullptr && RAM->find(root->file_pos) == nullptr)
					RAM->add(root, &disk_write);

				root = new_root;
				result = success;
			}
			return result;
		}

		Error_code remove(const K &target) {
			Error_code result = recursive_remove(root, target);
			if (root != nullptr && root->count == 0) { // root is now empty.
				shared_ptr<Node> old_root = root;

				// Read node root->branch[0] from disk
				root = get_node(root->branch[0]);

				// Delete old_root node
				fm->free(old_root->file_pos);
				RAM->remove(old_root);
				//delete old_root;
			}
			return result;
		}

		// save the btree
		void save() {
			if (root != nullptr) {
				//disk_write(root);
				RAM->flush(&disk_write);
			}
		}

		long count() {
			return this->n;
		}

		shared_ptr<Node> get_node(long file_pos) {
			if (file_pos == -1)
				return nullptr;

			auto node = RAM->find(file_pos);
			if (node == nullptr) { //if it is not in the cache
				node = disk_read(file_pos);
				RAM->add(node, &disk_write);
			}
			return node;
		}

		void free_node(shared_ptr<Node> x) {
			if (x != nullptr) {
				auto node = RAM->find(x->file_pos);
				if (node == nullptr) //if it is not in the cache
					delete(x);
			}
		}

		static void disk_write(shared_ptr<Node> &x) {
			//1-count
			//2-branch
			//3-data

			fm->fileStream.seekp(x->file_pos);

			// Write the count of keys the node has
			fm->fileStream.write((char *)&(x->count), sizeof(size_t));

			// Write the references to the childs
			for (size_t i = 0; i < x->branch.size(); i++) {
				fm->fileStream.write((char *)&(x->branch[i]), sizeof(long));
			}

			// Write the keys (K[])
			for (size_t i = 0; i < x->data.size(); i++) {
				auto key = x->data[i].save();
				fm->fileStream.write(key, keySize);
				delete[] key;
				//index += keySize;
			}

			// Write info in the file
			fm->fileStream.flush();

			writes_count++;
		}

		static shared_ptr<Node> disk_read(long file_pos) {
			//1-count
			//2-branch
			//3-data

			int index = 0;
			auto result = make_shared<Node>(order, file_pos);

			// Move to the riquired position in file
			fm->fileStream.seekg(file_pos);

			// Read the node info
			char* bufferNode = new char[nodeSize];
			fm->fileStream.read(bufferNode, nodeSize);

			// Read the count of keys of node
			result->count = *((size_t*)(bufferNode));
			index += sizeof(size_t);

			// Read references to the childs
			for (int i = 0; i <= result->count; i++)
			{
				result->branch[i] = *((long*)(bufferNode + index));
				index += sizeof(long);
			}
			index += ((order) - (result->count + 1)) * sizeof(long);

			// Read the keys (K[])
			char* subBuffer = new char[keySize];
			for (int i = 0; i < result->count; i++)
			{
				strncpy(subBuffer, bufferNode + index, keySize);
				K key;
				key.load(subBuffer, keySize);
				result->data[i] = key;
				index += keySize;
			}

			delete[] subBuffer;
			delete[] bufferNode;

			reads_count++;

			return result;
		}

	protected:
	private:
		Error_code recursive_search(shared_ptr<Node> &current, K &target) {
			Error_code result = not_present;
			size_t position;
			if (current != nullptr) {
				result = search_node(current, target, position);
				if (result == not_present) {
					// Read node current->branch[position] from disk
					auto child = get_node(current->branch[position]);

					result = recursive_search(child, target);
				}
				else
					target = current->data[position];
			}
			return result;
		}

		Error_code search_node(shared_ptr<Node> &current, const K &target, size_t &position) {
			position = 0;
			while (position < current->count && target > current->data[position])
				position++; // Perform a sequential search through the keys.
			return position < current->count && target == current->data[position] ? success : not_present;
		}

		Error_code push_down(shared_ptr<Node> &current, const K &new_entry, K &median, shared_ptr<Node> &right_branch) {
			Error_code result;
			size_t position;
			if (current == nullptr) {
				// Since we cannot insert in an empty tree, the recursion terminates.
				median = new_entry;
				right_branch = nullptr;
				this->n++;
				result = overflow;
			}
			else { // Search the current node.
				/*if (*/search_node(current, new_entry, position);/* == success) {
					result = duplicate_error;
				}
				else {*/
				K extra_entry;
				shared_ptr<Node> extra_branch;

				// Read node current->branch[position] from disk
				auto child = get_node(current->branch[position]);
				result = push_down(child, new_entry, extra_entry, extra_branch);

				if (result == overflow) {
					// Record extra_entry now must be added to current
					if (current->count < this->order - 1) {
						result = success;
						push_in(current, extra_entry, extra_branch, position);
					}
					else
						// Record median and its right_branch will go up to a higher node.
						split_node(current, extra_entry, extra_branch, position, right_branch, median);
				}
				//}
			}
			return result;
		}

		void push_in(shared_ptr<Node> &current, const K &entry, shared_ptr<Node> &right_branch, int position) {
			for (size_t i = current->count; i > position; i--) {
				// Shift all later data to the right.
				current->data[i] = current->data[i - 1];
				current->branch[i + 1] = current->branch[i];
			}
			current->data[position] = entry;
			current->branch[position + 1] = right_branch == nullptr ? -1 : right_branch->file_pos;
			current->count++;

			// Write node current in disk
			//disk_write(current);
		}

		void split_node(shared_ptr<Node> &current, // node to be split
			const K &extra_entry, // new entry to insert
			shared_ptr<Node> &extra_branch, // subtree on right of extra_entry
			int position, // index in node where extra_entry goes
			shared_ptr<Node> &right_half, // new node for right half of entries
			K &median) // median entry (in neither half)
		{
			//allocate new node for the right half
			right_half = alloc();

			int mid = order / 2; // The entries from mid on will go to right_half.
			if (position <= mid) { // First case: extra_entry belongs in left half.
				for (int i = mid; i < order - 1; i++) { // Move entries to right_half.
					right_half->data[i - mid] = current->data[i];
					right_half->branch[i + 1 - mid] = current->branch[i + 1];
				}
				current->count = mid;
				right_half->count = order - 1 - mid;
				push_in(current, extra_entry, extra_branch, position);
			}
			else { // Second case: extra_entry belongs in right half.
				mid++; // Temporarily leave the median in left half.
				for (int i = mid; i < order - 1; i++) { // Move entries to right_half.
					right_half->data[i - mid] = current->data[i];
					right_half->branch[i + 1 - mid] = current->branch[i + 1];
				}
				current->count = mid;
				right_half->count = order - 1 - mid;
				push_in(right_half, extra_entry, extra_branch, position - mid);
			}
			median = current->data[current->count - 1];
			// Remove median from left half.
			right_half->branch[0] = current->branch[current->count];
			current->count--;

			// Write right_half node in disk
			//disk_write(right_half);
			// Write current node in disk
			//disk_write(current);

		}

		Error_code recursive_remove(shared_ptr<Node> &current, const K &target) {
			Error_code result;
			size_t position;
			if (current == nullptr)
				result = not_present;
			else {
				if (search_node(current, target, position) == success) {
					// The target is in the current node.
					result = success;
					if (current->branch[position] != -1) { // not at a leaf node
						copy_in_predecessor(current, position);

						// Read node current->branch[position] from disk
						auto child = get_node(current->branch[position]);

						recursive_remove(child, current->data[position]);
					}
					else
						remove_data(current, position); // Remove from a leaf node.
				}
				else {
					// Read node current->branch[position] from disk
					auto child = get_node(current->branch[position]);

					result = recursive_remove(child, target);
				}
				if (current->branch[position] != -1) {
					// Read node current->branch[position] from disk
					auto child = get_node(current->branch[position]);

					if (child->count < (order - 1) / 2)
						restore(current, position);
				}
			}
			return result;
		}

		void remove_data(shared_ptr<Node> &current, int position) {
			for (int i = position; i < current->count - 1; i++)
				current->data[i] = current->data[i + 1];
			current->count--;

			this->n--;

			// Write current node in disk
			//disk_write(current);
		}

		void copy_in_predecessor(shared_ptr<Node> &current, int position) {
			// Read node current->branch[position] from disk
			auto leaf = get_node(current->branch[position]);
			// First go left from the current entry.
			while (leaf->branch[leaf->count] != -1) {
				leaf = get_node(leaf->branch[leaf->count]); // Move as far rightward as possible.
			}
			current->data[position] = leaf->data[leaf->count - 1];
		}

		void restore(shared_ptr<Node> &current, int position) {
			if (position == current->count) {// case: rightmost branch
				// Read node current->branch[position - 1] from disk
				auto child = get_node(current->branch[position - 1]);
				if (child->count > (order - 1) / 2)
					move_right(current, position - 1);
				else
					combine(current, position);
			}
			else if (position == 0) {// case: leftmost branch
				// Read node current->branch[1] from disk
				auto child = get_node(current->branch[1]);
				if (child->count > (order - 1) / 2)
					move_left(current, 1);
				else
					combine(current, 1);
			}
			else {// remaining cases: intermediate branches
				// Read node current->branch[position - 1] from disk
				auto child = get_node(current->branch[position - 1]);
				if (child->count > (order - 1) / 2)
					move_right(current, position - 1);
				else {
					// Read node current->branch[position + 1] from disk
					child = get_node(current->branch[position + 1]);
					if (child->count > (order - 1) / 2)
						move_left(current, position + 1);
					else
						combine(current, position);
				}
			}
		}

		void move_left(shared_ptr<Node> &current, int position) {
			shared_ptr<Node> left_branch = get_node(current->branch[position - 1]),
				right_branch = get_node(current->branch[position]);
			left_branch->data[left_branch->count] = current->data[position - 1];
			// Take entry from the parent.
			left_branch->branch[++left_branch->count] = right_branch->branch[0];
			current->data[position - 1] = right_branch->data[0];
			// Add the right-hand entry to the parent.
			right_branch->count--;
			for (int i = 0; i < right_branch->count; i++) {
				// Move right-hand entries to fill the hole.
				right_branch->data[i] = right_branch->data[i + 1];
				right_branch->branch[i] = right_branch->branch[i + 1];
			}
			right_branch->branch[right_branch->count] = right_branch->branch[right_branch->count + 1];

			// Write left_branch node in disk
			//disk_write(left_branch);
			// Write right_branch node in disk
			//disk_write(right_branch);
			// Write current node in disk
			//disk_write(current);
		}

		void move_right(shared_ptr<Node> &current, int position) {
			shared_ptr<Node> right_branch = get_node(current->branch[position + 1]),
				left_branch = get_node(current->branch[position]);
			right_branch->branch[right_branch->count + 1] = right_branch->branch[right_branch->count];
			for (int i = right_branch->count; i > 0; i--) { // Make room for new entry.
				right_branch->data[i] = right_branch->data[i - 1];
				right_branch->branch[i] = right_branch->branch[i - 1];
			}
			right_branch->count++;
			right_branch->data[0] = current->data[position];
			// Take entry from parent.
			right_branch->branch[0] = left_branch->branch[left_branch->count--];
			current->data[position] = left_branch->data[left_branch->count];

			// Write left_branch node in disk
			//disk_write(left_branch);
			// Write right_branch node in disk
			//disk_write(right_branch);
			// Write current node in disk
			//disk_write(current);
		}

		void combine(shared_ptr<Node> &current, int position) {
			int i;
			shared_ptr<Node> left_branch = get_node(current->branch[position - 1]),
				right_branch = get_node(current->branch[position]);
			left_branch->data[left_branch->count] = current->data[position - 1];
			left_branch->branch[++left_branch->count] = right_branch->branch[0];
			for (i = 0; i < right_branch->count; i++) {
				left_branch->data[left_branch->count] = right_branch->data[i];
				left_branch->branch[++left_branch->count] = right_branch->branch[i + 1];
			}
			current->count--;
			for (i = position - 1; i < current->count; i++) {
				current->data[i] = current->data[i + 1];
				current->branch[i + 1] = current->branch[i + 2];
			}

			// Delete right_branch node
			fm->free(right_branch->file_pos);
			//RAM->remove(right_branch);
			//delete right_branch;

			// Write left_branch node in disk
			//disk_write(left_branch);
			// Write current node in disk
			//disk_write(current);
		}

		// create and return a new node
		shared_ptr<Node> alloc() {
			long freeMemory = fm->alloc();
			auto result = make_shared<Node>(this->order, freeMemory);

			//Adds node to cache
			RAM->add(result, &disk_write);

			return result;
		}

		void calculate_node_size()
		{
			nodeSize = /*1 +*/                                   //IsLeaf.
				sizeof(size_t) +                                   //count of keys (count).       
				sizeof(long) * (order) +                         //references to the childs (branch).
				valueSize * (order - 1) +            //satellite info (satellite).
				//8 +                                   //position in the file (file_pos).
				keySize * (order - 1);                //keys of the node(Keys).
		}

		// root of the btree
		shared_ptr<Node> root;
		// branching factor
		static size_t order;
		// count of keys
		long n;
		// nodes in ram
		Cache *RAM;
	};

	template<typename K, typename R, typename TK, typename TR>
	FileManager* BTree<K, R, TK, TR, true, true>::fm = nullptr;
	template<typename K, typename R, typename TK, typename TR>
	long BTree<K, R, TK, TR, true, true>::nodeSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int BTree<K, R, TK, TR, true, true>::keySize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int BTree<K, R, TK, TR, true, true>::valueSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	size_t BTree<K, R, TK, TR, true, true>::order = 10;
	template<typename K, typename R, typename TK, typename TR>
	int BTree<K, R, TK, TR, true, true>::reads_count = 0;
	template<typename K, typename R, typename TK, typename TR>
	int BTree<K, R, TK, TR, true, true>::writes_count = 0;
}