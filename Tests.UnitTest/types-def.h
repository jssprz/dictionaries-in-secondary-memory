#pragma once

#include "../common_headers/data-types.h""
#include <string>
#include <vector>

using namespace std;
using namespace common;

class ADN : public Key<string> {
public:
	ADN() {

	}
	ADN(string value) {
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
		string s(buffer, count);
		this->set_value(s);
	}
	//long long hash() {
		//long long hash = 5381;
		//int c;
		//auto str = value.c_str();
		//while (c = *str++)
		//	hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	//}
	long long hash() {
		long long hash = 0;
		for (int i = 0; i < value.length(); i++) {
			//C - 01, G - 10, T - 11, A - 00
			hash += value[i] == 'C' ? pow(2, i * 2 + 1) : value[i] == 'G' ? pow(2, i * 2) : value[i] == 'T' ? pow(2, i * 2) + pow(2, i * 2 + 1) : 0;
		}
		return hash;
	}

	static ADN generate_adn(const size_t length) {
		string str;
		for (size_t i = 0; i < length; ++i) {
			str += gen_rand();
		}
		return ADN(str);
	}

private:
	static char gen_rand() {
		static const char alpha[] = "ATCG";
		return alpha[rand() % (sizeof(alpha) - 1)];
	}
};

class RecordDef : public Record<string> {
public:
	RecordDef() {

	}
	RecordDef(string value) {
		this->value = value;
	}
	char* save() {
		vector<char> bytes(this->value.begin(), this->value.end());
		return &bytes[0];
	}
};