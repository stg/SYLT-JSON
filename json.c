#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include "json.h"

// JSON parser states
#define PSTATE_ENTITY        0
#define PSTATE_STRING        1
#define PSTATE_NUMBER        2
#define PSTATE_CONSTANT      3
#define PSTATE_INVALID       4

// JSON grammar states
#define GSTATE_ENTER         0
#define GSTATE_OBJECT_IN     1
#define GSTATE_OBJECT_KEY    2
#define GSTATE_OBJECT_ASSIGN 3
#define GSTATE_OBJECT_PRE    4
#define GSTATE_OBJECT_POST   5
#define GSTATE_ARRAY_IN      6
#define GSTATE_ARRAY_PRE     7
#define GSTATE_ARRAY_POST    8
#define GSTATE_EXIT          9

const char json_str_null[] = "null";
const char json_str_true[] = "true";
const char json_str_false[] = "false";

char *json_to_string(void * value) {
  return (char *)((json_string *)value)->string;
}

double json_to_double(void * value) {
  json_number *ctx = (json_number *)value;
  double d;
  d = (double)ctx->number * ((ctx->flags & 1) ? -1.0 : +1.0);
  d *= pow(10, -(uint16_t)ctx->decimals + ((uint16_t)ctx->exponent * ((ctx->flags & 2) ? -1 : +1)));
  return d;
}

const char * json_const_str(uint8_t type) {
  switch(type) {
    case JSON_NULL: return json_str_null;
    case JSON_TRUE: return json_str_true;
    case JSON_FALSE: return json_str_false;
  }
  return NULL;
}

