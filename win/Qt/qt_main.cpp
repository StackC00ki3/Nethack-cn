// Copyright (c) Warwick Allison, 1999.
// Qt4 conversion copyright (c) Ray Chason, 2012-2014.
// NetHack may be freely redistributed.  See license for details.

// qt_main.cpp -- the main window

extern "C" {
#include "hack.h"
#define CTRL(c) (0x1f & (c)) // substitute for C() from global.h, for ^V hack
}

#include "qt_pre.h"
#include <QtGui/QtGui>
#if QT_VERSION >= 0x060000
#include <QtGui/QShortcut>
#elif QT_VERSION >= 0x050000
#include <QtWidgets/QShortcut>
#endif

#if QT_VERSION >= 0x050000
#include <QtWidgets/QtWidgets>
#endif
#include "qt_post.h"
#include "qt_main.h"
#include "qt_main.moc"
#include "qt_bind.h"
#include "qt_glyph.h"
#include "qt_inv.h"
#include "qt_key.h"
#include "qt_map.h"
#include "qt_msg.h"
#include "qt_set.h"
#include "qt_stat.h"
#include "qt_str.h"

#ifndef KDE
#include "qt_kde0.moc"
#endif

// temporary
extern char *qt_tilewidth;
extern char *qt_tileheight;
extern int qt_compact_mode;
// end temporary

