/******************************************************************************
* Copyright (c) 2018(-2026) STMicroelectronics.
* All rights reserved.
*
* This file is part of the TouchGFX 4.26.1 distribution.
*
* This software is licensed under terms that can be found in the LICENSE file in
* the root directory of this software component.
* If no LICENSE file comes with this software, it is provided AS-IS.
*
*******************************************************************************/

#include <touchgfx/Color.hpp>
#include <touchgfx/hal/Paint.hpp>
#include <touchgfx/lcd/LCD.hpp>

namespace touchgfx
{
namespace paint
{
namespace argb8888
{
void lineFromColor(uint32_t* const ptr, const int16_t count, const uint32_t painterColor, const uint8_t alpha)
{
    uint32_t* framebuffer = ptr;
    const uint32_t* const lineEnd = framebuffer + count;
    const uint32_t color = (painterColor & 0x00FFFFFF) | (alpha << 24);

    if (alpha == 0xFF)
    {
        do
        {
            *framebuffer = color;
        } while (++framebuffer < lineEnd);
    }
    else
    {
        const uint8_t painterRed = Color::getRed(color);
        const uint8_t painterGreen = Color::getGreen(color);
        const uint8_t painterBlue = Color::getBlue(color);

        do
        {
            const uint32_t argbBg = *framebuffer;
            const uint8_t alphaBg = argbBg >> 24;
            if (alphaBg == 0)
            {
                *framebuffer = color;
            }
            else if (alpha > 0)
            {
                const uint8_t alphaMult = LCD::div255(alpha * alphaBg);
                const uint8_t alphaOut = alpha + alphaBg - alphaMult;
                const uint8_t blueBg = (argbBg & 0xFF);
                const uint8_t greenBg = ((argbBg >> 8) & 0xFF);
                const uint8_t redBg = ((argbBg >> 16) & 0xFF);
                *framebuffer = ((painterBlue * alpha + blueBg * alphaBg - blueBg * alphaMult) / alphaOut) |
                               (((painterGreen * alpha + greenBg * alphaBg - greenBg * alphaMult) / alphaOut) << 8) |
                               (((painterRed * alpha + redBg * alphaBg - redBg * alphaMult) / alphaOut) << 16) |
                               (alphaOut << 24);
            }
        } while (++framebuffer < lineEnd);
    }
}

void lineFromRGB565(uint8_t* const ptr, const uint16_t* const data, const int16_t length, const uint8_t alpha)
{
    uint8_t* framebuffer = ptr;
    const uint8_t* const chunkend = framebuffer + length * 4;
    const uint16_t* bitmapPointer = data;

    if (alpha == 255)
    {
        do
        {
            const uint16_t pixel565 = *bitmapPointer++;
            *framebuffer++ = Color::getBlueFromRGB565(pixel565);
            *framebuffer++ = Color::getGreenFromRGB565(pixel565);
            *framebuffer++ = Color::getRedFromRGB565(pixel565);
            *framebuffer++ = 255;
        } while (framebuffer < chunkend);
    }
    else
    {
        do
        {
            const uint8_t alphaBg = framebuffer[3];
            const uint8_t alphaMult = LCD::div255(alpha * alphaBg);
            const uint8_t alphaOut = alpha + alphaBg - alphaMult;

            const uint8_t blueBg = *framebuffer;
            const uint8_t blueFg = Color::getBlueFromRGB565(*bitmapPointer);
            *framebuffer++ = (blueFg * alpha + blueBg * alphaBg - blueBg * alphaMult) / alphaOut;
            const uint8_t greenBg = *framebuffer;
            const uint8_t greenFg = Color::getGreenFromRGB565(*bitmapPointer);
            *framebuffer++ = (greenFg * alpha + greenBg * alphaBg - greenBg * alphaMult) / alphaOut;
            const uint8_t redBg = *framebuffer;
            const uint8_t redFg = Color::getRedFromRGB565(*bitmapPointer);
            *framebuffer++ = (redFg * alpha + redBg * alphaBg - redBg * alphaMult) / alphaOut;
            *framebuffer++ = alphaOut;
            bitmapPointer++;
        } while (framebuffer < chunkend);
    }
}

void lineFromARGB8888(uint8_t* const ptr, const uint32_t* const data, const int16_t length, const uint8_t alpha)
{
    uint8_t* framebuffer = ptr;
    const uint8_t* const chunkend = framebuffer + length * 4;
    const uint32_t* bitmapPointer = data;

    do
    {
        const uint8_t srcAlpha = (*bitmapPointer) >> 24;
        const uint8_t alphaFg = LCD::div255(alpha * srcAlpha);
        const uint8_t alphaBg = framebuffer[3];
        if (alphaFg == 255 || alphaBg == 0)
        {
            const uint8_t blueFg = *bitmapPointer;
            *framebuffer++ = blueFg;
            const uint8_t greenFg = (*bitmapPointer) >> 8;
            *framebuffer++ = greenFg;
            const uint8_t redFg = (*bitmapPointer) >> 16;
            *framebuffer++ = redFg;
            *framebuffer++ = alphaFg;
        }
        else if (alphaFg)
        {
            const uint8_t alphaMult = LCD::div255(alphaFg * alphaBg);
            const uint8_t alphaOut = alphaFg + alphaBg - alphaMult;

            const uint8_t blueBg = *framebuffer;
            const uint8_t blueFg = *bitmapPointer;
            *framebuffer++ = (blueFg * alphaFg + blueBg * (alphaBg - alphaMult)) / alphaOut;
            const uint8_t greenBg = *framebuffer;
            const uint8_t greenFg = (*bitmapPointer) >> 8;
            *framebuffer++ = (greenFg * alphaFg + greenBg * (alphaBg - alphaMult)) / alphaOut;
            const uint8_t redBg = *framebuffer;
            const uint8_t redFg = (*bitmapPointer) >> 16;
            *framebuffer++ = (redFg * alphaFg + redBg * (alphaBg - alphaMult)) / alphaOut;
            *framebuffer++ = alphaOut;
        }
        else
        {
            framebuffer += 4;
        }
        bitmapPointer++;
    } while (framebuffer < chunkend);
}

} // namespace argb8888
} // namespace paint
} // namespace touchgfx
