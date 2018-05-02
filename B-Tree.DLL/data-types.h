#pragma once

namespace btree {
	template<typename T>
	class Key {
	public:
		virtual void load(char* buffer, size_t count) = 0;
		virtual char* save() = 0;
		T get_value() {
			return value;
		}
		void set_value(T value) {
			this->value = value;
		}
		T& operator=(const T& other) // copy assignment
		{
			if (this != &other) { // self-assignment check expected
				value = other.value;
			}
			return *this;
		}
		friend bool operator>(const Key& l, const Key& r) {
			return l.value > r.value; // keep the same order
		}
		friend bool operator==(const Key& l, const Key& r) {
			return l.value == r.value; // keep the same order
		}

	protected:
		T value;
	};

	template<typename T>
	class Record {
	public:
		virtual char* save() = 0;
		T get_value() {
			return value;
		}
		T set_value(T value) {
			this->value = value;
		}
	protected:
		T value;
	};
}