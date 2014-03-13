/*
 * File name: gen-ftype.c
 * Description:
 *   Generate a table to translate stat() S_IFMT values
 *   to 1-character file type a la ls -l.
 *
 * Copyright 2014 Guy Shaw
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>             // malloc(), exit()
#include <string.h>             // memset()
#include <getopt.h>		// getopt_long()
#include <sys/stat.h>

#if defined(__GNUC__)

#define eprintf(fmt, ...) \
    fprintf(stderr, fmt, ## __VA_ARGS__)

#else

extern void eprintf(const char *fmt, ...);

#endif

typedef unsigned int uint_t;
typedef unsigned int bool_t;

static bool_t opt_verbose = false;
static char *ftype_translate = NULL;
static uint_t ifmt_shift;


#if defined(__GNUC__)

#define ISPOWEROF2(expr) ({ typeof(expr) n = (expr); (n & (n - 1)) == 0; })

#else

#define ISPOWEROF2(n) (((n) & ((n) - 1)) == 0)

#endif

/*
 * A value is a simple mask if it contains a single run of 1 bits.
 * A word can be interpreted as being partitioned into fields.
 * Each of its fields can be extracted by shifting and masking.
 * The bit mask used to extract a single field would have just one
 * run of contiguous 1 bits.  Anything more complicated than that
 * would be multiple masks representing multiple fields that would
 * have to be extracted and concatenated.  The logic to build a
 * table of single-character file types is a single mask and shift.
 * So, we do not know what to do if the value of S_IFMT is anything
 * is more complicated than a single bit field.
 */
static inline bool_t
is_simple_mask(uint_t mask)
{
    return (ISPOWEROF2(mask + 1));
}

#if 0
static inline uint_t
mode_to_ftype_enum(int imode)
{
    uint_t mode = imode;
    return ((mode & S_IFMT) >> ifmt_shift);
}
#endif

void
add_ftype(char *table, uint_t pos, int chr)
{
    int old_chr;

    old_chr = table[pos];
    if (old_chr == '?') {
        table[pos] = chr;
    }
    else {
        eprintf("ERROR: collision, position %u, '%c' vs '%c'\n",
            pos, chr, old_chr);
    }
}

void
show_argv(int iargc, char * const *argv)
{
    char nfmt[20];
    uint_t argc;
    uint_t maxw;
    uint_t i;

    argc = (uint_t)iargc;
    sprintf(nfmt, "%u", argc);
    maxw = strlen(nfmt);
    sprintf(nfmt, "%%%uu", maxw);
    for (i = 0; i < argc; ++i) {
        printf(nfmt, i);
        printf(") [%s]\n", argv[i]);
    }
}

void
show_languages(void) {
    printf("Known programming languages are:\n");
    printf("    C, D, perl\n");
}


void
build_ftype_table(char *ftype_table, size_t ftype_table_size)
{
#if defined(S_IFIFO)
    add_ftype(ftype_table, S_IFIFO >> ifmt_shift, 'p');
#endif

#if defined(S_IFCHR)
    add_ftype(ftype_table, S_IFCHR >> ifmt_shift, 'c');
#endif

#if defined(S_IFDIR)
    add_ftype(ftype_table, S_IFDIR >> ifmt_shift, 'd');
#endif

#if defined(S_IFBLK)
    add_ftype(ftype_table, S_IFBLK >> ifmt_shift, 'b');
#endif

#if defined(S_IFREG)
    add_ftype(ftype_table, S_IFREG >> ifmt_shift, '-');
#endif

#if defined(S_IFLNK)
    add_ftype(ftype_table, S_IFLNK >> ifmt_shift, 'l');
#endif

#if defined(S_IFSOCK)
    add_ftype(ftype_table, S_IFSOCK >> ifmt_shift, 's');
#endif

#if defined(S_IFDOOR)
    add_ftype(ftype_table, S_IFDOOR >> ifmt_shift, 'D');
#endif

/*
 * Solaris event port.
 */
#if defined(S_IFPORT)
    add_ftype(ftype_table, S_IFPORT >> ifmt_shift, 'E');
#endif

/*
 * BSD whiteout
 */
#if defined(S_IFWHT)
    add_ftype(ftype_table, S_IFWHT >> ifmt_shift, 'w');
#endif

/*
 * HP-UX Network Special
 */
#if defined(S_IFNWK)
    add_ftype(ftype_table, S_IFNWK >> ifmt_shift, 'n');
#endif

    /*
     * Post-process the file type decoding table with any given translations
     */
    if (ftype_translate != NULL) {
        size_t i;
        size_t tr_size;
        size_t ti;

        tr_size = strlen(ftype_translate);
        for (i = 0; i < ftype_table_size; ++i) {
            for (ti = 0; ti < tr_size; ti += 2) {
                if (ftype_table[i] == ftype_translate[ti]) {
                    ftype_table[i] = ftype_translate[ti + 1];
                }
            }
        }
    }
}

