/**
  ******************************************************************************
  * @file    app_media_jpeg.c
  * @brief   Fixed-purpose grayscale baseline JPEG encoder.
  ******************************************************************************
  */

#include "app_media_jpeg.h"

#include <stddef.h>

typedef struct
{
  uint16_t code;
  uint8_t size;
} AppMediaJpegHuffCode_t;

typedef struct
{
  uint8_t *data;
  uint32_t capacity;
  uint32_t size;
  uint32_t error;
  uint32_t bit_buffer;
  uint8_t bit_count;
} AppMediaJpegWriter_t;

static const uint8_t s_zigzag[64] =
{
   0,  1,  8, 16,  9,  2,  3, 10,
  17, 24, 32, 25, 18, 11,  4,  5,
  12, 19, 26, 33, 40, 48, 41, 34,
  27, 20, 13,  6,  7, 14, 21, 28,
  35, 42, 49, 56, 57, 50, 43, 36,
  29, 22, 15, 23, 30, 37, 44, 51,
  58, 59, 52, 45, 38, 31, 39, 46,
  53, 60, 61, 54, 47, 55, 62, 63
};

static const uint8_t s_luma_quant[64] =
{
  12,  8,  8, 12, 18, 30, 38, 46,
   9,  9, 11, 15, 20, 44, 45, 41,
  11, 10, 12, 18, 30, 43, 52, 42,
  11, 13, 17, 22, 38, 65, 60, 47,
  14, 17, 28, 42, 51, 81, 77, 57,
  18, 26, 41, 48, 61, 78, 85, 69,
  37, 48, 59, 65, 77, 91, 90, 76,
  54, 69, 71, 73, 84, 75, 77, 74
};

static const float s_dct_cos[8][8] =
{
  { 1.000000f,  1.000000f,  1.000000f,  1.000000f,  1.000000f,  1.000000f,  1.000000f,  1.000000f },
  { 0.980785f,  0.831470f,  0.555570f,  0.195090f, -0.195090f, -0.555570f, -0.831470f, -0.980785f },
  { 0.923880f,  0.382683f, -0.382683f, -0.923880f, -0.923880f, -0.382683f,  0.382683f,  0.923880f },
  { 0.831470f, -0.195090f, -0.980785f, -0.555570f,  0.555570f,  0.980785f,  0.195090f, -0.831470f },
  { 0.707107f, -0.707107f, -0.707107f,  0.707107f,  0.707107f, -0.707107f, -0.707107f,  0.707107f },
  { 0.555570f, -0.980785f,  0.195090f,  0.831470f, -0.831470f, -0.195090f,  0.980785f, -0.555570f },
  { 0.382683f, -0.923880f,  0.923880f, -0.382683f, -0.382683f,  0.923880f, -0.923880f,  0.382683f },
  { 0.195090f, -0.555570f,  0.831470f, -0.980785f,  0.980785f, -0.831470f,  0.555570f, -0.195090f }
};

static const float s_dct_scale[8] =
{
  0.707107f, 1.000000f, 1.000000f, 1.000000f,
  1.000000f, 1.000000f, 1.000000f, 1.000000f
};

static const uint8_t s_dc_luma_bits[17] =
{
  0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0
};

static const uint8_t s_dc_luma_values[12] =
{
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
};

static const uint8_t s_ac_luma_bits[17] =
{
  0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d
};

static const uint8_t s_ac_luma_values[162] =
{
  0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
  0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
  0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xA1, 0x08,
  0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52, 0xD1, 0xF0,
  0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
  0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28,
  0x29, 0x2A, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
  0x3A, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
  0x4A, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
  0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
  0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
  0x7A, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
  0x8A, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
  0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
  0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
  0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
  0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4,
  0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xE1, 0xE2,
  0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA,
  0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
  0xF9, 0xFA
};

static void writer_put_u8(AppMediaJpegWriter_t *writer, uint8_t value)
{
  if (writer->error != 0U)
  {
    return;
  }

  if (writer->size >= writer->capacity)
  {
    writer->error = APP_MEDIA_JPEG_NO_SPACE;
    return;
  }

  writer->data[writer->size++] = value;
}

