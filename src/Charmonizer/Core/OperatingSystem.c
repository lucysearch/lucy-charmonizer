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

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

#define CHAZ_OS_TARGET_PATH  "_charmonizer_target"
#define CHAZ_OS_NAME_MAX     31

static struct {
    char name[CHAZ_OS_NAME_MAX+1];
    char dev_null[20];
    char dir_sep[2];
    char local_command_start[3];
    int  shell_type;
    int  run_sh_via_cmd_exe;
} chaz_OS = { "", "", "", "", 0, 0 };

static int
chaz_OS_run_sh_via_cmd_exe(const char *command, const char *path);

void
chaz_OS_init(void) {
    char *output;
    size_t output_len;

    if (chaz_Util_verbosity) {
        printf("Initializing Charmonizer/Core/OperatingSystem...\n");
    }

    /* Detect shell based on escape character. */

    /* Needed to make redirection work. */
    chaz_OS.shell_type = CHAZ_OS_POSIX;

    output = chaz_OS_run_and_capture("echo foo\\^bar", &output_len);

    if (output_len >= 7 && memcmp(output, "foo\\bar", 7) == 0) {
        /* Escape character is caret. */
        if (chaz_Util_verbosity) {
            printf("Detected cmd.exe shell\n");
        }

        /* Try to see whether running commands via the `sh` command works.
         * Run the `find` command to check whether we're in a somewhat POSIX
         * compatible environment. */
        free(output);
        chaz_OS.run_sh_via_cmd_exe = 1;
        output = chaz_OS_run_and_capture("find . -prune", &output_len);

        if (output_len >= 2
            && output[0] == '.'
            && isspace((unsigned char)output[1])
           ) {
            if (chaz_Util_verbosity) {
                printf("Detected POSIX shell via cmd.exe\n");
            }
            chaz_OS.shell_type = CHAZ_OS_POSIX;
        }
        else {
            chaz_OS.shell_type = CHAZ_OS_CMD_EXE;
            chaz_OS.run_sh_via_cmd_exe = 0;
        }

        /* Redirection is always run through cmd.exe. */
        strcpy(chaz_OS.dev_null, "nul");
    }
    else if (output_len >= 7 && memcmp(output, "foo^bar", 7) == 0) {
        /* Escape character is backslash. */
        if (chaz_Util_verbosity) {
            printf("Detected POSIX shell\n");
        }
        chaz_OS.shell_type = CHAZ_OS_POSIX;
        strcpy(chaz_OS.dev_null, "/dev/null");
    }

    if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        strcpy(chaz_OS.dir_sep, "\\");
        /* Empty string should work, too. */
        strcpy(chaz_OS.local_command_start, ".\\");
    }
    else if (chaz_OS.shell_type == CHAZ_OS_POSIX) {
        strcpy(chaz_OS.dir_sep, "/");
        strcpy(chaz_OS.local_command_start, "./");
    }
    else {
        chaz_Util_die("Couldn't identify shell");
    }

    free(output);
}

const char*
chaz_OS_dev_null(void) {
    return chaz_OS.dev_null;
}

const char*
chaz_OS_dir_sep(void) {
    return chaz_OS.dir_sep;
}

int
chaz_OS_shell_type(void) {
    return chaz_OS.shell_type;
}

const char*
chaz_OS_exe_ext(void) {
#ifdef _WIN32
    return ".exe";
#else
    return "";
#endif
}

