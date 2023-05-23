#ifndef S21_SSCANF_H
#define S21_SSCANF_H

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "s21_string.h"

#define SPRINTF 1
#define SSCANF 0
#define WIDTH_ASTERISK -1
#define WIDTH_NONE 0
#define PRECISION_ASTERISK -2
#define PRECISION_NONE -1

struct flags {
  int plus;
  int minus;
  int space;
  int hash;
  int zero;
};

struct spec {
  int len;
  int width;
  int precision;
  int specifier;
  struct flags flag;
};

int s21_sscanf(const char* str, const char* format, ...);

int check(char** str, char** format, int res);
struct spec parser(char** format, int mode);
int proc_specifier(const char* orig, char** str, va_list* data, struct spec q);

int read_c(char** str, va_list* data, struct spec q);
int read_d(char** str, va_list* data, struct spec q);
int read_i(char** str, va_list* data, struct spec q);
int read_oux(char** str, va_list* data, struct spec q, int base);
void spec_d(char** str, va_list* data, struct spec q, int base, int sign);
void spec_odux(char** str, va_list* data, struct spec q, int base, int sign);
int read_real(char** str, va_list* data, struct spec q);
long double spec_real(char** str, struct spec q, int sign);
int read_s(char** str, va_list* data, struct spec q);
int read_p(char** str, va_list* data, struct spec q);
int read_proc(char** str);
int read_n(const char* begin, const char* end, va_list* data, struct spec q);
int empty(char* copy_str);

int sign_num(char c);
int cipher16(char c);
long double pow_10(int n);
void space(char** str);
int inf_check(const char* str);
int nan_check(const char* str);
#endif
