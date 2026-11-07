// Minimal header-only test framework for the MT2D tests (no external dependencies).
//
//   TEST(name) { CHECK(cond); CHECK_EQ(a, b); CHECK_STR_EQ(a, b); }
//
// Link the test file with `int main() { return mt2d_test::run_all(); }`,
// which is already provided by MT2D_TEST_MAIN() at the end of each test file.
#pragma once
#include <stdio.h>
#include <string.h>
#include <vector>

namespace mt2d_test {

struct Case {
    const char *name;
    void (*fn)();
    bool xfail; // known engine bug: the test documents the correct behaviour and is expected to fail
};

inline std::vector<Case> &cases() {
    static std::vector<Case> c;
    return c;
}

inline int &failures_in_case() {
    static int f = 0;
    return f;
}

struct Registrar {
    Registrar(const char *name, void (*fn)(), bool xfail = false) { cases().push_back(Case{name, fn, xfail}); }
};

inline int run_all() {
    setvbuf(stdout, NULL, _IONBF, 0); // keep output visible if a test crashes or hangs
    int failed = 0;
    for (size_t i = 0; i < cases().size(); i++) {
        failures_in_case() = 0;
        cases()[i].fn();
        if (cases()[i].xfail) {
            if (failures_in_case() == 0) {
                printf("[ XPASS] %s (known bug seems fixed, change TEST_XFAIL to TEST)\n", cases()[i].name);
            } else {
                printf("[ XFAIL] %s (known bug)\n", cases()[i].name);
            }
        } else if (failures_in_case() == 0) {
            printf("[ OK   ] %s\n", cases()[i].name);
        } else {
            printf("[ FAIL ] %s\n", cases()[i].name);
            failed++;
        }
    }
    printf("%d/%d tests passed\n", (int)cases().size() - failed, (int)cases().size());
    return failed == 0 ? 0 : 1;
}

} // namespace mt2d_test

#define TEST(name) \
    static void name(); \
    static mt2d_test::Registrar registrar_##name(#name, name); \
    static void name()

// Same as TEST, for a test that describes the intended behaviour of something
// that is currently broken in the engine. It does not fail the run.
#define TEST_XFAIL(name) \
    static void name(); \
    static mt2d_test::Registrar registrar_##name(#name, name, true); \
    static void name()

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            printf("    %s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #cond); \
            mt2d_test::failures_in_case()++; \
        } \
    } while (0)

#define CHECK_EQ(a, b) \
    do { \
        long long va_ = (long long)(a), vb_ = (long long)(b); \
        if (va_ != vb_) { \
            printf("    %s:%d: CHECK_EQ(%s, %s) failed: %lld != %lld\n", __FILE__, __LINE__, #a, #b, va_, vb_); \
            mt2d_test::failures_in_case()++; \
        } \
    } while (0)

#define CHECK_STR_EQ(a, b) \
    do { \
        const char *sa_ = (const char *)(a), *sb_ = (const char *)(b); \
        if (!sa_ || !sb_ || strcmp(sa_, sb_) != 0) { \
            printf("    %s:%d: CHECK_STR_EQ(%s, %s) failed: \"%s\" != \"%s\"\n", __FILE__, __LINE__, #a, #b, sa_ ? sa_ : "(null)", sb_ ? sb_ : "(null)"); \
            mt2d_test::failures_in_case()++; \
        } \
    } while (0)

#define MT2D_TEST_MAIN() \
    int main() { return mt2d_test::run_all(); }
