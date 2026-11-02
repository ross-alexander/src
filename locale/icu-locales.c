/* ----------------------------------------------------------------------
   --
   -- 2026-10-02: Program to get list of locales from ICU4C
   --
   -- This was triggered by PHP issues GH-8115.  This was caused
   -- by intl.default_locale being set to "en_US.UTF8" rather than
   -- just "en_US"
   --
   ---------------------------------------------------------------------- */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <unicode/ustring.h>
#include <unicode/utypes.h>
#include <unicode/uloc.h>

char* convertUCharToUTF8(const UChar* ustr, int32_t ustr_len)
{
  UErrorCode status = U_ZERO_ERROR;
  int32_t target_length = 0;
  
  // 1. Get required destination capacity
  u_strToUTF8(NULL, 0, &target_length, ustr, ustr_len, &status);
  
  if (status != U_BUFFER_OVERFLOW_ERROR && U_FAILURE(status)) {
    return NULL; // Error handling
  }
  
  // 2. Allocate target buffer (+1 for null terminator)
  char* dest = (char*)malloc(target_length + 1);
  if (!dest) return NULL;
  
  status = U_ZERO_ERROR;
  
  // 3. Perform actual conversion
  u_strToUTF8(dest, target_length + 1, &target_length, ustr, ustr_len, &status);
  
  if (U_FAILURE(status))
    {
      free(dest);
      return NULL;
    }
  dest[target_length] = '\0';
  return dest;
}

int main()
{
  int32_t count;
  UChar result[100];
  int i = 0;
  UErrorCode status = U_ZERO_ERROR;
  count = uloc_countAvailable();
  for (i = 0; i < count; i++)
    {
      //      int len = uloc_getDisplayName(uloc_getAvailable(i), "en_US", result, 100, &status);
      int len = uloc_getDisplayKeyword(uloc_getAvailable(i), "en_US", result, 100, &status);
      char *utf8 = convertUCharToUTF8(result, len);
      printf("%s\n", utf8);
      free(utf8);
    }
}
