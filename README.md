SYLT-JSON
========
DEVSOUND JSON LOW-LEVEL PARSING LIBRARY
-------------------------------------
<sup>And some other funky character conversion stuff, like Base-64 and Latin-1 to/from UTF-8</sup>

**Resource optimized for small 8-bit and 32-bit microcontrollers**

**Authors:**
* D. Taylor 2018 (gmail: senseitg)

**License:**
* Ask and thou shalt receive

**Features:**
* Parses and validates JSON
* No dynamic memory allocation
* Delivers decoded data via callback
* Handles JSON in RAM as well as streaming JSON
* Designed for UTF-8
* Fully compliant and well tested

**Options (*.h):**
* 
* Configurable max nesting depth
* Configurable standards-breaking optimizations
* Configurable data sizes for number representation

**Resource requirements:**
* Minimal memory requirements (not 32 bytes of RAM)
* Minimal stack use (non-recursive, few local variables)
* Minimal program space (compact, lean codebase)
* No heap use

**Notes:**
* Designed for high speed, low foot print - not a rich feature set.

**Caveats:**
* Does not handle UTF-16
* No UTF-8 error checking - strings delivered as is
* Requires C99 (-std=c99 for GCC)

**Performance:**
* Pretty darn good. You do the comparisons!

**WhatDoesItDo™:**

It takes JSON data in RAM:

```C
char json[] = "{\"JSON\":\"We haz it\"}";
result = json_parse(json, strlen(json), print_json);
```

Or it takes a stream of JSON data:

```C
char jsbuf[32]; // String extraction buffer
json_parser_ctx ctx = json_stream(jsbuf, sizeof(jsbuf), print_json);
while(*p_json) {
  result = json_octet(&ctx, *p_json++);
  if(result) break;
}
```

And delivers its contents to a callback function:

```C
uint8_t print_json(uint32_t depth, char * key, uint8_t type, void * value) {
  uint8_t n;
  for(n = 0; n < depth; n++) printf("  ");
  if(key) printf("\"%s\" : ", key);
  switch(type) {
    case JSON_OBJECT:     printf("{"); break;
    case JSON_OBJECT_END: printf("}"); break;
    case JSON_ARRAY:      printf("["); break;
    case JSON_ARRAY_END:  printf("]"); break;
    case JSON_STRING:     printf("\"%s\"", json_to_string(value)); break;
    case JSON_NUMBER:     printf("%0.10g", json_to_double(value)); break;
    default:              printf("%s", json_const_str(type));
  }
  printf("\n");
  return JSON_OK;
}
```

Thus converting this JSON string:

```JS
[null, -1.23000456789e+2, false, "Sing ♪ a \u266B song", {"var" : [0.0, 1.500, 2, {}]}]
```

Into this result (printed from above function):

```
[
  null
  -123.0004567
  false
  "Sing ♪ a ♫ song"
  {
    "var" : [
      0
      1.5
      2
      {
      }
    ]
  }
]
```
