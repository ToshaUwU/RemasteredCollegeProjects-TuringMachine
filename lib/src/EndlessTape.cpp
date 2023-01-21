#include "EndlessTape.hpp"
#include <cstring>

void EndlessTape::MoveLeft()
{
	if(PositionInChunk == 0)
	{
		if(!GlobalPosition->PrevChunk)
		{
			GlobalPosition->PrevChunk = new DataChunk;
			GlobalPosition->PrevChunk->NextChunk = GlobalPosition;
			FirstChunk = GlobalPosition->PrevChunk;
			ChunksNumber++;
		}

		GlobalPosition = GlobalPosition->PrevChunk;
		PositionInChunk = SYMBOLS_IN_CHUNK - 1;
	}
	else
		PositionInChunk--;
	LastShift = LEFT;
}

void EndlessTape::MoveRight()
{
	if(PositionInChunk == SYMBOLS_IN_CHUNK - 1)
	{
		if(!GlobalPosition->NextChunk)
		{
			GlobalPosition->NextChunk = new DataChunk;
			GlobalPosition->NextChunk->PrevChunk = GlobalPosition;
			LastChunk = GlobalPosition->NextChunk;
			ChunksNumber++;
		}

		GlobalPosition = GlobalPosition->NextChunk;
		PositionInChunk = 0;
	}
	else
		PositionInChunk++;
	LastShift = RIGHT;
}

EndlessTape::EndlessTape()
{
	PositionInChunk = 0;
	LastShift = NONE;

	ChunksNumber = 1;
	GlobalPosition = new DataChunk;
	FirstChunk = GlobalPosition;
	LastChunk = GlobalPosition;
}

void EndlessTape::operator=(const char * String)
{
	size_t StringLength = strlen(String);

	DataChunk * CurrentChunk;
	while(FirstChunk)
	{
		CurrentChunk = FirstChunk->NextChunk;
		delete FirstChunk;
		FirstChunk = CurrentChunk;
	}

	PositionInChunk = 0;
	ChunksNumber = 1;
	GlobalPosition = FirstChunk = new DataChunk;
	for(CurrentChunk = FirstChunk; StringLength > SYMBOLS_IN_CHUNK; StringLength -= SYMBOLS_IN_CHUNK, String += SYMBOLS_IN_CHUNK)
	{
		strncpy(CurrentChunk->Symbols, String, SYMBOLS_IN_CHUNK);
		CurrentChunk->NextChunk = new DataChunk;
		CurrentChunk->NextChunk->PrevChunk = CurrentChunk;
		CurrentChunk = CurrentChunk->NextChunk;
		ChunksNumber++;
	}
	strncpy(CurrentChunk->Symbols, String, StringLength);
	LastChunk = CurrentChunk;
}

EndlessTape::~EndlessTape()
{
	DataChunk * Temp;
	while(FirstChunk)
	{
		Temp = FirstChunk->NextChunk;
		delete FirstChunk;
		FirstChunk = Temp;
	}
}

EndlessTape & EndlessTape::operator=(EndlessTape & CopiedTape)
{
	PositionInChunk = CopiedTape.PositionInChunk;
	ChunksNumber = CopiedTape.ChunksNumber;

	FirstChunk = new DataChunk;
	strncpy(FirstChunk->Symbols, CopiedTape.FirstChunk->Symbols, SYMBOLS_IN_CHUNK);
	if(CopiedTape.GlobalPosition == CopiedTape.FirstChunk)
		GlobalPosition = FirstChunk;

	DataChunk * i = FirstChunk;
	for(DataChunk * j = CopiedTape.FirstChunk->NextChunk; j; i = i->NextChunk, j = j->NextChunk)
	{
		i->NextChunk = new DataChunk;
		i->NextChunk->PrevChunk = i;
		strncpy(i->NextChunk->Symbols, j->Symbols, SYMBOLS_IN_CHUNK);
		if(CopiedTape.GlobalPosition == j)
			GlobalPosition = i->NextChunk;
	}
	LastChunk = i;

	return *this;
}

EndlessTape & EndlessTape::operator=(EndlessTape && MovedTape)
{
	PositionInChunk = MovedTape.PositionInChunk;
	GlobalPosition = MovedTape.GlobalPosition;
	MovedTape.GlobalPosition = nullptr;

	ChunksNumber = MovedTape.ChunksNumber;
	FirstChunk = MovedTape.FirstChunk;
	LastChunk = MovedTape.LastChunk;
	MovedTape.FirstChunk = MovedTape.LastChunk = nullptr;

	return *this;
}

void EndlessTape::ResetPosition()
{
	PositionInChunk = 0;
	GlobalPosition = FirstChunk;
	LastShift = NONE;
}
