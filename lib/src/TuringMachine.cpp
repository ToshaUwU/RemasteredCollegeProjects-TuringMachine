#include "TuringMachine.hpp"

#include <EndlessTape.hpp>

size_t Program::WordLen(const char * String)
{
	size_t i = 0;
	while(String[i] != ' ')i++;
	return i;
}

bool Program::WordCmp(const char * String1, const char * String2)
{
	if(WordLen(String1) != WordLen(String2))
		return false;
	for(size_t i = 0; String1[i] != ' '; i++)
		if(String1[i] != String2[i])
			return false;
	return true;
}

Program::~Program()
{
	if(ProgramData)
		for(uint16_t i = 0; i < StatesCount; i++)
			delete [] ProgramData[i];

	if(StatesNames)
		for(uint16_t i = 0; i < StatesCount; i++)
			delete [] StatesNames[i];

	delete [] ProgramData;
	delete [] StatesNames;
	delete [] StatesEntriesCount;

	delete [] ErrorString;
}

static size_t itos(uint32_t Num, char * Str)
{
	uint64_t NumCpy = Num, Numl = 1;
	while(NumCpy /= 10)
		Numl++;
	Str[Numl--] = '\0';
	do
		Str[Numl--] = Num % 10 + 48;
	while(Num /= 10);
	return strlen(Str);
}

