#include "pch.h"

#include <chrono>
#include <tchar.h>

#include "ThreadManager.h"
#include "Service.h"
#include "Session.h"
#include "GameSessionManager.h"
#include "GameSession.h"
#include "BufferWriter.h"
#include "ClientPacketHandler.h"
#include "Room.h"
#include "Job.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "XMLParser.h"
#include "Protocol.pb.h"

enum
{
    WORKER_TICK = 64,
};

void DoWorkerJob(ServerServiceRef& service)
{
    while (true)
    {
        LEndTickCount = ::GetTickCount64() + WORKER_TICK;

        // 네트워크 입출력 처리 -> 패킷 핸들러에 의해 인게임 로직까지 처리
        service->GetIOCPCore()->Dispatch(10);

        // 예약된 job 처리(job timer에 있는 job들을 배분)
        ThreadManager::DistributeReserveJobs();

        // global queue
        ThreadManager::DoGlobalQueueWork();
    }
}

int main()
{
    GSessionManager = new GameSessionManager;
    GRoom = std::make_shared<Room>();

    {
		// XML 파일 파싱
		XMLNode root;
		XMLParser parser;
		if (parser.ParseFromFile(L"GameDB.xml", OUT root) == false)
			return 0;

		// 파싱한 데이터를 통해 table 추출
		Vector<XMLNode> tables = root.FindChildren(L"Table");
		for (XMLNode& table : tables)
		{
			WString name = table.GetStringAttr(L"name");
			WString desc = table.GetStringAttr(L"desc");

			Vector<XMLNode> columns = table.FindChildren(L"Column");
			for (XMLNode& column : columns)
			{
				WString colName = column.GetStringAttr(L"name");
				WString colType = column.GetStringAttr(L"type");
				bool nullable = !column.GetBoolAttr(L"notnull", false);
				WString identity = column.GetStringAttr(L"identity");
				WString colDefault = column.GetStringAttr(L"default");
				// Etc...
			}

			Vector<XMLNode> indices = table.FindChildren(L"Index");
			for (XMLNode& index : indices)
			{
				WString indexType = index.GetStringAttr(L"type");
				bool primaryKey = index.FindChild(L"PrimaryKey").IsValid();
				bool uniqueConstraint = index.FindChild(L"UniqueKey").IsValid();

				Vector<XMLNode> columns = index.FindChildren(L"Column");
				for (XMLNode& column : columns)
				{
					WString colName = column.GetStringAttr(L"name");
				}
			}
		}

		// 파싱한 데이터를 통해 stored procedure 추출
		Vector<XMLNode> procedures = root.FindChildren(L"Procedure");
		for (XMLNode& procedure : procedures)
		{
			WString name = procedure.GetStringAttr(L"name");
			WString body = procedure.FindChild(L"Body").GetStringValue();

			Vector<XMLNode> params = procedure.FindChildren(L"Param");
			for (XMLNode& param : params)
			{
				WString paramName = param.GetStringAttr(L"name");
				WString paramType = param.GetStringAttr(L"type");
				// TODO..
			}
		}

        ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={ODBC Driver 17 for SQL Server};Server=(localdb)\\MSSQLLocalDB;Database=ServerDB;Trusted_Connection=Yes;"));


    }

    ClientPacketHandler::Init();

    ServerServiceRef service{ MakeShared<ServerService>(
        NetworkAddress(L"127.0.0.1", 7777),
        MakeShared<IOCPCore>(),
        MakeShared<GameSession>,                // TODO: Session manager 등
        100
    ) };

    ASSERT_CRASH(service->Start());

    for (int32 i = 0; i < 5; ++i)
    {
        GThreadManager->Launch(
            [&service]()
            {
                while (true)
                {
                    DoWorkerJob(service);
                }
            }
        );
    }

    // Main Thread
    DoWorkerJob(service);
    

    GThreadManager->Join();
    delete GSessionManager;

    return 0;
}


/**
* Command 패턴
* - 어떤 요청을 캡슐화해서 클래스, 함수 객체 등으로 만드는 패턴(주문서를 만들어서 직원에게 전달한다.)
*   - 어떤 요청을 다른 객체로 담고 있다가 누군가에게 건네준다.
*   - 요청을 처리하는 쓰레드는 요청만 처리하고,
*       job를 만드는 쓰레드는 job를 만들고 기다렸다가 요청을 처리하는 쓰레드에게 건내준다.
* - 장점
*   - 쓰레드마다 영역이 분리되어서, 각 쓰레드들은 각자 할일들에 집중할 수 있음
*   - 요청하는 시점, 실행하는 시점을 분리할 수 있음
*   - 요청을 수정, 취소 할수있다.
*/