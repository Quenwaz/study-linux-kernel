#include "common_hashtable_dll.h"


typedef struct hash_table_context{
    __intptr_t bucket_index;
    hash_node_t* current_node;
}hash_table_context;

unsigned int hash_func(const char *key) {
    unsigned int hash = 0;
    const char *p = key;
    while (*p) {
        hash = hash * 31 + *p;
        p++;
    }
    return hash % HASH_SIZE;
}


hash_node_t *hash_node_create(const char *key, void *value) {
    hash_node_t *node = (hash_node_t *)malloc(sizeof(hash_node_t));
    if (node != NULL) {
        strcpy(node->key, key);
        node->value.anyval = value;
        // strcpy(node->value.strval, value);
        node->next = NULL;
    }
    return node;
}


void hash_table_init(hash_table_t *table) {
    int i;
    for (i = 0; i < HASH_SIZE; i++) {
        table->buckets[i] = NULL;
    }
}


void hash_table_put(hash_table_t *table, const char *key, void *value) {
    unsigned int hash = hash_func(key);
    hash_node_t *node = table->buckets[hash];
    if (node == NULL) {
        table->buckets[hash] = hash_node_create(key, value);
    } else {
        while (node->next != NULL) {
            if (strcmp(node->key, key) == 0) {
                node->value.anyval = value;
                // strcpy(node->value.strval, value);
                return;
            }
            node = node->next;
        }
        if (strcmp(node->key, key) == 0) {
            node->value.anyval = value;
            // strcpy(node->value.strval, value);
        } else {
            node->next = hash_node_create(key, value);
        }
    }
}

void *hash_table_get(hash_table_t *table, const char *key) {
    unsigned int hash = hash_func(key);
    hash_node_t *node = table->buckets[hash];
    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node->value.anyval;
        }
        node = node->next;
    }
    return NULL;
}

__intptr_t hash_table_next_node(hash_table_t *table, size_t start_node_idx)
{
    if (start_node_idx >= HASH_SIZE){
        return -1;
    }

    for(size_t i = start_node_idx; i < HASH_SIZE; ++i){
        if (table->buckets[i] != NULL){
            return i;
        }
    }
    return -1;
}

void* hash_table_begin(hash_table_t *table)
{
    hash_table_context* context = (hash_table_context*)malloc(sizeof(hash_table_context));
    context->bucket_index = hash_table_next_node(table, 0);
    if (context->bucket_index != -1)
        context->current_node = table->buckets[context->bucket_index];
    else 
        context->current_node = NULL;
    return context;
}

Boolean hash_table_end(hash_table_t *table, void* _context)
{
    hash_table_context* context = (hash_table_context*)_context;
    if (context->current_node == NULL){
        free(_context);
        return TRUE;
    }
    return FALSE;
}

void hash_table_next(hash_table_t *table, void* _context)
{
    hash_table_context* context = (hash_table_context*)_context;
    if(context->current_node->next == NULL){
        context->bucket_index = hash_table_next_node(table,context->bucket_index + 1);
        if (context->bucket_index == -1 || 
            context->bucket_index >= HASH_SIZE){
            context->current_node = NULL;
        }
        else 
            context->current_node = table->buckets[context->bucket_index];
    }else{
        context->current_node = context->current_node->next;
    }
}

void* hash_table_current_value(hash_table_t *table, void* _context, char key[MAX_KEY_LEN])
{
    hash_table_context* context = (hash_table_context*)_context;
    strncpy(key,context->current_node->key,MAX_KEY_LEN);
    return context->current_node->value.anyval;
}