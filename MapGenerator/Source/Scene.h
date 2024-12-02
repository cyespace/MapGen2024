#pragma once

class Scene
{
public:
    enum class SceneId
    {
        MenuScene,
        WorldScene,
        LocalScene,
    };

public:
    virtual ~Scene() = default;
    virtual bool Init() { return false; }
    virtual void End() {}
    virtual void Reset() {}
    virtual void Update(float deltaTime) = 0;
    virtual SceneId GetId() = 0;
};