/*
 * Generate C code for function, 'mode_to_ftype'.
 */
static void
generate_c(const char *ftype_table, size_t ftype_table_size, unsigned int ifmt_shift)
{
    if (opt_verbose) {
        printf("// INFO: ftype_table_size = %u\n", ftype_table_size);
        printf("// INFO: ftype_table = q[%s]\n", ftype_table);
        printf("// INFO: S_IFMT = 0x%x\n", S_IFMT);
        printf("// INFO: ifmt_shift = %u\n", ifmt_shift);
        printf("\n");
    }

    printf("static const char *ftype_table = \"%s\";\n", ftype_table);
    printf("\n");
    printf("static inline unsigned int\n");
    printf("extract_bitfield(unsigned int wrd, unsigned int msk, unsigned int shft)\n");
    printf("{\n");
    printf("    return ((wrd & msk) >> shft);\n");
    printf("}\n");
    printf("\n");
    printf("mode_to_ftype(int m)\n");
    printf("{\n");
    printf("    unsigned int pos = extract_bitfield(m, S_IFMT, ifmt_shift);\n");
    printf("    return (ftype_table[pos]);\n");
    printf("}\n");
    printf("\n");
    printf("#define mode_to_filetype(m) (\"%s\"[((m) & 0x%x) >> %u])\n",
        ftype_table, S_IFMT, ifmt_shift);
}

/*
 * Generate D code for function, 'mode_to_ftype'.
 */
static void
generate_d(const char *ftype_table, size_t ftype_table_size, unsigned int ifmt_shift)
{
    if (opt_verbose) {
        printf("// INFO: ftype_table_size = %u\n", ftype_table_size);
        printf("// INFO: ftype_table = q[%s]\n", ftype_table);
        printf("// INFO: S_IFMT = 0x%x\n", S_IFMT);
        printf("// INFO: ifmt_shift = %u\n", ifmt_shift);
        printf("\n");
    }

    printf("static const char *ftype_table = \"%s\";\n", ftype_table);
    printf("\n");
    printf("static inline unsigned int\n");
    printf("extract_bitfield(unsigned int wrd, unsigned int msk, unsigned int shft)\n");
    printf("{\n");
    printf("    return ((wrd & msk) >> shft);\n");
    printf("}\n");
    printf("\n");
    printf("mode_to_ftype(int m)\n");
    printf("{\n");
    printf("    unsigned int pos = extract_bitfield(m, S_IFMT, ifmt_shift);\n");
    printf("    return (ftype_table[pos]);\n");
    printf("}\n");
    printf("\n");
    printf("#define mode_to_filetype(m) (\"%s\"[((m) & 0x%x) >> %u])\n",
        ftype_table, S_IFMT, ifmt_shift);
}

/*
 * Generate Perl code for function, 'mode_to_ftype'.
 */