static void writer_put_u16(AppMediaJpegWriter_t *writer, uint16_t value)
{
  writer_put_u8(writer, (uint8_t)(value >> 8));
  writer_put_u8(writer, (uint8_t)value);
}

static void writer_put_marker(AppMediaJpegWriter_t *writer, uint8_t marker)
{
  writer_put_u8(writer, 0xFFU);
  writer_put_u8(writer, marker);
}

static void writer_put_entropy_byte(AppMediaJpegWriter_t *writer, uint8_t value)
{
  writer_put_u8(writer, value);
  if (value == 0xFFU)
  {
    writer_put_u8(writer, 0x00U);
  }
}

static void writer_put_bits(AppMediaJpegWriter_t *writer, uint16_t code, uint8_t size)
{
  for (int32_t i = (int32_t)size - 1; i >= 0; --i)
  {
    writer->bit_buffer = (writer->bit_buffer << 1) | ((code >> (uint32_t)i) & 1U);
    writer->bit_count++;

    if (writer->bit_count == 8U)
    {
      writer_put_entropy_byte(writer, (uint8_t)writer->bit_buffer);
      writer->bit_buffer = 0U;
      writer->bit_count = 0U;
    }
  }
}

static void writer_flush_bits(AppMediaJpegWriter_t *writer)
{
  if (writer->bit_count != 0U)
  {
    const uint8_t pad = (uint8_t)(8U - writer->bit_count);
    const uint8_t value = (uint8_t)((writer->bit_buffer << pad) | ((1U << pad) - 1U));
    writer_put_entropy_byte(writer, value);
    writer->bit_buffer = 0U;
    writer->bit_count = 0U;
  }
}

static void build_huffman(const uint8_t *bits,
                          const uint8_t *values,
                          uint32_t value_count,
                          AppMediaJpegHuffCode_t *table,
                          uint32_t table_count)
{
  uint16_t code = 0U;
  uint32_t value_index = 0U;

  for (uint32_t i = 0U; i < table_count; ++i)
  {
    table[i].code = 0U;
    table[i].size = 0U;
  }

  for (uint8_t size = 1U; size <= 16U; ++size)
  {
    for (uint8_t j = 0U; j < bits[size]; ++j)
    {
      if (value_index < value_count)
      {
        const uint8_t value = values[value_index++];
        if (value < table_count)
        {
          table[value].code = code;
          table[value].size = size;
        }
      }
      code++;
    }
    code = (uint16_t)(code << 1);
  }
}

static uint8_t category_for_value(int32_t value)
{
  uint32_t magnitude = (value < 0) ? (uint32_t)(-value) : (uint32_t)value;
  uint8_t category = 0U;

  while (magnitude != 0U)
  {
    magnitude >>= 1;
    category++;
  }

  return category;
}

static uint16_t bits_for_value(int32_t value, uint8_t category)
{
  if (category == 0U)
  {
    return 0U;
  }

  if (value >= 0)
  {
    return (uint16_t)value;
  }

  return (uint16_t)(((1UL << category) - 1UL) + value);
}

static void emit_huffman_symbol(AppMediaJpegWriter_t *writer,
                                const AppMediaJpegHuffCode_t *table,
                                uint8_t symbol)
{
  writer_put_bits(writer, table[symbol].code, table[symbol].size);
}

static uint8_t rgb565_luma(uint16_t pixel)
{
  const uint32_t r5 = (pixel >> 11) & 0x1FU;
  const uint32_t g6 = (pixel >> 5) & 0x3FU;
  const uint32_t b5 = pixel & 0x1FU;
  const uint32_t r = (r5 << 3) | (r5 >> 2);
  const uint32_t g = (g6 << 2) | (g6 >> 4);
  const uint32_t b = (b5 << 3) | (b5 >> 2);

  return (uint8_t)((77U * r + 150U * g + 29U * b) >> 8);
}

static int32_t round_to_i32(float value)
{
  if (value >= 0.0f)
  {
    return (int32_t)(value + 0.5f);
  }
  return (int32_t)(value - 0.5f);
}

