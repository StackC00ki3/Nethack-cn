/* NetHack 5.0  mdlib.c  $NHDT-Date: 1701499945 2023/12/02 06:52:25 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.51 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Kenneth Lorber, Kensington, Maryland, 2015. */
/* Copyright (c) M. Stephenson, 1990, 1991.                       */
/* Copyright (c) Dean Luick, 1990.                                */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This can be linked into a binary to provide the functionality
 * via the contained functions, or it can be #included directly
 * into util/makedefs.c to provide it there.
 */

#ifndef MAKEDEFS_C
#define MDLIB_C
#include "config.h"
#include "permonst.h"
#include "objclass.h"
#include "wintype.h"
#include "sym.h"
#include "artilist.h"
#include "dungeon.h"
#include "sndprocs.h"
#include "obj.h"
#include "monst.h"
#include "you.h"
#include "context.h"
#include "flag.h"
#include "dlb.h"
#include "hacklib.h"

/* version information */
#ifdef SHORT_FILENAMES
#include "patchlev.h"
#else
#include "patchlevel.h"
#endif
#define Fprintf (void) fprintf
#define Fclose (void) fclose
#define Unlink (void) unlink
#if !defined(AMIGA) || defined(AZTEC_C)
#define rewind(fp) fseek((fp), 0L, SEEK_SET) /* guarantee a return value */
#endif  /* AMIGA || AZTEC_C */
#else
#ifndef GLOBAL_H
#include "global.h"
#endif
#endif  /* !MAKEDEFS_C */

/* shorten up some lines */
#define FOR_RUNTIME

#if defined(MAKEDEFS_C) || defined(FOR_RUNTIME)
#include <stdarg.h>
/* REPRODUCIBLE_BUILD will change this to TRUE */
static boolean date_via_env = FALSE;

extern unsigned long md_ignored_features(void);
extern const char *datamodel(int);
char *version_id_string(char *, size_t, const char *) NONNULL NONNULLPTRS;
char *bannerc_string(char *, size_t, const char *) NONNULL NONNULLPTRS;
int case_insensitive_comp(const char *, const char *) NONNULLPTRS;

#ifndef SFCTOOL
staticfn
#endif /* SFCTOOL */
void make_version(void);

#ifndef HAS_NO_MKSTEMP
#ifdef _MSC_VER
static int mkstemp(char *);
#endif
#endif

#endif /* MAKEDEFS_C || FOR_RUNTIME */

#if !defined(MAKEDEFS_C) && defined(WIN32)
extern int GUILaunched;
#endif

/* these are in extern.h but we don't include hack.h */
/* XXX move to new file mdlib.h? */
extern void populate_nomakedefs(struct version_info *) NONNULLARG1; /*date.c*/
extern void free_nomakedefs(void); /* date.c */
void runtime_info_init(void);
const char *do_runtime_info(int *) NO_NNARGS;
void release_runtime_info(void);
char *mdlib_version_string(char *, const char *) NONNULL NONNULLPTRS;

staticfn void build_options(void);
staticfn int count_and_validate_winopts(void);
staticfn void opt_out_words(char *, int *) NONNULLPTRS;
staticfn void build_savebones_compat_string(void);

static int idxopttext, done_runtime_opt_init_once = 0;
#define MAXOPT 60 /* 5.0: currently 40 lines get inserted into opttext[] */
static char *opttext[MAXOPT] = { 0 };
#define STOREOPTTEXT(line) \
    ((void) ((idxopttext < MAXOPT)                      \
             ? (opttext[idxopttext++] = dupstr(line))   \
             : 0))
static char optbuf[COLBUFSZ];
static struct version_info version;
static const char opt_indent[] = "    ";

struct win_information {
    const char *id, /* windowtype value */
        *name;      /* description, often same as id */
    boolean valid;
};

