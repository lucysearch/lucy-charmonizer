/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Charmonizer/Core/HeaderChecker.h"
#include "Charmonizer/Core/CFlags.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Probe/Floats.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void
chaz_Floats_run(void) {
    chaz_ConfWriter_start_module("Floats");

    chaz_ConfWriter_append_conf(
        "typedef union { unsigned char c[4]; float f; } chy_floatu32;\n"
        "typedef union { unsigned char c[8]; double d; } chy_floatu64;\n"
        "#ifdef CHY_BIG_END\n"
        "static const chy_floatu32 chy_f32inf\n"
        "    = { { 0x7F, 0x80, 0, 0 } };\n"
        "static const chy_floatu32 chy_f32neginf\n"
        "    = { { 0xFF, 0x80, 0, 0 } };\n"
        "static const chy_floatu32 chy_f32nan\n"
        "    = { { 0x7F, 0xC0, 0, 0 } };\n"
        "static const chy_floatu64 chy_f64inf\n"
        "    = { { 0x7F, 0xF0, 0, 0, 0, 0, 0, 0 } };\n"
        "static const chy_floatu64 chy_f64neginf\n"
        "    = { { 0xFF, 0xF0, 0, 0, 0, 0, 0, 0 } };\n"
        "static const chy_floatu64 chy_f64nan\n"
        "    = { { 0x7F, 0xF8, 0, 0, 0, 0, 0, 0 } };\n"
        "#else /* BIG_END */\n"
        "static const chy_floatu32 chy_f32inf\n"
        "    = { { 0, 0, 0x80, 0x7F } };\n"
        "static const chy_floatu32 chy_f32neginf\n"
        "    = { { 0, 0, 0x80, 0xFF } };\n"
        "static const chy_floatu32 chy_f32nan\n"
        "    = { { 0, 0, 0xC0, 0x7F } };\n"
        "static const chy_floatu64 chy_f64inf\n"
        "    = { { 0, 0, 0, 0, 0, 0, 0xF0, 0x7F } };\n"
        "static const chy_floatu64 chy_f64neginf\n"
        "    = { { 0, 0, 0, 0, 0, 0, 0xF0, 0xFF } };\n"
        "static const chy_floatu64 chy_f64nan\n"
        "    = { { 0, 0, 0, 0, 0, 0, 0xF8, 0x7F } };\n"
        "#endif /* BIG_END */\n"
    );
    chaz_ConfWriter_add_def("F32_INF", "(chy_f32inf.f)");
    chaz_ConfWriter_add_def("F32_NEGINF", "(chy_f32neginf.f)");
    chaz_ConfWriter_add_def("F32_NAN", "(chy_f32nan.f)");
    chaz_ConfWriter_add_def("F64_INF", "(chy_f64inf.d)");
    chaz_ConfWriter_add_def("F64_NEGINF", "(chy_f64neginf.d)");
    chaz_ConfWriter_add_def("F64_NAN", "(chy_f64nan.d)");

    chaz_ConfWriter_end_module();
}

const char*
chaz_Floats_math_library(void) {
    /*
     * The cast to a specific function pointer type is required because
     * C++ overloads sqrt.
     */
    static const char sqrt_code[] =
        CHAZ_QUOTE(  #include <math.h>                              )
        CHAZ_QUOTE(  typedef double (*sqrt_t)(double);              )
        CHAZ_QUOTE(  int main() { return (int)(sqrt_t)sqrt; }       );
    chaz_CFlags *temp_cflags = chaz_CC_get_temp_cflags();
    int success;

    if (chaz_CC_test_link(sqrt_code)) {
        /* Linking against libm not needed. */
        return NULL;
    }

    chaz_CFlags_add_external_lib(temp_cflags, "m");
    success = chaz_CC_test_link(sqrt_code);
    chaz_CFlags_clear(temp_cflags);

    if (!success) {
        chaz_Util_die("Don't know how to use math library.");
    }

    return "m";
}


