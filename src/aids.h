#ifndef AIDS
#define AIDS

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define STRING "%.*s"
#define FMTSTRING(s) (int)(s)->len, (s)->data

#define BOOL "%s"
#define FMTBOOL(s) (s? "true": "false")

#define auto __auto_type

//must be first in a struct
#define IMPL_ALLOCATOR\
    fun_alloc alloc;\
    fun_calloc calloc;\
    fun_realloc realloc;\
    fun_free free;\

typedef struct Allocator Allocator;
typedef void*(*fun_alloc)(Allocator*, size_t);
typedef void*(*fun_realloc)(Allocator*, void*, size_t);
typedef void*(*fun_calloc)(Allocator*, size_t, size_t);
typedef void(*fun_free)(Allocator*, void*);

struct Allocator {
    IMPL_ALLOCATOR;
};

extern Allocator DEFAULT_ALLOCATOR;

#define ALLOC(allocator, size) (allocator)->alloc((Allocator*)(allocator), size)
#define CALLOC(alloc, nmemb, size) (alloc)->calloc((Allocator*)(alloc), nmemb, size)
#define REALLOC(alloc, ptr, size) (alloc)->realloc((Allocator*)(alloc), ptr, size)
#define FREE(alloc, ptr) (alloc)->free((Allocator*)(alloc), ptr)

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

#define panic(...)\
    do {\
        fprintf(stderr, __VA_ARGS__);\
        exit(1);\
    }while(0)

#define assert(c, ...)\
    do {\
        if(!(c)) {\
            panic(__VA_ARGS__);\
        }\
    } while(0)

//doesn't matter where in the struct it is
#define IMPL_DA(type)\
    Allocator* alloc;\
size_t len;\
size_t cap;\
type* data;

#define DA(type)\
    struct {\
        IMPL_DA(type);\
    }


#define da_init(da, allocator)\
    do {\
        (da)->alloc = (Allocator*)allocator;\
        (da)->cap = 0;\
        (da)->len = 0;\
        (da)->data = NULL;\
    } while(0)

#define da_init_if_not(da, allocator)\
    do {\
        if(!(da)->alloc) {\
            da_init(da, allocator);\
        }\
    } while(0)

#define da_reserve(da, size)\
    do {\
        assert((da)->alloc, "DA not initialized\n");\
        if((size)>(da)->cap) {\
            (da)->cap = size;\
            (da)->data = REALLOC((da)->alloc, (da)->data, sizeof(*(da)->data) * (da)->cap);\
            assert((da)->data != NULL, "But more RAM LOL\n");\
        }\
    } while(0)

#define da_append(da, el)\
    do {\
        while((da)->len >= (da)->cap) {\
            da_reserve((da), (da)->cap? (da)->cap*2: DA_INIT_CAP);\
        }\
        (da)->data[(da)->len++] = el;\
    } while(0)

#define da_free(da)\
    do{\
        FREE((da)->alloc, (da)->data);\
        (da)->data = NULL;\
        (da)->len = 0;\
        (da)->cap = 0;\
    } while(0)

#define da_foreach(it, da) for (typeof(*(da)->data)* it = (da)->data; it < (da)->data + (da)->len; ++it)
#define da_foreach_enum(idx, it, da) for(size_t idx = 0; idx < (da)->len; idx++)\
                                                      for(typeof(*(da)->data)* it = &(da)->data[idx]; it != NULL; it = NULL)

#define fa_len(fa) sizeof(fa)/sizeof(*fa)
#define fa_foreach(it, fa) for (typeof(*(fa))* it = (fa); it < (fa) + fa_len(fa); ++it)
#define fa_foreach_len(it, fa, size) for (typeof(*(fa))* it = (fa); it < (fa) + size; ++it)
#define fa_foreach_enum(idx, it, fa) for(size_t idx = 0; idx < fa_len(fa); idx++)\
                                                      for(typeof(*(fa))* it = &(fa)[idx]; it != NULL; it = NULL)
#define fa_foreach_enum_len(idx, it, fa, size) for(size_t idx = 0; idx < size; idx++)\
                                                                for(typeof(*(fa))* it = &(fa)[idx]; it != NULL; it = NULL)

