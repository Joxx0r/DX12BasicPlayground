#pragma once

#include <vector>
#include <string>

enum class EYInputType
{
    Game,
    Overall
};

enum class EYInputBlockingContext : unsigned char
{
    Console,
    WindowApplication
};




class RevInputManager
{
public:
    RevInputManager() {   };

    static bool IsInputEnabled(EYInputType desiredType);
    static bool RegisterBlockingContext(EYInputBlockingContext context, bool state);
    static bool IsContextBlocked(EYInputBlockingContext context);

protected:

    int FindBlockingContext(EYInputBlockingContext context);
    
    std::vector<EYInputBlockingContext> m_blockingContexts;
};