static struct win_information window_opts[] = {
#ifdef TTY_GRAPHICS
    { "tty",
      /* testing TILES_IN_GLYPHMAP here would bring confusion because it could
         apply to another interface such as X11, so check MSDOS explicitly
         instead; even checking TTY_TILES_ESCCODES would probably be
         confusing to most users (and it will already be listed separately
         in the compiled options section so users aware of it can find it) */
#ifdef MSDOS
      "传统文本, 可选 'tiles' 图形",
#else
      /* assume that one or more of IBMgraphics, DECgraphics
         can be enabled; we can't tell from here whether that is accurate */
      "传统文本, 可选线条绘制",
#endif
      TRUE
    },
#endif /*TTY_GRAPHICS */
#ifdef CURSES_GRAPHICS
    { "curses", "基于终端的图形", TRUE },
#endif
#ifdef X11_GRAPHICS
    { "X11", "X11", TRUE },
#endif
#ifdef QT_GRAPHICS /* too vague; there are multiple incompatible versions */
    { "Qt", "Qt", TRUE },
#endif
#ifdef MSWIN_GRAPHICS /* win32 */
    { "mswin", "Windows 图形界面", TRUE },
#endif
#ifdef SHIM_GRAPHICS
    { "shim", "NetHack 库窗口系统兼容层", TRUE },
#endif

#if 0  /* remainder have been retired */
#ifdef GNOME_GRAPHICS /* unmaintained/defunct */
    { "Gnome", "Gnome", TRUE },
#endif
#ifdef MACOS9 /* defunct OS 9 interface */
    { "mac", "Mac", TRUE },
#endif
#ifdef AMIGA_INTUITION /* unmaintained/defunct */
    { "amii", "Amiga Intuition", TRUE },
#endif
#ifdef GEM_GRAPHICS /* defunct Atari interface */
    { "Gem", "Gem", TRUE },
#endif
#ifdef BEOS_GRAPHICS /* unmaintained/defunct */
    { "BeOS", "BeOS InterfaceKit", TRUE },
#endif
#endif  /* 0 => retired */
    { 0, 0, FALSE }
};

#if !defined(MAKEDEFS_C)
staticfn int count_and_validate_soundlibopts(void);

struct soundlib_information {
    enum soundlib_ids id;
    const char *const text_id;
    const char *const Url;
    boolean valid;
};

/*
 * soundlibs
 *
 * None of these are endorsements or recommendations of one library
 * or another, in any way. They are just preprocessor conditionals
 * in the event that glue code for such a library is ever added into
 * NetHack.
 */
static struct soundlib_information soundlib_opts[] = {
    { soundlib_nosound, "soundlib_nosound", "", FALSE },
#ifdef SND_LIB_PORTAUDIO
    { soundlib_portaudio, "soundlib_portaudio",
        "http://www.portaudio.com/", FALSE },
#endif
#ifdef SND_LIB_OPENAL
    { soundlib_openal, "soundlib_openal",
        "https://www.openal.org/", FALSE },
#endif
#ifdef SND_LIB_SDL_MIXER
    { soundlib_sdl_mixer, "soundlib_sdl_mixer",
        "https://github.com/libsdl-org/SDL_mixer/", FALSE },
#endif
#ifdef SND_LIB_MINIAUDIO
    { soundlib_miniaudio, "soundlib_miniaudio",
        "https://miniaud.io/", FALSE },
#endif
#ifdef SND_LIB_FMOD
    /* proprietary, though a Free Indie License exists.
     * https://www.fmod.com/licensing#indie-note
     */
    { soundlib_fmod, "soundlib_fmod",
        "https://www.fmod.com/", FALSE },
#endif
#ifdef SND_LIB_SOUND_ESCCODES
    { soundlib_sound_esccodes, "sound_esccodes", "", FALSE },
#endif
#ifdef SND_LIB_VISSOUND
    { soundlib_vissound, "soundlib_vissound", "", FALSE },
#endif
#ifdef SND_LIB_WINDSOUND
    /* Uses Windows WIN32 API */
    { soundlib_windsound, "soundlib_windsound",
  "https://learn.microsoft.com/en-us/windows/win32/multimedia/waveform-audio",
        FALSE },
#endif
#ifdef SND_LIB_MACSOUND
    /* Uses AppKit NSSound */
    { soundlib_macsound, "soundlib_macsound",
        "https://developer.apple.com/documentation/appkit/nssound",
        FALSE },
#endif
#ifdef SND_LIB_QTSOUND
    { soundlib_qtsound, "soundlib_qtsound",
        "https://doc.qt.io/qt-5/qsoundeffect.html", FALSE },
#endif
    { 0, 0, 0, FALSE },
};
#endif  /* !MAKEDEFS_C */

