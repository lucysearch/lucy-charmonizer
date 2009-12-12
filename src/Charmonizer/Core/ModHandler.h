/* Chaz/Core/ModHandler.h -- symbols used by modules in the Charmonizer 
 * core distro.
 */

#ifndef H_CHAZ_MOD_HAND
#define H_CHAZ_MOD_HAND 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include "Charmonizer/Core/Defines.h"

struct chaz_OperSys;
struct chaz_Compiler;

/* Temporary files used by Charmonizer. 
 */
#define CHAZ_MOD_HAND_TRY_SOURCE_PATH  "_charmonizer_try.c"
#define CHAZ_MOD_HAND_TRY_APP_BASENAME "_charmonizer_try"
#define CHAZ_MOD_HAND_TARGET_PATH      "_charmonizer_target"

/* Global variables.
 */
extern struct chaz_OperSys  *chaz_ModHand_os;
extern struct chaz_Compiler *chaz_ModHand_compiler;
extern chaz_bool_t chaz_ModHand_charm_run_available;
extern FILE* chaz_ModHand_charmony_fh;

/* Initialize elements needed by ModHandler.  Must be called before anything 
 * else, but after os and compiler are created.
 */
void
chaz_ModHand_init();

/* Open the charmony.h file handle.  Print supplied text to it, if non-null.
 * Print an explanatory comment and open the include guard.
 */
void
chaz_ModHand_open_charmony_h(const char *charmony_start);

/* Close the include guard on charmony.h, then close the file.  Delete temp
 * files and perform any other needed cleanup.
 */
void
chaz_ModHand_clean_up(void);

/* Attempt to compile the supplied source code and return true if the
 * effort succeeds.
 */
chaz_bool_t
chaz_ModHand_test_compile(char *source, size_t source_len);

/* Attempt to compile the supplied source code.  If successful, capture the 
 * output of the program and return a pointer to a newly allocated buffer.
 * If the compilation fails, return NULL.  The length of the captured 
 * output will be placed into the integer pointed to by [output_len].
 */
char*
chaz_ModHand_capture_output(char *source, size_t source_len, 
                            size_t *output_len);

/* Print output to charmony.h.
 */
void
chaz_ModHand_append_conf(const char *fmt, ...);

/* Print bookends delimiting a short names block.
 */
#define CHAZ_MODHAND_START_SHORT_NAMES \
  chaz_ModHand_append_conf( \
    "\n#if defined(CHY_USE_SHORT_NAMES) || defined(CHAZ_USE_SHORT_NAMES)\n")

#define CHAZ_MODHAND_END_SHORT_NAMES \
    chaz_ModHand_append_conf("#endif /* USE_SHORT_NAMES */\n")

/* Define a shortened version of a macro symbol (minus the "CHY_" prefix);
 */
void
chaz_ModHand_shorten_macro(const char *symbol);

/* Define a shortened version of a typedef symbol (minus the "chy_" prefix);
 */
void
chaz_ModHand_shorten_typedef(const char *symbol);

/* Define a shortened version of a function symbol (minus the "chy_" prefix);
 */
void
chaz_ModHand_shorten_function(const char *symbol);

/* Print a "chapter heading" when starting a module. 
 */
#define CHAZ_MODHAND_START_RUN(module_name) \
    do { \
        chaz_ModHand_append_conf("\n/* %s */\n", module_name); \
        if (chaz_Util_verbosity > 0) { \
            printf("Running %s module...\n", module_name); \
        } \
    } while (0)

/* Leave a little whitespace at the end of each module.
 */
#define CHAZ_MODHAND_END_RUN chaz_ModHand_append_conf("\n")

#ifdef   CHAZ_USE_SHORT_NAMES
  #define TRY_SOURCE_PATH                   CHAZ_MOD_HAND_TRY_SOURCE_PATH
  #define TRY_APP_BASENAME                  CHAZ_MOD_HAND_TRY_APP_BASENAME
  #define TARGET_PATH                       CHAZ_MOD_HAND_TARGET_PATH
  #define ModHand_os                        chaz_ModHand_os
  #define ModHand_compiler                  chaz_ModHand_compiler
  #define ModHand_charm_run_available       chaz_ModHand_charm_run_available
  #define ModHand_charmony_fh               chaz_ModHand_charmony_fh
  #define ModHand_init                      chaz_ModHand_init
  #define ModHand_open_charmony_h           chaz_ModHand_open_charmony_h
  #define ModHand_clean_up                  chaz_ModHand_clean_up
  #define ModHand_write_charm_h             chaz_ModHand_write_charm_h
  #define ModHand_build_charm_run           chaz_ModHand_build_charm_run
  #define START_SHORT_NAMES                 CHAZ_MODHAND_START_SHORT_NAMES
  #define END_SHORT_NAMES                   CHAZ_MODHAND_END_SHORT_NAMES
  #define ModHand_test_compile              chaz_ModHand_test_compile
  #define ModHand_capture_output            chaz_ModHand_capture_output 
  #define ModHand_append_conf               chaz_ModHand_append_conf
  #define ModHand_shorten_macro             chaz_ModHand_shorten_macro
  #define ModHand_shorten_typedef           chaz_ModHand_shorten_typedef
  #define ModHand_shorten_function          chaz_ModHand_shorten_function
  #define START_RUN                         CHAZ_MODHAND_START_RUN
  #define END_RUN                           CHAZ_MODHAND_END_RUN
#endif

#ifdef __cplusplus
}
#endif

#endif /* H_CHAZ_MOD_HAND */

/**
 * Copyright 2006 The Apache Software Foundation
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
 */

