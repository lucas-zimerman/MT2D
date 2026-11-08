// Tests for MT2D/Audio/MT2D_Mixer.c (software mixer: 8 bit + 16 bit voices into one output)
#include <mt2d_test.h>
#include <MT2D/Audio/MT2D_Mixer.h>
#include <vector>

static std::vector<short> render16(MT2D_Mixer *m, int n) {
    std::vector<short> out(n > 0 ? n : 1, 12345);
    MT2D_Mixer_Render16(m, &out[0], n);
    return out;
}

static std::vector<unsigned char> render8(MT2D_Mixer *m, int n) {
    std::vector<unsigned char> out(n > 0 ? n : 1, 77);
    MT2D_Mixer_Render8(m, &out[0], n);
    return out;
}

TEST(idle_mixer_renders_silence) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    std::vector<short> a = render16(&m, 10);
    std::vector<unsigned char> b = render8(&m, 10);
    for (int i = 0; i < 10; i++) {
        CHECK_EQ(a[i], 0);
        CHECK_EQ(b[i], 128);
    }
    CHECK_EQ(MT2D_Mixer_Active(&m), 0);
}

TEST(u8_voice_at_unity_is_the_same_sample) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const unsigned char s[4] = {128, 200, 50, 255};
    CHECK(MT2D_Mixer_Play(&m, s, 4, MT2D_MIXER_U8, 0, MT2D_MIXER_VOL_MAX) >= 0);
    std::vector<unsigned char> o = render8(&m, 6);
    CHECK_EQ(o[0], 128);
    CHECK_EQ(o[1], 200);
    CHECK_EQ(o[2], 50);
    CHECK_EQ(o[3], 255);
    CHECK_EQ(o[4], 128); // ended, silence
    CHECK_EQ(o[5], 128);
}

TEST(u8_voice_in_16_bit_output_is_scaled_up) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const unsigned char s[3] = {128, 129, 0};
    MT2D_Mixer_Play(&m, s, 3, MT2D_MIXER_U8, 0, MT2D_MIXER_VOL_MAX);
    std::vector<short> o = render16(&m, 3);
    CHECK_EQ(o[0], 0);
    CHECK_EQ(o[1], 256);
    CHECK_EQ(o[2], -32768);
}

TEST(s16_voice_at_unity_is_the_same_sample) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[4] = {0, 1000, -1000, 32767};
    MT2D_Mixer_Play(&m, s, 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    std::vector<short> o = render16(&m, 4);
    CHECK_EQ(o[0], 0);
    CHECK_EQ(o[1], 1000);
    CHECK_EQ(o[2], -1000);
    CHECK_EQ(o[3], 32767);
}

TEST(s16_voice_in_8_bit_output_is_scaled_down) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[3] = {0, 0x4000, -0x4000};
    MT2D_Mixer_Play(&m, s, 3, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    std::vector<unsigned char> o = render8(&m, 3);
    CHECK_EQ(o[0], 128);
    CHECK_EQ(o[1], 128 + 64);
    CHECK_EQ(o[2], 128 - 64);
}

TEST(u8_and_s16_voices_play_together) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const unsigned char a[2] = {128 + 10, 128 - 10}; // +2560, -2560 as 16 bit
    const short b[2] = {1000, 1000};
    MT2D_Mixer_Play(&m, a, 2, MT2D_MIXER_U8, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, b, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    CHECK_EQ(MT2D_Mixer_Active(&m), 2);
    std::vector<short> o = render16(&m, 2);
    CHECK_EQ(o[0], 3560);
    CHECK_EQ(o[1], -1560);
}

TEST(volume_scales_each_voice) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[1] = {10000};
    const short t[1] = {10000}; // another sound: the same one would restart the voice
    MT2D_Mixer_Play(&m, s, 1, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX / 2);
    MT2D_Mixer_Play(&m, t, 1, MT2D_MIXER_S16, 0, 0);
    std::vector<short> o = render16(&m, 1);
    CHECK_EQ(o[0], 5000);
}

TEST(volume_is_clamped) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[1] = {1000};
    const short t[1] = {1000};
    MT2D_Mixer_Play(&m, s, 1, MT2D_MIXER_S16, 0, 100000);
    MT2D_Mixer_Play(&m, t, 1, MT2D_MIXER_S16, 0, -5);
    std::vector<short> o = render16(&m, 1);
    CHECK_EQ(o[0], 1000);
}

TEST(master_volume_scales_the_sum) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[1] = {10000};
    const short t[1] = {10000};
    MT2D_Mixer_Play(&m, s, 1, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, t, 1, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    m.master = MT2D_MIXER_VOL_MAX / 2;
    std::vector<short> o = render16(&m, 1);
    CHECK_EQ(o[0], 10000);
}

