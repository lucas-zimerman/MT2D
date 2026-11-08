// Tests for MT2D/Audio/MT2D_Wav.c (WAV header reader and rate converter for the mixer)
#include <mt2d_test.h>
#include <MT2D/Audio/MT2D_Wav.h>
#include <string.h>
#include <vector>

typedef std::vector<unsigned char> Bytes;

static void put32(Bytes &b, unsigned long v) {
    for (int i = 0; i < 4; i++) b.push_back((unsigned char)(v >> (8 * i)));
}
static void put16(Bytes &b, unsigned v) {
    b.push_back((unsigned char)v);
    b.push_back((unsigned char)(v >> 8));
}
static void tag(Bytes &b, const char *t) { b.insert(b.end(), t, t + 4); }

// A canonical 44 byte header file around `pcm`.
static Bytes wav(unsigned rate, unsigned bits, const Bytes &pcm, unsigned channels = 1, unsigned fmt_tag = 1) {
    Bytes b;
    tag(b, "RIFF");
    put32(b, 36 + pcm.size());
    tag(b, "WAVE");
    tag(b, "fmt ");
    put32(b, 16);
    put16(b, fmt_tag);
    put16(b, channels);
    put32(b, rate);
    put32(b, rate * channels * bits / 8);
    put16(b, channels * bits / 8);
    put16(b, bits);
    tag(b, "data");
    put32(b, pcm.size());
    b.insert(b.end(), pcm.begin(), pcm.end());
    return b;
}

static Bytes bytes(const unsigned char *p, int n) { return Bytes(p, p + n); }

TEST(parses_8_bit_mono) {
    const unsigned char pcm[4] = {1, 2, 3, 4};
    Bytes f = wav(22050, 8, bytes(pcm, 4));
    MT2D_Wav w;
    CHECK_EQ(MT2D_Wav_Parse(&f[0], f.size(), &w), MT2D_WAV_OK);
    CHECK_EQ((int)w.samples, 4);
    CHECK_EQ((int)w.rate, 22050);
    CHECK_EQ(w.format, MT2D_MIXER_U8);
    CHECK(w.data == &f[44]);
}

TEST(parses_16_bit_mono) {
    const unsigned char pcm[8] = {1, 0, 2, 0, 3, 0, 4, 0};
    Bytes f = wav(11025, 16, bytes(pcm, 8));
    MT2D_Wav w;
    CHECK_EQ(MT2D_Wav_Parse(&f[0], f.size(), &w), MT2D_WAV_OK);
    CHECK_EQ((int)w.samples, 4);
    CHECK_EQ((int)w.rate, 11025);
    CHECK_EQ(w.format, MT2D_MIXER_S16);
}

TEST(skips_unknown_chunks_with_padding) {
    // RIFF, WAVE, LIST (odd size 3 + pad), fmt, data
    Bytes f;
    tag(f, "RIFF"); put32(f, 0); tag(f, "WAVE");
    tag(f, "LIST"); put32(f, 3); f.push_back(9); f.push_back(9); f.push_back(9); f.push_back(0);
    tag(f, "fmt "); put32(f, 16); put16(f, 1); put16(f, 1); put32(f, 8000); put32(f, 8000); put16(f, 1); put16(f, 8);
    tag(f, "data"); put32(f, 2); f.push_back(7); f.push_back(8);
    MT2D_Wav w;
    CHECK_EQ(MT2D_Wav_Parse(&f[0], f.size(), &w), MT2D_WAV_OK);
    CHECK_EQ((int)w.samples, 2);
    CHECK_EQ(w.data[0], 7);
}

TEST(cut_data_chunk_is_clamped_to_the_file) {
    const unsigned char pcm[4] = {1, 2, 3, 4};
    Bytes f = wav(8000, 8, bytes(pcm, 4));
    f.resize(f.size() - 2); // the header still says 4 samples
    MT2D_Wav w;
    CHECK_EQ(MT2D_Wav_Parse(&f[0], f.size(), &w), MT2D_WAV_OK);
    CHECK_EQ((int)w.samples, 2);
}

TEST(rejects_what_the_mixer_cannot_play) {
    const unsigned char pcm[4] = {1, 2, 3, 4};
    MT2D_Wav w;
    Bytes st = wav(22050, 8, bytes(pcm, 4), 2);
    CHECK_EQ(MT2D_Wav_Parse(&st[0], st.size(), &w), MT2D_WAV_ERR_UNSUPPORTED); // stereo
    Bytes adpcm = wav(22050, 8, bytes(pcm, 4), 1, 2);
    CHECK_EQ(MT2D_Wav_Parse(&adpcm[0], adpcm.size(), &w), MT2D_WAV_ERR_UNSUPPORTED);
    Bytes b24 = wav(22050, 24, bytes(pcm, 4));
    CHECK_EQ(MT2D_Wav_Parse(&b24[0], b24.size(), &w), MT2D_WAV_ERR_UNSUPPORTED);
    Bytes fast = wav(96000, 8, bytes(pcm, 4));
    CHECK_EQ(MT2D_Wav_Parse(&fast[0], fast.size(), &w), MT2D_WAV_ERR_UNSUPPORTED);
}

