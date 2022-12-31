#pragma once
#include <cassert>

struct source_location
{
    const char* file_name;
    unsigned line_number;
    const char* function_name;
};

#define CUR_SOURCE_LOCATION source_location{__FILE__, __LINE__, __func__}

static void CheckAssert(bool expression)
{
    if (!expression)
    {
        // handle failed assertion
        if(IsDebuggerPresent())
        {
            DebugBreak();
        }
        assert(expression);   
    }
}

#define DEBUG_ASSERT_ENABLED 1

#if DEBUG_ASSERT_ENABLED
#define DEBUG_ASSERT(Expr) \
CheckAssert(!!(Expr))
#else
#define DEBUG_ASSERT(Expr)
#endif