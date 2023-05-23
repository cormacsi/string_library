#include "s21_sscanf.h"

int s21_sscanf(const char* str, const char* format, ...) {
  char *copy_str = (char*)str, *copy_format = (char*)format;
  int count = 0;
  va_list data;
  if (empty(copy_str) && !empty(copy_format)) {
    count = -1;
  } else {
    int res = 1;
    va_start(data, format);
    while (check(&copy_str, &copy_format, res)) {
      struct spec pattern = parser(&copy_format, SSCANF);
      res = proc_specifier(str, &copy_str, &data, pattern);
      count += res && pattern.specifier != '%' && pattern.specifier != 'n' &&
               pattern.width != WIDTH_ASTERISK;
    }
    va_end(data);
  }
  return count;
}

int empty(char* str) {
  while (*str && (*str == '\n' || *str == '\t' || *str == ' ')) str++;
  return !(*str);
}

int check(char** str, char** format, int prev) {
  int q = 1, res = 1;
  if (!prev) res = 0;
  while (q && res) {
    while (**format &&
           (**format == ' ' || **format == '\t' || **format == '\n')) {
      (*format)++;
      while (**str && (**str == ' ' || **str == '\t' || **str == '\n'))
        (*str)++;
    }
    q = (**str == **format) && (**format != '%') && *str && *format;
    if (**format) {
      if (**format != '%') {
        (*format)++;
        if (**str) (*str)++;
      }
    } else {
      res = 0;
    }
  }
  if (**format != '%') {
    res = 0;
  } else {
    (*format)++;
    res = 1;
  }
  return res;
}

struct spec parser(char** format, int mode) {
  struct spec pattern = {0, 0, -1, -1, {0, 0, 0, 0, 0}};
  char c;
  while ((c = **format) && mode &&
         ((c == '-') || (c == '+') || (c == '#') || (c == ' ') || (c == '0'))) {
    pattern.flag.minus = pattern.flag.minus || (c == '-');
    pattern.flag.plus = pattern.flag.plus || (c == '+');
    pattern.flag.hash = pattern.flag.hash || (c == '#');
    pattern.flag.space = pattern.flag.space || (c == ' ');
    pattern.flag.zero = pattern.flag.zero || (c == '0');
    (*format)++;
  }

  if (c == '*') {
    pattern.width = WIDTH_ASTERISK;
    (*format)++;
  } else {
    while ((c = **format) && c >= '0' && c <= '9') {
      pattern.width = pattern.width * 10 + c - '0';
      (*format)++;
    }
  }

  if ((c = **format) == '.' && mode) {
    (*format)++;
    c = **format;
    if (c == '*') {
      pattern.precision = PRECISION_ASTERISK;
      (*format)++;
    } else {
      pattern.precision = 0;
      while ((c = **format) && c >= '0' && c <= '9') {
        pattern.precision = pattern.precision * 10 + c - '0';
        (*format)++;
      }
    }
  }

  while ((c = **format) &&
         ((c == 'h') || (c == 'l') || (c == 'L') || (c == '*'))) {
    if (c == 'h') pattern.len = pattern.len > -1 ? -1 : -2;
    if (c == 'l')
      pattern.len =
          pattern.len >= 0 && pattern.len < 2 ? pattern.len + 1 : pattern.len;
    if (c == 'L')
      pattern.len = pattern.len >= 0 && pattern.len < 2 ? 2 : pattern.len;
    if (c == '*' && !mode) pattern.width = WIDTH_ASTERISK;
    (*format)++;
  }

  if (c) {
    pattern.specifier = c;
    (*format)++;
  } else {
    pattern.specifier = 0;
  }

  return pattern;
}

int proc_specifier(const char* orig, char** str, va_list* data, struct spec q) {
  int res = 0, sp = q.specifier;
  if (sp != 'c' && sp != 's' && sp != '%' && sp != 'n') space(str);
  res = sp == 'c' ? read_c(str, data, q) : res;
  res = sp == 'd' ? read_d(str, data, q) : res;
  res = sp == 'i' ? read_i(str, data, q) : res;
  res = sp == 'G' || sp == 'e' || sp == 'E' || sp == 'f' || sp == 'g'
            ? read_real(str, data, q)
            : res;
  res = sp == 'o' ? read_oux(str, data, q, 8) : res;
  res = sp == 's' ? read_s(str, data, q) : res;
  res = sp == 'u' ? read_oux(str, data, q, 10) : res;
  res = sp == 'X' || sp == 'x' ? read_oux(str, data, q, 16) : res;
  res = sp == 'p' ? read_p(str, data, q) : res;
  res = sp == 'n' ? read_n(orig, *str, data, q) : res;
  res = sp == '%' ? read_proc(str) : res;
  // в каких-то случаях sscanf возвращает -1
  // printf("%d\n---\n", res);
  return res;
}

