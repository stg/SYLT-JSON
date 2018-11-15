#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "json.h"
#include "helpers.h"

int main() {
  char json[] = "[null,-1.23000456789e+2,false,\"Sing ♪ a \\u266B song\",{\"var\":[0.0,1.500,2,{}]}]";
  uint8_t result;
  
  printf("Output:\n");
  
  // Decode JSON in-place (will modify string)
  result = json_parse(json, strlen(json), print_json);
  
  printf("\nCompletion status: %s\n\n", result_to_string(result));
}