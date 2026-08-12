/* NetHack 5.0	optlist.h */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef OPTLIST_H
#define OPTLIST_H

/*
 *  NOTE:  If you add (or delete) an option, please review:
 *             doc/options.txt
 *
 *         It contains how-to info and outlines some required/suggested
 *         updates that should accompany your change.
 */

#define BACKWARD_COMPAT

extern int optfn_boolean(int, int, boolean, char *, char *);

enum OptType { BoolOpt, CompOpt, OthrOpt };
enum Y_N { No, Yes };
enum Off_On { Off, On };
/* Advanced options are only shown in the full, traditional options menu */
enum OptSection {
    OptS_General, OptS_Behavior, OptS_Map, OptS_Status, OptS_Advanced
};
enum menu_terminology_preference {
    Term_False, Term_Off, Term_Disabled, Term_Excluded, num_terms
};

struct allopt_t {
    const char *name;
    enum OptSection section;
    int minmatch;
    int expectedbuf;
    int idx;
    enum optset_restrictions setwhere;
    enum OptType opttyp;
    enum Y_N negateok;
    enum Y_N valok;
    enum Y_N dupeok;
    enum Y_N pfx;
    enum menu_terminology_preference termpref;
    boolean opt_in_out, *addr;
    int (*optfn)(int, int, boolean, char *, char *);
    const char *alias;
    const char *descr;
    const char *prefixgw;
    boolean initval, has_handler, dupdetected, disregarded;
};

#endif /* OPTLIST_H */

#if defined(NHOPT_PROTO) || defined(NHOPT_ENUM) || defined(NHOPT_PARSE)
/* clang-format off */
/* *INDENT-OFF* */

#define NoAlias ((const char *) 0)

#if defined(NHOPT_PROTO)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc) /*empty*/
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z)               \
static int optfn_##a(int, int, boolean, char *, char *);
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z)               \
static int pfxfn_##a(int, int, boolean, char *, char *);
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z)               \
static int optfn_##a(int, int, boolean, char *, char *);

#elif defined(NHOPT_ENUM)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc) opt_##a,
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z)   opt_##a,
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z)   pfx_##a,
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z)   opt_##a,

#elif defined(NHOPT_PARSE)
#define NHOPTB(a, sec, b, c, s, i, n, v, d, al, bp, termp, desc)             \
    { #a, OptS_##sec, 0, b, opt_##a, s, BoolOpt, n, v, d, No, termp, c,  \
      bp, &optfn_boolean, al, desc, (const char *) 0, i, 0, 0 , 0 },
