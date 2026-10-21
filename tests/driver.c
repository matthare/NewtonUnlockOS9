/* Test driver: reads "gameIndex<TAB>hex-of-MacRoman-bytes" lines, prints the result or ERR. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../unlockers.h"

int main(void)
{
    char line[2048], out[64];
    unsigned char text[600];
    const char *err;
    while (fgets(line, sizeof line, stdin)) {
        int g = atoi(line), n = 0;
        char *h = strchr(line, '\t') + 1;
        while (h[0] && h[0] != '\n') { unsigned v; sscanf(h, "%2x", &v); text[n++] = (unsigned char)v; h += 2; }
        puts(ComputeUnlock(&gGames[g], text, n, out, sizeof out, &err) ? out : "ERR");
    }
    return 0;
}