namespace nethack_qt_ {

// temporary
void centerOnMain( QWidget* w );
// end temporary

/* XPM */
static const char * nh_icon[] = {
"40 40 6 1",
" 	s None c none",
".	c #ffffff",
"X	c #dadab6",
"o	c #6c91b6",
"O	c #476c6c",
"+	c #000000",
"                                        ",
"                                        ",
"                                        ",
"        .      .X..XX.XX      X         ",
"        ..   .....X.XXXXXX   XX         ",
"        ... ....X..XX.XXXXX XXX         ",
"   ..   ..........X.XXXXXXXXXXX   XX    ",
"   .... ........X..XX.XXXXXXXXX XXXX    ",
"   .... ..........X.XXXXXXXXXXX XXXX    ",
"   ooOOO..ooooooOooOOoOOOOOOOXX+++OO++  ",
"   ooOOO..ooooooooOoOOOOOOOOOXX+++OO++  ",
"   ....O..ooooooOooOOoOOOOOOOXX+XXXX++  ",
"   ....O..ooooooooOoOOOOOOOOOXX+XXXX++  ",
"   ..OOO..ooooooOooOOoOOOOOOOXX+++XX++  ",
"    ++++..ooooooooOoOOOOOOOOOXX+++ +++  ",
"     +++..ooooooOooOOoOOOOOOOXX+++  +   ",
"      ++..ooooooooOoOOOOOOOOOXX+++      ",
"        ..ooooooOooOOoOOOOOOOXX+++      ",
"        ..ooooooooOoOOOOOOOOOXX+++      ",
"        ..ooooooOooOOoOOOOOOOXX+++      ",
"        ..ooooooooOoOOOOOOOOOXX+++      ",
"         ..oooooOooOOoOOOOOOXX+++       ",
"         ..oooooooOoOOOOOOOOXX+++       ",
"          ..ooooOooOOoOOOOOXX+++        ",
"          ..ooooooOoOOOOOOOXX++++       ",
"        ..o..oooOooOOoOOOOXX+XX+++      ",
"       ...o..oooooOoOOOOOXX++XXX++      ",
"      ....OO..ooOooOOoOOXX+++XXXX++     ",
"     ...oo..+..oooOoOOOXX++XXooXXX++    ",
"    ...ooo..++..OooOOoXX+++XXooOXXX+    ",
"   ..oooOOXX+++....XXXX++++XXOOoOOXX+   ",
"   ..oooOOXX+++ ...XXX+++++XXOOooOXX++  ",
"   ..oooOXXX+++  ..XX+++  +XXOOooOXX++  ",
"   .....XXX++++             XXXXXXX++   ",
"    ....XX++++              XXXXXXX+    ",
"     ...XX+++                XXXXX++    ",
"                                        ",
"                                        ",
"                                        ",
"                                        "};
/* XPM */
static const char * nh_icon_small[] = {
/* width height ncolors chars_per_pixel */
"16 16 16 1",
/* colors */
"  c #587070",
". c #D1D5C9",
"X c #8B8C84",
"o c #2A2A28",
"O c #9AABA9",
"+ c #6A8FB2",
"@ c #C4CAC4",
"# c #B6BEB6",
"$ c None",
"% c #54564E",
"& c #476C6C",
"* c #ADB2AB",
"= c #ABABA2",
"- c #5E8295",
"; c #8B988F",
": c #E8EAE7",
/* pixels */
"$$$$$$$$$$$$$$$$",
"$$$.$#::.#==*$$$",
"$.*:::::....#*=$",
"$@#:..@#*==#;XX;",
"$@O:+++- &&; X%X",
"$#%.+++- &&;% oX",
"$$o.++-- &&;%%X$",
"$$$:++-- &&;%%$$",
"$$$.O++- &&=o $$",
"$$$=:++- & XoX$$",
"$$*:@O--  ;%Xo$$",
"$*:O#$+--;oOOX $",
"$:+ =o::=oo=-;%X",
"$::.%o$*;X;##@%$",
"$$@# ;$$$$$=*;X$",
"$$$$$$$$$$$$$$$$"
};

#if 0 // RLC
/* XPM */
static const char * map_xpm[] = {
"12 13 4 1",
".	c None",
" 	c #000000000000",
"X	c #0000B6DAFFFF",
"o	c #69A69248B6DA",
"           .",
" XXXXX ooo  ",
" XoooX o    ",
" XoooX o o  ",
" XoooX ooo  ",
" XXoXX o    ",
"  oooooXXX  ",
" oo o oooX  ",
"    o XooX  ",
" oooo XooX  ",
" o  o XXXX  ",
"            ",
".           "};
/* XPM */
static const char * msg_xpm[] = {
"12 13 4 1",
".	c None",
" 	c #FFFFFFFFFFFF",
"X	c #69A69248B6DA",
"o	c #000000000000",
"           .",
" XXX XXX X o",
"           o",
" XXXXX XX  o",
"           o",
" XX XXXXX  o",
"           o",
" XXXXXX    o",
"           o",
" XX XXX XX o",
"           o",
"           o",
".ooooooooooo"};
/* XPM */
static const char * stat_xpm[] = {
"12 13 5 1",
"  c None",
".	c #FFFF00000000",
"X	c #000000000000",
"o	c #FFFFFFFF0000",
"O	c #69A6FFFF0000",
"            ",
"            ",
"...         ",
"...X        ",
"...X    ... ",
"oooX    oooX",
"oooXooo oooX",
"OOOXOOOXOOOX",
"OOOXOOOXOOOX",
"OOOXOOOXOOOX",
"OOOXOOOXOOOX",
"OOOXOOOXOOOX",
" XXXXXXXXXXX"};
#endif
/* XPM */
static const char * info_xpm[] = {
"12 13 4 1",
"  c None",
".	c #00000000FFFF",
"X	c #FFFFFFFFFFFF",
"o	c #000000000000",
"    ...     ",
"  .......   ",
" ...XXX...  ",
" .........o ",
"...XXXX.... ",
"....XXX....o",
"....XXX....o",
"....XXX....o",
" ...XXX...oo",
" ..XXXXX..o ",
"  .......oo ",
"   o...ooo  ",
"     ooo    "};


/* XPM */
static const char * again_xpm[] = {
"12 13 2 1",
" 	c None",
".	c #000000000000",
"    ..      ",
"     ..     ",
"   .....    ",
" .......    ",
"...  ..  .. ",
"..  ..   .. ",
"..        ..",
"..        ..",
"..        ..",
" ..      .. ",
" .......... ",
"   ......   ",
"            "};
/* XPM */
static const char * kick_xpm[] = {
"12 13 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFF6DB60000",
"            ",
"            ",
"   .  .  .  ",
"  ...  .  . ",
"   ...  .   ",
"    ...  .  ",
"     ...    ",
"XXX   ...   ",
"XXX.  ...   ",
"XXX. ...    ",
"XXX. ..     ",
" ...        ",
"            "};
/* XPM */
static const char * throw_xpm[] = {
"12 13 3 1",
" 	c None",
".	c #FFFF6DB60000",
"X	c #000000000000",
"            ",
"            ",
"            ",
"            ",
"....     X  ",
"....X     X ",
"....X XXXXXX",
"....X     X ",
" XXXX    X  ",
"            ",
"            ",
"            ",
"            "};
/* XPM */
static const char * fire_xpm[] = {
"12 13 5 1",
" 	c None",
".	c #B6DA45140000",
"X	c #FFFFB6DA9658",
"o	c #000000000000",
"O	c #FFFF6DB60000",
" .          ",
" X.         ",
" X .        ",
" X .o       ",
" X  .    o  ",
" X  .o    o ",
"OOOOOOOOoooo",
" X  .o    o ",
" X . o   o  ",
" X .o       ",
" X. o       ",
" . o        ",
"  o         "};
/* XPM */
static const char * pickup_xpm[] = {
"12 13 3 1",
" 	c None",
".	c #000000000000",
"X	c #FFFF6DB60000",
"            ",
"     .      ",
"    ...     ",
"   . . .    ",
"     .      ",
"     .      ",
"            ",
"   XXXXX    ",
"   XXXXX.   ",
"   XXXXX.   ",
"   XXXXX.   ",
"    .....   ",
"            "};
/* XPM */
static const char * drop_xpm[] = {
"12 13 3 1",
" 	c None",
".	c #FFFF6DB60000",
"X	c #000000000000",
"            ",
"   .....    ",
"   .....X   ",
"   .....X   ",
"   .....X   ",
"    XXXXX   ",
"            ",
"      X     ",
"      X     ",
"    X X X   ",
"     XXX    ",
"      X     ",
"            "};
/* XPM */
static const char * eat_xpm[] = {
"12 13 4 1",
" 	c None",
".	c #000000000000",
"X	c #FFFFB6DA9658",
"o	c #FFFF6DB60000",
"  .X.  ..   ",
"  .X.  ..   ",
"  .X.  ..   ",
"  .X.  ..   ",
"  ...  ..   ",
"   ..  ..   ",
"   ..  ..   ",
"   oo  oo   ",
"   oo  oo   ",
"   oo  oo   ",
"   oo  oo   ",
"   oo  oo   ",
"   oo  oo   "};
/* XPM */
static const char * search_xpm[] = {
"12 13 3 1",
" 	c None",
".	c #FFFFFFFF0000",
"X	c #7F0000000000",
"            ",
"    XXXXX   ",
"   X ... X  ",
"   X.....X  ",
"   X.....X  ",
"   X ... X  ",
"    XXXXX   ",
"      X     ",
"      X     ",
"      X     ",
"      X     ",
"      X     ",
"            "};
/* XPM */
static const char * rest_xpm[] = {
"12 13 2 1",
" 	c None",
".	c #000000000000",
"  .....     ",
"     .      ",
"    .       ",
"   .    ....",
"  .....   . ",
"         .  ",
"        ....",
"            ",
"     ....   ",
"       .    ",
"      .     ",
"     ....   ",
"            "};
/* XPM */
static const char * cast_a_xpm[] UNUSED = {
"12 13 3 1",
" 	c None",
".	c #FFFF6DB60000",
"X	c #000000000000",
"    .       ",
"    .       ",
"   ..       ",
"   ..       ",
"  ..  .     ",
"  ..  .     ",
" ......     ",
" .. ..  XX  ",
"    .. X  X ",
"   ..  X  X ",
"   ..  XXXX ",
"   .   X  X ",
"   .   X  X "};
/* XPM */
static const char * cast_b_xpm[] UNUSED = {
"12 13 3 1",
" 	c None",
".	c #FFFF6DB60000",
"X	c #000000000000",
"    .       ",
"    .       ",
"   ..       ",
"   ..       ",
"  ..  .     ",
"  ..  .     ",
" ......     ",
" .. .. XXX  ",
"    .. X  X ",
"   ..  XXX  ",
"   ..  X  X ",
"   .   X  X ",
"   .   XXX  "};
/* XPM */
static const char * cast_c_xpm[] UNUSED = {
"12 13 3 1",
" 	c None",
".	c #FFFF6DB60000",
"X	c #000000000000",
"    .       ",
"    .       ",
"   ..       ",
"   ..       ",
"  ..  .     ",
"  ..  .     ",
" ......     ",
" .. ..  XX  ",
"    .. X  X ",
"   ..  X    ",
"   ..  X    ",
"   .   X  X ",
"   .    XX  "};

static QString
aboutMsg()
{
    char *p, vbuf[BUFSZ];
    /* nethack's getversionstring() includes a final period
       but we're using it mid-sentence so strip period off */
    if ((p = strrchr(::getversionstring(vbuf, sizeof vbuf), '.')) != 0
        && *(p + 1) == '\0')
        *p = '\0';
    /* it's also long; break it into two pieces */
    (void) strsubst(vbuf, " - ", "\n- ");
    QString msg = nh_qsprintf(
        // format
        "NetHack-Qt是NetHack的一种版本, 基于" // no newline
#ifdef KDE
        "KDE和"                                       // ditto
#endif
        "Qt %d GUI toolkit构建.\n"                      // short Qt version
        "\n"
        "游戏版本是%s%s, Lua版本是%s.\n" // long nethack version, Qt & Lua versions
        "\n"
        "NetHack的Qt界面最初由Warwick Allison开发.\n"
        "\n"
#if 0
        "主页:\n     http://trolls.troll.no/warwick/nethack/\n" //obsolete
#endif
#ifdef KDE
        "KDE:\n     https://kde.org/\n"
#endif
#if 1
        "Qt:\n     https://qt.io/\n"
#else
        "Qt:\n     http://www.troll.no/\n"      // obsolete
#endif
        "Lua:\n     https://lua.org/\n"
        "NetHack:\n     %s\n", // DEVTEAM_URL
        // arguments
#ifdef QT_VERSION_MAJOR
        QT_VERSION_MAJOR,
#else
        5,              // Qt version macro should exist; if not, assume Qt5
#endif
        vbuf,           // nethack version
#ifdef QT_VERSION_STR
        ", Qt版本是" QT_VERSION_STR,
#else
        "",
#endif
        ::get_lua_version(),
        DEVTEAM_URL);
    return msg;
}

class SmallToolButton : public QToolButton {
public:
    SmallToolButton(const QPixmap &pm, const QString &textLabel,
                    const QString &grouptext,
                    QObject *receiver, const char *slot,
                    QWidget *parent) :
	QToolButton(parent)
    {
	setIcon(QIcon(pm));
	setToolTip(textLabel);
	setStatusTip(grouptext);
	connect(this, SIGNAL(clicked(bool)), receiver, slot);
    }

