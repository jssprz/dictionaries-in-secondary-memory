#pragma once

#include "b-tree-node.h"
#include "key.h"
#include "error-code.h"
#include "file-manager.h"
#include "cache-memory.h"
#include <type_traits>

namespace btree {

	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class BTree { };

	template <typename K, typename R, typename TK, typename TR>
	class BTree<K, R, TK, TR, true, true> {
	public:
		//typedef K::Key K;
		typedef BTreeNode<K, R, TK, TR> Node;
		typedef CacheMemory<K, R, TK, TR> Cache;
		typedef int isSpecialized;

		// file manager
		static FileManager *fm;
		static long nodeSize;
		static int keySize;   //K.Size
		static int valueSize; //R.Size

		//constructor
		BTree(size_t branchingFactor, FileManager *handler, int keySize, int valueSize)
			: root(NULL), n(0) {
			BTree<K, R, TK, TR>::order = branchingFactor;
			BTree<K, R, TK, TR>::fm = handler;
			BTree<K, R, TK, TR>::keySize = keySize;
			BTree<K, R, TK, TR>::valueSize = valueSize;
			this->RAM = new Cache(50);
			calculate_node_size();
		}

		BTree(size_t branchingFactor, FileManager *handler, long root_file_position, int keySize, int valueSize)
			: root(NULL), n(0) {
			BTree<K, R, TK, TR>::order = branchingFactor;
			BTree<K, R, TK, TR>::fm = handler;
			BTree<K, R, TK, TR>::keySize = keySize;
			BTree<K, R, TK, TR>::valueSize = valueSize;
			this->RAM = new Cache(50);
			calculate_node_size();

			this->root = disk_read(root_file_position);
		}

		~BTree() {

		}

		//main functions
		Error_code search(K &target) {
			return recursive_search(root, target);
		}

		Error_code insert(const K &new_entry) {
			K median;
			Node *right_branch, *new_root;
			Error_code result = push_down(root, new_entry, median, right_branch);
			if (result == overflow) { // The whole tree grows in height.
									  // Make a brand new root for the whole B-tree.

				//allocate new node on disk
				new_root = alloc();
				//new_root = new Node(this->order);

				new_root->count = 1;
				new_root->data[0] = median;
				new_root->branch[0] = root == NULL ? -1 : root->file_position;
				new_root->branch[1] = right_branch == NULL ? -1 : right_branch->file_position;

				if (root != NULL && RAM->contains(root->file_position) == NULL)
					RAM->add(root);

				root = new_root;
				result = success;
			}
			return result;
		}

		Error_code remove(const K &target) {
			Error_code result;
			result = recursive_remove(root, target);
			if (root != NULL && root->count == 0) { // root is now empty.
				Node *old_root = root;

				// Read node root->branch[0] from disk
				root = get_node(root->branch[0]);

				// Delete old_root node
				fm->free(old_root->file_position);
				RAM->remove(old_root);
				delete old_root;
			}
			return result;
		}

		// save the btree
		void save() {
			if (root != NULL) {
				disk_write(root);
				RAM->flush();
			}
		}

		long count() {
			return this->n;
		}

		Node* get_node(long file_position) {
			if (file_position == -1)
				return NULL;

			auto node = RAM->find(file_position);
			if (node == NULL) { //if it is not in the cache
				node = disk_read(file_position);
				RAM->add(node);
			}
			return node;
		}

		static void disk_write(Node *x) {
			//1-IsLeaf
			//2-CantKeys
			//3-Children
			//4-Satellite
			//5-PosFile
			//6-Keys

			bool caso = false;
			if (x->file_position == 34304)
				caso = true;

			fm->fileStream.seekp(x->file_position);

			//long index = 0;
			//auto bufferNode = new char[nodeSize];

			// Write the count of keys the node has
			//strncpy(bufferNode, (char *)&(x->count), 4);
			//index += 4;
			fm->fileStream.write((char *)&(x->count), sizeof(size_t));

			//strncpy(bufferNode + index, (char *)&(x->branch[0]), (x->count + 1) * 8);
			// Write the references to the childs
			for (size_t i = 0; i < x->branch.size(); i++) {
				//long f_pos = x->branch[i];
				//memcpy(bufferNode + index, reinterpret_cast<char * const>(&f_pos), 8);
				//index += 8;
				fm->fileStream.write((char *)&(x->branch[i]), sizeof(long));
			}
			//index += ((order) - (x->count + 1)) * 8;

			////Escribimos la información satélite.(R[])
			//for (int i = 0; i < x.CantKeys; i++)
			//{
			//	aux = x.Satellite[i].Save();
			//	Array.Copy(aux, 0, bufferNode, index, valueSize);
			//	index += valueSize;
			//}
			//index += (x.Satellite.Length - x.CantKeys) * valueSize;//nos saltamos los valores no asignados

			// Write the file_position of the node
			//strncpy(bufferNode + index, (char *)&(x->file_position), 8);
			//index += 8;

			// Write the keys (K[])
			for (size_t i = 0; i < x->data.size(); i++) {
				auto key = x->data[i].save();
				//strncpy(bufferNode + index, key, keySize);
				fm->fileStream.write(key, keySize);
				delete[] key;
				//index += keySize;
			}
			//index += (order - x->count) * keySize;

			// Write info in the file
			//fm->fileStream.seekp(x->file_position);
			//fm->fileStream.write(bufferNode, nodeSize);
			fm->fileStream.flush();

			//delete[] bufferNode;
		}

