#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  if (argc < 2) {
    int nl = 0, words = 0, characters = 0;
    int n;
    int in_word = 0;
    char buffer[512];

    while ((n = read(stdin, buffer, sizeof(buffer))) > 0) {
        for (int i = 0; i < n; i++) {
            characters++;
            if (buffer[i] == '\n') nl++;
            if (isspace(buffer[i])) {
                in_word = 0;
            } else if (!in_word) {
                in_word = 1;
                words++;
            }
        }
    }
    printf("newlines: %d   words: %d  characters: %d\n", nl, words, characters);
    exit(0);
  }

  int total_nl = 0, total_words = 0, total_characters = 0;
  char buffer[512];

  for (int i = 1; i < argc; ++i) {
    int fd = open(argv[i], 0);
    if (fd < 0) {
      printf("Could not open file\n");
      exit(1);
    }
    int nl = 0, words = 0, characters = 0;
    int n;
    bool in = false, out = true;

    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
      int i = 0;

      while (i < n) {
        while (i < n && buffer[i] == ' '){
          i++;
          characters++;
          if (in) {
            in = false;
            out = true;
            words++;
          }
        }

        if (i < n && buffer[i] == '\n') {
          nl++;
        } 

        i++;
        characters++;
        if (out) {
          in = true;
          out = false;
        }
      }

      if (in) {
        out = true;
        in = false;
        words++;
      }
      close(fd);
    }
    
    printf("newlines: %d   words: %d  characters: %d  %s\n", nl, words, characters, argv[i]);
    total_nl += nl; 
    total_words += words;
    total_characters += characters;
  }

  if (argc > 2) {
    printf("newlines: %d   words: %d  characters: %d  %s\n", total_nl, total_words, total_characters, "total");
  }

  exit(0);
}