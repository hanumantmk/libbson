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


#include "bson-iter.h"


bson_bool_t
bson_iter_init (bson_iter_t  *iter,
                const bson_t *bson)
{
   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (bson, FALSE);

   if (BSON_UNLIKELY (bson->len < 5)) {
      return FALSE;
   }

   memset (iter, 0, sizeof *iter);

   iter->bson = bson;
   iter->offset = 0;
   iter->next_offset = 4;

   return TRUE;
}


bson_bool_t
bson_iter_recurse (const bson_iter_t *iter,
                   bson_iter_t       *child)
{
   const bson_uint8_t *data = NULL;
   bson_uint32_t len = 0;
   bson_t b;

   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (child, FALSE);

   memset (child, 0, sizeof *child);

   if (*iter->type == BSON_TYPE_DOCUMENT) {
      bson_iter_document (iter, &len, &data);
   } else if (*iter->type == BSON_TYPE_ARRAY) {
      bson_iter_array (iter, &len, &data);
   } else {
      return FALSE;
   }

   /*
    * Load the BSON into an inline document inside the iter. Sadly, we have
    * to do this twice for the time being. We can get rid of that by not
    * using memset() in bson_iter_init().
    */
   if (!bson_init_static (&b, data, len) ||
       !bson_iter_init (child, &b) ||
       !bson_init_static (&child->inl_bson, data, len)) {
      return FALSE;
   }

   child->bson = &child->inl_bson;

   return TRUE;
}


bson_bool_t
bson_iter_init_find (bson_iter_t  *iter,
                     const bson_t *bson,
                     const char   *key)
{
   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (bson, FALSE);
   bson_return_val_if_fail (key, FALSE);

   return bson_iter_init (iter, bson) && bson_iter_find (iter, key);
}


bson_bool_t
bson_iter_init_find_case (bson_iter_t  *iter,
                          const bson_t *bson,
                          const char   *key)
{
   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (bson, FALSE);
   bson_return_val_if_fail (key, FALSE);

   return bson_iter_init (iter, bson) && bson_iter_find_case (iter, key);
}


static bson_bool_t
bson_iter_find_with_len (bson_iter_t *iter,
                         const char  *key,
                         int          keylen)
{
   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (key, FALSE);

   if (keylen < 0) {
      keylen = strlen (key);
   }

   while (bson_iter_next (iter)) {
      if (!strncmp (key, bson_iter_key (iter), keylen)) {
         return TRUE;
      }
   }

   return FALSE;
}


bson_bool_t
bson_iter_find (bson_iter_t *iter,
                const char  *key)
{
   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (key, FALSE);

   return bson_iter_find_with_len (iter, key, -1);
}


bson_bool_t
bson_iter_find_case (bson_iter_t *iter,
                     const char  *key)
{
   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (key, FALSE);

   while (bson_iter_next (iter)) {
      if (!strcasecmp (key, bson_iter_key (iter))) {
         return TRUE;
      }
   }

   return FALSE;
}


bson_bool_t
bson_iter_find_descendant (bson_iter_t *iter,
                           const char  *dotkey,
                           bson_iter_t *descendant)
{
   bson_iter_t tmp;
   const char *dot;
   size_t sublen;

   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (dotkey, FALSE);
   bson_return_val_if_fail (descendant, FALSE);

   if ((dot = strchr (dotkey, '.'))) {
      sublen = dot - dotkey;
   } else {
      sublen = strlen (dotkey);
   }

   if (bson_iter_find_with_len (iter, dotkey, sublen)) {
      if (!dot) {
         *descendant = *iter;
         return TRUE;
      }

      if (BSON_ITER_HOLDS_DOCUMENT (iter) || BSON_ITER_HOLDS_ARRAY (iter)) {
         if (bson_iter_recurse (iter, &tmp)) {
            return bson_iter_find_descendant (&tmp, dot + 1, descendant);
         }
      }
   }

   return FALSE;
}