bool Program::InitProgram(char ** ProgramString, size_t LinesCount)
{
	size_t LineBufferSize = 16;
	this->~Program();
	CurrentState = 0;
	StatesCount = 1;
	ProgramIsValid = false;
	Halted = false;

	size_t * LinesNumbers = new size_t[LinesCount];
	for(size_t i = 0; i < LinesCount; i++)
		LinesNumbers[i] = i + 1;

	char * StringTemp;
	for(size_t i = 0, j, StringLength; i < LinesCount; i++)
	{
		j = 0;
		while(ProgramString[i][j] == ' ' || ProgramString[i][j] == '\t')
			j++;

		if(j > 0)
		{
			StringLength = strlen(ProgramString[i] + j) + 1;
			StringTemp = new char[StringLength];

			strncpy(StringTemp, ProgramString[i] + j, StringLength);
			strncpy(ProgramString[i], StringTemp, StringLength);

			delete [] StringTemp;
		}

		if(ProgramString[i][0] == ';' || ProgramString[i][0] == '\n')
			ProgramString[i][0] = '\0';
	}

	Sort(ProgramString, LinesCount, LinesNumbers);

	size_t EmptyStringsShift = 0;
	while(ProgramString[EmptyStringsShift][0] == '\0')
		EmptyStringsShift++;

	int64_t BadSyntax = -1;
	for(size_t i = EmptyStringsShift, j, k; i < LinesCount; i++)
	{
		j = 0;
		while(ProgramString[i][j] != '\n' && ProgramString[i][j] != ' ' && ProgramString[i][j] != ';' && ProgramString[i][j] != '\t' && ProgramString[i][j] != '\0')
			j++;

		if(ProgramString[i][j] != ' ')
		{
			BadSyntax = LinesNumbers[i];
			break;
		}

		k = 1;
		while(k < 7)
		{
			if(ProgramString[i][j+k] == '\n' || ProgramString[i][j+k] == ' ' || ProgramString[i][j+k] == ';' || ProgramString[i][j+k] == '\t' || ProgramString[i][j+k] == '\0')
			{
				BadSyntax = LinesNumbers[i];
				break;
			}
			k++;
			if(ProgramString[i][j+k] != ' ' && ProgramString[i][j+k] != '\t')
			{
				BadSyntax = LinesNumbers[i];
				break;
			}
			k++;
		}

		if(BadSyntax > 0)break;

		j += 7;
		if(ProgramString[i][j] == '\n' || ProgramString[i][j] == ' ' || ProgramString[i][j] == ';' || ProgramString[i][j] == '\t' || ProgramString[i][j] == '\0')
		{
			BadSyntax = LinesNumbers[i];
			break;
		}

		while(ProgramString[i][j] != '\n' && ProgramString[i][j] != ' ' && ProgramString[i][j] != ';' && ProgramString[i][j] != '\t' && ProgramString[i][j] != '\0')
			j++;

		k = j;
		while(ProgramString[i][j] != '\n' && ProgramString[i][j] != ';' && ProgramString[i][j] != '\0')
			if(ProgramString[i][j] != ' ' && ProgramString[i][j] != '\t')
			{
				const char * ErrorStr1 = "Unexpected symbol(";
				const char * ErrorStr2 = ") on line ";
				const char * ErrorStr3 = ".";

				char LineNumBuffer[LineBufferSize];
				ErrorString = new char[strlen(ErrorStr1) + 1 + strlen(ErrorStr2) + itos(LinesNumbers[i], LineNumBuffer) + strlen(ErrorStr3) + 1]{};

				strcpy(ErrorString, ErrorStr1);

				size_t ErrorStringShift = strlen(ErrorStr1);
				strncpy(ErrorString + ErrorStringShift, ProgramString[i] + j, 1);

				ErrorStringShift++;
				strcpy(ErrorString + ErrorStringShift, ErrorStr2);

				ErrorStringShift += strlen(ErrorStr2);
				strcpy(ErrorString + ErrorStringShift, LineNumBuffer);

				ErrorStringShift += strlen(LineNumBuffer);
				strcpy(ErrorString + ErrorStringShift, ErrorStr3);

				delete [] LinesNumbers;

				return ERROR;
			}
			else
				j++;

		ProgramString[i][k] = '\0';
	}

	if(BadSyntax > 0)
	{
		const char * ErrorStr1 = "Command on line ";
		const char * ErrorStr2 = " not correspond to Turing machine command syntax.";

		char LineNumBuffer[LineBufferSize];
		ErrorString = new char[strlen(ErrorStr1) + itos(BadSyntax, LineNumBuffer) + strlen(ErrorStr2) + 1]{};

		strcpy(ErrorString, ErrorStr1);

		size_t ErrorStringShift = strlen(ErrorStr1);
		strcpy(ErrorString + ErrorStringShift, LineNumBuffer);

		ErrorStringShift += strlen(LineNumBuffer);
		strcpy(ErrorString + ErrorStringShift, ErrorStr2);

		delete [] LinesNumbers;

		return ERROR;
	}

	if(strncmp(ProgramString[EmptyStringsShift], "0 ", 2))
	{
		bool StartStateNotFounded = true;
		for(size_t i = EmptyStringsShift + 1; i < LinesCount; i++)
			if(!strncmp(ProgramString[i], "0 ", 2))
			{
				StartStateNotFounded = false;
				break;
			}

		if(StartStateNotFounded)
		{
			const char * ErrorStr = "Start state(0) was not found.";
			ErrorString = new char[strlen(ErrorStr) + 1]{};
			strcpy(ErrorString, ErrorStr);

			delete [] LinesNumbers;

			return ERROR;
		}
	}

	const char * PrevString = ProgramString[EmptyStringsShift];
	for(size_t i = EmptyStringsShift + 1; i < LinesCount; i++)
	{
		if(!WordCmp(PrevString, ProgramString[i]))
		{
			StatesCount++;
			PrevString = ProgramString[i];
		}
	}

	StatesEntriesCount = new uint8_t[StatesCount]{};
	StatesEntriesCount[0]++;
	size_t WordSize = WordLen(ProgramString[EmptyStringsShift]);
	StatesNames = new char *[StatesCount];
	StatesNames[0] = new char[WordSize + 1];
	strncpy(StatesNames[0], ProgramString[EmptyStringsShift], WordSize);
	StatesNames[0][WordSize] = '\0';

	PrevString = ProgramString[EmptyStringsShift];
	for(size_t i = EmptyStringsShift + 1, j = 0; i < LinesCount; i++)
	{
		if(!WordCmp(PrevString, ProgramString[i]))
		{
			j++;
			PrevString = ProgramString[i];
			WordSize = WordLen(ProgramString[i]);
			StatesNames[j] = new char[WordSize + 1];
			strncpy(StatesNames[j], ProgramString[i], WordSize);
			StatesNames[j][WordSize] = '\0';
		}
		StatesEntriesCount[j]++;
	}

	ProgramData = new ProgramUnit *[StatesCount]{};
	bool HaltFound = false, StateNotFound;
	for(size_t i = 0, Line = EmptyStringsShift, StringShift; i < StatesCount; i++)
	{
		ProgramData[i] = new ProgramUnit[StatesEntriesCount[i]];
		if(!strcmp(StatesNames[i], "0"))
			CurrentState = i;
		for(size_t j = 0; j < StatesEntriesCount[i]; j++, Line++)
		{
			StringShift = 0;
			StringShift += WordLen(ProgramString[Line] + StringShift) + 1;
			ProgramData[i][j].Key = ProgramString[Line][StringShift] == '_'? 0: ProgramString[Line][StringShift];
			for(size_t k = 0; k < j; k++)
				if(ProgramData[i][j].Key == ProgramData[i][k].Key)
				{
					const char * ErrorStr1 = "Multiple entries for symbol \'";
					const char * ErrorStr2 = "\' in state, named \'";
					const char * ErrorStr3 = "\'.";

					ErrorString = new char[strlen(ErrorStr1) + 1 + strlen(ErrorStr2) + strlen(StatesNames[i]) + strlen(ErrorStr3) + 1]{};

					strcpy(ErrorString, ErrorStr1);

					size_t ErrorStringShift = strlen(ErrorStr1);
					ErrorString[ErrorStringShift] = ProgramData[i][j].Key? ProgramData[i][j].Key: '_';

					ErrorStringShift++;
					strcpy(ErrorString + ErrorStringShift, ErrorStr2);

					ErrorStringShift += strlen(ErrorStr2);
					strcpy(ErrorString + ErrorStringShift, StatesNames[i]);

					ErrorStringShift += strlen(StatesNames[i]);
					strcpy(ErrorString + ErrorStringShift, ErrorStr3);

					delete [] LinesNumbers;

					return ERROR;
				}
			StringShift += 2;
			ProgramData[i][j].SetTo = ProgramString[Line][StringShift] == '_'? 0: ProgramString[Line][StringShift];
			StringShift += 2;
			switch(ProgramString[Line][StringShift])
			{
				case '*':
					ProgramData[i][j].TapeMove = EndlessTape::Stay;
					break;

				case 'r':
					ProgramData[i][j].TapeMove = EndlessTape::MoveRight;
					break;

				case 'l':
					ProgramData[i][j].TapeMove = EndlessTape::MoveLeft;
					break;

				default:
					const char * ErrorStr1 = "Symbol \'";
					const char * ErrorStr2 = "\' in the command on line ";
					const char * ErrorStr3 = " does not match any direction control character(r/l/*).";

					char LineNumBuffer[LineBufferSize];
					ErrorString = new char[strlen(ErrorStr1) + 1 + strlen(ErrorStr2) + itos(LinesNumbers[Line], LineNumBuffer) + strlen(ErrorStr3) + 1]{};

					strcpy(ErrorString, ErrorStr1);

					size_t ErrorStringShift = strlen(ErrorStr1);
					ErrorString[ErrorStringShift] = ProgramString[Line][StringShift];

					ErrorStringShift++;
					strcpy(ErrorString + ErrorStringShift, ErrorStr2);

					ErrorStringShift += strlen(ErrorStr2);
					strcpy(ErrorString + ErrorStringShift, LineNumBuffer);

					ErrorStringShift += strlen(LineNumBuffer);
					strcpy(ErrorString + ErrorStringShift, ErrorStr3);

					delete [] LinesNumbers;

					return ERROR;
			}
			StringShift += 2;
			StateNotFound = true;
			if(!strncmp(ProgramString[Line] + StringShift, "halt", 4))
			{
				HaltFound = true;
				StateNotFound = false;
				ProgramData[i][j].NextState = HALT;
			}
			else
				for(size_t k = 0; k < StatesCount; k++)
					if(!strcmp(ProgramString[Line] + StringShift, StatesNames[k]))
					{
						StateNotFound = false;
						ProgramData[i][j].NextState = k;
						break;
					}

			if(StateNotFound)
			{
				const char * ErrorStr1 = "Next state, named \'";
				const char * ErrorStr2 = "\'(Line ";
				const char * ErrorStr3 = ") does not correspond any existing state.";

				char LineNumBuffer[LineBufferSize];
				ErrorString = new char[strlen(ErrorStr1) + strlen(ProgramString[Line] + StringShift) + strlen(ErrorStr2) + itos(LinesNumbers[Line], LineNumBuffer) + strlen(ErrorStr3) + 1]{};

				strcpy(ErrorString, ErrorStr1);

				size_t ErrorStringShift = strlen(ErrorStr1);
				strcpy(ErrorString + ErrorStringShift, ProgramString[Line] + StringShift);

				ErrorStringShift += strlen(ProgramString[Line] + StringShift);
				strcpy(ErrorString + ErrorStringShift, ErrorStr2);

				ErrorStringShift += strlen(ErrorStr2);
				strcpy(ErrorString + ErrorStringShift, LineNumBuffer);

				ErrorStringShift += strlen(LineNumBuffer);
				strcpy(ErrorString + ErrorStringShift, ErrorStr3);

				delete [] LinesNumbers;

				return ERROR;
			}
		}
	}

	if(!HaltFound)
	{
		const char * ErrorStr = "Halt state was not found.";
		ErrorString = new char[strlen(ErrorStr) + 1]{};
		strcpy(ErrorString, ErrorStr);

		delete [] LinesNumbers;

		return ERROR;
	}

	ProgramIsValid = true;

	delete [] LinesNumbers;

	return SUCCESS;
}

