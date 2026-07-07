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
        return (alignment == ALIGN_CENTER) ? T_WC_TITLE_CENTER : T_WC_TITLE_LEFT;
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

AppRgb565Preview::AppRgb565Preview()
    : source(0),
      sourceWidth(0U),
      sourceHeight(0U),
      backgroundColor(touchgfx::Color::getColorFromRGB(14, 16, 18)),
      borderColor(touchgfx::Color::getColorFromRGB(58, 66, 68)),
      sourceValid(false)
{
}

void AppRgb565Preview::setSource(const uint16_t* pixels, uint16_t width, uint16_t height, bool valid)
{
    const bool nextValid = valid && (pixels != 0) && (width != 0U) && (height != 0U);
    if ((source != pixels) || (sourceWidth != width) || (sourceHeight != height) || (sourceValid != nextValid))
    {
        source = pixels;
        sourceWidth = width;
        sourceHeight = height;
        sourceValid = nextValid;
        invalidate();
    }
}

void AppRgb565Preview::setColors(touchgfx::colortype background, touchgfx::colortype border)
{
    backgroundColor = background;
    borderColor = border;
    invalidate();
}

void AppRgb565Preview::draw(const touchgfx::Rect& area) const
{
    fillLocalRect(*this, area, touchgfx::Rect(0, 0, getWidth(), getHeight()), backgroundColor);
    fillLocalRect(*this, area, touchgfx::Rect(0, 0, getWidth(), 2), borderColor);
    fillLocalRect(*this, area, touchgfx::Rect(0, static_cast<int16_t>(getHeight() - 2), getWidth(), 2), borderColor);
    fillLocalRect(*this, area, touchgfx::Rect(0, 0, 2, getHeight()), borderColor);
    fillLocalRect(*this, area, touchgfx::Rect(static_cast<int16_t>(getWidth() - 2), 0, 2, getHeight()), borderColor);

    if (!sourceValid || (getWidth() <= 4) || (getHeight() <= 4))
    {
        return;
    }

    const touchgfx::Rect viewport(2, 2, static_cast<int16_t>(getWidth() - 4), static_cast<int16_t>(getHeight() - 4));
    const touchgfx::Rect dirty = viewport & area;
    if (dirty.isEmpty())
    {
        return;
    }

    touchgfx::HAL* hal = touchgfx::HAL::getInstance();
    if (hal == 0)
    {
        return;
    }

    uint16_t* framebuffer = hal->lockFrameBufferForRenderingMethod(touchgfx::HAL::SOFTWARE);
    if (framebuffer != 0)
    {
        touchgfx::Rect absolute = getAbsoluteRect();
        const uint32_t framebufferWidth = touchgfx::HAL::FRAME_BUFFER_WIDTH;

        for (int16_t y = dirty.y; y < dirty.bottom(); ++y)
        {
            const uint32_t srcY = (static_cast<uint32_t>(y - viewport.y) * sourceHeight) / static_cast<uint32_t>(viewport.height);
            uint16_t* dst = &framebuffer[(static_cast<uint32_t>(absolute.y + y) * framebufferWidth) + static_cast<uint32_t>(absolute.x + dirty.x)];

            for (int16_t x = dirty.x; x < dirty.right(); ++x)
            {
                const uint32_t srcX = (static_cast<uint32_t>(x - viewport.x) * sourceWidth) / static_cast<uint32_t>(viewport.width);
                *dst++ = source[(srcY * sourceWidth) + srcX];
            }
        }
    }

    hal->unlockFrameBuffer();
}

touchgfx::Rect AppRgb565Preview::getSolidRect() const
{
    return touchgfx::Rect(0, 0, getWidth(), getHeight());
}

