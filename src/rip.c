/* NetHack 5.0	rip.c	$NHDT-Date: 1597967808 2020/08/20 23:56:48 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.33 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2017. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Defining TEXT_TOMBSTONE causes genl_outrip() to exist, but it doesn't
   necessarily have to be used by a binary with multiple window-ports */

#if defined(TTY_GRAPHICS) || defined(X11_GRAPHICS) || defined(GEM_GRAPHICS) \
    || defined(DUMPLOG) || defined(CURSES_GRAPHICS) || defined(SHIM_GRAPHICS) \
    || defined(AMII_GRAPHICS)
#define TEXT_TOMBSTONE
#endif
#if defined(mac) || defined(__BEOS__)
#ifndef TEXT_TOMBSTONE
#define TEXT_TOMBSTONE
#endif
#endif

#ifdef TEXT_TOMBSTONE
staticfn int rip_utf8_decode(const char *, unsigned long *);
staticfn int rip_codepoint_width(unsigned long);
staticfn size_t rip_fit_bytes(const char *, int, int *);
staticfn void rip_copy_fit(char *, size_t, const char *, int);
staticfn void rip_next_line(const char **, char *, size_t, int);
staticfn void center(int, const char *);

#ifndef NH320_DEDICATION
/* A normal tombstone for end of game display. */
static const char *const rip_txt[] = {
    "                       ----------",
    "                      /          \\",
    "                     /    REST    \\",
    "                    /      IN      \\",
    "                   /     PEACE      \\",
    "                  /                  \\",
    "                  |                  |", /* Name of player */
    "                  |                  |", /* Amount of $ */
    "                  |                  |", /* Type of death */
    "                  |                  |", /* . */
    "                  |                  |", /* . */
    "                  |                  |", /* . */
    "                  |       1001       |", /* Real year of death */
    "                 *|     *  *  *      | *",
    "        _________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______", 0
};
#define STONE_LINE_CENT 28 /* char[] element of center of stone face */
#else                      /* NH320_DEDICATION */
/* NetHack 3.2.x displayed a dual tombstone as a tribute to Izchak. */
static const char *const rip_txt[] = {
    "              ----------                      ----------",
    "             /          \\                    /          \\",
    "            /    REST    \\                  /    This    \\",
    "           /      IN      \\                /  release of  \\",
    "          /     PEACE      \\              /   NetHack is   \\",
    "         /                  \\            /   dedicated to   \\",
    "         |                  |            |  the memory of   |",
    "         |                  |            |                  |",
    "         |                  |            |  Izchak Miller   |",
    "         |                  |            |   1935 - 1994    |",
    "         |                  |            |                  |",
    "         |                  |            |     Ascended     |",
    "         |       1001       |            |                  |",
    "      *  |     *  *  *      | *        * |      *  *  *     | *",
    (" _____)/\\|\\__//(\\/(/\\)/\\//\\/|_)___"
     "_____)/|\\\\_/_/(\\/(/\\)/\\/\\/|_)____"),
    0
};
#define STONE_LINE_CENT 19 /* char[] element of center of stone face */
#endif                     /* NH320_DEDICATION */
#define STONE_LINE_LEN  16 /* # chars that fit on one line
                            * (note 1 ' ' border)           */
#define NAME_LINE  6 /* *char[] line # for player name */
#define GOLD_LINE  7 /* *char[] line # for amount of gold */
#define DEATH_LINE 8 /* *char[] line # for death description */
#define YEAR_LINE 12 /* *char[] line # for year */

