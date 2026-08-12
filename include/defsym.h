/* NetHack 5.0   defsym.h $NHDT-Date: 1725653007 2024/09/06 20:03:27 $ $NHDT-Branch: NetHack-3.7 $ $NHDT-Revision: 1.25 $ */
/*      Copyright (c) 2016 by Pasi Kallinen              */
/* NetHack may be freely redistributed.  See license for details. */

/*
    This header is included in multiple places to produce
    different code depending on its use. Its purpose is to
    ensure that there is only one definitive source for
    pchar, objclass and mon symbols.

    The morphing macro expansions are used in these places:
  - in include/sym.h for enums of some S_* symbol values
    (define PCHAR_S_ENUM, MONSYMS_S_ENUM prior to #include defsym.h)
  - in include/sym.h for enums of some DEF_* symbol values
    (define MONSYMS_DEFCHAR_ENUM prior to #include defsym.h)
  - in include/objclass.h for enums of some default character values
    (define OBJCLASS_DEFCHAR_ENUM prior to #include defsym.h)
  - in include/objclass.h for enums of some *_CLASS values
    (define OBJCLASS_CLASS_ENUM prior to #include defsym.h)
  - in include/objclass.h for enums of S_* symbol values
    (define OBJCLASS_S_ENUM prior to #include defsym.h)
  - in src/symbols.c for parsing S_ entries in config files
    (define PCHAR_PARSE, MONSYMS_PARSE, OBJCLASS_PARSE prior
    to #include defsym.h)
  - in src/drawing.c for initializing some data structures/arrays
    (define PCHAR_DRAWING, MONSYMS_DRAWING, OBJCLASS_DRAWING prior
    to #include defsym.h)
  - in win/share/tilemap.c for processing a tile file
    (define PCHAR_TILES prior to #include defsym.h).
  - in src/allmain.c for setting up the dumping of several enums
    (define DUMP_ENUMS_PCHAR, DUMP_ENUMS_MONSYS, DUMP_ENUMS_MONSYMS_DEFCHAR
     DUMP_ENUMS_OBJCLASS_DEFCHARS, DUMP_ENUMS_OBJCLASS_DEFCHARS
     DUMP_ENUMS_OBJCLASS_CLASSES, DUMP_ENUMS_OBJCLASS_SYMS)
*/

#if defined(PCHAR_S_ENUM)               \
    || defined(PCHAR_PARSE)             \
    || defined(PCHAR_DRAWING)           \
    || defined(PCHAR_TILES)             \
    || defined(DUMP_ENUMS_PCHAR)

/*
   PCHAR(idx, ch, sym, bdesc, desc, clr)
       idx:     index used in enum
       ch:      character symbol
       sym:     symbol name for parsing purposes (also tile name)
       bdesc:   original English description
       desc:    description
       clr:     color

   PCHAR2(idx, ch, sym, tilenm, bdesc, desc, clr)
       idx:     index used in enum
       ch:      character symbol
       sym:     symbol name for parsing purposes
       tilenm:  if the name in the tile txt file differs from desc (below),
                the name in the tile txt file can be specified here.
       bdesc:   original English description
       desc:    description
       clr:     color
*/

#if defined(PCHAR_S_ENUM)
/* sym.h */
#define PCHAR(idx, ch, sym, bdesc, desc, clr) sym = idx,

#elif defined(PCHAR_PARSE)
/* symbols.c */
#define PCHAR(idx, ch, sym, bdesc, desc, clr) { SYM_PCHAR, sym, #sym },

#elif defined(PCHAR_DRAWING)
/* drawing.c */
#define PCHAR(idx, ch, sym, bdesc, desc, clr) { ch, bdesc, desc, clr },

#elif defined(PCHAR_TILES)
/* win/share/tilemap.c */
#define PCHAR(idx, ch, sym, bdesc, desc, clr) { sym, bdesc, bdesc },
#define PCHAR2(idx, ch, sym, tilenm, bdesc, desc, clr) { sym, tilenm, bdesc },

#elif defined(DUMP_ENUMS_PCHAR)
/* allmain.c */
#define PCHAR(idx, ch, sym, bdesc, desc, clr) { sym, #sym },
#ifndef PCHAR2
#define PCHAR2(idx, ch, sym, tilenm, bdesc, desc, clr) { sym, #sym },
#endif
#endif

