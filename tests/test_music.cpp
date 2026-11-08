// Tests for MT2D/Audio/MT2D_Music.c (IMF decoder, whole and stream modes)
#include <mt2d_test.h>
#include <MT2D/Audio/MT2D_Music.h>
#include <MT2D/Container/MT2D_Container.h>
#include <MT2D/File/MT2D_File.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

struct Rec {
    std::vector<int> ticks; // tick number of every write
    std::vector<int> regs;
    std::vector<int> vals;
    int tick;
    Rec() : tick(0) {}
};

static void on_write(unsigned char reg, unsigned char val, void *ctx) {
    Rec *r = (Rec *)ctx;
    r->ticks.push_back(r->tick);
    r->regs.push_back(reg);
    r->vals.push_back(val);
}

// Appends one event (reg, value, delay).
static void ev(std::vector<unsigned char> &s, int reg, int val, int delay) {
    s.push_back((unsigned char)reg);
    s.push_back((unsigned char)val);
    s.push_back((unsigned char)(delay & 255));
    s.push_back((unsigned char)(delay >> 8));
}

// Type 1 song: 16 bit length + events.
static std::vector<unsigned char> type1(const std::vector<unsigned char> &events) {
    std::vector<unsigned char> s;
    s.push_back((unsigned char)(events.size() & 255));
    s.push_back((unsigned char)(events.size() >> 8));
    s.insert(s.end(), events.begin(), events.end());
    return s;
}

static std::vector<unsigned char> small_events() {
    std::vector<unsigned char> e;
    ev(e, 0x20, 1, 2);
    ev(e, 0xB0, 5, 0);
    ev(e, 0xA0, 7, 1);
    return e;
}

// Runs the player until it ends (or max ticks), calling Update before each tick like the main loop does.
static void run(MT2D_Music *m, Rec *r, int max_ticks) {
    for (r->tick = 0; r->tick < max_ticks; r->tick++) {
        MT2D_Music_Update(m);
        if (!MT2D_Music_Tick(m)) break;
    }
}

TEST(whole_mode_plays_events_with_delays) {
    std::vector<unsigned char> song = type1(small_events());
    unsigned char buf[64];
    Rec r;
    MT2D_Music m;
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, 0, 0, buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_ERR_EMPTY);

    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_OK);
    CHECK(!m.streaming);
    MT2D_Music_Play(&m);
    run(&m, &r, 100);
    CHECK_EQ(r.regs.size(), 3);
    CHECK_EQ(r.regs[0], 0x20); CHECK_EQ(r.ticks[0], 0);
    CHECK_EQ(r.regs[1], 0xB0); CHECK_EQ(r.ticks[1], 2); // 2 ticks after the first
    CHECK_EQ(r.regs[2], 0xA0); CHECK_EQ(r.ticks[2], 2); // zero delay: same tick
    CHECK_EQ(r.vals[1], 5);
    CHECK(!m.playing);
}

TEST(type0_song_has_no_header) {
    std::vector<unsigned char> song = small_events();
    song[0] = 0; song[1] = 0; // type 0 songs start with a zero word (reg 0, value 0)
    unsigned char buf[64];
    Rec r;
    MT2D_Music m;
    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_OK);
    MT2D_Music_Play(&m);
    run(&m, &r, 100);
    CHECK_EQ(r.regs.size(), 3);
    CHECK_EQ(r.regs[0], 0);
}

TEST(trailing_tag_after_the_data_is_ignored) {
    std::vector<unsigned char> song = type1(small_events());
    const char *tag = "TITLE";
    song.insert(song.end(), tag, tag + 5);
    unsigned char buf[64];
    Rec r;
    MT2D_Music m;
    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_OK);
    MT2D_Music_Play(&m);
    run(&m, &r, 100);
    CHECK_EQ(r.regs.size(), 3);
}

TEST(stream_mode_matches_whole_mode) {
    // 40 events, delays 0..3, a 16 byte ring forces many refills
    std::vector<unsigned char> e;
    for (int i = 0; i < 40; i++) ev(e, i + 1, 255 - i, i % 4);
    std::vector<unsigned char> song = type1(e);
    MT2D_MusicMem mem = { song.data(), (long)song.size() };

    unsigned char big[512], small[16];
    Rec whole, stream;
    MT2D_Music a, b;
    CHECK_EQ(MT2D_Music_Open(&a, MT2D_Music_ReadMemory, &mem, (long)song.size(), big, sizeof(big), on_write, &whole, 0), MT2D_MUSIC_OK);
    CHECK_EQ(MT2D_Music_Open(&b, MT2D_Music_ReadMemory, &mem, (long)song.size(), small, sizeof(small), on_write, &stream, 0), MT2D_MUSIC_OK);
    CHECK(!a.streaming);
    CHECK(b.streaming);
    MT2D_Music_Play(&a); run(&a, &whole, 1000);
    MT2D_Music_Play(&b); run(&b, &stream, 1000);
    CHECK_EQ(whole.regs.size(), 40);
    CHECK(whole.regs == stream.regs);
    CHECK(whole.vals == stream.vals);
    CHECK(whole.ticks == stream.ticks);
}

