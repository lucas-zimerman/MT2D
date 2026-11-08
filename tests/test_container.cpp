// Tests for MT2D/Container/MT2D_Container.cpp
//
// The container keeps one global state, every test starts from a clean one.
#include <mt2d_test.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/File/MT2D_File.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void reset() {
    MT2D_Container_Clear();
    MT2D_Container_Init();
    MT2D_Container_Password_Init();
}

static void write_file(const char *name, const char *text) {
    FILE *f = fopen(name, "wb");
    fwrite(text, 1, strlen(text), f);
    fclose(f);
}

static char *dup(const char *s) {
    char *c = (char *)malloc(strlen(s) + 1);
    strcpy(c, s);
    return c;
}

// Loads `text` (with its '\0') into the container as a memory file.
static void load_memory(const char *name, const char *text) {
    MT2D_Container_Load_File_From_Memory(dup(name), (BYTE *)dup(text), (int)strlen(text) + 1);
}

TEST(starts_empty) {
    reset();
    CHECK_EQ(MT2D_Container_Count_Files(), 0);
    CHECK_EQ(MT2D_Container_Get_FileId((char *)"nothing"), -1);
}

TEST(load_disk_file) {
    reset();
    write_file("helloworld.txt", "Hi World");
    CHECK(MT2D_Container_Load_File((char *)"helloworld.txt"));
    CHECK_EQ(MT2D_Container_Count_Files(), 1);

    int id = MT2D_Container_Get_FileId((char *)"helloworld.txt");
    CHECK(id >= 0);
    CHECK_EQ(MT2D_Container_Get_FileLength(id), 8);
    CHECK_EQ(MT2D_GetFile_StoreType(id), MT2D_CONT_DATATYPE_FILE);

    char *name = MT2D_Container_Get_FileName_By_ID(id);
    CHECK_STR_EQ(name, "helloworld.txt");
    free(name);

    BYTE *data = MT2D_Container_Get_Data(id, false);
    CHECK(data != 0);
    CHECK(memcmp(data, "Hi World", 8) == 0);
    free(data);
    remove("helloworld.txt");
}

TEST(load_memory_file) {
    reset();
    load_memory("data", "data data and more data!");
    CHECK_EQ(MT2D_Container_Count_Files(), 1);
    int id = MT2D_Container_Get_FileId((char *)"data");
    CHECK(id >= 0);
    CHECK_EQ(MT2D_Container_Get_FileLength(id), 25);
    BYTE *data = MT2D_Container_Get_Data(id, false);
    CHECK_STR_EQ(data, "data data and more data!");
    free(data);
}

TEST(memory_data_is_copied_on_get) {
    reset();
    load_memory("data", "abc");
    int id = MT2D_Container_Get_FileId((char *)"data");
    BYTE *a = MT2D_Container_Get_Data(id, false);
    BYTE *b = MT2D_Container_Get_Data(id, false);
    CHECK(a != b);
    a[0] = 'z';
    CHECK_EQ(b[0], 'a');
    free(a);
    free(b);
}

TEST(multiple_files_have_different_ids) {
    reset();
    load_memory("data", "first");
    load_memory("data2", "second!");
    load_memory("third", "third");
    CHECK_EQ(MT2D_Container_Count_Files(), 3);
    int a = MT2D_Container_Get_FileId((char *)"data");
    int b = MT2D_Container_Get_FileId((char *)"data2");
    int c = MT2D_Container_Get_FileId((char *)"third");
    CHECK(a >= 0 && b >= 0 && c >= 0);
    CHECK(a != b && b != c && a != c);
    CHECK_EQ(MT2D_Container_Get_FileLength(a), 6);
    CHECK_EQ(MT2D_Container_Get_FileLength(b), 8);
    BYTE *d = MT2D_Container_Get_Data(b, false);
    CHECK_STR_EQ(d, "second!");
    free(d);
}

