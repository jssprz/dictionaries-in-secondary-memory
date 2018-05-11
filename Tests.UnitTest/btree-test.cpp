#include "stdafx.h"
#include "CppUnitTest.h"

#include "types-def.h"
#include <b-tree.h>
#include <../common_headers/file-manager.h>
#include <vector>
#include <algorithm>    // std::random_shuffle
#include "io_opers.h"
#include "timer.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;
using namespace btree;
using namespace common;

#define DATA_SIZE 1048576
#define ADN_LENGTH 15
#define MSG(msg) [&]{ std::wstringstream _s; _s << msg; return _s.str(); }().c_str()

DECLARE_TIMING(bt_insertion_timer);
DECLARE_TIMING(bt_deletion_timer);
DECLARE_TIMING(bt_searching_timer);
DECLARE_IOPERS(bt_insertion_io_counter);
DECLARE_IOPERS(bt_deletion_io_counter);
DECLARE_IOPERS(bt_searching_io_counter);

namespace TestsUnitTest {
	TEST_CLASS(BTreeUnitTest)
	{
	public:
		typedef BTree<ADN, RecordDef, string, string> BT;

		BT *tree = nullptr;
		FileManager *fm = nullptr;
		ofstream report_file;

		BTreeUnitTest() {
		}

		TEST_METHOD_INITIALIZE(BtreeInitialization) {
			//fm = new FileManager("test.btree", 1024);
			//tree = new BT(44, fm, ADN_LENGTH, 0);
		}

		TEST_METHOD_CLEANUP(BTreeCleanUp) {
			if (tree != nullptr) {
				tree->save();
				delete tree;
			}
			if (fm != nullptr)
				delete fm;
		}

		TEST_METHOD(BTreeImportantTest) {
			fm = new FileManager("btree-tests/BTreeImportantTest.ehash", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

			vector<ADN> inserted;
			//for i in {15,...,20}
			int size = 0;
			for (int i = 15; i <= 20; i++)
				size += pow(2, i);
			inserted.reserve(size);
			for (int i = 15; i <= 20; i++) {
				//generate 2^i ADN strings
				int count = pow(2, i);
				for (int j = 0; j < count; j++) {
					auto adn = ADN::generate_adn(ADN_LENGTH);
					//insert in the structure
					Assert::AreEqual(int(success), int(tree->insert(adn)));
					inserted.push_back(adn);
				}

				//select 2^i random insterted ADN strings and serach each one
				random_shuffle(inserted.begin(), inserted.end());
				for (int i = 0; i < 1000; i++)
					Assert::AreEqual(int(success), int(tree->search(inserted[i])));

				//generate 2^i random not inserted strings and search each one
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

		TEST_METHOD(BTreeRandomDataTest) {
			report_file.open("btree-tests/BTreeRandomDataTest.report", ios::trunc);
			fm = new FileManager("btree-tests/BTreeRandomDataTest.ehash", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

			int reads_opers = BT::reads_count, writes_opers = BT::writes_count;

			vector<ADN> inserted;
			inserted.reserve(DATA_SIZE);
			for (int i = 1; i <= DATA_SIZE; i++) {
				auto adn = ADN::generate_adn(ADN_LENGTH);

				START_TIMING(bt_insertion_timer);
				Assert::AreEqual(int(success), int(tree->insert(adn)));
				STOP_TIMING(bt_insertion_timer);
				INC_READS(bt_insertion_io_counter, BT::reads_count - reads_opers); reads_opers = BT::reads_count;
				INC_WRITES(bt_insertion_io_counter, BT::writes_count - writes_opers); writes_opers = BT::writes_count;

				inserted.push_back(adn);

				if (i == 64768 || i == 65536 || i == 131072 || i == 262144 || i == 524288 || i == DATA_SIZE) {
					int search1_reads = reads_opers;
					//select 1000 random insterted ADN strings and serach each one
					for (int j = 0; j < 1000; j++) {
						int r = rand() % i;

						START_TIMING(bt_searching_timer);
						Assert::AreEqual(int(success), int(tree->search(inserted[r])));
						STOP_TIMING(bt_searching_timer);
						INC_READS(bt_searching_io_counter, BT::reads_count - reads_opers); reads_opers = BT::reads_count;
						INC_WRITES(bt_searching_io_counter, BT::writes_count - writes_opers); writes_opers = BT::writes_count;
					}
					report_file << "i= " << i << ":" << endl;
					report_file << "good searches total reads:\t" << reads_opers - search1_reads << endl;
					report_file << "good searches avg reads:\t" << double(reads_opers - search1_reads) / 1000 << endl;
					report_file << endl;

					int search2_reads = reads_opers;
					//generate 1000 random not inserted strings and search each one
					for (int j = 0; j < 1000; j++) {
						auto not_adn = ADN::generate_adn(ADN_LENGTH);
						string value = not_adn.get_value();
						value[rand() % ADN_LENGTH] = 'B';
						not_adn.set_value(value);

						START_TIMING(bt_searching_timer);
						Assert::AreEqual(int(not_present), int(tree->search(not_adn)));
						STOP_TIMING(bt_searching_timer);
						INC_READS(bt_searching_io_counter, BT::reads_count - reads_opers); reads_opers = BT::reads_count;
						INC_WRITES(bt_searching_io_counter, BT::writes_count - writes_opers); writes_opers = BT::writes_count;
					}
					report_file << "bad searches total reads:\t" << reads_opers - search2_reads << endl;
					report_file << "bad searches avg reads:\t" << double(reads_opers - search2_reads) / 1000 << endl;
					report_file << endl;
				}
			}

			random_shuffle(inserted.begin(), inserted.end());

			report_file << "Deletions:" << endl;
			int deletion_reads = reads_opers, deletion_writes = writes_opers;
			for (int i = 1; i <= DATA_SIZE; i++) {
				START_TIMING(bt_deletion_timer);
				Assert::AreEqual(int(success), int(tree->remove(inserted[i - 1])), MSG("Element " << i << " failed removing"));
				STOP_TIMING(bt_deletion_timer);
				INC_READS(bt_deletion_io_counter, BT::reads_count - reads_opers); reads_opers = BT::reads_count;
				INC_WRITES(bt_deletion_io_counter, BT::writes_count - writes_opers); writes_opers = BT::writes_count;

				if (i == 64768 || i == 65536 || i == 131072 || i == 262144 || i == 524288 || i == DATA_SIZE) {
					report_file << "i= " << i << ":" << endl;
					report_file << "deletion total I/Os:\t" << (reads_opers - deletion_reads) + (writes_opers - deletion_writes) << endl;
					report_file << "deletion avg I/Os:\t" << double((reads_opers - deletion_reads) + (writes_opers - deletion_writes)) / i << endl;
					report_file << endl;
					deletion_reads = reads_opers;
				}
			}

			report_file << "TOTAL:" << endl;
			report_file << "insertion total timer:\t" << GET_TOTAL_TIME(bt_insertion_timer) << "s" << endl;
			report_file << "insertion avg timer:\t" << GET_AVERAGE_TIMING(bt_insertion_timer) << "s" << endl;
			report_file << "insertion total reads:\t" << GET_TOTAL_READS(bt_insertion_io_counter) << endl;
			report_file << "insertion avg reads:\t" << GET_READ_AVERAGE(bt_insertion_io_counter) << endl;
			report_file << endl;
			report_file << "searching total timer:\t" << GET_TOTAL_TIME(bt_searching_timer) << "s" << endl;
			report_file << "searching avg timer:\t" << GET_AVERAGE_TIMING(bt_searching_timer) << "s" << endl;
			report_file << "searching total reads:\t" << GET_TOTAL_READS(bt_searching_io_counter) << endl;
			report_file << "searching avg reads:\t" << GET_READ_AVERAGE(bt_searching_io_counter) << endl;
			report_file << endl;
			report_file << "deletion total timer:\t" << GET_TOTAL_TIME(bt_deletion_timer) << "s" << endl;
			report_file << "deletion avg timer:\t" << GET_AVERAGE_TIMING(bt_deletion_timer) << "s" << endl;
			report_file << "deletion total reads:\t" << GET_TOTAL_READS(bt_deletion_io_counter) << endl;
			report_file << "deletion avg reads:\t" << GET_READ_AVERAGE(bt_deletion_io_counter) << endl;
			report_file << endl;
			report_file << flush;

			Assert::AreEqual(0L, tree->count());
		}

		TEST_METHOD(BtreeSearchEmptyTest) {
			fm = new FileManager("btree-tests/BtreeSearchEmptyTest.btree", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(BtreeInsertEmptyAndSearchTest) {
			fm = new FileManager("btree-tests/BtreeInsertEmptyAndSearchTest.btree", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
			Assert::AreEqual(int(success), int(tree->insert(s)));
			Assert::AreEqual(int(success), int(tree->search(s)));
			Assert::AreEqual(int(not_present), int(tree->search(ADN(s.get_value() + "A"))));
		}

		TEST_METHOD(BtreeInsertAndSearchTest) {
			fm = new FileManager("btree-tests/BtreeInsertAndSearchTest.btree", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

			int count = 1000000;
			vector<ADN> data = generate_data(count);

			for (size_t i = 0; i < count; i++) {
				auto result = tree->insert(data[i]);
				Assert::AreEqual(int(success), int(result), MSG("Element " << i << " failed inserting"));
				result = tree->search(data[i]);
				Assert::AreEqual(int(success), int(result), MSG("Element " << i << " failed searching"));
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
			fm = new FileManager("btree-tests/BtreeInsertDulicatedSearchAndRemoveTest.btree", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

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

		TEST_METHOD(BtreeRemoveEmptyTest) {
			fm = new FileManager("btree-tests/BtreeRemoveEmptyTest.btree", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

			auto s = ADN::generate_adn(ADN_LENGTH);
			Assert::AreEqual(int(not_present), int(tree->search(s)));
		}

		TEST_METHOD(BtreeRemoveTest) {
			fm = new FileManager("btree-tests/BtreeRemoveTest.btree", 1024);
			tree = new BT(44, fm, ADN_LENGTH, 0);

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