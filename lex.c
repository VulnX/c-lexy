#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *content;
  size_t len;
} Source;
Source SOURCE;

typedef struct token {
  char *str;
  struct token *next;
} Token;
Token *TOKENS_HEAD = NULL;

const char *KEYWORDS[] = {
    "auto",   "double", "int",    "struct",   "break",   "else",     "long",
    "switch", "case",   "enum",   "register", "typedef", "char",     "extern",
    "return", "union",  "const",  "float",    "short",   "unsigned", "continue",
    "for",    "signed", "void",   "default",  "goto",    "sizeof",   "volatile",
    "do",     "if",     "static", "while"};
const int KEYWORDS_LEN = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);
const char SEPARATORS[] = {' ', ',', ';', '{', '}', '(',
                           ')', '[', ']', ':', '#', '\n'};
const int SEPARATORS_LEN = sizeof(SEPARATORS) / sizeof(SEPARATORS[0]);

void cleanup(int status);
void strip_strncpy(char *dst, char *src, unsigned int len);
void append_token(unsigned int fd, unsigned int bk);
void extract_tokens(void);
void display_tokens(void);
void analyze(void);
void read_file(FILE *fp);

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

void display_tokens(void) {
  Token *token;
  token = TOKENS_HEAD;
  puts("Tokens:");
  while (token) {
    printf("%s\n", token->str);
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
  display_tokens();
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
  cleanup(EXIT_SUCCESS);
}
