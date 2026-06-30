#include <gui/common/AppUiWidgets.hpp>

#include <touchgfx/Color.hpp>
#include <touchgfx/TypedText.hpp>
#include <texts/TextKeysAndLanguages.hpp>
#include <touchgfx/hal/HAL.hpp>
#include <touchgfx/lcd/LCD.hpp>

#include <stddef.h>

namespace
{
const uint8_t* glyphFor(char input)
{
    static const uint8_t blank[7] = {0, 0, 0, 0, 0, 0, 0};
    static const uint8_t unknown[7] = {0x0E, 0x11, 0x01, 0x06, 0x04, 0x00, 0x04};

    if ((input >= 'a') && (input <= 'z'))
    {
        input = static_cast<char>(input - ('a' - 'A'));
    }

    switch (input)
    {
    case ' ': return blank;
    case '0': { static const uint8_t g[7] = {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}; return g; }
    case '1': { static const uint8_t g[7] = {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}; return g; }
    case '2': { static const uint8_t g[7] = {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}; return g; }
    case '3': { static const uint8_t g[7] = {0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}; return g; }
    case '4': { static const uint8_t g[7] = {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}; return g; }
    case '5': { static const uint8_t g[7] = {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}; return g; }
    case '6': { static const uint8_t g[7] = {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}; return g; }
    case '7': { static const uint8_t g[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}; return g; }
    case '8': { static const uint8_t g[7] = {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}; return g; }
    case '9': { static const uint8_t g[7] = {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}; return g; }
    case 'A': { static const uint8_t g[7] = {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}; return g; }
    case 'B': { static const uint8_t g[7] = {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}; return g; }
    case 'C': { static const uint8_t g[7] = {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}; return g; }
    case 'D': { static const uint8_t g[7] = {0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}; return g; }
    case 'E': { static const uint8_t g[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}; return g; }
    case 'F': { static const uint8_t g[7] = {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}; return g; }
    case 'G': { static const uint8_t g[7] = {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}; return g; }
    case 'H': { static const uint8_t g[7] = {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}; return g; }
    case 'I': { static const uint8_t g[7] = {0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}; return g; }
    case 'J': { static const uint8_t g[7] = {0x07, 0x02, 0x02, 0x02, 0x12, 0x12, 0x0C}; return g; }
    case 'K': { static const uint8_t g[7] = {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}; return g; }
    case 'L': { static const uint8_t g[7] = {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}; return g; }
    case 'M': { static const uint8_t g[7] = {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}; return g; }
    case 'N': { static const uint8_t g[7] = {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}; return g; }
    case 'O': { static const uint8_t g[7] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}; return g; }
    case 'P': { static const uint8_t g[7] = {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}; return g; }
    case 'Q': { static const uint8_t g[7] = {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}; return g; }
    case 'R': { static const uint8_t g[7] = {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}; return g; }
    case 'S': { static const uint8_t g[7] = {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}; return g; }
    case 'T': { static const uint8_t g[7] = {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}; return g; }
    case 'U': { static const uint8_t g[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}; return g; }
    case 'V': { static const uint8_t g[7] = {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}; return g; }
    case 'W': { static const uint8_t g[7] = {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}; return g; }
    case 'X': { static const uint8_t g[7] = {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}; return g; }
    case 'Y': { static const uint8_t g[7] = {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}; return g; }
    case 'Z': { static const uint8_t g[7] = {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}; return g; }
    case '-': { static const uint8_t g[7] = {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}; return g; }
    case '_': { static const uint8_t g[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F}; return g; }
    case '.': { static const uint8_t g[7] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}; return g; }
    case ':': { static const uint8_t g[7] = {0x00, 0x0C, 0x0C, 0x00, 0x0C, 0x0C, 0x00}; return g; }
    case '/': { static const uint8_t g[7] = {0x01, 0x01, 0x02, 0x04, 0x08, 0x10, 0x10}; return g; }
    case '%': { static const uint8_t g[7] = {0x19, 0x1A, 0x02, 0x04, 0x08, 0x0B, 0x13}; return g; }
    case '+': { static const uint8_t g[7] = {0x00, 0x04, 0x04, 0x1F, 0x04, 0x04, 0x00}; return g; }
    case '<': { static const uint8_t g[7] = {0x02, 0x04, 0x08, 0x10, 0x08, 0x04, 0x02}; return g; }
    case '>': { static const uint8_t g[7] = {0x08, 0x04, 0x02, 0x01, 0x02, 0x04, 0x08}; return g; }
    case '@': { static const uint8_t g[7] = {0x0E, 0x11, 0x17, 0x15, 0x17, 0x10, 0x0F}; return g; }
    case '(': { static const uint8_t g[7] = {0x02, 0x04, 0x08, 0x08, 0x08, 0x04, 0x02}; return g; }
    case ')': { static const uint8_t g[7] = {0x08, 0x04, 0x02, 0x02, 0x02, 0x04, 0x08}; return g; }
    case '#': { static const uint8_t g[7] = {0x0A, 0x0A, 0x1F, 0x0A, 0x1F, 0x0A, 0x0A}; return g; }
    case '=': { static const uint8_t g[7] = {0x00, 0x00, 0x1F, 0x00, 0x1F, 0x00, 0x00}; return g; }
    default: return unknown;
    }
}

