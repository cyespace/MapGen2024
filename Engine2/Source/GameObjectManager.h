#pragma once
#include <unordered_map>


namespace E2
{
    class GameObject;
    class GameObjectManager
    {
    private:
        int m_objectId = -1;
        int m_zoneSize = -1;
        int m_zoneXLimit = -1;
        int m_zoneYLimit = -1;
        int m_currentZoneX = -1;
        int m_currentZoneY = -1;

        int m_currentZoneId = -1;
        bool m_usingZone = false;
        std::vector<std::unordered_map<int,GameObject*>> m_gameObjects;

    public:
        GameObjectManager() = default;
        ~GameObjectManager();
        void InitZone(int mapXLimit, int mapYLimit, int zoneSize);
        void UpdateCurrentZonePosition(int focusX, int focusY);
        void Update(float deltaTime);

        void RenderGameObjects();
        void RenderGameObject(int id, int objectX = -1, int objectY = -1);

        void AddGameObject(GameObject* pObject, int objectX = -1, int objectY = -1);
        GameObject* GetGameObject(int id, int objectX = -1, int objectY = -1);
        void RemoveGameObject(int id, int objectX = -1, int objectY = -1);
        void DestroyAll();

    };
}
