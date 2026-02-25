#pragma once

#include "rapidxml.hpp"
using namespace rapidxml;

/*-------------
	XML Node
--------------*/

using XMLNodeType = xml_node<WCHAR>;
using XMLDocumentType = xml_document<WCHAR>;
using XMLAttributeType = xml_attribute<WCHAR>;

class XMLNode
{
public:
	XMLNode(XMLNodeType* node = nullptr) : _node(node) {}

	bool				IsValid() { return _node != nullptr; }

	bool				GetBoolAttr(const WCHAR* key, bool defaultValue = false);
	int8				GetInt8Attr(const WCHAR* key, int8 defaultValue = 0);
	int16				GetInt16Attr(const WCHAR* key, int16 defaultValue = 0);
	int32				GetInt32Attr(const WCHAR* key, int32 defaultValue = 0);
	int64				GetInt64Attr(const WCHAR* key, int64 defaultValue = 0);
	float				GetFloatAttr(const WCHAR* key, float defaultValue = 0.0f);
	double				GetDoubleAttr(const WCHAR* key, double defaultValue = 0.0);
	const WCHAR*		GetStringAttr(const WCHAR* key, const WCHAR* defaultValue = L"");

	bool				GetBoolValue(bool defaultValue = false);
	int8				GetInt8Value(int8 defaultValue = 0);
	int16				GetInt16Value(int16 defaultValue = 0);
	int32				GetInt32Value(int32 defaultValue = 0);
	int64				GetInt64Value(int64 defaultValue = 0);
	float				GetFloatValue(float defaultValue = 0.0f);
	double				GetDoubleValue(double defaultValue = 0.0);
	const WCHAR*		GetStringValue(const WCHAR* defaultValue = L"");

	XMLNode				FindChild(const WCHAR* key);
	Vector<XMLNode>		FindChildren(const WCHAR* key);

private:
	XMLNodeType* _node = nullptr;
};

/*---------------
	XML Parser
----------------*/

class XMLParser
{
public:
	bool ParseFromFile(const WCHAR* path, OUT XMLNode& root);

private:
	std::shared_ptr<XMLDocumentType>		_document = nullptr;
	WString									_data;
};