uint32_t textLength(const char* text)
{
    uint32_t len = 0U;
    while ((text[len] != '\0') && (len < 63U))
    {
        ++len;
    }
    return len;
}

touchgfx::colortype heatColor(uint8_t value)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    if (value < 85U)
    {
        r = static_cast<uint8_t>(18U + (value / 6U));
        g = static_cast<uint8_t>(36U + ((uint32_t)value * 80U / 85U));
        b = static_cast<uint8_t>(50U + ((uint32_t)value * 70U / 85U));
    }
    else if (value < 170U)
    {
        const uint8_t t = static_cast<uint8_t>(value - 85U);
        r = static_cast<uint8_t>(45U + ((uint32_t)t * 140U / 85U));
        g = static_cast<uint8_t>(120U + ((uint32_t)t * 82U / 85U));
        b = static_cast<uint8_t>(115U - ((uint32_t)t * 80U / 85U));
    }
    else
    {
        const uint8_t t = static_cast<uint8_t>(value - 170U);
        r = static_cast<uint8_t>(185U + ((uint32_t)t * 55U / 85U));
        g = static_cast<uint8_t>(202U - ((uint32_t)t * 92U / 85U));
        b = static_cast<uint8_t>(35U - ((uint32_t)t * 20U / 85U));
    }

    return touchgfx::Color::getColorFromRGB(r, g, b);
}

void fillLocalRect(const touchgfx::Drawable& drawable,
                   const touchgfx::Rect& invalidatedArea,
                   touchgfx::Rect local,
                   touchgfx::colortype color,
                   uint8_t alpha = 255U)
{
    touchgfx::Rect dirty = local & invalidatedArea;
    if (dirty.isEmpty())
    {
        return;
    }

    drawable.translateRectToAbsolute(dirty);
    touchgfx::HAL::lcd().fillRect(dirty, color, alpha);
}
}

AppTextLabel::AppTextLabel()
    : foregroundColor(touchgfx::Color::getColorFromRGB(235, 240, 238)),
      backgroundColor(touchgfx::Color::getColorFromRGB(17, 19, 22)),
      opaque(true),
      style(STYLE_BODY),
      alignment(ALIGN_LEFT)
{
    wildcard[0] = 0U;

    background.setColor(backgroundColor);
    add(background);

    textArea.setColor(foregroundColor);
    textArea.setWildcard1(wildcard);
    add(textArea);

    refreshTypedText();
}

void AppTextLabel::setPosition(int16_t x, int16_t y, int16_t width, int16_t height)
{
    touchgfx::Container::setPosition(x, y, width, height);
    refreshLayout();
}

void AppTextLabel::setText(const char* value)
{
    if (value == 0)
    {
        value = "";
    }

    const uint16_t converted = touchgfx::Unicode::fromUTF8(reinterpret_cast<const uint8_t*>(value), wildcard, static_cast<uint16_t>(MaxText - 1U));
    wildcard[converted] = 0U;
    textArea.invalidate();
}