		static Node* disk_read(long file_position) {
			//1-IsLeaf
			//2-CantKeys
			//3-Children
			//4-Satellite
			//5-PosFile
			//6-Keys

			bool caso = false;
			if (file_position == 34304)
				caso = true;

			int index = 0;
			Node *result = new Node(order, file_position);

			// Move to the riquired position in file
			fm->fileStream.seekg(file_position);

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

			//Leemos la informacion satélite.(R[])
			//byte[] subBuffer = new byte[valueSize];
			//for (int i = 0; i < result.CantKeys; i++)
			//{
			//	Array.Copy(bufferNode, index, subBuffer, 0, valueSize);
			//	R valor = new R();
			//	valor.Load(subBuffer);
			//	result.Satellite[i] = valor;
			//	index += valueSize;
			//}
			//index += (result.Satellite.Length - result.CantKeys) * valueSize;//nos saltamos los valores vacios

			// Read the file_position
			//result->file_position = *((long*)(bufferNode + index));
			//index += 8;

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

			return result;
		}

	protected:
	private:
		Error_code recursive_search(Node *current, K &target) {
			Error_code result = not_present;
			size_t position;
			if (current != NULL) {
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

		Error_code search_node(Node *current, const K &target, size_t &position) {
			position = 0;
			while (position < current->count && target > current->data[position])
				position++; // Perform a sequential search through the keys.
			return position < current->count && target == current->data[position] ? success : not_present;
		}

		Error_code push_down(Node *current, const K &new_entry, K &median, Node * &right_branch) {
			Error_code result;
			size_t position;
			if (current == NULL) {
				// Since we cannot insert in an empty tree, the recursion terminates.
				median = new_entry;
				right_branch = NULL;
				this->n++;
				result = overflow;
			}
			else { // Search the current node.
				/*if (*/search_node(current, new_entry, position);/* == success) {
					result = duplicate_error;
				}
				else {*/
				K extra_entry;
				Node *extra_branch;

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

		void push_in(Node *current, const K &entry, Node *right_branch, int position) {
			for (size_t i = current->count; i > position; i--) {
				// Shift all later data to the right.
				current->data[i] = current->data[i - 1];
				current->branch[i + 1] = current->branch[i];
			}
			current->data[position] = entry;
			current->branch[position + 1] = right_branch == NULL ? -1 : right_branch->file_position;
			current->count++;

			// Write node current in disk
			//disk_write(current);
		}

		void split_node(Node *current, // node to be split
			const K &extra_entry, // new entry to insert
			Node *extra_branch, // subtree on right of extra_entry
			int position, // index in node where extra_entry goes
			Node * &right_half, // new node for right half of entries
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

		Error_code recursive_remove(Node *current, const K &target) {
			Error_code result;
			size_t position;
			if (current == NULL)
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

		void remove_data(Node *current, int position) {
			for (int i = position; i < current->count - 1; i++)
				current->data[i] = current->data[i + 1];
			current->count--;

			this->n--;

			// Write current node in disk
			//disk_write(current);
		}

		void copy_in_predecessor(Node *current, int position) {
			// Read node current->branch[position] from disk
			auto leaf = get_node(current->branch[position]);
			// First go left from the current entry.
			while (leaf->branch[leaf->count] != -1) {
				leaf = get_node(leaf->branch[leaf->count]); // Move as far rightward as possible.
			}
			current->data[position] = leaf->data[leaf->count - 1];
		}

		void restore(Node *current, int position) {
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

		void move_left(Node *current, int position) {
			Node *left_branch = get_node(current->branch[position - 1]),
				*right_branch = get_node(current->branch[position]);
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

		void move_right(Node *current, int position) {
			Node *right_branch = get_node(current->branch[position + 1]),
				*left_branch = get_node(current->branch[position]);
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

		void combine(Node *current, int position) {
			int i;
			Node *left_branch = get_node(current->branch[position - 1]),
				*right_branch = get_node(current->branch[position]);
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
			fm->free(right_branch->file_position);
			RAM->remove(right_branch);
			delete right_branch;

			// Write left_branch node in disk
			//disk_write(left_branch);
			// Write current node in disk
			//disk_write(current);
		}

		// create and return a new node
		Node* alloc() {
			long freeMemory = fm->alloc();
			Node *result = new Node(this->order, freeMemory);

			//Adds node to cache
			RAM->add(result);

			return result;
		}

		void calculate_node_size()
		{
			BTree<K, R, TK, TR>::nodeSize = /*1 +*/                                   //IsLeaf.
				sizeof(size_t) +                                   //count of keys (count).       
				sizeof(long) * (this->order) +                         //references to the childs (branch).
				valueSize * (this->order - 1) +            //satellite info (satellite).
				//8 +                                   //position in the file (file_position).
				keySize * (this->order - 1);                //keys of the node(Keys).
		}

		// root of the btree
		Node *root;
		// branching factor
		static size_t order;
		// count of keys
		long n;
		// nodes in ram
		Cache *RAM;
	};

	template<typename K, typename R, typename TK, typename TR>
	FileManager* BTree<K, R, TK, TR, true, true>::fm = NULL;
	template<typename K, typename R, typename TK, typename TR>
	long BTree<K, R, TK, TR, true, true>::nodeSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int BTree<K, R, TK, TR, true, true>::keySize = 0;
	template<typename K, typename R, typename TK, typename TR>
	int BTree<K, R, TK, TR, true, true>::valueSize = 0;
	template<typename K, typename R, typename TK, typename TR>
	size_t BTree<K, R, TK, TR, true, true>::order = 10;
}