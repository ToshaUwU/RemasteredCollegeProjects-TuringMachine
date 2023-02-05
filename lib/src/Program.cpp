#include "Program.hpp"

#include <cctype>
#include <set>

static constexpr char EndOfLine = '\n';
static constexpr char Comment = ';';

static bool isSpace(char symbol) { return std::isblank(static_cast<unsigned char>(symbol)); }

static bool isAllowedStateNameSymbol(char symbol) { return symbol == '_' || symbol == '-' || std::isalnum(static_cast<unsigned char>(symbol)); }
static bool isAllowedTokenSymbol(char symbol) { return symbol != Comment && std::isgraph(static_cast<unsigned char>(symbol)); }
static bool isHaltState(const std::string &state_name)
{
	return
		(state_name[0] == 'H' || state_name[0] == 'h') &&
		(state_name[1] == 'A' || state_name[1] == 'a') &&
		(state_name[2] == 'L' || state_name[2] == 'l') &&
		(state_name[3] == 'T' || state_name[3] == 't') &&
		(state_name.size() == 4);
}

namespace TM
{
	struct StateReference
	{
		size_t id;
		size_t parent_state_id = 0;

		size_t line = 0;
		size_t column = 0;
	};

	struct TuringProgram::CompilationContext
	{
		std::string current_state_name;
		char current_state_key;
		Action current_state_action;
		size_t current_state_id;
		std::string next_state_name;
		bool processing_state;

		std::unordered_map<std::string, StateReference> states_references;
		std::set<std::string> defined_states;

		size_t line;
		size_t column;

		ParserFunctionPtr nextTokenParser;
		ParserFunctionPtr currentParser;
	};

	/*
	 */
	bool TuringProgram::findNextToken(char symbol, std::string &error_description, CompilationContext &context)
	{
		if (isSpace(symbol))
			return true;

		if (symbol == EndOfLine || symbol == Comment)
		{
			if (context.processing_state)
			{
				error_description = "Compilation error: unexpected end-of-line at line " + std::to_string(context.line) + ", state is incomplete";
				return false;
			}

			if (symbol == Comment)
				context.currentParser = &TuringProgram::skipComment;

			return true;
		}

		context.currentParser = context.nextTokenParser;
		return (this->*context.nextTokenParser)(symbol, error_description, context);
	}

	bool TuringProgram::skipComment(char symbol, [[maybe_unused]] std::string &error_description, CompilationContext &context)
	{
		if (symbol != EndOfLine)
			return true;

		context.currentParser = &TuringProgram::findNextToken;
		return true;
	}

	bool TuringProgram::parseStateName(char symbol, std::string &error_description, CompilationContext &context)
	{
		context.processing_state = true;
		if (isAllowedTokenSymbol(symbol))
		{
			if (isAllowedStateNameSymbol(symbol))
			{
				context.current_state_name += symbol;
				return true;
			}

			error_description =
				std::string("Compilation error: symbol \'") + symbol +
				"\' (line " + std::to_string(context.line) + ", column " + std::to_string(context.column) +
				") is not allowed in state name";

			return false;
		}

		const std::string &state_name = context.current_state_name;
		if (isHaltState(state_name))
		{
			error_description =
				"Compilation error: invalid state name: \"" + state_name +
				"\"(line " + std::to_string(context.line) + ", column " + std::to_string(context.column - 4) +
				"), this name is reserved for final state";

			return false;
		}

		context.defined_states.insert(state_name);

		auto it = context.states_references.find(state_name);
		if (it == context.states_references.end())
		{
			states.push_back({ state_name, {} });

			size_t current_state_id = states.size() - 1;
			context.states_references.insert({ state_name, { current_state_id } });
			context.current_state_id = current_state_id;
		}
		else
			context.current_state_id = it->second.id;

		context.nextTokenParser = &TuringProgram::parseKeySymbol;
		context.currentParser = &TuringProgram::findNextToken;

		return true;
	}

	bool TuringProgram::parseKeySymbol(char symbol, std::string &error_description, CompilationContext &context)
	{
		if (!isAllowedTokenSymbol(symbol))
		{
			error_description =
				std::string("Compilation error: symbol \'") + symbol +
				"\' (line " + std::to_string(context.line) + ", column " + std::to_string(context.column) +
				") is not allowed as state key";

			return false;
		}

		const State &current_state = states[context.current_state_id];

		auto it = current_state.actions.find(symbol);
		if (it != current_state.actions.end())
		{
			error_description = "Compilation error: state \"" + context.current_state_name + "\" have multiple entries for symbol \'" + symbol + "\'";
			return false;
		}

		context.current_state_key = symbol;
		context.nextTokenParser = &TuringProgram::parseReplaceSymbol;
		context.currentParser = &TuringProgram::findNextToken;

		return true;
	}