TEST(large_song_streams_exactly_like_whole_mode) {
    // 15000 events (60 KB, like the playmusic medley) with every kind of delay, 1 KB ring vs everything in memory
    std::vector<unsigned char> e;
    srand(7);
    for (int i = 0; i < 15000; i++) ev(e, rand() & 255, rand() & 255, (rand() % 5 == 0) ? rand() % 300 : rand() % 3);
    std::vector<unsigned char> song = type1(e);
    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    std::vector<unsigned char> big(70000);
    unsigned char ring[1024];
    Rec whole, stream;
    MT2D_Music a, b;
    CHECK_EQ(MT2D_Music_Open(&a, MT2D_Music_ReadMemory, &mem, (long)song.size(), big.data(), big.size(), on_write, &whole, 0), MT2D_MUSIC_OK);
    CHECK_EQ(MT2D_Music_Open(&b, MT2D_Music_ReadMemory, &mem, (long)song.size(), ring, sizeof(ring), on_write, &stream, 0), MT2D_MUSIC_OK);
    CHECK(!a.streaming);
    CHECK(b.streaming);
    MT2D_Music_Play(&a); run(&a, &whole, 5000000);
    MT2D_Music_Play(&b); run(&b, &stream, 5000000);
    CHECK_EQ(whole.regs.size(), 15000);
    CHECK(whole.regs == stream.regs);
    CHECK(whole.vals == stream.vals);
    CHECK(whole.ticks == stream.ticks);
}

TEST(forced_stream_and_preload_flags) {
    std::vector<unsigned char> song = type1(small_events());
    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    unsigned char buf[64], tiny[8];
    Rec r;
    MT2D_Music m;
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), buf, sizeof(buf), on_write, &r, MT2D_MUSIC_STREAM), MT2D_MUSIC_OK);
    CHECK(m.streaming);
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), tiny, sizeof(tiny), on_write, &r, MT2D_MUSIC_PRELOAD), MT2D_MUSIC_ERR_FIT);
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), tiny, 4, on_write, &r, 0), MT2D_MUSIC_ERR_ARGS);
}

TEST(loop_whole_and_stream) {
    std::vector<unsigned char> e;
    ev(e, 1, 1, 1);
    ev(e, 2, 2, 1);
    std::vector<unsigned char> song = type1(e);
    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    unsigned char big[64], small[8];
    for (int stream = 0; stream < 2; stream++) {
        Rec r;
        MT2D_Music m;
        CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(),
                                 stream ? small : big, stream ? sizeof(small) : sizeof(big), on_write, &r, MT2D_MUSIC_LOOP | (stream ? MT2D_MUSIC_STREAM : 0)), MT2D_MUSIC_OK);
        CHECK_EQ((int)m.streaming, stream);
        MT2D_Music_Play(&m);
        run(&m, &r, 40);
        CHECK(m.playing);               // never ends
        CHECK(r.regs.size() >= 12);     // many repetitions
        for (size_t i = 0; i < r.regs.size(); i++) {
            CHECK_EQ(r.regs[i], (int)(i % 2) + 1);
        }
    }
}

TEST(memory_open_plays_in_place_without_copy) {
    std::vector<unsigned char> song = type1(small_events());
    Rec r;
    MT2D_Music m;
    CHECK_EQ(MT2D_Music_OpenMemory(&m, song.data(), (long)song.size(), on_write, &r, 0), MT2D_MUSIC_OK);
    CHECK(m.buf == song.data() + 2);
    MT2D_Music_Play(&m);
    run(&m, &r, 100);
    CHECK_EQ(r.regs.size(), 3);
}

TEST(stop_and_restart) {
    std::vector<unsigned char> song = type1(small_events());
    Rec r;
    MT2D_Music m;
    MT2D_Music_OpenMemory(&m, song.data(), (long)song.size(), on_write, &r, 0);
    MT2D_Music_Play(&m);
    r.tick = 0; MT2D_Music_Tick(&m);
    MT2D_Music_Stop(&m);
    CHECK_EQ(MT2D_Music_Tick(&m), 0);
    CHECK_EQ(r.regs.size(), 1);
    MT2D_Music_Play(&m);
    run(&m, &r, 100);
    CHECK_EQ(r.regs.size(), 4); // 1 + the whole song again
}

