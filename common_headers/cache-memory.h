#pragma once

#include <list>
#include <memory>

using namespace std;

namespace common {
	template<typename T>
	class CacheMemory {

	public:
		// Constructs a new cache
		// capacity: capacity of the cache
		CacheMemory(int capacity)
			: _cache(list<shared_ptr<T>>()), _capacity(capacity) {
		}

		// adds a node to the cache
		void add(shared_ptr<T> &x, void (*write)(shared_ptr<T>&)) {
			if (_cache.size() == _capacity) {
				write(_cache.back());
				//delete last;
				_cache.pop_back();
			}
			_cache.push_front(x);
		}

		// determine if a node is in cache and returns it
		shared_ptr<T> contains(long file_pos) {
			for (auto it = _cache.begin(); it != _cache.end(); ++it)
				if ((*it)->file_pos == file_pos) {
					//auto result = move(*it);
					return *it;
				}
			return nullptr;
		}

		// returns a node of cache
		shared_ptr<T> find(long file_pos) {
			auto result = contains(file_pos);
			if (result == nullptr)
				return nullptr;
			_cache.remove(result);
			_cache.push_front(result);
			return _cache.front();
		}

		// write in disk all nodes of cache
		void flush(void (*write)(shared_ptr<T>&)) {
			for (list<shared_ptr<T>>::iterator it = _cache.begin(); it != _cache.end(); ++it) {
				write(*it);
			}
			_cache.clear();
		}

		// update that was changed
		void update(shared_ptr<T> &x) {
			shared_ptr<T> result = contains(x->file_pos);
			if (result == nullptr) //if there is not in cache
				this->add(x);
			else //if there is in cache
				result = x;
		}

		// removes a node of cache, but only when its memory is freed
		void remove(shared_ptr<T> &x) {
			shared_ptr<T> result = contains(x->file_pos);
			if (result == nullptr)
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

		list<shared_ptr<T>> _cache;
		int _capacity;
	};
}