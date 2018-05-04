#pragma once

#include <list>

using namespace std;

namespace common {
	template<typename T>
	class CacheMemory {

	public:
		// Constructs a new cache
		// capacity: capacity of the cache
		CacheMemory(int capacity)
			: _cache(list<T*>()), _capacity(capacity) {
		}

		// adds a node to the cache
		void add(T* x, void (*write)(T*)) {
			if (_cache.size() == _capacity) {
				auto last = _cache.back();
				write(last);
				_cache.pop_back();
				//delete last;
			}
			_cache.push_front(x);
		}

		// determine if a node is in cache and returns it
		T* contains(long file_pos) {
			for (list<T*>::iterator it = _cache.begin(); it != _cache.end(); ++it)
				if ((*it)->file_pos == file_pos)
					return *it;
			return NULL;
		}

		// returns a node of cache
		T* find(long file_pos) {
			auto result = contains(file_pos);
			if (result == NULL)
				return NULL;
			_cache.remove(result);
			_cache.push_front(result);
			return _cache.front();
		}

		// write in disk all nodes of cache
		void flush(void (*write)(T*)) {
			for (list<T*>::iterator it = _cache.begin(); it != _cache.end(); ++it) {
				write(*it);
				delete *it;
			}
			_cache.clear();
		}

		// update that was changed
		void update(T *x) {
			T *result = contains(x->file_pos);
			if (result == NULL) //if there is not in cache
				this->add(x);
			else //if there is in cache
				result = x;
		}

		// removes a node of cache, but only when its memory is freed
		void remove(T *x) {
			T *result = contains(x->file_pos);
			if (result == NULL)
				return;
			_cache.remove(result);
		}

		int size() {
			return this->_cache.size();
		}

		int capacity() {
			return this->_capacity;
		}

	private:

		list<T*> _cache;
		int _capacity;
	};
}