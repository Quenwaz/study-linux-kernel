#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_KEY_LEN 128
#define MAX_VALUE_LEN 128
#define HASH_SIZE 1024

typedef struct hash_node {
    char key[MAX_KEY_LEN];
    union
    {
        char strval[MAX_VALUE_LEN];
        void* anyval;
    }value;
    
    struct hash_node *next;
} hash_node_t;

typedef struct {
    hash_node_t *buckets[HASH_SIZE];
} hash_table_t;

typedef enum{
    FALSE,
    TRUE
}Boolean;

unsigned int hash_func(const char *key);

extern hash_node_t *hash_node_create(const char *key, void *value);

extern void hash_table_init(hash_table_t *table);

extern void hash_table_put(hash_table_t *table, const char *key, void *value);

extern void *hash_table_get(hash_table_t *table, const char *key);

extern void* hash_table_begin(hash_table_t *table);

extern Boolean hash_table_end(hash_table_t *table, void* context);

extern void hash_table_next(hash_table_t *table, void* context);

extern void* hash_table_current_value(hash_table_t *table, void* context, char key[MAX_KEY_LEN]);
