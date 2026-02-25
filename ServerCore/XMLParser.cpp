#include "pch.h"
#include "FileUtils.h"
#include "XMLParser.h"


/*-------------
	XML Node
--------------*/

_locale_t kr = _create_locale(LC_NUMERIC, "kor");

bool XMLNode::GetBoolAttr(const WCHAR* key, bool defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return ::_wcsicmp(attr->value(), L"true") == 0;

	return defaultValue;
}

int8 XMLNode::GetInt8Attr(const WCHAR* key, int8 defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return static_cast<int8>(::_wtoi(attr->value()));

	return defaultValue;
}

int16 XMLNode::GetInt16Attr(const WCHAR* key, int16 defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return static_cast<int16>(::_wtoi(attr->value()));

	return defaultValue;
}

int32 XMLNode::GetInt32Attr(const WCHAR* key, int32 defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return ::_wtoi(attr->value());

	return defaultValue;
}

int64 XMLNode::GetInt64Attr(const WCHAR* key, int64 defaultValue)
{
	xml_attribute<WCHAR>* attr = _node->first_attribute(key);
	if (attr)
		return ::_wtoi64(attr->value());

	return defaultValue;
}

float XMLNode::GetFloatAttr(const WCHAR* key, float defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return static_cast<float>(::_wtof(attr->value()));

	return defaultValue;
}

double XMLNode::GetDoubleAttr(const WCHAR* key, double defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return ::_wtof_l(attr->value(), kr);

	return defaultValue;
}

const WCHAR* XMLNode::GetStringAttr(const WCHAR* key, const WCHAR* defaultValue)
{
	XMLAttributeType* attr = _node->first_attribute(key);
	if (attr)
		return attr->value();

	return defaultValue;
}

bool XMLNode::GetBoolValue(bool defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return ::_wcsicmp(val, L"true") == 0;

	return defaultValue;
}

int8 XMLNode::GetInt8Value(int8 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int8>(::_wtoi(val));

	return defaultValue;
}

int16 XMLNode::GetInt16Value(int16 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int16>(::_wtoi(val));
	return defaultValue;
}

int32 XMLNode::GetInt32Value(int32 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int32>(::_wtoi(val));

	return defaultValue;
}

int64 XMLNode::GetInt64Value(int64 defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<int64>(::_wtoi64(val));

	return defaultValue;
}

float XMLNode::GetFloatValue(float defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return static_cast<float>(::_wtof(val));

	return defaultValue;
}

double XMLNode::GetDoubleValue(double defaultValue)
{
	WCHAR* val = _node->value();
	if (val)
		return ::_wtof_l(val, kr);

	return defaultValue;
}

const WCHAR* XMLNode::GetStringValue(const WCHAR* defaultValue)
{
	WCHAR* val = _node->first_node()->value();
	if (val)
		return val;

	return defaultValue;
}

XMLNode XMLNode::FindChild(const WCHAR* key)
{
	return XMLNode(_node->first_node(key));
}

Vector<XMLNode> XMLNode::FindChildren(const WCHAR* key)
{
	Vector<XMLNode> nodes;

	xml_node<WCHAR>* node = _node->first_node(key);
	while (node)
	{
		nodes.push_back(XMLNode(node));
		node = node->next_sibling(key);
	}

	return nodes;
}


/*---------------
	XML Parser
----------------*/

bool XMLParser::ParseFromFile(const WCHAR* path, OUT XMLNode& root)
{
	Vector<BYTE> bytes = FileUtils::ReadFile(path);
	_data = FileUtils::Convert(std::string(bytes.begin(), bytes.end()));

	if (_data.empty())
		return false;

	_document = MakeShared<XMLDocumentType>();
	_document->parse<0>(reinterpret_cast<WCHAR*>(&_data[0]));
	root = XMLNode(_document->first_node());
	return true;
}