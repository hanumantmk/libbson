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


#ifndef BSON_CLOCK_H
#define BSON_CLOCK_H


#include "bson-macros.h"
#include "bson-types.h"


BSON_BEGIN_DECLS


/**
 * bson_get_monotonic_time:
 *
 * Returns the monotonic system time, if available. A best effort is made to
 * use the monotonic clock. However, some systems may not support such a
 * feature.
 *
 * Returns: A monotonic clock in microseconds.
 */
bson_int64_t
bson_get_monotonic_time (void);


BSON_END_DECLS
#endif /* BSON_CLOCK_H */
