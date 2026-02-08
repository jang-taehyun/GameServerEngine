#pragma once

class GameSession;

using GameSessionRef = std::shared_ptr<GameSession>;


/*---------------------------
     Game Session Manager
---------------------------*/

class GameSessionManager
{
public:
    void Add(GameSessionRef session);
    void Remove(GameSessionRef session);
    void BroadCast(SendBufferRef sendBuffer);

private:
    USE_LOCK;
    Set<GameSessionRef> _sessions;
};

extern GameSessionManager* GSessionManager;