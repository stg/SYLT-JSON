#include <stdint.h>
#include <stddef.h>

// UTF8/Latin-1 configuration

//#define UTF8_LOWCC_REPLACE   // Replace control characters 0x00-0x1F on Latin1 to UTF8 conversion
#define UTF8_UNAVAILABLE '?' // Replacement character for Latin1 to/from UTF8 conversion

// Base64 configuration

//#define B64_ERROR_CHECK      // Perform error checking on Base-64 decoding (slower)

// Base64 alphabet
extern const char base64lut[];

// Latin-1 to UTF8 conversion
// * Allows in place conversion (out == in)
// * Stops if output size (out_n) exceeded
size_t latin1_to_utf8(char *out, size_t out_n, char *inp, size_t inp_n);

// Return space requirements for string converted from Latin-1 to UTF8
size_t latin1_to_utf8_size(char *inp, size_t inp_n);

// UTF8 to Latin-1
// * Stops if output size (out_n) exceeded
size_t utf8_to_latin1(char *out, size_t out_n, char *inp, size_t inp_n);

// Convert octets to Base-64 (with padding for even 32-bit)
// * Stops if output size (out_n) exceeded
size_t octets_to_base64(char *out, size_t out_n, char *inp, size_t inp_n);

// Convert Base-64 to octets
// * Does not require padding
// * Allows in place conversion (out == in)
// * Stops on error, provided B64_ERROR_CHECK
// * Stops if output size (out_n) exceeded
size_t base64_to_octets(char *out, size_t out_n, char *inp, size_t inp_n);