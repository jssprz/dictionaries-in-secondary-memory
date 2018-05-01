#pragma once

namespace btree {
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