#include "trie.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Checks if child exists by its corresponding bitfield position */
#define EXIST_CHILD(node, pos) ((node->bitfield >> pos) & 1)
/* Finds the number of children of a node */
#define NO_OF_CHILD(node) (count_set_bits(node->bitfield) - (node->bitfield & 1))
/* Gets index of child at pos in bitfield in the node's children array */
#define GET_CHILD_INDEX(node, pos) (count_set_bits(node->bitfield >> pos) - 1)


// Returns the position of the bit that represents the existence of this symbol
// in the bitfield, or -1 if the symbol is not in the alphabet we support.
int get_bit_pos(char symbol) {
  if ('A' <= symbol && symbol <= 'Z')
    return 'Z' - symbol + 1;
  else switch (symbol) {
      case '_'  : return 'Z' - 'A' + 2;
      case '.'  : return 'Z' - 'A' + 3;
      case '-'  : return 'Z' - 'A' + 4;
      case '\'' : return 'Z' - 'A' + 5;
      case ' '  : return 'Z' - 'A' + 6;
      default   : return -1;
    }
}

trie_t *trie_new() {
  trie_t *trie = calloc(1, sizeof(trie_t));
  if (!trie) {
    perror("Cannot allocate new trie");
    exit(EXIT_FAILURE);
  }
  return trie;
}

#define WORD_SIZE (32)
int count_set_bits(uint32_t n) {
  int set = 0;
  for (int i = 0; i < WORD_SIZE; i++) {
    set += n & 1;
    n >>= 1;
  }
  return set;
}

void trie_free(trie_t *root) {
  if (root) {
    int child_no = NO_OF_CHILD(root);
    for (int i = 0; i < child_no; i++) {
      trie_free(root->children[i]);
    }
    free(root->children);
  }
  free(root);
}

bool trie_get(const trie_t *root, const char *key, int *value_out) {
  if (!root || !key) return false;
  // Reach end of key, check if value is set
  if (*key == '\0') {
    if (root->bitfield & 1) {
      *value_out = root->value;
      return true;
    }
    return false;
  }
  // Key has more characters
  // check for character validity and position in bitfield
  int pos = get_bit_pos(*key);
  if (pos == -1) {
    return false;
  }
  // if corresponding child node exists, recursive check into child 
  if (EXIST_CHILD(root, pos)) {
    int index = GET_CHILD_INDEX(root, pos);
    return trie_get(root->children[index], key + 1, value_out);
  }
  // child cannot be found
  return false;
}

bool trie_insert(trie_t *root, const char *key, int value) {
  if (!root) root = trie_new();
  // End of key - set current node
  if (*key == '\0') {
    root->bitfield |= 1; // mark node as set
    root->value = value; // set value of node
    return true;
  }
  // More characters
  int pos = get_bit_pos(*key);
  // check for validity of character
  if (pos == -1) return false;
  // char valid, insert into children
  int index = GET_CHILD_INDEX(root, pos);
  if (!EXIST_CHILD(root, pos)) {
    // child doesn't exist, resize children array
    root->bitfield |= (1 << pos);
    int child_no = NO_OF_CHILD(root);
    size_t new_size = sizeof(trie_t *) * child_no;
    root->children = realloc(root->children, new_size);
    index = GET_CHILD_INDEX(root, pos);
    // shift back to accommodate for new child at correct index
    trie_t **move_start = &(root->children[index]);
    size_t move_size = sizeof(trie_t **) * (child_no - index - 1);
    memmove(move_start + 1, move_start, move_size); 
    // assign new child
    root->children[index] = trie_new();   
  }
  // Recursive insert
  return trie_insert(root->children[index], key + 1, value);
}

#ifdef TRIE_MAIN

// TODO: DO NOT MODIFY TIHS FUNCTION!!! Remember to run:
//  valgrind --leak-check=full --show-leak-kinds=all ./trie

int main(void) {
  #define SIZE (8)
  char ks[SIZE][10] = { "GOOD", "",   "W",  "-_ _-", "123", "*",   "()",  "+}{" };
  bool bs[SIZE] =     { true,   true, true, true,    false, false, false, false };
  int vs[SIZE] =      { 12,     0,    -1,   2342,    999,   0,     2,     4 };

  printf("Mapping strings to ints.\n");
  trie_t *root = trie_new();
  for (int i = 0; i < SIZE; ++i) {
    char *status = bs[i] ? "(insert should succeed)" : "(insert should fail)";
    if (trie_insert(root, ks[i], vs[i])) {
      printf("Was able to insert %s with value %d %s\n", ks[i], vs[i], status);
    } else {
      printf("Unable to insert %s with value %d %s\n", ks[i], vs[i], status);
    }
  }
  for (int i = SIZE - 1; i >= 0; --i) {
    char *status = bs[i] ? "(get should succeed)" : "(get should fail)";
    int v;
    if (trie_get(root, ks[i], &v)) {
      printf("Was able to get %s -> %d %s\n", ks[i], v, status);
    } else {
      printf("Unable to get %s %s\n", ks[i], status);
    }
  }
  trie_free(root);
}

#endif