namespace
{
uint16_t toRgb565(touchgfx::colortype color)
{
    const uint8_t r = touchgfx::Color::getRed(color);
    const uint8_t g = touchgfx::Color::getGreen(color);
    const uint8_t b = touchgfx::Color::getBlue(color);
    return static_cast<uint16_t>(((r & 0xF8U) << 8) | ((g & 0xFCU) << 3) | (b >> 3));
}

uint16_t blendRgb565(uint16_t background, uint16_t foreground, uint8_t alpha)
{
    const uint32_t inverse = 255U - alpha;
    const uint32_t bgR = (background >> 11) & 0x1FU;
    const uint32_t bgG = (background >> 5) & 0x3FU;
    const uint32_t bgB = background & 0x1FU;
    const uint32_t fgR = (foreground >> 11) & 0x1FU;
    const uint32_t fgG = (foreground >> 5) & 0x3FU;
    const uint32_t fgB = foreground & 0x1FU;
    const uint32_t r = ((bgR * inverse) + (fgR * alpha)) / 255U;
    const uint32_t g = ((bgG * inverse) + (fgG * alpha)) / 255U;
    const uint32_t b = ((bgB * inverse) + (fgB * alpha)) / 255U;
    return static_cast<uint16_t>((r << 11) | (g << 5) | b);
}

int32_t isqrt32(int32_t value)
{
    if (value <= 0)
    {
        return 0;
    }

    int32_t result = 0;
    int32_t bit = 1 << 30;
    while (bit > value)
    {
        bit >>= 2;
    }
    while (bit != 0)
    {
        if (value >= result + bit)
        {
            value -= result + bit;
            result = (result >> 1) + bit;
        }
        else
        {
            result >>= 1;
        }
        bit >>= 2;
    }
    return result;
}
}

AppRoundedPanel::AppRoundedPanel()
    : fillColor(touchgfx::Color::getColorFromRGB(15, 23, 35)),
      borderColor(touchgfx::Color::getColorFromRGB(36, 53, 74)),
      radius(10U),
      borderEnabled(false)
{
}

void AppRoundedPanel::setStyle(touchgfx::colortype fill, uint8_t cornerRadius)
{
    fillColor = fill;
    radius = (cornerRadius > MaxRadius) ? MaxRadius : cornerRadius;
    invalidate();
}

void AppRoundedPanel::setBorder(touchgfx::colortype border, bool enabled)
{
    borderColor = border;
    borderEnabled = enabled;
    invalidate();
}

void AppRoundedPanel::setFillColor(touchgfx::colortype fill)
{
    if (fillColor != fill)
    {
        fillColor = fill;
        invalidate();
    }
}

void AppRoundedPanel::draw(const touchgfx::Rect& area) const
{
    const int16_t w = getWidth();
    const int16_t h = getHeight();
    int16_t r = static_cast<int16_t>(radius);

    if ((r * 2) > w)
    {
        r = w / 2;
    }
    if ((r * 2) > h)
    {
        r = h / 2;
    }

    for (int16_t y = area.y; y < area.bottom(); ++y)
    {
        if ((y < 0) || (y >= h))
        {
            continue;
        }

        int16_t inset = 0;
        bool cornerRow = false;
        if (y < r)
        {
            const int32_t k = r - 1 - y;
            inset = static_cast<int16_t>(r - isqrt32(((int32_t)r * r) - (k * k)));
            cornerRow = true;
        }
        else if (y >= (h - r))
        {
            const int32_t k = y - (h - r);
            inset = static_cast<int16_t>(r - isqrt32(((int32_t)r * r) - (k * k)));
            cornerRow = true;
        }

        const int16_t spanX = inset;
        const int16_t spanW = static_cast<int16_t>(w - (2 * inset));
        if (spanW <= 0)
        {
            continue;
        }

        fillLocalRect(*this, area, touchgfx::Rect(spanX, y, spanW, 1), fillColor);

        if (borderEnabled)
        {
            if (cornerRow || (y == 0) || (y == (h - 1)))
            {
                if ((y == 0) || (y == (h - 1)))
                {
                    fillLocalRect(*this, area, touchgfx::Rect(spanX, y, spanW, 1), borderColor);
                }
                else
                {
                    fillLocalRect(*this, area, touchgfx::Rect(spanX, y, 2, 1), borderColor);
                    fillLocalRect(*this, area, touchgfx::Rect(static_cast<int16_t>(spanX + spanW - 2), y, 2, 1), borderColor);
                }
            }
            else
            {
                fillLocalRect(*this, area, touchgfx::Rect(0, y, 1, 1), borderColor);
                fillLocalRect(*this, area, touchgfx::Rect(static_cast<int16_t>(w - 1), y, 1, 1), borderColor);
            }
        }
    }
}

touchgfx::Rect AppRoundedPanel::getSolidRect() const
{
    const int16_t r = static_cast<int16_t>(radius);
    if (getHeight() <= (2 * r))
    {
        return touchgfx::Rect();
    }
    return touchgfx::Rect(0, r, getWidth(), static_cast<int16_t>(getHeight() - (2 * r)));
}

