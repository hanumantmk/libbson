/*
 * Copyright 2013 MongoDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef BSON_H
#define BSON_H


#include <string.h>
#include <sys/time.h>
#include <time.h>


#define BSON_INSIDE

#include "bson-config.h"
#include "bson-context.h"
#include "bson-clock.h"
#include "bson-error.h"
#include "bson-iter.h"
#include "bson-keys.h"
#include "bson-macros.h"
#include "bson-md5.h"
#include "bson-memory.h"
#include "bson-oid.h"
#include "bson-reader.h"
#include "bson-string.h"
#include "bson-thread.h"
#include "bson-types.h"
#include "bson-utf8.h"
#include "bson-version.h"
#include "bson-writer.h"

#undef BSON_INSIDE


BSON_BEGIN_DECLS


/**
 * bson_empty:
 * @b: a bson_t.
 *
 * Checks to see if @b is an empty BSON document. An empty BSON document is
 * a 5 byte document which contains the length (4 bytes) and a single NUL
 * byte indicating end of fields.
 */
#define bson_empty(b) (((b)->len == 5) || !bson_get_data ((b))[4])


/**
 * bson_empty0:
 *
 * Like bson_empty() but treats NULL the same as an empty bson_t document.
 */
#define bson_empty0(b) (!(b) || bson_empty (b))


/**
 * BSON_MAX_SIZE:
 *
 * The maximum size in bytes of a BSON document.
 */
#define BSON_MAX_SIZE ((size_t)((1U << 31) - 1))


/**
 * bson_new:
 *
 * Allocates a new bson_t structure. Call the various bson_append_*()
 * functions to add fields to the bson. You can iterate the bson_t at any
 * time using a bson_iter_t and bson_iter_init().
 *
 * Returns: A newly allocated bson_t that should be freed with bson_destroy().
 */
bson_t *
bson_new (void);


/**
 * bson_init_static:
 * @b: A pointer to a bson_t.
 * @data: The data buffer to use.
 * @length: The length of @data.
 *
 * Initializes a bson_t using @data and @length. This is ideal if you would
 * like to use a stack allocation for your bson and do not need to grow the
 * buffer. @data must be valid for the life of @b.
 *
 * Returns: TRUE if initialized successfully; otherwise FALSE.
 */
bson_bool_t
bson_init_static (bson_t             *b,
                  const bson_uint8_t *data,
                  bson_uint32_t       length);


/**
 * bson_init:
 * @b: A pointer to a bson_t.
 *
 * Initializes a bson_t for use. This function is useful to those that want a
 * stack allocated bson_t. The usefulness of a stack allocated bson_t is
 * marginal as the target buffer for content will still require heap
 * allocations. It can help reduce heap fragmentation on allocators that do
 * not employ SLAB/magazine semantics.
 *
 * You must call bson_destroy() with @b to release resources when you are done
 * using @b.
 */
void
bson_init (bson_t *b);


/**
 * bson_reinit:
 * @b: (inout): A bson_t.
 *
 * Calls bson_destroy() on @b followed by bson_init(). This is useful as a
 * short helper to rebuild documents.
 */
void
bson_reinit (bson_t *b);


/**
 * bson_new_from_data:
 * @data: A buffer containing a serialized bson document.
 * @length: The length of the document in bytes.
 *
 * Creates a new bson_t structure using the data provided. @data should contain
 * at least @length bytes that can be copied into the new bson_t structure.
 *
 * Returns: A newly allocate bson_t that should be freed with bson_destroy().
 *   If the first four bytes (little-endian) of data do not match @length,
 *   then NULL will be returned.
 */
bson_t *
bson_new_from_data (const bson_uint8_t *data,
                    size_t              length);


/**
 * bson_sized_new:
 * @size: A size_t containing the number of bytes to allocate.
 *
 * This will allocate a new bson_t with enough bytes to hold a buffer
 * sized @size. @size must be smaller than INT_MAX bytes.
 *
 * Returns: A newly allocated bson_t that should be freed with bson_destroy().
 */
