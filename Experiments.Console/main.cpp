
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
	string s = generate_adn(15);

	cout << "inserting..." << endl;
	cout << db->insert(s) << endl;

	cout << "searching..." << endl;
	cout << int(db->search(s)) << endl;

	cout << "searching..." << endl;
	cout << int(db->remove(s)) << endl;

	return 0;
}