    QSize sizeHint() const
    {
	// get just a couple more pixels for the map
	return QToolButton::sizeHint()-QSize(0,2);
    }
};

NetHackQtMainWindow::NetHackQtMainWindow(NetHackQtKeyBuffer& ks) :
    message(0), map(0), status(0), invusage(0),
    hsplitter(0), vsplitter(0),
    keysink(ks), dirkey(0)
{
    QToolBar* toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setFocusPolicy(Qt::NoFocus);
    addToolBar(toolbar);
    menubar = menuBar();

    setWindowTitle("NetHack-Qt");
    setWindowIcon(QIcon(QPixmap(qt_compact_mode ? nh_icon_small : nh_icon)));

#ifdef MACOS
    /*
     * MacOS Note:
     *  The toolbar on MacOS starts with a system menu labeled with the
     *  Apple logo and an application menu labeled with the application's
     *  name (taken from Info.plist if present, otherwise the base name
     *  of the running program).  After that, application-specific menus
     *  (in our case "game",...,"help") follow.  Several menu entry
     *  names ("About", "Quit"/"Exit", "Preferences"/"Options"/
     *  "Settings"/"Setup"/"Config") get hijacked and placed in the
     *  application menu (and renamed in the process) even if the code
     *  here tries to put them in another menu.
     *  See QtWidgets/doc/qmenubar.html for slightly more information.
     *  setMenuRole() can be used to override this behavior.
     */
#endif

#ifdef CTRL_V_HACK
    // NetHackQtBind::notify() sees all control characters except for ^V
    QShortcut *c_V = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_V), this);
    connect(c_V, &QShortcut::activated, this, &NetHackQtMainWindow::CtrlV);
#endif

    QMenu* game=new QMenu;
    QMenu* apparel=new QMenu;
    QMenu* act1=new QMenu;
    QMenu* act2 = qt_compact_mode ? new QMenu : act1;
    QMenu* magic=new QMenu;
    QMenu* info=new QMenu;

    QMenu *help;
#ifdef KDE
    help = kapp->getHelpMenu( true, "" );
    help->addSeparator();
#else
    help = qt_compact_mode ? info : new QMenu;