const char *
bson_iter_key (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, NULL);

   return bson_iter_key_unsafe (iter);
}


bson_type_t
bson_iter_type (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, BSON_TYPE_EOD);
   bson_return_val_if_fail (iter->type, BSON_TYPE_EOD);

   return bson_iter_type_unsafe (iter);
}


bson_bool_t
bson_iter_next (bson_iter_t *iter)
{
   const bson_uint8_t *data;
   bson_uint32_t o;
   const bson_t *b;

   bson_return_val_if_fail (iter, FALSE);

   if (!iter->bson) {
      return FALSE;
   }

   b = iter->bson;
   data = bson_get_data (b);

   iter->offset = iter->next_offset;
   iter->type = &data[iter->offset];
   iter->key = &data[iter->offset + 1];
   iter->data1 = NULL;
   iter->data2 = NULL;
   iter->data3 = NULL;
   iter->data4 = NULL;

   for (o = iter->offset + 1; o < b->len; o++) {
      if (!data[o]) {
         iter->data1 = &data[++o];
         goto fill_data_fields;
      }
   }

   goto mark_invalid;

fill_data_fields:

   switch (*iter->type) {
   case BSON_TYPE_DATE_TIME:
   case BSON_TYPE_DOUBLE:
   case BSON_TYPE_INT64:
   case BSON_TYPE_TIMESTAMP:
      iter->next_offset = o + 8;
      break;
   case BSON_TYPE_CODE:
   case BSON_TYPE_SYMBOL:
   case BSON_TYPE_UTF8:
      {
         bson_uint32_t l;

         if ((o + 4) >= b->len) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         iter->data2 = &data[o + 4];
         memcpy (&l, iter->data1, 4);
         l = BSON_UINT32_FROM_LE (l);

         if (l > (b->len - (o + 4))) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         iter->next_offset = o + 4 + l;

         /*
          * Make sure the string length includes the NUL byte.
          */
         if (BSON_UNLIKELY ((l == 0) ||
                            (iter->next_offset >= iter->bson->len))) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         /*
          * Make sure the last byte is a NUL byte.
          */
         if (BSON_UNLIKELY (iter->data2[l - 1] != '\0')) {
            iter->err_offset = o + 4 + l - 1;
            goto mark_invalid;
         }
      }
      break;
   case BSON_TYPE_BINARY:
      {
         bson_uint32_t l;

         if ((o + 4) >= b->len) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         iter->data2 = &data[o + 4];
         iter->data3 = &data[o + 5];

         memcpy (&l, iter->data1, 4);
         l = BSON_UINT32_FROM_LE (l);
         iter->next_offset = o + 5 + l;
      }
      break;
   case BSON_TYPE_ARRAY:
   case BSON_TYPE_DOCUMENT:
      {
         bson_uint32_t l;

         if ((o + 4) >= b->len) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         memcpy (&l, iter->data1, 4);
         l = BSON_UINT32_FROM_LE (l);
         iter->next_offset = o + l;
      }
      break;
   case BSON_TYPE_OID:
      iter->next_offset = o + 12;
      break;
   case BSON_TYPE_BOOL:
      iter->next_offset = o + 1;
      break;
   case BSON_TYPE_REGEX:
      {
         bson_bool_t eor = FALSE;
         bson_bool_t eoo = FALSE;

         for (; o < b->len; o++) {
            if (!data[o]) {
               iter->data2 = &data[++o];
               eor = TRUE;
               break;
            }
         }

         if (!eor) {
            iter->err_offset = iter->next_offset;
            goto mark_invalid;
         }

         for (; o < b->len; o++) {
            if (!data[o]) {
               eoo = TRUE;
               break;
            }
         }

         if (!eoo) {
            iter->err_offset = iter->next_offset;
            goto mark_invalid;
         }

         iter->next_offset = o + 1;
      }
      break;
   case BSON_TYPE_DBPOINTER:
      {
         bson_uint32_t l;

         if ((o + 4) >= b->len) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         iter->data2 = &data[o + 4];
         memcpy (&l, iter->data1, 4);
         l = BSON_UINT32_FROM_LE (l);

         iter->data3 = &data[o + 4 + l];
         iter->next_offset = o + 4 + l + 12;
      }
      break;
   case BSON_TYPE_CODEWSCOPE:
      {
         bson_uint32_t l;
         bson_uint32_t doclen;

         if ((o + 8) >= b->len) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         iter->data2 = &data[o + 4];
         iter->data3 = &data[o + 8];

         memcpy (&l, iter->data1, 4);
         l = BSON_UINT32_FROM_LE (l);

         if (l < 14) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         iter->next_offset = o + l;

         if (iter->next_offset >= iter->bson->len) {
            iter->err_offset = o;
            goto mark_invalid;
         }

         memcpy (&l, iter->data2, 4);
         l = BSON_UINT32_FROM_LE (l);

         if (BSON_UNLIKELY ((o + 4 + 4 + l + 4) >= iter->next_offset)) {
            iter->err_offset = o + 4;
            goto mark_invalid;
         }

         iter->data4 = &data[o + 4 + 4 + l];
         memcpy (&doclen, iter->data4, 4);
         doclen = BSON_UINT32_FROM_LE (doclen);

         if ((o + 4 + 4 + l + doclen) != iter->next_offset) {
            iter->err_offset = o + 4 + 4 + l;
            goto mark_invalid;
         }
      }
      break;
   case BSON_TYPE_INT32:
      iter->next_offset = o + 4;
      break;
   case BSON_TYPE_MAXKEY:
   case BSON_TYPE_MINKEY:
   case BSON_TYPE_NULL:
   case BSON_TYPE_UNDEFINED:
      iter->data1 = NULL;
      iter->next_offset = o;
      break;
   case BSON_TYPE_EOD:
   default:
      iter->err_offset = o;
      goto mark_invalid;
   }

   /*
    * Check to see if any of the field locations would overflow the
    * current BSON buffer. If so, set the error location to the offset
    * of where the field starts.
    */
   if (!(iter->next_offset < b->len)) {
      iter->err_offset = o;
      goto mark_invalid;
   }

   iter->err_offset = 0;
   return TRUE;

mark_invalid:
   iter->bson = NULL;
   return FALSE;
}


