#pragma once

#include "b-tree.h"
#include "b-tree-node.h"
#include "key.h"
#include "record.h"
#include <list>

using namespace std;

namespace btree {
	template <typename K, typename R, typename TK, typename TR,
		bool = std::is_base_of<Key<typename TK>, K>::value, bool = std::is_base_of<Record<typename TR>, R>::value>
		class CacheMemory { };

	template<typename K, typename R, typename TK, typename TR>
	class CacheMemory<K, R, TK, TR, true, true> {
		typedef BTreeNode<K, R, TK, TR> Node;

	public:
		// Constructs a new cache
		// capacity: capacity of the cache
		CacheMemory(int capacity)
			: _cache(list<Node*>()), _capacity(capacity) {
		}

		// adds a node to the cache
		void add(Node* x) {
			if (this->_cache.size() == _capacity)
			{
				BTree<K, R, TK, TR>::disk_write(this->_cache.back());
				this->_cache.pop_back();
			}
			this->_cache.push_front(x);
		}

		// determine if a node is in cache and returns it
		Node* contains(long file_position) {
			for (list<Node*>::iterator it = mylist.begin(); it != mylist.end(); ++it)
				if ((*it)->file_position == file_position)
					return *it;
			return NULL;
		}

		// returns a node of cache
		Node* find(long file_position) {
			auto result = this->contains(file_position);
			if (result == NULL)
				return NULL;
			this->_cache.remove(result);
			this->_cache.push_front(result);
			return this->_cache.front();
		}

		// write in disk all nodes of cache
		void flush() {
			for (list<Node*>::iterator it = mylist.begin(); it != mylist.end(); ++it)
				BTree<K, R>::disk_write(*it);
			this->cache = list(capacity);
		}

		// update that was changed
		void update(Node *x) {
			Node *result = contains(x->file_position);
			if (result == NULL) //if there is not in cache
				this->add(x);
			else //if there is in cache
				result = x;
		}

		// removes a node of cache, but only when its memory is freed
		void remove(Node *x) {
			Node *result = contains(x->file_position);
			if (result = NULL)
				return;
			this->_cache.remove(result);
		}

		int size() {
			return this->_cache.size();
		}

		int capacity() {
			return this->_capacity;
		}

	private:

		list<Node*> _cache;
		int _capacity;
	};
}