staticfn int
rip_utf8_decode(const char *s, unsigned long *ucp)
{
    const unsigned char *u = (const unsigned char *) s;
    unsigned long cp, mincp;
    int i, len;

    if (!u[0]) {
        *ucp = 0L;
        return 0;
    }
    if (u[0] < 0x80) {
        *ucp = (unsigned long) u[0];
        return 1;
    } else if ((u[0] & 0xE0) == 0xC0) {
        len = 2;
        cp = (unsigned long) (u[0] & 0x1F);
        mincp = 0x80L;
    } else if ((u[0] & 0xF0) == 0xE0) {
        len = 3;
        cp = (unsigned long) (u[0] & 0x0F);
        mincp = 0x800L;
    } else if ((u[0] & 0xF8) == 0xF0) {
        len = 4;
        cp = (unsigned long) (u[0] & 0x07);
        mincp = 0x10000L;
    } else {
        *ucp = (unsigned long) u[0];
        return 1;
    }

    for (i = 1; i < len; ++i) {
        if (!u[i] || (u[i] & 0xC0) != 0x80) {
            *ucp = (unsigned long) u[0];
            return 1;
        }
        cp = (cp << 6) | (unsigned long) (u[i] & 0x3F);
    }
    if (cp < mincp || (cp >= 0xD800L && cp <= 0xDFFFL)
        || cp > 0x10FFFFL) {
        *ucp = (unsigned long) u[0];
        return 1;
    }
    *ucp = cp;
    return len;
}

staticfn int
rip_codepoint_width(unsigned long cp)
{
    if (cp == 0L)
        return 0;
    if (cp < 0x20L || (cp >= 0x7FL && cp < 0xA0L))
        return 0;
    if ((cp >= 0x1100L && cp <= 0x115FL)
        || (cp >= 0x2E80L && cp <= 0xA4CFL)
        || (cp >= 0xAC00L && cp <= 0xD7A3L)
        || (cp >= 0xF900L && cp <= 0xFAFFL)
        || (cp >= 0xFE10L && cp <= 0xFE19L)
        || (cp >= 0xFE30L && cp <= 0xFE6FL)
        || (cp >= 0xFF00L && cp <= 0xFF60L)
        || (cp >= 0xFFE0L && cp <= 0xFFE6L)
        || (cp >= 0x1F300L && cp <= 0x1FAFFL))
        return 2;
    return 1;
}

staticfn size_t
rip_fit_bytes(const char *text, int maxwidth, int *widthp)
{
    const char *p = text;
    unsigned long cp = 0L;
    int width = 0, len, chwidth;

    while (*p) {
        len = rip_utf8_decode(p, &cp);
        if (len <= 0)
            break;
        chwidth = rip_codepoint_width(cp);
        if (width + chwidth > maxwidth)
            break;
        width += chwidth;
        p += len;
    }
    if (widthp)
        *widthp = width;
    return (size_t) (p - text);
}

staticfn void
rip_copy_fit(char *dst, size_t dstsz, const char *src, int maxwidth)
{
    size_t bytes;

    if (!dstsz)
        return;
    bytes = rip_fit_bytes(src, maxwidth, (int *) 0);
    if (bytes >= dstsz)
        bytes = dstsz - 1;
    (void) memcpy((genericptr_t) dst, (genericptr_t) src, bytes);
    dst[bytes] = '\0';
}

staticfn void
rip_next_line(const char **textp, char *out, size_t outsz, int maxwidth)
{
    const char *start, *p, *end, *next, *last_space, *last_space_next;
    unsigned long cp;
    int width, len, chwidth;
    size_t bytes;

    start = *textp;
    while (*start == ' ')
        ++start;
    p = start;
    end = next = p;
    last_space = last_space_next = (const char *) 0;
    width = 0;

    while (*p) {
        len = rip_utf8_decode(p, &cp);
        if (len <= 0)
            break;
        if (cp == '\n') {
            end = p;
            next = p + len;
            break;
        }
        chwidth = rip_codepoint_width(cp);
        if (width + chwidth > maxwidth)
            break;
        if (cp == ' ') {
            last_space = p;
            last_space_next = p + len;
        }
        width += chwidth;
        p += len;
        end = next = p;
    }

    if (*p && cp != '\n' && last_space && last_space > start) {
        end = last_space;
        next = last_space_next;
    } else if (p == start && *p) {
        len = rip_utf8_decode(p, &cp);
        if (len <= 0)
            len = 1;
        end = next = p + len;
    }

    bytes = (size_t) (end - start);
    if (!outsz) {
        *textp = next;
        return;
    }
    if (bytes >= outsz)
        bytes = outsz - 1;
    (void) memcpy((genericptr_t) out, (genericptr_t) start, bytes);
    out[bytes] = '\0';
    while (*next == ' ')
        ++next;
    *textp = next;
}