#define da_append_da(da, from)\
    do{\
        da_foreach(v, from) {\
            da_append(da, *v);\
        }\
    } while(0)

#define da_append_fa(da, from)\
    do{\
        fa_foreach(v, from) {\
            da_append(da, *v);\
        }\
    } while(0)

#define da_clone(da, allocator)\
    ({\
        typeof(*da) n = {0};\
        da_init(&n, allocator);\
        da_append_da(&n, da);\
        n;\
    })

#define da_pop(da) ((da)->data[--(da)->len])

#define da_get_last(da) ((da)->data[(da)->len-1])

#define da_set_last(da, v) ((da)->data[(da)->len-1]) = v

#define da_get(da, i) (da)->data[i]
#define da_set(da, i, v) (da)->data[i] = v

#define da_free_call(da, c)\
    do{\
        da_foreach(b, da) c\
        FREE((da)->alloc, (da)->data);\
        (da)->data = NULL;\
        (da)->len = 0;\
        (da)->cap = 0;\
    } while(0)

#define da_remove_unordered(da, idx)\
    ({\
     size_t _i = (idx);\
     assert(_i < (da)->len, "Index out of bounds\n");\
     typeof(*(da)->data) a = da_get_last(da);\
     typeof(*(da)->data) b = da_get(da, _i);\
     da_set_last(da, b);\
     da_set(da, _i, a);\
     da_pop(da);\
     })

//TASK(20260217-105205-686-n6-024): add more things to the da

typedef struct {
    IMPL_ALLOCATOR;
    Allocator* allocator;
    DA(void*) pointers;
} TrackingAllocator;

void tracking_destroy(TrackingAllocator*);
TrackingAllocator tracking_create(Allocator*);
//TASK(20260217-112915-392-n6-805): add more allocators

typedef DA(char) String;

bool string_compare(String, String);
bool str_cmp(const char*, const char*);
String string_from(const char*, Allocator*);
String string_clone(String*, Allocator*);
String string_create(Allocator*);
char* from_string(String*, Allocator*);
void append_char(String*, char);
void append_cstr(String*, char*);
void append_string(String*, String*);

#ifndef HM_SIZE
#define HM_SIZE 103 
#endif

#define KV(ktype, vtype)\
    struct {\
        ktype key;\
        vtype value;\
    }

#define IMPL_HM(ktype, vtype)\
    Allocator* alloc;\
size_t(*hash)(ktype);\
DA(KV(ktype, vtype)) buckets[HM_SIZE];

#define HM(ktype, vtype)\
    struct {\
        IMPL_HM(ktype, vtype);\
    }

#ifndef HASHES
#define HASHES
#endif

#define _hm_hash(x) _Generic((x), _HASH_CASES HASHES)
#define _HASH_CASES \
    const char*: hm_hash_cstr,\
char*: (size_t(*)(char*))hm_hash_cstr,\
String: hm_hash_string

#define _DUMMY_KEY(hm) ((typeof((hm)->buckets[0].data->key)){0})
size_t hm_hash_cstr(const char*);
size_t hm_hash_int(int);
size_t hm_hash_string(String);

#define hm_init(hm, allocator)\
    do {\
        (hm)->alloc = (Allocator*)allocator;\
        (hm)->hash = _hm_hash(_DUMMY_KEY(hm));\
    } while(0)

#ifndef COMPARE
#define COMPARE
#endif

#define _CALL(c, a, b) _Generic((c),\
        void*: ((bool (*)(typeof(a), typeof(b))) c)(a, b),\
        int: a == b/*NOLINT*/\
        )

#define _EQUALS(a, b) _CALL(_Generic((a),\
            const char*: (void*) str_cmp,\
            String: (void*) string_compare,\
            COMPARE\
            default: 0\
            ), (a), (b))

#define hm_set(hm, _nkey, _nvalue)\
    do {\
        typeof(_nkey) nkey = _nkey;\
        typeof(_nvalue) nvalue = _nvalue;\
        bool done = false;\
        size_t index = (hm)->hash(nkey)%HM_SIZE;\
        da_init_if_not(&(hm)->buckets[index], (hm)->alloc);\
        da_foreach(i, &((hm)->buckets[index])) {\
            if(_EQUALS(i->key, nkey)){\
                i->value = nvalue;\
                done = true;\
                break;\
            }\
        }\
        if(!done) {\
            da_append(&(hm)->buckets[index], ((typeof(*(hm)->buckets[index].data)) {\
                        .key=nkey,\
                        .value=nvalue,\
                        }));\
        }\
    } while(0)

