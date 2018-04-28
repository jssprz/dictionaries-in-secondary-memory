
#include <iostream>
#include "b-tree.h"


using namespace std;
using namespace btree;


char gen_rand() {
	static const char alpha[] = "ATCG";
	return alpha[rand() % (sizeof(alpha) - 1)];
}

string generate_adn(const size_t length){
	// If you use C++'s std::string:
	string str;
	for (size_t i = 0; i < length; ++i) {
		str += gen_rand();
	}
	return str;
}

int main() {
	cout << "Hy" << endl;

	auto db = new BTree<string, string>(20);

	cout << "generating..." << endl;
	string data[1000];
	for (size_t i = 0; i < 1000; i++)
		data[i] = generate_adn(15);

	cout << "inserting..." << endl;
	for (size_t i = 0; i < 1000; i++)
		cout << i << ToString(db->insert(data[i])) << endl;

	cout << "searching..." << endl;
	for (size_t i = 0; i < 1000; i++)
		cout << i << ToString(db->search(data[i])) << endl;

	cout << "deleting..." << endl;
	for (size_t i = 0; i < 1000; i++)
		cout << i << ToString(db->remove(data[i])) << endl;

	return 0;
}