#endif

    enum { OnDesktop=1, OnHandhelds=2 };
    struct Macro {
        QMenu *menu;
        const char *name;
        int flags;         // 1 desktop, 2 handheld, 3 either/both
        int (*funct)(void);
    } item[] = {
        { game,    0, 3, (int (*)(void)) 0},
        { game,    "扩展命令",  3, doextcmd },
        { game,    0, 3, (int (*)(void)) 0},
        { game,    "版本",            3, doversion},
        { game,    "编译时选项",        3, doextversion},
        { game,    "历史",            3, dohistory},
        { game,    "重绘画面",             0, doredraw}, // useless
        { game,
#ifdef MACOS
            /* Qt on OSX would rename "Options" to "Preferences..." and
               move it from intended destination to the application menu;
               the ampersand produces &O which makes Alt+O into a keyboard
               shortcut--except those are disabled by default by Qt on OSX */
                   "运行时 &" // rely on adjacent string concatenation
#endif
                   "选项",            3, doset},
        { game,    "探索模式",       3, enter_explore_mode},
        { game,    0, 3, (int (*)(void)) 0},
        { game,    "保存并退出",      3, dosave},
        { game,
#ifdef MACOS
            /* need something to prevent matching leading "quit" so that it
               isn't hijacked for the application menu; the ampersand is to
               make &Q be a keyboard shortcut (but see Options above) */
                   "\177&"
#endif
                   "不保存退出", 3, done2},

        { apparel, "脱下/拿下",        2, doddoremarm},
        { apparel, "批量脱下/拿下",        1, doddoremarm},
        { apparel, 0, 3, (int (*)(void)) 0},
        { apparel, "装备武器",       3, dowield},
        { apparel, "交换武器",   3, doswapweapon},
        { apparel, "双持战斗",  3, dotwoweapon},
        { apparel, "装填弹药",        3, dowieldquiver},
        { apparel, 0, 3, (int (*)(void)) 0},
        { apparel, "穿上护甲",         3, dowear},
        { apparel, "脱下护甲",     3, dotakeoff},
        { apparel, 0, 3, (int (*)(void)) 0},
        { apparel, "戴上饰品", 3, doputon},
        { apparel, "摘下饰品", 3, doremring},

        /* { act1,      "Again\tCtrl+A",           "\001", 2},
        { act1, 0, 0, 3}, */
        { act1, "使用",             3, doapply},
        { act1, "谈话",              3, dotalk},
        { act1, "关门",        3, doclose},
        { act1, "下楼梯",              3, dodown},
        { act1, "批量丢下",         2, doddrop},
        { act1, "丢下",              2, dodrop},
        { act1, "吃",               2, doeat},
        { act1, "刻字",           3, doengrave},
        /* { act1,      "Fight\tShift+F",             "F", 3}, */
        { act1, "发射弹药",  2, dofire},
        { act1, "暴力开锁",             3, doforce},
        { act1, "跳",              3, dojump},
        { act2, "踢",              2, dokick},
        { act2, "搜刮",              3, doloot},
        { act2, "开门",         3, doopen},
        { act2, "付款",               3, dopay},
        // calling this "Get" was confusing to experienced players
        { act1, "拾取", 3, dopickup},
        { act2, "休息",              2, donull},
        { act2, "骑乘",              3, doride},
        { act2, "搜索",            3, dosearch},
        { act2, "坐下",               3, dosit},
        { act2, "投掷",             2, dothrow},
        { act2, "解除陷阱",            3, dountrap},
        { act2, "上楼",                3, doup},
        { act2, "擦脸",         3, dowipe},

        { magic, "喝药水",     3, dodrink},
        { magic, "读卷轴/书", 3, doread},
        { magic, "挥舞魔杖",         3, dozap},
        { magic, "施放法术",        3, docast},
        { magic, "浸入",              3, dodip},
        { magic, "摩擦",              3, dorub},
        { magic, "激活特殊能力",           3, doinvoke},
        { magic, 0, 3, (int (*)(void)) 0},
        { magic, "献祭",            3, dosacrifice},
        { magic, "祈祷",             3, dopray},
        { magic, 0, 3, (int (*)(void)) 0},
        { magic, "传送",         3, dotelecmd},
        { magic, "使用怪物能力",   3, domonability},
        { magic, "超度",      3, doturn},

        { help,  "帮助",             3, dohelp},
        { help,  0, 3, (int (*)(void)) 0},
        { help,  "查看这里有什么",     3, dolook},
        { help,  "查找地图符号",    3, doquickwhatis},
        { help,  "查询符号含义...",       2, dowhatis},
        { help,  0, 1, (int (*)(void)) 0},

        { info,  "物品栏",        3, ddoinv},
        { info,  "属性", 3, doattributes },
        { info,  "地牢摘要",         3, dooverview },
        { info,  "自愿挑战",          3, doconduct},
        { info,  "已发现物品",      3, dodiscovered},
        { info,  "列举/重排法术",  3, dovspell},
        { info,  "调整物品栏字母", 3, doorganize },
        { info,  0, 3, (int (*)(void)) 0},
        { info,  "命名物品或怪物", 3, docallcmd},
        { info,  "命名当前层",   3, donamelevel },
        { info,  0, 3, (int (*)(void)) 0},
        { info,  "技能",  3, enhance_weapon_skill},

	{ 0, 0, 0, (int (*)(void)) 0 }
    };

    QAction *actn;
#ifndef MACOS
    (void) game->addAction("Qt选项...", this, SLOT(doQtSettings(bool)));
#else
    /* on OSX, put this in the application menu instead of the game menu;
       Qt would change the action name behind our backs; do it explicitly */
    actn = game->addAction("游戏选项...", this, SLOT(doQtSettings(bool)));
    actn->setMenuRole(QWidgetAction::PreferencesRole);
    /* we also want a "Quit NetHack" entry in the application menu;
       when "_Quit-without-saving" was called "Quit" it got intercepted
       for that, but now this needs to be added separately; we'll use a
       handy menu and let the interception put it in the intended place;
       unlike About, it is not a duplicate; _Quit-without-saving runs
       nethack's #quit command with "really quit?" prompt, this quit--with
       Command+q as shortcut--pops up a dialog to choose between quit or
       cancel-and-resume-playing */
    actn = game->addAction("退出NetHack-Qt", this, SLOT(doQuit(bool)));
    actn->setMenuRole(QWidgetAction::QuitRole);
#endif

    actn = help->addAction("关于NetHack-Qt", this, SLOT(doAbout(bool)));
#ifdef MACOS
    actn->setMenuRole(QWidgetAction::AboutRole);
    /* for OSX, the preceding "About" went into the application menu;
       now add another duplicate one to the Help dropdown menu */
    actn = help->addAction("关于NetHack-Qt", this, SLOT(doAbout(bool)));
    actn->setMenuRole(QWidgetAction::NoRole);
#else
    nhUse(actn);
