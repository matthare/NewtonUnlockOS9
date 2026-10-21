/* Newton Unlock for Mac OS 9.

   Classic Toolbox version of the Mac/iOS app, drawn to look like a Newton OS
   application: grey-green "LCD" window with a thick rounded border, a diamond
   picker that drops down a list, a dotted input line, and a status bar with
   an info button, Copy and the Newton close box. Everything in the window is
   drawn here with QuickDraw; the only standard parts are the menu bar and a
   TextEdit record for typing. The algorithms live in unlockers.c. */

#include <Quickdraw.h>
#include <Fonts.h>
#include <Windows.h>
#include <Menus.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Events.h>
/* Retro68's Multiversal headers put everything in one file and have no
   Scrap.h; Apple's Universal Interfaces need it. */
#ifndef __has_include
#  define __has_include(x) 1
#endif
#if __has_include(<Scrap.h>)
#  include <Scrap.h>
#endif
#include <ToolUtils.h>
#include <Devices.h>
#include <Resources.h>
#include <AppleEvents.h>
#include <Memory.h>
#include <string.h>

#include "unlockers.h"

enum {
    kMenuBarID = 128,
    kAppleMenuID = 128, kFileMenuID = 129, kEditMenuID = 130,
    kWindowID = 128
};
enum { iAbout = 1 };
enum { iQuit = 1 };
enum { iUndo = 1, iCut = 3, iCopy = 4, iPaste = 5, iClear = 6 };

/* Event Manager values, spelled out because Apple's Universal Interfaces and
   Retro68's Multiversal headers name them differently */
enum {
    kSuspendResumeMessage = 0x01,       /* high byte of an osEvt message */
    kResumeFlag = 0x01,
    kConvertClipboardFlag = 0x02
};

enum { kGeneva = 3, kTimes = 20 };

/* ---------- Newton colours (not const: the Toolbox prototypes take non-const pointers) ---------- */

static RGBColor kScreen = { 0xCCCC, 0xD4D4, 0xC2C2 };   /* grey-green LCD */
static RGBColor kInk    = { 0x1717, 0x1A1A, 0x1414 };
static RGBColor kFaded  = { 0x5C5C, 0x6262, 0x5757 };   /* ink at ~60% */

/* ---------- layout (window is 470 x 330; Rects are top, left, bottom, right) ---------- */

static const Rect kBorder     = {   6,   6, 324, 464 };
static const Rect kPickerHit  = {  54,  20,  78, 448 };
static const Rect kField      = {  90, 100, 108, 446 };
static const Rect kHint       = { 118,  22, 150, 448 };
static const Rect kResult     = { 160,  22, 232, 448 };
static const Rect kInfoBtn    = { 292,  18, 314,  44 };
static const Rect kCopyBtn    = { 292,  54, 314, 124 };
static const Rect kCloseBox   = { 295, 440, 311, 456 };
static const Rect kList       = {  80,  34, 236, 346 };      /* game picker list */
static const Rect kAboutBox   = {  80,  95, 252, 375 };
static const Rect kAboutOK    = { 218, 300, 240, 356 };

enum { kListRows = 8, kListRowH = 18, kListColW = 150 };

enum { kButtonInfo, kButtonCopy, kButtonClose, kButtonOK };

/* ---------- state ---------- */

static WindowPtr gWindow;
static TEHandle  gTE;
static Boolean   gQuit;
static int       gGame;                      /* set to World Conquest at start-up */
static char      gName[256], gRegCode[256];  /* each input type keeps its own text */
static char      gLastText[256];
static char      gResult[64];
static const char *gError;
static Boolean   gHaveResult, gCopied;

static const Game *CurrentGame(void) { return &gGames[gGame]; }

/* Games like Silicon Casino have one fixed code and nothing to type */
static Boolean HasInput(void) { return CurrentGame()->input != kInputNone; }

/* ---------- drawing helpers ---------- */

static void Ink(void)    { RGBForeColor(&kInk); }
static void Faded(void)  { RGBForeColor(&kFaded); }

static void Font(short font, short size, short face)
{
    TextFont(font);
    TextSize(size);
    TextFace(face);
}

static void DrawCString(short x, short y, const char *s)
{
    MoveTo(x, y);
    DrawText((Ptr)s, 0, (short)strlen(s));
}

static short CStringWidth(const char *s) { return TextWidth((Ptr)s, 0, (short)strlen(s)); }

static void HLine(short y, short left, short right)
{
    MoveTo(left, y);
    LineTo(right, y);
}