void
bson_iter_binary (const bson_iter_t   *iter,
                  bson_subtype_t      *subtype,
                  bson_uint32_t       *binary_len,
                  const bson_uint8_t **binary)
{
   bson_subtype_t backup;

   bson_return_if_fail (iter);
   bson_return_if_fail (!binary || binary_len);

   if (*iter->type == BSON_TYPE_BINARY) {
      if (!subtype) {
         subtype = &backup;
      }

      *subtype = (bson_subtype_t)*iter->data2;

      if (binary) {
         memcpy (binary_len, iter->data1, 4);
         *binary_len = BSON_UINT32_FROM_LE (*binary_len);
         *binary = iter->data3;

         if (*subtype == BSON_SUBTYPE_BINARY_DEPRECATED) {
            *binary_len -= sizeof (bson_int32_t);
            *binary += sizeof (bson_int32_t);
         }
      }

      return;
   }

   if (binary) {
      *binary = NULL;
   }

   if (binary_len) {
      *binary_len = 0;
   }

   if (subtype) {
      *subtype = BSON_SUBTYPE_BINARY;
   }
}


bson_bool_t
bson_iter_bool (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   if (*iter->type == BSON_TYPE_BOOL) {
      return bson_iter_bool_unsafe (iter);
   }

   return 0;
}


bson_bool_t
bson_iter_as_bool (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   switch (*iter->type) {
   case BSON_TYPE_BOOL:
      return bson_iter_bool (iter);

   case BSON_TYPE_DOUBLE:
      return !(bson_iter_double (iter) == 0.0);

   case BSON_TYPE_INT64:
      return !(bson_iter_int64 (iter) == 0);

   case BSON_TYPE_INT32:
      return !(bson_iter_int32 (iter) == 0);

   case BSON_TYPE_UTF8:
      return TRUE;

   case BSON_TYPE_NULL:
   case BSON_TYPE_UNDEFINED:
      return FALSE;

   default:
      return TRUE;
   }
}


