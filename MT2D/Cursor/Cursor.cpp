#if ! defined(SDL_USE) || !defined(WINDOWS_TARGET) || !defined(__MSDOS__) || !defined(linux)
void MT2D_Cursor_goto_topscreen() {}
void MT2D_Cursor_Set_Position(int line, int column) {}
#endif