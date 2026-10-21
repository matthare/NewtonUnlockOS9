/* Ports of the Python unlock scripts (see each game's folder). Each function
   must give exactly the same answer as its Python original; tests/check.py
   compares them. Text arrives as Mac OS Roman and is converted to Unicode code
   points first, because the Newton (and Python's ord()) work in Unicode. */

#include <string.h>
#include "unlockers.h"

typedef unsigned long u32;   /* at least 32 bits everywhere we build */

#define MAXCHARS 300         /* a Str255 plus the longest suffix, with room */

static const unsigned short kMacRomanHigh[128] = {
    0x00C4, 0x00C5, 0x00C7, 0x00C9, 0x00D1, 0x00D6, 0x00DC, 0x00E1,
    0x00E0, 0x00E2, 0x00E4, 0x00E3, 0x00E5, 0x00E7, 0x00E9, 0x00E8,
    0x00EA, 0x00EB, 0x00ED, 0x00EC, 0x00EE, 0x00EF, 0x00F1, 0x00F3,
    0x00F2, 0x00F4, 0x00F6, 0x00F5, 0x00FA, 0x00F9, 0x00FB, 0x00FC,
    0x2020, 0x00B0, 0x00A2, 0x00A3, 0x00A7, 0x2022, 0x00B6, 0x00DF,
    0x00AE, 0x00A9, 0x2122, 0x00B4, 0x00A8, 0x2260, 0x00C6, 0x00D8,
    0x221E, 0x00B1, 0x2264, 0x2265, 0x00A5, 0x00B5, 0x2202, 0x2211,
    0x220F, 0x03C0, 0x222B, 0x00AA, 0x00BA, 0x03A9, 0x00E6, 0x00F8,
    0x00BF, 0x00A1, 0x00AC, 0x221A, 0x0192, 0x2248, 0x2206, 0x00AB,
    0x00BB, 0x2026, 0x00A0, 0x00C0, 0x00C3, 0x00D5, 0x0152, 0x0153,
    0x2013, 0x2014, 0x201C, 0x201D, 0x2018, 0x2019, 0x00F7, 0x25CA,
    0x00FF, 0x0178, 0x2044, 0x20AC, 0x2039, 0x203A, 0xFB01, 0xFB02,
    0x2021, 0x00B7, 0x201A, 0x201E, 0x2030, 0x00C2, 0x00CA, 0x00C1,
    0x00CB, 0x00C8, 0x00CD, 0x00CE, 0x00CF, 0x00CC, 0x00D3, 0x00D4,
    0xF8FF, 0x00D2, 0x00DA, 0x00DB, 0x00D9, 0x0131, 0x02C6, 0x02DC,
    0x00AF, 0x02D8, 0x02D9, 0x02DA, 0x00B8, 0x02DD, 0x02DB, 0x02C7,
};

static const unsigned char kMD2S[256] = {
     41,  46,  67, 201, 162, 216, 124,   1,  61,  54,  84, 161, 236, 240,   6,  19,
     98, 167,   5, 243, 192, 199, 115, 140, 152, 147,  43, 217, 188,  76, 130, 202,
     30, 155,  87,  60, 253, 212, 224,  22, 103,  66, 111,  24, 138,  23, 229,  18,
    190,  78, 196, 214, 218, 158, 222,  73, 160, 251, 245, 142, 187,  47, 238, 122,
    169, 104, 121, 145,  21, 178,   7,  63, 148, 194,  16, 137,  11,  34,  95,  33,
    128, 127,  93, 154,  90, 144,  50,  39,  53,  62, 204, 231, 191, 247, 151,   3,
    255,  25,  48, 179,  72, 165, 181, 209, 215,  94, 146,  42, 172,  86, 170, 198,
     79, 184,  56, 210, 150, 164, 125, 182, 118, 252, 107, 226, 156, 116,   4, 241,
     69, 157, 112,  89, 100, 113, 135,  32, 134,  91, 207, 101, 230,  45, 168,   2,
     27,  96,  37, 173, 174, 176, 185, 246,  28,  70,  97, 105,  52,  64, 126,  15,
     85,  71, 163,  35, 221,  81, 175,  58, 195,  92, 249, 206, 186, 197, 234,  38,
     44,  83,  13, 110, 133,  40, 132,   9, 211, 223, 205, 244,  65, 129,  77,  82,
    106, 220,  55, 200, 108, 193, 171, 250,  36, 225, 123,   8,  12, 189, 177,  74,
    120, 136, 149, 139, 227,  99, 232, 109, 233, 203, 213, 254,  59,   0,  29,  57,
    242, 239, 183,  14, 102,  88, 208, 228, 166, 119, 114, 248, 235, 117,  75,  10,
     49,  68,  80, 180, 143, 237,  31,  26, 219, 153, 141,  51, 159,  17, 131,  20,
};

/* ---------- helpers ---------- */

static int ToUnicode(const unsigned char *text, int len, u32 *out)
{
    int i;
    if (len > 255) len = 255;
    for (i = 0; i < len; i++)
        out[i] = text[i] < 0x80 ? text[i] : kMacRomanHigh[text[i] - 0x80];
    return len;
}

