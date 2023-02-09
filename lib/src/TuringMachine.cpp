#include "TuringMachine.hpp"

namespace TM
{
	bool TuringMachine::execute(std::string &error_description, size_t iterations_limit, bool error_on_iterations_limit_exceed)
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

		for (size_t i = 0; i < iterations_limit; i++)
		{
			char &current_symbol = tape.getCurrentSymbol();

			TuringProgram::Action action;
			if (!program.findStateAction(current_state, current_symbol, action))
			{
				std::string state_name = program.getStateName(current_state);
				error_description = "Runtime error: state named \"" + state_name + "\" doesn't have entry for symbol \'" + current_symbol + "\'";

				return false;
			}

			if (action.replace_symbol)
				current_symbol = action.new_symbol;

			tape.moveHead(action.offset);
			current_state = action.new_state;
			if (action.is_final_state)
			{
				is_halted = true;
				return true;
			}
		}

		if (error_on_iterations_limit_exceed)
		{
			error_description = "Runtime error: exceed maximum iterations limit (set to " + std::to_string(iterations_limit) + ")";
			return false;
		}

		return true;
	}
}