int read_c(char** str, va_list* data, struct spec q) {
  char* c;
  c = q.width == WIDTH_ASTERISK ? malloc(sizeof(char)) : va_arg(*data, char*);
  *c = **str;
  (*str)++;

  if (q.width == WIDTH_ASTERISK) free(c);
  return 1;
}

int read_d(char** str, va_list* data, struct spec q) {
  int sign = sign_num(**str), res = 0;
  if (sign && (q.width != 1)) {
    (*str)++;
    q.width = q.width > 0 ? q.width - 1 : q.width;
  }
  if (!sign) sign = 1;

  if (cipher16(**str) < 10) {
    spec_odux(str, data, q, 10, sign);
    res = 1;
  }
  return res;
}

int read_i(char** str, va_list* data, struct spec q) {
  int base = 10, sign = sign_num(**str), res = 0;
  if (sign && (q.width != 1)) {
    (*str)++;
    q.width = q.width > 0 ? q.width - 1 : q.width;
  }
  if (!sign) sign = 1;
  if (q.width != 2 && q.width != 1 && **str == '0' &&
      (*(*str + 1) == 'x' || *(*str + 1) == 'X')) {
    base = 16;
    *str += 2;
    q.width = q.width > 0 ? q.width - 2 : q.width;
  }
  if (**str == '0' && q.width != 1) {
    base = 8;
  }
  if (cipher16(**str) < base) {
    spec_odux(str, data, q, base, sign);
    res = 1;
  }
  return res;
}

int read_oux(char** str, va_list* data, struct spec q, int base) {
  int sign = sign_num(**str), res = 0;
  if (sign && (q.width != 1)) {
    (*str)++;
    q.width = q.width > 0 ? q.width - 1 : q.width;
  }
  if (!sign) sign = 1;

  if (base == 16 && **str == '0' &&
      (*(*str + 1) == 'x' || *(*str + 1) == 'X') && q.width != 2 &&
      q.width != 1 && cipher16(*(*str + 2)) < 16) {
    *str += 2;
    q.width = q.width > 0 ? q.width - 2 : q.width;
  }

  if (cipher16(**str) < base) {
    spec_odux(str, data, q, base, sign);
    res = 1;
  }
  return res;
}

void spec_odux(char** str, va_list* data, struct spec q, int base, int sign) {
  int i, overflow = 0;
  long long buf = 0;
  for (i = 0; cipher16(**str) < base && ((i < q.width) || q.width <= 0); ++i) {
    long long prev_buf = buf;
    buf = buf * base + cipher16(**str);
    overflow = overflow || prev_buf > buf;
    (*str)++;
  }
  if (q.width != WIDTH_ASTERISK) {
    if (q.len == -2)
      *((signed char*)va_arg(*data, signed char*)) =
          overflow ? (sign == 1 ? -1 : 0) : buf * sign;
    if (q.len == -1)
      *((short*)va_arg(*data, short*)) =
          overflow ? (sign == 1 ? -1 : 0) : buf * sign;
    if (q.len == 0)
      *((int*)va_arg(*data, int*)) =
          overflow ? (sign == 1 ? -1 : 0) : buf * sign;
    if (q.len == 1)
      *((long*)va_arg(*data, long*)) =
          overflow ? (sign == 1 ? LLONG_MAX : LLONG_MIN) : buf * sign;
    if (q.len == 2)
      *((long long*)va_arg(*data, long long*)) =
          overflow ? (sign == 1 ? LLONG_MAX : LLONG_MIN) : buf * sign;
  }
}

int read_real(char** str, va_list* data, struct spec q) {
  int sign = sign_num(**str), res = 0;
  long double num = 0;
  if (sign && (q.width != 1)) {
    (*str)++;
    q.width = q.width > 0 ? q.width - 1 : q.width;
  }
  if (!sign) sign = 1;
  if (inf_check(*str)) {
    num = sign / 0.0;
    *str = *str + 3;
    res = 1;
  }
  if (nan_check(*str)) {
    num = S21_NAN;
    *str = *str + 3;
    res = 1;
  }

  if (!res && (cipher16(**str) < 10 || **str == '.')) {
    num = spec_real(str, q, sign);
    res = 1;
  }
  if (res && q.width != WIDTH_ASTERISK) {
    void* ans;
    if (q.len <= 0) *((float*)(ans = va_arg(*data, float*))) = num;
    if (q.len == 1) *((double*)(ans = va_arg(*data, double*))) = num;
    if (q.len == 2) *((long double*)(ans = va_arg(*data, long double*))) = num;
  }
  return res;
}