static void sample_block(const uint16_t *src,
                         uint32_t src_width,
                         uint32_t src_height,
                         uint32_t out_width,
                         uint32_t out_height,
                         uint32_t block_x,
                         uint32_t block_y,
                         float block[64])
{
  for (uint32_t y = 0U; y < 8U; ++y)
  {
    uint32_t oy = block_y + y;
    uint32_t sy;

    if (oy >= out_height)
    {
      oy = out_height - 1U;
    }
    sy = (oy * src_height) / out_height;
    if (sy >= src_height)
    {
      sy = src_height - 1U;
    }

    for (uint32_t x = 0U; x < 8U; ++x)
    {
      uint32_t ox = block_x + x;
      uint32_t sx;

      if (ox >= out_width)
      {
        ox = out_width - 1U;
      }
      sx = (ox * src_width) / out_width;
      if (sx >= src_width)
      {
        sx = src_width - 1U;
      }

      block[(y * 8U) + x] = (float)rgb565_luma(src[(sy * src_width) + sx]) - 128.0f;
    }
  }
}

static void dct_quantize(const float block[64], int16_t quantized[64])
{
  int16_t natural[64];

  for (uint32_t v = 0U; v < 8U; ++v)
  {
    for (uint32_t u = 0U; u < 8U; ++u)
    {
      float sum = 0.0f;

      for (uint32_t y = 0U; y < 8U; ++y)
      {
        for (uint32_t x = 0U; x < 8U; ++x)
        {
          sum += block[(y * 8U) + x] * s_dct_cos[u][x] * s_dct_cos[v][y];
        }
      }

      sum *= 0.25f * s_dct_scale[u] * s_dct_scale[v];
      natural[(v * 8U) + u] = (int16_t)round_to_i32(sum / (float)s_luma_quant[(v * 8U) + u]);
    }
  }

  for (uint32_t i = 0U; i < 64U; ++i)
  {
    quantized[i] = natural[s_zigzag[i]];
  }
}

static void encode_block(AppMediaJpegWriter_t *writer,
                         const int16_t quantized[64],
                         int32_t *previous_dc,
                         const AppMediaJpegHuffCode_t *dc_table,
                         const AppMediaJpegHuffCode_t *ac_table)
{
  const int32_t dc_diff = (int32_t)quantized[0] - *previous_dc;
  uint8_t category = category_for_value(dc_diff);
  uint8_t run = 0U;

  emit_huffman_symbol(writer, dc_table, category);
  writer_put_bits(writer, bits_for_value(dc_diff, category), category);
  *previous_dc = quantized[0];

  for (uint32_t i = 1U; i < 64U; ++i)
  {
    const int32_t value = quantized[i];
    if (value == 0)
    {
      run++;
      continue;
    }

    while (run >= 16U)
    {
      emit_huffman_symbol(writer, ac_table, 0xF0U);
      run = (uint8_t)(run - 16U);
    }

    category = category_for_value(value);
    emit_huffman_symbol(writer, ac_table, (uint8_t)((run << 4) | category));
    writer_put_bits(writer, bits_for_value(value, category), category);
    run = 0U;
  }

  if (run != 0U)
  {
    emit_huffman_symbol(writer, ac_table, 0x00U);
  }
}

static void emit_app0(AppMediaJpegWriter_t *writer)
{
  static const uint8_t jfif[5] = {'J', 'F', 'I', 'F', 0};

  writer_put_marker(writer, 0xE0U);
  writer_put_u16(writer, 16U);
  for (uint32_t i = 0U; i < sizeof(jfif); ++i)
  {
    writer_put_u8(writer, jfif[i]);
  }
  writer_put_u16(writer, 0x0101U);
  writer_put_u8(writer, 0U);
  writer_put_u16(writer, 1U);
  writer_put_u16(writer, 1U);
  writer_put_u8(writer, 0U);
  writer_put_u8(writer, 0U);
}

static void emit_dqt(AppMediaJpegWriter_t *writer)
{
  writer_put_marker(writer, 0xDBU);
  writer_put_u16(writer, 67U);
  writer_put_u8(writer, 0U);
  for (uint32_t i = 0U; i < 64U; ++i)
  {
    writer_put_u8(writer, s_luma_quant[s_zigzag[i]]);
  }
}

