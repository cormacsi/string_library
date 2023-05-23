#include "s21_string.h"

char *s21_strstr(const char *haystack, const char *needle) {
  char *res = S21_NULL;
  int res_ind = 0;
  int found = 0;
  int len = s21_strlen(haystack);
  int lned = s21_strlen(needle);
  for (int i = 0; i < len && lned && !found; i++) {
    if (haystack[i] == needle[0]) {
      int i2 = i, n2 = 0;
      res_ind = i;
      while (needle[n2] && haystack[i2]) {
        if (haystack[i2] == needle[n2]) {
          i2++;
          n2++;
        } else {
          break;
        }
      }
      if (!needle[n2]) found = 1;
    }
  }
  if (found) res = (char *)haystack + res_ind;
  if (!lned) res = (char *)haystack;
  return res;
}