unsigned long
md_ignored_features(void)
{
    return (0UL
            | (1UL << 19) /* SCORE_ON_BOTL */
            | SFCTOOL_BIT /* stored by SFCTOOL, not NetHack itself */
            );
}

#ifndef SFCTOOL
staticfn
#endif
void
make_version(void)
{
    int i;

    /*
     * integer version number
     */
    version.incarnation = ((unsigned long) VERSION_MAJOR << 24)
                          | ((unsigned long) VERSION_MINOR << 16)
                          | ((unsigned long) PATCHLEVEL << 8)
                          | ((unsigned long) EDITLEVEL);
    /*
     * encoded feature list
     * Note:  if any of these magic numbers are changed or reassigned,
     * EDITLEVEL in patchlevel.h should be incremented at the same time.
     * The actual values have no special meaning, and the category
     * groupings are just for convenience.
     */
    version.feature_set = (unsigned long) (0L
/* levels and/or topology (0..4) */
/* monsters (5..9) */
#ifdef MAIL_STRUCTURES
                                           | (1L << 6)
#endif
/* objects (10..14) */
/* flag bits and/or other global variables (15..26) */
 /* color support always*/                 | (1L << 17)
#ifdef INSURANCE
                                           | (1L << 18)
#endif
#ifdef SCORE_ON_BOTL
                                           | (1L << 19)
#endif
                                               );
    /*
     * Value used for object & monster sanity check.
     *    (NROFARTIFACTS<<24) | (NUM_OBJECTS<<12) | (NUMMONS<<0)
     */
    for (i = 1; artifact_names[i]; i++)
        continue;
    version.entity_count = (unsigned long) (i - 1);
    i = NUM_OBJECTS;
    version.entity_count = (version.entity_count << 12) | (unsigned long) i;
    i = NUMMONS;
    version.entity_count = (version.entity_count << 12) | (unsigned long) i;
/* free bits in here */
    return;
}

#if defined(MAKEDEFS_C) || defined(FOR_RUNTIME)

char *
mdlib_version_string(char *outbuf, const char *delim)
{
    Sprintf(outbuf, "%d%s%d%s%d", VERSION_MAJOR, delim, VERSION_MINOR, delim,
            PATCHLEVEL);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
    Sprintf(eos(outbuf), "-%d", EDITLEVEL);
#endif
    return outbuf;
}

#define Snprintf(str, size, ...) \
    nh_snprintf(__func__, __LINE__, str, size, __VA_ARGS__)
extern void nh_snprintf(const char *func, int line, char *str, size_t size,
                        const char *fmt, ...);

char *
version_id_string(char *outbuf, size_t bufsz, const char *build_date)
{
    char subbuf[64], versbuf[64];
    char statusbuf[64];

#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#if (NH_DEVEL_STATUS == NH_STATUS_BETA)
    Strcpy(statusbuf, " 测试版");
#else
#if (NH_DEVEL_STATUS == NH_STATUS_WIP)
    Strcpy(statusbuf, " 开发中");
#else
    Strcpy(statusbuf, " 发布后");
#endif
#endif
#else
    statusbuf[0] = '\0';
#endif
    subbuf[0] = '\0';
#ifdef PORT_SUB_ID
    subbuf[0] = ' ';
    Strcpy(&subbuf[1], PORT_SUB_ID);
#endif

    Snprintf(outbuf, bufsz, "%s NetHack%s 版本 %s%s - 最后%s %s.",
             PORT_ID, subbuf, mdlib_version_string(versbuf, "."), statusbuf,
             date_via_env ? "修订" : "构建", build_date);
    return outbuf;
}

/* still within #if MAKDEFS_C || FOR_RUNTIME */

