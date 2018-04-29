
#include <iostream>
#include "b-tree.h"


using namespace std;
using namespace btree;


char gen_rand() {
	static const char alpha[] = "ATCG";
	return alpha[rand() % (sizeof(alpha) - 1)];
}

string generate_adn(const size_t length){
	string str;
	for (size_t i = 0; i < length; ++i) {
		str += gen_rand();
	}
	return str;
}

void print_btree(BTreeNode<string, string>  *current, string level) {
	if (current == NULL)
		cout << (level + "NULL").c_str() << endl;
	else {
		for (size_t i = 0; i < current->count; i++)
		{
			print_btree(current->branch[i], level + ' ');
			cout << level.c_str() << current->data[i].c_str() << endl;
		}
		print_btree(current->branch[current->count], level + ' ');
	}
}

int main() {
	cout << "Hy" << endl;
	auto fm = new FileManager("", 20);
	auto db = new BTree<string, string>(3, fm);

	//print_btree(db->root, "");

	/*cout << "generating..." << endl;
	string data[1000];
	for (size_t i = 0; i < 1000; i++)
		data[i] = generate_adn(15);
	
	char c;
	cout << "inserting..." << endl;
	for (size_t i = 0; i < 1000; i++) {
		cout << i << ToString(db->insert(data[i])) << endl;
	}

	cout << "searching..." << endl;
	for (size_t i = 0; i < 1000; i++)
		cout << i << ToString(db->search(data[i])) << endl;

	cout << "deleting..." << endl;
	for (size_t i = 0; i < 1000; i++) {
		cout << i << ToString(db->remove(data[i])) << endl;
		print_btree(db->root, "");
		cin >> c;
	}*/

	//cout << c;
	return 0;
}