#include <stdio.h>
#include "latin1_utf8_b64.h"

void main() {
  char str2[256], str1[256];
  
  // Base-64
  strcpy(str1, "Hello Base-64");
  str2[octets_to_base64(str2, sizeof(str2), str1, strlen(str1))] = 0;
  str1[base64_to_octets(str1, sizeof(str1), str2, strlen(str2))] = 0;
  printf("%s => %s\n\n", str1, str2);
  
  // Latin-1/UTF-8
  strcpy(str1, " ¡¢£¤¥¦§¨©ª«¬ ®¯\n°±²³´µ¶·¸¹º»¼½¾¿\nÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏ\nÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß\nàáâãäåæçèéêëìíîï\nðñòóôõö÷øùúûüýþÿ\n");
  str2[latin1_to_utf8(str2, sizeof(str2), str1, strlen(str1))] = 0;
  printf("Latin-1 codes 0x80-0xFF as UTF-8: (on windows: chcp 65001)\n%s\n", str2);
  str1[utf8_to_latin1(str1, sizeof(str1), str2, strlen(str2))] = 0;
  printf("Latin-1 codes 0x80-0xFF as Latin-1: (on windows: chcp 1252)\n%s\n", str1);
  
}
