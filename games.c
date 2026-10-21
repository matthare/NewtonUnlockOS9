#include "unlockers.h"

/* Same order as GAMES in tests/check.py -- keep them in step. The picker list
   in main.c holds up to 16. */

#define TACTILE_HINT "Enter the registration code shown on the game's registration screen. " \
                     "The X at each end of the unlock code can be any letter."

const Game gGames[] = {
    { "Columns",        "Tactile Systems", kInputRegCode, kSchemeTactile, "MF", TACTILE_HINT },
    { "DeepSpaceNewt",  "DejaVu Software", kInputName, kSchemeDeepSpaceNewt, 0,
      "Enter the owner name exactly as it appears on the Newton's owner card." },
    { "Mahjongg",       "Scrawl",          kInputName, kSchemeScrawl, "Mahjongg",
      "Enter the owner name from the Newton's owner card, or the Serial Number shown in Mahjongg's About box." },
    { "MineField II",   "Tactile Systems", kInputRegCode, kSchemeMineField, 0,
      "Enter the registration code shown on the game's registration screen. "
      "Type the unlock code exactly, including the X at each end." },
    { "Motile",         "Tactile Systems", kInputRegCode, kSchemeTactile, "FK", TACTILE_HINT },
    { "NewtHack",       "Chris Dopp",      kInputName, kSchemeNewtHack, 0,
      "Enter the owner name from the Newton's owner card. Enter the number with NewtHack's number picker." },
    { "Newtopoly",      "",                kInputName, kSchemeNewtopoly, 0,
      "Enter the name exactly as you will type it on Newtopoly's registration slip." },
    { "Scramble",       "Tactile Systems", kInputRegCode, kSchemeTactile, "OW", TACTILE_HINT },
    { "Sea Hunt",       "Tactile Systems", kInputRegCode, kSchemeTactile, "VB", TACTILE_HINT },
    /* fixed code, from siliconcasino/serialnumber.txt */
    { "Silicon Casino", "Casady & Greene", kInputNone, kSchemeFixed, "20837826",
      "No input needed: this is your Silicon Casino registration code." },
    { "Solito DX",      "Tactile Systems", kInputRegCode, kSchemeTactile, "BL", TACTILE_HINT },
    { "Super Newtris",  "Tactile Systems", kInputRegCode, kSchemeTactile, "NS", TACTILE_HINT },
    { "World Conquest", "Tactile Systems", kInputRegCode, kSchemeTactile, "KQ", TACTILE_HINT },
    { "xWord Buddy",    "Tactile Systems", kInputRegCode, kSchemeTactile, "UO", TACTILE_HINT },
    { "xWord Search",   "Tactile Systems", kInputRegCode, kSchemeTactile, "YA", TACTILE_HINT },
    { "YAZI",           "Scrawl",          kInputName, kSchemeScrawl, "YAZI",
      "Enter the owner name from the Newton's owner card (or an owner persona's name)." },
};

const int gGameCount = sizeof(gGames) / sizeof(gGames[0]);
