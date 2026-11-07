// Tests for MT2D/Container/aes.cpp (AES-128, known answer tests from NIST SP 800-38A)
#include <mt2d_test.h>
#include <MT2D/Container/aes.h>
#include <string.h>

static const uint8_t key[16] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
static const uint8_t iv[16] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

static const uint8_t plain[32] = {
    0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a,
    0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51};

TEST(ecb_encrypt_known_answer) {
    uint8_t expected[16] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
    uint8_t buf[16];
    memcpy(buf, plain, 16);
    AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    AES_ECB_encrypt(&ctx, buf);
    CHECK(memcmp(buf, expected, 16) == 0);
}

TEST(ecb_decrypt_known_answer) {
    uint8_t buf[16] = {0x3a, 0xd7, 0x7b, 0xb4, 0x0d, 0x7a, 0x36, 0x60, 0xa8, 0x9e, 0xca, 0xf3, 0x24, 0x66, 0xef, 0x97};
    AES_ctx ctx;
    AES_init_ctx(&ctx, key);
    AES_ECB_decrypt(&ctx, buf);
    CHECK(memcmp(buf, plain, 16) == 0);
}

TEST(cbc_encrypt_known_answer) {
    uint8_t expected[32] = {
        0x76, 0x49, 0xab, 0xac, 0x81, 0x19, 0xb2, 0x46, 0xce, 0xe9, 0x8e, 0x9b, 0x12, 0xe9, 0x19, 0x7d,
        0x50, 0x86, 0xcb, 0x9b, 0x50, 0x72, 0x19, 0xee, 0x95, 0xdb, 0x11, 0x3a, 0x91, 0x76, 0x78, 0xb2};
    uint8_t buf[32];
    memcpy(buf, plain, 32);
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, buf, 32);
    CHECK(memcmp(buf, expected, 32) == 0);
}

TEST(cbc_roundtrip) {
    uint8_t buf[32];
    memcpy(buf, plain, 32);
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, buf, 32);
    CHECK(memcmp(buf, plain, 32) != 0);
    AES_ctx_set_iv(&ctx, iv);
    AES_CBC_decrypt_buffer(&ctx, buf, 32);
    CHECK(memcmp(buf, plain, 32) == 0);
}

TEST(ctr_is_its_own_inverse) {
    uint8_t buf[32];
    memcpy(buf, plain, 32);
    AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CTR_xcrypt_buffer(&ctx, buf, 32);
    CHECK(memcmp(buf, plain, 32) != 0);
    AES_ctx_set_iv(&ctx, iv);
    AES_CTR_xcrypt_buffer(&ctx, buf, 32);
    CHECK(memcmp(buf, plain, 32) == 0);
}

TEST(different_keys_give_different_output) {
    uint8_t other[16];
    memcpy(other, key, 16);
    other[0] ^= 1;
    uint8_t a[16], b[16];
    memcpy(a, plain, 16);
    memcpy(b, plain, 16);
    AES_ctx c1, c2;
    AES_init_ctx(&c1, key);
    AES_init_ctx(&c2, other);
    AES_ECB_encrypt(&c1, a);
    AES_ECB_encrypt(&c2, b);
    CHECK(memcmp(a, b, 16) != 0);
}

MT2D_TEST_MAIN()
