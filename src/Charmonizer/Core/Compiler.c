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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

/* Detect binary format.
 */
static void
chaz_CC_detect_binary_format(const char *filename);

/* Detect macros which may help to identify some compilers.
 */
static void
chaz_CC_detect_known_compilers(void);

/** Build a library filename from its components.
 */
static char*
chaz_CC_build_lib_filename(const char *dir, const char *prefix,
                           const char *basename, const char *version,
                           const char *ext);

/* Temporary files. */
#define CHAZ_CC_TRY_SOURCE_PATH  "_charmonizer_try.c"
#define CHAZ_CC_TRY_BASENAME     "_charmonizer_try"
#define CHAZ_CC_TARGET_PATH      "_charmonizer_target"

/* Static vars. */
static struct {
    char     *cc_command;
    char     *cflags;
    char     *try_exe_name;
    char      exe_ext[10];
    char      shared_lib_ext[10];
    char      static_lib_ext[10];
    char      import_lib_ext[10];
    char      obj_ext[10];
    char      gcc_version_str[30];
    int       binary_format;
    int       cflags_style;
    int       is_gcc;
    int       is_msvc;
    int       is_clang;
    int       is_sun_c;
    int       is_cygwin;
    int       is_mingw;
    chaz_CFlags *extra_cflags;
    chaz_CFlags *temp_cflags;
} chaz_CC = {
    NULL, NULL, NULL,
    "", "", "", "", "", "",
    0, 0, 0, 0, 0, 0, 0, 0,
    NULL, NULL
};

void
chaz_CC_init(const char *compiler_command, const char *compiler_flags) {
    const char *code = "int main() { return 0; }\n";
    int compile_succeeded = 0;

    if (chaz_Util_verbosity) { printf("Creating compiler object...\n"); }

    /* Assign, init. */
    chaz_CC.cc_command   = chaz_Util_strdup(compiler_command);
    chaz_CC.cflags       = chaz_Util_strdup(compiler_flags);
    chaz_CC.extra_cflags = NULL;
    chaz_CC.temp_cflags  = NULL;

    /* Set names for the targets which we "try" to compile. */
    strcpy(chaz_CC.exe_ext, ".exe");
    chaz_CC.try_exe_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_CC.exe_ext, NULL);

    /* If we can't compile or execute anything, game over. */
    if (chaz_Util_verbosity) {
        printf("Trying to compile and execute a small test file...\n");
    }

    /* Try MSVC argument style. */
    if (!compile_succeeded) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_MSVC;
        if (!chaz_Util_remove_and_verify(chaz_CC.try_exe_name)) {
            chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_exe_name);
        }
        compile_succeeded = chaz_CC_compile_exe(CHAZ_CC_TRY_SOURCE_PATH,
                                                CHAZ_CC_TRY_BASENAME, code);
        if (compile_succeeded) {
            strcpy(chaz_CC.obj_ext, ".obj");
        }
    }

    /* Try POSIX argument style. */
    if (!compile_succeeded) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_POSIX;
        if (!chaz_Util_remove_and_verify(chaz_CC.try_exe_name)) {
            chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_exe_name);
        }
        compile_succeeded = chaz_CC_compile_exe(CHAZ_CC_TRY_SOURCE_PATH,
                                                CHAZ_CC_TRY_BASENAME, code);
        if (compile_succeeded) {
            strcpy(chaz_CC.obj_ext, ".o");
        }
    }

    if (!compile_succeeded) {
        chaz_Util_die("Failed to compile a small test file");
    }
    chaz_CC_detect_binary_format(chaz_CC.try_exe_name);
    chaz_Util_remove_and_verify(chaz_CC.try_exe_name);

    chaz_CC_detect_known_compilers();

    if (chaz_CC_is_gcc()) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_GNU;
    }
    else if (chaz_CC_is_msvc()) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_MSVC;
    }
    else if (chaz_CC_is_sun_c()) {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_SUN_C;
    }
    else {
        chaz_CC.cflags_style = CHAZ_CFLAGS_STYLE_POSIX;
    }
    chaz_CC.extra_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    chaz_CC.temp_cflags  = chaz_CFlags_new(chaz_CC.cflags_style);

    /* File extensions. */
    if (chaz_CC.binary_format == CHAZ_CC_BINFMT_ELF) {
        if (chaz_Util_verbosity) {
            printf("Detected binary format: ELF\n");
        }
        strcpy(chaz_CC.exe_ext, "");
        strcpy(chaz_CC.shared_lib_ext, ".so");
        strcpy(chaz_CC.static_lib_ext, ".a");
        strcpy(chaz_CC.obj_ext, ".o");
    }
    else if (chaz_CC.binary_format == CHAZ_CC_BINFMT_MACHO) {
        if (chaz_Util_verbosity) {
            printf("Detected binary format: Mach-O\n");
        }
        strcpy(chaz_CC.exe_ext, "");
        strcpy(chaz_CC.shared_lib_ext, ".dylib");
        strcpy(chaz_CC.static_lib_ext, ".a");
        strcpy(chaz_CC.obj_ext, ".o");
    }
    else if (chaz_CC.binary_format == CHAZ_CC_BINFMT_PE) {
        if (chaz_Util_verbosity) {
            printf("Detected binary format: Portable Executable\n");
        }
        strcpy(chaz_CC.exe_ext, ".exe");
        strcpy(chaz_CC.shared_lib_ext, ".dll");
        if (chaz_CC_is_gcc()) {
            strcpy(chaz_CC.static_lib_ext, ".a");
            strcpy(chaz_CC.import_lib_ext, ".dll.a");
            strcpy(chaz_CC.obj_ext, ".o");
        }
        else {
            strcpy(chaz_CC.static_lib_ext, ".lib");
            strcpy(chaz_CC.import_lib_ext, ".lib");
            strcpy(chaz_CC.obj_ext, ".obj");
        }

        if (chaz_CC_has_macro("__CYGWIN__")) {
            chaz_CC.is_cygwin = 1;
        }
        if (chaz_CC_has_macro("__MINGW32__")) {
            chaz_CC.is_mingw = 1;
        }
    }
    else {
        chaz_Util_die("Failed to detect binary format");
    }

    free(chaz_CC.try_exe_name);
    chaz_CC.try_exe_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_CC.exe_ext, NULL);
}