bson_t *
bson_sized_new (size_t size);


/**
 * bson_copy:
 * @bson: A bson_t.
 *
 * Copies @bson into a newly allocated bson_t. You must call bson_destroy()
 * when you are done with the resulting value to free its resources.
 *
 * Returns: A newly allocated bson_t that should be free'd with bson_destroy()
 */
bson_t *
bson_copy (const bson_t *bson);


/**
 * bson_copy_to:
 * @src: The source bson_t.
 * @dst: The destination bson_t.
 *
 * Initializes @dst and copies the content from @src into @dst.
 */
void
bson_copy_to (const bson_t *src,
              bson_t       *dst);


/**
 * bson_copy_to_excluding:
 * @src: A bson_t.
 * @dst: A bson_t to initialize and copy into.
 * @first_exclude: First field name to exclude.
 *
 * Copies @src into @dst excluding any field that is provided.
 * This is handy for situations when you need to remove one or
 * more fields in a bson_t.
 */
void
bson_copy_to_excluding (const bson_t *src,
                        bson_t       *dst,
                        const char   *first_exclude,
                        ...) BSON_GNUC_NULL_TERMINATED;


/**
 * bson_destroy:
 * @bson: A bson_t.
 *
 * Frees the resources associated with @bson.
 */
void
bson_destroy (bson_t *bson);


/**
 * bson_get_data:
 * @bson: A bson_t.
 *
 * Fetched the data buffer for @bson of @bson->len bytes in length.
 *
 * Returns: A buffer that should not be modified or freed.
 */
const bson_uint8_t *
bson_get_data (const bson_t *bson);


/**
 * bson_count_keys:
 * @bson: A bson_t.
 *
 * Counts the number of elements found in @bson.
 */
bson_uint32_t
bson_count_keys (const bson_t *bson);


/**
 * bson_has_field:
 * @bson: A bson_t.
 * @key: The key to lookup.
 *
 * Checks to see if @bson contains a field named @key.
 *
 * This function is case-sensitive.
 *
 * Returns: TRUE if @key exists in @bson; otherwise FALSE.
 */
bson_bool_t
bson_has_field (const bson_t *bson,
                const char   *key);


/**
 * bson_compare:
 * @bson: A bson_t.
 * @other: A bson_t.
 *
 * Compares @bson to @other in a qsort() style comparison.
 * See qsort() for information on how this function works.
 *
 * Returns: Less than zero, zero, or greater than zero.
 */
int
bson_compare (const bson_t *bson,
              const bson_t *other);

/*
 * bson_compare:
 * @bson: A bson_t.
 * @other: A bson_t.
 *
 * Checks to see if @bson and @other are equal.
 *
 * Returns: TRUE if equal; otherwise FALSE.
 */
bson_bool_t
bson_equal (const bson_t *bson,
            const bson_t *other);


/**
 * bson_validate:
 * @bson: A bson_t.
 * @offset: A location for the error offset.
 *
 * Validates a BSON document by walking through the document and inspecting
 * the fields for valid content.
 *
 * Returns: TRUE if @bson is valid; otherwise FALSE and @offset is set.
 */
bson_bool_t
bson_validate (const bson_t         *bson,
               bson_validate_flags_t flags,
               size_t               *offset);


/**
 * bson_as_json:
 * @bson: A bson_t.
 * @length: A location for the string length, or NULL.
 *
 * Creates a new string containing @bson in extended JSON format. The caller
 * is responsible for freeing the resulting string. If @length is non-NULL,
 * then the length of the resulting string will be placed in @length.
 *
 * See http://docs.mongodb.org/manual/reference/mongodb-extended-json/ for
 * more information on extended JSON.
 *
 * Returns: A newly allocated string that should be freed with bson_free().
 */
char *
bson_as_json (const bson_t *bson,
              size_t       *length);