#define NHOPTC(a, sec, b, c, s, n, v, d, h, al, z) \
    { #a, OptS_##sec, 0, b, opt_##a, s, CompOpt, n, v, d, No, 0, c,  \
      (boolean *) 0, &optfn_##a, al, z, (const char *) 0, Off, h, 0, 0 },
#define NHOPTP(a, sec, b, c, s, n, v, d, h, al, z) \
    { #a, OptS_##sec, 0, b, pfx_##a, s, CompOpt, n, v, d, Yes, 0, c, \
      (boolean *) 0, &pfxfn_##a, al, z, #a, Off, h, 0, 0 },
#define NHOPTO(m, sec, a, b, c, s, n, v, d, al, z) \
    { m, OptS_##sec, 0, b, opt_##a, s, OthrOpt, n, v, d, No, 0, c,   \
      (boolean *) 0, &optfn_##a, al, z, (const char *) 0, On, On, 0, 0 },

/* this is not reliable because TILES_IN_GLYPHMAP might be defined
 * in a multi-interface binary but not apply to the current interface */
#ifdef TILES_IN_GLYPHMAP
#define tiled_map_Def On
#define ascii_map_Def Off
#else
#define ascii_map_Def On
#define tiled_map_Def Off
#endif
#endif

/* B:nm, sec, ln, opt_*, setwhere?, on?, negat?, val?, dup?, hndlr? Alias,
            bool_p, term */
/* C:nm, sec, ln, opt_*, setwhere?, negateok?, valok?, dupok?, hndlr? Alias,
            desc */
/* P:pfx, sec, ln, opt_*, setwhere?, negateok?, valok?, dupok?, hndlr? Alias,
            desc*/
    /*
     * Most of the options are in alphabetical order; a few are forced
     * to the top of list so that doset() will list them first and
     * all_options_str() will gather them first to write to the top of
     * a new RC file by #saveoptions.
     *
     * windowtype comes first because its value can affect how wc_ and
     * wc2_ options are processed; playmode (for players who can't or
     * don't know how to specify a command line) and name (ditto, more
     * or less) come next; then role, race, gender, align.  Those will
     * be at the top of the file for #saveoptions constructed RC file.
     */
    NHOPTC(windowtype, Advanced, WINTYPELEN, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "使用的窗口系统(应最先设定)")
    NHOPTC(playmode, Advanced, 8, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "正常游戏, 不计分的探索模式, 或者调试模式")
    NHOPTC(name, Advanced, PL_NSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "你的角色的名字(e.g., name:Merlin-W)")
    NHOPTC(role, Advanced, PL_CSIZ, opt_in, set_gameview,
                Yes, Yes, Yes, No, "character",
                "你的起始职业(e.g., Barbarian, Valkyrie)")
    NHOPTC(race, Advanced, PL_CSIZ, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "你的起始种族(e.g., Human, Elf)")
    NHOPTC(gender, Advanced, 8, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "你的起始性别(male or female)")
    NHOPTC(alignment, Advanced, 8, opt_in, set_gameview,
                Yes, Yes, Yes, No, "align",
                "你的起始阵营(lawful, neutral, or chaotic)")
    /* end of special ordering; remainder of entries are in alphabetical order
     */
    NHOPTB(accessiblemsg, Advanced, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.accessiblemsg, Term_False,
           "在消息中添加位置信息")
    NHOPTB(acoustics, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.acoustics, Term_False,
           "你的角色是否能听到")
 /* NHOPTC(align) -- moved to top */
    NHOPTC(align_message, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, Yes, NoAlias, "消息窗口对齐")
    NHOPTC(align_status, Advanced, 20, opt_in, set_gameview,
                No, Yes, No, Yes, NoAlias, "状态窗口对齐")
#ifdef WIN32
    NHOPTC(altkeyhandling, Advanced, 20, opt_in, set_in_game,
                No, Yes, No, Yes, "altkeyhandler", "alt键处理")
#else
    NHOPTC(altkeyhandling, Advanced, 20, opt_in, set_in_config,
                No, Yes, No, Yes, "altkeyhandler", "(不可用)")
#endif
#ifdef ALTMETA
    NHOPTB(altmeta, Advanced, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.altmeta, Term_False,
           "将\"ESC c\"视为M-c(Meta+c, 8th bit set)")
#else
    NHOPTB(altmeta, Advanced, 0, opt_out, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(armorstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.armorstatus, Term_False,
                "在状态栏中汇总当前穿戴的护甲")
    NHOPTB(ascii_map, Advanced, 0, opt_in, set_in_game,
                ascii_map_Def, Yes, No, No, NoAlias, &iflags.wc_ascii_map,
                Term_False, "以文本显示地图")
    NHOPTO("autocompletions", Advanced, o_autocomplete, BUFSZ, opt_in,
                set_in_game, No, Yes, No, NoAlias, "编辑自动补全")
    NHOPTB(autodescribe, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.autodescribe, Term_False,
           "描述光标下的地形")
    NHOPTB(autodig, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.autodig, Term_False,
           "有挖掘工具时, 移动会自动挖掘")
    NHOPTB(autoopen, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.autoopen, Term_False,
           "走向门时尝试打开")
    NHOPTB(autopickup, Behavior, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.pickup, Term_False,
           "自动捡起物品")
    NHOPTO("autopickup exceptions", Behavior, o_autopickup_exceptions, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "编辑自动拾取例外")
    NHOPTB(autoquiver, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.autoquiver, Term_False,
           "射击时自动填满空箭袋")
    NHOPTC(autounlock, Behavior, 80, opt_out, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "遇到上锁的门或箱子时采取的行动")
    NHOPTB(bgcolors, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.bgcolors, Term_Off,
           "将背景色用于某些地图高亮")
    NHOPTO("bind keys", Advanced, o_bind_keys, BUFSZ, opt_in, set_in_game,
                No, Yes, No, NoAlias, "编辑键绑定")
#if defined(MICRO) && !defined(AMIGA)
    NHOPTB(BIOS, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.BIOS, Term_False,
           "使用IBM ROM BIOS调用")
#else
    NHOPTB(BIOS, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(blind, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "permablind", &u.uroleplay.blind, Term_False,
           "你的角色永久失明")
    NHOPTB(bones, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.bones, Term_False,
           "允许加载遗骨文件")
#ifdef BACKWARD_COMPAT
    NHOPTC(boulder, Advanced, 1, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "已弃用(请在sym文件中改用S_boulder)")
#endif
    NHOPTC(catname, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "你的起始宠物的名字(如果是猫)")
#ifdef INSURANCE
    NHOPTB(checkpoint, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.ins_chkpt, Term_False,
           "每次层变更后保存游戏状态")
#else
    NHOPTB(checkpoint, Advanced, 0, opt_out, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(cmdassist, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.cmdassist, Term_False,
           "输入方向错误时显示帮助信息")
    NHOPTB(color, Map, 0, opt_in, set_in_game,
           On, Yes, No, No, "colour", &iflags.wc_color, Term_False,
           "使用地图颜色")
    NHOPTB(confirm, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.confirm, Term_False,
           "在攻击驯服或和平状态的怪物之前询问")
#ifdef CRASHREPORT
    NHOPTC(crash_email, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "你在报告中使用的邮箱地址")
    NHOPTC(crash_name, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "你在报告中使用的名字")
    NHOPTC(crash_urlmax, Advanced, PL_NSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "可生成的url长度上线")
#endif
#ifdef CURSES_GRAPHICS
    NHOPTC(cursesgraphics, Advanced, 70, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "将curses显示符号加载到symset中")
#endif
    NHOPTB(customcolors, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, "customcolours", &iflags.customcolors,
           Term_False, "在地图中使用自定义颜色")
    NHOPTB(customsymbols, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, "customsymbols", &iflags.customsymbols,
           Term_False, "在地图中使用自定义utf8符号")
    NHOPTB(dark_room, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.dark_room, Term_False,
           "以不同方式显示视线范围外的地板")
    NHOPTB(deaf, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "permadeaf", &u.uroleplay.deaf, Term_False,
           "你的角色永久失聪")
#ifdef BACKWARD_COMPAT
    NHOPTC(DECgraphics, Advanced, 70, opt_in, set_in_config,
                Yes, Yes, No, No, NoAlias,
                "将DECGraphics加载到symset中")
#endif
    NHOPTB(debug_hunger, Advanced, 0, opt_in, set_wiznofuz,
           Off, Yes, No, No, NoAlias, &iflags.debug_hunger, Term_False,
           "无饥饿")
    NHOPTB(debug_mongen, Advanced, 0, opt_in, set_wiznofuz,
           Off, Yes, No, No, NoAlias, &iflags.debug_mongen, Term_False,
           "无随机怪物生成")
    NHOPTB(debug_overwrite_stairs, Advanced, 0, opt_in, set_wiznofuz,
                Off, Yes, No, No, NoAlias, &iflags.debug_overwrite_stairs,
           Term_False, "地图生成时可以覆盖楼梯")
    NHOPTC(disclose, Advanced, sizeof flags.end_disclose * 2,
                opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "游戏结束时应披露的信息类型")
    NHOPTC(dogname, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "你的起始宠物的名字(如果是小狗)")
    NHOPTB(dropped_nopick, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.nopick_dropped, Term_False,
           "不要自动捡起丢下的物品")
    NHOPTC(dungeon, Advanced, MAXDCHARS + 1,opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "绘制地牢地图时使用的符号列表")
    NHOPTC(effects, Advanced, MAXECHARS + 1, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "绘制特殊效果时使用的符号列表")
    NHOPTB(eight_bit_tty, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &iflags.wc_eight_bit_input,
           Term_False, "直接对终端输出8-bit字符")
    NHOPTB(extmenu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.extmenu, Term_False,
           "使用扩展命令菜单")
    NHOPTB(female, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, "male", &flags.female, Term_False,
           "已弃用; 使用gender:female")
    NHOPTB(fireassist, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.fireassist, Term_False,
           "fire-command试图提供帮助")
    NHOPTB(fixinv, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.invlet_constant, Term_False,
           "物品栏里的物品保持其字母")
    NHOPTC(font_map, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "地图窗口使用的字体")
    NHOPTC(font_menu, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "菜单使用的字体")
    NHOPTC(font_message, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias,
                "消息窗口使用的字体")
    NHOPTC(font_size_map, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "地图字体的大小")
    NHOPTC(font_size_menu, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "菜单字体的大小")
    NHOPTC(font_size_message, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "信息字体的大小")
    NHOPTC(font_size_status, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "状态字体的大小")
    NHOPTC(font_size_text, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "文本字体的大小")
    NHOPTC(font_status, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "状态窗口使用的字体")
    NHOPTC(font_text, Advanced, 40, opt_in, set_gameview,
                Yes, Yes, Yes, No, NoAlias, "文本窗口使用的字体")
    NHOPTB(force_invmenu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.force_invmenu, Term_False,
           "询问物品的命令会显示菜单")
    NHOPTC(fruit, General, PL_FSIZ, opt_in, set_in_game,
                No, Yes, No, No, NoAlias, "你喜欢吃的水果名称")
    NHOPTB(fullscreen, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.wc2_fullscreen, Term_False,
           "开启全屏")
 /* NHOPTC(gender) -- moved to top */
    NHOPTC(glyph, Advanced, 40, opt_in, set_in_game,
                No, Yes, Yes, No, NoAlias,
                "将字形的表示形式设置为Unicode值和颜色")
    NHOPTB(goldX, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.goldX, Term_False,
           " 金币视为未知或无诅咒的")
    NHOPTB(guicolor, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.wc2_guicolor, Term_False,
           "在UI中使用颜色")
    NHOPTB(help, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.help, Term_False,
           "使用whatis命令时显示所有可用信息")
    NHOPTB(herecmd_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.herecmd_menu, Term_False,
           "显示此位置可用的命令")
#if defined(MACOS9)
    NHOPTC(hicolor, Advanced, 15, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "同palette, 只是顺序相反")
#endif
    NHOPTB(hilite_pet, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc_hilite_pet, Term_False,
           "高亮宠物")
    NHOPTB(hilite_pile, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.hilite_pile, Term_False,
           "高亮物品堆")
#ifdef STATUS_HILITES
    NHOPTC(hilite_status, Advanced, 13, opt_out, set_in_game,
                Yes, Yes, Yes, No, NoAlias,
                "状态突出显示规则(可出现多次)")
#else
    NHOPTC(hilite_status, Advanced, 13, opt_out, set_in_config,
                Yes, Yes, Yes, No, NoAlias, "(不可用)")
#endif
    NHOPTB(hitpointbar, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc2_hitpointbar, Term_False,
           "生命值条显示颜色")
    NHOPTC(horsename, Advanced, PL_PSIZ, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "你的起始宠物的名字(如果是小马)")
#ifdef BACKWARD_COMPAT
    NHOPTC(IBMgraphics, Advanced, 70, opt_in, set_in_config,
                Yes, Yes, No, No, NoAlias,
                "将IBMGraphics加载到symset中")
#endif
    NHOPTB(idlecheckpoint, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.idlecheckpoint, Term_Off,
           "如果输入处于空闲状态10秒, 则更新检查点文件")
#ifndef MACOS9
    NHOPTB(ignintr, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.ignintr, Term_False,
           "忽略中断信号")
#else
    NHOPTB(ignintr, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(implicit_uncursed, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.implicit_uncursed, Term_False,
           "省略物品栏中的\"无诅咒的\"")
#if 0   /* obsolete - pre-OSX Mac */
    NHOPTB(large_font, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.obsolete,
           (char *)0)
#endif
    NHOPTB(legacy, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.legacy, Term_False,
           "显示介绍信息")
    NHOPTB(lit_corridor, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.lit_corridor, Term_False,
           "如果能看见, 将黑暗的走廊显示为明亮的")
    NHOPTB(lootabc, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.lootabc, Term_False,
           "搜刮时使用a/b/c而非o/i/c")
    NHOPTB(mail, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.biff, Term_False,
           "启用传信小鬼")
    NHOPTC(map_mode, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "Windows下的地图显示模式")
    NHOPTB(mention_decor, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.mention_decor, Term_False,
           "在经过有趣的地块特征时提供反馈")
    NHOPTB(mention_map, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.glyph_updates, Term_False,
           "在有趣的地图位置变化时提供反馈")
    NHOPTB(mention_walls, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.mention_walls, Term_False,
           "撞到墙时提供反馈")
    NHOPTC(menu_deselect_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "在菜单中取消选择所有选项")
    NHOPTC(menu_deselect_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "在本页菜单中取消选择所有选项")
    NHOPTC(menu_first_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "跳到菜单第一页")
    NHOPTC(menu_headings, Advanced, 4, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias, "菜单标题的显示风格")
    NHOPTC(menu_invert_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "在菜单中反选所有选项")
    NHOPTC(menu_invert_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "在本页菜单中反选所有选项")
    NHOPTC(menu_last_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "跳到菜单最后一页")
    NHOPTC(menu_next_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "切换到菜单下一页")
    NHOPTC(menu_objsyms, Advanced, 12, opt_in, set_in_game,
           Yes, Yes, No, Yes, "use_menu_glyphs",
           "在菜单中显示物品符号")
#ifdef TTY_GRAPHICS
    NHOPTB(menu_overlay, Advanced, 0, opt_in, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.menu_overlay, Term_False,
           "菜单叠加并向右对齐")
#else
    NHOPTB(menu_overlay, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTC(menu_previous_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "切换到菜单上一页")
    NHOPTC(menu_search, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "搜索一个菜单选项")
    NHOPTC(menu_select_all, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "在菜单中全选")
    NHOPTC(menu_select_page, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "在本页菜单中全选")
    NHOPTC(menu_shift_left, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "本页菜单左移")
    NHOPTC(menu_shift_right, Advanced, 4, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "本页菜单右移")
    NHOPTB(menu_tab_sep, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.menu_tab_sep, Term_False,
           "菜单格式化")
    NHOPTB(menucolors, Advanced, 0, opt_in, set_in_game,
           Off, Yes, Yes, No, NoAlias, &iflags.use_menu_color, Term_False,
           "使用菜单颜色")
    NHOPTO("menu colors", Status, o_menu_colors, BUFSZ, opt_in, set_in_game,
                No, Yes, No, NoAlias, "修改菜单使用的颜色")
    NHOPTC(menuinvertmode, Advanced, 5, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "菜单反转实验性行为")
    NHOPTC(menustyle, Advanced, MENUTYPELEN, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "选择物品的用户接口")
    NHOPTO("message types", Advanced, o_message_types, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "编辑消息类型")
    NHOPTB(mon_movement, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.mon_movement, Term_False,
           "主角看见怪物移动时显示的消息")
    NHOPTB(monpolycontrol, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.mon_polycontrol, Term_False,
           "控制怪物变形")
    NHOPTB(montelecontrol, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.mon_telecontrol, Term_False,
           "控制怪物传送目标")
    NHOPTC(monsters, Advanced, MAXMCLASSES, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "怪物符号列表")
    NHOPTC(mouse_support, Advanced, 0, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "游戏接收鼠标按键信号")
#if PREV_MSGS /* tty or curses */
    NHOPTC(msg_window, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "控制\"查看之前消息\"(^P)的行为")
#else
    NHOPTC(msg_window, Advanced, 1, opt_in, set_in_config,
                Yes, Yes, No, Yes, NoAlias, "(不可用)")
#endif
    NHOPTC(msghistory, Advanced, 5, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "保存的顶层消息数")
 /* NHOPTC(name) -- moved to top */
#ifdef NEWS
    NHOPTB(news, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.news, Term_False,
           "游戏开始时显示新闻")
#else
    NHOPTB(news, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(nudist, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.nudist, Term_False,
           "你的角色起始时没有防具")
    NHOPTB(null, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.null, Term_False,
           "允许想终端发送null")
    NHOPTC(number_pad, General, 1, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "使用数字键盘移动")
    NHOPTC(objects, Advanced, MAXOCLASSES, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "物品符号列表")
    NHOPTC(packorder, Advanced, MAXOCLASSES, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "物品栏中的物品种类顺序")
#ifdef CHANGE_COLOR
#ifndef MACOS9     /* not old Mac OS9 */
    NHOPTC(palette, Advanced, 15, opt_in, set_gameview,
                No, Yes, Yes, No, "hicolor",
                "样式(以palette(color/R-G-B)调整RGB颜色)")
#else
    NHOPTC(palette, Advanced, 15, opt_in, set_in_game,
                No, Yes, Yes, No, "hicolor",
                "样式(00c/880/-fff等于blue/yellow/reverse white)")
#endif
#endif
    /* prior to paranoid_confirmation, 'prayconfirm' was a distinct option */
    NHOPTC(paranoid_confirmation, Advanced, 28, opt_in, set_in_game,
                Yes, Yes, Yes, Yes, "prayconfirm",
                "特定情况下再询问一次")
    NHOPTB(pauper, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.pauper, Term_False,
           "你的角色起始时没有物品")
    NHOPTB(perm_invent, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &iflags.perm_invent, Term_Off,
                "显示永久物品栏窗口")
    NHOPTC(perminv_mode, Advanced, 20, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "永久物品栏窗口显示的内容")
    NHOPTC(petattr, Advanced, 88, opt_in, set_in_game, /* tty/curses only */
                No, Yes, No, Yes, NoAlias, "高亮宠物的属性")
    /* pettype is ignored for some roles */
    NHOPTC(pettype, Advanced, 4, opt_in, set_gameview,
                Yes, Yes, No, No, "pet", "你喜欢的起始宠物类型")
    NHOPTC(pickup_burden, Advanced, 20, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "不询问是否捡起物品的最大负重度")
    NHOPTB(pickup_stolen, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.pickup_stolen, Term_False,
           "自动拾取被偷的物品")
    NHOPTB(pickup_thrown, Behavior, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.pickup_thrown, Term_False,
           "自动拾取被扔出的物品")
    NHOPTC(pickup_types, Behavior, MAXOCLASSES, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "自动拾取的种类")
    NHOPTC(pile_limit, Advanced, 24, opt_in, set_in_game,
                Yes, Yes, No, No, NoAlias,
                "\"这里有许多物品\"的阈值")
    NHOPTC(player_selection, Advanced, 12, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "通过对话或询问选择角色")
 /* NHOPTC(playmode) -- moved to top */
    NHOPTB(popup_dialog, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc_popup_dialog, Term_False,
           (char *)0)
    NHOPTB(preload_tiles, Advanced, 0, opt_out, set_in_config, /* MSDOS only */
           On, Yes, No, No, NoAlias, &iflags.wc_preload_tiles, Term_False,
           (char *)0)
    NHOPTB(price_quotes, General, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.pricequotes, Term_False,
           "为未鉴定物品显示你知道的价格")
    NHOPTB(pushweapon, Behavior, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.pushweapon, Term_False,
           "上一个武器进入副武器槽")
    NHOPTB(query_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.query_menu, Term_False,
           "询问yes/no时使用确认")
    NHOPTB(quick_farsight, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.quick_farsight, Term_False,
           "强制看地图时跳过地图浏览指南")
 /* NHOPTC(race) -- moved to top */
#ifdef MICRO
    NHOPTB(rawio, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.rawio, Term_False,
           "允许使用raw I/O")
#else
    NHOPTB(rawio, Advanced, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(reroll, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &u.uroleplay.reroll, Term_False,
           "允许重掷起始物品栏")
    NHOPTB(rest_on_space, Advanced, 0, opt_in, set_in_game, Off,
           Yes, No, No, NoAlias, &flags.rest_on_space, Term_False,
           "空格绑定休息命令")
    NHOPTC(roguesymset, Advanced, 70, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "从symbols文件中加载加载一系列rogue显示符号")
 /* NHOPTC(role) -- moved to top */
    NHOPTC(runmode, Advanced, sizeof "teleport", opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "`奔跑'或者`旅行'时显示频率")
    NHOPTB(safe_pet, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.safe_dog, Term_False,
           "阻止你攻击宠物")
    NHOPTB(safe_wait, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.safe_wait, Term_False,
           "在有相邻敌人时阻止等待")
    NHOPTB(sanity_check, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.sanity_check, Term_False,
           "实行数据合法性检测")
    NHOPTC(scores, Advanced, 32, opt_in, set_in_game,
                No, Yes, No, No, NoAlias,
                "你想看的得分表部分")
    NHOPTC(scroll_amount, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "到达scroll_margin时地图滚动的长度")
    NHOPTC(scroll_margin, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias,
                "距离边缘这么远时滚动地图")
    NHOPTB(selectsaved, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc2_selectsaved, Term_False,
           (char *)0)
    NHOPTB(showdamage, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.showdamage, Term_False,
           "在消息栏中显示主角受到的伤害")
    NHOPTB(showexp, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showexp, Term_False,
           "在状态行中显示经验点数")
    NHOPTB(showrace, Map, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showrace, Term_False,
           "通过种族而非职业显示你的角色")
#ifdef SCORE_ON_BOTL
    NHOPTB(showscore, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showscore, Term_False,
           "在状态行中显示当前得分")
#else
    NHOPTB(showscore, Status, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(showvers, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.showvers, Term_False,
           "在状态行中显示版本信息")
    NHOPTB(silent, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.silent, Term_False,
           "不使用终端时钟")
    NHOPTB(softkeyboard, Advanced, 0, opt_in, set_in_config,
                Off, Yes, No, No, NoAlias, &iflags.wc2_softkeyboard,
           Term_False, (char *)0)
    NHOPTC(sortdiscoveries, Advanced, 0, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "显示已发现物品时的优先顺序")
    NHOPTC(sortloot, Advanced, 4, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "通过描述对物品选择列表进行排序")
    NHOPTB(sortpack, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.sortpack, Term_False,
           "在物品栏中通过类别分组")
    NHOPTC(sortvanquished, Advanced, 0, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "显示已击败怪物时的优先顺序")
    NHOPTC(soundlib, Advanced, WINTYPELEN, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "使用的soundlib接口(如果有)")
#ifdef SND_LIB_INTEGRATED
    NHOPTB(sounds, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.sounds, Term_Off,
           "使用内置音效")
#else
    NHOPTB(sounds, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.sounds, Term_Off,
           "使用音效")
#endif
    NHOPTB(sparkle, Map, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.sparkle, Term_False,
           "抵抗魔法时显示粒子效果(推荐在火之位面打开)")
    NHOPTB(spot_monsters, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &a11y.mon_notices, Term_False,
           "英雄发现怪物时显示的消息")
    NHOPTB(splash_screen, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc_splash_screen, Term_False,
           (char *)0)
    NHOPTB(standout, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.standout, Term_False,
           "使用standout配合--more--")
    NHOPTB(status_updates, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.status_updates, Term_False,
           "允许状态行更新")
    NHOPTO("status condition fields", Status, o_status_cond, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "更改状态条件的高亮显示")
#ifdef STATUS_HILITES
    NHOPTC(statushilites, Advanced, 20, opt_in, set_in_game,
                Yes, Yes, Yes, No, NoAlias,
                "0=无状态条件高亮, N=状态条件高亮N回合")
    NHOPTO("status highlight rules", Status, o_status_hilites, BUFSZ,
                opt_in, set_in_game,
                No, Yes, No, NoAlias, "更改状态行高亮显示")
#else
    NHOPTC(statushilites, Advanced, 20, opt_in, set_in_config,
                Yes, Yes, Yes, No, NoAlias, "高亮控制")
#endif
    NHOPTC(statuslines, Status, 20, opt_in, set_in_game,
                No, Yes, No, No, NoAlias, "状态显示2或3行")
#ifdef WIN32CON
    NHOPTC(subkeyvalue, Advanced, 7, opt_in, set_in_config,
                No, Yes, Yes, No, NoAlias, "覆盖按键值")
#endif
    NHOPTC(suppress_alert, Advanced, 8, opt_in, set_in_game,
                No, Yes, Yes, No, NoAlias,
                "屏蔽有关特定版本功能的提醒")
    NHOPTC(symset, Map, 70, opt_in, set_in_game,
                No, Yes, No, Yes, NoAlias,
                "从symbols文件中加载加载一系列显示符号")
    NHOPTC(term_cols, Advanced, 6, opt_in, set_in_config,
                No, Yes, No, No, "termcolumns", "列数")
    NHOPTC(term_rows, Advanced, 6, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "行数")
    NHOPTB(terrainstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.terrainstatus, Term_False,
                "将主角的位置显示为状态字段")
    NHOPTC(tile_file, Advanced, 70, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "tile文件的名称")
    NHOPTC(tile_height, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "tile的高度")
    NHOPTC(tile_width, Advanced, 20, opt_in, set_gameview,
                Yes, Yes, No, No, NoAlias, "tile的宽度")
    NHOPTB(tiled_map, Advanced, 0, opt_in, set_in_game,
                tiled_map_Def, Yes, No, No, NoAlias, &iflags.wc_tiled_map,
           Term_False, (char *)0)
    NHOPTB(time, Status, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.time, Term_False,
           "在状态行中显示游戏回合数")
#ifdef TIMED_DELAY
    NHOPTB(timed_delay, Map, 0, opt_out, set_in_game,
           Off, Yes, No, No, NoAlias, &flags.nap, Term_False,
           "暂停时使用延迟以实现显示效果")
#else
    NHOPTB(timed_delay, Map, 0, opt_in, set_in_config,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(tips, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.tips, Term_False,
           "在游戏中显示一些有用的帮助")
    NHOPTB(tombstone, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.tombstone, Term_False,
           "当你的角色死亡时, 显示墓碑")
    NHOPTB(toptenwin, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.toptenwin, Term_False,
           "在窗口中显示最高分")
    NHOPTC(traps, Advanced, MAXTCHARS + 1, opt_in, set_in_config,
                No, Yes, No, No, NoAlias,
                "绘制陷阱时使用的符号列表")
    NHOPTB(travel, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.travelcmd, Term_False,
           "允许通过鼠标点击旅行")
#ifdef DEBUG
    NHOPTB(travel_debug, Advanced, 0, opt_out, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.trav_debug, Term_False,
           (char *)0)
#else
    NHOPTB(travel_debug, Advanced, 0, opt_out, set_wizonly,
           Off, No, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTB(tutorial, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &flags.tutorial, Term_False,
           "询问是否开始教程")
    NHOPTB(use_darkgray, Advanced, 0, opt_out, set_in_config,
           On, Yes, No, No, NoAlias, &iflags.wc2_darkgray, Term_False,
           "使用黑色粗体字而非蓝色")
    NHOPTB(use_inverse, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &iflags.wc_inverse, Term_False,
           "以反色显示发现到的怪物")
    NHOPTB(use_truecolor, Advanced, 0, opt_in, set_in_config,
                Off, Yes, No, No, "use_truecolour",
           &iflags.use_truecolor, Term_False,
           (char *)0)
    NHOPTC(vary_msgcount, Advanced, 20, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "每次显示更多旧消息")
    NHOPTB(verbose, Advanced, 0, opt_out, set_in_game,
           On, Yes, No, No, NoAlias, &flags.verbose, Term_False,
           (char *)0)
    NHOPTC(versinfo, Advanced, 80, opt_out, set_in_game,
           No, Yes, No, Yes, NoAlias, "'showvers'显示额外信息")
#ifdef MSDOS
    NHOPTC(video, Advanced, 20, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "视频更新的方法")
#endif
#ifdef VIDEOSHADES
    NHOPTC(videocolors, Advanced, 40, opt_in, set_gameview,
                No, Yes, No, No, "videocolours",
                "内部屏幕例程的颜色映射")
    NHOPTC(videoshades, Advanced, 32, opt_in, set_gameview,
                No, Yes, No, No, NoAlias,
                "gray在地图中渲染成black/gray/white")
#endif
#ifdef MSDOS
    NHOPTC(video_width, Advanced, 10, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "视频宽度")
    NHOPTC(video_height, Advanced, 10, opt_in, set_gameview,
                No, Yes, No, No, NoAlias, "视频高度")
#endif
#ifdef SND_SPEECH
    NHOPTB(voices, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.voices, Term_Off,
           (char *)0)
#else
    NHOPTB(voices, Advanced, 0, opt_in, set_gameview,
           Off, Yes, No, No, NoAlias, &iflags.voices, Term_Excluded,
           (char *)0)
#endif
#ifdef TTY_TILES_ESCCODES
    NHOPTB(vt_tiledata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.vt_tiledata, Term_False,
           "输出特殊的转义码")
#else
    NHOPTB(vt_tiledata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
#ifdef TTY_SOUND_ESCCODES
    NHOPTB(vt_sounddata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, &iflags.vt_sounddata, Term_False,
           "以特殊转义码输出声音数据")
#else
    NHOPTB(vt_sounddata, Advanced, 0, opt_in, set_in_config,
           Off, Yes, No, No, NoAlias, (boolean *) 0, Term_False,
           (char *)0)
#endif
    NHOPTC(warnings, Advanced, 10, opt_in, set_in_config,
                No, Yes, No, No, NoAlias, "显示警告字符")
    NHOPTB(weaponstatus, Advanced, 0, opt_in, set_in_game,
                Off, Yes, No, No, NoAlias, &flags.weaponstatus, Term_False,
                "在状态栏中显示装备的武器")
    NHOPTC(whatis_coord, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "自动描述光标位置时显示坐标")
    NHOPTC(whatis_filter, Advanced, 1, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias,
                "在定位下一项或上一项时过滤坐标位置")
    NHOPTB(whatis_menu, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.getloc_usemenu, Term_False,
           "获取地图位置时显示菜单")
    NHOPTB(whatis_moveskip, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.getloc_moveskip, Term_False,
           "获取地图位置时跳过相同的图标")
    NHOPTC(windowborders, Advanced, 9, opt_in, set_in_game,
                Yes, Yes, No, Yes, NoAlias, "0(关), 1(开), 2(自动)")
#ifdef WINCHAIN
    NHOPTC(windowchain, Advanced, WINTYPELEN, opt_in, set_in_sysconf,
                No, Yes, No, No, NoAlias, "使用的窗口处理器")
#endif
    NHOPTC(windowcolors, Advanced, 80, opt_in, set_gameview,
                No, Yes, Yes, No, NoAlias,
                "窗口的前景/背景色")
 /* NHOPTC(windowtype) -- moved to top */
    NHOPTB(wizmgender, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.wizmgender, Term_False,
           (char *)0)
    NHOPTB(wizweight, Advanced, 0, opt_in, set_wizonly,
           Off, Yes, No, No, NoAlias, &iflags.wizweight, Term_False,
           (char *)0)
    NHOPTB(wraptext, Advanced, 0, opt_in, set_in_game,
           Off, Yes, No, No, NoAlias, &iflags.wc2_wraptext, Term_False,
           (char *)0)

    /*
     * Prefix-based Options
     */

    NHOPTP(cond_, Advanced, 0, opt_in, set_hidden,
                Yes, No, Yes, Yes, NoAlias, "cond_选项的前缀")
    NHOPTP(font, Advanced, 0, opt_in, set_hidden,
                Yes, Yes, Yes, No, NoAlias, "字体选项的前缀")
#if defined(MICRO) && !defined(AMIGA)
    /* included for compatibility with old NetHack.cnf files */
    NHOPTP(IBM_, Advanced, 0, opt_in, set_hidden,
                No, No, Yes, No, NoAlias, "旧的micro IBM_选项的前缀")
#endif /* MICRO */

#undef NoAlias
#undef NHOPTB
#undef NHOPTC
#undef NHOPTP
#undef NHOPTO

/* *INDENT-ON* */
/* clang-format on */
#endif /* NHOPT_PROTO || NHOPT_ENUM || NHOPT_PARSE */

/*optlist.h*/
