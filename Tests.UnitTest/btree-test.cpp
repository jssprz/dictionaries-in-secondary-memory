#include "stdafx.h"
#include "CppUnitTest.h"

#include "b-tree.h"
#include <vector>
#include <algorithm>    // std::random_shuffle

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace btree;

#define DATA_SIZE 1048576

namespace TestsUnitTest
{
	TEST_CLASS(BTreeUnitTest)
	{
	public:
		BTree<string, string> *tree;
		vector<string> data;

		BTreeUnitTest() {
			for (size_t i = 0; i < DATA_SIZE; i++)
				data.push_back(this->generate_adn(15));
		}

		TEST_METHOD_INITIALIZE(BtreeInitialization) {
			tree = new BTree<string, string>(20);
		}

		TEST_METHOD_CLEANUP(BTreeCleanUp) {
			delete tree;
		}
		
		TEST_METHOD(BtreeSearchEmptyTest)
		{
			string s = generate_adn(15);
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(BtreeInsertEmptyAndSearchTest)
		{
			string s = generate_adn(15);
			Assert::AreEqual(int(success), int(tree->insert(s)));
			Assert::AreEqual(int(success), int(tree->search(s)));
			Assert::AreEqual(int(not_present), int(tree->search(s+"A")));
		}

		TEST_METHOD(BtreeInsertAndSearchTest)
		{
			for (size_t i = 0; i < data.size(); i++)
			{
				auto result = tree->insert(data[i]);
				Assert::AreEqual(int(success), int(result));
				result = tree->search(data[i]);
				Assert::AreEqual(int(success), int(result));
			}
		}

		TEST_METHOD(BtreeInsertDulicatedSearchAndRemoveTest)
		{
			string s = generate_adn(15);

			long i;
			for (i = 0; i < 50; i++) {
				tree->insert(s);
				//Assert::AreEqual(int(duplicate_error), int(tree->insert(s)));
				Assert::AreEqual(tree->n, i + 1);
			}

			Assert::AreEqual(int(success), int(tree->search(s)));

			for (; i > 0; i--) {
				tree->remove(s);
				Assert::AreEqual(tree->n, i - 1);
			}
		}
		
		TEST_METHOD(BtreeRemoveEmptyTest)
		{
			string s = generate_adn(15);
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(BtreeInsertRemoveSearchTest)
		{
			for (size_t i = 0; i < data.size(); i++) {
				auto result = tree->insert(data[i]);
				Assert::AreEqual(int(success), int(result));
				result = tree->remove(data[i]);
				Assert::AreEqual(int(success), int(result));
				result = tree->search(data[i]);
				Assert::AreEqual(int(not_present), int(result));
			}
		}

		TEST_METHOD(BtreeRemoveTest)
		{
			// insert all elements in the tree
			for (size_t i = 0; i < data.size(); i++)
				tree->insert(data[i]);

			// random shuffle data
			random_shuffle(data.begin(), data.end());

			size_t count = min(int(data.size()), 5000);
			for (size_t i = 0; i < count; i++) {
				// remove current element from tree
				Assert::AreEqual(int(success), int(tree->remove(data[i])));
				
				// check the removed element is not present in the tree
				// Assert::AreEqual(int(not_present), int(tree->search(data[i])));
				
				// check all other elements are in the tree
				for (size_t j = i + 1; j < count; j++)
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