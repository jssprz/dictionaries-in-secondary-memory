#include "stdafx.h"
#include "CppUnitTest.h"

#include "types-def.h"
#include <b-tree.h>
#include <file-manager.h>
#include <vector>
#include <algorithm>    // std::random_shuffle

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace btree;

#define DATA_SIZE 1048576
#define ADN_LENGTH 15

namespace TestsUnitTest {
	

	TEST_CLASS(BTreeUnitTest)
	{
	public:
		BTree<KeyDef, RecordDef, string, string> *tree = NULL;
		FileManager *fm = NULL;
		vector<KeyDef> data;

		BTreeUnitTest() {
			data.reserve(DATA_SIZE);
			for (size_t i = 0; i < DATA_SIZE; i++)
				data.push_back(KeyDef(this->generate_adn(ADN_LENGTH)));
		}

		TEST_METHOD_INITIALIZE(BtreeInitialization) {
			//fm = new FileManager("test.btree", 512);
			//tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);
		}

		TEST_METHOD_CLEANUP(BTreeCleanUp) {
			if (tree != NULL) {
				tree->save();
				delete tree;
			}
			if(fm != NULL)
				delete fm;
		}

		TEST_METHOD(BtreeSearchEmptyTest) {
			fm = new FileManager("btree-tests/BtreeSearchEmptyTest.btree", 512);
			tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			KeyDef s(generate_adn(ADN_LENGTH));
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(BtreeInsertEmptyAndSearchTest) {
			fm = new FileManager("btree-tests/BtreeInsertEmptyAndSearchTest.btree", 512);
			tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			KeyDef s(generate_adn(ADN_LENGTH));
			Assert::AreEqual(int(success), int(tree->insert(s)));
			Assert::AreEqual(int(success), int(tree->search(s)));
			Assert::AreEqual(int(not_present), int(tree->search(KeyDef(s.get_value() + "A"))));
		}

		TEST_METHOD(BtreeInsertAndSearchTest) {
			fm = new FileManager("btree-tests/BtreeInsertAndSearchTest.btree", 512);
			tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			for (size_t i = 0; i < data.size(); i++) {
				auto result = tree->insert(data[i]);
				Assert::AreEqual(int(success), int(result));
				result = tree->search(data[i]);
				Assert::AreEqual(int(success), int(result));
			}
		}

		TEST_METHOD(BtreeLoadFromFileAndSearchTest) {
			//create a file, write freeMemory, hs and he at the beginning
			//create the btree using filemanager with this file
			//insert all elements
			//save and close the file
			//create the new btree using this file
			//search all elements
		}

		TEST_METHOD(BtreeInsertDulicatedSearchAndRemoveTest) {
			fm = new FileManager("btree-tests/BtreeInsertDulicatedSearchAndRemoveTest.btree", 512);
			tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			KeyDef s(generate_adn(ADN_LENGTH));
			long i;
			for (i = 1; i <= 50; i++) {
				tree->insert(s);
				//Assert::AreEqual(int(duplicate_error), int(tree->insert(s)));
				Assert::AreEqual(i, tree->count());
			}

			Assert::AreEqual(int(success), int(tree->search(s)));

			i--;
			for (; i >= 1; i--) {
				tree->remove(s);
				Assert::AreEqual(i - 1,tree->count());
			}
		}

		TEST_METHOD(BtreeRemoveEmptyTest) {
			fm = new FileManager("btree-tests/BtreeRemoveEmptyTest.btree", 512);
			tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			KeyDef s(generate_adn(ADN_LENGTH));
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(BtreeRemoveTest) {
			fm = new FileManager("btree-tests/BtreeRemoveTest.btree", 512);
			tree = new BTree<KeyDef, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			vector<KeyDef> data1;

			// insert all elements in the tree
			for (size_t i = 0; i < 5000; i++) {
				tree->insert(data[i]);
				data1.push_back(data[i]);
			}

			// random shuffle data
			random_shuffle(data1.begin(), data1.end());

			for (size_t i = 0; i < data1.size(); i++) {
				// remove current element from tree
				Assert::AreEqual(int(success), int(tree->remove(data[i])));

				// check the removed element is not present in the tree
				// Assert::AreEqual(int(not_present), int(tree->search(data[i])));

				// check all other elements are in the tree
				for (size_t j = i + 1; j < data1.size(); j++)
					Assert::AreEqual(int(success), int(tree->search(data[j])));
			}
		}

	private:
		char gen_rand() {
			static const char alpha[] = "ATCG";
			return alpha[rand() % (sizeof(alpha) - 1)];
		}

		string generate_adn(const size_t length) {
			string str;
			for (size_t i = 0; i < length; ++i) {
				str += gen_rand();
			}
			return str;
		}

	};
}