double
bson_iter_double (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   if (*iter->type == BSON_TYPE_DOUBLE) {
      return bson_iter_double_unsafe (iter);
   }

   return 0;
}


bson_int32_t
bson_iter_int32 (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   if (*iter->type == BSON_TYPE_INT32) {
      return bson_iter_int32_unsafe (iter);
   }

   return 0;
}


bson_int64_t
bson_iter_int64 (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   if (*iter->type == BSON_TYPE_INT64) {
      return bson_iter_int64_unsafe (iter);
   }

   return 0;
}


bson_int64_t
bson_iter_as_int64 (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   switch (*iter->type) {
   case BSON_TYPE_BOOL:
      return bson_iter_bool (iter);

   case BSON_TYPE_DOUBLE:
      return bson_iter_double (iter);

   case BSON_TYPE_INT64:
      return bson_iter_int64 (iter);

   case BSON_TYPE_INT32:
      return bson_iter_int32 (iter);

   default:
      return 0;
   }
}


const bson_oid_t *
bson_iter_oid (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, NULL);

   if (*iter->type == BSON_TYPE_OID) {
      return bson_iter_oid_unsafe (iter);
   }

   return NULL;
}


const char *
bson_iter_regex (const bson_iter_t *iter,
                 const char       **options)
{
   const char *ret = NULL;
   const char *ret_options = NULL;

   bson_return_val_if_fail (iter, NULL);

   if (*iter->type == BSON_TYPE_REGEX) {
      ret = (const char *)iter->data1;
      ret_options = (const char *)iter->data2;
   }

   if (options) {
      *options = ret_options;
   }

   return ret;
}


const char *
bson_iter_utf8 (const bson_iter_t *iter,
                bson_uint32_t     *length)
{
   bson_return_val_if_fail (iter, NULL);

   if (*iter->type == BSON_TYPE_UTF8) {
      if (length) {
         *length = bson_iter_utf8_len_unsafe (iter);
      }

      return (const char *)iter->data2;
   }

   if (length) {
      *length = 0;
   }

   return NULL;
}


char *
bson_iter_dup_utf8 (const bson_iter_t *iter,
                    bson_uint32_t     *length)
{
   bson_uint32_t local_length = 0;
   const char *str;
   char *ret = NULL;

   bson_return_val_if_fail (iter, NULL);

   if ((str = bson_iter_utf8 (iter, &local_length))) {
      ret = bson_malloc0 (local_length + 1);
      memcpy (ret, str, local_length);
      ret[local_length] = '\0';
   }

   if (length) {
      *length = local_length;
   }

   return ret;
}


const char *
bson_iter_code (const bson_iter_t *iter,
                bson_uint32_t     *length)
{
   bson_return_val_if_fail (iter, NULL);

   if (*iter->type == BSON_TYPE_CODE) {
      if (length) {
         *length = bson_iter_utf8_len_unsafe (iter);
      }

      return (const char *)iter->data2;
   }

   if (length) {
      *length = 0;
   }

   return NULL;
}


const char *
bson_iter_codewscope (const bson_iter_t   *iter,
                      bson_uint32_t       *length,
                      bson_uint32_t       *scope_len,
                      const bson_uint8_t **scope)
{
   bson_uint32_t len;

   bson_return_val_if_fail (iter, NULL);

   if (*iter->type == BSON_TYPE_CODEWSCOPE) {
      if (length) {
         memcpy (&len, iter->data2, 4);
         *length = BSON_UINT32_FROM_LE (len) - 1;
      }

      memcpy (&len, iter->data4, 4);
      *scope_len = BSON_UINT32_FROM_LE (len);
      *scope = iter->data4;
      return (const char *)iter->data3;
   }

   return NULL;
}


