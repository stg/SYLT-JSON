#include <stdlib.h>

// Callback to print parsed JSON to screen
uint8_t print_json(uint32_t depth, uint8_t type, void * value) {
  uint8_t n;
  static bool was_key = false;
  if(!was_key) for(n = 0; n < depth; n++) printf("  ");
  was_key = false;  
  switch(type) {
    case JSON_OBJECT:     printf("{\n"); break;
    case JSON_OBJECT_END: printf("}\n"); break;
    case JSON_ARRAY:      printf("[\n"); break;
    case JSON_ARRAY_END:  printf("]\n"); break;
    case JSON_KEY:        printf("\"%s\" : ", json_to_string(value)); was_key = true; break;
    case JSON_STRING:     printf("\"%s\"\n", json_to_string(value)); break;
    case JSON_NUMBER:     printf("%0.10g\n", json_to_double(value)); break;
    default:              printf("%s\n", json_const_str(type));
  }
  return JSON_OK;
}

// Convert result to readable string
char * result_to_string(uint8_t result) {
  switch(result) {
    case JSON_OK                  : return "OK";
    case JSON_MALFORMED_ESCAPE    : return "MALFORMED ESCAPE";
    case JSON_MALFORMED_NUMBER    : return "MALFORMED NUMBER";
    case JSON_MALFORMED_STRING    : return "MALFORMED STRING";
    case JSON_UNEXPECTED_CHARACTER: return "UNEXPECTED CHARACTER";
    case JSON_BAD_GRAMMAR         : return "BAD GRAMMAR";
    case JSON_TRAILING_DATA       : return "TRAILING DATA";
    case JSON_UNEXPECTED_END      : return "UNEXPECTED END";
    case JSON_BAD_CONSTANT        : return "BAD CONSTANT";
    case JSON_TOO_DEEP            : return "TOO DEEP";
    case JSON_NUMBER_OVERFLOW     : return "NUMBER OVERFLOW";
    case JSON_STRING_OVERFLOW     : return "STRING OVERFLOW";
  }
  return "UNKNOWN RESULT";
}

// Allocate and read entire file into memory, zero-terminate
void *file_to_string(char *filename, size_t *size) {
  void *data;
  FILE *f;
  f = fopen(filename, "rb");
  if(f) {
    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    fseek(f, 0, 0);
    data = malloc(*size + 1);
    if(data) fread(data, 1, *size, f);
    ((char*)data)[*size] = 0x00;
    fclose(f);
  }
  return data;
}