static void
chaz_CC_detect_binary_format(const char *filename) {
    char *output;
    size_t output_len;
    int binary_format = 0;

    output = chaz_Util_slurp_file(filename, &output_len);

    /* ELF. */
    if (binary_format == 0 && output_len >= 4
        && memcmp(output, "\x7F" "ELF", 4) == 0
       ) {
        binary_format = CHAZ_CC_BINFMT_ELF;
    }

    /* Macho-O. */
    if (binary_format == 0 && output_len >= 4
        && (memcmp(output, "\xCA\xFE\xBA\xBE", 4) == 0      /* Fat binary. */
            || memcmp(output, "\xFE\xED\xFA\xCE", 4) == 0   /* 32-bit BE. */
            || memcmp(output, "\xFE\xED\xFA\xCF", 4) == 0   /* 64-bit BE. */
            || memcmp(output, "\xCE\xFA\xED\xFE", 4) == 0   /* 32-bit LE. */
            || memcmp(output, "\xCF\xFA\xED\xFE", 4) == 0)  /* 64-bit LE. */
       ) {
        binary_format = CHAZ_CC_BINFMT_MACHO;
    }

    /* Portable Executable. */
    if (binary_format == 0 && output_len >= 0x40
        && memcmp(output, "MZ", 2) == 0
       ) {
        size_t pe_header_off =
            (unsigned char)output[0x3C]
            | ((unsigned char)output[0x3D] << 8)
            | ((unsigned char)output[0x3E] << 16)
            | ((unsigned char)output[0x3F] << 24);

        if (output_len >= pe_header_off + 4
            && memcmp(output + pe_header_off, "PE\0\0", 4) == 0
           ) {
            binary_format = CHAZ_CC_BINFMT_PE;
        }
    }

    chaz_CC.binary_format = binary_format;
    free(output);
}