void
bson_iter_dbpointer (const bson_iter_t *iter,
                     bson_uint32_t     *collection_len,
                     const char       **collection,
                     const bson_oid_t **oid)
{
   bson_return_if_fail (iter);

   if (collection) {
      *collection = NULL;
   }

   if (oid) {
      *oid = NULL;
   }

   if (*iter->type == BSON_TYPE_DBPOINTER) {
      if (collection_len) {
         memcpy (collection_len, iter->data1, 4);
         *collection_len = BSON_UINT32_FROM_LE (*collection_len);

         if ((*collection_len) > 0) {
            (*collection_len)--;
         }
      }

      if (collection) {
         *collection = (const char *)iter->data2;
      }

      if (oid) {
         *oid = (const bson_oid_t *)iter->data3;
      }
   }
}


const char *
bson_iter_symbol (const bson_iter_t *iter,
                  bson_uint32_t     *length)
{
   const char *ret = NULL;
   bson_uint32_t ret_length = 0;

   bson_return_val_if_fail (iter, NULL);

   if (*iter->type == BSON_TYPE_SYMBOL) {
      ret = (const char *)iter->data2;
      ret_length = bson_iter_utf8_len_unsafe (iter);
   }

   if (length) {
      *length = ret_length;
   }

   return ret;
}


bson_int64_t
bson_iter_date_time (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   if (*iter->type == BSON_TYPE_DATE_TIME) {
      return bson_iter_int64_unsafe (iter);
   }

   return 0;
}


time_t
bson_iter_time_t (const bson_iter_t *iter)
{
   bson_return_val_if_fail (iter, 0);

   if (*iter->type == BSON_TYPE_DATE_TIME) {
      return bson_iter_time_t_unsafe (iter);
   }

   return 0;
}


void
bson_iter_timestamp (const bson_iter_t *iter,
                     bson_uint32_t     *timestamp,
                     bson_uint32_t     *increment)
{
   bson_uint64_t encoded;
   bson_uint32_t ret_timestamp = 0;
   bson_uint32_t ret_increment = 0;

   bson_return_if_fail (iter);

   if (*iter->type == BSON_TYPE_TIMESTAMP) {
      memcpy (&encoded, iter->data1, 8);
      encoded = BSON_UINT64_FROM_LE (encoded);
      ret_timestamp = (encoded >> 32) & 0xFFFFFFFF;
      ret_increment = encoded & 0xFFFFFFFF;
   }

   if (timestamp) {
      *timestamp = ret_timestamp;
   }

   if (increment) {
      *increment = ret_increment;
   }
}


void
bson_iter_timeval (const bson_iter_t *iter,
                   struct timeval    *tv)
{
   bson_return_if_fail (iter);

   if (*iter->type == BSON_TYPE_DATE_TIME) {
      bson_iter_timeval_unsafe (iter, tv);
      return;
   }

   memset (tv, 0, sizeof *tv);
}


void
bson_iter_document (const bson_iter_t   *iter,
                    bson_uint32_t       *document_len,
                    const bson_uint8_t **document)
{
   bson_return_if_fail (iter);
   bson_return_if_fail (document_len);
   bson_return_if_fail (document);

   *document = NULL;
   *document_len = 0;

   if (*iter->type == BSON_TYPE_DOCUMENT) {
      memcpy (document_len, iter->data1, 4);
      *document_len = BSON_UINT32_FROM_LE (*document_len);
      *document = iter->data1;
   }
}


void
bson_iter_array (const bson_iter_t   *iter,
                 bson_uint32_t       *array_len,
                 const bson_uint8_t **array)
{
   bson_return_if_fail (iter);
   bson_return_if_fail (array_len);
   bson_return_if_fail (array);

   *array = NULL;
   *array_len = 0;

   if (*iter->type == BSON_TYPE_ARRAY) {
      memcpy (array_len, iter->data1, 4);
      *array_len = BSON_UINT32_FROM_LE (*array_len);
      *array = iter->data1;
   }
}