TEST(sum_is_clamped_not_wrapped) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short hi[2] = {30000, -30000};
    const short hi2[2] = {30000, -30000};
    MT2D_Mixer_Play(&m, hi, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, hi2, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    std::vector<short> o = render16(&m, 2);
    CHECK_EQ(o[0], 32767);
    CHECK_EQ(o[1], -32768);
    MT2D_Mixer_Play(&m, hi, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, hi2, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    std::vector<unsigned char> b = render8(&m, 2);
    CHECK_EQ(b[0], 255);
    CHECK_EQ(b[1], 0);
}

TEST(loop_restarts_and_stop_ends_it) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[3] = {1, 2, 3};
    int h = MT2D_Mixer_Play(&m, s, 3, MT2D_MIXER_S16, MT2D_MIXER_LOOP, MT2D_MIXER_VOL_MAX);
    std::vector<short> o = render16(&m, 7);
    short want[7] = {1, 2, 3, 1, 2, 3, 1};
    for (int i = 0; i < 7; i++) CHECK_EQ(o[i], want[i]);
    CHECK(MT2D_Mixer_IsPlaying(&m, h));
    MT2D_Mixer_Stop(&m, h);
    CHECK(!MT2D_Mixer_IsPlaying(&m, h));
    o = render16(&m, 2);
    CHECK_EQ(o[0], 0);
}

TEST(one_shot_voice_frees_itself) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[2] = {5, 5};
    int h = MT2D_Mixer_Play(&m, s, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    CHECK(MT2D_Mixer_IsPlaying(&m, h));
    render16(&m, 2);
    CHECK(!MT2D_Mixer_IsPlaying(&m, h));
    CHECK_EQ(MT2D_Mixer_Active(&m), 0);
}

TEST(rendering_in_pieces_equals_rendering_at_once) {
    // crosses the internal chunk size and a loop point
    std::vector<short> s(37);
    for (size_t i = 0; i < s.size(); i++) s[i] = (short)(i * 100 - 1500);
    MT2D_Mixer a, b;
    MT2D_Mixer_Init(&a);
    MT2D_Mixer_Init(&b);
    MT2D_Mixer_Play(&a, &s[0], s.size(), MT2D_MIXER_S16, MT2D_MIXER_LOOP, 200);
    MT2D_Mixer_Play(&b, &s[0], s.size(), MT2D_MIXER_S16, MT2D_MIXER_LOOP, 200);
    std::vector<short> whole = render16(&a, 300);
    std::vector<short> parts;
    int sizes[] = {1, 63, 64, 65, 7, 100};
    for (int i = 0; i < 6; i++) {
        std::vector<short> p = render16(&b, sizes[i]);
        parts.insert(parts.end(), p.begin(), p.end());
    }
    CHECK_EQ((int)parts.size(), 300);
    for (int i = 0; i < 300; i++) CHECK_EQ(whole[i], parts[i]);
}

TEST(default_is_three_voices) {
    CHECK_EQ(MT2D_MIXER_VOICES, 3);
}

// Distinct one-sample sounds of a given value, they live as long as the test.
static short pool[16][8];
static const short *snd(int i, int value, int len) {
    for (int k = 0; k < len; k++) pool[i][k] = (short)value;
    return pool[i];
}

TEST(same_sound_restarts_its_voice) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short *a = snd(0, 1000, 4);
    int first = MT2D_Mixer_Play(&m, a, 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    render16(&m, 3); // one sample left
    int again = MT2D_Mixer_Play(&m, a, 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    CHECK(again >= 0);
    CHECK_EQ(MT2D_Mixer_Active(&m), 1); // not stacked
    CHECK(!MT2D_Mixer_IsPlaying(&m, first)); // the old handle is stale, the voice is the new sound now
    CHECK(MT2D_Mixer_IsPlaying(&m, again));
    std::vector<short> o = render16(&m, 5);
    CHECK_EQ(o[0], 1000); // it started over: 4 full samples again
    CHECK_EQ(o[3], 1000);
    CHECK_EQ(o[4], 0);
}

TEST(restart_applies_the_new_volume_and_loop_flag) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short *a = snd(0, 1000, 2);
    MT2D_Mixer_Play(&m, a, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, a, 2, MT2D_MIXER_S16, MT2D_MIXER_LOOP, MT2D_MIXER_VOL_MAX / 2);
    std::vector<short> o = render16(&m, 5); // looping now, half volume
    for (int i = 0; i < 5; i++) CHECK_EQ(o[i], 500);
}