static uint8_t json_parse_grammar(char type, json_parser_ctx *parser_ctx) {
  json_grammar_ctx * ctx = &parser_ctx->grammar_ctx;

  uint8_t register state = ctx->state;
  uint8_t error = JSON_OK;

  if(state == GSTATE_EXIT) return JSON_TRAILING_DATA;

  if(type == '{') {

    // OBJECT BEGINS
    if(state == GSTATE_ENTER) {
      state = GSTATE_OBJECT_IN;
      if(parser_ctx->callback) error = parser_ctx->callback(0, JSON_OBJECT, NULL);
    } else if (state == GSTATE_ARRAY_IN || state == GSTATE_ARRAY_PRE) {
      state = GSTATE_OBJECT_IN;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_OBJECT, NULL);
    } else if(state == GSTATE_OBJECT_PRE) {
      state = GSTATE_OBJECT_IN;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_OBJECT, NULL);
    } else return JSON_BAD_GRAMMAR;
    
    if(ctx->stack_depth >= JSON_NESTING) return JSON_TOO_DEEP;
    ctx->stack[(ctx->stack_depth >> 3)] |= 1 << (ctx->stack_depth & 0x07);
    ctx->stack_depth++;
    
  } else if(type == '}') {

    // OBJECT ENDS
    if(state != GSTATE_OBJECT_IN && state != GSTATE_OBJECT_POST) return JSON_BAD_GRAMMAR;
    ctx->stack_depth--;
    if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_OBJECT_END, NULL);
    if(ctx->stack_depth == 0) {
      state = GSTATE_EXIT;
    } else if((ctx->stack[(ctx->stack_depth - 1) >> 3] & (1 << ((ctx->stack_depth - 1) & 0x07))) == 0) {
      state = GSTATE_ARRAY_POST;
    } else {
      state = GSTATE_OBJECT_POST;
    }

  } else if(type == '[') {

    // ARRAY BEGINS
    if(state == GSTATE_ENTER) {
      state = GSTATE_ARRAY_IN;
      if(parser_ctx->callback) error = parser_ctx->callback(0, JSON_ARRAY, NULL);
    } else if (state == GSTATE_ARRAY_IN || state == GSTATE_ARRAY_PRE) {
      state = GSTATE_ARRAY_IN;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_ARRAY, NULL);
    } else if(state == GSTATE_OBJECT_PRE) {
      state = GSTATE_ARRAY_IN;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_ARRAY, NULL);
    } else return JSON_BAD_GRAMMAR;
    if(ctx->stack_depth >= JSON_NESTING) return JSON_TOO_DEEP;
    ctx->stack[(ctx->stack_depth >> 3)] &= ~(1 << (ctx->stack_depth & 0x07));
    ctx->stack_depth++;
  
  } else if(type == ']') {
    
    // ARRAY ENDS
    if(state != GSTATE_ARRAY_IN && state != GSTATE_ARRAY_POST) return JSON_BAD_GRAMMAR;
    ctx->stack_depth--;
    if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_ARRAY_END, NULL);
    if(ctx->stack_depth == 0) {
      state = GSTATE_EXIT;
    } else if((ctx->stack[(ctx->stack_depth - 1) >> 3] & (1 << ((ctx->stack_depth - 1) & 0x07))) == 0) {
      state = GSTATE_ARRAY_POST;
    } else {
      state = GSTATE_OBJECT_POST;
    }
    
  } else if(type == ':') {
    
    // KEY/VALUE SEPARATOR
    if(state == GSTATE_OBJECT_ASSIGN) {
      state = GSTATE_OBJECT_PRE;
    } else return JSON_BAD_GRAMMAR;
    
  } else if(type == ',') {
    
    // ITEM SEPARATOR
    if(state == GSTATE_ARRAY_POST) {
      state = GSTATE_ARRAY_PRE;
    } else if(state == GSTATE_OBJECT_POST) {
      state = GSTATE_OBJECT_KEY;
    } else return JSON_BAD_GRAMMAR;
    
  } else if(type == 'S') {
    
    // STRING
    if(state == GSTATE_ENTER) {
      state = GSTATE_EXIT;
      if(parser_ctx->callback) error = parser_ctx->callback(0, JSON_STRING, &parser_ctx->u.s);
    } else if(state == GSTATE_ARRAY_IN || state == GSTATE_ARRAY_PRE) {
      state = GSTATE_ARRAY_POST;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_STRING, &parser_ctx->u.s);
    } else if(state == GSTATE_OBJECT_PRE) {
      state = GSTATE_OBJECT_POST;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_STRING, &parser_ctx->u.s);
    } else if(state == GSTATE_OBJECT_IN || state == GSTATE_OBJECT_KEY) {
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_KEY, &parser_ctx->u.s);
      state = GSTATE_OBJECT_ASSIGN;
    } else return JSON_BAD_GRAMMAR;
    
  } else if(type == 'N') {

    // NUMBER
    if(state == GSTATE_ENTER) {
      state = GSTATE_EXIT;
      if(parser_ctx->callback) error = parser_ctx->callback(0, JSON_NUMBER, &parser_ctx->u.n);
    } else if (state == GSTATE_ARRAY_IN || state == GSTATE_ARRAY_PRE) {
      state = GSTATE_ARRAY_POST;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_NUMBER, &parser_ctx->u.n);
    } else if(state == GSTATE_OBJECT_PRE) {
      state = GSTATE_OBJECT_POST;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, JSON_NUMBER, &parser_ctx->u.n);
    } else return JSON_BAD_GRAMMAR;
    
  } else if(type == 'C') {

    // CONSTANT
    if(parser_ctx->u.s.string_end - parser_ctx->u.s.string == 4) {
      if(!JSON_MEMCMP(parser_ctx->u.s.string, json_str_null, 4)) type = JSON_NULL;
      else if(!JSON_MEMCMP(parser_ctx->u.s.string, json_str_true, 4)) type = JSON_TRUE;
    } else if(parser_ctx->u.s.string_end - parser_ctx->u.s.string == 5) {
      if(!JSON_MEMCMP(parser_ctx->u.s.string, json_str_false, 5)) type = JSON_FALSE;
    }
    if(type == 'C') return JSON_BAD_CONSTANT;

    if(state == GSTATE_ENTER) {
      state = GSTATE_EXIT;
      if(parser_ctx->callback) error = parser_ctx->callback(0, type, NULL);
    } else if(state == GSTATE_ARRAY_IN || state == GSTATE_ARRAY_PRE) {
      state = GSTATE_ARRAY_POST;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, type, NULL);
    } else if(state == GSTATE_OBJECT_PRE) {
      state = GSTATE_OBJECT_POST;
      if(parser_ctx->callback) error = parser_ctx->callback(ctx->stack_depth, type, NULL);
    } else return JSON_BAD_GRAMMAR;
  }

  ctx->state = state;

  return error;
}