#endif
    help->addSeparator();

    //help->addAction("NetHack Guidebook", this, SLOT(doGuidebook(bool)));
    //help->addSeparator();

    for (int i = 0; item[i].menu; ++i) {
	if ( item[i].flags & (qt_compact_mode ? 1 : 2) ) {
	    if (item[i].name) {
                char actchar[32];
                char menuitem[BUFSZ];
                actchar[0] = actchar[1] = '\0';
                if (item[i].funct) {
                    actchar[0] = cmd_from_func(item[i].funct);
                    if (actchar[0]
                        /* M-c won't work; translation between character
                           sets by the QString class can classify such
                           characters as erroneous and change them to '?' */
                        && ((actchar[0] & 0x7f) != actchar[0]
                        /* the vi movement keys won't work reliably
                           because toggling number_pad affects them but
                           doesn't redo these menus */
                            || strchr("hjklyubnHJKLYUBN", actchar[0])
                            || strchr("hjklyubn", (actchar[0] | 0x60))))
                        actchar[0] = '\0';
                }
                if (actchar[0] && !qt_compact_mode)
                    Sprintf(menuitem, "%.50s\t%.9s", item[i].name,
                            visctrl(actchar[0]));
                else
                    Sprintf(menuitem, "%s", item[i].name);

                if (item[i].funct && !actchar[0]) {
                    actchar[0] = '#';
                    (void) cmdname_from_func(item[i].funct,
                                             &actchar[1], FALSE);
                }
                if (actchar[0]) {
                    QString name = menuitem;
                    QAction *action = item[i].menu->addAction(name);
#if QT_VERSION < 0x060000
		    action->setData(actchar);
#else
		    action->setData(QString(actchar));
#endif
                }
	    } else {
		item[i].menu->addSeparator();
	    }
	}
    }

    game->setTitle("游戏");
    menubar->addMenu(game);
    apparel->setTitle("装备");
    menubar->addMenu(apparel);

    if ( qt_compact_mode ) {
	act1->setTitle("A-J");
	menubar->addMenu(act1);
	act2->setTitle("K-Z");
	menubar->addMenu(act2);
	magic->setTitle("魔法");
	menubar->addMenu(magic);
	info->setIcon(QIcon(QPixmap(info_xpm)));
	info->setTitle("信息");
	menubar->addMenu(info);
	//menubar->insertItem(QPixmap(map_xpm), this, SLOT(raiseMap()));
	//menubar->insertItem(QPixmap(msg_xpm), this, SLOT(raiseMessages()));
	//menubar->insertItem(QPixmap(stat_xpm), this, SLOT(raiseStatus()));
	info->addSeparator();
	info->addAction("地图", this, SLOT(raiseMap()));
	info->addAction("消息", this, SLOT(raiseMessages()));
	info->addAction("状态", this, SLOT(raiseStatus()));
    } else {
	act1->setTitle("行动");
	menubar->addMenu(act1);
	magic->setTitle("魔法");
	menubar->addMenu(magic);
	info->setTitle("信息");
	menubar->addMenu(info);
	menubar->addSeparator();
#ifndef MACOS
	help->setTitle("帮助");
#else
        // On OSX, an entry in the menubar called "Help" will get an
        // extra action, "Search [______]", inserted as the first entry.
        // We have no control over what it does and don't want it.
        //
        // Using actions() to fetch a list of all entries doesn't find it,
        // so we don't have its widget pointer to pass to removeAction().
        //
        // Altering the name with an invisible character to inhibit
        // string matching is ludicrous but keeps the unwanted action
        // from getting inserted into the "Help" menu behind our back.
        // Underscore works too and is more robust but unless we prepend
        // it to every entry, "_Help" would stand out as strange.
	help->setTitle("\177Help");
        // (Renaming back to "Help" after the fact does reset the menu's
        // name but it also results in the Search action being added.
        // Perhaps a custom context menu that changes its name to "Help"
        // as it is being shown--and possibly changes back afterward--
        // would work but the name mangling hack is much simpler.)
#endif
	menubar->addMenu(help);
    }

    // order changed: was Again, Get, Kick, Throw, Fire, Drop, Eat, Rest
    //                now Again, PickUp, Drop, Kick, Throw, Fire, Eat, Rest
    QSignalMapper* sm = new QSignalMapper(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(sm, SIGNAL(mappedString(const QString&)),
            this, SLOT(doKeys(const QString&)));
#else
    connect(sm, SIGNAL(mapped(const QString&)),
            this, SLOT(doKeys(const QString&)));
#endif
    AddToolButton(toolbar, sm, "重复上一个行为", do_repeat, QPixmap(again_xpm));
    // this used to be called "Get" which is confusing to experienced players
    AddToolButton(toolbar, sm, "拾取", dopickup, QPixmap(pickup_xpm));
    AddToolButton(toolbar, sm, "丢下", doddrop, QPixmap(drop_xpm));
    AddToolButton(toolbar, sm, "踢", dokick, QPixmap(kick_xpm));
    AddToolButton(toolbar, sm, "投掷", dothrow, QPixmap(throw_xpm));
    AddToolButton(toolbar, sm, "发射", dofire, QPixmap(fire_xpm));
    AddToolButton(toolbar, sm, "吃", doeat, QPixmap(eat_xpm));
    AddToolButton(toolbar, sm, "搜索", dosearch, QPixmap(search_xpm));
    AddToolButton(toolbar, sm, "休息", donull, QPixmap(rest_xpm));

    connect(game, SIGNAL(triggered(QAction *)),
            this, SLOT(doMenuItem(QAction *)));
    connect(apparel, SIGNAL(triggered(QAction *)),
            this, SLOT(doMenuItem(QAction *)));
    connect(act1, SIGNAL(triggered(QAction *)),
            this, SLOT(doMenuItem(QAction *)));
    if (act2 != act1)
	connect(act2, SIGNAL(triggered(QAction *)),
                this, SLOT(doMenuItem(QAction *)));
    connect(magic, SIGNAL(triggered(QAction *)),
            this, SLOT(doMenuItem(QAction *)));
    connect(info, SIGNAL(triggered(QAction *)),
            this, SLOT(doMenuItem(QAction *)));
    connect(help, SIGNAL(triggered(QAction *)),
            this, SLOT(doMenuItem(QAction *)));

#ifdef KDE
    setMenu (menubar);
#endif

#if QT_VERSION < 0x060000
    QSize screensize = QApplication::desktop()->size();
#else
    QSize screensize = screen()->size();
#endif
    int x=0,y=0;
    int w=screensize.width()-10; // XXX arbitrary extra space for frame
    int h=screensize.height()-50;

    int maxwn = 1400;
    int maxhn = 1024;
    if (qt_settings != NULL) {
        auto glyphs = &qt_settings->glyphs();
        if (glyphs != NULL) {
            maxwn = glyphs->width() * (COLNO + 1);
            maxhn = glyphs->height() * ROWNO * 6/4 + glyphs->height() * 10;
        }
    }

    // Be exactly the size we want to be - full map...
    if (w>maxwn) {
	x+=(w-maxwn)/2;
	w=maxwn; // Doesn't need to be any wider
    }
    if (h>maxhn) {
	y+=(h-maxhn)/2;
	h=maxhn; // Doesn't need to be any taller
    }

    setGeometry(x,y,w,h);

    if ( qt_compact_mode ) {
	stack = new QStackedWidget(this);
	setCentralWidget(stack);
    } else {
	vsplitter = new QSplitter(Qt::Vertical);
	setCentralWidget(vsplitter);
	hsplitter = new QSplitter(Qt::Horizontal);
	invusage = new NetHackQtInvUsageWindow(hsplitter);
	vsplitter->insertWidget(0, hsplitter);
	hsplitter->insertWidget(1, invusage);
    }
}

