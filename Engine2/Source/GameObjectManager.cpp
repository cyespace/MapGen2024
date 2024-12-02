#include "pch.h"
#include "GameObjectManager.h"
#include "GameObject.h"

E2::GameObjectManager::~GameObjectManager()
{
    DestroyAll();
}

//mapXLimit/mapYLimit = the column/row count of the map
//zoneSize = how many tiles each zone would have, mesuring only one side
void E2::GameObjectManager::InitZone(int mapXLimit, int mapYLimit, int zoneSize)
{
    m_usingZone = true;
    m_zoneSize = zoneSize;

    int x = mapXLimit / zoneSize;
    int y = mapYLimit / zoneSize;
    if (mapXLimit % zoneSize != 0)
    {
        ++x;
    }
    if (mapYLimit % zoneSize != 0)
    {
        ++y;
    }

    m_zoneXLimit = x;
    m_zoneYLimit = y;

    int zoneCount = x * y;
    m_gameObjects.resize(zoneCount);
    m_currentZoneId = 0;
}

void E2::GameObjectManager::UpdateCurrentZonePosition(int focusX, int focusY)
{
    m_currentZoneX = focusX / m_zoneSize;
    m_currentZoneY = focusY / m_zoneSize;
}


void E2::GameObjectManager::Update(float deltaTime)
{
    if (!m_usingZone)
    {
        for (auto& zone : m_gameObjects)
        {
            for (auto& [objectId, pObject] : zone)
            {
                if (pObject)
                {
                    pObject->Update(deltaTime);
                }
                else
                {
                    assert(false && "GameObjectManager::Update: missing gameobject");
                }
            }
        }
    }
    else
    {
        int startZoneX = (m_currentZoneX - 1) >= 0 ? (m_currentZoneX - 1): 0 ;
        int startZoneY = (m_currentZoneY - 1) >= 0 ? (m_currentZoneY - 1): 0 ;
        int endZoneX = (m_currentZoneX + 1) <= m_zoneXLimit ? (m_currentZoneX + 1 ): m_zoneXLimit;
        int endZoneY = (m_currentZoneY + 1) <= m_zoneYLimit ? (m_currentZoneY + 1) : m_zoneYLimit;

        for (int y = startZoneY; y <= endZoneY; ++y)
        {
            for (int x = startZoneX; x <= endZoneX; ++x)
            {
                int currentId = x + y * m_zoneXLimit;
                assert(currentId >= 0 && currentId < m_gameObjects.size()
                    && "E2::GameObjectManager::Update: object pool overflow\n");
                auto& pool = m_gameObjects[currentId];
                for (auto& [id, pObject] : pool)
                {
                    if (pObject)
                    {
                        pObject->Update(deltaTime);
                    }
                    else
                    {
                        assert(false && "GameObjectManager::Update: missing gameobject");
                    }
                }
            }
        }
    }
}

void E2::GameObjectManager::RenderGameObjects()
{
    if (!m_usingZone)
    {
        for (auto& zone : m_gameObjects)
        {
            for (auto& [objectId, pObject] : zone)
            {
                if (pObject)
                {
                    pObject->Draw();
                }
                else
                {
                    assert(false && "GameObjectManager::RenderGameObjects: missing gameobject");
                }
            }
        }
    }
    else //only render what's on screen
    {
        //TODO
    }
}

void E2::GameObjectManager::RenderGameObject(int id, int objectX, int objectY)
{
    if (!m_usingZone || objectX == -1)
    {
        for (auto& zone : m_gameObjects)
        {
            for (auto& [objectId, pObject] : zone)
            {
                pObject->Draw();
            }
        }
    }
    else if (m_usingZone && objectX != -1)
    {
        int objectInZoneId = (objectX / m_zoneSize) + (objectY / m_zoneSize) * m_zoneXLimit;
        m_gameObjects[objectInZoneId][id]->Draw();
    }
    else
    {
        assert(false && "E2::GameObjectManager::RenderGameObject: couldn't find the object\n");
    }
}


void E2::GameObjectManager::AddGameObject(GameObject* pObject, int objectX, int objectY)
{
    if (pObject)
    {
        ++m_objectId;
        pObject->SetId(m_objectId);
        if(!m_usingZone || objectX == -1)
        {
            if (m_gameObjects.size() == 0)
            {
                m_gameObjects.resize(1);
            }
            m_gameObjects[0][m_objectId] = pObject;
        }
        else if (m_usingZone && objectX != -1)
        {
            int objectInZoneId = (objectX / m_zoneSize) + (objectY / m_zoneSize) * m_zoneXLimit;
            auto& currentZone = m_gameObjects[objectInZoneId];
            currentZone[m_objectId] = pObject;
        }
    }
}
E2::GameObject* E2::GameObjectManager::GetGameObject(int id, int objectX, int objectY)
{
    if (!m_usingZone || objectX == -1)
    {
        for (auto& pool : m_gameObjects)
        {
            if (pool.find(id) != pool.end())
            {
                return pool[id];
            }
        }
    }
    else if (m_usingZone && objectX != -1)
    {
        int objectInZoneId = (objectX / m_zoneSize) + (objectY / m_zoneSize) * m_zoneXLimit;
        auto& currentZone = m_gameObjects[objectInZoneId];
        if (currentZone.find(id) != currentZone.end())
        {
            return currentZone[id];
        }
    }
    else
    {
        assert(false && "E2::GameObjectManager::GetGameObject: couldn't find the object\n");
    }
    return nullptr;
}
void E2::GameObjectManager::RemoveGameObject(int id, int objectX, int objectY)
{
    if (!m_usingZone || objectX == -1)
    {
        for (auto& pool : m_gameObjects)
        {
            if (pool.find(id) != pool.end())
            {
                delete pool[id];
                pool[id] = nullptr;
                pool.erase(id);
                return;
            }
        }
    }
    else if(m_usingZone && objectX != -1)
    {
        int objectInZoneId = (objectX / m_zoneSize) + (objectY / m_zoneSize) * m_zoneXLimit;
        auto& currentZone = m_gameObjects[objectInZoneId];
        if (currentZone.find(id) != currentZone.end())
        {
            delete currentZone[id];
            currentZone[id] = nullptr;
            currentZone.erase(id);
            return;
        }
        else
        {
            assert(false && "E2::GameObjectManager::RemoveGameObject: couldn't find the object\n");
        }
    }
}

void E2::GameObjectManager::DestroyAll()
{
    for (auto& pool : m_gameObjects)
    {
        for (auto& [objectId, pObject] : pool)
        {
            delete pObject;
            pObject = nullptr;
        }
        pool.clear();
    }
    m_gameObjects.clear();
}
