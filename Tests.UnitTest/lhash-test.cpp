#include "stdafx.h"
#include "CppUnitTest.h"

#include "linear-hashing.h"
#include "types-def.h"
#include <algorithm>    // std::random_shuffle

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace linear_hashing;

#define ADN_LENGTH 15
#define MSG(msg) [&]{ std::wstringstream _s; _s << msg; return _s.str(); }().c_str()

namespace TestsUnitTest {
	TEST_CLASS(LinearHashingUnitTest) {
public:
	LinearHashing<ADN, RecordDef, string, string> *tree = nullptr;
	FileManager *fm = nullptr;

	LinearHashingUnitTest() {
	}

	TEST_METHOD_INITIALIZE(LinearHashingInitialization) {
	}

	TEST_METHOD_CLEANUP(LinearHashingCleanUp) {
		if (tree != nullptr) {
			tree->save();
			delete tree;
		}
		if (fm != nullptr)
			delete fm;
	}

	TEST_METHOD(LinearHashingImportantTest) {
		fm = new FileManager("lhash-tests/LHashSearchEmptyTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

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
			Assert::AreEqual(int(success), int(tree->remove(inserted[i])), MSG("Element " << i << " failed removing"));

		Assert::AreEqual(0L, tree->count());
	}

	TEST_METHOD(LinearHashingSearchEmptyTest) {
		fm = new FileManager("lhash-tests/LHashSearchEmptyTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

		auto s = ADN::generate_adn(ADN_LENGTH);
		Assert::AreEqual(int(not_present), int(tree->search(s)));
	}

	TEST_METHOD(LinearHashingInsertEmptyAndSearchTest) {
		fm = new FileManager("lhash-tests/LHashInsertEmptyAndSearchTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

		auto s = ADN::generate_adn(ADN_LENGTH);
		Assert::AreEqual(int(success), int(tree->insert(s)));
		Assert::AreEqual(int(success), int(tree->search(s)));
		Assert::AreEqual(int(not_present), int(tree->search(ADN(s.get_value() + "A"))));
	}

	TEST_METHOD(LinearHashingInsertAndSearchTest) {
		fm = new FileManager("lhash-tests/LHashInsertAndSearchTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

		int count = 1000000;
		vector<ADN> data = generate_data(count);

		for (size_t i = 0; i < count; i++) {
			auto result = tree->insert(data[i]);
			Assert::AreEqual(int(success), int(result));
			result = tree->search(data[i]);
			Assert::AreEqual(int(success), int(result));
		}
	}

	TEST_METHOD(LinearHashingLoadFromFileAndSearchTest) {
		//create a file, write freeMemory, hs and he at the beginning
		//create the EHash using filemanager with this file
		//insert all elements
		//save and close the file
		//create the new EHash using this file
		//search all elements
	}

	TEST_METHOD(LinearHashingInsertDulicatedSearchAndRemoveTest) {
		fm = new FileManager("lhash-tests/LHashInsertDulicatedSearchAndRemoveTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

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

	TEST_METHOD(LinearHashingRemoveEmptyTest) {
		fm = new FileManager("lhash-tests/LHashRemoveEmptyTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

		auto s = ADN::generate_adn(ADN_LENGTH);
		Assert::AreEqual(int(not_present), int(tree->search(s)));
	}

	TEST_METHOD(LinearHashingRemoveTest) {
		fm = new FileManager("lhash-tests/LHashRemoveTest.ehash", 512);
		tree = new LinearHashing<ADN, RecordDef, string, string>(32, maximum_average_search_cost, 4, fm, ADN_LENGTH, 0);
		//tree = new LinearHashing<ADN, RecordDef, string, string>(32, minimum_filled_percent, 0.5, fm, ADN_LENGTH, 0);

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