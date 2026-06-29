#ifndef APP_UI_WIDGETS_HPP
#define APP_UI_WIDGETS_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/widgets/Widget.hpp>

#include <stdint.h>

class AppAsciiLabel : public touchgfx::Widget
{
public:
    enum Align
    {
        ALIGN_LEFT = 0,
        ALIGN_CENTER = 1,
        ALIGN_RIGHT = 2
    };

    AppAsciiLabel();

    void setText(const char* value);
    void setColors(touchgfx::colortype foreground, touchgfx::colortype background, bool opaqueBackground = true);
    void setScale(uint8_t value);
    void setAlignment(Align value);

    virtual void draw(const touchgfx::Rect& area) const;
    virtual touchgfx::Rect getSolidRect() const;

private:
    static const uint32_t MaxText = 64U;

    char text[MaxText];
    touchgfx::colortype foregroundColor;
    touchgfx::colortype backgroundColor;
    uint8_t scale;
    bool opaque;
    Align alignment;
};

class AppHeatMap : public touchgfx::Widget
{
public:
    AppHeatMap();

    void setValues(const uint8_t* source, uint32_t count, uint8_t peakIndex);
    void setColors(touchgfx::colortype background, touchgfx::colortype gridLine);

    virtual void draw(const touchgfx::Rect& area) const;
    virtual touchgfx::Rect getSolidRect() const;

private:
    static const uint32_t GridSize = 9U;
    static const uint32_t CellCount = GridSize * GridSize;

    uint8_t values[CellCount];
    uint8_t peak;
    touchgfx::colortype backgroundColor;
    touchgfx::colortype gridColor;
};

#endif /* APP_UI_WIDGETS_HPP */
