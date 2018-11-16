#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "json.h"
#include "helpers.h"

int main() {
  char json[] = "[null,-1.23000456789e+2,false,\"Sing ♪ a \\u266B song\",{\"var\":[0.0,1.500,2,{}]}]";
  uint8_t result;
  char *p_json = json;
  char jsbuf[32]; // Decoding buffer (minimum size is 5)
  
  printf("Output:\n");
    
  // Prepare context for streaming
  json_parser_ctx ctx = json_stream(jsbuf, sizeof(jsbuf), print_json, NULL);
  
  // Stream data to json decoder
  while(*p_json) {
    result = json_octet(&ctx, *p_json++);
    if(result) break;
  }
  
  printf("\nCompletion status: %s\n\n", result_to_string(result));
}