	bool TuringProgram::parseReplaceSymbol(char symbol, std::string &error_description, CompilationContext &context)
	{
		if (!isAllowedTokenSymbol(symbol))
		{
			error_description =
				std::string("Compilation error: symbol \'") + symbol +
				"\' (line " + std::to_string(context.line) + ", column " + std::to_string(context.column) +
				") is not allowed as replace value";

			return false;
		}

		context.current_state_action.new_symbol = symbol;
		context.nextTokenParser = &TuringProgram::parseDirection;
		context.currentParser = &TuringProgram::findNextToken;

		return true;
	}

	bool TuringProgram::parseDirection(char symbol, std::string &error_description, CompilationContext &context)
	{
		switch (symbol)
		{
			case '*':
			case 's':
			case 'S':
			case '0':
				context.current_state_action.offset = 0;
				break;

			case 'r':
			case 'R':
			case '+':
				context.current_state_action.offset = 1;
				break;

			case 'l':
			case 'L':
			case '-':
				context.current_state_action.offset = -1;
				break;

			default:
				error_description = "Compilation error: invalid direction (line " + std::to_string(context.line) + ", column " + std::to_string(context.column) + ")";
				return false;
		}

		context.nextTokenParser = &TuringProgram::parseNextStateName;
		context.currentParser = &TuringProgram::findNextToken;

		return true;
	}

	bool TuringProgram::parseNextStateName(char symbol, std::string &error_description, CompilationContext &context)
	{
		if (isAllowedTokenSymbol(symbol))
		{
			if (isAllowedStateNameSymbol(symbol))
			{
				context.next_state_name += symbol;
				return true;
			}

			error_description =
				std::string("Compilation error: symbol \'") + symbol +
				"\' (line " + std::to_string(context.line) + ", column " + std::to_string(context.column) +
				") is not allowed in state name";

			return false;
		}

		const std::string &state_name = context.next_state_name;

		if (!isHaltState(state_name))
		{
			auto it = context.states_references.find(state_name);
			if (it == context.states_references.end())
			{
				states.push_back({ state_name, {} });

				size_t next_state_id = states.size() - 1;
				context.states_references.insert({ state_name, { next_state_id, context.current_state_id, context.line, context.column - state_name.size() } });
				context.current_state_action.new_state = StateHandle(next_state_id, program_id);
			}
			else
				context.current_state_action.new_state = StateHandle(it->second.id, program_id);
		}
		else
			context.current_state_action.new_state = StateHandle();

		states[context.current_state_id].actions.insert({ context.current_state_key, context.current_state_action });

		context.current_state_name.clear();
		context.next_state_name.clear();
		context.processing_state = false;
		context.nextTokenParser = &TuringProgram::parseStateName;
		context.currentParser = &TuringProgram::findNextToken;

		return true;
	}

	/*
	 */
	bool TuringProgram::compile(const std::string &source_code, ErrorInfo &error_info)
	{
		if (source_code.empty())
		{
			error_info.description = "Compilation error: no source code provided";
			return false;
		}

		clear();
		program_id = generateProgramID();

		CompilationContext context;
		context.nextTokenParser = &TuringProgram::parseStateName;
		context.currentParser = &TuringProgram::findNextToken;
		context.processing_state = false;

		states.push_back({"0", {}});
		context.states_references.insert({"0", {0}});

		size_t line = 1;
		size_t column = 1;
		for (char symbol : source_code)
		{
			context.line = line;
			context.column = column;
			bool result = (this->*context.currentParser)(symbol, error_info.description, context);

			if (!result)
			{
				error_info.line = line;
				error_info.column = column;

				clear();
				return false;
			}

			if (symbol == EndOfLine)
			{
				line++;
				column = 1;
			}
			else
				column++;
		}

		const std::set<std::string> &defined_states = context.defined_states;
		for (const auto & [state_name, reference_info] : context.states_references)
		{
			const std::string &undefined_state_name = states[reference_info.id].name;

			auto it = defined_states.find(undefined_state_name);
			if (it != defined_states.end())
				continue;

			if (undefined_state_name != "0")
			{
				const std::string &parent_state_name = states[reference_info.parent_state_id].name;

				error_info.description = "Compilation error: state, named \"" + undefined_state_name + "\" is undefined, but referenced (first reference by state \"" +
				parent_state_name + "\", line " + std::to_string(reference_info.line) + ", column " + std::to_string(reference_info.column) + ")";
				error_info.line = reference_info.line;
				error_info.column = reference_info.column;
			}
			else
			{
				error_info.description = "Compilation error: initial state is undefined (should have name \"0\")";
				error_info.line = 0;
				error_info.column = 0;
			}

			clear();
			return false;
		}

		return true;
	}
}