TEST(same_data_but_other_length_or_format_is_another_sound) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short *a = snd(0, 1000, 4);
    MT2D_Mixer_Play(&m, a, 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, a, 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, a, 4, MT2D_MIXER_U8, 0, MT2D_MIXER_VOL_MAX);
    CHECK_EQ(MT2D_Mixer_Active(&m), 3);
}

TEST(full_mixer_steals_the_voice_with_the_least_left) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    // voices: long (8 left), short (2 left), medium (4 left)
    int h_long = MT2D_Mixer_Play(&m, snd(0, 100, 8), 8, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    int h_short = MT2D_Mixer_Play(&m, snd(1, 10, 2), 2, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    int h_mid = MT2D_Mixer_Play(&m, snd(2, 1, 4), 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    CHECK_EQ(MT2D_Mixer_Active(&m), 3);
    int extra = MT2D_Mixer_Play(&m, snd(3, 1000, 8), 8, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    CHECK(extra >= 0);
    CHECK_EQ(MT2D_Mixer_Active(&m), 3);
    CHECK(MT2D_Mixer_IsPlaying(&m, h_long)); // the long roar survives
    CHECK(!MT2D_Mixer_IsPlaying(&m, h_short)); // the nearly finished one is replaced
    CHECK(MT2D_Mixer_IsPlaying(&m, h_mid));
    CHECK(MT2D_Mixer_IsPlaying(&m, extra));
    std::vector<short> o = render16(&m, 1);
    CHECK_EQ(o[0], 100 + 1 + 1000);
}

TEST(steal_tie_goes_to_the_oldest_and_loops_are_last) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    int a = MT2D_Mixer_Play(&m, snd(0, 1, 4), 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX); // oldest
    int b = MT2D_Mixer_Play(&m, snd(1, 1, 4), 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    int c = MT2D_Mixer_Play(&m, snd(2, 1, 4), 4, MT2D_MIXER_S16, MT2D_MIXER_LOOP, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_Play(&m, snd(3, 1, 4), 4, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    CHECK(!MT2D_Mixer_IsPlaying(&m, a)); // a and b tie, a is older
    CHECK(MT2D_Mixer_IsPlaying(&m, b));
    CHECK(MT2D_Mixer_IsPlaying(&m, c)); // the loop is never picked while one-shots exist
}

TEST(stale_handle_does_not_touch_the_new_voice) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[1] = {7};
    int old = MT2D_Mixer_Play(&m, s, 1, MT2D_MIXER_S16, 0, MT2D_MIXER_VOL_MAX);
    render16(&m, 1); // it ended, the voice is free again
    int fresh = MT2D_Mixer_Play(&m, s, 1, MT2D_MIXER_S16, MT2D_MIXER_LOOP, MT2D_MIXER_VOL_MAX);
    CHECK(old != fresh);
    MT2D_Mixer_Stop(&m, old);
    CHECK(MT2D_Mixer_IsPlaying(&m, fresh));
    CHECK(!MT2D_Mixer_IsPlaying(&m, old));
}

TEST(invalid_arguments_are_rejected) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    const short s[1] = {1};
    CHECK_EQ(MT2D_Mixer_Play(NULL, s, 1, MT2D_MIXER_S16, 0, 256), -1);
    CHECK_EQ(MT2D_Mixer_Play(&m, NULL, 1, MT2D_MIXER_S16, 0, 256), -1);
    CHECK_EQ(MT2D_Mixer_Play(&m, s, 0, MT2D_MIXER_S16, 0, 256), -1);
    CHECK_EQ(MT2D_Mixer_Play(&m, s, 1, 9, 0, 256), -1);
    CHECK_EQ(MT2D_Mixer_Active(&m), 0);
    CHECK(!MT2D_Mixer_IsPlaying(&m, -1));
    CHECK(!MT2D_Mixer_IsPlaying(&m, 15)); // index past the voice table
    MT2D_Mixer_Stop(&m, -1);
    MT2D_Mixer_Render16(&m, NULL, 4);
    MT2D_Mixer_Render16(&m, NULL, 0);
}

TEST(stop_all_silences_everything) {
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    for (int i = 0; i < 3; i++) MT2D_Mixer_Play(&m, snd(i, 9, 1), 1, MT2D_MIXER_S16, MT2D_MIXER_LOOP, MT2D_MIXER_VOL_MAX);
    MT2D_Mixer_StopAll(&m);
    CHECK_EQ(MT2D_Mixer_Active(&m), 0);
    CHECK_EQ(render16(&m, 1)[0], 0);
}

MT2D_TEST_MAIN()