char *
bannerc_string(char *outbuf, size_t bufsz, const char *build_date)
{
    char subbuf[64], versbuf[64];

    subbuf[0] = '\0';
#ifdef PORT_SUB_ID
    subbuf[0] = ' ';
    Strcpy(&subbuf[1], PORT_SUB_ID);
#endif
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#if (NH_DEVEL_STATUS == NH_STATUS_BETA)
    Strcat(subbuf, " 测试版");
#else
    Strcat(subbuf, " 开发中");
#endif
#endif

    Snprintf(outbuf, bufsz, "         版本 %s %s%s, %s于 %s.",
            mdlib_version_string(versbuf, "."), PORT_ID, subbuf,
            date_via_env ? "修订" : "构建", build_date);
    return outbuf;
}

#ifndef HAS_NO_MKSTEMP
#ifdef _MSC_VER
int
mkstemp(char *template)
{
    int err;

    err = _mktemp_s(template, strlen(template) + 1);
    if( err != 0 )
        return -1;
    return _open(template,
                 _O_RDWR | _O_BINARY | _O_TEMPORARY | _O_CREAT,
                 _S_IREAD | _S_IWRITE);
}
#endif /* _MSC_VER */
#endif /* HAS_NO_MKSTEMP */
#endif /* MAKEDEFS_C || FOR_RUNTIME */

static char save_bones_compat_buf[BUFSZ];

