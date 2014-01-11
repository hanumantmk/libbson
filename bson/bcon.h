/*
 * @file bcon.h
 * @brief BCON (BSON C Object Notation) Declarations
 */

/*    Copyright 2009-2013 MongoDB Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef BCON_H_
#define BCON_H_

#include <bson.h>

#define BCON_STACK_MAX 100

#define BCON_ENSURE_DECLARE(fun, type) \
   static BSON_INLINE bson_bool_t bcon_ensure_##fun (type _t) { return FALSE; }

#define BCON_ENSURE(fun, val) \
   ((FALSE) ? bcon_ensure_##fun (val) ? (val) : (val) : (val))

#define BCON_ENSURE_STORAGE(fun, val) \
   ((FALSE) ? bcon_ensure_##fun (val) ? &(val) : &(val) : &(val))

BCON_ENSURE_DECLARE (const_char_ptr, const char *)
BCON_ENSURE_DECLARE (double, double)
BCON_ENSURE_DECLARE (const_bson_ptr, const bson_t *)
BCON_ENSURE_DECLARE (bson_ptr, bson_t *)
BCON_ENSURE_DECLARE (bson, bson_t)
BCON_ENSURE_DECLARE (subtype, bson_subtype_t)
BCON_ENSURE_DECLARE (const_uint8_ptr, const bson_uint8_t *)
BCON_ENSURE_DECLARE (uint32, bson_uint32_t)
BCON_ENSURE_DECLARE (timeval, struct timeval)
BCON_ENSURE_DECLARE (const_oid_ptr, const bson_oid_t *)
BCON_ENSURE_DECLARE (oid_ptr, bson_oid_t *)
BCON_ENSURE_DECLARE (int32, bson_int32_t)
BCON_ENSURE_DECLARE (int64, bson_int64_t)
BCON_ENSURE_DECLARE (bool, bson_bool_t)
BCON_ENSURE_DECLARE (bson_type, bson_type_t)
BCON_ENSURE_DECLARE (bson_iter, bson_iter_t)

#define BCON_UTF8(_val) \
   BCON_MAGIC, BCON_TYPE_UTF8, BCON_ENSURE (const_char_ptr, (_val))
#define BCON_DOUBLE(_val) \
   BCON_MAGIC, BCON_TYPE_DOUBLE, BCON_ENSURE (double, (_val))
#define BCON_DOCUMENT(_val) \
   BCON_MAGIC, BCON_TYPE_DOCUMENT, BCON_ENSURE (const_bson_ptr, (_val))
#define BCON_ARRAY(_val) \
   BCON_MAGIC, BCON_TYPE_ARRAY, BCON_ENSURE (const_bson_ptr, (_val))
#define BCON_BIN(_subtype, _binary, _length) \
   BCON_MAGIC, BCON_TYPE_BIN, \
   BCON_ENSURE (subtype, (_subtype)), \
   BCON_ENSURE (const_uint8_ptr, (_binary)), \
   BCON_ENSURE (uint32, (_length))
#define BCON_UNDEFINED BCON_MAGIC, BCON_TYPE_UNDEFINED
#define BCON_OID(_val) \
   BCON_MAGIC, BCON_TYPE_OID, BCON_ENSURE (const_oid_ptr, (_val))
#define BCON_BOOL(_val) \
   BCON_MAGIC, BCON_TYPE_BOOL, BCON_ENSURE (bool, (_val))
#define BCON_DATE_TIME(_val) \
   BCON_MAGIC, BCON_TYPE_DATE_TIME, BCON_ENSURE (timeval, (_val))
#define BCON_NULL BCON_MAGIC, BCON_TYPE_NULL
#define BCON_REGEX(_regex, _flags) \
   BCON_MAGIC, BCON_TYPE_REGEX, \
   BCON_ENSURE (const_char_ptr, (_regex)), \
   BCON_ENSURE (const_char_ptr, (_flags))
#define BCON_DBPOINTER(_collection, _oid) \
   BCON_MAGIC, BCON_TYPE_DBPOINTER, \
   BCON_ENSURE (const_char_ptr, (_collection)), \
   BCON_ENSURE (const_oid_ptr, (_oid))
#define BCON_CODE(_val) \
   BCON_MAGIC, BCON_TYPE_CODE, BCON_ENSURE (const_char_ptr, (_val))
#define BCON_SYMBOL(_val) \
   BCON_MAGIC, BCON_TYPE_SYMBOL, BCON_ENSURE (const_char_ptr, (_val))
#define BCON_CODEWSCOPE(_js, _scope) \
   BCON_MAGIC, BCON_TYPE_CODEWSCOPE, \
   BCON_ENSURE (const_char_ptr, (_js)), \
   BCON_ENSURE (const_bson_ptr, (_scope))
#define BCON_INT32(_val) \
   BCON_MAGIC, BCON_TYPE_INT32, BCON_ENSURE (int32, (_val))
#define BCON_TIMESTAMP(_timestamp, _increment) \
   BCON_MAGIC, BCON_TYPE_TIMESTAMP, \
   BCON_ENSURE (int32, (_timestamp)), \
   BCON_ENSURE (int32, (_increment))
#define BCON_INT64(_val) \
   BCON_MAGIC, BCON_TYPE_INT64, BCON_ENSURE (int64, (_val))
#define BCON_MAXKEY BCON_MAGIC, BCON_TYPE_MAXKEY
#define BCON_MINKEY BCON_MAGIC, BCON_TYPE_MINKEY
#define BCON(_val) \
   BCON_MAGIC, BCON_TYPE_BCON, BCON_ENSURE (const_bson_ptr, (_val))

#define BCONE_UTF8(_val) BCONE_MAGIC, BCON_TYPE_UTF8, \
   BCON_ENSURE_STORAGE (const_char_ptr, (_val))
#define BCONE_DOUBLE(_val) BCONE_MAGIC, BCON_TYPE_DOUBLE, \
   BCON_ENSURE_STORAGE (double, (_val))
#define BCONE_DOCUMENT(_val) BCONE_MAGIC, BCON_TYPE_DOCUMENT, \
   BCON_ENSURE_STORAGE (bson, (_val))
#define BCONE_ARRAY(_val) BCONE_MAGIC, BCON_TYPE_ARRAY, \
   BCON_ENSURE_STORAGE (bson, (_val))
#define BCONE_BIN(subtype, binary, length) \
   BCONE_MAGIC, BCON_TYPE_BIN, \
   BCON_ENSURE_STORAGE (subtype, (subtype)), \
   BCON_ENSURE_STORAGE (const_uint8_ptr, (binary)), \
   BCON_ENSURE_STORAGE (uint32, (length))
#define BCONE_UNDEFINED BCONE_MAGIC, BCON_TYPE_UNDEFINED
#define BCONE_OID(_val) BCONE_MAGIC, BCON_TYPE_OID, \
   BCON_ENSURE_STORAGE (const_oid_ptr, (_val))
#define BCONE_BOOL(_val) BCONE_MAGIC, BCON_TYPE_BOOL, \
   BCON_ENSURE_STORAGE (bool, (_val))
#define BCONE_DATE_TIME(_val) BCONE_MAGIC, BCON_TYPE_DATE_TIME, \
   BCON_ENSURE_STORAGE (timeval, (_val))
#define BCONE_NULL BCONE_MAGIC, BCON_TYPE_NULL
#define BCONE_REGEX(_regex, _flags) \
   BCONE_MAGIC, BCON_TYPE_REGEX, \
   BCON_ENSURE_STORAGE (const_char_ptr, (_regex)), \
   BCON_ENSURE_STORAGE (const_char_ptr, (_flags))
#define BCONE_DBPOINTER(_collection, _oid) \
   BCONE_MAGIC, BCON_TYPE_DBPOINTER, \
   BCON_ENSURE_STORAGE (const_char_ptr, (_collection)), \
   BCON_ENSURE_STORAGE (const_oid_ptr, (_oid))
#define BCONE_CODE(_val) BCONE_MAGIC, BCON_TYPE_CODE, \
   BCON_ENSURE_STORAGE (const_char_ptr, (_val))
#define BCONE_SYMBOL(_val) BCONE_MAGIC, BCON_TYPE_SYMBOL, \
   BCON_ENSURE_STORAGE (const_char_ptr, (_val))
#define BCONE_CODEWSCOPE(_js, _scope) \
   BCONE_MAGIC, BCON_TYPE_CODEWSCOPE, \
   BCON_ENSURE_STORAGE (const_char_ptr, (_js)), \
   BCON_ENSURE_STORAGE (bson, (_scope))
#define BCONE_INT32(_val) BCONE_MAGIC, BCON_TYPE_INT32, \
   BCON_ENSURE_STORAGE (int32, (_val))
#define BCONE_TIMESTAMP(_timestamp, _increment) \
   BCONE_MAGIC, BCON_TYPE_TIMESTAMP, \
   BCON_ENSURE_STORAGE (int32, (_timestamp)), \
   BCON_ENSURE_STORAGE (int32, (_increment))
#define BCONE_INT64(_val) BCONE_MAGIC, BCON_TYPE_INT64, \
   BCON_ENSURE_STORAGE (int64, (_val))
#define BCONE_MAXKEY BCONE_MAGIC, BCON_TYPE_MAXKEY
#define BCONE_MINKEY BCONE_MAGIC, BCON_TYPE_MINKEY
#define BCONE_SKIP(_val) BCONE_MAGIC, BCON_TYPE_SKIP, \
   BCON_ENSURE (bson_type, (_val))
#define BCONE_ITER(_val) BCONE_MAGIC, BCON_TYPE_ITER, \
   BCON_ENSURE_STORAGE (bson_iter, (_val))

extern char *BCON_MAGIC;
extern char *BCONE_MAGIC;

typedef enum
{
   BCON_TYPE_UTF8,
   BCON_TYPE_DOUBLE,
   BCON_TYPE_DOCUMENT,
   BCON_TYPE_ARRAY,
   BCON_TYPE_BIN,
   BCON_TYPE_UNDEFINED,
   BCON_TYPE_OID,
   BCON_TYPE_BOOL,
   BCON_TYPE_DATE_TIME,
   BCON_TYPE_NULL,
   BCON_TYPE_REGEX,
   BCON_TYPE_DBPOINTER,
   BCON_TYPE_CODE,
   BCON_TYPE_SYMBOL,
   BCON_TYPE_CODEWSCOPE,
   BCON_TYPE_INT32,
   BCON_TYPE_TIMESTAMP,
   BCON_TYPE_INT64,
   BCON_TYPE_MAXKEY,
   BCON_TYPE_MINKEY,
   BCON_TYPE_BCON,
   BCON_TYPE_ARRAY_START,
   BCON_TYPE_ARRAY_END,
   BCON_TYPE_DOC_START,
   BCON_TYPE_DOC_END,
   BCON_TYPE_END,
   BCON_TYPE_RAW,
   BCON_TYPE_SKIP,
   BCON_TYPE_ITER,
   BCON_TYPE_ERROR,
} bcon_type_t;

typedef struct bcon_append_ctx_frame
{
   int         i;
   bson_bool_t is_array;
   bson_t      bson;
} bcon_append_ctx_frame_t;

typedef struct bcon_extract_ctx_frame
{
   int         i;
   bson_bool_t is_array;
   bson_iter_t iter;
} bcon_extract_ctx_frame_t;

typedef struct bcon_append_ctx
{
   bcon_append_ctx_frame_t stack[BCON_STACK_MAX];
   int                     n;
} bcon_append_ctx_t;

typedef struct bcon_extract_ctx
{
   bcon_extract_ctx_frame_t stack[BCON_STACK_MAX];
   int                      n;
} bcon_extract_ctx_t;

void
bcon_append (bson_t *bson,
             ...) BSON_GNUC_NULL_TERMINATED;
void
bcon_append_ctx (bson_t            *bson,
                 bcon_append_ctx_t *ctx,
                 ...) BSON_GNUC_NULL_TERMINATED;
void
bcon_append_ctx_va (bson_t            *bson,
                    bcon_append_ctx_t *ctx,
                    va_list           *va);
void
bcon_append_ctx_init (bcon_append_ctx_t *ctx);

void
bcon_extract_ctx_init (bcon_extract_ctx_t *ctx);

bson_bool_t
bcon_extract_ctx_va (bson_t             *bson,
                     bcon_extract_ctx_t *ctx,
                     va_list            *ap);

bson_bool_t
bcon_extract (bson_t *bson,
              ...) BSON_GNUC_NULL_TERMINATED;

bson_bool_t
bcon_extract_va (bson_t             *bson,
                 bcon_extract_ctx_t *ctx,
                 ...) BSON_GNUC_NULL_TERMINATED;

bson_t *
bcon_new (void *unused,
          ...) BSON_GNUC_NULL_TERMINATED;

#define BCON_APPEND(_bson, ...) \
   bcon_append ((_bson), __VA_ARGS__, NULL)
#define BCON_APPEND_CTX(_bson, _ctx, ...) \
   bcon_append_ctx ((_bson), (_ctx), __VA_ARGS__, NULL)

#define BCON_EXTRACT(_bson, ...) \
   bcon_extract ((_bson), __VA_ARGS__, NULL)

#define BCON_EXTRACT_CTX(_bson, _ctx, ...) \
   bcon_extract ((_bson), (_ctx), __VA_ARGS__, NULL)

#define BCON_NEW(...) \
   bcon_new (NULL, __VA_ARGS__, NULL)

#endif
