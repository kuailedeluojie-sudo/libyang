/**
 * @file completion.c
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief libyang's yanglint tool auto completion
 *
 * Copyright (c) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 */

#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#include "commands.h"
#include "../../linenoise/linenoise.h"

void
get_cmd_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    int i;

    *match_count = 0;
    *matches = NULL;

    for (i = 0; commands[i].name; i++) {
        if (!strncmp(hint, commands[i].name, strlen(hint))) {
            ++(*match_count);
            *matches = realloc(*matches, *match_count * sizeof **matches);
            (*matches)[*match_count-1] = strdup(commands[i].name);
        }
    }
}

void
get_path_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    const char *ptr, *path;
    DIR *dir;
    struct dirent *ent;

    *match_count = 0;
    *matches = NULL;

    path = strchr(hint, ' ');
    ++path;
    ptr = strrchr(path, '/');

    /* new relative path */
    if (ptr == NULL) {
        ptr = path;
        dir = opendir(".");
    } else {
        ++ptr;
        dir = opendir(strndupa(path, ptr-path));
    }

    if (dir == NULL) {
        fprintf(stderr, "opendir failed (%s)\n", strerror(errno));
        return;
    }

    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.') {
            continue;
        }

        /* some serious pointer fun */
        if (!strncmp(ptr, ent->d_name, strlen(ptr))) {
            ++(*match_count);
            *matches = realloc(*matches, *match_count * sizeof **matches);
            //asprintf(&(*matches)[*match_count-1], "%.*s%s", (int)(ptr-hint), hint, ent->d_name);
            (*matches)[*match_count-1] = malloc((ptr-hint)+strlen(ent->d_name)+1);
            strncpy((*matches)[*match_count-1], hint, ptr-hint);
            strcpy((*matches)[*match_count-1]+(ptr-hint), ent->d_name);
        }
    }

    closedir(dir);
}

void
get_print_completion(const char *hint, char ***matches, unsigned int *match_count)
{
    /* TODO */
    (void)hint;
    (void)matches;
    (void)match_count;
}

void
complete_cmd(const char *buf, linenoiseCompletions *lc)
{
    char **matches = NULL;
    unsigned int match_count = 0, i;

    if (!strncmp(buf, "add ", 4) || !strncmp(buf, "searchpath ", 11)) {
        get_path_completion(buf, &matches, &match_count);
    } else if (!strncmp(buf, "print ", 6)) {
        get_print_completion(buf, &matches, &match_count);
    } else {
        get_cmd_completion(buf, &matches, &match_count);
    }

    for (i = 0; i < match_count; ++i) {
        linenoiseAddCompletion(lc, matches[i]);
        free(matches[i]);
    }
    free(matches);
}