#include "Tape.hpp"

#include <algorithm>
#include <cstring>

namespace TM
{
	void Tape::resize()
	{
		size_t storage_size = storage.size() - 1;
		size_t new_size = storage_size*resize_policy;
		std::vector<char> new_storage(new_size + 1, empty_symbol);

		std::memcpy(new_storage.data() + storage_size, storage.data(), storage_size);
		storage = std::move(new_storage);

		string_begin += storage_size;
		string_end += storage_size;
		storage[string_end] = '\0';
		current_symbol += storage_size - 1;
	}

	/*
	 */
	void Tape::reset(char default_symbol, const std::string &initial_string, size_t initial_position)
	{
		size_t block_size = std::max(initial_size, initial_string.size());
		size_t storage_required_size = block_size*resize_policy;
		if (storage.size() <= storage_required_size || empty_symbol != default_symbol)
		{
			std::vector<char> new_storage(storage_required_size + 1, default_symbol);
			storage = std::move(new_storage);
			empty_symbol = default_symbol;
		}

		std::memcpy(storage.data() + block_size, initial_string.c_str(), initial_string.size());

		string_begin = block_size;
		string_end = string_begin + initial_string.size() + 1;
		storage[string_end] = '\0';

		current_symbol = string_begin + std::min(initial_position, initial_string.size());

		last_move_offset = 0;
		current_symbol_initial_value = storage[current_symbol];
	}

	void Tape::moveHead(int8_t offset)
	{
		while (storage.size() - 1 <= current_symbol + offset)
			resize();

		current_symbol += offset;
		string_begin = std::min(string_begin, current_symbol);
		if (current_symbol >= string_end)
		{
			storage[string_end] = empty_symbol;
			storage[current_symbol + 1] = '\0';
			string_end = current_symbol + 1;
		}

		last_move_offset = offset;
		current_symbol_initial_value = storage[current_symbol];
	}

	void Tape::trimRedundantSpaces()
	{
		while (string_begin < current_symbol && storage[string_begin] == empty_symbol)
			string_begin++;

		storage[string_end] = empty_symbol;
		while (string_end > string_begin && storage[string_end - 1] == empty_symbol)
			string_end--;

		storage[string_end] = '\0';
	}
}