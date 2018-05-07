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
#define MSG(msg) [&]{ std::wstringstream _s; _s << msg; return _s.str(); }().c_str()

namespace TestsUnitTest {
	TEST_CLASS(ExtHashingUnitTest) {
public:
	ExtensibleHashing<ADN, RecordDef, string, string> *tree = NULL;
	FileManager *fm = NULL;

	ExtHashingUnitTest() {
		/*data.reserve(DATA_SIZE);
		for (size_t i = 0; i < DATA_SIZE; i++)
			data.push_back(ADN::generate_adn(ADN_LENGTH));*/
	}

	TEST_METHOD_INITIALIZE(ExtHashingInitialization) {

	}

	TEST_METHOD_CLEANUP(ExtHashingCleanUp) {
		if (tree != NULL) {
			tree->save();
			delete tree;
		}
		if (fm != NULL)
			delete fm;
	}

	TEST_METHOD(ExtHashigImportantTest) {
		fm = new FileManager("ehash-tests/EHashImportantTest.ehash", 512);
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(32, 30, fm, ADN_LENGTH, 0);

		vector<ADN> inserted;
		//for i in {15,...,20}
		int size = 0;
		for (int i = 15; i <= 19; i++)
			size += pow(2, i);
		inserted.reserve(size);
		for (int i = 15; i <= 19; i++) {
			//generate 2^i ADN strings
			int count = pow(2, i);
			for (int j = 0; j < count; j++) {
				auto adn = ADN::generate_adn(ADN_LENGTH);
				//insert in the structure
				Assert::AreEqual(int(success), int(tree->insert(adn)));
				inserted.push_back(adn);
			}

			//select 1000 random insterted ADN strings and serach each one
			random_shuffle(inserted.begin(), inserted.end());
			for (int i = 0; i < 1000; i++)
				Assert::AreEqual(int(success), int(tree->search(inserted[i])));

			//generate 1000 random not inserted strings and search each one
			for (int i = 0; i < 1000; i++) {
				auto not_adn = ADN::generate_adn(ADN_LENGTH);
				string value = not_adn.get_value();
				value[rand() % ADN_LENGTH] = 'B';
				not_adn.set_value(value);
				Assert::AreEqual(int(not_present), int(tree->search(not_adn)));
			}
		}

		//for (vector<ADN>::iterator it = inserted.begin(); it != inserted.end(); it++)
		//	Assert::AreEqual(int(success), int(tree->remove(*it)));
		int count_to_delete = inserted.size();
		for (int i = 0; i < count_to_delete; i++)
		{
			if (i != 382275)
				Assert::AreEqual(int(success), int(tree->remove(inserted[i])), MSG("Element " << i << " failed removing"));
			else
				Assert::AreEqual(int(success), int(tree->remove(inserted[i])), MSG("Element " << i << " failed removing"));
			/*auto result = tree->remove(inserted[i]);
			if (result == not_present)
				result = success;*/
		}

		Assert::AreEqual(0L, tree->count());
	}

	TEST_METHOD(ExtHashingSearchEmptyTest) {
		fm = new FileManager("ehash-tests/EHashSearchEmptyTest.ehash", 512);
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(32, 30, fm, ADN_LENGTH, 0);

		auto s = ADN::generate_adn(ADN_LENGTH);
		Assert::AreEqual(int(not_present), int(tree->search(s)));
	}

	TEST_METHOD(ExtHashingInsertEmptyAndSearchTest) {
		fm = new FileManager("ehash-tests/EHashInsertEmptyAndSearchTest.ehash", 512);
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(32, 30, fm, ADN_LENGTH, 0);

		auto s = ADN::generate_adn(ADN_LENGTH);
		Assert::AreEqual(int(success), int(tree->insert(s)));
		Assert::AreEqual(int(success), int(tree->search(s)));
		Assert::AreEqual(int(not_present), int(tree->search(ADN(s.get_value() + "A"))));
	}

	TEST_METHOD(ExtHashingInsertAndSearchTest) {
		fm = new FileManager("ehash-tests/EHashInsertAndSearchTest.ehash", 512);
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(32, 30, fm, ADN_LENGTH, 0);

		int count = 1000000;
		vector<ADN> data = generate_data(count);

		for (size_t i = 0; i < count; i++) {
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
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(5, 30, fm, ADN_LENGTH, 0);

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
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(32, 30, fm, ADN_LENGTH, 0);

		auto s = ADN::generate_adn(ADN_LENGTH);
		Assert::AreEqual(int(not_present), int(tree->search(s)));
	}

	TEST_METHOD(ExtHashingRemoveTest) {
		fm = new FileManager("ehash-tests/EHashRemoveTest.ehash", 512);
		tree = new ExtensibleHashing<ADN, RecordDef, string, string>(32, 30, fm, ADN_LENGTH, 0);

		int count = 5000;
		vector<ADN> data = generate_data(count);

		// insert all elements in the tree
		for (size_t i = 0; i < count; i++)
			tree->insert(data[i]);

		// random shuffle data
		random_shuffle(data.begin(), data.end());

		for (size_t i = 0; i < count; i++) {
			// remove current element from tree
			Assert::AreEqual(int(success), int(tree->remove(data[i])), MSG("Element " << i << " failed removing"));

			// check the removed element is not present in the tree
			// Assert::AreEqual(int(not_present), int(tree->search(data[i])));

			// check all other elements are in the tree
			for (size_t j = i + 1; j < data.size(); j++)
				Assert::AreEqual(int(success), int(tree->search(data[j])), MSG("Element " << j << " failed searching"));
		}
	}

private:
	vector<ADN> generate_data(size_t count) {
		vector<ADN> result;
		result.reserve(count);
		for (size_t i = 0; i < count; i++)
			result.push_back(ADN::generate_adn(ADN_LENGTH));
		return result;
	}
	};
}