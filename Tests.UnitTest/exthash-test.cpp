#include "stdafx.h"
#include "CppUnitTest.h"

#include "ext-hashing.h"
#include "types-def.h"
#include <algorithm>    // std::random_shuffle

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace extensible_hashing;

#define DATA_SIZE 1048576
#define ADN_LENGTH 15

namespace TestsUnitTest {
	TEST_CLASS(ExtHashingUnitTest) {
	public:
		ExtensibleHashing<ADN, RecordDef, string, string> *tree = NULL;
		FileManager *fm = NULL;
		vector<ADN> data;

		ExtHashingUnitTest() {
			data.reserve(DATA_SIZE);
			for (size_t i = 0; i < DATA_SIZE; i++)
				data.push_back(ADN::generate_adn(ADN_LENGTH));
		}

		TEST_METHOD_INITIALIZE(ExtHashingInitialization) {

		}

		TEST_METHOD_CLEANUP(ExtHashingCleanUp) {
		}

		TEST_METHOD(ExtHashingSearchEmptyTest) {
			fm = new FileManager("ehash-tests/EHashSearchEmptyTest.ehash", 512);
			tree = new ExtensibleHashing<ADN, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(ExtHashingInsertEmptyAndSearchTest) {
			fm = new FileManager("ehash-tests/EHashInsertEmptyAndSearchTest.ehash", 512);
			tree = new ExtensibleHashing<ADN, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
			Assert::AreEqual(int(success), int(tree->insert(s)));
			Assert::AreEqual(int(success), int(tree->search(s)));
			Assert::AreEqual(int(not_present), int(tree->search(ADN(s.get_value() + "A"))));
		}

		TEST_METHOD(ExtHashingInsertAndSearchTest) {
			fm = new FileManager("ehash-tests/EHashInsertAndSearchTest.ehash", 512);
			tree = new ExtensibleHashing<ADN, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			for (size_t i = 0; i < data.size(); i++) {
				auto result = tree->insert(data[i]);
				Assert::AreEqual(int(success), int(result));
				result = tree->search(data[i]);
				Assert::AreEqual(int(success), int(result));
			}
		}

		TEST_METHOD(ExtHashingLoadFromFileAndSearchTest) {
			//create a file, write freeMemory, hs and he at the beginning
			//create the EHash using filemanager with this file
			//insert all elements
			//save and close the file
			//create the new EHash using this file
			//search all elements
		}

		TEST_METHOD(ExtHashingInsertDulicatedSearchAndRemoveTest) {
			fm = new FileManager("ehash-tests/EHashInsertDulicatedSearchAndRemoveTest.ehash", 512);
			tree = new ExtensibleHashing<ADN, RecordDef, string, string>(5, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
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
				Assert::AreEqual(i - 1, tree->count());
			}
		}

		TEST_METHOD(ExtHashingRemoveEmptyTest) {
			fm = new FileManager("ehash-tests/EHashRemoveEmptyTest.ehash", 512);
			tree = new ExtensibleHashing<ADN, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(ExtHashingRemoveTest) {
			fm = new FileManager("ehash-tests/EHashRemoveTest.ehash", 512);
			tree = new ExtensibleHashing<ADN, RecordDef, string, string>(22, fm, ADN_LENGTH, 0);

			vector<ADN> data1;

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
	};
}