static int AppendASCII(u32 *s, int n, const char *suffix)
{
    while (*suffix) s[n++] = (unsigned char)*suffix++;
    return n;
}

/* Python's str.isspace() for the characters Mac OS Roman can hold */
static int IsSpace(u32 c)
{
    return (c >= 0x09 && c <= 0x0D) || (c >= 0x1C && c <= 0x20) || c == 0xA0;
}

/* Python's str.lower() for the characters Mac OS Roman can hold */
static u32 ToLower(u32 c)
{
    if (c >= 'A' && c <= 'Z') return c + 32;
    if (c >= 0xC0 && c <= 0xDE && c != 0xD7) return c + 32;
    if (c == 0x152) return 0x153;   /* OE ligature */
    if (c == 0x178) return 0xFF;    /* Y diaeresis */
    if (c == 0x3A9) return 0x3C9;   /* Omega */
    return c;
}

static int Trim(u32 *s, int n)
{
    int start = 0, i;
    while (n > 0 && IsSpace(s[n - 1])) n--;
    while (start < n && IsSpace(s[start])) start++;
    for (i = start; i < n; i++) s[i - start] = s[i];
    return n - start;
}

static char *PutNumber(char *p, long v)   /* v >= 0 */
{
    char tmp[24];
    int n = 0;
    do { tmp[n++] = (char)('0' + v % 10); v /= 10; } while (v);
    while (n) *p++ = tmp[--n];
    return p;
}

/* ---------- Tactile Systems: worldcon_unlock.py and friends ---------- */

static int ParseRegCode(const unsigned char *text, int len, long *out, const char **error)
{
    u32 s[MAXCHARS];
    int n = Trim(s, ToUnicode(text, len, s)), i;
    long v = 0;
    /* a long is 32 bits on the Mac; real codes are 8 digits */
    if (n == 0 || n > 9) { *error = n ? "That registration code is too long." :
                                       "The registration code should be digits only."; return 0; }
    for (i = 0; i < n; i++) {
        if (s[i] < '0' || s[i] > '9') { *error = "The registration code should be digits only."; return 0; }
        v = v * 10 + (long)(s[i] - '0');
    }
    *out = v;
    return 1;
}

/* digit pairs from the right become letters, and their character codes are summed */
static int TactileCode(long n, char first, char last, char *out, const char **error)
{
    char digits[24], letters[16], *p = out;
    int nd, nl = 0, i;
    long total = 0;

    if (n < 0) { *error = "That registration code is too small to be valid."; return 0; }
    nd = (int)(PutNumber(digits, n) - digits);
    for (i = nd - 1; i >= 1; i -= 2) {
        total += digits[i];                                  /* ord() of the digit */
        letters[nl++] = (char)('A' + (digits[i] - '0') + (digits[i - 1] - '0'));
    }
    *p++ = first;
    for (i = 0; i < nl && i < 2; i++) *p++ = letters[i];    /* letters[:2] */
    p = PutNumber(p, total);
    for (i = nl >= 2 ? nl - 2 : 0; i < nl; i++) *p++ = letters[i];   /* letters[-2:] */
    *p++ = last;
    *p = 0;
    return 1;
}

static int Tactile(const char *salt, const unsigned char *text, int len, char *out, const char **error)
{
    long reg;
    if (!ParseRegCode(text, len, &reg, error)) return 0;
    return TactileCode(reg - ((unsigned char)salt[0] * 100L + (unsigned char)salt[1]), 'X', 'X', out, error);
}

/* MineField II: the salt comes from the unlock code's own first and last letters */
static int MineField(const unsigned char *text, int len, char *out, const char **error)
{
    const char first = 'X', last = 'X';
    long reg, a = 90 - (first - 65), b = 90 - (last - 65);
    if (!ParseRegCode(text, len, &reg, error)) return 0;
    return TactileCode(reg - (a * 100 + b), first, last, out, error);
}

/* ---------- Newtopoly: newtopoly_unlock.py (MD2 with two quirks) ---------- */