uint8_t json_octet(json_parser_ctx * ctx, uint8_t q) {
  uint8_t error = 0;
  bool repeat;
  do {
    repeat = false;
    if(ctx->state == PSTATE_ENTITY) {
      if(q == ' ' || q == '\t' || q == '\n' || q == '\r') {
        // White-space ignored
      } else if(q == '{' || q == '}' || q == '[' || q == ']' || q == ',' || q == ':') {
        // Structural
        error = json_parse_grammar(q, ctx);
      } else if(q == '"') {
        // Start of string
        ctx->state = PSTATE_STRING;
        ctx->u.s.string_end = ctx->u.s.string = ctx->buffer;
        ctx->sub_state = 0;
      } else if(q == '-' || (q >= '0' && q <= '9')) {
        // Start of number
        ctx->state = PSTATE_NUMBER;
        ctx->u.n.number = 0;
        ctx->sub_state = 0;
        repeat = true;
      } else if(q >= 'a' && q <= 'z') {
        // Start of constant
        ctx->state = PSTATE_CONSTANT;
        ctx->u.s.string_end = ctx->u.s.string = ctx->buffer;
        repeat = true;
      } else return JSON_UNEXPECTED_CHARACTER;
    } else if(ctx->state == PSTATE_CONSTANT) {
      if(q < 'a' || q > 'z') {
        error = json_parse_grammar('C', ctx);
        ctx->state = PSTATE_ENTITY;
        repeat = true;
      } else {
        if(ctx->u.s.string_end - ctx->u.s.string > 5) return JSON_BAD_CONSTANT;
        *ctx->u.s.string_end++ = q;
      }
    } else if(ctx->state == PSTATE_STRING) {
      if(ctx->sub_state == 0) {
        if(q == '"') {
          *ctx->u.s.string_end = 0;
          error = json_parse_grammar('S', ctx);
          ctx->state = PSTATE_ENTITY;
        } else if(q == '\\') {
          ctx->sub_state = 1;
        } else if(q > 0x1F) {
          *ctx->u.s.string_end++ = q;
          if(ctx->buffer_size && ctx->u.s.string_end - ctx->u.s.string >= ctx->buffer_size) return JSON_STRING_OVERFLOW;
        } else {
          return JSON_MALFORMED_STRING;
        }
      } else if(ctx->sub_state == 1) {
        if(q == 'u') {
          ctx->sub_state = 2;
        } else {
          ctx->sub_state = 0;
          switch(q) {
            case '\"': *ctx->u.s.string_end++ = '\"'; break;
            case '\\': *ctx->u.s.string_end++ = '\\'; break;
            case  '/': *ctx->u.s.string_end++ =  '/'; break;
            case  'b': *ctx->u.s.string_end++ = 0x08; break;
            case  't': *ctx->u.s.string_end++ = 0x09; break;
            case  'n': *ctx->u.s.string_end++ = 0x0A; break;
            case  'f': *ctx->u.s.string_end++ = 0x0C; break;
            case  'r': *ctx->u.s.string_end++ = 0x0D; break;
            default : return JSON_MALFORMED_ESCAPE;
          }
        }
      } else {
        if(q >= '0' && q <= '9') q -= '0';
        else if(q >= 'a' && q <= 'f') q -= ('a' - 0x0A);
        else if(q >= 'A' && q <= 'F') q -= ('A' - 0x0A);
        else return JSON_MALFORMED_ESCAPE;
        if(ctx->sub_state > 4) {
          ctx->u.s.string_end[1] |= q;
          if(ctx->u.s.string_end[0] == 0x00 && ctx->u.s.string_end[1] < 0x80) {
            ctx->u.s.string_end[0] = ctx->u.s.string_end[1];
            ctx->u.s.string_end += 1;
          } else if(ctx->u.s.string_end[0] < 0x08) {
            ctx->u.s.string_end[1] = 0x80 | (ctx->u.s.string_end[1] & 0x3F);
            ctx->u.s.string_end[0] = 0xC0 | (ctx->u.s.string_end[1] >> 6) | (ctx->u.s.string_end[0] << 2);
            ctx->u.s.string_end += 2;
          } else {
            ctx->u.s.string_end[2] = 0x80 | (ctx->u.s.string_end[1] & 0x3F);
            ctx->u.s.string_end[1] = 0x80 | ((ctx->u.s.string_end[0] & 0x0F) << 2) | (ctx->u.s.string_end[1] >> 6);
            ctx->u.s.string_end[0] = 0xE0 | (ctx->u.s.string_end[0] >> 4);
            ctx->u.s.string_end += 3;
          }
          ctx->sub_state = 0;
        } else {
          if(ctx->sub_state == 2) ctx->u.s.string_end[0] = q << 4;
          else if(ctx->sub_state == 3) ctx->u.s.string_end[0] |= q;
          else if(ctx->sub_state == 4) ctx->u.s.string_end[1] = q << 4;
          ctx->sub_state++;
        }
      }
    } else if(ctx->state == PSTATE_NUMBER) {
      if(ctx->sub_state == 0) {
        if(q == '-') {
          ctx->u.n.flags = 1;
        } else if(q >= '0' && q <= '9') {
          ctx->u.n.flags = 0;
          repeat = true;
        } else return JSON_MALFORMED_NUMBER;
        ctx->sub_state = 1;
      } else if(ctx->sub_state == 1) {
        if(q >= '0' && q <= '9') {
          ctx->u.n.number = q - '0';
          ctx->u.n.exponent = 0;
          ctx->u.n.decimals = 0;
          ctx->u.n.zero = 0;
          ctx->sub_state = q == '0' ? 3 : 2;
        } else return JSON_MALFORMED_NUMBER;
      } else if(ctx->sub_state == 2) {
        if(q >= '0' && q <= '9') {
#ifndef JSON_NO_OVERFLOW_CHECK
          if(ctx->u.n.number > (JSON_NUM_MAX / 10)) return JSON_NUMBER_OVERFLOW;
#endif
          ctx->u.n.number *= 10;
#ifndef JSON_NO_OVERFLOW_CHECK
          if(ctx->u.n.number > JSON_NUM_MAX - (q - '0')) return JSON_NUMBER_OVERFLOW;
#endif
          ctx->u.n.number += q - '0';
        } else if(q == '.') {
          ctx->sub_state = 4;
        } else if(q == 'e' || q == 'E') {
          ctx->sub_state = 6;
        } else {
          error = json_parse_grammar('N', ctx);
          ctx->state = PSTATE_ENTITY;
          repeat = true;
        }
      } else if(ctx->sub_state == 3) {
        if(q == '.') {
          ctx->sub_state = 4;
        } else if(q == 'e' || q == 'E') {
          ctx->sub_state = 6;
        } else {
          error = json_parse_grammar('N', ctx);
          ctx->state = PSTATE_ENTITY;
          repeat = true;
        }
      } else if(ctx->sub_state == 4) {
        if(q >= '0' && q <= '9') {
#ifndef JSON_SIMPLE_NUMBERS
          if(q == '0') {
#ifndef JSON_NO_OVERFLOW_CHECK
            if(ctx->u.n.zero > 0xFD) return JSON_NUMBER_OVERFLOW;
#endif
            ctx->u.n.zero++;
          } else {
#ifndef JSON_NO_OVERFLOW_CHECK
            ctx->u.n.zero++;
#else
            ctx->decimals += ++ctx->u.n.zero;
#endif              
            while(ctx->u.n.zero) {
#ifndef JSON_NO_OVERFLOW_CHECK
              if(ctx->u.n.number > (JSON_NUM_MAX / 10)) break;
              ctx->u.n.decimals++;
#endif
              ctx->u.n.number *= 10;
              ctx->u.n.zero--;
            }
            if(ctx->u.n.zero == 0 && ctx->u.n.number <= JSON_NUM_MAX - (q - '0')) ctx->u.n.number += q - '0';
          }
#endif
          ctx->sub_state = 5;
        } else return JSON_MALFORMED_NUMBER;
      } else if(ctx->sub_state == 5) {
        if(q >= '0' && q <= '9') {
#ifndef JSON_SIMPLE_NUMBERS
          if(q == '0') {
#ifndef JSON_NO_OVERFLOW_CHECK
            if(ctx->u.n.zero > 0xFD) return JSON_NUMBER_OVERFLOW;
#endif
            ctx->u.n.zero++;
          } else {
#ifndef JSON_NO_OVERFLOW_CHECK
            ctx->u.n.zero++;
#else
            ctx->decimals += ++ctx->u.n.zero;
#endif              
            while(ctx->u.n.zero) {
#ifndef JSON_NO_OVERFLOW_CHECK
              if(ctx->u.n.number > (JSON_NUM_MAX / 10)) break;
              ctx->u.n.decimals++;
#endif
              ctx->u.n.number *= 10;
              ctx->u.n.zero--;
            }
            if(ctx->u.n.zero == 0 && ctx->u.n.number <= JSON_NUM_MAX - (q - '0')) ctx->u.n.number += q - '0';
          }
#endif
          } else if(q == 'e' || q == 'E') {
          ctx->sub_state = 6;
        } else {
          error = json_parse_grammar('N', ctx);
          ctx->state = PSTATE_ENTITY;
          repeat = true;
        }
      } else if(ctx->sub_state == 6) {
        if(q >= '0' && q <= '9') {
#ifndef JSON_SIMPLE_NUMBERS
          ctx->u.n.exponent = 1;
#endif
          ctx->sub_state = 8;
        } else if(q == '+' || q == '-') {
#ifndef JSON_SIMPLE_NUMBERS
          if(q == '-') ctx->u.n.flags |= 2;
#endif
          ctx->sub_state = 7;
        } else return JSON_MALFORMED_NUMBER;
      } else if(ctx->sub_state == 7) {
        if(q >= '0' && q <= '9') {
#ifndef JSON_SIMPLE_NUMBERS
          ctx->u.n.exponent = (q - '0');
#endif
          ctx->sub_state = 8;
        } else return JSON_MALFORMED_NUMBER;
      } else if(ctx->sub_state == 8) {
        if(q >= '0' && q <= '9') {
#ifndef JSON_SIMPLE_NUMBERS
          if(ctx->u.n.exponent > (JSON_EXP_MAX / 10)) return JSON_NUMBER_OVERFLOW;
          ctx->u.n.exponent *= 10;
          if(ctx->u.n.exponent > JSON_EXP_MAX - (q - '0')) return JSON_NUMBER_OVERFLOW;
          ctx->u.n.exponent += q - '0';
#endif
        } else {
          error = json_parse_grammar('N', ctx);
          ctx->state = PSTATE_ENTITY;
          repeat = true;
        }
      }
    } else error = JSON_BAD_STATE;
  } while(repeat && !error);
  return error;
}

bool json_eof(json_parser_ctx * ctx) {
  return ctx->state == 0 && ctx->grammar_ctx.state == GSTATE_EXIT;
}

json_parser_ctx json_stream(char *buffer, uint16_t buffer_size, json_cb callback) {
  return (json_parser_ctx){callback, (uint8_t *)buffer, buffer_size, buffer_size >= 5 ? PSTATE_ENTITY : PSTATE_INVALID};
}

uint8_t json_parse(char *data, size_t length, json_cb callback) {
  size_t n;
  uint8_t error = 0;
  json_parser_ctx ctx = {callback, (uint8_t *)data};
  for(n = 0; n < length; n++, ctx.buffer++) {
    error = json_octet(&ctx, *ctx.buffer); // Process character
    if(error) return error;
  }
  error = json_octet(&ctx, ' '); // Terminate "lonely" values
  if(error) return error;
  if(json_eof(&ctx)) return JSON_OK;
  return JSON_UNEXPECTED_END;
}