/**
 * bson_append_array:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @array: A bson_t containing the array.
 *
 * Appends a BSON array to @bson. BSON arrays are like documents where the
 * key is the string version of the index. For example, the first item of the
 * array would have the key "0". The second item would have the index "1".
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_array (bson_t       *bson,
                   const char   *key,
                   int           key_length,
                   const bson_t *array);


/**
 * bson_append_binary:
 * @bson: A bson_t to append.
 * @key: The key for the field.
 * @subtype: The bson_subtype_t of the binary.
 * @binary: The binary buffer to append.
 * @length: The length of @binary.
 *
 * Appends a binary buffer to the BSON document.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_binary (bson_t             *bson,
                    const char         *key,
                    int                 key_length,
                    bson_subtype_t      subtype,
                    const bson_uint8_t *binary,
                    bson_uint32_t       length);


/**
 * bson_append_bool:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: The boolean value.
 *
 * Appends a new field to @bson of type BSON_TYPE_BOOL.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_bool (bson_t     *bson,
                  const char *key,
                  int         key_length,
                  bson_bool_t value);


/**
 * bson_append_code:
 * @bson: A bson_t.
 * @key: The key for the document.
 * @javascript: JavaScript code to be executed.
 *
 * Appends a field of type BSON_TYPE_CODE to the BSON document. @javascript
 * should contain a script in javascript to be executed.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_code (bson_t     *bson,
                  const char *key,
                  int         key_length,
                  const char *javascript);


/**
 * bson_append_code_with_scope:
 * @bson: A bson_t.
 * @key: The key for the document.
 * @javascript: JavaScript code to be executed.
 * @scope: A bson_t containing the scope for @javascript.
 *
 * Appends a field of type BSON_TYPE_CODEWSCOPE to the BSON document.
 * @javascript should contain a script in javascript to be executed.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_code_with_scope (bson_t       *bson,
                             const char   *key,
                             int           key_length,
                             const char   *javascript,
                             const bson_t *scope);


/**
 * bson_append_dbpointer:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @collection: The collection name.
 * @oid: The oid to the reference.
 *
 * Appends a new field of type BSON_TYPE_DBPOINTER. This datum type is
 * deprecated in the BSON spec and should not be used in new code.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_dbpointer (bson_t           *bson,
                       const char       *key,
                       int               key_length,
                       const char       *collection,
                       const bson_oid_t *oid);


/**
 * bson_append_double:
 * @bson: A bson_t.
 * @key: The key for the field.
 *
 * Appends a new field to @bson of the type BSON_TYPE_DOUBLE.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_double (bson_t     *bson,
                    const char *key,
                    int         key_length,
                    double      value);


/**
 * bson_append_document:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: A bson_t containing the subdocument.
 *
 * Appends a new field to @bson of the type BSON_TYPE_DOCUMENT.
 * The documents contents will be copied into @bson.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_document (bson_t       *bson,
                      const char   *key,
                      int           key_length,
                      const bson_t *value);


/**
 * bson_append_document_begin:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @key_length: The length of @key in bytes not including NUL or -1
 *    if @key_length is NUL terminated.
 * @child: A location to an uninitialized bson_t.
 *
 * Appends a new field named @key to @bson. The field is, however,
 * incomplete.  @child will be initialized so that you may add fields to the
 * child document.  Child will use a memory buffer owned by @bson and
 * therefore grow the parent buffer as additional space is used. This allows
 * a single malloc'd buffer to be used when building documents which can help
 * reduce memory fragmentation.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_document_begin (bson_t     *bson,
                            const char *key,
                            int         key_length,
                            bson_t     *child);


/**
 * bson_append_document_end:
 * @bson: A bson_t.
 * @child: A bson_t supplied to bson_append_document_begin().
 *
 * Finishes the appending of a document to a @bson. @child is considered
 * disposed after this call and should not be used any further.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_document_end (bson_t *bson,
                          bson_t *child);


/**
 * bson_append_array_begin:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @key_length: The length of @key in bytes not including NUL or -1
 *    if @key_length is NUL terminated.
 * @child: A location to an uninitialized bson_t.
 *
 * Appends a new field named @key to @bson. The field is, however,
 * incomplete. @child will be initialized so that you may add fields to the
 * child array. Child will use a memory buffer owned by @bson and
 * therefore grow the parent buffer as additional space is used. This allows
 * a single malloc'd buffer to be used when building arrays which can help
 * reduce memory fragmentation.
 *
 * The type of @child will be BSON_TYPE_ARRAY and therefore the keys inside
 * of it MUST be "0", "1", etc.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_array_begin (bson_t     *bson,
                         const char *key,
                         int         key_length,
                         bson_t     *child);


/**
 * bson_append_array_end:
 * @bson: A bson_t.
 * @child: A bson_t supplied to bson_append_array_begin().
 *
 * Finishes the appending of a array to a @bson. @child is considered
 * disposed after this call and should not be used any further.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_array_end (bson_t *bson,
                       bson_t *child);


/**
 * bson_append_int32:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: The bson_int32_t 32-bit integer value.
 *
 * Appends a new field of type BSON_TYPE_INT32 to @bson.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_int32 (bson_t      *bson,
                   const char  *key,
                   int          key_length,
                   bson_int32_t value);


/**
 * bson_append_int64:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: The bson_int64_t 64-bit integer value.
 *
 * Appends a new field of type BSON_TYPE_INT64 to @bson.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_int64 (bson_t      *bson,
                   const char  *key,
                   int          key_length,
                   bson_int64_t value);


/**
 * bson_append_iter:
 * @bson: A bson_t to append to.
 * @key: The key name or %NULL to take current key from @iter.
 * @key_length: The key length or -1 to use strlen().
 * @iter: The iter located on the position of the element to append.
 *
 * Appends a new field to @bson that is equivalent to the field currently
 * pointed to by @iter.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_iter (bson_t            *bson,
                  const char        *key,
                  int                key_length,
                  const bson_iter_t *iter);


/**
 * bson_append_minkey:
 * @bson: A bson_t.
 * @key: The key for the field.
 *
 * Appends a new field of type BSON_TYPE_MINKEY to @bson. This is a special
 * type that compares lower than all other possible BSON element values.
 *
 * See http://bsonspec.org for more information on this type.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_minkey (bson_t     *bson,
                    const char *key,
                    int         key_length);


/**
 * bson_append_maxkey:
 * @bson: A bson_t.
 * @key: The key for the field.
 *
 * Appends a new field of type BSON_TYPE_MAXKEY to @bson. This is a special
 * type that compares higher than all other possible BSON element values.
 *
 * See http://bsonspec.org for more information on this type.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_maxkey (bson_t     *bson,
                    const char *key,
                    int         key_length);


/**
 * bson_append_null:
 * @bson: A bson_t.
 * @key: The key for the field.
 *
 * Appends a new field to @bson with NULL for the value.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_null (bson_t     *bson,
                  const char *key,
                  int         key_length);


/**
 * bson_append_oid:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @oid: bson_oid_t.
 *
 * Appends a new field to the @bson of type BSON_TYPE_OID using the contents of
 * @oid.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_oid (bson_t           *bson,
                 const char       *key,
                 int               key_length,
                 const bson_oid_t *oid);


/**
 * bson_append_regex:
 * @bson: A bson_t.
 * @key: The key of the field.
 * @regex: The regex to append to the bson.
 * @options: Options for @regex.
 *
 * Appends a new field to @bson of type BSON_TYPE_REGEX. @regex should
 * be the regex string. @options should contain the options for the regex.
 *
 * Valid options for @options are:
 *
 *   'i' for case-insensitive.
 *   'm' for multiple matching.
 *   'x' for verbose mode.
 *   'l' to make \w and \W locale dependent.
 *   's' for dotall mode ('.' matches everything)
 *   'u' to make \w and \W match unicode.
 *
 * For more information on what comprimises a BSON regex, see bsonspec.org.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_regex (bson_t     *bson,
                   const char *key,
                   int         key_length,
                   const char *regex,
                   const char *options);


/**
 * bson_append_utf8:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: A UTF-8 encoded string.
 * @length: The length of @value or -1 if it is NUL terminated.
 *
 * Appends a new field to @bson using @key as the key and @value as the UTF-8
 * encoded value.
 *
 * It is the callers responsibility to ensure @value is valid UTF-8. You can
 * use bson_utf8_validate() to perform this check.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_utf8 (bson_t     *bson,
                  const char *key,
                  int         key_length,
                  const char *value,
                  int         length);


/**
 * bson_append_symbol:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: The symbol as a string.
 * @length: The length of @value or -1 if NUL-terminated.
 *
 * Appends a new field to @bson of type BSON_TYPE_SYMBOL. This BSON type is
 * deprecated and should not be used in new code.
 *
 * See http://bsonspec.org for more information on this type.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_symbol (bson_t     *bson,
                    const char *key,
                    int         key_length,
                    const char *value,
                    int         length);


/**
 * bson_append_time_t:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: A time_t.
 *
 * Appends a BSON_TYPE_DATE_TIME field to @bson using the time_t @value for the
 * number of seconds since UNIX epoch in UTC.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_time_t (bson_t     *bson,
                    const char *key,
                    int         key_length,
                    time_t      value);


/**
 * bson_append_timeval:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @value: A struct timeval containing the date and time.
 *
 * Appends a BSON_TYPE_DATE_TIME field to @bson using the struct timeval
 * provided. The time is persisted in milliseconds since the UNIX epoch in UTC.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_timeval (bson_t         *bson,
                     const char     *key,
                     int             key_length,
                     struct timeval *value);


/**
 * bson_append_date_time:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @key_length: The length of @key in bytes or -1 if \0 terminated.
 * @value: The number of milliseconds elapsed since UNIX epoch.
 *
 * Appends a new field to @bson of type BSON_TYPE_DATE_TIME.
 *
 * Returns: TRUE if sucessful; otherwise FALSE.
 */