static void DottedLine(short y, short left, short right)
{
    short x;
    for (x = left; x <= right; x += 3) { MoveTo(x, y); LineTo(x, y); }
}

static void WrappedText(const char *s, const Rect *r)
{
    TETextBox((Ptr)s, (long)strlen(s), r, teFlushLeft);
}

static void Diamond(short x, short y)        /* filled, centred on x, y */
{
    PolyHandle p = OpenPoly();
    MoveTo(x, y - 4); LineTo(x + 4, y); LineTo(x, y + 4); LineTo(x - 4, y); LineTo(x, y - 4);
    ClosePoly();
    PaintPoly(p);
    KillPoly(p);
}

/* A box floating above the screen: solid offset shadow, thick rounded border */
static void Floater(const Rect *box, short border)
{
    Rect r = *box;
    OffsetRect(&r, 3, 3);
    Ink();
    PaintRoundRect(&r, 12, 12);
    EraseRoundRect(box, 12, 12);
    PenSize(border, border);
    FrameRoundRect(box, 12, 12);
    PenNormal();
}

static void DrawButton(int which, Boolean pressed)
{
    const Rect *r = which == kButtonInfo ? &kInfoBtn : which == kButtonCopy ? &kCopyBtn :
                    which == kButtonOK ? &kAboutOK : &kCloseBox;
    short oval = which == kButtonClose ? 6 : 14;
    Boolean enabled = which != kButtonCopy || gHaveResult;
    const char *label;

    if (pressed) { Ink(); PaintRoundRect(r, oval, oval); }
    else EraseRoundRect(r, oval, oval);
    if (enabled) Ink(); else Faded();
    FrameRoundRect(r, oval, oval);

    if (pressed) RGBForeColor(&kScreen);
    if (which == kButtonClose) {                   /* the Newton close box: an X */
        MoveTo(r->left + 4, r->top + 4);    LineTo(r->right - 5, r->bottom - 5);
        MoveTo(r->right - 5, r->top + 4);   LineTo(r->left + 4, r->bottom - 5);
    } else {
        if (which == kButtonInfo) { Font(kTimes, 14, bold | italic); label = "i"; }
        else { Font(kGeneva, 12, 0); label = which == kButtonOK ? "OK" : gCopied ? "Copied" : "Copy"; }
        DrawCString((short)((r->left + r->right - CStringWidth(label)) / 2), (short)(r->bottom - 7), label);
    }
    Ink();
}

/* Like TrackControl: show the button pressed while the mouse is inside it.
   Returns true if the mouse was released inside. */
static Boolean TrackButton(int which, const Rect *r)
{
    Boolean inside = true, shown = true;
    Point p;
    DrawButton(which, true);
    while (StillDown()) {
        GetMouse(&p);
        inside = PtInRect(p, r);
        if (inside != shown) { DrawButton(which, inside); shown = inside; }
    }
    if (shown) DrawButton(which, false);
    return inside;
}

/* ---------- the window ---------- */

static void DrawPicker(void)
{
    const Game *g = CurrentGame();
    short x;
    Ink();
    Diamond(28, 66);
    Font(kGeneva, 12, bold);
    DrawCString(38, 70, "Game");
    x = (short)(38 + CStringWidth("Game") + 8);
    Font(kGeneva, 14, 0);
    DrawCString(x, 70, g->title);
    x = (short)(x + CStringWidth(g->title) + 6);
    if (g->publisher[0]) {
        Faded();
        Font(kGeneva, 10, 0);
        DrawCString(x, 70, g->publisher);
        Ink();
    }
}

static void DrawResult(void)
{
    Rect box;
    EraseRect(&kResult);
    Ink();
    Font(kGeneva, 12, bold);
    DrawCString(22, 176, "Unlock Code");
    if (gHaveResult) {
        Font(kGeneva, 24, bold);
        DrawCString(34, 208, gResult);
    } else {
        SetRect(&box, 34, 184, 448, 232);
        Font(kGeneva, 12, 0);
        if (gError) {
            WrappedText(gError, &box);
        } else {
            Faded();
            WrappedText(CurrentGame()->input == kInputName ? "Write a name above."
                                                           : "Write a registration code above.", &box);
            Ink();
        }
    }
}

