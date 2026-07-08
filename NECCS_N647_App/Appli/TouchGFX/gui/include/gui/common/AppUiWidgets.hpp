#ifndef APP_UI_WIDGETS_HPP
#define APP_UI_WIDGETS_HPP

#include <touchgfx/hal/Types.hpp>
#include <touchgfx/Unicode.hpp>
#include <touchgfx/containers/Container.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/widgets/Widget.hpp>

#include <stdint.h>

class AppTextLabel : public touchgfx::Container
{
public:
    enum Align
    {
        ALIGN_LEFT = 0,
        ALIGN_CENTER = 1,
        ALIGN_RIGHT = 2
    };

    enum Style
    {
        STYLE_SMALL = 0,
        STYLE_BODY = 1,
        STYLE_TITLE = 2
    };

    AppTextLabel();

    void setPosition(int16_t x, int16_t y, int16_t width, int16_t height);
    void setText(const char* value);
    void setColors(touchgfx::colortype foreground, touchgfx::colortype background, bool opaqueBackground = true);
    void setScale(uint8_t value);
    void setStyle(Style value);
    void setAlignment(Align value);

private:
    /* Longest live string is a media path (~40 chars); 56 keeps margin while
     * saving 80 bytes per label instance (~150 labels in the View). */
    static const uint32_t MaxText = 56U;

    uint16_t typedTextId() const;
    void refreshTypedText();
    void refreshLayout();

    touchgfx::Box background;
    touchgfx::TextAreaWithOneWildcard textArea;
    touchgfx::Unicode::UnicodeChar wildcard[MaxText];
    touchgfx::colortype foregroundColor;
    touchgfx::colortype backgroundColor;
    bool opaque;
    Style style;
    Align alignment;
};

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

class AppRgb565Preview : public touchgfx::Widget
{
public:
    AppRgb565Preview();

    void setSource(const uint16_t* pixels, uint16_t width, uint16_t height, bool valid);
    void setColors(touchgfx::colortype background, touchgfx::colortype border);

    virtual void draw(const touchgfx::Rect& area) const;
    virtual touchgfx::Rect getSolidRect() const;

private:
    const uint16_t* source;
    uint16_t sourceWidth;
    uint16_t sourceHeight;
    touchgfx::colortype backgroundColor;
    touchgfx::colortype borderColor;
    bool sourceValid;
};

/** Flat panel with software-drawn rounded corners and optional 1px border. */
class AppRoundedPanel : public touchgfx::Widget
{
public:
    AppRoundedPanel();

    void setStyle(touchgfx::colortype fill, uint8_t cornerRadius);
    void setBorder(touchgfx::colortype border, bool enabled);
    void setFillColor(touchgfx::colortype fill);

    virtual void draw(const touchgfx::Rect& area) const;
    virtual touchgfx::Rect getSolidRect() const;

private:
    static const uint8_t MaxRadius = 20U;

    touchgfx::colortype fillColor;
    touchgfx::colortype borderColor;
    uint8_t radius;
    bool borderEnabled;
};

#endif /* APP_UI_WIDGETS_HPP */
