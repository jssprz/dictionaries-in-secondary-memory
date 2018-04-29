#pragma once

#include <fstream>

using namespace std;

namespace btree {
	class FileManager
	{
	public:
		FileManager(string path, long blockSize) {
			this->blockSize = blockSize;
			this->fileStream = fstream(path);
			this->load();
		}
		~FileManager() {

		}

		// path of the file
		//string get_path() {
		//	return this->writer; 
		//}

		// get pointr to the beginning of the dynamic memory
		long get_heap_start() {
			return hs;
		}

		// set pointr to the beginning of the dynamic memory
		void set_heap_start(long value) {
			this->hs = value;
		}
		
		// get pointer to te end of the dynamic memory
		long get_heap_end() {
			return he;
		}

		// set pointer to te end of the dynamic memory
		void set_heap_end(long value) {
			he = value;
		}
		
		// set the size of the file's block
		void set_block_size(int value) {
			blockSize = value;
		}

		// get the stram
		fstream& get_file_stram() {
			return this->fileStream;
		}

		// in charge of reserving memory
		// Returns a long where memory is free
		long alloc() {
			if (freeMemory == 0) {
				long tmp = he;
				he += blockSize;
				return tmp; //is searched at the end of the file
			}
			else {
				long tmp = freeMemory;
				fileStream.seekp(freeMemory);

				//the next free memory is read
				char buffer[8];
				fileStream.read(buffer, 8);
				unsigned long next_free = *((unsigned long*)buffer);

				freeMemory = next_free;//the next free memory is saved

				return tmp;//returns the free memory
			}
		}

		// Responsible for freeing a space in memory.
		// memory: space in memory to free
		void free(long memory) {
			long tmp = freeMemory;
			freeMemory = memory;

			fileStream.seekp(memory);
			fileStream.write((char*)tmp, 8);
			fileStream.flush();
		}

		// In charge of saving all the necessary information.
		void save() {
			char* buffer = (char*)freeMemory;

			//write the first free space of memory
			fileStream.seekp(0);
			fileStream.write(buffer, 8);

			//write the biginning of dynamic memory
			buffer = (char*)hs;
			fileStream.write(buffer, 8);

			//wrte the end of the dynamic memory
			buffer = (char*)he;
			fileStream.write(buffer, 8);

			fileStream.flush();//write in the file
			fileStream.close();
		}

		// In charge of reading all the necessary information.
		void load()
		{
			char buffer[8];

			fileStream.seekg(0);
			fileStream.read(buffer, 8);
			unsigned long freeMemory = *((unsigned long*)buffer);

			fileStream.read(buffer, 8);
			unsigned long hs = *((unsigned long*)buffer);

			fileStream.read(buffer, 8);
			unsigned long he = *((unsigned long*)buffer);
		}

	private:
		// pointer to the fist freespace in memory, if equal to 0 must go to the end of the file
		long freeMemory;
		// HeapStart pointer to the beguining of the dinamic information (the BTreeNode)
		long hs;
		// HeapEnd pointer to the end of the dinamic memory
		long he;
		// size of the file's block
		int blockSize;
		// underlaying out stream
		fstream fileStream;
	};
}