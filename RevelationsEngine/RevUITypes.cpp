#include "stdafx.h"
#include "RevUITypes.h"
#include "imgui/imgui.h"

void RevFont::Destroy()
{
    if(m_bigFont)
    {
        delete m_bigFont;
        m_bigFont = nullptr;
    }
    
    if(m_normalFont)
    {
        delete m_normalFont;
        m_normalFont = nullptr;
    }
    if(m_smallFont)
    {
        delete m_smallFont;
        m_smallFont = nullptr;
    }
}
