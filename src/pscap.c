/*
 * MIT License
 *
 * Copyright (c) 2021 Arthur Guyader
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <ctype.h>
#include <dirent.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/capability.h>
#include <sys/types.h>
#include <unistd.h>

#define PSCAP_NAME "pscap"
#define PSCAP_VERSION "1.0"

#define AUTHORS "Arthur Guyader"
#define PROC_PATH "/proc"
#define CAP_MAX 0x1FFFFFFFFFF

static struct option const longopts[] = {{"human", no_argument, NULL, 'H'},   {"all", no_argument, NULL, 'a'},
                                         {"line", no_argument, NULL, 'l'},    {"help", no_argument, NULL, 'h'},
                                         {"version", no_argument, NULL, 'v'}, {NULL, 0, NULL, 0}};

int get_number(char *str) {
    int n;
    if (sscanf(str, "%d", &n) == 0) {
        return -1;
    }
    return n;
}

unsigned long long get_hex(char *str) { return strtoull(str, NULL, 16); }

void print_human_cap(unsigned long long value) {  //
    // This function is under the Copyright :
    // Copyright (c) 2008-11 Andrew G. Morgan <morgan@kernel.org>
    // And was recovered in the file capsh.c
    if (value == 0) {
        puts("NONE");
    } else if (value == CAP_MAX) {
        puts("ALL");
    } else {
        unsigned cap;
        const char *sep = "";
        for (cap = 0; (cap < 64) && (value >> cap); ++cap) {
            if (value & (1ULL << cap)) {
                char *ptr;
                ptr = cap_to_name((int)cap);
                if (ptr != NULL) {
                    printf("%s%s", sep, ptr);
                    cap_free(ptr);
                } else {
                    printf("%s%u", sep, cap);
                }
                sep = ",";
            }
        }
        puts("");
    }
}

void print_cap(char *name, unsigned long long value, bool human) {
    printf("%s: ", name);
    if (human) {
        print_human_cap(value);
    } else {
        printf("0x%016llx\n", value);
    }
}

char *trim(char *str) {
    while (isblank(*str)) str++;

    char *end = str + strlen(str) - 1;
    while (isblank(*end) || *end == '\n') end--;
    *(end + 1) = '\0';
    return str;
}

int parse_status_file(pid_t pid, char **cmd, pid_t *tid, unsigned long long *eff, unsigned long long *per,
                      unsigned long long *ihn, unsigned long long *amb, unsigned long long *bound) {
    FILE *f;
    int s = 0;
    char path[255];
    char buf[255];

    // check directory exist
    s = snprintf(path, 255, "%s/%d", PROC_PATH, pid);
    if (s < 0 || s >= 255) {
        fprintf(stderr, "buffer error\n");
        return -1;
    }
    DIR *dir = opendir(path);
    if (!dir) {
        fprintf(stderr, "process not found\n");
        return -1;
    }
    closedir(dir);

    // open file
    s = snprintf(path, 255, "%s/%d/status", PROC_PATH, pid);
    if (s < 0 || s >= 255) {
        fprintf(stderr, "buffer error\n");
        return -1;
    }

    f = fopen(path, "r");

    while (fgets(buf, 255, (FILE *)f) != NULL) {
        if (strstr(buf, "Name:") != NULL) {
            *cmd = strdup(trim(buf + 5));
        } else if (strstr(buf, "Tgid:") != NULL) {
            *tid = get_number(trim(buf + 5));
        } else if (strstr(buf, "CapInh:") != NULL) {
            *ihn = get_hex(trim(buf + 7));
        } else if (strstr(buf, "CapPrm:") != NULL) {
            *per = get_hex(trim(buf + 7));
        } else if (strstr(buf, "CapEff:") != NULL) {
            *eff = get_hex(trim(buf + 7));
        } else if (strstr(buf, "CapBnd:") != NULL) {
            *bound = get_hex(trim(buf + 7));
        } else if (strstr(buf, "CapAmb:") != NULL) {
            *amb = get_hex(trim(buf + 7));
        }
    }
    return 0;
}

int display_pid(pid_t pid, bool human, bool line) {
    // data
    char *cmd;
    pid_t tid;
    unsigned long long eff;
    unsigned long long per;
    unsigned long long inh;
    unsigned long long amb;
    unsigned long long bound;
    //

    if (parse_status_file(pid, &cmd, &tid, &eff, &per, &inh, &amb, &bound) < 0) {
        return -1;
    }

    if (line) {
        printf("%d\t%d\t%.7s\t\t0x%016llx\t0x%016llx\t0x%016llx\t0x%016llx\t0x%016llx\n", pid, tid, cmd, eff, per, inh,
               amb, bound);
    } else {
        printf("pid: %d \t tid: %d\n", pid, tid);
        printf("cmd: %s\n", cmd);
        print_cap("CapEff", eff, human);
        print_cap("CapPrm", per, human);
        print_cap("CapInh", inh, human);
        print_cap("CapAmb", amb, human);
        print_cap("CapBnd", bound, human);
    }

    free(cmd);
    return 0;
}

int display_all(bool human, bool line) {
    DIR *proc;
    struct dirent *d;
    pid_t pid;
    proc = opendir(PROC_PATH);
    const char *sep = "";
    if (proc) {
        while ((d = readdir(proc)) != NULL) {
            pid = get_number(d->d_name);
            if (pid != -1) {
                if (!line) printf(sep);
                if (display_pid(pid, human, line) < 0) {
                    return -1;
                }
            }
            sep = "**************************\n";
        }
    }
    closedir(proc);
    return 0;
}

void usage(int status) {
    if (status != EXIT_SUCCESS)
        fprintf(stderr, ("Try `%s --help' for more information.\n"), PSCAP_NAME);
    else {
        printf(("Usage: %s [OPTION]... [PID]\n"), PSCAP_NAME);
        fputs(("\
Print process capabilities, default print current PID\n\
\n\
OPTIONS :\n\
  -H  --human       print human readable\n\
  -a, --all         print all process\n\
  -l, --line        print on one line\n\
  -h, --help        print all group IDs\n\
  -v, --version     print version\n\
"),
              stdout);
    }
    exit(status);
}

void print_version() {
    puts(PSCAP_VERSION);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    pid_t pid = 0;
    int optc;
    bool human = false;
    bool all = false;
    bool line = false;
    int rc = 0;

    while ((optc = getopt_long(argc, argv, "Halhv", longopts, NULL)) != -1) {
        switch (optc) {
            case 'H':
                human = true;
                break;
            case 'a':
                all = true;
                break;
            case 'l':
                line = true;
                break;
            case 'h':
                usage(EXIT_SUCCESS);
                break;
            case 'v':
                print_version();
                break;
            default:
                usage(EXIT_FAILURE);
        }
    }

    if (argc - optind > 1) {
        fprintf(stderr, ("extra operand '%s' "), argv[optind + 1]);
        usage(EXIT_FAILURE);
    } else if (argc - optind == 1) {
        pid = get_number(argv[optind]);
        if (pid == -1) {
            usage(EXIT_FAILURE);
        }
    } else {
        pid = getppid();
    }

    if (human) {
        line = false;
    } else if (line) {
        puts("PID\tTID\tCOMMAND\t\tEFFECTIVE\t\tPERMITTED\t\tINHERITABL\t\tAMBIENT\t\t\tBOUND");
    }

    if (all) {
        rc = display_all(human, line);
    } else {
        rc = display_pid(pid, human, line);
    }
    exit(rc);
}