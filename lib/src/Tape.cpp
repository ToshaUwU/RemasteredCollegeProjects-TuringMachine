#include "Tape.hpp"

#include <algorithm>
#include <cstring>

namespace TM
{
	Tape & Tape::copy(const Tape &tape)
	{
		if (storage == tape.storage)
			return *this;

		storage_size = tape.storage_size;
		storage = new char[storage_size + 1];
		std::memcpy(storage, tape.storage, storage_size);
		storage[storage_size] = 0;

		string_begin = tape.string_begin;
		string_end = tape.string_end;
		current_symbol = tape.current_symbol;

		last_move_offset = tape.last_move_offset;
		current_symbol_initial_value = tape.current_symbol_initial_value;

		return *this;
	}

	Tape & Tape::extract(Tape &&tape)
	{
		if (storage == tape.storage)
			return *this;

		storage = tape.storage;
		tape.storage = nullptr;
		storage_size = tape.storage_size;

		string_begin = tape.string_begin;
		string_end = tape.string_end;
		current_symbol = tape.current_symbol;

		last_move_offset = tape.last_move_offset;
		current_symbol_initial_value = tape.current_symbol_initial_value;

		tape.reset();

		return *this;
	}

	void Tape::resize()
	{
		size_t new_size = storage_size*resize_policy;
		char *new_storage = new char[new_size + 1];
		std::memset(new_storage, '_', new_size);
		std::memcpy(new_storage + storage_size, storage, storage_size);

		string_begin += storage_size;
		string_end += storage_size;
		new_storage[string_end] = '\0';
		current_symbol += storage_size - 1;

		delete [] storage;
		storage = new_storage;
		storage_size = new_size;
	}

	/*
	 */
	Tape::~Tape()
	{
		delete [] storage;
		storage_size = 0;

		string_begin = 0;
		string_end = 0;
		current_symbol = 0;

		last_move_offset = 0;
		current_symbol_initial_value = '\0';
	}

	void Tape::reset(const std::string &initial_string, size_t initial_position)
	{
		delete [] storage;

		size_t block_size = std::max(initial_size, initial_string.size());
		storage_size = block_size*resize_policy;
		storage = new char[storage_size + 1];

		std::memset(storage, '_', storage_size);
		std::replace_copy(initial_string.begin(), initial_string.end(), storage + block_size, ' ', '_');

		string_begin = block_size;
		string_end = string_begin + initial_string.size();
		storage[string_end] = '\0';

		current_symbol = string_begin + std::min(initial_position, initial_string.size());

		last_move_offset = 0;
		current_symbol_initial_value = storage[current_symbol];
	}

	void Tape::moveHead(int8_t offset)
	{
		while (storage_size <= current_symbol + offset)
			resize();

		current_symbol += offset;
		string_begin = std::min(string_begin, current_symbol);
		if (current_symbol >= string_end)
		{
			storage[string_end] = '_';
			storage[current_symbol + 1] = '\0';
			string_end = current_symbol + 1;
		}

		last_move_offset = offset;
		current_symbol_initial_value = storage[current_symbol];
	}

	void Tape::trimRedundantSpaces()
	{
		while (string_begin < current_symbol && storage[string_begin] == '_')
			string_begin++;

		storage[string_end--] = '_';
		while (string_end > current_symbol && storage[string_end] == '_')
			string_end--;
		storage[++string_end] = '\0';
	}
}