TEST(garbage_is_harmless) {
    srand(1234);
    for (int round = 0; round < 50; round++) {
        std::vector<unsigned char> song(4 + rand() % 400);
        for (size_t i = 0; i < song.size(); i++) song[i] = (unsigned char)rand();
        if (round % 5 == 0) memset(&song[2], 0, song.size() - 2); // zero delay storm
        MT2D_MusicMem mem = { song.data(), (long)song.size() };
        unsigned char buf[24];
        Rec r;
        MT2D_Music m;
        int err = MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), buf, sizeof(buf), on_write,
                                  &r, MT2D_MUSIC_LOOP | ((round & 1) ? MT2D_MUSIC_STREAM : 0));
        if (err != MT2D_MUSIC_OK) continue;
        MT2D_Music_Play(&m);
        for (int t = 0; t < 2000; t++) {
            MT2D_Music_Update(&m);
            MT2D_Music_Tick(&m);
        }
        // at most MAX_EVENTS writes per tick
        CHECK(r.regs.size() <= (size_t)2000 * MT2D_MUSIC_MAX_EVENTS);
    }
}

TEST(short_read_plays_what_exists) {
    // the header claims more data than the file has
    std::vector<unsigned char> e = small_events();
    std::vector<unsigned char> song = type1(e);
    song[0] = 200; // length lies
    MT2D_MusicMem mem = { song.data(), (long)song.size() };
    unsigned char buf[8];
    Rec r;
    MT2D_Music m;
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, &mem, (long)song.size(), buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_OK);
    MT2D_Music_Play(&m);
    run(&m, &r, 100);
    CHECK_EQ(r.regs.size(), 3);
}

TEST(stream_from_container_file) {
    MT2D_Container_Clear();
    MT2D_Container_Init();
    std::vector<unsigned char> e;
    for (int i = 0; i < 20; i++) ev(e, i + 1, i, 1);
    std::vector<unsigned char> song = type1(e);
    unsigned char *copy = (unsigned char *)malloc(song.size());
    memcpy(copy, song.data(), song.size());
    char *name = (char *)malloc(10);
    strcpy(name, "SONG.IMF");
    MT2D_Container_Load_File_From_Memory(name, copy, (int)song.size());
    int id = MT2D_Container_Get_FileId((char *)"SONG.IMF");
    CHECK(id >= 0);

    unsigned char buf[16];
    Rec r;
    MT2D_Music m;
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadContainer, &id, MT2D_Container_Get_FileLength(id), buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_OK);
    CHECK(m.streaming);
    MT2D_Music_Play(&m);
    run(&m, &r, 200);
    CHECK_EQ(r.regs.size(), 20);
    CHECK_EQ(r.regs[19], 20);
}

TEST(stream_from_disk_file) {
    std::vector<unsigned char> e;
    for (int i = 0; i < 20; i++) ev(e, i + 1, i, 1);
    std::vector<unsigned char> song = type1(e);
    FILE *f = fopen("song.imf", "wb");
    fwrite(song.data(), 1, song.size(), f);
    fclose(f);

    MT2D_FILE *file = MT2D_FILE_OPEN("song.imf", "rb");
    CHECK(file != 0);
    unsigned char buf[16];
    Rec r;
    MT2D_Music m;
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadFile, file, (long)song.size(), buf, sizeof(buf), on_write, &r, 0), MT2D_MUSIC_OK);
    CHECK(m.streaming);
    MT2D_Music_Play(&m);
    run(&m, &r, 200);
    CHECK_EQ(r.regs.size(), 20);
    MT2D_FILE_CLOSE(file);
    remove("song.imf");
}

TEST(open_rejects_bad_arguments) {
    unsigned char buf[16];
    Rec r;
    MT2D_Music m;
    unsigned char d[8] = {0};
    CHECK_EQ(MT2D_Music_OpenMemory(&m, 0, 8, on_write, &r, 0), MT2D_MUSIC_ERR_ARGS);
    CHECK_EQ(MT2D_Music_OpenMemory(&m, d, 8, 0, &r, 0), MT2D_MUSIC_ERR_ARGS);
    CHECK_EQ(MT2D_Music_OpenMemory(&m, d, 2, on_write, &r, 0), MT2D_MUSIC_ERR_EMPTY);
    CHECK_EQ(MT2D_Music_Open(&m, MT2D_Music_ReadMemory, 0, 8, 0, 16, on_write, &r, 0), MT2D_MUSIC_ERR_ARGS);
    (void)buf;
}

MT2D_TEST_MAIN()