TEST(lookup_is_exact_match) {
    reset();
    load_memory("data2", "x");
    CHECK_EQ(MT2D_Container_Get_FileId((char *)"data"), -1);   // prefix of a stored name
    CHECK_EQ(MT2D_Container_Get_FileId((char *)"data22"), -1); // stored name is a prefix
    CHECK(MT2D_Container_Get_FileId((char *)"data2") >= 0);
}

TEST(file_name_of_unknown_id_is_null) {
    reset();
    CHECK(MT2D_Container_Get_FileName_By_ID(0x35) == 0);
}

TEST(clear_removes_everything) {
    reset();
    load_memory("data", "x");
    load_memory("data2", "y");
    MT2D_Container_Clear();
    CHECK_EQ(MT2D_Container_Count_Files(), 0);
    MT2D_Container_Init();
    CHECK_EQ(MT2D_Container_Get_FileId((char *)"data"), -1);
}

TEST(load_missing_container_fails) {
    reset();
    CHECK(!MT2D_Container_Load((char *)"missing.mt2"));
    CHECK_EQ(MT2D_Container_Count_Files(), 0);
}

TEST(load_empty_container) {
    reset();
    FILE *f = fopen("empty.mt2", "wb");
    fclose(f);
    CHECK(MT2D_Container_Load(dup("empty.mt2")));
    CHECK_EQ(MT2D_Container_Count_Files(), 0);
    remove("empty.mt2");
}

static long file_size(const char *name) {
    FILE *f = fopen(name, "rb");
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long n = ftell(f);
    fclose(f);
    return n;
}

TEST(export_into_container_appends_to_the_file) {
    reset();
    FILE *f = fopen("pack.mt2", "wb");
    fclose(f);
    load_memory("data", "data data and more data!");
    CHECK(MT2D_Container_Load(dup("pack.mt2")));
    int id = MT2D_Container_Get_FileId((char *)"data");

    CHECK(MT2D_Container_Export_into_Container(id, (char *)"pack.mt2", false));
    long first = file_size("pack.mt2");
    CHECK(first > 25);
    CHECK(MT2D_Container_Export_into_Container(id, (char *)"pack.mt2", false));
    CHECK(file_size("pack.mt2") > first);
    // the exported file is also registered in the loaded container
    CHECK_EQ(MT2D_Container_Count_Files(), 3);

    reset();
    CHECK(MT2D_Container_Load(dup("pack.mt2")));
    CHECK_EQ(MT2D_Container_Count_Files(), 2);
    reset();
    remove("pack.mt2");
}

// BUG: write_Name() shifts every byte of the name by 7 instead of rotating
// the bit, so the stored name is destroyed and the file cannot be found again
// after the container is loaded from disk.
TEST_XFAIL(exported_files_can_be_found_after_reloading) {
    reset();
    FILE *f = fopen("pack.mt2", "wb");
    fclose(f);
    load_memory("data", "data data and more data!");
    MT2D_Container_Load(dup("pack.mt2"));
    MT2D_Container_Export_into_Container(MT2D_Container_Get_FileId((char *)"data"), (char *)"pack.mt2", false);

    reset();
    MT2D_Container_Load(dup("pack.mt2"));
    int id = MT2D_Container_Get_FileId((char *)"data");
    CHECK(id >= 0);
    if (id >= 0) {
        BYTE *d = MT2D_Container_Get_Data(id, false);
        CHECK_STR_EQ(d, "data data and more data!");
        free(d);
    }
    reset();
    remove("pack.mt2");
}

TEST_XFAIL(encrypted_files_can_be_decrypted_after_reloading) {
    reset();
    FILE *f = fopen("secret.mt2", "wb");
    fclose(f);
    for (int i = 0; i < 1000; i += 3) {
        MT2D_Container_Password_AddKey((unsigned char)i);
    }
    load_memory("data2", "data data and more data!");
    MT2D_Container_Load(dup("secret.mt2"));
    MT2D_Container_Export_into_Container(MT2D_Container_Get_FileId((char *)"data2"), (char *)"secret.mt2", true);

    reset();
    for (int i = 0; i < 1000; i += 3) {
        MT2D_Container_Password_AddKey((unsigned char)i);
    }
    MT2D_Container_Load(dup("secret.mt2"));
    int id = MT2D_Container_Get_FileId((char *)"data2");
    CHECK(id >= 0);
    if (id >= 0) {
        BYTE *plain = MT2D_Container_Get_Data(id, true);
        CHECK_STR_EQ(plain, "data data and more data!");
        free(plain);
    }
    reset();
    remove("secret.mt2");
}