#ifdef CTRL_V_HACK
// called when ^V is typed while the main window has keyboard focus;
// all other control characters go through NetHackQtBind::notify()
void NetHackQtMainWindow::CtrlV()
{
    static const char cV[] = { CTRL('V'), '\0' };
    doKeys(cV);
}
#endif

// add a toolbar button to invoke command 'name' via function '(*func)()'
void NetHackQtMainWindow::AddToolButton(QToolBar *toolbar, QSignalMapper *sm,
                                        const char *name, int (*func)(void),
                                        QPixmap xpm)
{
    char actchar[2];
    uchar key;

    key = (uchar) cmd_from_func(func);

    // if key is valid, add a button for it; otherwise omit the command
    // (won't work as intended if a different command is bound to same key)
    if (key) {
        QToolButton *tb = new SmallToolButton(xpm, QString(name), "行动",
                                              sm, SLOT(map()), toolbar);
        actchar[0] = '\0';
        sm->setMapping(tb, strkitten(actchar, (char) key));
        toolbar->addWidget(tb);
    }
}

void NetHackQtMainWindow::zoomMap()
{
    qt_settings->toggleGlyphSize();
}

void NetHackQtMainWindow::raiseMap()
{
    if ( stack->currentWidget() == map->Widget() ) {
	zoomMap();
    } else {
	stack->setCurrentWidget(map->Widget());
    }
}

void NetHackQtMainWindow::raiseMessages()
{
    stack->setCurrentWidget(message->Widget());
}

void NetHackQtMainWindow::raiseStatus()
{
    stack->setCurrentWidget(status->Widget());
}

#if 0 // RLC this isn't used
class NetHackMimeSourceFactory : public Q3MimeSourceFactory {
public:
    const QMimeSource* data(const QString& abs_name) const
    {
	const QMimeSource* r = 0;
	if ( (NetHackMimeSourceFactory *) this
             == Q3MimeSourceFactory::defaultFactory() )
	    r = Q3MimeSourceFactory::data(abs_name);
	else
	    r = Q3MimeSourceFactory::defaultFactory()->data(abs_name);
	if ( !r ) {
	    int sl = abs_name.length();
	    do {
		sl = abs_name.lastIndexOf('/',sl-1);
		QString name = sl>=0 ? abs_name.mid(sl+1) : abs_name;
		int dot = name.lastIndexOf('.');
		if ( dot >= 0 )
		    name = name.left(dot);
		if ( name == "map" )
		    r = new Q3ImageDrag(QImage(map_xpm));
		else if ( name == "msg" )
		    r = new Q3ImageDrag(QImage(msg_xpm));
		else if ( name == "stat" )
		    r = new Q3ImageDrag(QImage(stat_xpm));
	    } while (!r && sl>0);
	}
	return r;
    }
};
#endif

/* used by doMenuItem() and for the toolbar buttons */
bool NetHackQtMainWindow::ok_for_command()
{
    /*
     * If the core expects text to be entered (perhaps typing in a wish,
     * assigning a name to something, or answering a y/n prompt), or a
     * map position or a direction is being picked, don't accept commands
     * from the toolbar.
     *
     * FIXME: it would be much better to gray-out inapplicable entries
     * when popping up a command menu instead of needing this.
     */
    if (::program_state.input_state != commandInp) {
        NetHackQtBind::qt_nhbell();
        // possibly call doKeys("\033"); here?
        return false;
    }
    return true;
}

void NetHackQtMainWindow::doMenuItem(QAction *action)
{
    if (!ok_for_command())
        return;
    /* this converts meta characters to '?'; menu processing has been
       changed to send multi-character "#abc" instead (already needed
       for commands that didn't have either a regular keystroke or a
       meta shortcut); it must send just enough to disambiguate from
       other extended command names, otherwise the remainder would be
       left in the queue for subsequent handling as additional commands */
    doKeys(action->data().toString());
}

void NetHackQtMainWindow::doQtSettings(bool)
{
    centerOnMain(qt_settings);
    qt_settings->show();
}

void NetHackQtMainWindow::doAbout(bool)
{
    QMessageBox::about(this, "关于NetHack-Qt", aboutMsg());
}

// on OSX, "quit nethack" has been selected in the application menu or
// "Command+Q" has been typed -- user is asking to quit the application;
// unlike with the window's Close button, user has a chance to back out
void NetHackQtMainWindow::doQuit(bool)
{
    // there is a separate Quit-without-saving menu entry in the game menu
    // that leads to nethack's "Really quit?" prompt; OSX players can use
    // either one, other implementations only have that other one (plus
    // nethack's #quit command itself) but this routine is unconditional
    // in case someone wants to change that
#ifdef MACOS
    QString info = nh_qsprintf("这将结束你的NetHack进程.%s",
                 !program_state.something_worth_saving ? ""
                 : "\n(如果要保存当前游戏, 请取消, "
                   "\n然后使用保存命令. )");
    /* this is similar to closeEvent but the details are different;
       first choice (Cancel) is the default action for most arbitrary keys;
       the second choice (Quit) is the action for <return> or <space>;
       <escape> leaves the popup waiting for some other response;
       the &<char> settings for Alt+<char> shortcuts don't work on OSX */
    int act = QMessageBox::information(this, "NetHack", info,
                                       "&取消并返回游戏",
                                       "&不保存退出",
                                       0, 1);
    switch (act) {
    case 0:
        // cancel
        break; // return to game
    case 1:
        // quit -- bypass the prompting preformed by done2()
        program_state.stopprint++;
        ::done(QUIT);
        /*NOTREACHED*/
        break;
    }
#endif
    return;
}