static void
generate_perl(const char *ftype_table, size_t ftype_table_size, unsigned int ifmt_shift)
{
    if (opt_verbose) {
        printf("# INFO: ftype_table_size = %u\n", ftype_table_size);
        printf("# INFO: ftype_table = q[%s]\n", ftype_table);
        printf("# INFO: S_IFMT = 0x%x\n", S_IFMT);
        printf("# INFO: ifmt_shift = %u\n", ifmt_shift);
        printf("\n");
    }

   printf("sub mode_to_ftype { substr(\"%s\", ($_[0] & 0x%x) >> %u, 1); }\n",
       ftype_table, S_IFMT, ifmt_shift);
}

enum opt  { OPT_VERBOSE = 10000, OPT_LANGUAGE, OPT_TRANSLATE };
enum lang { LANG_UNSPECIFIED, LANG_C, LANG_D, LANG_PERL };

typedef enum opt  opt_t;
typedef enum lang language_t;

static language_t language = LANG_UNSPECIFIED;

static struct option long_options[] = {
    {"verbose",   no_argument,       0, OPT_VERBOSE  },
    {"language",  required_argument, 0, OPT_LANGUAGE },
    {"translate", required_argument, 0, OPT_TRANSLATE },
    {NULL,        0,                 0, 0 }
};

int
main(int argc, char * const *argv)
{
    char *ftype_table;
    size_t ftype_table_size;
    uint_t msk;

    int c;

    while (1) {
        int option_index = 0;

#if 0
eprintf("getopt_long: argc=%d, optind=%d, argv[%d]=[%s]\n",
argc, optind, optind, argv[optind]);
#endif
        c = getopt_long(argc, argv, "", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            printf("option %s", long_options[option_index].name);
            if (optarg) {
                printf(" with arg %s", optarg);
            }
            printf("\n");
            break;

        case OPT_VERBOSE:
            opt_verbose = true;
            break;

        case OPT_LANGUAGE:
            if (optarg == NULL) {
                printf("--language: no language specified.\n");
                show_languages();
                exit(2);
            }

#if 0
eprintf("language=[%s]\n", optarg);
#endif
            if (strcmp(optarg, "C") == 0) {
                language = LANG_C;
            }
            else if (strcmp(optarg, "D") == 0) {
                language = LANG_D;
            }
            else if (strcmp(optarg, "perl") == 0) {
                language = LANG_PERL;
            }
            else {
                printf("%s: unknown programming language, '%s'.\n",
                    argv[optind], optarg);
                show_languages();
                exit(2);
            }
            break;

        case OPT_TRANSLATE:
            if (optarg == NULL) {
                printf("%s: no argument.\n", argv[optind]);
                exit(2);
            }
            ftype_translate = optarg;
            break;

        case '?':
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc) {
            printf("%s ", argv[optind++]);
        }
        printf("\n");
    }

    msk = S_IFMT;
    ifmt_shift = 0;
    while ((msk & 1) == 0) {
        msk >>= 1U;
        ++ifmt_shift;
    }

    if (!is_simple_mask(msk)) {
        eprintf("S_IFMT = 0x%x\n", S_IFMT);
        eprintf("S_IFMT must be a simple mask.\n");
        eprintf("That is, it must be a mask that has a single run of 1 bits.\n");
        exit(2);
    }

    ftype_table_size = msk + 1;
    ftype_table = (char *)malloc(ftype_table_size + 1);

    /*
     * Build the table of binary file type to single-letter mnemonic types.
     * Start off with all unknown file types.
     * Fill in values according to what S_* preprocessor symbols are defined.
     */
    memset(ftype_table, '?', ftype_table_size);
    ftype_table[ftype_table_size] = '\0';
    build_ftype_table(ftype_table, ftype_table_size);

    switch (language) {
    case LANG_C:
        generate_c(ftype_table, ftype_table_size, ifmt_shift);
        break;
    case LANG_D:
        generate_d(ftype_table, ftype_table_size, ifmt_shift);
        break;
    case LANG_PERL:
        generate_perl(ftype_table, ftype_table_size, ifmt_shift);
        break;
    default:
        eprintf("ERROR: Unknown language, %u\n", language);
        exit(2);
    }
    exit(0);
}
