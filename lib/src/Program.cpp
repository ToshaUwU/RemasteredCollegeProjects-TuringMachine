#include "Program.hpp"

#include <cctype>
#include <set>

static constexpr char EndOfLine = '\n';
static constexpr char Comment = ';';
static constexpr char AnySymbol = '*';

static bool isSpace(char symbol) { return symbol == EndOfLine || std::isblank(static_cast<unsigned char>(symbol)); }

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

template<typename... Args>
static std::string formatString(const std::string &format, const Args&... args)
{
	int required_size = std::snprintf(nullptr, 0, format.c_str(), args...);
	size_t size = static_cast<size_t>(required_size) + 1;

	std::string output_string(size, '\0');
	std::snprintf(output_string.data(), size, format.c_str(), args...);

	return output_string;
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

	enum class TuringProgram::CompilationError
	{
		NoError,

		InvalidStateNameSymbol,
		InvalidKeySymbol,
		InvalidReplaceSymbol,
		InvalidDirection,

		StateHaveMultipleEntries,
		StateNameEqualToFinalStateName,
	};

	std::string TuringProgram::formatErrorMessage(CompilationError error, char current_symbol, const CompilationContext &context)
	{
		std::string error_message = formatString("Compilation error(%d, %d): ", context.line, context.column);
		switch (error)
		{
			case CompilationError::NoError:
				break;

			case CompilationError::InvalidStateNameSymbol:
				error_message += formatString("symbol \'%c\' is not allowed in state name", current_symbol);
				break;

			case CompilationError::InvalidKeySymbol:
				error_message += formatString("symbol \'%c\' is not allowed as state key", current_symbol);
				break;

			case CompilationError::InvalidReplaceSymbol:
				error_message += formatString("symbol \'%c\' is not allowed as replace value", current_symbol);
				break;

			case CompilationError::InvalidDirection:
				error_message += formatString("\'%c\' is not a valid direction", current_symbol);
				break;

			case CompilationError::StateHaveMultipleEntries:
				error_message += formatString("state \"%s\" have multiple entries for symbol \'%c\'", context.current_state_name.c_str(), current_symbol);
				break;

			case CompilationError::StateNameEqualToFinalStateName:
				error_message += formatString("invalid state, name \"%s\" is reserved for final state", context.current_state_name.c_str());
				break;
		}

		return error_message;
	}

	/*
	 */
	TuringProgram::CompilationError TuringProgram::findNextToken(char symbol, CompilationContext &context)
	{
		if (isSpace(symbol))
			return CompilationError::NoError;

		if (symbol == Comment)
		{
			context.currentParser = &TuringProgram::skipComment;
			return CompilationError::NoError;
		}

		context.currentParser = context.nextTokenParser;
		return (this->*context.nextTokenParser)(symbol, context);
	}

	TuringProgram::CompilationError TuringProgram::skipComment(char symbol, CompilationContext &context)
	{
		if (symbol != EndOfLine)
			return CompilationError::NoError;

		context.currentParser = &TuringProgram::findNextToken;
		return CompilationError::NoError;
	}

	TuringProgram::CompilationError TuringProgram::parseStateName(char symbol, CompilationContext &context)
	{
		context.processing_state = true;
		if (isAllowedTokenSymbol(symbol))
		{
			if (isAllowedStateNameSymbol(symbol))
			{
				context.current_state_name += symbol;
				return CompilationError::NoError;
			}

			return CompilationError::InvalidStateNameSymbol;
		}

		const std::string &state_name = context.current_state_name;
		if (isHaltState(state_name))
			return CompilationError::StateNameEqualToFinalStateName;

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

		return CompilationError::NoError;
	}

	TuringProgram::CompilationError TuringProgram::parseKeySymbol(char symbol, CompilationContext &context)
	{
		if (!isAllowedTokenSymbol(symbol))
			return CompilationError::InvalidKeySymbol;

		const State &current_state = states[context.current_state_id];

		bool have_multiple_entries;
		if (symbol != AnySymbol)
		{
			auto it = current_state.actions.find(symbol);
			have_multiple_entries = (it != current_state.actions.end());
		}
		else
			have_multiple_entries = current_state.have_default_action;

		if (have_multiple_entries)
			return CompilationError::StateHaveMultipleEntries;

		context.current_state_key = symbol;
		context.nextTokenParser = &TuringProgram::parseReplaceSymbol;
		context.currentParser = &TuringProgram::findNextToken;

		return CompilationError::NoError;
	}

	TuringProgram::CompilationError TuringProgram::parseReplaceSymbol(char symbol, CompilationContext &context)
	{
		if (!isAllowedTokenSymbol(symbol))
			return CompilationError::InvalidReplaceSymbol;

		context.current_state_action.new_symbol = symbol;
		context.current_state_action.replace_symbol = (symbol != AnySymbol);
		context.nextTokenParser = &TuringProgram::parseDirection;
		context.currentParser = &TuringProgram::findNextToken;

		return CompilationError::NoError;
	}

	TuringProgram::CompilationError TuringProgram::parseDirection(char symbol, CompilationContext &context)
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
				return CompilationError::InvalidDirection;
		}

		context.nextTokenParser = &TuringProgram::parseNextStateName;
		context.currentParser = &TuringProgram::findNextToken;

		return CompilationError::NoError;
	}

	TuringProgram::CompilationError TuringProgram::parseNextStateName(char symbol, CompilationContext &context)
	{
		if (isAllowedTokenSymbol(symbol))
		{
			if (isAllowedStateNameSymbol(symbol))
			{
				context.next_state_name += symbol;
				return CompilationError::NoError;
			}

			return CompilationError::InvalidStateNameSymbol;
		}

		const std::string &state_name = context.next_state_name;

		context.current_state_action.is_final_state = isHaltState(state_name);
		if (!context.current_state_action.is_final_state)
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

		State &current_state = states[context.current_state_id];
		if (context.current_state_key == AnySymbol)
		{
			current_state.default_action = context.current_state_action;
			current_state.have_default_action = true;
		}
		else
			current_state.actions.insert({ context.current_state_key, context.current_state_action });

		context.current_state_name.clear();
		context.next_state_name.clear();
		context.processing_state = false;
		context.nextTokenParser = &TuringProgram::parseStateName;
		context.currentParser = &TuringProgram::findNextToken;

		return CompilationError::NoError;
	}

	/*
	 */
	bool TuringProgram::compile(const std::string &source_code, ErrorInfo &error_info, const std::string &initial_state_name)
	{
		error_info.description = "";
		error_info.line = 0;
		error_info.column = 0;

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

		states.push_back({initial_state_name, {}});
		context.states_references.insert({initial_state_name, {0}});

		size_t line = 1;
		size_t column = 1;
		for (char symbol : source_code)
		{
			context.line = line;
			context.column = column;
			CompilationError error = (this->*context.currentParser)(symbol, context);

			if (error != CompilationError::NoError)
			{
				error_info.description = formatErrorMessage(error, symbol, context);
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

		if (context.processing_state)
		{
			error_info.description = "Compilation error: unexpected end-of-file, state definition is incomplete";
			return false;
		}

		const std::set<std::string> &defined_states = context.defined_states;
		for (const auto & [state_name, reference_info] : context.states_references)
		{
			const std::string &undefined_state_name = states[reference_info.id].name;

			auto it = defined_states.find(undefined_state_name);
			if (it != defined_states.end())
				continue;

			if (undefined_state_name != initial_state_name)
			{
				const std::string &parent_state_name = states[reference_info.parent_state_id].name;

				error_info.description = formatString
				(
					"Compilation error: state named \"%s\" is undefined (first reference by state \"%s\", line %d, column %d)",
						undefined_state_name.c_str(),
						parent_state_name.c_str(),
						reference_info.line,
						reference_info.column
				);
				error_info.line = reference_info.line;
				error_info.column = reference_info.column;
			}
			else
			{
				error_info.description = formatString("Compilation error: initial state is undefined (should have name \"%s\")", initial_state_name.c_str());
				error_info.line = 0;
				error_info.column = 0;
			}

			clear();
			return false;
		}

		return true;
	}
}