#define hm_get(hm, _nkey)\
    ({\
     typeof(_nkey) nkey = _nkey;\
     typeof(((hm)->buckets[0].data->value))* _res = NULL;\
     size_t index = (hm)->hash(nkey)%HM_SIZE;\
     da_init_if_not(&(hm)->buckets[index], (hm)->alloc);\
     da_foreach(i, &((hm)->buckets[index])) {\
     if(_EQUALS(i->key, nkey)){\
     _res = &i->value;\
     break;\
     }\
     }\
     _res;\
     })

#define hm_has(hm, key) (hm_get(hm, key) != NULL)

#define hm_free(hm)\
    do {\
        for(size_t i = 0; i < HM_SIZE; i++) {\
            da_free(&(hm)->buckets[i]);\
        }\
    } while(0)

#define hm_free_call(hm, c)\
    do {\
        for(size_t i = 0; i < HM_SIZE; i++) {\
            da_free_call(&(hm)->buckets[i], c);\
        }\
    } while(0)

#define hm_remove(hm, nkey)\
    ({\
     typeof(nkey) _nkey = nkey;\
     size_t index = (hm)->hash(_nkey)%HM_SIZE;\
     typeof(*((hm)->buckets[0].data)) _res = {0};\
     for(size_t i = 0; i < (hm)->buckets[index].len; i++){\
     if(_EQUALS((hm)->buckets[index].data[i].key, _nkey)){\
     _res=da_remove_unordered(&(hm)->buckets[index], i);\
     break;\
     }\
     }\
     _res;\
     })

#define hm_keys(hm, allocator)\
    ({\
     DA(typeof(((hm)->buckets[0].data->key))) _res = {0};\
     da_init(&_res, allocator);\
     for(size_t i = 0; i < HM_SIZE; i++){\
        da_init_if_not(&(hm)->buckets[i], (hm)->alloc);\
        da_foreach(v, &(hm)->buckets[i]) {\
            da_append(&_res, v->key);\
        }\
     }\
     _res;\
     })

#define hm_values(hm, allocator)\
    ({\
     DA(typeof(((hm)->buckets[0].data->value))) _res = {0};\
     da_init(&_res, allocator);\
     for(size_t i = 0; i < HM_SIZE; i++){\
        da_init_if_not(&(hm)->buckets[i], (hm)->alloc);\
        da_foreach(v, &(hm)->buckets[i]) {\
            da_append(&_res, v->value);\
        }\
     }\
     _res;\
     })

#define hm_entries(hm, allocator)\
    ({\
     DA(typeof(*((hm)->buckets[0].data))) _res = {0};\
     da_init(&_res, allocator);\
     for(size_t i = 0; i < HM_SIZE; i++){\
        da_init_if_not(&(hm)->buckets[i], (hm)->alloc);\
        da_foreach(v, &(hm)->buckets[i]) {\
            da_append(&_res, *v);\
        }\
     }\
     _res;\
     })
//TASK(20260218-083256-729-n6-649): add more functions for hashmaps

//WARNIGN requires `-fms-extensions` to function
#define BaseClass(name, feilds)\
    typedef struct name name;\
    struct name {\
        struct feilds;\
    };

#define Class(name, parent, feilds)\
    typedef struct name name;\
    struct name {\
        parent;\
        struct feilds;\
    };

#define constructor(name, ...)\
    void name##_new (name* self, ##__VA_ARGS__)

#define new_stack(T, ...) ({\
    T var;\
    T##_new(&var, ##__VA_ARGS__);\
    var;\
})
#define new_heap(alloc, T, ...) ({\
    T* var = ALLOC(alloc, sizeof(T));\
    T##_new(var, ##__VA_ARGS__);\
    var;\
})

#define SUPER(T, ...) \
    T##_new((T*) self, ##__VA_ARGS__);

#define METHOD(R, name, ...) \
    R(*name)(__VA_ARGS__)



#endif

#ifdef AIDS_IMPLEMENTATION
#endif
