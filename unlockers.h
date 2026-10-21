/* Unlock-code algorithms, ported from the Python scripts in each game's folder
   (e.g. worldcon/worldcon_unlock.py). Plain ANSI C with no Toolbox calls, so the
   same file builds for Mac OS 9 and on a modern machine for testing. */

#ifndef UNLOCKERS_H
#define UNLOCKERS_H

enum { kInputRegCode = 0, kInputName = 1, kInputNone = 2 };   /* None: one fixed code */

enum {
    kSchemeTactile,      /* param = two-letter salt */
    kSchemeMineField,
    kSchemeNewtopoly,
    kSchemeNewtHack,
    kSchemeScrawl,       /* param = suffix ("YAZI", "Mahjongg") */
    kSchemeDeepSpaceNewt,
    kSchemeFixed         /* param = the code itself */
};

typedef struct {
    const char *title;
    const char *publisher;
    int         input;      /* kInputRegCode, kInputName or kInputNone */
    int         scheme;
    const char *param;
    const char *hint;
} Game;

extern const Game gGames[];
extern const int  gGameCount;

/* text is Mac OS Roman, len bytes. On success writes a C string to out and
   returns 1. On failure returns 0 and sets *error to a message. */
int ComputeUnlock(const Game *game, const unsigned char *text, int len,
                  char *out, int outSize, const char **error);

#endif