TEST(rejects_garbage) {
    MT2D_Wav w;
    unsigned char junk[64];
    memset(junk, 0x55, sizeof(junk));
    CHECK_EQ(MT2D_Wav_Parse(NULL, 10, &w), MT2D_WAV_ERR_ARGS);
    CHECK_EQ(MT2D_Wav_Parse(junk, 64, NULL), MT2D_WAV_ERR_ARGS);
    CHECK_EQ(MT2D_Wav_Parse(junk, 5, &w), MT2D_WAV_ERR_FORMAT);
    CHECK_EQ(MT2D_Wav_Parse(junk, 64, &w), MT2D_WAV_ERR_FORMAT);
    const unsigned char pcm[2] = {1, 2};
    Bytes f = wav(8000, 8, bytes(pcm, 2));
    // every truncation of a good file must fail cleanly, never read past the end
    for (size_t n = 0; n < 44; n++) {
        CHECK(MT2D_Wav_Parse(&f[0], n, &w) != MT2D_WAV_OK);
    }
    Bytes nodata = f;
    nodata.resize(36);
    CHECK_EQ(MT2D_Wav_Parse(&nodata[0], nodata.size(), &w), MT2D_WAV_ERR_FORMAT);
}

TEST(data_before_fmt_is_rejected) {
    Bytes f;
    tag(f, "RIFF"); put32(f, 0); tag(f, "WAVE");
    tag(f, "data"); put32(f, 2); f.push_back(1); f.push_back(2);
    tag(f, "fmt "); put32(f, 16); put16(f, 1); put16(f, 1); put32(f, 8000); put32(f, 8000); put16(f, 1); put16(f, 8);
    MT2D_Wav w;
    CHECK_EQ(MT2D_Wav_Parse(&f[0], f.size(), &w), MT2D_WAV_ERR_FORMAT);
}

TEST(resampled_length) {
    CHECK_EQ((int)MT2D_Wav_ResampledLength(100, 11025, 22050), 200);
    CHECK_EQ((int)MT2D_Wav_ResampledLength(100, 44100, 22050), 50);
    CHECK_EQ((int)MT2D_Wav_ResampledLength(100, 22050, 22050), 100);
    CHECK_EQ((int)MT2D_Wav_ResampledLength(1, 44100, 8000), 1); // never zero
    CHECK_EQ((int)MT2D_Wav_ResampledLength(10, 0, 22050), 0);
    // big sounds must not overflow 32 bits: 4M samples * 22050
    CHECK(MT2D_Wav_ResampledLength(4000000UL, 22050, 22050) == 4000000UL);
}

TEST(resample_8_bit_doubles_and_halves) {
    const unsigned char s[4] = {10, 20, 30, 40};
    unsigned char up[8], down[2];
    MT2D_Wav_Resample(s, 4, MT2D_MIXER_U8, 1000, 2000, up);
    const unsigned char want_up[8] = {10, 10, 20, 20, 30, 30, 40, 40};
    CHECK(memcmp(up, want_up, 8) == 0);
    MT2D_Wav_Resample(s, 4, MT2D_MIXER_U8, 2000, 1000, down);
    CHECK_EQ(down[0], 10);
    CHECK_EQ(down[1], 30);
}

TEST(resample_16_bit_reads_little_endian) {
    const unsigned char s[4] = {0x34, 0x12, 0x00, 0x80}; // 0x1234, -32768
    short same[2];
    MT2D_Wav_Resample(s, 2, MT2D_MIXER_S16, 22050, 22050, same);
    CHECK_EQ(same[0], 0x1234);
    CHECK_EQ(same[1], -32768);
    short up[4];
    MT2D_Wav_Resample(s, 2, MT2D_MIXER_S16, 11025, 22050, up);
    CHECK_EQ(up[0], 0x1234);
    CHECK_EQ(up[1], 0x1234);
    CHECK_EQ(up[2], -32768);
    CHECK_EQ(up[3], -32768);
}

TEST(resample_never_reads_past_the_source) {
    // odd ratio: the last output samples must map to the last source sample
    std::vector<unsigned char> s(7);
    for (int i = 0; i < 7; i++) s[i] = (unsigned char)(i + 1);
    unsigned long n = MT2D_Wav_ResampledLength(7, 8000, 22050);
    std::vector<unsigned char> out(n + 1, 0xEE);
    MT2D_Wav_Resample(&s[0], 7, MT2D_MIXER_U8, 8000, 22050, &out[0]);
    CHECK_EQ(out[n - 1], 7);
    CHECK_EQ(out[n], 0xEE); // nothing written past the end
    for (unsigned long i = 0; i < n; i++) CHECK(out[i] >= 1 && out[i] <= 7);
}

TEST(wav_feeds_the_mixer) {
    const unsigned char pcm[3] = {128, 138, 118};
    Bytes f = wav(22050, 8, bytes(pcm, 3));
    MT2D_Wav w;
    CHECK_EQ(MT2D_Wav_Parse(&f[0], f.size(), &w), MT2D_WAV_OK);
    MT2D_Mixer m;
    MT2D_Mixer_Init(&m);
    CHECK(MT2D_Mixer_Play(&m, w.data, w.samples, w.format, 0, MT2D_MIXER_VOL_MAX) >= 0);
    unsigned char out[3];
    MT2D_Mixer_Render8(&m, out, 3);
    CHECK_EQ(out[0], 128);
    CHECK_EQ(out[1], 138);
    CHECK_EQ(out[2], 118);
}

MT2D_TEST_MAIN()
