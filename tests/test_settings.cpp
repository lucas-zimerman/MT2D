// Tests for the shared part of the settings (MT2D/MT2D_Settings.c): the result codes, the operation
// ranges and the fallbacks used when a platform has no backend. The backends (SDL, MS-DOS, ...) need
// their platform, see "Not covered" in README.md.
#include <mt2d_test.h>
#include <MT2D/MT2D_Settings.h>

static const MT2D_Setting_Operation all_operations[] = {
    MT2D_DOS_SET_AUDIO,
    MT2D_SET_PALETTE,
    MT2D_SDL_SET_WINDOW_SIZE,
    MT2D_SDL_SET_WINDOWED,
    MT2D_SDL_SET_FULLSCREEN,
    MT2D_SDL_ALT_ENTER_TOGGLE_FULLSCREEN
};
static const int operation_count = (int)(sizeof(all_operations) / sizeof(all_operations[0]));

TEST(result_values) {
    CHECK_EQ((int)MT2D_UNSUPPORTED, -3);
    CHECK_EQ((int)MT2D_BAD_PARAMETER, -2);
    CHECK_EQ((int)MT2D_BEFORE_INIT_ONLY, -1);
    CHECK_EQ((int)MT2D_INVALID, 0);
    CHECK_EQ((int)MT2D_OK, 1);
}

TEST(operations_are_grouped_by_platform) {
    // 1xxx Windows, 2xxx Linux, 3xxx macOS, 4xxx MS-DOS, 5xxx SDL, 6xxx shared
    CHECK((int)MT2D_DOS_SET_AUDIO / 1000 == 4);
    CHECK((int)MT2D_SET_PALETTE / 1000 == 6);
    CHECK((int)MT2D_SDL_SET_WINDOW_SIZE / 1000 == 5);
    CHECK((int)MT2D_SDL_SET_WINDOWED / 1000 == 5);
    CHECK((int)MT2D_SDL_SET_FULLSCREEN / 1000 == 5);
    CHECK((int)MT2D_SDL_ALT_ENTER_TOGGLE_FULLSCREEN / 1000 == 5);
}

TEST(operation_values_are_unique) {
    for (int i = 0; i < operation_count; i++)
        for (int j = i + 1; j < operation_count; j++)
            CHECK((int)all_operations[i] != (int)all_operations[j]);
}

TEST(set_settings_without_backend_is_unsupported) {
    for (int i = 0; i < operation_count; i++)
        CHECK_EQ((int)MT2D_Set_Settings(all_operations[i], 1, 1), (int)MT2D_UNSUPPORTED);
}

TEST(set_settings_unknown_operation_is_unsupported) {
    CHECK_EQ((int)MT2D_Set_Settings((MT2D_Setting_Operation)-1, 0, 0), (int)MT2D_UNSUPPORTED);
    CHECK_EQ((int)MT2D_Set_Settings((MT2D_Setting_Operation)99999, 0, 0), (int)MT2D_UNSUPPORTED);
}

TEST(argv_without_backend_is_unsupported) {
    int audio[] = { MT2D_DOS_SET_AUDIO, 0x220, 5, 1, 22050 };
    int one[] = { MT2D_SDL_SET_FULLSCREEN };
    CHECK_EQ((int)MT2D_Set_Settings_Argv(5, audio), (int)MT2D_UNSUPPORTED);
    CHECK_EQ((int)MT2D_Set_Settings_Argv(1, one), (int)MT2D_UNSUPPORTED);
}

TEST(argv_bad_arguments_are_invalid) {
    int cfg[] = { MT2D_SDL_SET_WINDOWED, 800, 600 };
    CHECK_EQ((int)MT2D_Set_Settings_Argv(0, cfg), (int)MT2D_INVALID);
    CHECK_EQ((int)MT2D_Set_Settings_Argv(-1, cfg), (int)MT2D_INVALID);
    CHECK_EQ((int)MT2D_Set_Settings_Argv(3, NULL), (int)MT2D_INVALID);
    CHECK_EQ((int)MT2D_Set_Settings_Argv(0, NULL), (int)MT2D_INVALID);
}

MT2D_TEST_MAIN()
