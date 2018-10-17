#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tst.h"
#include "bloom.c"

#define DotStepSize 10000
#define DotStart 20000
#define TableSize 5000000
#define HashNumber 2

enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };

#define REF INS
#define CPY DEL

#define IN_FILE "cities.txt"
#define OUT_INS_TIME "ins_time.txt"

long poolsize = 2000000 * WRDMAX;

/* trim '\n' from end of buffer filled by fgets */
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}

/* initialize tst with bloom filter
   if init success, return time consumed in second
   if init fail, return -1 */
int init_tst_bloom(FILE *fpin, bloom_t bloom, tst_node *root);

/* initialize tst without bloom filter
   if init success, return time consumed in second
   if init fail, return -1 */
int init_tst_wbloom(FILE *fpin, tst_node *root);

int main(int argc, char **argv)
{
    char word[WRDMAX] = "";
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    int rtn = 0, idx = 0, sidx = 0;
    double t1, t2;

    FILE *fpin = fopen(IN_FILE, "r");
    FILE *fpout_ins = fopen(OUT_INS_TIME, "w");

    if (!fpin) {
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        fclose(fpout_ins);
        return 1;
    }

    if (!fpout_ins) {
        fprintf(stderr, "error: file write failed '%s'.\n", OUT_INS_TIME);
        fclose(fpin);
        return 1;
    }

    /* insert time of none-bloom tst */
    /* insert time of bloom tst */

    char *pool = (char *) malloc(poolsize * sizeof(char));
    char *Top = pool;
    while ((rtn = fscanf(fp, "%s", Top)) != EOF) {
        char *p = Top;

        /* insert refence to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {
            /* failed to insert*/
            fprintf(stderr, "")
        }
    }

    fclose(fpout_ins);
    fclose(fpin);
}