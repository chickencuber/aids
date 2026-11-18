// this is temporary
#pragma GCC system_header
#ifndef AIDS
#define AIDS

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#define IMPORT __declspec(dllimport)
#else
#define EXPORT
#define IMPORT
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned __int128 u128;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef __int128 i128;

#define INT128_MAX ((__int128)(((__uint128_t)1 << 127) - 1))
#define INT128_MIN (-INT128_MAX - 1)
#define UINT128_MAX ((__uint128_t) - 1)

#define STRING "%.*s"
#define FMTSTRING(s) (int)(s)->len, (s)->data

#define BOOL "%s"
#define FMTBOOL(s) (s ? "true" : "false")

// must be first in a struct
#define IMPL_ALLOCATOR                                                         \
    fun_alloc alloc;                                                           \
    fun_calloc calloc;                                                         \
    fun_realloc realloc;                                                       \
    fun_free free;

typedef struct Allocator Allocator;
typedef void* (*fun_alloc)(Allocator*, size_t);
typedef void* (*fun_realloc)(Allocator*, void*, size_t);
typedef void* (*fun_calloc)(Allocator*, size_t, size_t);
typedef void (*fun_free)(Allocator*, void*);

struct Allocator {
    IMPL_ALLOCATOR;
};

IMPORT extern Allocator DEFAULT_ALLOCATOR;

#define ALLOC(allocator, size) (allocator)->alloc((Allocator*)(allocator), size)
#define CALLOC(alloc, nmemb, size)                                             \
    (alloc)->calloc((Allocator*)(alloc), nmemb, size)
#define REALLOC(alloc, ptr, size)                                              \
    (alloc)->realloc((Allocator*)(alloc), ptr, size)
#define FREE(alloc, ptr) (alloc)->free((Allocator*)(alloc), ptr)

#ifndef DA_INIT_CAP
#define DA_INIT_CAP 256
#endif

#define panic(...)                                                             \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        exit(1);                                                               \
    } while (0)

#define unreachable() panic("unreachable at %s:%d", __FILE__, __LINE__)

#define assert(c, ...)                                                         \
    do {                                                                       \
        if (!(c))                                                              \
            panic(__VA_ARGS__);                                                \
    } while (0)

#define IMPL_SLICE(type)                                                       \
    type* data;                                                                \
    size_t len;

#define SLICE(type)                                                            \
    struct {                                                                   \
        IMPL_SLICE(type);                                                      \
    }

#define Option(t) const t*
#define Some(v)                                                                \
    ({                                                                         \
        static auto __var__ = v;                                               \
        &__var__;                                                              \
    })
#define SomeT(t, v)                                                            \
    ({                                                                         \
        static t __var__ = (v);                                                \
        &__var__;                                                              \
    })
#define None NULL

/* ------------------------------------------------------------------------- */
/* Slices                                                                    */
/* ------------------------------------------------------------------------- */

#define slice_len(s) ((s).len)
#define slice_empty(s) ((s).len == 0)

#define slice_sub(s, start, count)                                             \
    ({                                                                         \
        typeof(s) _s = (s);                                                    \
        size_t _start = (start);                                               \
        size_t _count = (count);                                               \
        assert(_start <= _s.len && _count <= _s.len - _start,                  \
               "Slice out of bounds\n");                                       \
        typeof(_s) _res = {                                                    \
            .data = _s.data ? _s.data + _start : NULL,                         \
            .len = _count,                                                     \
        };                                                                     \
        _res;                                                                  \
    })

#define slice_from(s, start) slice_sub(s, start, (s).len - (start))

#define slice_to(s, end) slice_sub(s, 0, end)

#define slice_get(s, i)                                                        \
    ({                                                                         \
        typeof(s) _s = (s);                                                    \
        size_t _i = (i);                                                       \
        assert(_i < _s.len, "Slice index out of bounds\n");                    \
        _s.data[_i];                                                           \
    })

#define slice_first(s) slice_get(s, 0)
#define slice_last(s) slice_get(s, (s).len - 1)

#define slice_foreach(it, s)                                                   \
    for (typeof(*(s).data)* it = (s).data; it < (s).data + (s).len; ++it)

#define slice_foreach_enum(idx, it, s)                                         \
    for (size_t idx = 0; idx < (s).len; idx++)                                 \
        for (typeof(*(s).data)* it = &(s).data[idx]; it != NULL; it = NULL)

/* ------------------------------------------------------------------------- */
/* Dynamic arrays                                                            */
/* ------------------------------------------------------------------------- */

#define IMPL_DA(type)                                                          \
    IMPL_SLICE(type)                                                           \
    size_t cap;                                                                \
    Allocator* alloc;

#define DA(type)                                                               \
    struct {                                                                   \
        IMPL_DA(type);                                                         \
    }

#define da_init(da, allocator)                                                 \
    do {                                                                       \
        (da)->alloc = (Allocator*)allocator;                                   \
        (da)->cap = 0;                                                         \
        (da)->len = 0;                                                         \
        (da)->data = NULL;                                                     \
    } while (0)

#define da_init_if_not(da, allocator)                                          \
    do {                                                                       \
        if (!(da)->alloc)                                                      \
            da_init(da, allocator);                                            \
    } while (0)