void AppTextLabel::setColors(touchgfx::colortype foreground, touchgfx::colortype backgroundColorValue, bool opaqueBackground)
{
    foregroundColor = foreground;
    backgroundColor = backgroundColorValue;
    opaque = opaqueBackground;

    background.setColor(backgroundColor);
    background.setVisible(opaque);
    textArea.setColor(foregroundColor);
    invalidate();
}

void AppTextLabel::setScale(uint8_t value)
{
    if (value >= 3U)
    {
        setStyle(STYLE_TITLE);
    }
    else if (value == 2U)
    {
        setStyle(STYLE_BODY);
    }
    else
    {
        setStyle(STYLE_SMALL);
    }
}

void AppTextLabel::setStyle(Style value)
{
    if (style != value)
    {
        style = value;
        refreshTypedText();
    }
}

void AppTextLabel::setAlignment(Align value)
{
    if (alignment != value)
    {
        alignment = value;
        refreshTypedText();
    }
}

uint16_t AppTextLabel::typedTextId() const
{
    if (style == STYLE_TITLE)
    {
        return T_WC_TITLE_LEFT;
    }

    if (style == STYLE_BODY)
    {
        if (alignment == ALIGN_CENTER)
        {
            return T_WC_BODY_CENTER;
        }
        if (alignment == ALIGN_RIGHT)
        {
            return T_WC_BODY_RIGHT;
        }
        return T_WC_BODY_LEFT;
    }

    if (alignment == ALIGN_CENTER)
    {
        return T_WC_SMALL_CENTER;
    }
    if (alignment == ALIGN_RIGHT)
    {
        return T_WC_SMALL_RIGHT;
    }
    return T_WC_SMALL_LEFT;
}

void AppTextLabel::refreshTypedText()
{
    textArea.setTypedText(touchgfx::TypedText(typedTextId()));
    textArea.setWildcard1(wildcard);
    refreshLayout();
    invalidate();
}

void AppTextLabel::refreshLayout()
{
    background.setPosition(0, 0, getWidth(), getHeight());
    textArea.setPosition(0, 0, getWidth(), getHeight());
}

AppAsciiLabel::AppAsciiLabel()
    : foregroundColor(touchgfx::Color::getColorFromRGB(235, 240, 238)),
      backgroundColor(touchgfx::Color::getColorFromRGB(17, 19, 22)),
      scale(2U),
      opaque(true),
      alignment(ALIGN_LEFT)
{
    text[0] = '\0';
}

void AppAsciiLabel::setText(const char* value)
{
    if (value == 0)
    {
        value = "";
    }

    uint32_t i = 0U;
    for (; (i + 1U) < MaxText && value[i] != '\0'; ++i)
    {
        text[i] = value[i];
    }
    text[i] = '\0';
    invalidate();
}

void AppAsciiLabel::setColors(touchgfx::colortype foreground, touchgfx::colortype background, bool opaqueBackground)
{
    foregroundColor = foreground;
    backgroundColor = background;
    opaque = opaqueBackground;
    invalidate();
}

void AppAsciiLabel::setScale(uint8_t value)
{
    scale = (value == 0U) ? 1U : value;
    invalidate();
}

void AppAsciiLabel::setAlignment(Align value)
{
    alignment = value;
    invalidate();
}

void AppAsciiLabel::draw(const touchgfx::Rect& area) const
{
    if (opaque)
    {
        fillLocalRect(*this, area, touchgfx::Rect(0, 0, getWidth(), getHeight()), backgroundColor);
    }

    const uint32_t len = textLength(text);
    const int16_t charAdvance = static_cast<int16_t>(6U * scale);
    const int16_t textWidth = (len == 0U) ? 0 : static_cast<int16_t>((len * 6U - 1U) * scale);
    int16_t startX = 0;

    if (alignment == ALIGN_CENTER)
    {
        startX = static_cast<int16_t>((getWidth() - textWidth) / 2);
    }
    else if (alignment == ALIGN_RIGHT)
    {
        startX = static_cast<int16_t>(getWidth() - textWidth);
    }

    if (startX < 0)
    {
        startX = 0;
    }

    const int16_t glyphHeight = static_cast<int16_t>(7U * scale);
    const int16_t startY = (getHeight() > glyphHeight) ? static_cast<int16_t>((getHeight() - glyphHeight) / 2) : 0;

    for (uint32_t i = 0U; i < len; ++i)
    {
        const uint8_t* rows = glyphFor(text[i]);
        const int16_t glyphX = static_cast<int16_t>(startX + (i * charAdvance));

        for (uint32_t row = 0U; row < 7U; ++row)
        {
            for (uint32_t col = 0U; col < 5U; ++col)
            {
                if ((rows[row] & (1U << (4U - col))) != 0U)
                {
                    fillLocalRect(*this,
                                  area,
                                  touchgfx::Rect(static_cast<int16_t>(glyphX + (col * scale)),
                                                 static_cast<int16_t>(startY + (row * scale)),
                                                 scale,
                                                 scale),
                                  foregroundColor);
                }
            }
        }
    }
}

