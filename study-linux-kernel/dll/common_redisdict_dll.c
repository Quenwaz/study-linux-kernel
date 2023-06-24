#ifndef __COMMON_REDISDICT_DLL_INCLUDED__
#define __COMMON_REDISDICT_DLL_INCLUDED__



typedef struct dictEntry{
    // key
    void * key;

    // value
    union 
    {
        void * val;
        uint64_t u64;
        int64_t s64;
    }v;

    // link to next
    struct dictEntry* next;
}dictEntry;


typedef struct dictht{
    // hast table array
    dictEntry ** table;

    // hash table size
    unsigned long size;

    // mask
    unsigned long sizemask;

    // 
    unsigned long used;
}dictht;


typedef struct dictType{
    unsigned int (*hashFunction)(const void* key);
    void *(*keyDup)(void * privdata, const void* key);
    void *(*valDup)(void* privdata, const void* obj);
    int (*keyCompare)(void *privdata, const void* key1, const void* key2);
    void (*keyDestructor)(void *privdata, void *key);
    void (*valDestructor)(void *privdata, void *obj);
}dictType;


typedef struct dict{
    dictType *type;
    void* privdata;
    dictht ht[2];
    int rehashidx;
}dict;


#endif // __COMMON_REDISDICT_DLL_INCLUDED__