#include <stdint.h>
#include <stdbool.h>

// JSON optimizations (advanced use only: makes assumptions about contents and breaks standard)
//#define JSON_NO_OVERFLOW_CHECK     // Disable number overflow checks 
//#define JSON_SIMPLE_NUMBERS        // Disable fractions and exponents for numbers

// JSON nesting depth
#define JSON_NESTING              8 // Max 64k

// JSON number configuration
#define JSON_NUM_TYPE      uint32_t
#define JSON_NUM_MAX     0xFFFFFFFF
#define JSON_EXP_TYPE       uint8_t
#define JSON_EXP_MAX           0xFF

// JSON error codes
#define JSON_OK                   0
#define JSON_MALFORMED_ESCAPE     1 // Invalid character found in string escape sequence
#define JSON_MALFORMED_NUMBER     2 // Number does not conform to JSON standard
#define JSON_MALFORMED_STRING     3 // String does not conform to JSON standard (forbidden characters)
#define JSON_UNEXPECTED_CHARACTER 4 // Forbidden character in structural parts of text
#define JSON_BAD_GRAMMAR          5 // Structure of document does not conform to JSON standard
#define JSON_TRAILING_DATA        6 // Unexpected data encountered after end of document
#define JSON_UNEXPECTED_END       7 // Unexpected end of document encountered
#define JSON_BAD_CONSTANT         8 // Invalid constant encountered (only true, false and null allowed)
#define JSON_TOO_DEEP             9 // Nesting depth exceede (JSON_NESTING)
#define JSON_NUMBER_OVERFLOW     10 // One or more parts of a number exceeded set limits (JSON number configuration)
#define JSON_STRING_OVERFLOW     11 // String length exceeded (JSON_MAX_STRING)
#define JSON_BAD_STATE           12 // Programming error lead to bad state
#define JSON_CUSTOM_ERROR       128 // Custom errors from callback (128-255)

// JSON object types
#define JSON_OBJECT               0
#define JSON_OBJECT_END           1
#define JSON_ARRAY                2
#define JSON_ARRAY_END            3
#define JSON_KEY                  4
#define JSON_STRING               5
#define JSON_NUMBER               6
#define JSON_CONSTANT             7
#define JSON_NULL                 8
#define JSON_TRUE                 9
#define JSON_FALSE               10

// Memory compare function (AVR f.ex. requires special procedure for comparing to below ROM strings)
#define JSON_MEMCMP(RAM, ROM, LENGTH) memcmp(RAM, ROM, LENGTH)

extern const char json_str_null[];
extern const char json_str_true[];
extern const char json_str_false[];

// JSON parsing callback
typedef uint8_t (*json_cb)(uint32_t depth, uint8_t type, void * value);

// JSON number representation
typedef struct {
  JSON_NUM_TYPE number;
  JSON_EXP_TYPE exponent;
  uint8_t      zero;
  uint8_t      decimals;
  uint8_t      flags;
} json_number;

typedef struct {
  uint8_t * string;
  uint8_t * string_end;
} json_string;

typedef struct {
  uint8_t state;
  uint8_t stack[(JSON_NESTING + 7) / 8];
  uint16_t stack_depth;
} json_grammar_ctx;

typedef struct {
  json_cb callback;
  uint8_t *buffer;
  uint16_t buffer_size;
  uint8_t state;
  uint8_t sub_state;
  union {
    json_string s;
    json_number n;
  } u;
  json_grammar_ctx grammar_ctx;
} json_parser_ctx;

// JSON type (constants only) to string
const char * json_const_str(uint8_t type);

// JSON value to double
double json_to_double(void *value);

// JSON value to string
char *json_to_string(void *value);
  
// JSON end-of-file reached
bool json_eof(json_parser_ctx * ctx);

// JSON parse single character
uint8_t json_octet(json_parser_ctx * ctx, uint8_t q);

// JSON return context for streaming
json_parser_ctx json_stream(char *buffer, uint16_t buffer_size, json_cb callback);

// JSON in-place parser
uint8_t json_parse(char *data, size_t length, json_cb callback);