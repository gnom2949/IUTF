/* errtypes.h
 *
 * Copyright 2026 Int Software, Aleksandr Silaev
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * IUTF Error Types version v0.1
 */

#ifndef IUTF_ERROR_TYPES_H
#define IUTF_ERROR_TYPES_H
#include "types.h"
#define IUTF_ERROR_MAP(XX) \
    XX(IUTF_OK, "OK") \
    XX(IUTF_ERR, "Error") \
    XX(IUTF_ERR_MISSING_FIELD, "Missing field") \
    XX(IUTF_ERR_ESCAPED_TOKEN, "Escaped token Notification!") \
    XX(IUTF_ERR_MALLOC_FAILED, "Error: Malloc fail") \
    XX(IUTF_WAIT, "Waiting for data") \
    XX(IUTF_ALERT, "Alert condition!") \
    XX(IUTF_PRSR_PANIC, "Iutf Parser Panic!") \
    XX(IUTF_STOP, "Iutf Stopped") \
    XX(IUTF_ERR_UNKNOWN_TYPE, "Iutf Unknown type") \
    XX(IUTF_BUSY, "Iutf busy") \
    XX(IUTF_ERR_MISSING_FILE, "Missing required file.") \
    XX(IUTF_ERR_UNCOMPACTABLE_EXT, "Uncompactable file extension!") \
    XX(IUTF_ERR_UNRESOLVED_REF, "Unresolved variable reference") \
    XX(IUTF_ERR_INVALID_REF, "Invalid reference syntax") \
    XX(IUTF_ERR_UNDEFINED_VAR, "Undefined variable")

typedef enum {
    #define ITER(code, msg) code, 
        IUTF_ERROR_MAP(ITER)
    #undef ITER
} iutf_errtype;

string iutf_strerror (iutf_errtype err);

#endif