#define da_reserve(da, size)                                                   \
    do {                                                                       \
        assert((da)->alloc, "DA not initialized\n");                           \
        if ((size) > (da)->cap) {                                              \
            (da)->data = REALLOC((da)->alloc, (da)->data,                      \
                                 sizeof(*(da)->data) * (size));                \
            assert((da)->data != NULL, "But more RAM LOL\n");                  \
            (da)->cap = (size);                                                \
        }                                                                      \
    } while (0)

#define da_append(da, el)                                                      \
    do {                                                                       \
        while ((da)->len >= (da)->cap)                                         \
            da_reserve((da), (da)->cap ? (da)->cap * 2 : DA_INIT_CAP);         \
        (da)->data[(da)->len++] = (el);                                        \
    } while (0)

#define da_free(da)                                                            \
    do {                                                                       \
        if ((da)->alloc) {                                                     \
            FREE((da)->alloc, (da)->data);                                     \
            (da)->data = NULL;                                                 \
            (da)->len = 0;                                                     \
            (da)->cap = 0;                                                     \
        }                                                                      \
    } while (0)

#define da_free_call(da, b, c)                                                 \
    do {                                                                       \
        if ((da)->alloc) {                                                     \
            da_foreach(b, da) c FREE((da)->alloc, (da)->data);                 \
            (da)->data = NULL;                                                 \
            (da)->len = 0;                                                     \
            (da)->cap = 0;                                                     \
        }                                                                      \
    } while (0)

#define da_foreach(it, da)                                                     \
    for (typeof(*(da)->data)* it = (da)->data; it < (da)->data + (da)->len;    \
         ++it)

#define da_foreach_enum(idx, it, da)                                           \
    for (size_t idx = 0; idx < (da)->len; idx++)                               \
        for (typeof(*(da)->data)* it = &(da)->data[idx]; it != NULL; it = NULL)

#define da_sub(da, start, count)                                               \
    ({                                                                         \
        typeof(da) _da = (da);                                                 \
        size_t _start = (start);                                               \
        size_t _count = (count);                                               \
        assert(_start <= _da->len && _count <= _da->len - _start,              \
               "DA out of bounds\n");                                          \
        SLICE(typeof(*_da->data))                                              \
        _res = {                                                               \
            .data = _da->data ? _da->data + _start : NULL,                     \
            .len = _count,                                                     \
        };                                                                     \
        _res;                                                                  \
    })

#define da_from(da, start) da_sub(da, start, (da)->len - (start))

#define da_to(da, end) da_sub(da, 0, end)

#define da_get(da, i) ((da)->data[i])
#define da_first(da) ((da)->data[0])
#define da_last(da) ((da)->data[(da)->len - 1])

#define da_slice(da)                                                           \
    ((SLICE(typeof(*(da)->data)){.data = (da)->data, .len = (da)->len}))

#define string_slice(da) ((Str){.data = (da)->data, .len = (da)->len})

#define da_append_da(da, from)                                                 \
    do {                                                                       \
        da_foreach(v, from) da_append(da, *v);                                 \
    } while (0)

#define da_append_fa(da, from)                                                 \
    do {                                                                       \
        fa_foreach(v, from) da_append(da, *v);                                 \
    } while (0)

#define da_append_many(da, ...)                                                \
    do {                                                                       \
        typeof(*(da)->data) _items[] = {__VA_ARGS__};                          \
        size_t _count = sizeof(_items) / sizeof(*_items);                      \
        da_reserve(da, (da)->len + _count);                                    \
        memcpy((da)->data + (da)->len, _items, sizeof(_items));                \
        (da)->len += _count;                                                   \
    } while (0)

#define da_clone(da, allocator)                                                \
    ({                                                                         \
        typeof(*da) n = {0};                                                   \
        da_init(&n, allocator);                                                \
        da_append_da(&n, da);                                                  \
        n;                                                                     \
    })

#define da_pop(da) ((da)->data[--(da)->len])
#define da_get_last(da) ((da)->data[(da)->len - 1])
#define da_set_last(da, v) ((da)->data[(da)->len - 1]) = v
#define da_set(da, i, v) (da)->data[i] = v

#define da_remove_unordered(da, idx)                                           \
    ({                                                                         \
        size_t _i = (idx);                                                     \
        assert(_i < (da)->len, "Index out of bounds\n");                       \
        typeof(*(da)->data) a = da_get_last(da);                               \
        typeof(*(da)->data) b = da_get(da, _i);                                \
        da_set_last(da, b);                                                    \
        da_set(da, _i, a);                                                     \
        da_pop(da);                                                            \
    })

#define da_remove_ordered(arr, index)                                          \
    ({                                                                         \
        assert((index) < (arr)->len, "Index out of bounds");                   \
        typeof((arr)->data[0]) _removed = (arr)->data[index];                  \
        for (size_t _i = (index); _i + 1 < (arr)->len; _i++)                   \
            (arr)->data[_i] = (arr)->data[_i + 1];                             \
        (arr)->len--;                                                          \
        _removed;                                                              \
    })

// TASK(20260217-105205-686-n6-024): add more things to the da

/* ------------------------------------------------------------------------- */
/* Fixed arrays                                                              */
/* ------------------------------------------------------------------------- */

#define fa_len(fa) (sizeof(fa) / sizeof(*(fa)))