#define VISIT_FIELD(name) visitor->visit_##name && visitor->visit_##name
#define VISIT_AFTER VISIT_FIELD (after)
#define VISIT_BEFORE VISIT_FIELD (before)
#define VISIT_CORRUPT if (visitor->visit_corrupt) visitor->visit_corrupt
#define VISIT_DOUBLE VISIT_FIELD (double)
#define VISIT_UTF8 VISIT_FIELD (utf8)
#define VISIT_DOCUMENT VISIT_FIELD (document)
#define VISIT_ARRAY VISIT_FIELD (array)
#define VISIT_BINARY VISIT_FIELD (binary)
#define VISIT_UNDEFINED VISIT_FIELD (undefined)
#define VISIT_OID VISIT_FIELD (oid)
#define VISIT_BOOL VISIT_FIELD (bool)
#define VISIT_DATE_TIME VISIT_FIELD (date_time)
#define VISIT_NULL VISIT_FIELD (null)
#define VISIT_REGEX VISIT_FIELD (regex)
#define VISIT_DBPOINTER VISIT_FIELD (dbpointer)
#define VISIT_CODE VISIT_FIELD (code)
#define VISIT_SYMBOL VISIT_FIELD (symbol)
#define VISIT_CODEWSCOPE VISIT_FIELD (codewscope)
#define VISIT_INT32 VISIT_FIELD (int32)
#define VISIT_TIMESTAMP VISIT_FIELD (timestamp)
#define VISIT_INT64 VISIT_FIELD (int64)
#define VISIT_MAXKEY VISIT_FIELD (maxkey)
#define VISIT_MINKEY VISIT_FIELD (minkey)