/* PCHAR with extra arg */
#ifndef PCHAR2
#define PCHAR2(idx, ch, sym, tilenm, bdesc, desc, clr) \
    PCHAR(idx, ch, sym, bdesc, desc, clr)
#endif

    PCHAR2( 0, ' ',  S_stone,  "dark part of a room", "stone", "石头", NO_COLOR)
    PCHAR2( 1, '|',  S_vwall,  "vertical wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 2, '-',  S_hwall,  "horizontal wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 3, '-',  S_tlcorn, "top left corner wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 4, '-',  S_trcorn, "top right corner wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 5, '-',  S_blcorn, "bottom left corner wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 6, '-',  S_brcorn, "bottom right corner wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 7, '-',  S_crwall, "cross wall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 8, '-',  S_tuwall, "tuwall", "wall", "墙壁", CLR_GRAY)
    PCHAR2( 9, '-',  S_tdwall, "tdwall", "wall", "墙壁", CLR_GRAY)
    PCHAR2(10, '|',  S_tlwall, "tlwall", "wall", "墙壁", CLR_GRAY)
    PCHAR2(11, '|',  S_trwall, "trwall", "wall", "墙壁", CLR_GRAY)
    /* start cmap A                                                      */
    PCHAR2(12, '.',  S_ndoor,  "no door", "doorway", "门口", CLR_GRAY)
    PCHAR2(13, '-',  S_vodoor, "vertical open door", "open door", "打开的门", CLR_BROWN)
    PCHAR2(14, '|',  S_hodoor, "horizontal open door", "open door", "打开的门", CLR_BROWN)
    PCHAR2(15, '+',  S_vcdoor, "vertical closed door", "closed door", "关闭的门", CLR_BROWN)
    PCHAR2(16, '+',  S_hcdoor, "horizontal closed door", "closed door", "关闭的门", CLR_BROWN)
    PCHAR2(17, '#',  S_bars,   "iron bars", "iron bars", "铁栅栏", HI_METAL)
    PCHAR2(18, '#',  S_tree,   "tree", "tree", "树木", CLR_GREEN)
    PCHAR2(19, '.',  S_room,   "floor of a room", "floor of a room", "房间地板", CLR_GRAY)
    PCHAR2(20, '.',  S_darkroom, "dark part of a room", "dark part of a room", "房间暗处", CLR_BLACK)
    PCHAR2(21, '`',  S_engroom, "engraving in a room", "engraving", "写字", CLR_BRIGHT_BLUE)
    PCHAR2(22, '#',  S_corr,   "dark corridor", "corridor", "走廊", CLR_GRAY)
    PCHAR2(23, '#',  S_litcorr, "lit corridor", "lit corridor", "照亮的走廊", CLR_GRAY)
    PCHAR2(24, '#',  S_engrcorr, "engraving in a corridor", "engraving", "写字", CLR_BRIGHT_BLUE)
    PCHAR2(25, '<',  S_upstair, "up stairs", "staircase up", "上行楼梯", CLR_GRAY)
    PCHAR2(26, '>',  S_dnstair, "down stairs", "staircase down", "下行楼梯", CLR_GRAY)
    PCHAR2(27, '<',  S_upladder, "up ladder", "ladder up", "上行梯子", CLR_BROWN)
    PCHAR2(28, '>',  S_dnladder, "down ladder", "ladder down", "下行梯子", CLR_BROWN)
    PCHAR2(29, '<',  S_brupstair, "branch staircase up", "branch staircase up", "分支上行楼梯", CLR_YELLOW)
    PCHAR2(30, '>',  S_brdnstair, "branch staircase down", "branch staircase down", "分支下行楼梯", CLR_YELLOW)
    PCHAR2(31, '<',  S_brupladder, "branch ladder up", "branch ladder up", "分支上行梯子", CLR_YELLOW)
    PCHAR2(32, '>',  S_brdnladder, "branch ladder down", "branch ladder down", "分支下行梯子", CLR_YELLOW)
    /* end cmap A */
    PCHAR2(33, '_',  S_altar,  "altar", "altar", "祭坛", CLR_GRAY)
    /* start cmap B */
    PCHAR2(34, '|',  S_grave,  "grave", "grave", "坟墓", CLR_WHITE)
    PCHAR2(35, '\\', S_throne, "throne", "opulent throne", "华丽的宝座", HI_GOLD)
    PCHAR2(36, '{',  S_sink,   "sink", "sink", "水槽", CLR_WHITE)
    PCHAR2(37, '{',  S_fountain, "fountain", "fountain", "喷泉", CLR_BRIGHT_BLUE)
    /* the S_pool symbol is used for both POOL terrain and MOAT terrain */
    PCHAR2(38, '}',  S_pool,   "pool", "water", "水", CLR_BLUE)
    PCHAR2(39, '.',  S_ice,    "ice", "ice", "冰", CLR_CYAN)
    PCHAR2(40, '}',  S_lava,   "molten lava", "molten lava", "熔岩", CLR_RED)
    PCHAR2(41, '}',  S_lavawall,  "wall of lava", "wall of lava", "熔岩墙", CLR_ORANGE)
    PCHAR2(42, '.',  S_vodbridge, "vertical open drawbridge", "lowered drawbridge", "放下的吊桥", CLR_BROWN)
    PCHAR2(43, '.',  S_hodbridge, "horizontal open drawbridge", "lowered drawbridge", "放下的吊桥", CLR_BROWN)
    PCHAR2(44, '#',  S_vcdbridge, "vertical closed drawbridge", "raised drawbridge", "升起的吊桥", CLR_BROWN)
    PCHAR2(45, '#',  S_hcdbridge, "horizontal closed drawbridge", "raised drawbridge", "升起的吊桥", CLR_BROWN)
    PCHAR2(46, ' ',  S_air,    "air", "air", "空气", CLR_CYAN)
    PCHAR2(47, '#',  S_cloud,  "cloud", "cloud", "云", CLR_GRAY)
    /* the S_water symbol is used for WATER terrain: wall of water in the
       dungeon and Plane of Water in the endgame */
    PCHAR2(48, '}',  S_water,  "water", "water", "水", CLR_BRIGHT_BLUE)
    /* end dungeon characters                                          */
    /*                                                                 */
    /* begin traps                                                     */
    /*                                                                 */
    PCHAR2(49, '^',  S_arrow_trap, "arrow trap", "arrow trap", "箭矢陷阱", HI_METAL)
    PCHAR2(50, '^',  S_dart_trap, "dart trap", "dart trap", "飞镖陷阱", HI_METAL)
    PCHAR2(51, '^',  S_falling_rock_trap, "falling rock trap", "falling rock trap", "落石陷阱", CLR_GRAY)
    PCHAR2(52, '^',  S_squeaky_board, "squeaky board", "squeaky board", "嘎吱作响的木板", CLR_BROWN)
    PCHAR2(53, '^',  S_bear_trap, "bear trap", "bear trap", "捕兽夹", HI_METAL)
    PCHAR2(54, '^',  S_land_mine, "land mine", "land mine", "地雷", CLR_RED)
    PCHAR2(55, '^',  S_rolling_boulder_trap, "rolling boulder trap", "rolling boulder trap", "滚石陷阱", CLR_GRAY)
    PCHAR2(56, '^',  S_sleeping_gas_trap, "sleeping gas trap", "sleeping gas trap", "催眠气体陷阱", HI_ZAP)
    PCHAR2(57, '^',  S_rust_trap, "rust trap", "rust trap", "锈蚀陷阱", CLR_BLUE)
    PCHAR2(58, '^',  S_fire_trap, "fire trap", "fire trap", "火焰陷阱", CLR_ORANGE)
    PCHAR2(59, '^',  S_pit, "pit", "pit", "深坑", CLR_BLACK)
    PCHAR2(60, '^',  S_spiked_pit, "spiked pit", "spiked pit", "有刺的坑", CLR_BLACK)
    PCHAR2(61, '^',  S_hole, "hole", "hole", "洞穴", CLR_BROWN)
    PCHAR2(62, '^',  S_trap_door, "trap door", "trap door", "陷阱门", CLR_BROWN)
    PCHAR2(63, '^',  S_teleportation_trap, "teleportation trap", "teleportation trap", "传送陷阱", CLR_MAGENTA)
    PCHAR2(64, '^',  S_level_teleporter, "level teleporter", "level teleporter", "层级传送器", CLR_MAGENTA)
    PCHAR2(65, '^',  S_magic_portal, "magic portal", "magic portal", "魔法传送门", CLR_BRIGHT_MAGENTA)
    PCHAR2(66, '"',  S_web, "web", "web", "蜘蛛网", CLR_GRAY)
    PCHAR2(67, '^',  S_statue_trap, "statue trap", "statue trap", "雕像陷阱", CLR_GRAY)
    PCHAR2(68, '^',  S_magic_trap, "magic trap", "magic trap", "魔法陷阱", HI_ZAP)
    PCHAR2(69, '^',  S_anti_magic_trap, "anti magic trap", "anti-magic field", "反魔法力场",
                                        HI_ZAP)
    PCHAR2(70, '^',  S_polymorph_trap, "polymorph trap", "polymorph trap", "变形陷阱", CLR_BRIGHT_GREEN)
    PCHAR2(71, '~',  S_vibrating_square, "vibrating square", "vibrating square", "振动方格", CLR_MAGENTA)
    PCHAR2(72, '^',  S_trapped_door, "trapped door", "trapped door", "带陷阱的门", CLR_ORANGE)
    PCHAR2(73, '^',  S_trapped_chest, "trapped chest", "trapped chest", "带陷阱的箱子", CLR_ORANGE)
    /* end traps                                                       */
    /* end cmap B */
    /*                                                                   */
    /* begin special effects                                             */
    /*                                                                   */
    /* zap colors are changed by reset_glyphmap() to match type of beam */
    /*                                                                   */
    PCHAR2(74, '|',  S_vbeam, "vertical beam", "", "", CLR_GRAY)
    PCHAR2(75, '-',  S_hbeam, "horizontal beam", "", "", CLR_GRAY)
    PCHAR2(76, '\\', S_lslant, "left slant beam", "", "", CLR_GRAY)
    PCHAR2(77, '/',  S_rslant, "right slant beam", "", "", CLR_GRAY)
    /* start cmap C */
    PCHAR2(78, '*',  S_digbeam, "dig beam", "", "", CLR_WHITE)
    PCHAR2(79, '!',  S_flashbeam, "flash beam", "", "", CLR_WHITE)
    PCHAR2(80, ')',  S_boomleft, "boom left", "", "", HI_WOOD)
    PCHAR2(81, '(',  S_boomright, "boom right", "", "", HI_WOOD)
    /* 4 magic shield symbols                                          */
    PCHAR2(82, '0',  S_ss1, "shield1", "", "", HI_ZAP)
    PCHAR2(83, '#',  S_ss2, "shield2", "", "", HI_ZAP)
    PCHAR2(84, '@',  S_ss3, "shield3", "", "", HI_ZAP)
    PCHAR2(85, '*',  S_ss4, "shield4", "", "", HI_ZAP)
    PCHAR2(86, '#',  S_poisoncloud, "poison cloud", "poison cloud", "毒云", CLR_BRIGHT_GREEN)
    /* for a time S_goodpos was a question mark, but dollar sign is the
       default keystroke for getpos() to toggle goodpos glyphs on or off */
    PCHAR2(87, '$',  S_goodpos, "valid position", "valid position", "有效位置", HI_ZAP)
    /* end cmap C */
    /*                                                             */
    /* The 8 swallow symbols.  Do NOT separate.                    */
    /* To change order or add, see the function swallow_to_glyph() */
    /* in display.c. swallow colors are changed by                 */
    /* reset_glyphmap() to match the engulfing monst.              */
    /*                                                             */
    /*  Order:                                                     */
    /*                                                             */
    /*      1 2 3                                                  */
    /*      4 5 6                                                  */
    /*      7 8 9                                                  */
    /*                                                             */
    PCHAR2(88, '/',  S_sw_tl, "swallow top left", "", "", CLR_GREEN)      /*1*/
    PCHAR2(89, '-',  S_sw_tc, "swallow top center", "", "", CLR_GREEN)    /*2*/
    PCHAR2(90, '\\', S_sw_tr, "swallow top right", "", "", CLR_GREEN)     /*3*/
    PCHAR2(91, '|',  S_sw_ml, "swallow middle left", "", "", CLR_GREEN)   /*4*/
    PCHAR2(92, '|',  S_sw_mr, "swallow middle right", "", "", CLR_GREEN)  /*6*/
    PCHAR2(93, '\\', S_sw_bl, "swallow bottom left", "", "", CLR_GREEN)   /*7*/
    PCHAR2(94, '-',  S_sw_bc, "swallow bottom center", "", "", CLR_GREEN) /*8*/
    PCHAR2(95, '/',  S_sw_br, "swallow bottom right", "", "", CLR_GREEN)  /*9*/
    /*                                                             */
    /* explosion colors are changed by reset_glyphmap() to match   */
    /* the type of expl.                                           */
    /*                                                             */
    /*    Ex.                                                      */
    /*                                                             */
    /*      /-\                                                    */
    /*      |@|                                                    */
    /*      \-/                                                    */
    /*                                                             */
    PCHAR2(96, '/',  S_expl_tl, "explosion top left", "", "", CLR_ORANGE)
    PCHAR2(97, '-',  S_expl_tc, "explosion top center", "", "", CLR_ORANGE)
    PCHAR2(98, '\\', S_expl_tr, "explosion top right", "", "", CLR_ORANGE)
    PCHAR2(99, '|',  S_expl_ml, "explosion middle left", "", "", CLR_ORANGE)
    PCHAR2(100, ' ',  S_expl_mc, "explosion middle center", "", "", CLR_ORANGE)
    PCHAR2(101, '|',  S_expl_mr, "explosion middle right", "", "", CLR_ORANGE)
    PCHAR2(102, '\\', S_expl_bl, "explosion bottom left", "", "", CLR_ORANGE)
    PCHAR2(103, '-', S_expl_bc, "explosion bottom center", "", "", CLR_ORANGE)
    PCHAR2(104, '/', S_expl_br, "explosion bottom right", "", "", CLR_ORANGE)
#undef PCHAR
#undef PCHAR2
#endif /* PCHAR_S_ENUM || PCHAR_PARSE || PCHAR_DRAWING || PCHAR_TILES
        * || DUMP_ENUMS_PCHAR */

#if defined(MONSYMS_S_ENUM)                     \
    || defined(MONSYMS_DEFCHAR_ENUM)            \
    || defined(MONSYMS_PARSE)                   \
    || defined(MONSYMS_DRAWING)                 \
    || defined(DUMP_ENUMS_MONSYMS)              \
    || defined(DUMP_ENUMS_MONSYMS_DEFCHAR)

/*
    MONSYM(idx, ch, sym desc)
        idx:     index used in enum
        ch:      character symbol
        sym:     symbol name for parsing purposes
        desc:    description
*/

#if defined(MONSYMS_S_ENUM)
/* sym.h */
#define MONSYM(idx, ch, basename, sym, desc) sym = idx,

#elif defined(MONSYMS_DEFCHAR_ENUM)
/* sym.h */
#define MONSYM(idx, ch, basename, sym,  desc) DEF_##basename = ch,

#elif defined(MONSYMS_PARSE)
/* symbols.c */
#define MONSYM(idx, ch, basename, sym, desc) \
    { SYM_MON, sym + SYM_OFF_M, #sym },

#elif defined(MONSYMS_DRAWING)
/* drawing.c */
#define MONSYM(idx, ch, basename, sym, desc) { DEF_##basename, "", desc },

/* allmain.c */
#elif defined(DUMP_ENUMS_MONSYMS)
#define MONSYM(idx, ch, basename, sym, desc) { sym, #sym },

#elif defined(DUMP_ENUMS_MONSYMS_DEFCHAR)
#define MONSYM(idx, ch, basename, sym, desc) \
    { DEF_##basename, "DEF_" #basename },

#endif

    MONSYM( 1, 'a', ANT, S_ANT,   "蚂蚁或其他昆虫")
    MONSYM( 2, 'b', BLOB, S_BLOB, "黏液怪")
    MONSYM( 3, 'c', COCKATRICE, S_COCKATRICE, "鸡蛇")
    MONSYM( 4, 'd', DOG, S_DOG, "狗或其他犬科动物")
    MONSYM( 5, 'e', EYE, S_EYE, "眼睛或球体")
    MONSYM( 6, 'f', FELINE, S_FELINE, "猫或其他猫科动物")
    MONSYM( 7, 'g', GREMLIN, S_GREMLIN, "小鬼")
    /* small humanoids: hobbit, dwarf */
    MONSYM( 8, 'h', HUMANOID, S_HUMANOID, "类人生物")
    /* minor demons */
    MONSYM( 9, 'i', IMP, S_IMP, "小恶魔或低级恶魔")
    MONSYM(10, 'j', JELLY, S_JELLY, "凝胶怪")
    MONSYM(11, 'k', KOBOLD, S_KOBOLD, "科博尔德")
    MONSYM(12, 'l', LEPRECHAUN, S_LEPRECHAUN, "小矮妖")
    MONSYM(13, 'm', MIMIC, S_MIMIC, "拟形怪")
    MONSYM(14, 'n', NYMPH, S_NYMPH, "仙女")
    MONSYM(15, 'o', ORC, S_ORC, "兽人")
    MONSYM(16, 'p', PIERCER, S_PIERCER, "锥子")
    /* quadruped excludes horses */
    MONSYM(17, 'q', QUADRUPED, S_QUADRUPED, "四足动物")
    MONSYM(18, 'r', RODENT, S_RODENT, "啮齿动物")
    MONSYM(19, 's', SPIDER, S_SPIDER, "蛛形类或蜈蚣")
    MONSYM(20, 't', TRAPPER, S_TRAPPER, "诱陷者或蛰伏怪")
    /* unicorn, horses */
    MONSYM(21, 'u', UNICORN, S_UNICORN, "独角兽或马")
    MONSYM(22, 'v', VORTEX, S_VORTEX, "漩涡")
    MONSYM(23, 'w', WORM, S_WORM, "蠕虫")
    MONSYM(24, 'x', XAN, S_XAN, "玄蚊或其他神话/奇幻昆虫")
    /* yellow light, black light */
    MONSYM(25, 'y', LIGHT, S_LIGHT, "光")
    MONSYM(26, 'z', ZRUTY, S_ZRUTY, "山区巨人")
    MONSYM(27, 'A', ANGEL, S_ANGEL, "天使类生物")
    MONSYM(28, 'B', BAT, S_BAT, "蝙蝠或鸟类")
    MONSYM(29, 'C', CENTAUR, S_CENTAUR, "半人马")
    MONSYM(30, 'D', DRAGON, S_DRAGON, "龙")
    /* elemental includes invisible stalker */
    MONSYM(31, 'E', ELEMENTAL, S_ELEMENTAL, "元素生物")
    MONSYM(32, 'F', FUNGUS, S_FUNGUS, "真菌或霉菌")
    MONSYM(33, 'G', GNOME, S_GNOME, "侏儒")
    /* large humanoid: giant, ettin, minotaur */
    MONSYM(34, 'H', GIANT, S_GIANT, "巨型类人生物")
    MONSYM(35, 'I', INVISIBLE, S_invisible, "隐形怪物")
    MONSYM(36, 'J', JABBERWOCK, S_JABBERWOCK, "贾巴沃克")
    MONSYM(37, 'K', KOP, S_KOP, "吉斯通警察")
    MONSYM(38, 'L', LICH, S_LICH, "巫妖")
    MONSYM(39, 'M', MUMMY, S_MUMMY, "木乃伊")
    MONSYM(40, 'N', NAGA, S_NAGA, "纳迦")
    MONSYM(41, 'O', OGRE, S_OGRE, "食人魔")
    MONSYM(42, 'P', PUDDING, S_PUDDING, "布丁或黏液")
    MONSYM(43, 'Q', QUANTMECH, S_QUANTMECH, "量子工程师")
    MONSYM(44, 'R', RUSTMONST, S_RUSTMONST, "锈蚀怪或祛魔怪")
    MONSYM(45, 'S', SNAKE, S_SNAKE, "蛇")
    MONSYM(46, 'T', TROLL, S_TROLL, "巨魔")
    /* umber hulk */
    MONSYM(47, 'U', UMBER, S_UMBER, "土巨怪")
    MONSYM(48, 'V', VAMPIRE, S_VAMPIRE, "吸血鬼")
    MONSYM(49, 'W', WRAITH, S_WRAITH, "幽灵")
    MONSYM(50, 'X', XORN, S_XORN, "索尔石怪")
    /* apelike creature includes owlbear, monkey */
    MONSYM(51, 'Y', YETI, S_YETI, "类猿生物")
    MONSYM(52, 'Z', ZOMBIE, S_ZOMBIE, "僵尸")
    MONSYM(53, '@', HUMAN, S_HUMAN, "人类或精灵")
    /* space symbol*/
    MONSYM(54, ' ', GHOST, S_GHOST, "幽灵")
    MONSYM(55, '\'', GOLEM, S_GOLEM, "傀儡")
    MONSYM(56, '&', DEMON, S_DEMON, "大恶魔")
    /* fish */
    MONSYM(57, ';', EEL, S_EEL,  "海怪")
    /* reptiles */
    MONSYM(58, ':', LIZARD, S_LIZARD, "蜥蜴")
    MONSYM(59, '~', WORM_TAIL, S_WORM_TAIL, "长尾蠕虫")
    MONSYM(60, ']', MIMIC_DEF, S_MIMIC_DEF, "拟形怪")

#undef MONSYM
#endif /* MONSYMS_S_ENUM || MONSYMS_DEFCHAR_ENUM || MONSYMS_PARSE
        * || MONSYMS_DRAWING || DUMP_ENUMS_MONSYMS)
        * || DUMP_ENUMS_MONSYMS_DEFCHAR */

#if defined(OBJCLASS_S_ENUM)                    \
    || defined(OBJCLASS_DEFCHAR_ENUM)           \
    || defined(OBJCLASS_CLASS_ENUM)             \
    || defined(OBJCLASS_PARSE)                  \
    || defined(OBJCLASS_DRAWING)                \
    || defined(DUMP_ENUMS_OBJCLASS_DEFCHARS)    \
    || defined(DUMP_ENUMS_OBJCLASS_CLASSES)     \
    || defined(DUMP_ENUMS_OBJCLASS_SYMS)

/*
    OBJCLASS(idx, ch, basename, sym, name, explain)
        idx:      index used in enum
        ch:       default character
        basename: unadorned base name of objclass, used
                  to construct enums through suffixes/prefixes
        sym:      symbol name for enum and parsing purposes
        name:     used in object_detect()
        explain:  used in do_look()

    OBJCLASS2(idx, ch, basename, sname, sym, name, explain)
        idx:      index used in enum
        ch:       default character
        basename: unadorned base name of objclass, used
                  to construct enums through suffixes/prefixes
        sname:    hardcoded *_SYM value for this entry (required
                  only because basename and GOLD_SYM differ
        sym:      symbol name for enum and parsing purposes
        name:     used in object_detect()
        explain:  used in do_look()
*/

#if defined(OBJCLASS_CLASS_ENUM)
/* objclass.h */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    basename##_CLASS = idx,

#elif defined(OBJCLASS_DEFCHAR_ENUM)
/* objclass.h */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    basename##_SYM = ch,

#elif defined(OBJCLASS_S_ENUM)
/* objclass.h */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    sym = idx,

#elif defined(OBJCLASS_PARSE)
/* symbols.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { SYM_OC, sym + SYM_OFF_O, #sym },

#elif defined(OBJCLASS_DRAWING)
/* drawing.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { basename##_SYM, name, explain },

#elif defined(DUMP_ENUMS_OBJCLASS_DEFCHARS)
/* allmain.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { basename##_SYM, #basename "_SYM" },

#elif defined(DUMP_ENUMS_OBJCLASS_CLASSES)
/* allmain.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { basename##_CLASS, #basename "_CLASS" },

#elif defined(DUMP_ENUMS_OBJCLASS_SYMS)
/* allmain.c */
#define OBJCLASS(idx, ch, basename, sym, name, explain) \
    { sym , #sym },
#endif

/* OBJCLASS with extra arg */
#if defined(OBJCLASS_DEFCHAR_ENUM)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    sname = ch,
#elif defined(OBJCLASS_DRAWING)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { sname, name, explain },
#elif defined(DUMP_ENUMS_OBJCLASS_DEFCHARS)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { sname, #sname },
#elif defined(DUMP_ENUMS_OBJCLASS_CLASSES)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { basename##_CLASS, #basename "_CLASS" },
#elif defined(DUMP_ENUMS_OBJCLASS_SYMS)
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    { sym , #sym },
#else
#define OBJCLASS2(idx, ch, basename, sname, sym, name, explain) \
    OBJCLASS(idx, ch, basename, sym, name, explain)
#endif

    OBJCLASS( 1,  ']', ILLOBJ, S_strange_obj, "illegal objects", "奇怪的物品")
    OBJCLASS( 2,  ')', WEAPON, S_weapon, "weapons", "武器")
    OBJCLASS( 3,  '[', ARMOR,  S_armor, "armor", "盔甲或护甲部件")
    OBJCLASS( 4,  '=', RING,   S_ring, "rings", "戒指")
    OBJCLASS( 5,  '"', AMULET, S_amulet, "amulets", "护身符")
    OBJCLASS( 6,  '(', TOOL,   S_tool, "tools", "有用的东西(镐,钥匙,提灯...)")
    OBJCLASS( 7,  '%', FOOD,   S_food, "food", "食物")
    OBJCLASS( 8,  '!', POTION, S_potion, "potions", "药水")
    OBJCLASS( 9,  '?', SCROLL, S_scroll, "scrolls", "卷轴")
    OBJCLASS(10,  '+', SPBOOK, S_book, "spellbooks", "魔法书")
    OBJCLASS(11,  '/', WAND,   S_wand, "wands", "魔杖")
    OBJCLASS2(12, '$', COIN,   GOLD_SYM, S_coin, "coins", "一堆金币")
    OBJCLASS(13,  '*', GEM,    S_gem, "rocks", "宝石或岩石")
    OBJCLASS(14,  '`', ROCK,   S_rock, "large stones", "巨石或雕像")
    OBJCLASS(15,  '0', BALL,   S_ball, "iron balls", "铁球")
    OBJCLASS(16,  '_', CHAIN,  S_chain, "chains", "铁链")
    OBJCLASS(17,  '.', VENOM,  S_venom, "venoms", "一滩毒液")

#undef OBJCLASS
#undef OBJCLASS2
#endif /* OBJCLASS_S_ENUM || OBJCLASS_DEFCHAR_ENUM || OBJCLASS_CLASS_ENUM
        * || OBJCLASS_PARSE || OBJCLASS_DRAWING
        * || DUMP_ENUMS_OBJCLASS_DEFCHARS || DUMP_ENUMS_OBJCLASS_CLASSES
        * || DUMP_ENUMS_OBJCLASS_SYMS */

#ifdef DEBUG
#if !defined(PCHAR_S_ENUM) && !defined(PCHAR_DRAWING) \
    && !defined(PCHAR_PARSE) && !defined(PCHAR_TILES) \
    && !defined(DUMP_ENUMS_PCHAR) \
    && !defined(MONSYMS_S_ENUM) && !defined(MONSYMS_DEFCHAR_ENUM) \
    && !defined(MONSYMS_PARSE) && !defined(MONSYMS_DRAWING) \
    && !defined(DUMP_ENUMS_MONSYMS) \
    && !defined(DUMP_ENUMS_MONSYMS_DEFCHAR) \
    && !defined(OBJCLASS_S_ENUM) && !defined(OBJCLASS_DEFCHAR_ENUM) \
    && !defined(OBJCLASS_CLASS_ENUM) && !defined(OBJCLASS_PARSE) \
    && !defined (OBJCLASS_DRAWING) \
    && !defined(DUMP_ENUMS_OBJCLASS_DEFCHARS) \
    && !defined(DUMP_ENUMS_OBJCLASS_CLASSES) \
    && !defined(DUMP_ENUMS_OBJCLASS_SYMS)
#error Non-productive inclusion of defsym.h
#endif
#endif /* DEBUG */

/* end of defsym.h */