static void DrawWindow(void)
{
    const Game *g = CurrentGame();
    Rect r;

    SetPort(gWindow);
    EraseRect(&gWindow->portRect);
    Ink();

    PenSize(3, 3);                                  /* app border */
    FrameRoundRect(&kBorder, 24, 24);
    PenNormal();

    Font(kGeneva, 14, bold);                        /* title and double rule */
    DrawCString((short)((470 - CStringWidth("Newton Unlock")) / 2), 30, "Newton Unlock");
    HLine(40, 8, 461);
    HLine(43, 8, 461);

    DrawPicker();

    r = kHint;
    if (HasInput()) {
        Font(kGeneva, 12, bold);
        DrawCString(22, 104, g->input == kInputName ? "Name" : "Reg. Code");
        DottedLine(111, kField.left, kField.right);
        TEUpdate(&gWindow->portRect, gTE);
    } else {
        OffsetRect(&r, 0, -28);                     /* no field: hint moves up */
    }

    Faded();
    Font(kGeneva, 10, 0);
    WrappedText(g->hint, &r);
    Ink();

    DrawResult();

    PenSize(2, 2);                                  /* status bar */
    HLine(281, 8, 460);
    PenNormal();
    DrawButton(kButtonInfo, false);
    DrawButton(kButtonCopy, false);
    DrawButton(kButtonClose, false);
}

/* ---------- unlock code ---------- */

static void GetFieldText(char *out)
{
    long len = (**gTE).teLength;
    if (len > 255) len = 255;
    memcpy(out, *(**gTE).hText, (size_t)len);
    out[len] = 0;
}

static void Recompute(void)
{
    char text[256];
    int i, blank = 1;
    GetFieldText(text);
    for (i = 0; text[i]; i++)
        if (text[i] != ' ' && text[i] != '\t') blank = 0;
    gError = 0;
    if (!HasInput()) blank = 0;
    gHaveResult = !blank && ComputeUnlock(CurrentGame(), (const unsigned char *)text, (int)strlen(text),
                                          gResult, sizeof gResult, &gError);
    if (blank) gError = 0;
    gCopied = false;
    SetPort(gWindow);
    InvalRect(&kResult);
    InvalRect(&kCopyBtn);
}

/* After every event: recompute only when the typed text has changed */
static void CheckForEdits(void)
{
    char now[256];
    if (!HasInput()) return;
    GetFieldText(now);
    if (strcmp(now, gLastText) != 0) {
        strcpy(gLastText, now);
        strcpy(CurrentGame()->input == kInputName ? gName : gRegCode, now);
        Recompute();
    }
}

static void ShowGame(void)
{
    const char *text = !HasInput() ? "" : CurrentGame()->input == kInputName ? gName : gRegCode;
    TESetText((Ptr)text, (long)strlen(text), gTE);
    TESetSelect(0, 32767, gTE);
    if (HasInput() && FrontWindow() == gWindow) TEActivate(gTE); else TEDeactivate(gTE);
    strcpy(gLastText, text);
    Recompute();
    InvalRect(&gWindow->portRect);
}

static void CopyResult(void)
{
    if (!gHaveResult) return;
    ZeroScrap();
    PutScrap((long)strlen(gResult), 'TEXT', gResult);
    gCopied = true;
    DrawButton(kButtonCopy, false);
}

/* ---------- game picker list ---------- */

static void ListRowRect(int i, Rect *r)
{
    short left = (short)(kList.left + 4 + (i / kListRows) * kListColW);
    short top = (short)(kList.top + 4 + (i % kListRows) * kListRowH);
    SetRect(r, left, top, (short)(left + kListColW), (short)(top + kListRowH));
}

static int ListRowAt(Point p)
{
    int i;
    Rect r;
    for (i = 0; i < gGameCount; i++) {
        ListRowRect(i, &r);
        if (PtInRect(p, &r)) return i;
    }
    return -1;
}

static void DrawListRow(int i, Boolean hot)
{
    Rect r;
    ListRowRect(i, &r);
    if (hot) { Ink(); PaintRect(&r); RGBForeColor(&kScreen); }
    else { EraseRect(&r); Ink(); }
    if (i == gGame) {                                /* check mark, from the system font */
        Font(0, 12, 0);
        MoveTo(r.left + 4, r.bottom - 4);
        DrawChar((char)0x12);
    }
    Font(kGeneva, 12, 0);
    DrawCString((short)(r.left + 20), (short)(r.bottom - 5), gGames[i].title);
    Ink();
}

