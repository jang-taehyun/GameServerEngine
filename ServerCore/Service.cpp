#include "pch.h"
#include "Service.h"
#include "Session.h"

/*----------------
	 Service
----------------*/

Service::Service(ServiceType type, NetworkAddress address, IOCPCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: _type(type), _netAddress(address), _iocpCore(core), _sessionFactory(factory), _maxSessionCount(maxSessionCount)
{
}

Service::~Service()
{
}

void Service::CloseService()
{
	// TODO
}

// TODO: TEMP
void Service::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (const auto& session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

SessionRef Service::CreateSession()
{
	SessionRef session = _sessionFactory();
	session->SetService(shared_from_this());

	if (false == _iocpCore->Register(session))
		return nullptr;

	return session;
}

void Service::AddSession(SessionRef session)
{
	WRITE_LOCK;

	++_sessionCount;
	_sessions.insert(session);
}

void Service::ReleaseSession(SessionRef session)
{
	WRITE_LOCK;
	ASSERT_CRASH(0 != _sessions.erase(session));

	--_sessionCount;
}


/*---------------------
	 Client Service
---------------------*/

ClientService::ClientService(NetworkAddress targetAddress, IOCPCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::CLIENT, targetAddress, core, factory, maxSessionCount)
{
}

ClientService::~ClientService()
{
}

bool ClientService::Start()
{
	if (false == CanStart())
		return false;

	const int32 sessionCount = GetMaxSessionCount();
	for (int32 i = 0; i < sessionCount; ++i)
	{
		SessionRef session = CreateSession();
		if (false == session->Connect())
			return false;
	}

	return true;
}


/*---------------------
	 Server Service
---------------------*/

ServerService::ServerService(NetworkAddress address, IOCPCoreRef core, SessionFactory factory, int32 maxSessionCount)
	: Service(ServiceType::SERVER, address, core, factory, maxSessionCount)
{
}

ServerService::~ServerService()
{
}

bool ServerService::Start()
{
	if (false == CanStart())
		return false;

	_listener = MakeShared<Listener>();
	if (nullptr == _listener)
		return false;

	ServerServiceRef service = std::static_pointer_cast<ServerService>(shared_from_this());
	if (false == _listener->StartAccept(service))
		return false;

	return true;
}

void ServerService::CloseService()
{
	// TODO:
	Service::CloseService();
}