#define fa_sub(fa, start, count)                                               \
    ({                                                                         \
        typeof(fa) _fa = (fa);                                                 \
        assert((start) <= fa_len(_fa) && (count) <= fa_len(_fa) - (start),     \
               "FA out of bounds\n");                                          \
        SLICE(typeof(*_fa))                                                    \
        _res = {                                                               \
            .data = _fa + (start),                                             \
            .len = (count),                                                    \
        };                                                                     \
        _res;                                                                  \
    })

#define fa_from(fa, start) fa_sub(fa, start, fa_len(fa) - (start))

#define fa_to(fa, end) fa_sub(fa, 0, end)

#define fa_get(fa, i) ((fa)[i])
#define fa_first(fa) ((fa)[0])
#define fa_last(fa) ((fa)[fa_len(fa) - 1])

#define fa_sub_len(fa, len, start, count)                                      \
    ({                                                                         \
        assert((start) <= (len) && (count) <= (len) - (start),                 \
               "FA out of bounds\n");                                          \
        SLICE(typeof(*(fa)))                                                   \
        _res = {                                                               \
            .data = (fa) + (start),                                            \
            .len = (count),                                                    \
        };                                                                     \
        _res;                                                                  \
    })

#define fa_from_len(fa, len, start) fa_sub_len(fa, len, start, (len) - (start))

#define fa_to_len(fa, len, end) fa_sub_len(fa, len, 0, end)

#define fa_get_len(fa, len, i)                                                 \
    ({                                                                         \
        assert((i) < (len), "FA index out of bounds\n");                       \
        (fa)[i];                                                               \
    })

#define fa_first_len(fa, len) fa_get_len(fa, len, 0)
#define fa_last_len(fa, len) fa_get_len(fa, len, (len) - 1)

#define fa_foreach(it, fa)                                                     \
    for (typeof(*(fa))* it = (fa); it < (fa) + fa_len(fa); ++it)

#define fa_foreach_len(it, fa, size)                                           \
    for (typeof(*(fa))* it = (fa); it < (fa) + (size); ++it)

#define fa_foreach_enum(idx, it, fa)                                           \
    for (size_t idx = 0; idx < fa_len(fa); idx++)                              \
        for (typeof(*(fa))* it = &(fa)[idx]; it != NULL; it = NULL)

#define fa_foreach_enum_len(idx, it, fa, size)                                 \
    for (size_t idx = 0; idx < (size); idx++)                                  \
        for (typeof(*(fa))* it = &(fa)[idx]; it != NULL; it = NULL)

/* ------------------------------------------------------------------------- */
/* Allocators                                                                */
/* ------------------------------------------------------------------------- */

typedef struct {
    IMPL_ALLOCATOR;
    Allocator* allocator;
    DA(void*) pointers;
} TrackingAllocator;

IMPORT void tracking_destroy(TrackingAllocator*);
IMPORT TrackingAllocator tracking_create(Allocator*);

#define UNUSED(a) (void)(a)

typedef struct {
    IMPL_ALLOCATOR;
    Allocator* allocator;
    max_align_t* data;
    size_t offset, cap;
} ArenaAllocator;

IMPORT void arena_destroy(ArenaAllocator*);
IMPORT ArenaAllocator arena_create(Allocator*, size_t);
IMPORT void arena_free_all(ArenaAllocator*);

#ifndef REGION_SIZE
#define REGION_SIZE (1024 * 64)
#endif

typedef struct Region Region;
struct Region {
    max_align_t* data;
    size_t offset, cap;
    Region* next;
};

typedef struct {
    IMPL_ALLOCATOR;
    Allocator* allocator;
    Region *first, *last;
} ScratchAllocator;

IMPORT void scratch_destroy(ScratchAllocator*);
IMPORT ScratchAllocator scratch_create(Allocator*);

/* ------------------------------------------------------------------------- */
/* Strings                                                                   */
/* ------------------------------------------------------------------------- */

typedef DA(char) String;
typedef SLICE(char) Str;

/* String */
IMPORT bool string_compare(String, String);
IMPORT bool string_compare_cstr(String, const char*);
IMPORT bool str_cmp(const char*, const char*);
IMPORT String string_from(const char*, Allocator*);
IMPORT String string_clone(String*, Allocator*);
IMPORT String string_create(Allocator*);
IMPORT char* from_string(String*, Allocator*);
IMPORT void append_char(String*, char);
IMPORT void append_cstr(String*, char*);
IMPORT void append_string(String*, String*);

/* Str construction */
IMPORT Str str_from_cstr(const char*);
IMPORT Str str_from_parts(const char*, size_t);

/* Str comparison */
IMPORT bool str_equal(Str, Str);
IMPORT bool str_equal_cstr(Str, const char*);
IMPORT int str_compare(Str, Str);

/* Str searching */
IMPORT bool str_find(Str, char, size_t*);
IMPORT bool str_find_str(Str, Str, size_t*);

/* Str inspection */
IMPORT bool str_starts_with(Str, Str);
IMPORT bool str_ends_with(Str, Str);

/* Str manipulation */
IMPORT Str str_trim(Str);
IMPORT Str str_trim_start(Str);
IMPORT Str str_trim_end(Str);
IMPORT bool str_split_once(Str, char, Str*, Str*);

/* Str conversion */
IMPORT char* str_to_cstr(Str, Allocator*);
IMPORT String str_to_string(Str, Allocator*);

/* ------------------------------------------------------------------------- */
/* Hash maps                                                                 */
/* ------------------------------------------------------------------------- */

#ifndef HM_INITIAL_CAP
#define HM_INITIAL_CAP 16
#endif

