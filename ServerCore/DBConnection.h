#pragma once

#include <sql.h>
#include <sqlext.h>


/*---------------------
	 DB Connection
---------------------*/

class DBConnection
{
public:
	bool Connect(SQLHENV henv, const WCHAR* connectionString);
	void Clear();

	bool Execute(const WCHAR* query);			// SQL 쿼리 실행
	bool Fetch();								// 쿼리 결과 받아오는 함수
	int32 GetRowCount();						// 데이터의 개수
	void Unbind();								// 바인딩한 인자들을 제거하는 함수

public:

	// SQL 쿼리에 넘길 인자들을 바인딩
	// paramIndex : 넘겨준 인자의 index 번호(인자가 여러 개 있을 수 있기 때문에 사용)
	// cType : 넘겨준 인자의 데이터 형식(자료형)
	// sqlType : SQL에 사용할 데이터 형식(자료형)
	bool BindParam(SQLUSMALLINT paramIndex, SQLSMALLINT cType, SQLSMALLINT sqlType, SQLULEN len, SQLPOINTER ptr, SQLLEN* index);
	
	// 쿼리 결과를 받아올 때, 결과를 받을 메모리 주소와 바인딩하는 함수
	bool BindCol(SQLUSMALLINT columnIndex, SQLSMALLINT cType, SQLULEN len, SQLPOINTER value, SQLLEN* index);
	void HandleError(SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle);

private:
	SQLHDBC _connection = SQL_NULL_HANDLE;		// DB 연결
	SQLHSTMT _statement = SQL_NULL_HANDLE;		// 연결 상태 관리
};

