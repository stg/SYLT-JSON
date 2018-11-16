#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <unistd.h> 
#include <dirent.h> 
#include "json.h"
#include "helpers.h"

// This example lists runs test cases from https://github.com/nst/JSONTestSuite
// Files are located in ./test_case/ together with the LICENSE file for test cases and this example

int main() {
  struct dirent *de;
  DIR *dr = opendir("unit_test");
  if(!dr) return 1;
  while((de = readdir(dr))) {
    char path[256];
    if(strlen(de->d_name) > 5) {
      sprintf(path, "unit_test/%s", de->d_name);
      size_t size;
      if(strlen(de->d_name) > 5 && !strcmp(".json", &de->d_name[strlen(de->d_name) - 5])) {
        char *json = file_to_string(path, &size);
        if(json) {
          uint8_t result = json_parse(json, size, NULL, NULL);
          //if(de->d_name[0] == 'i' || (de->d_name[0] == 'n' && result == 0) || (de->d_name[0] == 'y' && result != 0)) {
            printf("File: %s\n", de->d_name);
            printf("Result: %s\n\n", result_to_string(result));
          //}
          free(json);
        }
      }
    }
  }
  closedir(dr);

  return 0;   
}