// BUG: memory files have fileDataOffset == 0, which is checked first, so they
// are reported as MT2D_CONT_DATATYPE_FILE.
TEST_XFAIL(memory_file_reports_memory_store_type) {
    reset();
    load_memory("data", "x");
    CHECK_EQ(MT2D_GetFile_StoreType(MT2D_Container_Get_FileId((char *)"data")), MT2D_CONT_DATATYPE_MEM);
}

TEST(export_into_unknown_container_fails) {
    reset();
    load_memory("data", "x");
    int id = MT2D_Container_Get_FileId((char *)"data");
    CHECK(!MT2D_Container_Export_into_Container(id, (char *)"unknown.mt2", false));
}

TEST(export_as_file_writes_the_data) {
    reset();
    load_memory("data", "exported text");
    int id = MT2D_Container_Get_FileId((char *)"data");
    CHECK(MT2D_Container_Export_as_File(id, (char *)"exported.txt", (char *)".", false));

    FILE *f = fopen("exported.txt", "rb");
    CHECK(f != 0);
    if (f) {
        char buf[64] = {0};
        size_t n = fread(buf, 1, sizeof(buf) - 1, f);
        fclose(f);
        CHECK(n >= 13);
        CHECK(strncmp(buf, "exported text", 13) == 0);
    }
    remove("exported.txt");
}

TEST(export_as_file_fails_on_invalid_path) {
    reset();
    load_memory("data", "x");
    int id = MT2D_Container_Get_FileId((char *)"data");
    CHECK(!MT2D_Container_Export_as_File(id, (char *)"x.txt", (char *)"/definitely/not/a/dir", false));
}

TEST(read_pieces_of_memory_file) {
    reset();
    MT2D_Container_Load_File_From_Memory(dup("song"), (BYTE *)dup("0123456789"), 10);
    int id = MT2D_Container_Get_FileId((char *)"song");
    BYTE buf[16];
    CHECK_EQ(MT2D_Container_Read(id, 0, buf, 4), 4);
    CHECK(memcmp(buf, "0123", 4) == 0);
    CHECK_EQ(MT2D_Container_Read(id, 8, buf, 16), 2);  // clamped to the end
    CHECK(memcmp(buf, "89", 2) == 0);
    CHECK_EQ(MT2D_Container_Read(id, 10, buf, 4), 0);  // at the end
}

TEST(read_pieces_of_disk_file) {
    reset();
    write_file("readme.bin", "abcdefghij");
    CHECK(MT2D_Container_Load_File((char *)"readme.bin"));
    int id = MT2D_Container_Get_FileId((char *)"readme.bin");
    BYTE buf[16];
    CHECK_EQ(MT2D_Container_Read(id, 3, buf, 4), 4);
    CHECK(memcmp(buf, "defg", 4) == 0);
    CHECK_EQ(MT2D_Container_Read(id, 8, buf, 16), 2);
    CHECK(memcmp(buf, "ij", 2) == 0);
    remove("readme.bin");
}

TEST(read_rejects_bad_arguments) {
    reset();
    BYTE buf[4];
    CHECK_EQ(MT2D_Container_Read(-1, 0, buf, 4), -1);
    load_memory("x", "abc");
    int id = MT2D_Container_Get_FileId((char *)"x");
    CHECK_EQ(MT2D_Container_Read(id, 0, 0, 4), -1);
    CHECK_EQ(MT2D_Container_Read(id, -1, buf, 4), -1);
    CHECK_EQ(MT2D_Container_Read(0x7f0 | 5, 0, buf, 4), -1);  // unknown id
}

MT2D_TEST_MAIN()
