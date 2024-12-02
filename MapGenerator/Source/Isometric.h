#pragma once

#include <IGame.h>
#include <EventListener.h>

class IsometricTest : public E2::IGame ,public E2::EventListener
{
private:
    int m_selectX = -1;
    int m_selectY = -1;
public:
    static IsometricTest& Get();
    virtual ~IsometricTest();
    virtual bool Init() final;
    virtual void Update(float deltaTime) final;
    virtual void ShutDown() final {}
    virtual const char* Config() final;

    virtual void OnNotify(E2::Event evt);
};