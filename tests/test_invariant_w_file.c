#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

/* WAD header structure */
typedef struct {
    char id[4];
    int32_t numlumps;
    int32_t infotableofs;
} wadheader_t;

/* WAD directory entry */
typedef struct {
    int32_t filepos;
    int32_t size;
    char name[8];
} filelump_t;

/* Forward declare the WAD open function */
extern void *W_OpenFile(const char *filename);

static void write_wad(const char *path, int32_t numlumps, int32_t infotableofs,
                      filelump_t *lumps, size_t filesize) {
    FILE *f = fopen(path, "wb");
    wadheader_t hdr = { .id = {'I','W','A','D'}, .numlumps = numlumps,
                        .infotableofs = infotableofs };
    fwrite(&hdr, sizeof(hdr), 1, f);
    if (lumps && numlumps > 0) {
        fwrite(lumps, sizeof(filelump_t), numlumps, f);
    }
    /* Pad to filesize */
    long cur = ftell(f);
    if ((size_t)cur < filesize) {
        char zero = 0;
        fseek(f, filesize - 1, SEEK_SET);
        fwrite(&zero, 1, 1, f);
    }
    fclose(f);
}

START_TEST(test_wad_malicious_lump_bounds)
{
    /* Invariant: Opening a WAD with lump offsets/sizes exceeding file size
       must not cause out-of-bounds access — it should either reject the file
       or clamp/validate entries. We test that the engine does not crash. */
    const char *tmpfile = "/tmp/test_security_wad.wad";

    /* Case 1: lump offset far beyond file size */
    filelump_t bad_lump1 = { .filepos = 0x7FFFFFFF, .size = 1024, .name = "BADLUMP" };
    write_wad(tmpfile, 1, sizeof(wadheader_t), &bad_lump1, 64);
    void *wad1 = W_OpenFile(tmpfile);
    /* If it returns non-NULL, the loader accepted it — later lump reads must not crash */
    /* We just verify no crash/SEGV occurred during open */
    (void)wad1;

    /* Case 2: numlumps causes integer overflow in table size calculation */
    write_wad(tmpfile, 0x7FFFFFFF, sizeof(wadheader_t), NULL, 32);
    void *wad2 = W_OpenFile(tmpfile);
    ck_assert_msg(wad2 == NULL, "WAD with absurd numlumps should be rejected");

    /* Case 3: infotableofs points beyond file */
    filelump_t ok_lump = { .filepos = sizeof(wadheader_t), .size = 4, .name = "VALID" };
    write_wad(tmpfile, 1, 0x7FFFFFFF, &ok_lump, 64);
    void *wad3 = W_OpenFile(tmpfile);
    ck_assert_msg(wad3 == NULL, "WAD with infotableofs beyond EOF should be rejected");

    /* Case 4: valid small WAD — should succeed */
    filelump_t valid_lump = { .filepos = (int32_t)(sizeof(wadheader_t) + sizeof(filelump_t)),
                              .size = 4, .name = "TEST" };
    size_t valid_size = sizeof(wadheader_t) + sizeof(filelump_t) + 4;
    write_wad(tmpfile, 1, sizeof(wadheader_t), &valid_lump, valid_size);
    void *wad4 = W_OpenFile(tmpfile);
    ck_assert_msg(wad4 != NULL, "Valid WAD should open successfully");

    remove(tmpfile);
}
END_TEST

Suite *security_suite(void)
{
    Suite *s = suite_create("Security");
    TCase *tc_core = tcase_create("Core");
    tcase_add_test(tc_core, test_wad_malicious_lump_bounds);
    suite_add_tcase(s, tc_core);
    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = security_suite();