#define HM_LOAD_NUM 3
#define HM_LOAD_DEN 4

#define KV(ktype, vtype)                                                       \
    struct {                                                                   \
        ktype key;                                                             \
        vtype value;                                                           \
    }

#define IMPL_HM(ktype, vtype)                                                  \
    Allocator* alloc;                                                          \
    size_t (*hash)(ktype);                                                     \
    bool (*equals)(ktype, ktype);                                              \
    DA(DA(KV(ktype, vtype))) buckets;                                          \
    size_t len;

#define HM(ktype, vtype)                                                       \
    struct {                                                                   \
        IMPL_HM(ktype, vtype);                                                 \
    }

/* Default implementations */
IMPORT size_t hm_hash_cstr(const char*);
IMPORT size_t hm_hash_int(int);
IMPORT size_t hm_hash_string(String);
IMPORT bool hm_equals_int(int, int);

/* Type helpers */
#define _HM_KEY_TYPE(hm) typeof(((hm)->buckets.data->data)->key)

#define _HM_KEY_SAMPLE(hm) ((_HM_KEY_TYPE(hm)){0})

#define _HM_HAS_DEFAULT_HASH(hm)                                               \
    _Generic(_HM_KEY_SAMPLE(hm),                                               \
        const char*: 1,                                                        \
        char*: 1,                                                              \
        String: 1,                                                             \
        int: 1,                                                                \
        default: 0)

#define _HM_DEFAULT_HASH(hm)                                                   \
    _Generic(_HM_KEY_SAMPLE(hm),                                               \
        const char*: hm_hash_cstr,                                             \
        char*: (size_t (*)(char*))hm_hash_cstr,                                \
        String: hm_hash_string,                                                \
        int: hm_hash_int)

#define _HM_HAS_DEFAULT_EQUALS(hm)                                             \
    _Generic(_HM_KEY_SAMPLE(hm),                                               \
        const char*: 1,                                                        \
        char*: 1,                                                              \
        String: 1,                                                             \
        int: 1,                                                                \
        default: 0)

#define _HM_DEFAULT_EQUALS(hm)                                                 \
    _Generic(_HM_KEY_SAMPLE(hm),                                               \
        const char*: str_cmp,                                                  \
        char*: (bool (*)(char*, char*))str_cmp,                                \
        String: string_compare,                                                \
        int: hm_equals_int)

/* Initialization */
#define hm_init(hm, allocator)                                                 \
    do {                                                                       \
        (hm)->alloc = (Allocator*)allocator;                                   \
        if (_HM_HAS_DEFAULT_HASH(hm))                                          \
            (hm)->hash = _HM_DEFAULT_HASH(hm);                                 \
        if (_HM_HAS_DEFAULT_EQUALS(hm))                                        \
            (hm)->equals = _HM_DEFAULT_EQUALS(hm);                             \
        (hm)->len = 0;                                                         \
        da_init(&(hm)->buckets, allocator);                                    \
        da_reserve(&(hm)->buckets, HM_INITIAL_CAP);                            \
        for (size_t _i = 0; _i < HM_INITIAL_CAP; _i++)                         \
            da_init(&(hm)->buckets.data[_i], allocator);                       \
        (hm)->buckets.len = HM_INITIAL_CAP;                                    \
    } while (0)

/* Rehashing */
#define hm_rehash(hm, new_capacity)                                            \
    do {                                                                       \
        size_t _new_cap = (new_capacity);                                      \
        assert(_new_cap > 0, "hash map capacity cannot be zero\n");            \
        assert((hm)->hash != NULL, "hash map has no hash function\n");         \
        typeof((hm)->buckets) _new_buckets = {0};                              \
        da_init(&_new_buckets, (hm)->alloc);                                   \
        da_reserve(&_new_buckets, _new_cap);                                   \
        for (size_t _i = 0; _i < _new_cap; _i++)                               \
            da_init(&_new_buckets.data[_i], (hm)->alloc);                      \
        _new_buckets.len = _new_cap;                                           \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++) {                    \
            da_foreach(_entry, &(hm)->buckets.data[_i]) {                      \
                size_t _index = (hm)->hash(_entry->key) % _new_cap;            \
                da_append(&_new_buckets.data[_index], *_entry);                \
            }                                                                  \
        }                                                                      \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++)                      \
            da_free(&(hm)->buckets.data[_i]);                                  \
        da_free(&(hm)->buckets);                                               \
        (hm)->buckets = _new_buckets;                                          \
    } while (0)

