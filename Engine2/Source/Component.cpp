#include "pch.h"
#include "Component.h"

#include <assert.h>
E2::Component::Component(GameObject* pOwner)
    :m_pOwner{pOwner}
{
    //
}

uint32_t E2::Component::GetTypeId() const
{
    if (m_typeId == 0)
    {
        assert(false && "Component::GetTypeId: missing component type");
    }

    return m_typeId;
}
