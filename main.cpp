#include <Tape.hpp>
#include <Program.hpp>
#include <TuringMachine.hpp>

#include <fstream>
#include <iostream>
#include <string>

constexpr static const char *HelloWorldSourceCode =
{
	"0 * H r 1\n"
	"1 * e r 2\n"
	"2 * l r 3\n"
	"3 * l r 4\n"
	"4 * o r 5\n"
	"5 * _ r 6\n"
	"6 * W r 7\n"
	"7 * o r 8\n"
	"8 * r r 9\n"
	"9 * l r 10\n"
	"10 * d r 11\n"
	"11 * ! r halt\n"
};

int main(int argc, char *argv[])
{
	std::string source_code = HelloWorldSourceCode;
	std::string begin_state_name = "0";
	char default_tape_symbol = '_';
	std::string tape_initial_data = "";
	size_t program_iteration_limit = 10000;

	switch (argc)
	{
		case 6:
			program_iteration_limit = std::stoull(argv[5]);
		case 5:
			tape_initial_data = argv[4];
		case 4:
			default_tape_symbol = argv[3][0];
		case 3:
			begin_state_name = argv[2];
		case 2:
		{
			std::ifstream program_source_code_file(argv[1]);
			if (!program_source_code_file.is_open())
			{
				std::cout << "Unable to load program source code" << std::endl;
				return -1;
			}

			std::cout << "Loaded program source code\n\n";

			source_code.clear();
			for (std::string file_input; !program_source_code_file.eof(); source_code += '\n')
			{
				std::getline(program_source_code_file, file_input);
				source_code += file_input;
			}
			program_source_code_file.close();
		}
	}

	TM::TuringProgram program;
	TM::ErrorInfo error_info;
	if (!program.compile(source_code, error_info, begin_state_name))
	{
		std::cout << error_info.description << std::endl;
		return -1;
	}

	std::cout << "Program compilation successful!\n\n";

	TM::Tape tape(default_tape_symbol, tape_initial_data);
	TM::TuringMachine turing_machine(program, tape);

	std::string error_description;
	if (!turing_machine.execute(error_description, program_iteration_limit))
		std::cout << error_description << std::endl;

	tape.trimRedundantSpaces();
	std::cout << "Result tape:\n";
	std::cout << tape.getString() << std::endl;

	return 0;
}