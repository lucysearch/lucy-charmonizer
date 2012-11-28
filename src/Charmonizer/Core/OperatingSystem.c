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

#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/ConfWriter.h"
#include "Charmonizer/Core/OperatingSystem.h"

static struct {
    char dev_null[20];
    char exe_ext[5];
    char obj_ext[5];
    char local_command_start[3];
    int  shell_type;
} chaz_OS = { "", "", "", "", 0 };
#define CHAZ_OS_POSIX    1
#define CHAZ_OS_CMD_EXE  2

void
chaz_OS_init(void) {
    if (chaz_Util_verbosity) {
        printf("Initializing Charmonizer/Core/OperatingSystem...\n");
    }

    if (chaz_Util_verbosity) {
        printf("Trying to find a bit-bucket a la /dev/null...\n");
    }

    /* Detect shell based on whether the bitbucket is "/dev/null" or "nul". */
    if (chaz_Util_can_open_file("/dev/null")) {
        strcpy(chaz_OS.dev_null, "/dev/null");
        strcpy(chaz_OS.exe_ext, "");
        strcpy(chaz_OS.obj_ext, "");
        strcpy(chaz_OS.local_command_start, "./");
        chaz_OS.shell_type = CHAZ_OS_POSIX;
    }
    else if (chaz_Util_can_open_file("nul")) {
        strcpy(chaz_OS.dev_null, "nul");
        strcpy(chaz_OS.exe_ext, ".exe");
        strcpy(chaz_OS.obj_ext, ".obj");
        strcpy(chaz_OS.local_command_start, ".\\");
        chaz_OS.shell_type = CHAZ_OS_CMD_EXE;
    }
    else {
        /* Bail out because we couldn't find anything like /dev/null. */
        chaz_Util_die("Couldn't find anything like /dev/null");
    }
}

const char*
chaz_OS_exe_ext(void) {
    return chaz_OS.exe_ext;
}

const char*
chaz_OS_obj_ext(void) {
    return chaz_OS.obj_ext;
}

const char*
chaz_OS_dev_null(void) {
    return chaz_OS.dev_null;
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
    int retval;

    static const size_t num_random_chars = 16;

    size_t  name_len = strlen(name);
    size_t  i;
    char   *temp_name = (char*)malloc(name_len + num_random_chars + 1);

    strcpy(temp_name, name);
    for (i = 0; i < num_random_chars; i++) {
        temp_name[name_len+i] = 'A' + rand() % 26;
    }
    temp_name[name_len+num_random_chars] = '\0';

    if (rename(name, temp_name) == 0) {
        retval = !remove(temp_name);
    }
    else {
        // Error during rename, remove using old name.
        retval = !remove(name);
    }

    free(temp_name);
    return retval;
}

int
chaz_OS_run_local_redirected(const char *command, const char *path) {
    size_t size = strlen(command) + strlen(chaz_OS.local_command_start) + 20;
    char *local_command = (char*)malloc(size);
    int retval;
    sprintf(local_command, "%s%s", chaz_OS.local_command_start, command);
    retval = chaz_OS_run_redirected(local_command, path);
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
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
        ) {
        char pattern[] = "%s > %s 2>&1";
        size_t size = sizeof(pattern)
                      + strlen(command)
                      + strlen(path)
                      + 10;
        quiet_command = (char*)malloc(size);
        sprintf(quiet_command, pattern, command, path);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    retval = system(quiet_command);
    free(quiet_command);
    return retval;
}

void
chaz_OS_mkdir(const char *filepath) {
    char *command = NULL;
    if (chaz_OS.shell_type == CHAZ_OS_POSIX
        || chaz_OS.shell_type == CHAZ_OS_CMD_EXE
       ) {
        unsigned size = sizeof("mkdir") + 1 + strlen(filepath) + 1;
        command = (char*)malloc(size);
        sprintf(command, "mkdir %s", filepath);
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
        unsigned size = strlen("rmdir") + 1 + strlen(filepath) + 1;
        command = (char*)malloc(size);
        sprintf(command, "rmdir %s", filepath);
    }
    else if (chaz_OS.shell_type == CHAZ_OS_CMD_EXE) {
        unsigned size = strlen("rmdir /q") + 1 + strlen(filepath) + 1;
        command = (char*)malloc(size);
        sprintf(command, "rmdir /q %s", filepath);
    }
    else {
        chaz_Util_die("Don't know the shell type");
    }
    chaz_OS_run_quietly(command);
    free(command);
}

