/* Resources for Newton Unlock (Mac OS 9): the window, menus, version, Finder
   bundle and icons. Everything inside the window is drawn by main.c. */

#include "Types.r"
#include "Processes.r"


/* ---------- main window ---------- */

/* Plain box: no title bar; main.c draws the Newton frame and lets the user
   drag the window by its background. */
resource 'WIND' (128, "Newton Unlock") {
    {0, 0, 330, 470},
    plainDBox,
    invisible,
    noGoAway,
    0x0,
    "Newton Unlock",
    centerMainScreen
};

/* ---------- menus ---------- */

resource 'MBAR' (128) { { 128; 129; 130 } };

resource 'MENU' (128) {
    128, textMenuProc, 0x7FFFFFFD, enabled, apple,
    {
        "About Newton Unlock\0xC9", noIcon, noKey, noMark, plain;
        "-", noIcon, noKey, noMark, plain
    }
};

resource 'MENU' (129) {
    129, textMenuProc, allEnabled, enabled, "File",
    {
        "Quit", noIcon, "Q", noMark, plain
    }
};

resource 'MENU' (130) {
    130, textMenuProc, 0x7FFFFFFC, enabled, "Edit",
    {
        "Undo", noIcon, "Z", noMark, plain;
        "-", noIcon, noKey, noMark, plain;
        "Cut", noIcon, "X", noMark, plain;
        "Copy", noIcon, "C", noMark, plain;
        "Paste", noIcon, "V", noMark, plain;
        "Clear", noIcon, noKey, noMark, plain
    }
};

/* ---------- application info ---------- */

resource 'vers' (1) {
    0x01, 0x00, release, 0x00, verUS,
    "1.0",
    "1.0, Newton Unlock"
};

resource 'SIZE' (-1) {
    reserved,
    acceptSuspendResumeEvents,
    reserved,
    canBackground,
    doesActivateOnFGSwitch,
    backgroundAndForeground,
    dontGetFrontClicks,
    ignoreChildDiedEvents,
    is32BitCompatible,
    isHighLevelEventAware,
    onlyLocalHLEvents,
    notStationeryAware,
    dontUseTextEditServices,
    reserved,
    reserved,
    reserved,
    512 * 1024,
    384 * 1024
};

/* Finder bundle: creator 'NwUl' */
type 'NwUl' as 'STR ';
resource 'NwUl' (0, "Owner resource") { "Newton Unlock 1.0" };

resource 'BNDL' (128) {
    'NwUl', 0,
    {
        'ICN#', { 0, 128 };
        'FREF', { 0, 128 }
    }
};

resource 'FREF' (128) { 'APPL', 0, "" };

#include "icons.r"