int
chaz_CC_has_macro(const char *macro) {
    static const char template[] =
        CHAZ_QUOTE(  #ifdef %s             )
        CHAZ_QUOTE(  int i;                )
        CHAZ_QUOTE(  #else                 )
        CHAZ_QUOTE(  #error "nope"         )
        CHAZ_QUOTE(  #endif                );
    size_t size = sizeof(template)
                  + strlen(macro)
                  + 20;
    char *code = (char*)malloc(size);
    int retval = 0;
    sprintf(code, template, macro);
    retval = chaz_CC_test_compile(code);
    free(code);
    return retval;
}

int
chaz_CC_test_macro(const char *expression, const char *predicate) {
    static const char template[] =
        CHAZ_QUOTE(  #if (%s) %s           )
        CHAZ_QUOTE(  int i;                )
        CHAZ_QUOTE(  #else                 )
        CHAZ_QUOTE(  #error "nope"         )
        CHAZ_QUOTE(  #endif                );
    size_t size = sizeof(template)
                  + strlen(expression)
                  + strlen(predicate)
                  + 20;
    char *code = (char*)malloc(size);
    int retval = 0;
    sprintf(code, template, expression, predicate);
    retval = chaz_CC_test_compile(code);
    free(code);
    return retval;
}

static void
chaz_CC_detect_known_compilers(void) {
    chaz_CC.is_gcc   = chaz_CC_has_macro("__GNUC__");
    chaz_CC.is_msvc  = chaz_CC_has_macro("_MSC_VER");
    chaz_CC.is_clang = chaz_CC_has_macro("__clang__");
    chaz_CC.is_sun_c = chaz_CC_has_macro("__SUNPRO_C");
}

void
chaz_CC_clean_up(void) {
    free(chaz_CC.cc_command);
    free(chaz_CC.cflags);
    free(chaz_CC.try_exe_name);
    chaz_CFlags_destroy(chaz_CC.extra_cflags);
    chaz_CFlags_destroy(chaz_CC.temp_cflags);
}

int
chaz_CC_compile_exe(const char *source_path, const char *exe_name,
                    const char *code) {
    chaz_CFlags *local_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    const char *extra_cflags_string = "";
    const char *temp_cflags_string  = "";
    const char *local_cflags_string;
    char *exe_file = chaz_Util_join("", exe_name, chaz_CC.exe_ext, NULL);
    char *command;
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (chaz_CC.extra_cflags) {
        extra_cflags_string = chaz_CFlags_get_string(chaz_CC.extra_cflags);
    }
    if (chaz_CC.temp_cflags) {
        temp_cflags_string = chaz_CFlags_get_string(chaz_CC.temp_cflags);
    }
    chaz_CFlags_set_output_exe(local_cflags, exe_file);
    local_cflags_string = chaz_CFlags_get_string(local_cflags);
    command = chaz_Util_join(" ", chaz_CC.cc_command, chaz_CC.cflags,
                             source_path, extra_cflags_string,
                             temp_cflags_string, local_cflags_string, NULL);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        printf("%s\n", command);
        system(command);
    }

    if (chaz_CC_is_msvc()) {
        /* Zap MSVC junk. */
        size_t  junk_buf_size = strlen(exe_file) + 4;
        char   *junk          = (char*)malloc(junk_buf_size);
        sprintf(junk, "%s.obj", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.ilk", exe_name);
        chaz_Util_remove_and_verify(junk);
        sprintf(junk, "%s.pdb", exe_name);
        chaz_Util_remove_and_verify(junk);
        free(junk);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(exe_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    chaz_CFlags_destroy(local_cflags);
    free(command);
    free(exe_file);
    return result;
}

int
chaz_CC_compile_obj(const char *source_path, const char *obj_name,
                    const char *code) {
    chaz_CFlags *local_cflags = chaz_CFlags_new(chaz_CC.cflags_style);
    const char *extra_cflags_string = "";
    const char *temp_cflags_string  = "";
    const char *local_cflags_string;
    char *obj_file = chaz_Util_join("", obj_name, chaz_CC.obj_ext, NULL);
    char *command;
    int result;

    /* Write the source file. */
    chaz_Util_write_file(source_path, code);

    /* Prepare and run the compiler command. */
    if (chaz_CC.extra_cflags) {
        extra_cflags_string = chaz_CFlags_get_string(chaz_CC.extra_cflags);
    }
    if (chaz_CC.temp_cflags) {
        temp_cflags_string = chaz_CFlags_get_string(chaz_CC.temp_cflags);
    }
    chaz_CFlags_set_output_obj(local_cflags, obj_file);
    local_cflags_string = chaz_CFlags_get_string(local_cflags);
    command = chaz_Util_join(" ", chaz_CC.cc_command, chaz_CC.cflags,
                             source_path, extra_cflags_string,
                             temp_cflags_string, local_cflags_string, NULL);
    if (chaz_Util_verbosity < 2) {
        chaz_OS_run_quietly(command);
    }
    else {
        printf("%s\n", command);
        system(command);
    }

    /* See if compilation was successful.  Remove the source file. */
    result = chaz_Util_can_open_file(obj_file);
    if (!chaz_Util_remove_and_verify(source_path)) {
        chaz_Util_die("Failed to remove '%s'", source_path);
    }

    chaz_CFlags_destroy(local_cflags);
    free(command);
    free(obj_file);
    return result;
}

int
chaz_CC_test_compile(const char *source) {
    int compile_succeeded;
    char *try_obj_name
        = chaz_Util_join("", CHAZ_CC_TRY_BASENAME, chaz_CC.obj_ext, NULL);
    if (!chaz_Util_remove_and_verify(try_obj_name)) {
        chaz_Util_die("Failed to delete file '%s'", try_obj_name);
    }
    compile_succeeded = chaz_CC_compile_obj(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    chaz_Util_remove_and_verify(try_obj_name);
    free(try_obj_name);
    return compile_succeeded;
}

int
chaz_CC_test_link(const char *source) {
    int link_succeeded;
    if (!chaz_Util_remove_and_verify(chaz_CC.try_exe_name)) {
        chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_exe_name);
    }
    link_succeeded = chaz_CC_compile_exe(CHAZ_CC_TRY_SOURCE_PATH,
                                         CHAZ_CC_TRY_BASENAME, source);
    chaz_Util_remove_and_verify(chaz_CC.try_exe_name);
    return link_succeeded;
}

char*
chaz_CC_capture_output(const char *source, size_t *output_len) {
    char *captured_output = NULL;
    int compile_succeeded;

    /* Clear out previous versions and test to make sure removal worked. */
    if (!chaz_Util_remove_and_verify(chaz_CC.try_exe_name)) {
        chaz_Util_die("Failed to delete file '%s'", chaz_CC.try_exe_name);
    }
    if (!chaz_Util_remove_and_verify(CHAZ_CC_TARGET_PATH)) {
        chaz_Util_die("Failed to delete file '%s'", CHAZ_CC_TARGET_PATH);
    }

    /* Attempt compilation; if successful, run app and slurp output. */
    compile_succeeded = chaz_CC_compile_exe(CHAZ_CC_TRY_SOURCE_PATH,
                                            CHAZ_CC_TRY_BASENAME, source);
    if (compile_succeeded) {
        chaz_OS_run_local_redirected(chaz_CC.try_exe_name,
                                     CHAZ_CC_TARGET_PATH);
        captured_output = chaz_Util_slurp_file(CHAZ_CC_TARGET_PATH,
                                               output_len);
    }
    else {
        *output_len = 0;
    }

    /* Remove all the files we just created. */
    chaz_Util_remove_and_verify(CHAZ_CC_TRY_SOURCE_PATH);
    chaz_Util_remove_and_verify(chaz_CC.try_exe_name);
    chaz_Util_remove_and_verify(CHAZ_CC_TARGET_PATH);

    return captured_output;
}

const char*
chaz_CC_get_cc(void) {
    return chaz_CC.cc_command;
}

const char*
chaz_CC_get_cflags(void) {
    return chaz_CC.cflags;
}

chaz_CFlags*
chaz_CC_get_extra_cflags(void) {
    return chaz_CC.extra_cflags;
}

chaz_CFlags*
chaz_CC_get_temp_cflags(void) {
    return chaz_CC.temp_cflags;
}

chaz_CFlags*
chaz_CC_new_cflags(void) {
    return chaz_CFlags_new(chaz_CC.cflags_style);
}

int
chaz_CC_binary_format(void) {
    return chaz_CC.binary_format;
}

const char*
chaz_CC_exe_ext(void) {
    return chaz_CC.exe_ext;
}

const char*
chaz_CC_shared_lib_ext(void) {
    return chaz_CC.shared_lib_ext;
}

const char*
chaz_CC_static_lib_ext(void) {
    return chaz_CC.static_lib_ext;
}

const char*
chaz_CC_import_lib_ext(void) {
    return chaz_CC.import_lib_ext;
}

const char*
chaz_CC_obj_ext(void) {
    return chaz_CC.obj_ext;
}

int
chaz_CC_is_gcc(void) {
    return chaz_CC.is_gcc;
}

int
chaz_CC_is_msvc(void) {
    return chaz_CC.is_msvc;
}

int
chaz_CC_is_sun_c(void) {
    return chaz_CC.is_sun_c;
}

int
chaz_CC_is_cygwin(void) {
    return chaz_CC.is_cygwin;
}

int
chaz_CC_is_mingw(void) {
    return chaz_CC.is_mingw;
}

int
chaz_CC_test_gcc_version(const char *predicate) {
    static const char version[] =
        "10000 * __GNUC__ + 100 * __GNUC_MINOR__ + __GNUC_PATCHLEVEL__";
    return chaz_CC_test_macro(version, predicate);
}

int
chaz_CC_test_msvc_version(const char *predicate) {
    return chaz_CC_test_macro("_MSC_VER", predicate);
}

int
chaz_CC_test_sun_c_version(const char *predicate) {
    return chaz_CC_test_macro("__SUNPRO_C", predicate);
}

const char*
chaz_CC_link_command() {
    if (chaz_CC_is_msvc()) {
        return "link";
    }
    else {
        return chaz_CC.cc_command;
    }
}

char*
chaz_CC_format_archiver_command(const char *target, const char *objects) {
    if (chaz_CC_is_msvc()) {
        /* TODO: Write `objects` to a temporary file in order to avoid
         * exceeding line length limits. */
        char *out = chaz_Util_join("", "/OUT:", target, NULL);
        char *command = chaz_Util_join(" ", "lib", "/NOLOGO", objects, out,
                                       NULL);
        free(out);
        return command;
    }
    else {
        return chaz_Util_join(" ", "ar", "rcs", target, objects, NULL);
    }
}

char*
chaz_CC_format_ranlib_command(const char *target) {
    if (chaz_CC_is_msvc()) {
        return NULL;
    }
    return chaz_Util_join(" ", "ranlib", target, NULL);
}

char*
chaz_CC_shared_lib_filename(const char *dir, const char *basename,
                            const char *version) {
    /* Cygwin uses a "cyg" prefix for shared libraries. */
    const char *prefix = chaz_CC_is_msvc()
                         ? ""
                         : chaz_CC_is_cygwin() ? "cyg" : "lib";
    return chaz_CC_build_lib_filename(dir, prefix, basename, version,
                                      chaz_CC.shared_lib_ext);
}

char*
chaz_CC_import_lib_filename(const char *dir, const char *basename,
                            const char *version) {
    const char *prefix = chaz_CC_is_msvc() ? "" : "lib";
    return chaz_CC_build_lib_filename(dir, prefix, basename, version,
                                      chaz_CC.import_lib_ext);
}

char*
chaz_CC_export_filename(const char *dir, const char *basename,
                        const char *version) {
    /* Only for MSVC. */
    return chaz_CC_build_lib_filename(dir, "", basename, version, ".exp");
}

static char*
chaz_CC_build_lib_filename(const char *dir, const char *prefix,
                           const char *basename, const char *version,
                           const char *ext) {
    char *suffix;
    char *retval;

    if (version == NULL) {
        suffix = chaz_Util_strdup(ext);
    }
    else {
        int binary_format = chaz_CC_binary_format();

        if (binary_format == CHAZ_CC_BINFMT_PE) {
            suffix = chaz_Util_join("", "-", version, ext, NULL);
        }
        else if (binary_format == CHAZ_CC_BINFMT_MACHO) {
            suffix = chaz_Util_join("", ".", version, ext, NULL);
        }
        else if (binary_format == CHAZ_CC_BINFMT_ELF) {
            suffix = chaz_Util_join("", ext, ".", version, NULL);
        }
        else {
            chaz_Util_die("Unsupported binary format");
            return NULL;
        }
    }

    if (dir == NULL || strcmp(dir, ".") == 0) {
        retval = chaz_Util_join("", prefix, basename, suffix, NULL);
    }
    else {
        const char *dir_sep = chaz_OS_dir_sep();
        retval = chaz_Util_join("", dir, dir_sep, prefix, basename, suffix,
                                NULL);
    }

    free(suffix);
    return retval;
}

char*
chaz_CC_static_lib_filename(const char *dir, const char *basename) {
    const char *prefix = chaz_CC_is_msvc() ? "" : "lib";

    if (dir == NULL || strcmp(dir, ".") == 0) {
        return chaz_Util_join("", prefix, basename, chaz_CC.static_lib_ext,
                              NULL);
    }
    else {
        const char *dir_sep = chaz_OS_dir_sep();
        return chaz_Util_join("", dir, dir_sep, prefix, basename,
                              chaz_CC.static_lib_ext, NULL);
    }
}

