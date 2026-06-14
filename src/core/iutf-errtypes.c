/* iutf-errtypes.c
 *
 * Copyright 2026 Int Software, Aleksandr Silaev
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * IUTF Variable System version 0.1
 */
#include "../includes/iutf.h"

string iutf_strerror (iutf_errtype err)
{
    switch (err) {
        #define ITER(code, msg) case code: return msg;
            IUTF_ERROR_MAP(ITER)
        #undef ITER
            default: return "Unknown Error code.";
    }
}
