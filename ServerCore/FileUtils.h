#pragma once


/*-----------------
	FileUtils
------------------*/

class FileUtils
{
public:
	static Vector<BYTE>		ReadFile(const WCHAR* path);

	// UTF-8 파일을 UTF-16으로 변환
	static String			Convert(std::string str);
};