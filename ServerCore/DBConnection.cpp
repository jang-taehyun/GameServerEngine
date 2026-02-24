#include "pch.h"
#include "DBConnection.h"


/*---------------------
	 DB Connection
---------------------*/


bool DBConnection::Connect(SQLHENV henv, const WCHAR* connectionString)
{
	if (SQL_SUCCESS != ::SQLAllocHandle(SQL_HANDLE_DBC, henv, &_connection))
		return false;

	WCHAR stringBuffer[MAX_PATH] = { 0, };
	::wcscpy_s(stringBuffer, connectionString);

	WCHAR resultString[MAX_PATH] = { 0, };
	SQLSMALLINT resultStringLen = 0;

	SQLRETURN ret = ::SQLDriverConnectW(
		_connection,
		NULL,
		reinterpret_cast<SQLWCHAR*>(stringBuffer),
		_countof(stringBuffer),
		OUT reinterpret_cast<SQLWCHAR*>(resultString),
		_countof(resultString),
		OUT &resultStringLen,
		SQL_DRIVER_NOPROMPT);

	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		HandleError(ret, SQL_HANDLE_DBC, _connection);
		return false;
	}

	ret = ::SQLAllocHandle(SQL_HANDLE_STMT, _connection, &_statement);
	if (SQL_SUCCESS != ret)
	{
		HandleError(ret, SQL_HANDLE_STMT, _statement);
		return false;
	}
		
	return true;
}

void DBConnection::Clear()
{
	if (SQL_NULL_HANDLE != _connection)
	{
		::SQLFreeHandle(SQL_HANDLE_DBC, _connection);
		_connection = SQL_NULL_HANDLE;
	}
	
	if (SQL_NULL_HANDLE != _statement)
	{
		::SQLFreeHandle(SQL_HANDLE_STMT, _statement);
		_statement = SQL_NULL_HANDLE;
	}
}

bool DBConnection::Execute(const WCHAR* query)
{
	SQLRETURN ret = ::SQLExecDirectW(_statement, (SQLWCHAR*)query, SQL_NTSL);
	if (SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret)
		return true;

	HandleError(ret, SQL_HANDLE_STMT, _statement);
	return false;
}

bool DBConnection::Fetch()
{
	SQLRETURN ret = ::SQLFetch(_statement);

	switch (ret)
	{
	case SQL_SUCCESS:
	case SQL_SUCCESS_WITH_INFO:
		return true;
	case SQL_NO_DATA:
		return false;
	case SQL_ERROR:
		HandleError(ret, SQL_HANDLE_STMT, _statement);
		return false;
	default:
		return true;
	}
}

int32 DBConnection::GetRowCount()
{
	SQLLEN count = 0;
	SQLRETURN ret = ::SQLRowCount(_statement, OUT &count);

	if (SQL_SUCCESS == ret || SQL_SUCCESS_WITH_INFO == ret)
		return static_cast<int32>(count);

	return -1;
}

void DBConnection::Unbind()
{
	::SQLFreeStmt(_statement, SQL_UNBIND);
	::SQLFreeStmt(_statement, SQL_RESET_PARAMS);
	::SQLFreeStmt(_statement, SQL_CLOSE);
}

bool DBConnection::BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindParameter(_statement, paramIndex, SQL_PARAM_INPUT, cType, sqlType, len, 0, ptr, 0, index);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		HandleError(ret, SQL_HANDLE_STMT, _statement);
		return false;
	}

	return true;
}

bool DBConnection::BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index)
{
	SQLRETURN ret = ::SQLBindCol(_statement, columnIndex, cType, value, len, index);
	if (SQL_SUCCESS != ret && SQL_SUCCESS_WITH_INFO != ret)
	{
		HandleError(ret, SQL_HANDLE_STMT, _statement);
		return false;
	}

	return true;
}

void DBConnection::HandleError(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle)
{
	if (SQL_SUCCESS == ret)
		return;

	SQLSMALLINT index = 1;
	SQLWCHAR sqlState[MAX_PATH] = { 0, };
	SQLINTEGER nativeErr = 0;
	SQLWCHAR errMsg[MAX_PATH] = { 0, };
	SQLSMALLINT msgLen = 0;
	SQLRETURN errorReturn = 0;

	while (true)
	{
		errorReturn = ::SQLGetDiagRecW(
			handleType,
			handle,
			index,
			sqlState,
			OUT &nativeErr,
			errMsg,
			_countof(errMsg),
			OUT &msgLen
		);

		if (SQL_NO_DATA == errorReturn)
			break;

		if (SQL_SUCCESS != errorReturn && SQL_SUCCESS_WITH_INFO != errorReturn)
			break;

		// TODO: Log
		std::wcout.imbue(std::locale("kor"));
		std::wcout << errMsg << std::endl;

		++index;
	}
}
