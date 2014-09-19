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

#include <string.h>
#include <stdlib.h>
#include "Charmonizer/Core/Library.h"
#include "Charmonizer/Core/Compiler.h"
#include "Charmonizer/Core/Util.h"
#include "Charmonizer/Core/OperatingSystem.h"

struct chaz_Lib {
    char *name;
    char *version;
    char *major_version;
};

static char*
S_build_filename(chaz_Lib *lib, const char *version, const char *ext);

static const char*
S_get_prefix(void);

chaz_Lib*
chaz_Lib_new(const char *name, const char *version,
                   const char *major_version) {
    chaz_Lib *lib = (chaz_Lib*)malloc(sizeof(chaz_Lib));
    lib->name          = chaz_Util_strdup(name);
    lib->version       = chaz_Util_strdup(version);
    lib->major_version = chaz_Util_strdup(major_version);
    return lib;
}

void
chaz_Lib_destroy(chaz_Lib *lib) {
    free(lib->name);
    free(lib->version);
    free(lib->major_version);
    free(lib);
}

const char*
chaz_Lib_get_name(chaz_Lib *lib) {
    return lib->name;
}

const char*
chaz_Lib_get_version(chaz_Lib *lib) {
    return lib->version;
}

const char*
chaz_Lib_get_major_version(chaz_Lib *lib) {
    return lib->major_version;
}

char*
chaz_Lib_filename(chaz_Lib *lib) {
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    if (strcmp(shlib_ext, ".dll") == 0) {
        return S_build_filename(lib, lib->major_version, shlib_ext);
    }
    else {
        return S_build_filename(lib, lib->version, shlib_ext);
    }
}

char*
chaz_Lib_major_version_filename(chaz_Lib *lib) {
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    return S_build_filename(lib, lib->major_version, shlib_ext);
}

char*
chaz_Lib_no_version_filename(chaz_Lib *lib) {
    const char *prefix    = S_get_prefix();
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    return chaz_Util_join("", prefix, lib->name, shlib_ext, NULL);
}

char*
chaz_Lib_implib_filename(chaz_Lib *lib) {
    return S_build_filename(lib, lib->major_version, ".lib");
}

char*
chaz_Lib_export_filename(chaz_Lib *lib) {
    return S_build_filename(lib, lib->major_version, ".exp");
}

static char*
S_build_filename(chaz_Lib *lib, const char *version, const char *ext) {
    const char *prefix    = S_get_prefix();
    const char *shlib_ext = chaz_OS_shared_lib_ext();

    if (strcmp(shlib_ext, ".dll") == 0) {
        return chaz_Util_join("", prefix, lib->name, "-", version, ext, NULL);
    }
    else if (strcmp(shlib_ext, ".dylib") == 0) {
        return chaz_Util_join("", prefix, lib->name, ".", version, ext, NULL);
    }
    else {
        return chaz_Util_join("", prefix, lib->name, ext, ".", version, NULL);
    }
}

static const char*
S_get_prefix() {
    if (chaz_CC_msvc_version_num()) {
        return "";
    }
    else if (chaz_OS_is_cygwin()) {
        return "cyg";
    }
    else {
        return "lib";
    }
}