touchgfx::Rect AppAsciiLabel::getSolidRect() const
{
    if (opaque)
    {
        return touchgfx::Rect(0, 0, getWidth(), getHeight());
    }
    return touchgfx::Rect();
}

AppHeatMap::AppHeatMap()
    : peak(40U),
      backgroundColor(touchgfx::Color::getColorFromRGB(18, 20, 23)),
      gridColor(touchgfx::Color::getColorFromRGB(44, 49, 54))
{
    for (uint32_t i = 0U; i < CellCount; ++i)
    {
        values[i] = 0U;
    }
}

void AppHeatMap::setValues(const uint8_t* source, uint32_t count, uint8_t peakIndex)
{
    const uint32_t copyCount = (count < CellCount) ? count : CellCount;

    for (uint32_t i = 0U; i < copyCount; ++i)
    {
        values[i] = source[i];
    }
    for (uint32_t i = copyCount; i < CellCount; ++i)
    {
        values[i] = 0U;
    }

    peak = (peakIndex < CellCount) ? peakIndex : 0U;
    invalidate();
}

void AppHeatMap::setColors(touchgfx::colortype background, touchgfx::colortype gridLine)
{
    backgroundColor = background;
    gridColor = gridLine;
    invalidate();
}

void AppHeatMap::draw(const touchgfx::Rect& area) const
{
    fillLocalRect(*this, area, touchgfx::Rect(0, 0, getWidth(), getHeight()), backgroundColor);

    const int16_t gap = 4;
    const int16_t cellW = static_cast<int16_t>((getWidth() - ((GridSize + 1U) * gap)) / GridSize);
    const int16_t cellH = static_cast<int16_t>((getHeight() - ((GridSize + 1U) * gap)) / GridSize);

    for (uint32_t row = 0U; row < GridSize; ++row)
    {
        for (uint32_t col = 0U; col < GridSize; ++col)
        {
            const uint32_t index = (row * GridSize) + col;
            const int16_t x = static_cast<int16_t>(gap + (col * (cellW + gap)));
            const int16_t y = static_cast<int16_t>(gap + (row * (cellH + gap)));
            const touchgfx::Rect cell(x, y, cellW, cellH);

            fillLocalRect(*this, area, cell, heatColor(values[index]));

            if (index == peak)
            {
                fillLocalRect(*this, area, touchgfx::Rect(x, y, cellW, 3), touchgfx::Color::getColorFromRGB(240, 244, 236));
                fillLocalRect(*this, area, touchgfx::Rect(x, static_cast<int16_t>(y + cellH - 3), cellW, 3), touchgfx::Color::getColorFromRGB(240, 244, 236));
                fillLocalRect(*this, area, touchgfx::Rect(x, y, 3, cellH), touchgfx::Color::getColorFromRGB(240, 244, 236));
                fillLocalRect(*this, area, touchgfx::Rect(static_cast<int16_t>(x + cellW - 3), y, 3, cellH), touchgfx::Color::getColorFromRGB(240, 244, 236));
            }
        }
    }

    for (uint32_t row = 0U; row <= GridSize; ++row)
    {
        const int16_t y = static_cast<int16_t>(row * (cellH + gap));
        fillLocalRect(*this, area, touchgfx::Rect(0, y, getWidth(), 1), gridColor, 160U);
    }
}

touchgfx::Rect AppHeatMap::getSolidRect() const
{
    return touchgfx::Rect(0, 0, getWidth(), getHeight());
}