bson_bool_t
bson_iter_visit_all (bson_iter_t          *iter,
                     const bson_visitor_t *visitor,
                     void                 *data)
{
   const char *key;

   bson_return_val_if_fail (iter, FALSE);
   bson_return_val_if_fail (visitor, FALSE);

   while (bson_iter_next (iter)) {
      key = bson_iter_key_unsafe (iter);

      if (VISIT_BEFORE (iter, key, data)) {
         return TRUE;
      }

      switch (bson_iter_type (iter)) {
      case BSON_TYPE_DOUBLE:

         if (VISIT_DOUBLE (iter, key, bson_iter_double (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_UTF8:
         {
            bson_uint32_t utf8_len;
            const char *utf8;
            utf8 = bson_iter_utf8 (iter, &utf8_len);

            if (VISIT_UTF8 (iter, key, utf8_len, utf8, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_DOCUMENT:
         {
            const bson_uint8_t *docbuf = NULL;
            bson_uint32_t doclen = 0;
            bson_t b;

            bson_iter_document (iter, &doclen, &docbuf);

            if (bson_init_static (&b, docbuf, doclen) &&
                VISIT_DOCUMENT (iter, key, &b, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_ARRAY:
         {
            const bson_uint8_t *docbuf = NULL;
            bson_uint32_t doclen = 0;
            bson_t b;

            bson_iter_array (iter, &doclen, &docbuf);

            if (bson_init_static (&b, docbuf, doclen)
                && VISIT_ARRAY (iter, key, &b, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_BINARY:
         {
            const bson_uint8_t *binary = NULL;
            bson_subtype_t subtype = BSON_SUBTYPE_BINARY;
            bson_uint32_t binary_len = 0;

            bson_iter_binary (iter, &subtype, &binary_len, &binary);

            if (VISIT_BINARY (iter, key, subtype, binary_len, binary, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_UNDEFINED:

         if (VISIT_UNDEFINED (iter, key, data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_OID:

         if (VISIT_OID (iter, key, bson_iter_oid (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_BOOL:

         if (VISIT_BOOL (iter, key, bson_iter_bool (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_DATE_TIME:

         if (VISIT_DATE_TIME (iter, key, bson_iter_date_time (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_NULL:

         if (VISIT_NULL (iter, key, data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_REGEX:
         {
            const char *regex = NULL;
            const char *options = NULL;
            regex = bson_iter_regex (iter, &options);

            if (VISIT_REGEX (iter, key, regex, options, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_DBPOINTER:
         {
            bson_uint32_t collection_len = 0;
            const char *collection = NULL;
            const bson_oid_t *oid = NULL;

            bson_iter_dbpointer (iter, &collection_len, &collection, &oid);

            if (VISIT_DBPOINTER (iter, key, collection_len, collection, oid,
                                 data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_CODE:
         {
            bson_uint32_t code_len;
            const char *code;

            code = bson_iter_code (iter, &code_len);

            if (VISIT_CODE (iter, key, code_len, code, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_SYMBOL:
         {
            bson_uint32_t symbol_len;
            const char *symbol;

            symbol = bson_iter_symbol (iter, &symbol_len);

            if (VISIT_SYMBOL (iter, key, symbol_len, symbol, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_CODEWSCOPE:
         {
            bson_uint32_t length = 0;
            const char *code;
            const bson_uint8_t *docbuf = NULL;
            bson_uint32_t doclen = 0;
            bson_t b;

            code = bson_iter_codewscope (iter, &length, &doclen, &docbuf);

            if (bson_init_static (&b, docbuf, doclen) &&
                VISIT_CODEWSCOPE (iter, key, length, code, &b, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_INT32:

         if (VISIT_INT32 (iter, key, bson_iter_int32 (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_TIMESTAMP:
         {
            bson_uint32_t timestamp;
            bson_uint32_t increment;
            bson_iter_timestamp (iter, &timestamp, &increment);

            if (VISIT_TIMESTAMP (iter, key, timestamp, increment, data)) {
               return TRUE;
            }
         }
         break;
      case BSON_TYPE_INT64:

         if (VISIT_INT64 (iter, key, bson_iter_int64 (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_MAXKEY:

         if (VISIT_MAXKEY (iter, bson_iter_key_unsafe (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_MINKEY:

         if (VISIT_MINKEY (iter, bson_iter_key_unsafe (iter), data)) {
            return TRUE;
         }

         break;
      case BSON_TYPE_EOD:
      default:
         break;
      }

      if (VISIT_AFTER (iter, bson_iter_key_unsafe (iter), data)) {
         return TRUE;
      }
   }

   if (iter->err_offset) {
      VISIT_CORRUPT (iter, data);
   }

#undef VISIT_FIELD

   return FALSE;
}


void
bson_iter_overwrite_bool (bson_iter_t *iter,
                          bson_bool_t  value)
{
   bson_return_if_fail (iter);
   bson_return_if_fail (value == 1 || value == 0);

   if (*iter->type == BSON_TYPE_BOOL) {
      memcpy ((void *)iter->data1, &value, 1);
   }
}


void
bson_iter_overwrite_int32 (bson_iter_t *iter,
                           bson_int32_t value)
{
   bson_return_if_fail (iter);

   if (*iter->type == BSON_TYPE_INT32) {
#if BSON_BYTE_ORDER != BSON_LITTLE_ENDIAN
      value = BSON_UINT32_TO_LE (value);
#endif
      memcpy ((void *)iter->data1, &value, 4);
   }
}


void
bson_iter_overwrite_int64 (bson_iter_t *iter,
                           bson_int64_t value)
{
   bson_return_if_fail (iter);

   if (*iter->type == BSON_TYPE_INT64) {
#if BSON_BYTE_ORDER != BSON_LITTLE_ENDIAN
      value = BSON_UINT64_TO_LE (value);
#endif
      memcpy ((void *)iter->data1, &value, 8);
   }
}


void
bson_iter_overwrite_double (bson_iter_t *iter,
                            double       value)
{
   bson_return_if_fail (iter);

   if (*iter->type == BSON_TYPE_DOUBLE) {
      value = BSON_DOUBLE_TO_LE (value);
      memcpy ((void *)iter->data1, &value, 8);
   }
}
