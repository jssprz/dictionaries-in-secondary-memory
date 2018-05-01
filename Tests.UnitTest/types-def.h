#pragma once

#include <key.h>
#include <record.h>
#include <string>
#include <vector>

class KeyDef : public btree::Key<std::string> {
public:
	KeyDef() {

	}
	KeyDef(std::string value) {
		this->value = value;
	}
	char* save() {
		size_t len = this->value.length();
		auto result = new char[len];
		for (size_t i = 0; i < len; i++)
			result[i] = this->value[i];
		return result;
	}
	void load(char *buffer, size_t count) {
		std::string s(buffer, count);
		this->set_value(s);
	}
};

class RecordDef : public btree::Record<std::string> {
public:
	RecordDef() {

	}
	RecordDef(std::string value) {
		this->value = value;
	}
	char* save() {
		std::vector<char> bytes(this->value.begin(), this->value.end());
		return &bytes[0];
	}
};