long double spec_real(char** str, struct spec q, int sign) {
  char *exp, *mantissa = *str;
  long double num = 0;
  int i = 0, e = 0;

  while (cipher16(*mantissa) < 10 && (i < q.width || q.width <= 0)) {
    mantissa++;
    i++;
  }
  if (*mantissa == '.' && (i < q.width || q.width <= 0)) {
    mantissa++;
    i++;
    while (cipher16(*mantissa) < 10 && (i < q.width || q.width <= 0)) {
      mantissa++;
      i++;
    }
  }
  if ((*mantissa == 'e' || *mantissa == 'E') &&
      (i + 1 < q.width || q.width <= 0) && cipher16(*(mantissa + 1) < 10)) {
    exp = mantissa + 1;
    int signe = sign_num(*exp);
    if (!signe) signe = 1;
    i++;
    if ((*exp == '+' || *exp == '-')) {
      if ((i + 1 < q.width || q.width <= 0) && cipher16(*(exp + 1)) < 10) {
        i++;
        exp++;
      }
    }
    for (; cipher16(*exp) < 10 && ((i < q.width) || q.width <= 0); ++i) {
      e = e * 10 + cipher16(*exp) * signe;
      exp++;
    }
  } else {
    exp = mantissa;
  }
  while (*str < mantissa && **str != '.') {
    num = num * 10 + cipher16(**str) * sign * pow_10(e);
    (*str)++;
  }
  if (**str == '.' && *str < exp) {
    int k = 1;
    (*str)++;
    while (cipher16(**str) < 10 && *str < exp) {
      num = num + cipher16(**str) * sign * pow_10(e - k++);
      (*str)++;
      i++;
    }
  }
  *str = exp;
  return num;
}

int read_s(char** str, va_list* data, struct spec q) {
  char c;
  int res = 1;
  while ((c = **str) && (c == ' ' || c == '\t' || c == '\n')) (*str)++;
  if (!c) {
    res = 0;
  } else {
    int i;
    char* s = S21_NULL;
    if (q.width != WIDTH_ASTERISK) s = va_arg(*data, char*);
    for (i = 0; (c = **str) && c != ' ' && c != '\n' && c != '\t' &&
                ((i < q.width) || q.width <= 0);
         ++i) {
      if (q.width != WIDTH_ASTERISK) {
        *s = c;
        s++;
      }
      (*str)++;
    }
    *s = '\0';
  }
  return res;
}

int read_p(char** str, va_list* data, struct spec q) {
  q.len = 2;
  return read_oux(str, data, q, 16);
}

int read_proc(char** str) {
  int res = 0;
  if (**str == '%') {
    (*str)++;
    res = 1;
  }
  return res;
}

int read_n(const char* begin, const char* end, va_list* data, struct spec q) {
  if (q.width != WIDTH_ASTERISK) {
    if (q.len == -2) *((signed char*)va_arg(*data, signed char*)) = end - begin;
    if (q.len == -1) *((short*)va_arg(*data, short*)) = end - begin;
    if (q.len == -0) *((int*)va_arg(*data, int*)) = end - begin;
    if (q.len == 1) *((long*)va_arg(*data, long*)) = end - begin;
    if (q.len == 2) *((long long*)va_arg(*data, long long*)) = end - begin;
  }
  return end - begin != 0;
}

int sign_num(char c) {
  int res = 0;
  if (c == '-') res = -1;
  if (c == '+') res = 1;
  return res;
}

int cipher16(char c) {
  int res = 16;
  if (c >= '0' && c <= '9') res = c - '0';
  if (c >= 'A' && c <= 'F') res = c - 'A' + 10;
  if (c >= 'a' && c <= 'f') res = c - 'a' + 10;
  return res;
}

long double pow_10(int n) {
  long double f = 1;
  int i;
  if (n > 0) {
    for (i = 0; i < n; ++i) f *= 10;
  } else {
    for (i = 0; i > n; --i) f /= 10;
  }
  return f;
}

void space(char** str) {
  while (**str && (**str == ' ' || **str == '\t' || **str == '\n')) (*str)++;
}

int inf_check(const char* str) {
  int res = 0;
  if ((str[0] == 'i' || str[0] == 'I') && (str[1] == 'n' || str[1] == 'N') &&
      (str[2] == 'f' || str[2] == 'F'))
    res = 1;
  return res;
}

int nan_check(const char* str) {
  int res = 0;
  if ((str[0] == 'n' || str[0] == 'N') && (str[1] == 'a' || str[1] == 'A') &&
      (str[2] == 'n' || str[2] == 'N'))
    res = 1;
  return res;
}
