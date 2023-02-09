#pragma once
#ifndef TM_ENDLESS_TAPE_INCLUDED
#define TM_ENDLESS_TAPE_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace TM
{
	class Tape
	{
		public:
			constexpr static size_t initial_size = 64;
			constexpr static size_t resize_policy = 3;

		private:
			std::vector<char> storage;

			size_t string_begin;
			size_t string_end;
			size_t current_symbol;

			int8_t last_move_offset;
			char current_symbol_initial_value;
			char empty_symbol;

			void resize();

		public:
			Tape(char default_symbol = '_', const std::string &initial_string = "", size_t initial_position = 0) { reset(default_symbol, initial_string, initial_position); }

			void reset(char default_symbol = '_', const std::string &initial_string = "", size_t initial_position = 0);

			void moveHead(int8_t offset);
			char & getCurrentSymbol() { return storage[current_symbol]; }
			char getCurrentSymbol() const { return storage[current_symbol]; }
			char & operator*() { return getCurrentSymbol(); }
			char operator*() const { return getCurrentSymbol(); }

			int8_t getLastOffset() const { return last_move_offset; }
			bool isCurrentSymbolChanged() const { return current_symbol_initial_value != getCurrentSymbol(); }
			char getDefaultSymbol() const { return empty_symbol; }
			const char * getString() const { return storage.data() + string_begin; }
			size_t size() const { return string_end - string_begin; }
			void trimRedundantSpaces();
	};
}

#endif // TM_ENDLESS_TAPE_INCLUDED