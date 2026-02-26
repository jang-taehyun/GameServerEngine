#pragma once


/*---------------
	ConsoleLog
----------------*/

enum class Color
{
	BLACK,
	WHITE,
	RED,
	GREEN,
	BLUE,
	YELLOW,
};

class ConsoleLog
{
	enum { BUFFER_SIZE = 4096 };

public:
	void		WriteStdOut(Color color, const WCHAR* str, ...);
	void		WriteStdErr(Color color, const WCHAR* str, ...);

protected:
	void		SetColor(bool stdOut, Color color);

private:
	HANDLE		_stdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	HANDLE		_stdErr = ::GetStdHandle(STD_ERROR_HANDLE);
};