static int Newtopoly(const unsigned char *text, int len, char *out, const char **error)
{
    static const char hex[] = "0123456789abcdef";
    u32 s[MAXCHARS];
    int msg[MAXCHARS + 32];
    int c[16], x[48], n, i, j, k, pad, l, t;

    n = AppendASCII(s, ToUnicode(text, len, s), "    ");
    if (n < 6) { *error = "The name must be at least 2 characters."; return 0; }
    s[0] ^= 'Q';
    s[1] ^= 'X';
    for (i = 0; i < n; i++) {
        if (s[i] > 255) { *error = "The name contains a character the Newton can't hash."; return 0; }
        msg[i] = (int)s[i];
    }

    pad = 16 - n % 16;                        /* MD2 padding */
    for (i = 0; i < pad; i++) msg[n++] = pad;

    l = 0;                                    /* checksum quirk: C[j] = S[c ^ L] */
    for (i = 0; i < n; i += 16)
        for (j = 0; j < 16; j++) { c[j] = kMD2S[msg[i + j] ^ l]; l = c[j]; }
    for (j = 0; j < 16; j++) msg[n++] = c[j];

    memset(x, 0, sizeof x);                   /* MD2 compression */
    for (i = 0; i < n; i += 16) {
        for (j = 0; j < 16; j++) { x[16 + j] = msg[i + j]; x[32 + j] = x[16 + j] ^ x[j]; }
        t = 0;
        for (j = 0; j < 18; j++) {
            for (k = 0; k < 48; k++) { t = x[k] ^ kMD2S[t]; x[k] = t; }
            t = (t + j) % 256;
        }
    }

    /* first 8 bytes as hex; quirk: high nibble is (b - 1) div 16, truncating toward zero */
    for (i = 0; i < 8; i++) {
        *out++ = hex[(x[i] - 1) / 16];
        *out++ = hex[x[i] % 16];
    }
    *out = 0;
    return 1;
}

/* ---------- NewtHack: newthack_unlock.py ---------- */

static int NewtHack(const unsigned char *text, int len, char *out, const char **error)
{
    static const char key[] = "NewtHack:ChrisDopp";
    u32 name[MAXCHARS], d;
    int n, i;
    char digits[9], *p = digits;

    n = ToUnicode(text, len, name);
    for (i = 0; i < n; i++) name[i] = ToLower(name[i]);
    n = Trim(name, n);
    if (n < 7) n = AppendASCII(name, n, key);
    if (n < 8) { *error = "NewtHack can't register a name that is exactly 7 characters long."; return 0; }

    /* even positions come from the app symbol, odd positions from the name */
    for (i = 0; i < 8; i++) {
        d = (i % 2 ? name[i] : (u32)(unsigned char)key[i]) % 10;
        digits[i] = (char)('0' + d);
    }
    while (p < digits + 7 && *p == '0') p++;   /* StringToNumber drops leading zeros */
    memcpy(out, p, (size_t)(digits + 8 - p));
    out[digits + 8 - p] = 0;
    return 1;
}

/* ---------- YAZI and Mahjongg (Scrawl): yazi_unlock.py, mahjongg_unlock.py ---------- */

static int Scrawl(const char *suffix, const unsigned char *text, int len, char *out)
{
    static u32 s[MAXCHARS * 4];
    u32 h = 0, g;
    int n, i;

    n = AppendASCII(s, ToUnicode(text, len, s), suffix);
    while (n < 20) { for (i = 0; i < n; i++) s[n + i] = s[i]; n *= 2; }

    for (i = 0; i < n; i++) {                  /* 29-bit PJW/ELF-style hash */
        h = ((h << 4) & 0x1FFFFFFFUL) + s[i];
        h &= 0x1FFFFFFFUL;
        g = h & 0x1E000000UL;
        if (g) h = (h ^ (g >> 22)) & 0x01FFFFFFUL;
    }
    for (i = 0; i < 27; i += 3) *out++ = (char)(((h >> i) & 0xF) + 'a');
    *out = 0;
    return 1;
}

/* ---------- DeepSpaceNewt: deepspacenewt_unlock.py ---------- */

static int DeepSpaceNewt(const unsigned char *text, int len, char *out, const char **error)
{
    static const char key[] = "DeepSpaceNewt";
    const int klen = sizeof key - 1;
    u32 name[MAXCHARS], a, b, v;
    int n, count, i = 0, j = 0, step;

    n = ToUnicode(text, len, name);
    if (n == 0) { *error = "Enter the owner name."; return 0; }
    count = (n + klen) % 7 + 7;                 /* loop runs 0..count inclusive */
    for (step = 0; step <= count; step++) {
        a = name[i] + (unsigned char)key[j];
        i = (i + 1) % n; j = (j + 1) % klen;
        b = name[i] + (unsigned char)key[j];
        i = (i + 1) % n; j = (j + 1) % klen;
        v = ((a & 85) | (b & 170)) % 36 + 48;
        if (v >= 58) v = v - 58 + 97;           /* past '9' -> 'a'..'z' */
        *out++ = (char)v;
    }
    *out = 0;
    return 1;
}

int ComputeUnlock(const Game *game, const unsigned char *text, int len,
                  char *out, int outSize, const char **error)
{
    if (outSize < 40) { *error = "Internal error."; return 0; }
    switch (game->scheme) {
    case kSchemeTactile:       return Tactile(game->param, text, len, out, error);
    case kSchemeMineField:     return MineField(text, len, out, error);
    case kSchemeNewtopoly:     return Newtopoly(text, len, out, error);
    case kSchemeNewtHack:      return NewtHack(text, len, out, error);
    case kSchemeScrawl:        return Scrawl(game->param, text, len, out);
    case kSchemeDeepSpaceNewt: return DeepSpaceNewt(text, len, out, error);
    case kSchemeFixed:         strcpy(out, game->param); return 1;
    }
    *error = "Unknown game.";
    return 0;
}
