#ifndef TM_TURING_PROGRAM_INCLUDED
#define TM_TURING_PROGRAM_INCLUDED

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace TM
{
	struct ErrorInfo
	{
		std::string description = "";
		size_t line = 0;
		size_t column = 0;
	};

	class TuringProgram;
	class StateHandle
	{
		private:
			size_t index;
			size_t program_id;

			StateHandle(size_t index, size_t program_id) : index(index), program_id(program_id) {}
			operator size_t() const { return index; }

			friend TuringProgram;

		public:
			StateHandle() : index(static_cast<size_t>(-1)), program_id(0) {}
			bool isNull() const { return index == static_cast<size_t>(-1) || program_id == 0; }
	};

	/*
	 */
	class TuringProgram
	{
		public:
			struct Action
			{
				char new_symbol;
				int8_t offset;
				StateHandle new_state;
			};

		private:
			struct State
			{
				std::string name;
				std::unordered_map<char, Action> actions;
			};

			std::vector<State> states;
			size_t program_id;

			static size_t generateProgramID()
			{
				static size_t free_id = 1;
				return free_id++;
			}

			class Symbol;
			struct CompilationContext;
			using ParserFunctionType = bool (char symbol, std::string &error_description, CompilationContext &context);
			using ParserFunctionPtr = TuringProgram::ParserFunctionType TuringProgram::*;

			ParserFunctionType findNextToken;
			ParserFunctionType skipComment;

			ParserFunctionType parseStateName;
			ParserFunctionType parseKeySymbol;
			ParserFunctionType parseReplaceSymbol;
			ParserFunctionType parseDirection;
			ParserFunctionType parseNextStateName;

		public:
			TuringProgram() : program_id(0) {}
			~TuringProgram() = default;

			bool compile(const std::string &source_code, ErrorInfo &error_info);

			bool isValid() const { return program_id != 0; }
			void clear() { states.clear(), program_id = 0; }

			StateHandle getInitialState() const { return isValid() ? StateHandle(0, program_id) : StateHandle(); }
			std::string getStateName(StateHandle state_handle) const { return isValid() ? states[state_handle].name : ""; }
			bool findStateAction(StateHandle state_handle, char symbol, Action &output_action) const
			{
				if (!isValid() || state_handle.isNull() || state_handle.program_id != program_id)
					return false;

				const State &state = states[state_handle];

				auto it = state.actions.find(symbol);
				if (it == state.actions.end())
				{
					it = state.actions.find('*');
					if (it == state.actions.end())
						return false;
				}

				output_action = it->second;
				return true;
			}
	};
}

#endif // TM_TURING_PROGRAM_INCLUDED