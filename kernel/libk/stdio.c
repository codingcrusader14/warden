#include "stdio.h"
#include "../../drivers/qemu/pl011.h"
#include "stdarg.h"
#include "string.h"
#include <stdbool.h>
#include <limits.h>
#include <stdarg.h>

int kprintf(const char *format, ...) {
  va_list ap;
  va_start(ap, format);
  while (*format != '\0') {
    char character = *format;

    switch (character) {
      case '%':
        format++;
        switch (*format) {
          case 'd': // int
          case 'i': { // int
            if (*format != '\0' && (*format == 'd' || *format == 'i')) { 
              int num = va_arg(ap, int);
              char num_buffer[12];
              bool negative = false;
              int i = 0;

              if (num == 0) { // empty case
                put_char('0');
                break;
              } 
              
              if (num < 0) { // negative case
                negative = true;
                num_buffer[i++] = '-';
              }

              unsigned int unum = (negative) ? -(unsigned int)num : (unsigned int)num;

              while (unum) {
                char digit = unum % 10;
                num_buffer[i++] = digit + '0';
                unum /= 10;
              }

              num_buffer[i] = '\0';
              int buffer_len = i - 1;
              int l = (negative) ? 1 : 0, r = buffer_len;
              while (l <= r) {
                char temp = num_buffer[l];
                num_buffer[l] = num_buffer[r];
                num_buffer[r] = temp;
                l++;
                r--;
              }

              send_message(num_buffer);
            }
            break;
          }
          case 's': { // string
            const char* str = va_arg(ap, char*);
            send_message(str);
            break;
          }
          case 'x': // hex
            break;

          case 'p': // pointer
            break;

          case '%': // display % symbol
            put_char('%');
            break;

          default: // format not specified
            put_char('%');
            put_char(*format);
            break;
        } 
        format++;
        break;

      default: // plain characters
        put_char(character);
        format++;
        break;
    }
  }
  va_end(ap);
  return 0;
}
