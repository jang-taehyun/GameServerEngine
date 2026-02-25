#pragma once

#include "DBConnection.h"


/*--------------------------
	 DB Connection Pool
--------------------------*/

class DBConnectionPool
{
public:
	DBConnectionPool();
	~DBConnectionPool();

	bool Connect(int32 connectionCount, const WCHAR* connectionString);
	void Clear();

	DBConnection* Pop();
	void Push(DBConnection* connection);

private:
	USE_LOCK;
	SQLHENV _environment = SQL_NULL_HANDLE;		// environment을 담당하는 handle
	Vector<DBConnection*> _connections;
};