int
chaz_OS_remove(const char *name) {
    /*
     * On Windows it can happen that another process, typically a
     * virus scanner, still has an open handle on the file. This can
     * make the subsequent recreation of a file with the same name
     * fail. As a workaround, files are renamed to a random name
     * before deletion.
     */
    int retval = 0;

    static const size_t num_random_chars = 16;

    size_t  name_len = strlen(name);
    size_t  i;
    char   *temp_name = (char*)malloc(name_len + num_random_chars + 1);
    const char *working_name = name;
    clock_t start, now;

    strcpy(temp_name, name);
    for (i = 0; i < num_random_chars; i++) {
        temp_name[name_len+i] = 'A' + rand() % 26;
    }
    temp_name[name_len+num_random_chars] = '\0';

    /* Try over and over again for around 1 second to rename the file.
     * Ideally we would sleep between attempts, but sleep functionality is not
     * portable. */
    start = now = clock();
    while (now - start < CLOCKS_PER_SEC) {
        now = clock();
        if (!rename(name, temp_name)) {
            /* The rename succeeded. */
            working_name = temp_name;
            break;
        }
        else if (errno == ENOENT) {
            /* No such file or directory, so no point in trying to remove it.
             * (Technically ENOENT is POSIX but hopefully this works.) */
            free(temp_name);
            return 0;
        }
    }

    /* Try over and over again for around 1 second to delete the file. */
    start = now = clock();
    while (!retval && now - start < CLOCKS_PER_SEC) {
        now = clock();
        retval = !remove(working_name);
    }

    free(temp_name);
    return retval;
}

int
chaz_OS_run_local_redirected(const char *command, const char *path) {
    char *local_command
        = chaz_Util_join("", chaz_OS.local_command_start, command, NULL);
    int retval = chaz_OS_run_redirected(local_command, path);
    free(local_command);
    return retval;
}

int
chaz_OS_run_quietly(const char *command) {
    return chaz_OS_run_redirected(command, chaz_OS.dev_null);
}

int
chaz_OS_run_redirected(const char *command, const char *path) {
    int retval = 1;
    char *quiet_command = NULL;
    if (chaz_OS.run_sh_via_cmd_exe) {
        return chaz_OS_run_sh_via_cmd_exe(command, path);
    }
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
        ) {
        quiet_command = chaz_Util_join(" ", command, ">", path, "2>&1", NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    retval = system(quiet_command);
    free(quiet_command);
    return retval;
}

static int
chaz_OS_run_sh_via_cmd_exe(const char *command, const char *path) {
    size_t i;
    size_t size;
    char *escaped_command;
    char *wrapped_command;
    char *p;
    int retval;

    /* Compute size. */

    size = 0;

    for (i = 0; command[i] != '\0'; i++) {
        char c = command[i];

        switch (c) {
            case '"':
            case '\\':
                size += 2;
                break;

            case '%':
            case '!':
                size += 3;
                break;

            default:
                size += 1;
                break;
        }
    }

    /* Build sh command. */

    escaped_command = (char*)malloc(size + 1);
    p = escaped_command;

    /* Escape special characters. */

    for (i = 0; command[i] != '\0'; i++) {
        char c = command[i];

        switch (c) {
            case '"':
            case '\\':
                /* Escape double quote and backslash. */
                *p++ = '\\';
                *p++ = c;
                break;

            case '%':
            case '!':
                /* Break out of double quotes for percent sign and
                 * exclamation mark. This prevents variable expansion. */
                *p++ = '"';
                *p++ = c;
                *p++ = '"';
                break;

            default:
                *p++ = c;
                break;
        }
    }

    *p++ = '\0';

    /* Run sh command. */

    wrapped_command = chaz_Util_join("", "sh -c \"", escaped_command, "\" > ",
                                     path, " 2>&1", NULL);
    retval = system(wrapped_command);

    free(wrapped_command);
    free(escaped_command);
    return retval;
}

char*
chaz_OS_run_and_capture(const char *command, size_t *output_len) {
    char *output;
    chaz_OS_run_redirected(command, CHAZ_OS_TARGET_PATH);
    output = chaz_Util_slurp_file(CHAZ_OS_TARGET_PATH, output_len);
    chaz_Util_remove_and_verify(CHAZ_OS_TARGET_PATH);
    return output;
}

void
chaz_OS_mkdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
       ) {
        command = chaz_Util_join(" ", "mkdir", filepath, NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}

void
chaz_OS_rmdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX) {
        command = chaz_Util_join(" ", "rmdir", filepath, NULL);
    }
    else if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        command = chaz_Util_join(" ", "rmdir", "/q", filepath, NULL);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}

