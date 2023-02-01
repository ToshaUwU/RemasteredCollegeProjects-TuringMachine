#include "TuringMachine.hpp"

namespace TM
{
	bool TuringMachine::execute(std::string &error_description, size_t max_steps)
	{
		if (is_halted)
		{
			error_description = "Runtime error: execution is halted";
			return false;
		}

		if (!program.isValid())
		{
			error_description = "Runtime error: program is invalid";
			return false;
		}

		if (current_state.isNull())
			current_state = program.getInitialState();

		for (size_t i = 0; i < max_steps; i++)
		{
			char &current_symbol = tape.getCurrentSymbol();
			TuringProgram::Action action;
			if (!program.findStateAction(current_state, current_symbol, action))
			{
				std::string state_name = program.getStateName(current_state);
				error_description = "Runtime error: state named \"" + state_name + "\" doesn't have entry for symbol \'" + current_symbol + "\'";

				return false;
			}

			current_symbol = action.new_symbol;
			tape.moveHead(action.offset);
			current_state = action.new_state;
			if (current_state.isNull())
			{
				is_halted = true;
				return true;
			}
		}

		return true;
	}
}