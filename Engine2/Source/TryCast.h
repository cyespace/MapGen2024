#pragma once

namespace E2
{
    template<class TargetClass, class YourClass>
    TargetClass* Cast(YourClass* pThing)
    {
#ifdef _DEBUG
        auto* p = dynamic_cast<TargetClass*>(pThing);
        if (p == nullptr)
        {
            // error?
            
        }
        return p;

#endif // _DEBUG

        return static_cast<TargetClass*>(pThing);
    }
}
