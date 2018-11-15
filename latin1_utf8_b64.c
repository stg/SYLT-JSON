#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "latin1_utf8_b64.h"

// Base64 alphabet
const char base64lut[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Latin-1 to UTF8 conversion
size_t latin1_to_utf8(char *out, size_t out_n, char *inp, size_t inp_n) {
  uint8_t l;
  uint_least16_t u;
  size_t ret_n = 0;
  while(inp_n--) {
    l = *inp++;
#ifdef UTF8_LOWCC_REPLACE
    if(l < 0x20) u = (int16_t)UTF8_UNAVAILABLE;
    else
#endif
    if(l < 0x7F) u = (int16_t)l;
    else if(l < 0xA0) u = (int16_t)UTF8_UNAVAILABLE;
    else u = (int16_t)l;
    if(u < 0x80) {
      if(ret_n < out_n) {
        *out++ = (char)u;
        ret_n++;
      } else break;
    } else {
      if(ret_n + 1 < out_n) {
        if(u < 0xC0) {
          *out++ = 0xC2;
          *out++ = (char)u;
        } else {
          *out++ = 0xC3;
          *out++ = (char)(u - 0x40);
        }
        ret_n += 2;
      } else break;
    }
  }
  return ret_n;
}

// Calculate space requirements for string converted from Latin-1 to UTF8
size_t latin1_to_utf8_size(char *inp, size_t inp_n) {
  size_t ret_n = 0;
  while(inp_n--) ret_n += (*inp++ < 0xA0 ? 1 : 2);
  return ret_n;
}

// UTF8 to Latin-1
size_t utf8_to_latin1(char *out, size_t out_n, char *inp, size_t inp_n) {
  size_t ret_n = 0;
  uint8_t d, dc;
  uint8_t l;
  while(inp_n--) {
    d = (uint8_t)*inp++;
    if(d & 0x80) {
      dc = 0;
      while(d & 0x80) {
        dc++;
        d <<= 1;
      }
      if(dc == 1) l = '?';
      else if(dc == 2) {
        if(inp_n) {
          if(d == 0x08) l = (uint8_t)*inp++;
          else if(d == 0x0C) l = ((uint8_t)*inp++) + 0x40;
          else l = UTF8_UNAVAILABLE;
        } else break; // error
      } else {
        l = UTF8_UNAVAILABLE;
        while(--dc) {
          if(inp_n) {
            inp++;
            inp_n--;
          } else break;
        }
      }
    } else l = (char)d;
    if(ret_n < out_n) {
      *out++ = l;
      out_n++;
    } else break;
  }
  return out_n;
}

// Convert octets to Base-64 (with padding for even 32-bit)
size_t octets_to_base64(char *out, size_t out_n, char *inp, size_t inp_n) {
  size_t ret_n = 0;
  while(inp_n) {
    if(out_n > 0) *out++ = base64lut[inp[0] >> 2];
    if(out_n > 1) *out++ = base64lut[((inp[0] & 0x03) << 4) | (((inp_n > 0 ? inp[1] & 0xF0 : 0)) >> 4)];
    if(out_n > 2) *out++ = (uint8_t)(inp_n > 1 ? base64lut[((inp[1] & 0x0F) << 2) | (((inp_n > 1 ? inp[2] & 0xC0 : 0)) >> 6)] : '=');
    if(out_n > 3) *out++ = (uint8_t)(inp_n > 2 ? base64lut[inp[2] & 0x3F] : '=');
    inp   += (inp_n > 2 ? 3 : inp_n);
    inp_n -= (inp_n > 2 ? 3 : inp_n);
    ret_n += (out_n > 3 ? 4 : out_n);
    out_n -= (out_n > 3 ? 4 : out_n);
  }
  return ret_n;
}

// Convert Base-64 to octets
size_t base64_to_octets(char *out, size_t out_n, char *inp, size_t inp_n) {
  uint8_t block[4], n = 0;
  char *p_dec;
  size_t ret_n = 0;
  // Make sure output is available (1 byte will always be written)
  if(out_n == 0) return 0;
  // Iterate over data
  while(inp_n--) {
#ifdef B64_ERROR_CHECK
    if(!(p_dec = memchr(base64lut, *inp++, 64))) break;
    block[n++] = p_dec - base64lut;
#else
    if(*inp == '=') break;
    else if(*inp < 0x30) block[n] = 62 + ((*inp - '+') >> 2);
    else if(*inp < 0x40) block[n] = 52 + (*inp - '0');
    else if(*inp < 0x60) block[n] = (*inp - 'A');
    else block[n] = 26 + (*inp - 'a');
    inp++;
    n++;
#endif
    // When four bytes have been read, decode block
    if((n &= 3) == 0) {
      *out++ = (block[0] << 2 | block[1] >> 4);
      if(ret_n++, !--out_n) return ret_n;
      *out++ = (block[1] << 4 | block[2] >> 2);
      if(ret_n++, !--out_n) return ret_n;
      *out++ = (block[2] << 6 | block[3]);
      if(ret_n++, !--out_n) return ret_n;
    }
  }
  // Decode remaining bytes
  if(n > 1) {
    ret_n += (n - 1 > out_n ? out_n : n - 1); // Precalculate return length
    *out++ = (block[0] << 2 | block[1] >> 4); // Decode first byte
    if(--out_n && n == 3) *out++ = (block[1] << 4 | block[2] >> 2); // Decode second byte
  }
  return ret_n;
}