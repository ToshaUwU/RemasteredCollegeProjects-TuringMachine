#pragma once
#ifndef TM_ENDLESS_TAPE_INCLUDED
#define TM_ENDLESS_TAPE_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

namespace TM
{
	class Tape
	{
		public:
			constexpr static size_t initial_size = 64;
			constexpr static size_t resize_policy = 3;

		private:
			char *storage;
			size_t storage_size;

			size_t string_begin;
			size_t string_end;
			size_t current_symbol;

			int8_t last_move_offset;
			char current_symbol_initial_value;

			Tape & copy(const Tape &tape);
			Tape & extract(Tape &&tape);
			void resize();

		public:
			Tape(const Tape &tape) { copy(tape); }
			Tape(Tape &&tape) { extract(std::move(tape)); }
			Tape(const std::string &initial_string = "", size_t initial_position = 0) : storage(nullptr) { reset(initial_string, initial_position); }
			~Tape();

			Tape & operator=(const Tape &tape) { return copy(tape); }
			Tape & operator=(Tape &&tape) { return extract(std::move(tape)); }

			void reset(const std::string &initial_string = "", size_t initial_position = 0);

			void moveHead(int8_t offset);
			char & getCurrentSymbol() { return storage[current_symbol]; }
			char getCurrentSymbol() const { return storage[current_symbol]; }
			char & operator*() { return getCurrentSymbol(); }
			char operator*() const { return getCurrentSymbol(); }

			int8_t getLastOffset() const { return last_move_offset; }
			bool isCurrentSymbolChanged() const { return current_symbol_initial_value != getCurrentSymbol(); }
			const char * getString() const { return storage + string_begin; }
			size_t size() const { return string_end - string_begin; }
			void trimRedundantSpaces();
	};
}

#endif // TM_ENDLESS_TAPE_INCLUDED