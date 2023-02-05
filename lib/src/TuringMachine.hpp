#ifndef TM_TURING_MACHINE_INCLUDED
#define TM_TURING_MACHINE_INCLUDED

#include <Tape.hpp>
#include <Program.hpp>

#include <string>

namespace TM
{
	class TuringMachine
	{
		private:
			const TuringProgram &program;
			Tape &tape;

			StateHandle current_state;
			bool is_halted;

		public:
			TuringMachine(const TuringMachine &) = delete;
			TuringMachine(TuringMachine &&) = delete;
			TuringMachine & operator=(const TuringMachine &) = delete;
			TuringMachine & operator=(TuringMachine &&) = delete;

			TuringMachine(const TuringProgram &program, Tape &tape) :
				program(program),
				tape(tape),
				current_state(program.getInitialState()),
				is_halted(false)
			{}

			void resetState(bool clear_tape = true)
			{
				if (clear_tape) tape.reset();
				current_state = program.getInitialState();
				is_halted = false;
			}

			bool execute(std::string &error_description, size_t max_steps = 10000);
			bool isHalted() const { return is_halted; }
	};
}

#endif // TM_TURING_MACHINE_INCLUDED