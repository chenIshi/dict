#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tst.h"
#include "bloom.h"
#include "bench.c"

#define DotStepSize 10000
#define DotStart 20000
#define DotNum (2400000 / DotStepSize)
#define TableSize 5000000
#define HashNumber 2

enum { INS, DEL, WRDMAX = 256, STKMAX = 512, LMAX = 1024 };

#define REF INS
#define CPY DEL

#define IN_FILE "cities.txt"
#define OUT_INS_TIME_BLOOM "ins_time_bloom.txt"
#define OUT_INS_TIME_WOBLOOM "ins_time_wobloom.txt"
#define OUT_SEARCH_TIME_BLOOM "search_time_bloom.txt"
#define OUT_SEARCH_TIME_WOBLOOM "search_time_wobloom.txt"

long poolsize = 2000000 * WRDMAX;

/* trim '\n' from end of buffer filled by fgets */
/*
static void rmcrlf(char *s)
{
    size_t len = strlen(s);
    if (len && s[len - 1] == '\n')
        s[--len] = 0;
}
*/

/* initialize tst with bloom filter
   if init success, return time consumed in second
   if init fail, return -1 */
int init_tst_bloom(FILE *fpin, bloom_t bloom, tst_node *root, size_t size);

/* initialize tst without bloom filter
   if init success, return time consumed in second
   if init fail, return -1 */
int init_tst_wbloom(FILE *fpin, tst_node *root, size_t size);

int main(int argc, char **argv)
{
    /*
    char word[WRDMAX] = "";
    char *sgl[LMAX] = {NULL};
    tst_node *root = NULL, *res = NULL;
    */
    int rtn = 0, idx = 0;
    double t1, t2;

    FILE *fpin = fopen(IN_FILE, "r");
    FILE *fpout_ins_bloom = fopen(OUT_INS_TIME_BLOOM, "w");
    FILE *fpout_ins_wobloom = fopen(OUT_INS_TIME_WOBLOOM, "w");

    if (!fpin) {
        fprintf(stderr, "error: file open failed '%s'.\n", argv[1]);
        fclose(fpout_ins_bloom);
        fclose(fpout_ins_wobloom);
        return 1;
    }

    if (!fpout_ins_bloom) {
        fprintf(stderr, "error: file write failed '%s'.\n", OUT_INS_TIME_BLOOM);
        fclose(fpin);
        fclose(fpout_ins_wobloom);
        return 1;
    }

    if (!fpout_ins_wobloom) {
        fprintf(stderr, "error: file write failed '%s'.\n",
                OUT_INS_TIME_WOBLOOM);
        fclose(fpin);
        fclose(fpout_ins_bloom);
        return 1;
    }

    /* insert time of none-bloom tst */
    for (int i = 0; i < DotNum; i++) {
        tst_node *root_test = NULL;
        t1 = tvgetf();
        init_tst_wbloom(fpin, root_test, i);
        t2 = tvgetf();
        tst_free_all(root_test);
        fprintf(fpout_ins_wobloom, "%d %lf\n", i + 1, (t2 - t1) * 1000000);
    }

    /* insert time of bloom tst */
    for (int i = 0; i < DotNum; i++) {
        tst_node *root_test = NULL;
        t1 = tvgetf();
        bloom_t blm_test = bloom_create(TableSize);
        init_tst_bloom(fpin, blm_test, root_test, i);
        t2 = tvgetf();
        tst_free_all(root_test);
        bloom_free(blm_test);
        fprintf(fpout_ins_bloom, "%d %lf\n", i + 1, (t2 - t1) * 1000000);
    }

    fclose(fpout_ins_bloom);
    fclose(fpout_ins_wobloom);


    /* regularly create a useful bloom and tst*/
    bloom_t bloom = bloom_create(TableSize);
    tst_node *root = NULL;

    /* memory pool */
    char *pool = (char *) malloc(poolsize * sizeof(char));
    char *Top = pool;
    while ((rtn = fscanf(fpin, "%s", Top)) != EOF) {
        char *p = Top;

        /* insert refence to each string */
        if (!tst_ins_del(&root, &p, INS, REF)) {
            /* failed to insert*/
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            fclose(fpin);
            return 1;
        } else {
            /* update bloom filter */
            bloom_add(bloom, Top);
        }
        idx++;
        Top += (strlen(Top) + 1);
    }

    /* search time with/without bloom filter */

    FILE *fpout_search_time_bloom = fopen(OUT_SEARCH_TIME_BLOOM, "w");
    FILE *fpout_search_time_wobloom = fopen(OUT_SEARCH_TIME_WOBLOOM, "w");

    if (!fpout_search_time_bloom) {
        fprintf(stderr, "error: file open failed '%s'.\n",
                OUT_SEARCH_TIME_BLOOM);
        fclose(fpout_search_time_wobloom);
        fclose(fpin);
        return 1;
    }

    if (!fpout_search_time_wobloom) {
        fprintf(stderr, "error: file open failed '%s'.\n",
                OUT_SEARCH_TIME_WOBLOOM);
        fclose(fpout_search_time_bloom);
        fclose(fpin);
        return 1;
    }

    int search_sample_cnt = 0;
    double search_time_total_bloom = 0.0;
    double search_time_total_wobloom = 0.0;

    rewind(fpin);
    while ((rtn = fscanf(fpin, "%s", Top)) != EOF) {
        char *p = Top;
        t1 = tvgetf();
        bloom_test(bloom, p);
        t2 = tvgetf();
        search_time_total_bloom += (t2 - t1) * 1000000;

        t1 = tvgetf();
        tst_search(root, p);
        t2 = tvgetf();
        search_time_total_wobloom += (t2 - t1) * 1000000;

        search_sample_cnt++;
        if (search_sample_cnt % 100 == 0) {
            fprintf(fpout_search_time_bloom, "%d %lf\n",
                    search_sample_cnt / 100, search_time_total_bloom / 100);
            fprintf(fpout_search_time_wobloom, "%d %lf\n",
                    search_sample_cnt / 100, search_time_total_wobloom / 100);
            search_time_total_bloom = 0.0;
            search_time_total_wobloom = 0.0;
        }
    }

    fclose(fpout_search_time_bloom);
    fclose(fpout_search_time_wobloom);

    bloom_free(bloom);
    tst_free(root);

    fclose(fpin);
}

int init_tst_bloom(FILE *fpin, bloom_t bloom, tst_node *root, size_t size)
{
    char *pool = (char *) malloc(poolsize * sizeof(char));
    char *Top = pool;
    int rtn = 0;
    int size_count = 0;
    /* stop when reaching EOF or size limit */
    while (((rtn = fscanf(fpin, "%s", Top)) != EOF) && (size_count < size)) {
        char *p = Top;
        if (!tst_ins_del(&root, &p, INS, REF)) {
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            return 1;
        } else {
            bloom_add(bloom, Top);
            size_count++;
        }
        Top += (strlen(Top) + 1);
    }
    return 0;
}

int init_tst_wbloom(FILE *fpin, tst_node *root, size_t size)
{
    char word[WRDMAX] = "";
    /*
    char *sgl[LMAX] = {NULL};
    */
    int rtn = 0;
    int size_count = 0;
    while (((rtn = fscanf(fpin, "%s", word)) != EOF) && size_count < size) {
        char *p = word;
        if (!tst_ins_del(&root, &p, INS, CPY)) {
            fprintf(stderr, "error: memory exhausted, tst_insert.\n");
            return 1;
        }
        size_count++;
    }
    return 0;
}