#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {
  char *content;
  size_t len;
} Source;
Source SOURCE;

enum TokenType { KEYWORD = 1, OPERATOR, CONSTANT };

typedef struct token {
  char *str;
  enum TokenType type;
  struct token *next;
} Token;
Token *TOKENS_HEAD = NULL;

#define ARR_LEN(arr) (sizeof((arr)) / sizeof((arr[0])))

const char *KEYWORDS[] = {
    "auto",   "double", "int",    "struct",   "break",   "else",     "long",
    "switch", "case",   "enum",   "register", "typedef", "char",     "extern",
    "return", "union",  "const",  "float",    "short",   "unsigned", "continue",
    "for",    "signed", "void",   "default",  "goto",    "sizeof",   "volatile",
    "do",     "if",     "static", "while"};
const int KEYWORDS_LEN = ARR_LEN(KEYWORDS);
const char SEPARATORS[] = {' ', ',', ';', '{', '}', '(',
                           ')', '[', ']', ':', '#', '\n'};
const int SEPARATORS_LEN = ARR_LEN(SEPARATORS);
const char *OPERATORS[] = {
    "+",  "-",  "*",      "/",  "%",  "==", "!=", ">",   "<",   ">=", "<=",
    "&&", "||", "!",      "&",  "|",  "^",  "~",  "<<",  ">>",  "=",  "+=",
    "-=", "*=", "/=",     "%=", "&=", "|=", "^=", "<<=", ">>=", "++", "--",
    "?",  ":",  "sizeof", ",",  "->", "##", "#",  "::",
};
const int OPERATORS_LEN = ARR_LEN(OPERATORS);

void cleanup(int status);
void strip_strncpy(char *dst, char *src, unsigned int len);
void append_token(unsigned int fd, unsigned int bk);
void extract_tokens(void);
bool is_keyword(char *s);
bool is_operator(char *s);
bool is_constant(char *s);
void categorize_tokens(void);
void analyze(void);
void read_file(FILE *fp);
void print_tokens(void);

void cleanup(int status) {
  if (SOURCE.content) {
    free(SOURCE.content);
    SOURCE.content = NULL;
  }
  exit(status);
}

void strip_strncpy(char *dst, char *src, unsigned int len) {
  unsigned int src_idx, dst_idx;
  char curr;
  bool skip;

  for (src_idx = dst_idx = 0; src_idx < len; src_idx++) {
    curr = src[src_idx];
    skip = false;
    for (int i = 0; i < SEPARATORS_LEN; i++) {
      if (curr == SEPARATORS[i]) {
        skip = true;
        break;
      }
    }
    if (skip)
      continue;
    dst[dst_idx++] = curr;
  }
  dst[dst_idx] = '\0';
}

void append_token(unsigned int fd, unsigned int bk) {
  unsigned int token_len;
  char *token_str;
  bool can_proceed;
  Token *token;

  token_len = fd - bk + 1;
  if (token_len == 0) {
    // ensure @ bk index we have token
    can_proceed = false;
    for (int i = 0; i < SEPARATORS_LEN; i++) {
      if (SOURCE.content[bk] == SEPARATORS[i]) {
        can_proceed = true;
      }
    }
    if (!can_proceed)
      return;
  }
  token_str = calloc((token_len + 1), sizeof(char));
  if (token_str == NULL) {
    fprintf(stderr, "Failed to allocate memory for token str\n");
    cleanup(EXIT_FAILURE);
  }
  strip_strncpy(token_str, &SOURCE.content[bk], token_len);
  token_str[token_len] = '\0';
  if (strlen(token_str) == 0) {
    free(token_str);
    return;
  }
  token = calloc(1, sizeof(Token));
  if (token == NULL) {
    fprintf(stderr, "Failed to allocate memory for token\n");
    free(token_str);
    cleanup(EXIT_FAILURE);
  }
  token->str = token_str;
  token->next = TOKENS_HEAD;
  TOKENS_HEAD = token;
}

void extract_tokens(void) {
  unsigned int fd, bk;
  char curr;
  bool curr_is_separator;

  fd = bk = 0;
  curr_is_separator = false;
  while (bk != SOURCE.len - 1) {
    curr = SOURCE.content[fd];
    // if curr is one of separators
    for (int i = 0; i < SEPARATORS_LEN; i++) {
      if (curr == SEPARATORS[i]) {
        curr_is_separator = true;
        break;
      }
    }
    if (curr_is_separator) {
      // todo
      curr_is_separator = false; // for next iteration
      append_token(fd, bk);
      bk = fd;
      fd++;
    } else {
      fd++;
    }
  }
}

bool is_keyword(char *s) {
  bool res;
  res = false;
  for (int i = 0; i < KEYWORDS_LEN; i++) {
    if (strcmp(s, KEYWORDS[i]) == 0) {
      res = true;
      break;
    }
  }
  return res;
}

bool is_operator(char *s) {
  bool res;
  res = false;
  for (int i = 0; i < OPERATORS_LEN; i++) {
    if (strcmp(s, OPERATORS[i]) == 0) {
      res = true;
      break;
    }
  }
  return res;
}

bool is_constant(char *s) {
  bool res;
  res = false;
  if (isdigit(s[0])) // numeric
    res = true;
  if (s[0] == '\'') // char
    res = true;
  if (s[0] == '"') // string
    res = true;
  return res;
}

void categorize_tokens(void) {
  Token *token;
  token = TOKENS_HEAD;
  while (token) {
    if (is_keyword(token->str))
      token->type = KEYWORD;
    if (is_operator(token->str))
      token->type = OPERATOR;
    if (is_constant(token->str))
      token->type = CONSTANT;
    token = token->next;
  }
}

void analyze(void) {
  puts("");
  for (int i = 0; i < 80; i++) {
    printf("=");
  }
  puts("");
  printf("Analyzing:\n");
  extract_tokens();
  categorize_tokens();
  for (int i = 0; i < 80; i++) {
    printf("=");
  }
  puts("");
}

void read_file(FILE *fp) {
  fseek(fp, 0, SEEK_END);
  SOURCE.len = (size_t)ftell(fp);
  fseek(fp, 0, SEEK_SET);
  SOURCE.content = calloc(SOURCE.len, sizeof(char));
  if (SOURCE.content == NULL) {
    fprintf(stderr, "Failed to allocate memory to read file\n");
    fclose(fp);
    exit(EXIT_FAILURE);
  }
  fread(SOURCE.content, sizeof(char), SOURCE.len, fp);
}

void print_tokens(void) {
  Token *token;
  int off;
  int spaces;
  off = 30;
  token = TOKENS_HEAD;
  while (token) {
    printf("[TOKEN] \"%s\" ", token->str);
    spaces = off - ((int)strlen(token->str) + 11);
    while (0 < spaces--)
      putchar(' ');
    printf("<-> type: ");
    switch (token->type) {
    case KEYWORD:
      puts("KEYWORD");
      break;
    case OPERATOR:
      puts("OPERATOR");
      break;
    case CONSTANT:
      puts("CONSTANT");
      break;
    default:
      puts("UNCATEGORIZED/LITERALS");
      break;
    }
    token = token->next;
  }
}

int main(int argc, char **argv) {
  char *file_name;
  FILE *fp;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file>\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  file_name = argv[1];
  fp = fopen(file_name, "r");
  if (fp == NULL) {
    fprintf(stderr, "Invalid file! Cannot open\n");
    exit(EXIT_FAILURE);
  }
  read_file(fp);
  fclose(fp);
  analyze();
  print_tokens();
  cleanup(EXIT_SUCCESS);
}
