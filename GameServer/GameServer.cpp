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
        ASSERT_CRASH(GDBConnectionPool->Connect(1, L"Driver={ODBC Driver 17 for SQL Server};Server=(localdb)\\MSSQLLocalDB;Database=ServerDB;Trusted_Connection=Yes;"));

        // Create table
        {
            auto query = L"                                     \
                DROP TABLE IF EXISTS [dbo].[Gold];              \
                CREATE TABLE [dbo].[Gold]                       \
                (                                               \
                    [id] INT NOT NULL PRIMARY KEY IDENTITY,     \
                    [gold] INT NULL,                            \
                    [name] NVARCHAR(50) NULL,                   \
                    [createDate] DATETIME NULL                  \
                );                                              \
            ";

            DBConnection* dbConn = GDBConnectionPool->Pop();
            ASSERT_CRASH(dbConn->Execute(query));
            GDBConnectionPool->Push(dbConn);
        }

        // Add data
        for (int32 i = 0; i < 3; ++i)
        {
            DBConnection* dbConn = GDBConnectionPool->Pop();

            {
                // 기존에 바인딩된 정보 제거
                //dbConn->Unbind();

                //// 넘길 인자를 바인딩
                //int32 gold = 100;
                //SQLLEN len = 0;

                //WCHAR name[100] = L"쿄쿄쿄";
                //SQLLEN nameLen = 0;

                //TIMESTAMP_STRUCT ts = { 0, };
                //ts.year = 2025;
                //ts.month = 2;
                //ts.day = 25;
                //SQLLEN tsLen = 0;

                //ASSERT_CRASH(dbConn->BindParam(1, &gold, &len));
                //ASSERT_CRASH(dbConn->BindParam(2, name, &nameLen));
                //ASSERT_CRASH(dbConn->BindParam(3, &ts, &tsLen));

                //// SQL 실행
                //// ?에 해당하는 부분에 인자가 들어감
                //ASSERT_CRASH(dbConn->Execute(L"INSERT INTO [dbo].[Gold]([gold], [name], [createDate]) VALUES(?, ? ,?)"));

            }
            
            {
                DBBind<3, 0> dbBind{ *dbConn, L"INSERT INTO [dbo].[Gold]([gold], [name], [createDate]) VALUES(?, ? ,?)" };

                int32 gold = 100;
                WCHAR name[100] = L"키키키";
                TIMESTAMP_STRUCT ts = { 0, };
                ts.year = 2025;
                ts.month = 2;
                ts.day = 26;

                dbBind.BindParam(0, gold);
                dbBind.BindParam(1, name);
                dbBind.BindParam(2, ts);

                ASSERT_CRASH(dbBind.Execute());
            }

            GDBConnectionPool->Push(dbConn);
        }

        // Read
        {
            DBConnection* dbConn = GDBConnectionPool->Pop();

            int32 gold = 100;

            int32 outId = 0;
            int32 outGold = 0;
            WCHAR outName[100] = { 0, };
            TIMESTAMP_STRUCT outDate = { 0, };

            {
                // 기존에 바인딩된 정보 제거
                //dbConn->Unbind();

                //// 넘길 인자를 바인딩
                //int32 gold = 100;
                //SQLLEN len = 0;
                //ASSERT_CRASH(dbConn->BindParam(1, &gold, &len));

                //// 결과물을 받을 메모리 바인딩
                //int32 outId = 0;
                //SQLLEN outIdLen = 0;

                //int32 outGold = 0;
                //SQLLEN outGoldLen = 0;

                //WCHAR outName[100] = { 0, };
                //SQLLEN outNameLen = 0;

                //TIMESTAMP_STRUCT outDate;
                //SQLLEN outDateLen = 0;

                //ASSERT_CRASH(dbConn->BindCol(1, &outId, &outIdLen));
                //ASSERT_CRASH(dbConn->BindCol(2, &outGold, &outGoldLen));
                //ASSERT_CRASH(dbConn->BindCol(3, outName, len32(outName), &outNameLen));
                //ASSERT_CRASH(dbConn->BindCol(4, &outDate, &outDateLen));

                //// SQL 실행
                //ASSERT_CRASH(dbConn->Execute(L"SELECT id, gold, name, createDate FROM [dbo].[Gold] WHERE gold = (?)"));

            }

            {
                DBBind<1, 4> dbBind{ *dbConn, L"SELECT id, gold, name, createDate FROM [dbo].[Gold] WHERE gold = (?)" };
                
                dbBind.BindParam(0, gold);

                dbBind.BindCol(0, OUT outId);
                dbBind.BindCol(1, OUT outGold);
                dbBind.BindCol(2, OUT outName);
                dbBind.BindCol(3, OUT outDate);

                ASSERT_CRASH(dbBind.Execute());
            }
            

            // 결과 가져오기
            {
                using namespace std;

                wcout.imbue(locale("kor"));
                while (dbConn->Fetch())
                {
                    wcout << L"id : " << outId << L", gold : " << outGold << L", name : " << outName << endl;
                    wcout << L"Data : " << outDate.year << L"/" << outDate.month << L"/" << outDate.day << endl;
                }
            }
            

            GDBConnectionPool->Push(dbConn);
        }
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