staticfn void
build_savebones_compat_string(void)
{
#ifdef VERSION_COMPATIBILITY
    unsigned long uver = VERSION_COMPATIBILITY,
                  cver  = (((unsigned long) VERSION_MAJOR << 24)
                         | ((unsigned long) VERSION_MINOR << 16)
                         | ((unsigned long) PATCHLEVEL    <<  8));
#endif

    Strcpy(save_bones_compat_buf,
           "可接受的存档和骨骸文件版本:");
#ifdef VERSION_COMPATIBILITY
    if (uver != cver)
        Sprintf(eos(save_bones_compat_buf), " %lu.%lu.%lu 至 %d.%d.%d",
                ((uver >> 24) & 0x0ffUL),
                ((uver >> 16) & 0x0ffUL),
                ((uver >>  8) & 0x0ffUL),
                VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
    else
#endif
        Sprintf(eos(save_bones_compat_buf), " 仅 %d.%d.%d",
                VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
}

static const char *const build_opts[] = {
#ifdef AMIGA_WBENCH
    "Amiga WorkBench 支持",
#endif
#ifdef ANSI_DEFAULT
    "ANSI 默认终端",
#endif
    "颜色",
#ifdef TTY_GRAPHICS
#ifdef TTY_TILES_ESCCODES
    "用于图块提示的控制台转义码",
#endif
#endif
#ifdef LIFE
    "康威生命游戏",
#endif
#ifdef COMPRESS
    "数据文件压缩",
#endif
#ifdef ZLIB_COMP
    "ZLIB 数据文件压缩",
#endif
#ifdef DLB
#ifndef VERSION_IN_DLB_FILENAME
    "数据库归档器",
#else
    "带版本相关名称的数据归档器",
#endif
#endif
#ifdef EDIT_GETLIN
    "编辑 getlin - 部分提示会记住上次响应",
#endif
#ifdef DUMPLOG
    "游戏结束转储日志",
#endif
#ifdef HOLD_LOCKFILE_OPEN
    "对第 0 层文件独占加锁",
#endif
#if defined(HANGUPHANDLING) && !defined(NO_SIGNAL)
#ifdef SAFERHANGUP
    "延迟处理挂起信号",
#else
    "立即处理挂起信号",
#endif
#endif
#ifdef INSURANCE
    "用于崩溃恢复的保险文件",
#endif
#ifdef LIVELOG
    "实时日志支持",
#endif
#ifdef LOGFILE
    "日志文件",
#endif
#ifdef XLOGFILE
    "扩展日志文件",
#endif
#ifdef PANICLOG
    "错误和警告日志文件",
#endif
#ifdef MAIL
    "邮件守护进程",
#endif
#ifdef MONITOR_HEAP
    "监控堆 - 记录内存使用以便后续分析",
#endif
#if defined(GNUDOS) || defined(__DJGPP__)
    "MSDOS 保护模式",
#endif
#ifdef NEWS
    "新闻文件",
#endif
#ifdef OVERLAY
#ifdef MOVERLAY
    "MOVE 覆盖层",
#else
#ifdef VROOMM
    "VROOMM 覆盖层",
#else
    "覆盖层",
#endif
#endif
#endif
#ifdef UNIX
#if defined(DEF_PAGER) && !defined(DLB)
    "用于查看帮助文件的外部分页器",
#else
    "用于查看帮助文件的内部分页器",
#endif
#endif /* UNIX */
    /* pattern matching method will be substituted by nethack at run time */
    "通过 :PATMATCH: 进行模式匹配",
#ifdef USE_ISAAC64
    "由 ISAAC64 生成伪随机数",
#ifdef DEV_RANDOM
    /* include which specific one */
    "强 PRNG 种子来自 " DEV_RANDOM,
#else
#ifdef WIN32
    "强 PRNG 种子来自 CNG BCryptGenRandom()",
#endif
#endif  /* DEV_RANDOM */
#else   /* ISAAC64 */
#ifdef RANDOM
    "由 random() 生成伪随机数",
#else
    "由 C rand() 生成伪随机数",
#endif
#endif /* ISAAC64 */
#ifdef SELECTSAVED
    "通过菜单恢复已保存游戏",
#endif
#ifdef SCORE_ON_BOTL
    "状态行显示分数",
#endif
#ifdef CLIPPING
    "屏幕裁剪",
#endif
#ifdef NO_TERMS
#ifdef MACOS9
    "通过 mactty 控制屏幕",
#endif
#ifdef SCREEN_BIOS
    "通过 BIOS 控制屏幕",
#endif
#ifdef SCREEN_DJGPPFAST
    "通过 DJGPP fast 控制屏幕",
#endif
#ifdef SCREEN_VGA
    "通过 VGA 图形控制屏幕",
#endif
#ifdef WIN32CON
    "通过 WIN32 控制台 I/O 控制屏幕",
#endif
#endif /* NO_TERMS */
#ifdef SHELL
    "shell 命令",
#endif
    "传统状态显示",
#ifdef STATUS_HILITES
    "通过窗口端口显示带高亮的状态",
#else
    "通过窗口端口显示无高亮的状态",
#endif
#ifdef SUSPEND
    "挂起命令",
#endif
#ifdef TTY_GRAPHICS
#ifdef TERMINFO
    "终端信息库",
#else
#if defined(TERMLIB) || (!defined(MICRO) && !defined(WIN32))
    "终端能力库",
#endif
#endif
#endif /*TTY_GRAPHICS*/
#ifdef USE_XPM
    "XPM 格式的图块文件",
#endif
#ifdef GRAPHIC_TOMBSTONE
    "图形化 RIP 画面",
#endif
#ifdef TIMED_DELAY
    "显示效果定时等待",
#endif
#ifdef PREFIXES_IN_USE
    "可变游戏场地",
#endif
#ifdef VISION_TABLES
    "视野表",
#endif
#ifdef SYSCF
    "运行时系统配置",
#endif
#ifdef PANICTRACE
    "出错时显示堆栈跟踪",
#endif
#ifdef CRASHREPORT
    "启动浏览器报告问题",
#endif
    save_bones_compat_buf,
    "以及基本 NetHack 功能"
};

staticfn int
count_and_validate_winopts(void)
{
    int i, cnt = 0;

    /* window_opts has a fencepost entry at the end */
    for (i = 0; i < SIZE(window_opts) - 1; i++) {
#if !defined(MAKEDEFS_C) && defined(FOR_RUNTIME)
#ifdef WIN32
        window_opts[i].valid = FALSE;
        if ((GUILaunched
             && case_insensitive_comp(window_opts[i].id, "curses") != 0
             && case_insensitive_comp(window_opts[i].id, "mswin") != 0)
            || (!GUILaunched
                && case_insensitive_comp(window_opts[i].id, "mswin") == 0))
            continue;
#endif
#endif /* !MAKEDEFS_C && FOR_RUNTIME */
        ++cnt;
        window_opts[i].valid = TRUE;
    }
    return cnt;
}

#if !defined(MAKEDEFS_C)
staticfn int
count_and_validate_soundlibopts(void)
{
    int i, cnt = 0;

    /* soundlib_opts has a fencepost entry at the end */
    for (i = 0; i < SIZE(soundlib_opts) - 1; i++) {
        ++cnt;
        soundlib_opts[i].valid = TRUE;
    }
    return cnt;
}
#endif

staticfn void
opt_out_words(
    char *str,     /* input, but modified during processing */
    int *length_p) /* in/out */
{
    char *word;

    while (*str) {
        word = strchr(str, ' ');
#if 0
        /* treat " (" as unbreakable space */
        if (word && *(word + 1) == '(')
            word = strchr(word + 1,  ' ');
#endif
        if (word)
            *word = '\0';
        if (*length_p + (int) strlen(str) > COLNO - 5) {
            STOREOPTTEXT(optbuf);
            Sprintf(optbuf, "%s", opt_indent),
                *length_p = (int) strlen(opt_indent);
        } else {
            Sprintf(eos(optbuf), " "), (*length_p)++;
        }
        Sprintf(eos(optbuf), "%s", str), *length_p += (int) strlen(str);
        str += strlen(str) + (word ? 1 : 0);
    }
}

staticfn void
build_options(void)
{
    char buf[COLBUFSZ];
    int i, length, winsyscnt, cnt = 0;
    const char *defwinsys = DEFAULT_WINDOW_SYS;
#if !defined(MAKEDEFS_C) && defined(FOR_RUNTIME)
    int soundlibcnt;

#ifdef WIN32
    defwinsys = GUILaunched ? "mswin" : "tty";
#endif
#endif
    build_savebones_compat_string();
    STOREOPTTEXT(optbuf);
#if (NH_DEVEL_STATUS != NH_STATUS_RELEASED)
#if (NH_DEVEL_STATUS == NH_STATUS_BETA)
#define STATUS_ARG " [测试版]"
#else
#define STATUS_ARG " [开发中]"
#endif
#else
#define STATUS_ARG ""
#endif /* NH_DEVEL_STATUS == NH_STATUS_RELEASED */
    Sprintf(optbuf, "%sNetHack 版本 %d.%d.%d%s\n",
            opt_indent, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, STATUS_ARG);
    STOREOPTTEXT(optbuf);
    Sprintf(optbuf, "此版本编译进的选项:");
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    length = COLNO + 1; /* force 1st item onto new line */
    Strcat(strcpy(buf, datamodel(0)), " 数据模型,");
    opt_out_words(buf, &length);
    for (i = 0; i < SIZE(build_opts); i++) {
#if !defined(MAKEDEFS_C) && defined(FOR_RUNTIME)
#ifdef WIN32
        /* ignore the console entry if GUI version */
        if (GUILaunched
            && !strcmp("通过 WIN32 控制台 I/O 控制屏幕", build_opts[i]))
            continue;
#endif
#endif /* !MAKEDEFS_C && FOR_RUNTIME */
        Strcat(strcpy(buf, build_opts[i]),
               (i < SIZE(build_opts) - 1) ? "," : ".");
        opt_out_words(buf, &length);
    }
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    winsyscnt = count_and_validate_winopts();
    STOREOPTTEXT(optbuf);
    Sprintf(optbuf, "支持的窗口系统:");
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    length = COLNO + 1; /* force 1st item onto new line */

    for (i = 0; i < SIZE(window_opts) - 1; i++) {
        if (!window_opts[i].valid)
            continue;
        Sprintf(buf, "\"%s\"", window_opts[i].id);
        if (strcmp(window_opts[i].name, window_opts[i].id))
            Sprintf(eos(buf), " (%s)", window_opts[i].name);
        /*
         * 1 : foo.
         * 2 : foo and bar,
         * 3+: for, bar, and quux,
         *
         * 2+ will be followed by " with a default of..."
         */
        Strcat(buf, (winsyscnt == 1) ? "." /* no 'default' */
                    : (winsyscnt == 2 && cnt == 0) ? " 和"
                      : (cnt == winsyscnt - 2) ? ", 和"
                        : ",");
        opt_out_words(buf, &length);
        cnt++;
    }
    if (cnt > 1) {
        /* loop ended with a comma; opt_out_words() will insert a space */
        Sprintf(buf, "默认使用 \"%s\".", defwinsys);
        opt_out_words(buf, &length);
    }

#if !defined(MAKEDEFS_C)
    cnt = 0;
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    soundlibcnt = count_and_validate_soundlibopts();
    STOREOPTTEXT(optbuf);
    Sprintf(optbuf, "支持的音频库:");
    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';
    length = COLNO + 1; /* force 1st item onto new line */

#ifdef USER_SOUNDS
    soundlibcnt += 1;
#endif
    for (i = 0; i < SIZE(soundlib_opts) - 1; i++) {
        const char *soundlib;

        if (!soundlib_opts[i].valid)
            continue;
        soundlib = soundlib_opts[i].text_id;
        if (!strncmp(soundlib, "soundlib_", 9))
            soundlib += 9;
        Sprintf(buf, "\"%s\"", soundlib);
        /*
         * 1 : foo.
         * 2 : foo and bar.
         * 3+: for, bar, and quux.
         */
        Strcat(buf, (soundlibcnt == 1 || cnt == soundlibcnt - 1)
                    ? "." /* no 'with default' */
                    : (soundlibcnt == 2 && cnt == 0) ? " 和"
                      : (cnt == soundlibcnt - 2) ? ", 和"
                        : ",");
        opt_out_words(buf, &length);
        cnt++;
    }
#ifdef USER_SOUNDS
    if (cnt > 1) {
        /* loop ended with a comma; opt_out_words() will insert a space */
        Sprintf(buf, "用户音效.");
        opt_out_words(buf, &length);
    }
#endif
#endif  /* !MAKEDEFS_C */

    STOREOPTTEXT(optbuf);
    optbuf[0] = '\0';

#if defined(MAKEDEFS_C) || defined(FOR_RUNTIME)
    {
        static const char *const lua_info[] = {
 "", "NetHack 5.0.* 使用 'Lua' 解释器处理部分数据:", "",
 "    :LUACOPYRIGHT:", "",
 /*        1         2         3         4         5         6         7
  1234567890123456789012345678901234567890123456789012345678901234567890123456
  */
 ("    \"Permission is hereby granted, free of charge,"
  " to any person obtaining"),
 "     a copy of this software and associated documentation files (the ",
 "     \"Software\"), to deal in the Software without restriction including",
 "     without limitation the rights to use, copy, modify, merge, publish,",
 "     distribute, sublicense, and/or sell copies of the Software, and to ",
 "     permit persons to whom the Software is furnished to do so, subject to",
 "     the following conditions:",
 "     The above copyright notice and this permission notice shall be",
 "     included in all copies or substantial portions of the Software.\"",
            (const char *) 0
        };

        /* add lua copyright notice;
           ":TAG:" substitutions are deferred to caller */
        for (i = 0; lua_info[i]; ++i) {
            STOREOPTTEXT(lua_info[i]);
        }
    }
#endif /* MAKEDEFS_C || FOR_RUNTIME */

    /* end with a blank line */
    STOREOPTTEXT("");
    return;
}

#undef STOREOPTTEXT

void
runtime_info_init(void)
{
    if (!done_runtime_opt_init_once) {
        done_runtime_opt_init_once = 1;
        build_savebones_compat_string();
        /* construct the current version number */
        make_version();
        populate_nomakedefs(&version);          /* date.c */
        idxopttext = 0;
        build_options();
    }
}

const char *
do_runtime_info(int *rtcontext)
{
    const char *retval = (const char *) 0;

    if (!done_runtime_opt_init_once)
        runtime_info_init();
    if (idxopttext && rtcontext)
        if (*rtcontext >= 0 && *rtcontext < MAXOPT) {
            retval = opttext[*rtcontext];
            *rtcontext += 1;
        }
    return retval;
}

void
release_runtime_info(void)
{
    while (idxopttext > 0) {
        --idxopttext;
        free((genericptr_t) opttext[idxopttext]), opttext[idxopttext] = 0;
    }
    done_runtime_opt_init_once = 0;
    free_nomakedefs();
}

/*mdlib.c*/