/* Newton-style picker: click a game to choose it, click elsewhere (or Esc) to cancel */
static void RunPicker(void)
{
    EventRecord ev;
    Point p;
    int i, hot = -1;

    Floater(&kList, 2);
    for (i = 0; i < gGameCount; i++) DrawListRow(i, false);

    for (;;) {
        if (WaitNextEvent(mDownMask | keyDownMask, &ev, 2, NULL)) {
            if (ev.what == keyDown) {
                if ((ev.message & charCodeMask) == 0x1B) break;
                continue;
            }
            p = ev.where;
            GlobalToLocal(&p);
            i = ListRowAt(p);
            if (i >= 0 && i != gGame) { gGame = i; ShowGame(); }
            while (StillDown()) {}
            break;
        }
        GetMouse(&p);
        i = ListRowAt(p);
        if (i != hot) {
            if (hot >= 0) DrawListRow(hot, false);
            if (i >= 0) DrawListRow(i, true);
            hot = i;
        }
    }
    FlushEvents(mDownMask | mUpMask, 0);
    InvalRect(&gWindow->portRect);
}

/* ---------- About box ---------- */

static void RunAbout(void)
{
    EventRecord ev;
    Point p;
    Rect text;
    char key;

    SetPort(gWindow);
    Floater(&kAboutBox, 3);
    Font(kGeneva, 12, bold);
    DrawCString((short)(kAboutBox.left + 16), (short)(kAboutBox.top + 22), "Newton Unlock");
    Font(kGeneva, 12, 0);
    SetRect(&text, (short)(kAboutBox.left + 16), (short)(kAboutBox.top + 32),
            (short)(kAboutBox.right - 14), (short)(kAboutBox.top + 80));
    WrappedText("Works out the unlock codes for Newton games from a registration code or owner name.", &text);
    SetRect(&text, (short)(kAboutBox.left + 16), (short)(kAboutBox.top + 84),
            (short)(kAboutBox.right - 14), (short)(kAboutBox.top + 116));
    WrappedText("Written by Matt Hare and AI agent Claude", &text);
    Faded();
    Font(kGeneva, 10, 0);
    DrawCString((short)(kAboutBox.left + 16), (short)(kAboutBox.top + 130), "Version 1.0");
    Ink();
    DrawButton(kButtonOK, false);

    for (;;) {
        if (!WaitNextEvent(mDownMask | keyDownMask, &ev, 30, NULL)) continue;
        if (ev.what == keyDown) {
            key = (char)(ev.message & charCodeMask);
            if (key == '\r' || key == 3 || key == 0x1B) {
                unsigned long t;
                DrawButton(kButtonOK, true);
                Delay(8, &t);
                break;
            }
            continue;
        }
        p = ev.where;
        GlobalToLocal(&p);
        if (PtInRect(p, &kAboutOK)) { if (TrackButton(kButtonOK, &kAboutOK)) break; }
        else if (!PtInRect(p, &kAboutBox)) break;
    }
    InvalRect(&gWindow->portRect);
}

/* ---------- menus ---------- */

static void DoMenu(long choice)
{
    short menu = (short)(choice >> 16), item = (short)(choice & 0xFFFF);
    Str255 name;

    switch (menu) {
    case kAppleMenuID:
        if (item == iAbout) {
            RunAbout();
        } else {
            GetMenuItemText(GetMenuHandle(kAppleMenuID), item, name);
            OpenDeskAcc(name);
        }
        break;
    case kFileMenuID:
        if (item == iQuit) gQuit = true;
        break;
    case kEditMenuID:
        if (SystemEdit(item - 1)) break;          /* a desk accessory took it */
        if (FrontWindow() != gWindow || !HasInput()) break;
        switch (item) {
        case iCut:   TECut(gTE);  ZeroScrap(); TEToScrap(); break;
        case iCopy:  TECopy(gTE); ZeroScrap(); TEToScrap(); break;
        case iPaste: TEFromScrap(); TEPaste(gTE); break;
        case iClear: TEDelete(gTE); break;
        }
        break;
    }
    HiliteMenu(0);
}

/* ---------- events ---------- */

static pascal OSErr QuitHandler(const AppleEvent *event, AppleEvent *reply, long refCon)
{
    (void)event; (void)reply; (void)refCon;
    gQuit = true;
    return noErr;
}