staticfn void
center(int line, const char *text)
{
    char *oldline, *newline, *op;
    const char *suffix;
    size_t prefix_len, text_bytes, suffix_len, new_len;
    int text_start, text_end, text_width, leftpad, rightpad;

    text_start = STONE_LINE_CENT - ((STONE_LINE_LEN + 1) >> 1);
    text_end = text_start + STONE_LINE_LEN;
    oldline = gr.rip[line];
    if ((int) strlen(oldline) < text_end)
        return;

    text_bytes = rip_fit_bytes(text, STONE_LINE_LEN, &text_width);
    leftpad = (STONE_LINE_LEN - text_width) / 2;
    rightpad = STONE_LINE_LEN - text_width - leftpad;

    prefix_len = (size_t) text_start;
    suffix = oldline + text_end;
    suffix_len = strlen(suffix);
    new_len = prefix_len + (size_t) leftpad + text_bytes
              + (size_t) rightpad + suffix_len;

    newline = (char *) alloc(new_len + 1);
    op = newline;
    (void) memcpy((genericptr_t) op, (genericptr_t) oldline, prefix_len);
    op += prefix_len;
    (void) memset((genericptr_t) op, ' ', (size_t) leftpad);
    op += leftpad;
    (void) memcpy((genericptr_t) op, (genericptr_t) text, text_bytes);
    op += text_bytes;
    (void) memset((genericptr_t) op, ' ', (size_t) rightpad);
    op += rightpad;
    (void) memcpy((genericptr_t) op, (genericptr_t) suffix, suffix_len + 1);

    free((genericptr_t) oldline);
    gr.rip[line] = newline;
}

void
genl_outrip(winid tmpwin, int how, time_t when)
{
    char **dp;
    const char *dpx;
    char buf[BUFSZ];
    int x;
    int line, year;
    long cash;

    gr.rip = dp = (char **) alloc(sizeof(rip_txt));
    for (x = 0; rip_txt[x]; ++x)
        dp[x] = dupstr(rip_txt[x]);
    dp[x] = (char *) 0;

    /* Put name on stone */
    rip_copy_fit(buf, sizeof buf, svp.plname, STONE_LINE_LEN);
    center(NAME_LINE, buf);

    /* Put $ on stone */
    cash = max(gd.done_money, 0L);
    /* arbitrary upper limit; practical upper limit is quite a bit less */
    if (cash > 999999999L)
        cash = 999999999L;
    Sprintf(buf, "%ld 金币", cash);
    center(GOLD_LINE, buf);

    /* Put together death description */
    formatkiller(buf, sizeof buf, how, FALSE);

    /* Put death type on stone */
    for (line = DEATH_LINE, dpx = buf; line < YEAR_LINE; line++) {
        char linebuf[BUFSZ];

        rip_next_line(&dpx, linebuf, sizeof linebuf, STONE_LINE_LEN);
        center(line, linebuf);
    }

    /* Put year on stone */
    year = (int) ((yyyymmdd(when) / 10000L) % 10000L);
    Sprintf(buf, "%4d", year);
    center(YEAR_LINE, buf);

#ifdef DUMPLOG
    if (tmpwin == 0)
        dump_forward_putstr(0, 0, "游戏结束:", TRUE);
    else
#endif
        putstr(tmpwin, 0, "");

    for (; *dp; dp++)
        putstr(tmpwin, 0, *dp);

    putstr(tmpwin, 0, "");
#ifdef DUMPLOG
    if (tmpwin != 0)
#endif
        putstr(tmpwin, 0, "");

    for (x = 0; rip_txt[x]; x++) {
        free((genericptr_t) gr.rip[x]);
    }
    free((genericptr_t) gr.rip);
    gr.rip = 0;
}

#endif /* TEXT_TOMBSTONE */

/*rip.c*/