static void emit_sof0(AppMediaJpegWriter_t *writer, uint32_t width, uint32_t height)
{
  writer_put_marker(writer, 0xC0U);
  writer_put_u16(writer, 11U);
  writer_put_u8(writer, 8U);
  writer_put_u16(writer, (uint16_t)height);
  writer_put_u16(writer, (uint16_t)width);
  writer_put_u8(writer, 1U);
  writer_put_u8(writer, 1U);
  writer_put_u8(writer, 0x11U);
  writer_put_u8(writer, 0U);
}

static void emit_dht(AppMediaJpegWriter_t *writer,
                     uint8_t table_class,
                     uint8_t table_id,
                     const uint8_t *bits,
                     const uint8_t *values,
                     uint32_t value_count)
{
  writer_put_marker(writer, 0xC4U);
  writer_put_u16(writer, (uint16_t)(3U + 16U + value_count));
  writer_put_u8(writer, (uint8_t)((table_class << 4) | table_id));
  for (uint32_t i = 1U; i <= 16U; ++i)
  {
    writer_put_u8(writer, bits[i]);
  }
  for (uint32_t i = 0U; i < value_count; ++i)
  {
    writer_put_u8(writer, values[i]);
  }
}

static void emit_sos(AppMediaJpegWriter_t *writer)
{
  writer_put_marker(writer, 0xDAU);
  writer_put_u16(writer, 8U);
  writer_put_u8(writer, 1U);
  writer_put_u8(writer, 1U);
  writer_put_u8(writer, 0x00U);
  writer_put_u8(writer, 0U);
  writer_put_u8(writer, 63U);
  writer_put_u8(writer, 0U);
}

uint32_t AppMediaJpeg_EncodeGrayFromRgb565(const uint16_t *src,
                                           uint32_t src_width,
                                           uint32_t src_height,
                                           uint32_t out_width,
                                           uint32_t out_height,
                                           uint8_t *out,
                                           uint32_t out_capacity,
                                           uint32_t *out_size)
{
  AppMediaJpegWriter_t writer;
  AppMediaJpegHuffCode_t dc_table[16];
  AppMediaJpegHuffCode_t ac_table[256];
  float block[64];
  int16_t quantized[64];
  int32_t previous_dc = 0;

  if ((src == NULL) || (out == NULL) || (out_size == NULL) ||
      (src_width == 0U) || (src_height == 0U) ||
      (out_width == 0U) || (out_height == 0U) ||
      (out_width > 65535U) || (out_height > 65535U))
  {
    return APP_MEDIA_JPEG_BAD_PARAM;
  }

  writer.data = out;
  writer.capacity = out_capacity;
  writer.size = 0U;
  writer.error = 0U;
  writer.bit_buffer = 0U;
  writer.bit_count = 0U;

  build_huffman(s_dc_luma_bits, s_dc_luma_values, sizeof(s_dc_luma_values), dc_table, 16U);
  build_huffman(s_ac_luma_bits, s_ac_luma_values, sizeof(s_ac_luma_values), ac_table, 256U);

  writer_put_marker(&writer, 0xD8U);
  emit_app0(&writer);
  emit_dqt(&writer);
  emit_sof0(&writer, out_width, out_height);
  emit_dht(&writer, 0U, 0U, s_dc_luma_bits, s_dc_luma_values, sizeof(s_dc_luma_values));
  emit_dht(&writer, 1U, 0U, s_ac_luma_bits, s_ac_luma_values, sizeof(s_ac_luma_values));
  emit_sos(&writer);

  for (uint32_t y = 0U; y < out_height; y += 8U)
  {
    for (uint32_t x = 0U; x < out_width; x += 8U)
    {
      sample_block(src, src_width, src_height, out_width, out_height, x, y, block);
      dct_quantize(block, quantized);
      encode_block(&writer, quantized, &previous_dc, dc_table, ac_table);
      if (writer.error != 0U)
      {
        *out_size = writer.size;
        return writer.error;
      }
    }
  }

  writer_flush_bits(&writer);
  writer_put_marker(&writer, 0xD9U);

  *out_size = writer.size;
  return (writer.error == 0U) ? APP_MEDIA_JPEG_OK : writer.error;
}
