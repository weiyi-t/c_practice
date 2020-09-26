#include "sonnets.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Checks that files were opened correctly.
#define FILECHECK(ptr, msg) if (!(ptr)) perror(msg), exit(EXIT_FAILURE)

// Returns a pointer to the last word in line, or NULL if there are no words.
// This function adds a sentinel '\0' after the word.
char *lastwordtok(char *line) {
  assert(line);
  int pos = -1;
  for (int i = 0; line[i];  ++i) if (isalpha(line[i])) pos = i;
  if (pos == -1) return NULL;
  line[pos + 1] = '\0';
  while(pos >= 0 && line[--pos] != ' ');
  while(!isalpha(line[++pos]));
  return line + pos;
}

/* Start of my solution */
char *uppercase(char *str) {
  for (int i = 0; str[i] != '\0'; i++) {
    str[i] = toupper(str[i]);
  }
  return str;
}

#define NUM_OF_VOWELS (5)
bool is_phoneme_vowel(const char *str) {
  if (*(str - 1) == ' ') {
    char vowels[] = {'A', 'E', 'I', 'O', 'U'};
    for (int i = 0; i < NUM_OF_VOWELS; i++) {
      if (*str == vowels[i]) return true;
    }
  }
  return false;
}

const char *strrhyme(const char *phonemes) {
  const char *curr = phonemes + strlen(phonemes);
  for (; curr != phonemes; curr--) {
    if (is_phoneme_vowel(curr)) return curr;
  }
  return curr;
}

#define MAX_DICT_LINE_LENGTH 120

// Assume the given file is formatted correctly and formed only of characters
// that are supported by the alphabet from PART A.
dict_t *load_rhyme_mappings_from_file(const char *phonetic_dict_filename) {
  dict_t *word_to_rhyme = dict_new();
  dict_t *rhyme_to_id = dict_new();
  int unique_id = 0;
  FILE *fp = fopen(phonetic_dict_filename, "r");
  FILECHECK(fp, "Phonetic dictionary file cannot be opened");
  char line[MAX_DICT_LINE_LENGTH];
  while (fgets(line, MAX_DICT_LINE_LENGTH, fp)) {
    const char *word = strtok(line, " ");
    const char *rhyme = strrhyme(strtok(NULL, "\n"));
    int rhyme_id;
    if (!dict_get(rhyme_to_id, rhyme, &rhyme_id)) {
      dict_insert(rhyme_to_id, rhyme, unique_id);
      rhyme_id = unique_id++;
    }
    dict_insert(word_to_rhyme, word, rhyme_id);
  }
  fclose(fp);
  dict_free(rhyme_to_id);
  return word_to_rhyme;
}

#define MAX_SONNET_LINE_LENGTH 80

bool next_rhyme_scheme(FILE *sonnets_file,
                       const dict_t *rhyme_mappings, char *out) {
  char line[MAX_SONNET_LINE_LENGTH];
  bool parsing_sonnet = false;

  int sonnet_line = 0;
  intcharmap_t *id_to_rhyme = intcharmap_new();
  char new_pattern = 'A';

  while (fgets(line, MAX_SONNET_LINE_LENGTH, sonnets_file)) {
    char *last_word = lastwordtok(line);
    if (!last_word) {
      if (parsing_sonnet) break;
      continue;
    }
    parsing_sonnet = true;

    last_word = uppercase(last_word);
    int rhyme_id;
    char pattern;
    if (dict_get(rhyme_mappings, last_word, &rhyme_id)) {
      if (!intcharmap_get(id_to_rhyme, rhyme_id, &pattern)) {
        pattern = new_pattern++;
        intcharmap_insert(id_to_rhyme, rhyme_id, pattern);
      }
      out[sonnet_line++] = pattern;
    } else {
      fprintf(stderr, "Cannot find rhyme mapping for %s\n", last_word);
      out[sonnet_line++] = new_pattern++;
    } 
  }
  intcharmap_free(id_to_rhyme);
  return parsing_sonnet;
}

#define MAX_NUM_SONNET_LINES 20

void most_common_rhyme_scheme(FILE *sonnets_file,
                              const dict_t *rhyme_mappings, char *out) {
  dict_t *rhyme_frequency = dict_new();
  strcpy(out, "N/A");

  // char rhyme_scheme[MAX_NUM_SONNET_LINES];
  char *rhyme_scheme = calloc(MAX_NUM_SONNET_LINES, sizeof(char));
  int max_frequency = 0;

  while (next_rhyme_scheme(sonnets_file, rhyme_mappings, rhyme_scheme)) {
    int frequency = 0;
    dict_get(rhyme_frequency, rhyme_scheme, &frequency);
    dict_insert(rhyme_frequency, rhyme_scheme, ++frequency);
    if (frequency > max_frequency) {
      strcpy(out, rhyme_scheme);
      max_frequency = frequency;
    }
  }

  free(rhyme_scheme);
  dict_free(rhyme_frequency);

}
/* End of my solution */

#ifdef SONNETS_MAIN

#define PHONETIC_DICT_FILE "dictionary.txt"

/* TODO: DO NOT MODIFY THIS FUNCTION!!! Remember to run:
 *  valgrind --leak-check=full --show-leak-kinds=all ./sonnets_map or
 *  valgrind --leak-check=full --show-leak-kinds=all ./sonnets_trie if your
 *  PART A is correct and want to see how much faster it is
 */
int main (void) {
  dict_t *rhyme_mappings = load_rhyme_mappings_from_file(PHONETIC_DICT_FILE);

  char *sonnets_files[3] = {"shakespeare.txt", "spenser.txt", "petrarch.txt"};
  for (int i = 0; i < 3; ++i) {
    FILE *f = fopen(sonnets_files[i], "r");
    FILECHECK(f, sonnets_files[i]);

    char rhyme_scheme[MAX_NUM_SONNET_LINES];
    most_common_rhyme_scheme(f, rhyme_mappings, rhyme_scheme);
    printf("The most common rhyme scheme of sonnets from %s is: %s\n",
           sonnets_files[i], rhyme_scheme);
    fclose(f);
  }

  dict_free(rhyme_mappings);
  return EXIT_SUCCESS;
}

#endif
