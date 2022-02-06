#pragma once

class ImFont;

enum class RevFontType
{
    Invalid,
    ProggyClean,
    DroidSans
};

struct RevFont
{
    RevFont() {};

    void Destroy();
    bool IsValid() { return m_bigFont != nullptr && m_normalFont != nullptr  && m_smallFont != nullptr;  } const
    ImFont* m_bigFont = nullptr;
    ImFont* m_normalFont  = nullptr;
    ImFont* m_smallFont = nullptr;
    RevFontType m_fontType = RevFontType::Invalid;
};