/* Set */
#define hm_set(hm, _nkey, _nvalue)                                             \
    do {                                                                       \
        typeof(_nkey) nkey = _nkey;                                            \
        typeof(_nvalue) nvalue = _nvalue;                                      \
        assert((hm)->hash != NULL, "hash map has no hash function\n");         \
        assert((hm)->equals != NULL, "hash map has no equality function\n");   \
        assert((hm)->buckets.len > 0, "hash map is not initialized\n");        \
        size_t _hash = (hm)->hash(nkey);                                       \
        size_t _index = _hash % (hm)->buckets.len;                             \
        bool _done = false;                                                    \
        da_foreach(_entry, &(hm)->buckets.data[_index]) {                      \
            if ((hm)->hash(_entry->key) == _hash &&                            \
                (hm)->equals(_entry->key, nkey)) {                             \
                _entry->value = nvalue;                                        \
                _done = true;                                                  \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        if (!_done) {                                                          \
            if (((hm)->len + 1) * HM_LOAD_DEN >                                \
                (hm)->buckets.len * HM_LOAD_NUM) {                             \
                hm_rehash(hm, (hm)->buckets.len * 2);                          \
                _index = _hash % (hm)->buckets.len;                            \
            }                                                                  \
            da_append(&(hm)->buckets.data[_index],                             \
                      ((typeof(*(hm)->buckets.data->data)){                    \
                          .key = nkey,                                         \
                          .value = nvalue,                                     \
                      }));                                                     \
            (hm)->len++;                                                       \
        }                                                                      \
    } while (0)

/* Get */
#define hm_get(hm, _nkey)                                                      \
    ({                                                                         \
        typeof(_nkey) nkey = _nkey;                                            \
        typeof(((hm)->buckets.data->data->value))* _res = NULL;                \
        assert((hm)->hash != NULL, "hash map has no hash function\n");         \
        assert((hm)->equals != NULL, "hash map has no equality function\n");   \
        assert((hm)->buckets.len > 0, "hash map is not initialized\n");        \
        size_t _hash = (hm)->hash(nkey);                                       \
        size_t _index = _hash % (hm)->buckets.len;                             \
        da_foreach(_entry, &(hm)->buckets.data[_index]) {                      \
            if ((hm)->hash(_entry->key) == _hash &&                            \
                (hm)->equals(_entry->key, nkey)) {                             \
                _res = &_entry->value;                                         \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        _res;                                                                  \
    })

#define hm_has(hm, key) (hm_get(hm, key) != NULL)

/* Remove */
#define hm_remove(hm, nkey)                                                    \
    ({                                                                         \
        typeof(nkey) _nkey = nkey;                                             \
        assert((hm)->hash != NULL, "hash map has no hash function\n");         \
        assert((hm)->equals != NULL, "hash map has no equality function\n");   \
        assert((hm)->buckets.len > 0, "hash map is not initialized\n");        \
        size_t _hash = (hm)->hash(_nkey);                                      \
        size_t _index = _hash % (hm)->buckets.len;                             \
        typeof(*((hm)->buckets.data->data)) _res = {0};                        \
        for (size_t _i = 0; _i < (hm)->buckets.data[_index].len; _i++) {       \
            typeof((hm)->buckets.data[_index].data) _data =                    \
                (hm)->buckets.data[_index].data;                               \
            if ((hm)->hash(_data[_i].key) == _hash &&                          \
                (hm)->equals(_data[_i].key, _nkey)) {                          \
                _res = da_remove_unordered(&(hm)->buckets.data[_index], _i);   \
                (hm)->len--;                                                   \
                break;                                                         \
            }                                                                  \
        }                                                                      \
        _res;                                                                  \
    })

/* Free */
#define hm_free(hm)                                                            \
    do {                                                                       \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++)                      \
            da_free(&(hm)->buckets.data[_i]);                                  \
        da_free(&(hm)->buckets);                                               \
        (hm)->len = 0;                                                         \
    } while (0)

#define hm_free_call(hm, b, c)                                                 \
    do {                                                                       \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++)                      \
            da_free_call(&(hm)->buckets.data[_i], b, c);                       \
        da_free(&(hm)->buckets);                                               \
        (hm)->len = 0;                                                         \
    } while (0)

/* Keys, values, entries */
#define hm_keys(hm, allocator)                                                 \
    ({                                                                         \
        DA(typeof(((hm)->buckets.data->data->key))) _res = {0};                \
        da_init(&_res, allocator);                                             \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++)                      \
            da_foreach(_entry, &(hm)->buckets.data[_i])                        \
                da_append(&_res, _entry->key);                                 \
        _res;                                                                  \
    })

#define hm_values(hm, allocator)                                               \
    ({                                                                         \
        DA(typeof(((hm)->buckets.data->data->value))) _res = {0};              \
        da_init(&_res, allocator);                                             \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++)                      \
            da_foreach(_entry, &(hm)->buckets.data[_i])                        \
                da_append(&_res, _entry->value);                               \
        _res;                                                                  \
    })

#define hm_entries(hm, allocator)                                              \
    ({                                                                         \
        DA(typeof(*((hm)->buckets.data->data))) _res = {0};                    \
        da_init(&_res, allocator);                                             \
        for (size_t _i = 0; _i < (hm)->buckets.len; _i++)                      \
            da_foreach(_entry, &(hm)->buckets.data[_i])                        \
                da_append(&_res, *_entry);                                     \
        _res;                                                                  \
    })

// TASK(20260218-083256-729-n6-649): add more functions for hashmaps