bson_bool_t
bson_append_date_time (bson_t      *bson,
                       const char  *key,
                       int          key_length,
                       bson_int64_t value);


/**
 * bson_append_now_utc:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @key_length: The length of @key or -1 if it is NULL terminated.
 *
 * Appends a BSON_TYPE_DATE_TIME field to @bson using the current time in UTC
 * as the field value.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_now_utc (bson_t     *bson,
                     const char *key,
                     int         key_length);

/**
 * bson_append_timestamp:
 * @bson: A bson_t.
 * @key: The key for the field.
 * @timestamp: 4 byte timestamp.
 * @increment: 4 byte increment for timestamp.
 *
 * Appends a field of type BSON_TYPE_TIMESTAMP to @bson. This is a special type
 * used by MongoDB replication and sharding. If you need generic time and date
 * fields use bson_append_time_t() or bson_append_timeval().
 *
 * Setting @increment and @timestamp to zero has special semantics. See
 * http://bsonspec.org for more information on this field type.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_timestamp (bson_t       *bson,
                       const char   *key,
                       int           key_length,
                       bson_uint32_t timestamp,
                       bson_uint32_t increment);


/**
 * bson_append_undefined:
 * @bson: A bson_t.
 * @key: The key for the field.
 *
 * Appends a field of type BSON_TYPE_UNDEFINED. This type is deprecated in the
 * spec and should not be used for new code. However, it is provided for those
 * needing to interact with legacy systems.
 *
 * Returns: TRUE if successful; FALSE if append would overflow max size.
 */
bson_bool_t
bson_append_undefined (bson_t     *bson,
                       const char *key,
                       int         key_length);


BSON_END_DECLS
#endif /* BSON_H */