void Program::Sort(char ** Strings, size_t n, size_t * Numbers)
{
	const size_t CiuraSteps[9] = {701, 301, 132, 57, 23, 10, 4, 1, 0};
	char * Cashe;
	size_t NumberCashe;
	for(size_t i = 0, d = CiuraSteps[i]; d != 0; d = CiuraSteps[++i])
		for(size_t i = d, j; i < n; i++)
		{
			Cashe = Strings[i];
			NumberCashe = Numbers[i];
			for(j = i; j >= d; j -= d)
			{
				if(strcmp(Cashe, Strings[j-d]) < 0)
				{
					Strings[j] = Strings[j-d];
					Numbers[j] = Numbers[j-d];
				}
				else
					break;
			}
			Strings[j] = Cashe;
			Numbers[j] = NumberCashe;
		}
}

bool Program::Execute(EndlessTape & TapeForExecution)
{
	if(!ProgramIsValid || Halted)
		return ERROR;

	char KeyForCheck = *TapeForExecution.GetCurrentSymbol();
	if(KeyForCheck == ' ')
		KeyForCheck = 0;

	int16_t KeyFinded = -1;
	for(uint8_t i = 0; i < StatesEntriesCount[CurrentState]; i++)
	{
		if(ProgramData[CurrentState][i].Key == KeyForCheck)
		{
			KeyFinded = i;
			break;
		}
		else
			if(ProgramData[CurrentState][i].Key == '*')
				KeyFinded = i;

	}

	if(KeyFinded > -1)
	{
		if(ProgramData[CurrentState][KeyFinded].SetTo != '*')
			TapeForExecution.PutSymbol(ProgramData[CurrentState][KeyFinded].SetTo);
		(TapeForExecution.*ProgramData[CurrentState][KeyFinded].TapeMove)();
		CurrentState = ProgramData[CurrentState][KeyFinded].NextState;
	}
	else
	{
		const char * ErrorStr1 = "State, named ";
		const char * ErrorStr2 = " has don't have entry for \'";
		const char * ErrorStr3 = "\'.";

		ErrorString = new char[strlen(ErrorStr1) + strlen(StatesNames[CurrentState]) + strlen(ErrorStr2) + 1 + strlen(ErrorStr3) + 1]{};

		strcpy(ErrorString, ErrorStr1);

		size_t ErrorStringShift = strlen(ErrorStr1);
		strcpy(ErrorString + ErrorStringShift, StatesNames[CurrentState]);

		ErrorStringShift += strlen(StatesNames[CurrentState]);
		strcpy(ErrorString + ErrorStringShift, ErrorStr2);

		ErrorStringShift += strlen(ErrorStr2);
		ErrorString[ErrorStringShift] = KeyForCheck? KeyForCheck: '_';

		ErrorStringShift++;
		strcpy(ErrorString + ErrorStringShift, ErrorStr3);

		return ERROR;
	}

	if(CurrentState == HALT)
		Halted = true;

	return SUCCESS;
}