#if 0 // RLC this isn't used
void NetHackQtMainWindow::doGuidebook(bool)
{
    QDialog dlg(this);
    new QVBoxLayout(&dlg);
    Q3TextBrowser browser(&dlg);
    NetHackMimeSourceFactory ms;
    browser.setMimeSourceFactory(&ms);
    browser.setSource(QDir::currentPath()+"/Guidebook.html");
    if ( qt_compact_mode )
	dlg.showMaximized();
    dlg.exec();
}
#endif

void NetHackQtMainWindow::doKeys(const char *cmds)
{
    keysink.Put(cmds);
    qApp->exit();
}

void NetHackQtMainWindow::doKeys(const QString& k)
{
    /* [this should probably be using toLocal8Bit();
       toAscii() is not offered as an alternative...] */
    doKeys(k.toUtf8().constData()); //危险:toLatin1
}

// queue up the command name for a function, as if user had typed it
void NetHackQtMainWindow::FuncAsCommand(int (*func)(void))
{
    char cmdbuf[32];
    Strcpy(cmdbuf, "#");
    (void) cmdname_from_func(func, &cmdbuf[1], FALSE);
    doKeys(cmdbuf);
}

void NetHackQtMainWindow::AddMessageWindow(NetHackQtMessageWindow* window)
{
    message=window;
    if (!qt_compact_mode)
        hsplitter->insertWidget(0, message->Widget());
    ShowIfReady();
}

NetHackQtMessageWindow * NetHackQtMainWindow::GetMessageWindow()
{
    return message;
}

void NetHackQtMainWindow::AddMapWindow(NetHackQtMapWindow2* window)
{

    map=window;
    if (!qt_compact_mode)
        vsplitter->insertWidget(1, map->Widget());
    ShowIfReady();
    connect(map,SIGNAL(resized()),this,SLOT(layout()));
}

void NetHackQtMainWindow::AddStatusWindow(NetHackQtStatusWindow* window)
{
    status=window;
    if (!qt_compact_mode)
        hsplitter->insertWidget(2, status->Widget());
    ShowIfReady();
}

void NetHackQtMainWindow::RemoveWindow(NetHackQtWindow* window)
{
    if (window==status) {
	status=0;
	ShowIfReady();
    } else if (window==map) {
	map=0;
	ShowIfReady();
    } else if (window==message) {
	message=0;
	ShowIfReady();
    }
}

void NetHackQtMainWindow::updateInventory()
{
    if (invusage) {
	invusage->repaint();
    }
}

void NetHackQtMainWindow::fadeHighlighting(bool before_key)
{
    if (before_key) {
        // status highlighting fades at start of turn
        if (status)
            status->fadeHighlighting();
    } else {
        // message highlighting fades after user has given input
        if (message && message->hilit_mesgs())
            message->unhighlight_mesgs();
    }
}

void NetHackQtMainWindow::layout()
{
#if 0
    if ( qt_compact_mode )
	return;
    if (message && map && status) {
	QSize maxs=map->Widget()->maximumSize();
	int maph=std::min(height()*2/3,maxs.height());

	QWidget* c = centralWidget();
	int h=c->height();
	int toph=h-maph;
	int iuw=3*qt_settings->glyphs().width();
	int topw=(c->width()-iuw)/2;

	message->Widget()->setGeometry(0,0,topw,toph);
	invusage->setGeometry(topw,0,iuw,toph);
	status->Widget()->setGeometry(topw+iuw,0,topw,toph);
	map->Widget()->setGeometry(std::max(0,(c->width()-maxs.width())/2),
				   toph,c->width(),maph);
    }
#endif

    if (qt_settings && !qt_compact_mode
        && map && message && status && invusage) {
        // For the initial PaperDoll sizing, message window
        // and/or status window might still be empty;
        // widen them before changing PaperDoll to use saved settings.
        QList<int> splittersizes = hsplitter->sizes();
#define MIN_WIN_WIDTH 400
        if (splittersizes[0] < MIN_WIN_WIDTH
            || splittersizes[2] < MIN_WIN_WIDTH) {
            if (splittersizes[0] < MIN_WIN_WIDTH)
                splittersizes[0] = MIN_WIN_WIDTH;
#ifndef ENHANCED_PAPERDOLL
            if (splittersizes[1] < 6) // TILEWMIN
                splittersizes[1] = 16; // 16x16
#endif
            if (splittersizes[2] < MIN_WIN_WIDTH)
                splittersizes[2] = MIN_WIN_WIDTH;
            hsplitter->setSizes(splittersizes);
        }
#ifdef ENHANCED_PAPERDOLL
        // call resizePaperDoll() indirectly...
        qt_settings->resizeDoll();
#endif
        // reset widths
        int w = width(); /* of main window */
        int d = invusage->width();
        splittersizes[2] = w / 2 - (d * 1 / 4); // status
        splittersizes[1] = d;                   // invusage
        splittersizes[0] = w / 2 - (d * 3 / 4); // messages
        hsplitter->setSizes(splittersizes);
    }
}

#ifdef DYNAMIC_STATUSLINES
// called when 'statuslines' changes from 2 to 3 or vice versa; simpler to
// destroy and recreate the status window than to adjust existing fields
NetHackQtWindow *NetHackQtMainWindow::redoStatus()
{
    NetHackQtStatusWindow *oldstatus = this->status;
    if (!oldstatus)
        return NULL; // not ready yet?
    this->status = new NetHackQtStatusWindow;

    if (!qt_compact_mode)
        hsplitter->replaceWidget(2, this->status->Widget());

    delete oldstatus;
    ShowIfReady();

    return (NetHackQtWindow *) this->status;
}
#endif