#define FORMAT(alloc, text, ...)                                               \
    ({                                                                         \
        int __size = snprintf(NULL, 0, text, ##__VA_ARGS__);                   \
        char* __txt = ALLOC(alloc, __size + 1);                                \
        sprintf(__txt, text, ##__VA_ARGS__);                                   \
        __txt;                                                                 \
    })

#endif /* AIDS */

#ifdef AIDS_IMPLEMENTATION
#ifndef AIDS_IMPLEMENTATION_H
#define AIDS_IMPLEMENTATION_H

/* ------------------------------------------------------------------------- */
/* Default allocator                                                         */
/* ------------------------------------------------------------------------- */

void* dmalloc(Allocator* _, size_t size) {
    UNUSED(_);
    return malloc(size);
}

void* drealloc(Allocator* _, void* ptr, size_t size) {
    UNUSED(_);
    return realloc(ptr, size);
}

void* dcalloc(Allocator* _, size_t nmemb, size_t size) {
    UNUSED(_);
    return calloc(nmemb, size);
}

void dfree(Allocator* _, void* ptr) {
    UNUSED(_);
    free(ptr);
}

EXPORT Allocator DEFAULT_ALLOCATOR = {
    .alloc = dmalloc,
    .realloc = drealloc,
    .calloc = dcalloc,
    .free = dfree,
};

/* ------------------------------------------------------------------------- */
/* Tracking allocator                                                        */
/* ------------------------------------------------------------------------- */

void* tracking_alloc_i(TrackingAllocator* self, size_t size) {
    void* ptr = ALLOC(self->allocator, size);
    if (ptr == NULL)
        return NULL;

    da_foreach(pointer, &self->pointers) {
        if (*pointer == NULL) {
            *pointer = ptr;
            return ptr;
        }
    }

    da_append(&self->pointers, ptr);
    return ptr;
}

void* tracking_calloc_i(TrackingAllocator* self, size_t nmemb, size_t size) {
    void* ptr = CALLOC(self->allocator, nmemb, size);
    if (ptr == NULL)
        return NULL;

    da_foreach(pointer, &self->pointers) {
        if (*pointer == NULL) {
            *pointer = ptr;
            return ptr;
        }
    }

    da_append(&self->pointers, ptr);
    return ptr;
}

void* tracking_realloc_i(TrackingAllocator* self, void* ptr, size_t size) {
    if (ptr == NULL)
        return ALLOC(self, size);

    da_foreach(pointer, &self->pointers) {
        if (*pointer == ptr) {
            *pointer = REALLOC(self->allocator, ptr, size);
            return *pointer;
        }
    }

    panic("the pointer isn't in the Tracking Allocator\n");
}

void tracking_free_i(TrackingAllocator* self, void* ptr) {
    if (ptr == NULL)
        return;

    da_foreach(pointer, &self->pointers) {
        if (*pointer == ptr) {
            FREE(self->allocator, ptr);
            *pointer = NULL;
            return;
        }
    }

    panic("the pointer isn't in the Tracking Allocator\n");
}

EXPORT TrackingAllocator tracking_create(Allocator* backing) {
    TrackingAllocator a = {0};
    a.allocator = backing;
    da_init(&a.pointers, a.allocator);
    a.alloc = (fun_alloc)tracking_alloc_i;
    a.calloc = (fun_calloc)tracking_calloc_i;
    a.free = (fun_free)tracking_free_i;
    a.realloc = (fun_realloc)tracking_realloc_i;
    return a;
}

EXPORT void tracking_destroy(TrackingAllocator* self) {
    da_foreach(ptr, &self->pointers) FREE(self->allocator, *ptr);
    da_free(&self->pointers);
}

/* ------------------------------------------------------------------------- */
/* String implementation                                                     */
/* ------------------------------------------------------------------------- */

EXPORT String string_from(const char* str, Allocator* alloc) {
    size_t len = strlen(str);

    String string = {0};
    da_init(&string, alloc);

    if (len == 0)
        return string;

    string.data = ALLOC(alloc, len + 1);
    if (!string.data)
        return string;

    memcpy(string.data, str, len + 1);
    string.cap = len + 1;
    string.len = len;
    return string;
}

EXPORT String string_clone(String* str, Allocator* alloc) {
    String result = string_create(alloc);

    if (str->len) {
        da_reserve(&result, str->len);
        memcpy(result.data, str->data, str->len);
        result.len = str->len;
    }

    return result;
}

EXPORT char* from_string(String* str, Allocator* alloc) {
    char* s = ALLOC(alloc, str->len + 1);
    if (!s)
        return NULL;

    if (str->len)
        memcpy(s, str->data, str->len);

    s[str->len] = '\0';
    return s;
}

EXPORT void append_char(String* str, char ch) { da_append(str, ch); }

EXPORT void append_cstr(String* str, char* s) {
    size_t len = strlen(s);

    da_reserve(str, str->len + len);
    memcpy(str->data + str->len, s, len);
    str->len += len;
}

EXPORT void append_string(String* str, String* val) {
    da_reserve(str, str->len + val->len);
    memcpy(str->data + str->len, val->data, val->len);
    str->len += val->len;
}

EXPORT bool string_compare(String a, String b) {
    return a.len == b.len && (a.len == 0 || memcmp(a.data, b.data, a.len) == 0);
}

EXPORT bool string_compare_cstr(String str, const char* cstr) {
    size_t len = strlen(cstr);

    return str.len == len && (len == 0 || memcmp(str.data, cstr, len) == 0);
}

EXPORT bool str_cmp(const char* a, const char* b) { return strcmp(a, b) == 0; }

EXPORT String string_create(Allocator* alloc) {
    String s = {0};
    da_init(&s, alloc);
    return s;
}

/* ------------------------------------------------------------------------- */
/* Str construction                                                          */
/* ------------------------------------------------------------------------- */

EXPORT Str str_from_cstr(const char* s) {
    return (Str){
        .data = (char*)s,
        .len = strlen(s),
    };
}

EXPORT Str str_from_parts(const char* data, size_t len) {
    return (Str){
        .data = (char*)data,
        .len = len,
    };
}

/* ------------------------------------------------------------------------- */
/* Str comparison                                                            */
/* ------------------------------------------------------------------------- */

EXPORT bool str_equal(Str a, Str b) {
    return a.len == b.len && (a.len == 0 || memcmp(a.data, b.data, a.len) == 0);
}

EXPORT bool str_equal_cstr(Str a, const char* b) {
    size_t len = strlen(b);

    return a.len == len && (len == 0 || memcmp(a.data, b, len) == 0);
}

EXPORT int str_compare(Str a, Str b) {
    size_t len = a.len < b.len ? a.len : b.len;
    int result = len ? memcmp(a.data, b.data, len) : 0;

    if (result)
        return result;

    return (a.len > b.len) - (a.len < b.len);
}

/* ------------------------------------------------------------------------- */
/* Str searching                                                             */
/* ------------------------------------------------------------------------- */

EXPORT bool str_find(Str s, char c, size_t* index) {
    for (size_t i = 0; i < s.len; i++) {
        if (s.data[i] == c) {
            if (index)
                *index = i;
            return true;
        }
    }

    return false;
}

EXPORT bool str_find_str(Str s, Str needle, size_t* index) {
    if (needle.len > s.len)
        return false;

    if (needle.len == 0) {
        if (index)
            *index = 0;
        return true;
    }

    for (size_t i = 0; i <= s.len - needle.len; i++) {
        if (memcmp(s.data + i, needle.data, needle.len) == 0) {
            if (index)
                *index = i;
            return true;
        }
    }

    return false;
}

/* ------------------------------------------------------------------------- */
/* Str inspection                                                            */
/* ------------------------------------------------------------------------- */

EXPORT bool str_starts_with(Str s, Str prefix) {
    return prefix.len <= s.len &&
           (prefix.len == 0 || memcmp(s.data, prefix.data, prefix.len) == 0);
}

EXPORT bool str_ends_with(Str s, Str suffix) {
    return suffix.len <= s.len &&
           (suffix.len == 0 ||
            memcmp(s.data + s.len - suffix.len, suffix.data, suffix.len) == 0);
}

/* ------------------------------------------------------------------------- */
/* Str manipulation                                                         */
/* ------------------------------------------------------------------------- */

static bool str_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' ||
           c == '\f';
}

EXPORT Str str_trim_start(Str s) {
    size_t start = 0;

    while (start < s.len && str_is_whitespace(s.data[start]))
        start++;

    return slice_from(s, start);
}

EXPORT Str str_trim_end(Str s) {
    size_t end = s.len;

    while (end > 0 && str_is_whitespace(s.data[end - 1]))
        end--;

    return slice_to(s, end);
}

EXPORT Str str_trim(Str s) { return str_trim_end(str_trim_start(s)); }

EXPORT bool str_split_once(Str s, char delimiter, Str* left, Str* right) {
    size_t index;

    if (!str_find(s, delimiter, &index))
        return false;

    if (left)
        *left = slice_to(s, index);

    if (right)
        *right = slice_from(s, index + 1);

    return true;
}

/* ------------------------------------------------------------------------- */
/* Str conversion                                                            */
/* ------------------------------------------------------------------------- */

EXPORT char* str_to_cstr(Str s, Allocator* alloc) {
    char* result = ALLOC(alloc, s.len + 1);
    if (!result)
        return NULL;

    if (s.len)
        memcpy(result, s.data, s.len);

    result[s.len] = '\0';
    return result;
}

EXPORT String str_to_string(Str s, Allocator* alloc) {
    String result = string_create(alloc);

    if (s.len) {
        da_reserve(&result, s.len);
        memcpy(result.data, s.data, s.len);
        result.len = s.len;
    }

    return result;
}

/* ------------------------------------------------------------------------- */
/* Hash map implementations                                                  */
/* ------------------------------------------------------------------------- */

EXPORT size_t hm_hash_cstr(const char* s) {
    size_t hash = 1469598103934665603ULL;

    while (*s) {
        hash ^= (unsigned char)*s++;
        hash *= 1099511628211ULL;
    }

    return hash;
}

EXPORT size_t hm_hash_int(int s) { return s; }

EXPORT size_t hm_hash_string(String s) {
    size_t hash = 1469598103934665603ULL;

    da_foreach(ch, &s) {
        hash ^= (unsigned char)*ch;
        hash *= 1099511628211ULL;
    }

    return hash;
}

EXPORT bool hm_equals_int(int a, int b) { return a == b; }

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* Arena allocator                                                           */
/* ------------------------------------------------------------------------- */

/* Each allocation reserves an aligned header containing its requested size. */
static size_t allocator_header_units(void) {
    return (sizeof(size_t) + sizeof(max_align_t) - 1) /
           sizeof(max_align_t);
}

static bool allocator_allocation_units(size_t size, size_t* units) {
    size_t payload_units = size / sizeof(max_align_t);
    if (size % sizeof(max_align_t) != 0)
        payload_units++;

    size_t header_units = allocator_header_units();
    if (payload_units > SIZE_MAX - header_units)
        return false;

    *units = header_units + payload_units;
    return true;
}

static size_t allocator_allocation_size(void* ptr) {
    max_align_t* base = (max_align_t*)ptr - allocator_header_units();
    return *(size_t*)base;
}

void* arena_alloc_i(ArenaAllocator* self, size_t size) {
    if (!self->data)
        return NULL;

    size_t units;
    if (!allocator_allocation_units(size, &units))
        return NULL;

    if (self->offset > self->cap || units > self->cap - self->offset)
        return NULL;

    max_align_t* base = self->data + self->offset;
    *(size_t*)base = size;
    self->offset += units;

    return base + allocator_header_units();
}

void* arena_calloc_i(ArenaAllocator* self, size_t nmemb, size_t size) {
    if (size != 0 && nmemb > SIZE_MAX / size)
        return NULL;

    size_t total = nmemb * size;
    void* ptr = ALLOC(self, total);
    if (!ptr)
        return NULL;

    memset(ptr, 0, total);
    return ptr;
}

void* arena_realloc_i(ArenaAllocator* self, void* ptr, size_t size) {
    if (!ptr)
        return ALLOC(self, size);

    size_t old_size = allocator_allocation_size(ptr);
    void* new_ptr = ALLOC(self, size);
    if (!new_ptr)
        return NULL;

    size_t copy_size = old_size < size ? old_size : size;
    if (copy_size)
        memcpy(new_ptr, ptr, copy_size);

    return new_ptr;
}

void arena_free_i(ArenaAllocator* _, void* ptr) {
    UNUSED(_);
    UNUSED(ptr);
#ifndef AIDS_SILENT_WARNINGS
    printf("Arena Allocator doesn't support free\n");
#endif
}

EXPORT ArenaAllocator arena_create(Allocator* base, size_t size) {
    ArenaAllocator alloc = {0};

    alloc.allocator = base;
    alloc.cap = size / sizeof(max_align_t);
    alloc.offset = 0;
    alloc.data = ALLOC(base, size);

    alloc.alloc = (fun_alloc)arena_alloc_i;
    alloc.calloc = (fun_calloc)arena_calloc_i;
    alloc.free = (fun_free)arena_free_i;
    alloc.realloc = (fun_realloc)arena_realloc_i;

    return alloc;
}

EXPORT void arena_free_all(ArenaAllocator* self) { self->offset = 0; }

EXPORT void arena_destroy(ArenaAllocator* self) {
    if (self->data)
        FREE(self->allocator, self->data);
    self->cap = 0;
    self->offset = 0;
    self->data = NULL;
}

/* ------------------------------------------------------------------------- */
/* Scratch allocator                                                         */
/* ------------------------------------------------------------------------- */

void* scratch_alloc_i(ScratchAllocator* self, size_t size) {
    size_t units;
    if (!allocator_allocation_units(size, &units))
        return NULL;

    if (units > SIZE_MAX / sizeof(max_align_t))
        return NULL;

    if (self->last == NULL ||
        self->last->offset > self->last->cap ||
        units > self->last->cap - self->last->offset) {
        Region* region = ALLOC(self->allocator, sizeof(Region));
        if (!region)
            return NULL;

        size_t default_cap = REGION_SIZE / sizeof(max_align_t);
        region->offset = 0;
        region->cap = units > default_cap ? units : default_cap;
        region->next = NULL;
        region->data = ALLOC(self->allocator,
                             region->cap * sizeof(max_align_t));

        if (!region->data) {
            FREE(self->allocator, region);
            return NULL;
        }

        if (self->last)
            self->last->next = region;
        else
            self->first = region;

        self->last = region;
    }

    max_align_t* base = self->last->data + self->last->offset;
    *(size_t*)base = size;
    self->last->offset += units;

    return base + allocator_header_units();
}

void* scratch_calloc_i(ScratchAllocator* self, size_t nmemb, size_t size) {
    if (size != 0 && nmemb > SIZE_MAX / size)
        return NULL;

    size_t total = nmemb * size;
    void* ptr = ALLOC(self, total);
    if (!ptr)
        return NULL;

    memset(ptr, 0, total);
    return ptr;
}

void* scratch_realloc_i(ScratchAllocator* self, void* ptr, size_t size) {
    if (!ptr)
        return ALLOC(self, size);

    size_t old_size = allocator_allocation_size(ptr);
    void* new_ptr = ALLOC(self, size);
    if (!new_ptr)
        return NULL;

    size_t copy_size = old_size < size ? old_size : size;
    if (copy_size)
        memcpy(new_ptr, ptr, copy_size);

    return new_ptr;
}

void scratch_free_i(ScratchAllocator* _, void* ptr) {
    UNUSED(_);
    UNUSED(ptr);
#ifndef AIDS_SILENT_WARNINGS
    printf("Scratch Allocator doesn't support free\n");
#endif
}

EXPORT ScratchAllocator scratch_create(Allocator* base) {
    ScratchAllocator alloc = {0};

    alloc.allocator = base;

    alloc.alloc = (fun_alloc)scratch_alloc_i;
    alloc.calloc = (fun_calloc)scratch_calloc_i;
    alloc.free = (fun_free)scratch_free_i;
    alloc.realloc = (fun_realloc)scratch_realloc_i;

    return alloc;
}

EXPORT void scratch_destroy(ScratchAllocator* self) {
    Region* s = self->first;

    while (s != NULL) {
        Region* next = s->next;
        FREE(self->allocator, s->data);
        FREE(self->allocator, s);
        s = next;
    }

    self->first = NULL;
    self->last = NULL;
}
#endif /* AIDS_IMPLEMENTATION_H */
#endif /* AIDS_IMPLEMENTATION */