static void ContentClick(EventRecord *ev)
{
    Point p = ev->where;
    SetPort(gWindow);
    GlobalToLocal(&p);

    if (HasInput() && PtInRect(p, &kField)) TEClick(p, (ev->modifiers & shiftKey) != 0, gTE);
    else if (PtInRect(p, &kPickerHit)) RunPicker();
    else if (PtInRect(p, &kInfoBtn))   { if (TrackButton(kButtonInfo, &kInfoBtn)) RunAbout(); }
    else if (PtInRect(p, &kCopyBtn))   { if (gHaveResult && TrackButton(kButtonCopy, &kCopyBtn)) CopyResult(); }
    else if (PtInRect(p, &kCloseBox))  { if (TrackButton(kButtonClose, &kCloseBox)) gQuit = true; }
    else DragWindow(gWindow, ev->where, &qd.screenBits.bounds);   /* drag by the background */
}

static void Activate(Boolean active)
{
    if (active && HasInput()) TEActivate(gTE); else TEDeactivate(gTE);
}

static void HandleEvent(EventRecord *ev)
{
    WindowPtr win;
    char key;

    switch (ev->what) {
    case nullEvent:
        if (FrontWindow() == gWindow) {
            Point p;
            SetPort(gWindow);
            TEIdle(gTE);
            GetMouse(&p);
            if (HasInput() && PtInRect(p, &kField)) SetCursor(*GetCursor(iBeamCursor));
            else SetCursor(&qd.arrow);
        }
        break;

    case mouseDown:
        switch (FindWindow(ev->where, &win)) {
        case inMenuBar:   DoMenu(MenuSelect(ev->where)); break;
        case inSysWindow: SystemClick(ev, win); break;
        case inContent:
            if (win != FrontWindow()) SelectWindow(win);
            else if (win == gWindow) ContentClick(ev);
            break;
        }
        break;

    case keyDown:
    case autoKey:
        key = (char)(ev->message & charCodeMask);
        if (ev->modifiers & cmdKey) { if (ev->what == keyDown) DoMenu(MenuKey(key)); break; }
        if (key == '\r' || key == 3) {                     /* Return / Enter: Copy */
            unsigned long t;
            if (!gHaveResult) break;
            DrawButton(kButtonCopy, true);
            Delay(8, &t);
            CopyResult();
            break;
        }
        if (key == '\t' || key == 0x1B || !HasInput()) break;
        TEKey(key, gTE);
        break;

    case updateEvt:
        win = (WindowPtr)ev->message;
        BeginUpdate(win);
        if (win == gWindow) DrawWindow();
        EndUpdate(win);
        break;

    case activateEvt:
        if ((WindowPtr)ev->message == gWindow) Activate((ev->modifiers & activeFlag) != 0);
        break;

    case osEvt:
        if ((ev->message >> 24 & 0xFF) == kSuspendResumeMessage) {
            Boolean resume = (ev->message & kResumeFlag) != 0;
            if (resume && (ev->message & kConvertClipboardFlag)) TEFromScrap();
            Activate(resume);
        }
        break;

    case kHighLevelEvent:
        AEProcessAppleEvent(ev);
        break;
    }
}

/* ---------- start-up ---------- */

static void Initialize(void)
{
    Rect dest;

    MaxApplZone();
    MoreMasters(); MoreMasters();
    InitGraf(&qd.thePort);
    InitFonts();
    InitWindows();
    InitMenus();
    TEInit();
    InitDialogs(NULL);
    InitCursor();
    FlushEvents(everyEvent, 0);

    SetMenuBar(GetNewMBar(kMenuBarID));
    AppendResMenu(GetMenuHandle(kAppleMenuID), 'DRVR');
    DrawMenuBar();

    AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
                          NewAEEventHandlerUPP(QuitHandler), 0, false);

    gWindow = GetNewCWindow(kWindowID, NULL, (WindowPtr)-1L);
    SetPort(gWindow);
    RGBBackColor(&kScreen);                 /* every erase paints the LCD colour */
    Ink();

    Font(kGeneva, 14, 0);                   /* TENew takes the port's font */
    dest = kField;
    dest.right = (short)(dest.left + 3000); /* one long line, scrolled sideways */
    gTE = TENew(&dest, &kField);
    TEAutoView(true, gTE);

    for (gGame = 0; gGame < gGameCount - 1; gGame++)
        if (strcmp(gGames[gGame].title, "World Conquest") == 0) break;
    ShowGame();
    ShowWindow(gWindow);
}

int main(void)
{
    EventRecord ev;

    Initialize();
    while (!gQuit) {
        if (!WaitNextEvent(everyEvent, &ev, GetCaretTime(), NULL)) ev.what = nullEvent;
        HandleEvent(&ev);
        CheckForEdits();
    }
    return 0;
}