void NetHackQtMainWindow::resizePaperDoll(bool showdoll)
{
#ifdef ENHANCED_PAPERDOLL
    // this is absurd...
    NetHackQtInvUsageWindow *w = static_cast <NetHackQtMainWindow *>
                                 (NetHackQtBind::mainWidget())->invusage;
    QList<int> hsplittersizes = hsplitter->sizes(),
              vsplittersizes = vsplitter->sizes();
    w->resize(w->sizeHint());

    int oldwidth = hsplittersizes[1],
        newwidth = w->width();
    if (newwidth != oldwidth) {
        if (oldwidth > newwidth)
            hsplittersizes[0] += (oldwidth - newwidth);
        else
            hsplittersizes[2] += (newwidth - oldwidth);
        hsplittersizes[1] = newwidth;
        hsplitter->setSizes(hsplittersizes);
    }

    // Height limit is 48+2 pixels per doll cell plus 1 pixel margin at top;
    // values greater than 44+2 need taller window which pushes the map down
    // (when font size 'Large' is used for messages and status; threshold
    // may vary by 1 or 2 for other sizes).
    // FIXME: this doesn't shrink the window back if size is reduced from 45+
    int oldheight = vsplittersizes[0],
        newheight = w->height();
    if (newheight > oldheight && oldheight > 0 && vsplittersizes[1] > 0) {
        vsplittersizes[0] = newheight;
        vsplitter->setSizes(vsplittersizes);
    }

    if (showdoll) {
        if (w->isHidden())
            w->show();
        else
            w->repaint();
    } else {
        if (w->isVisible())
            w->hide();
    }
#else
    nhUse(showdoll);
#endif /* ENHANCED_PAPERDOLL */
}

void NetHackQtMainWindow::resizeEvent(QResizeEvent*)
{
    layout();
#ifdef KDE
    updateRects();
#endif
}

void NetHackQtMainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if ( dirkey ) {
	doKeys(QString(QChar(dirkey)));
	if ( !event->isAutoRepeat() )
	    dirkey = 0;
    }
}

void NetHackQtMainWindow::keyPressEvent(QKeyEvent* event)
{
    // Global key controls

    // For desktop, arrow keys scroll map, since we don't want players
    // to think that's the way to move. For handhelds, the normal way is to
    // click-to-travel, so we allow the cursor keys for fine movements.

    //  321
    //  4 0
    //  567

    if ( event->isAutoRepeat() &&
	event->key() >= Qt::Key_Left && event->key() <= Qt::Key_Down )
	return;

    const char* d = gc.Cmd.dirchars;
    switch (event->key()) {
    case Qt::Key_Up:
	if ( dirkey == d[0] )
	    dirkey = d[1];
	else if ( dirkey == d[4] )
	    dirkey = d[3];
	else
	    dirkey = d[2];
        break;
    case Qt::Key_Down:
	if ( dirkey == d[0] )
	    dirkey = d[7];
	else if ( dirkey == d[4] )
	    dirkey = d[5];
	else
	    dirkey = d[6];
        break;
    case Qt::Key_Left:
	if ( dirkey == d[2] )
	    dirkey = d[1];
	else if ( dirkey == d[6] )
	    dirkey = d[7];
	else
	    dirkey = d[0];
        break;
    case Qt::Key_Right:
	if ( dirkey == d[2] )
	    dirkey = d[3];
	else if ( dirkey == d[6] )
	    dirkey = d[5];
	else
	    dirkey = d[4];
        break;
    case Qt::Key_PageUp:
	dirkey = 0;
	if (message) message->Scroll(0,-1);
        break;
    case Qt::Key_PageDown:
	dirkey = 0;
	if (message) message->Scroll(0,+1);
        break;
    case Qt::Key_Space:
        //if (flags.rest_on_space) {
        event->ignore(); // punt to NetHackQtBind::notify()
        return;
        //}
    case Qt::Key_Enter:
	if ( map )
	    map->clickCursor();
        break;
    default:
	dirkey = 0;
	event->ignore();
        break;
    }
}

// game window's Close button has been activated
void NetHackQtMainWindow::closeEvent(QCloseEvent *e UNUSED)
{
    int ok = 0;
    if ( program_state.something_worth_saving ) {
        /* this used to offer "Save" and "Cancel"
           but cancel (ignoring the close attempt) won't work
           if user has clicked on the window's Close button */
	int act = QMessageBox::information(this, "NetHack",
                              "这将结束你的NetHack进程.",
                              "&保存退出", "&不保存退出", 0, 1);
	switch (act) {
        case 0:
            // save portion of save-and-exit
            ok = dosave0();
            break;
        case 1:
            // quit -- bypass the prompting preformed by done2()
            ok = 1;
            program_state.stopprint++;
            ::done(QUIT);
            /*NOTREACHED*/
            break;
	}
    } else {
        /* nothing worth saving; just close/quit */
        ok = 1;
    }
    /* if !ok, we should try to continue, but we don't... */
    nhUse(ok);
    u.uhp = -1;
    NetHackQtBind::qt_exit_nhwindows(0);
    nh_terminate(EXIT_SUCCESS);
}

void NetHackQtMainWindow::ShowIfReady()
{
    if (message && map && status) {
        QWidget* hp = qt_compact_mode ? static_cast<QWidget *>(stack)
                                      : static_cast<QWidget *>(hsplitter);
        QWidget* vp = qt_compact_mode ? static_cast<QWidget *>(stack)
                                      : static_cast<QWidget *>(vsplitter);
	message->Widget()->setParent(hp);
	map->Widget()->setParent(vp);
	status->Widget()->setParent(hp);
	if ( qt_compact_mode ) {
	    message->setMap(map);
	    stack->addWidget(map->Widget());
	    stack->addWidget(message->Widget());
	    stack->addWidget(status->Widget());
	    raiseMap();
	} else {
	    layout();
	}
	showNormal();
    } else if (isVisible()) {
	hide();
    }
}

} // namespace nethack_qt_
