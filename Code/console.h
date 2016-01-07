#ifndef console_h
#define console_h


// This function dynamically creates a "Console" window and points stdout and stderr to it.
// It also hooks stdin to the window
// You must free it later with FreeConsole
void RedirectIOToConsole(const LPCWSTR title);

#endif
