/* NetHack 5.0	objects.h	$NHDT-Date: 1749097644 2025/06/04 20:27:24 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.30 $ */
/* Copyright (c) Mike Threepoint, 1989.                           */
/* NetHack may be freely redistributed.  See license for details. */

/*
   The data in this file is processed multiple times by its inclusion
   in several places in the code. The results depend on the definition
   of the following:
     OBJECTS_ENUM        to construct object onames enum entries (decl.h).
     OBJECTS_DESCR_INIT to construct obj_descr[] array entries (objects.c).
     OBJECTS_INIT       to construct objects[] array entries (objects.c).
*/

#ifndef NoDes
#define NoDes (char *) 0 /* less visual distraction for 'no description' */
#endif

#ifndef lint
#define HARDGEM(n) (n >= 8)
#else
#define HARDGEM(n) (0)
#endif

/*
 * Note...
 *  OBJECTS() currently has 15 parameters; it more become needed, some
 *  will need to be combined the way BITS() is used, because compilers
 *  are allowed to impose a limit of 15.
 */

#if defined(OBJECTS_DESCR_INIT)
#define OBJ(ename,edesc,name,desc)  ename, edesc, name, desc
#define OBJECT(obj,bits,prp,sym,prob,dly,wt, \
               cost,sdam,ldam,oc1,oc2,nut,color,sn)  { obj }
#define MARKER(tag,sn) /*empty*/

#elif defined(OBJECTS_INIT)
/* notes: 'sub' was once a bitfield but got changed to separate schar when
   it was overloaded to hold negative weapon skill indices; the first zero
   is padding for oc_prediscovered which has variable init at run-time;
   the second zero is oc_spare1 for padding between oc_tough and oc_dir */
#define BITS(nmkn,mrg,uskn,ctnr,mgc,chrg,uniq,nwsh,big,tuf,dir,sub,mtrl) \
    nmkn,mrg,uskn,0,mgc,chrg,uniq,nwsh,big,tuf,0,dir,mtrl,sub /*cpp fodder*/
/* note: 0UL-1UL is a method of expressing the largest possible
   unsigned long value whilst working around a false-positive warning
   in Microsoft Visual C (which assumes that a negative number was
   intended despite the explicit U suffix) */
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
  { 0, 0, (char *) 0, bits, prp, sym, dly, color, prob, wt, \
    cost, sdam, ldam, oc1, oc2, nut, (0UL-1UL), 0, (0UL-1UL), 0 }
#define MARKER(tag,sn) /*empty*/

#elif defined(OBJECTS_ENUM)
#define OBJ(ename,edesc,name,desc)
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
    sn
#define MARKER(tag,sn) tag = sn,

#elif defined(DUMP_ENUMS)
#define OBJ(ename,edesc,name,desc)
#define OBJECT(obj,bits,prp,sym,prob,dly,wt,        \
               cost,sdam,ldam,oc1,oc2,nut,color,sn) \
  { sn, #sn }
#define MARKER(tag,sn) /*empty*/

#else
#error Unproductive inclusion of objects.h
#endif  /* OBJECTS_DESCR_INIT || OBJECTS_INIT || OBJECTS_ENUM */

#define GENERIC(edesc, desc, class, gen_enum) \
    OBJECT(OBJ("generic " edesc, edesc, "通用" desc, desc),                                  \
           BITS(0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, 0),            \
           0, class, 0, 0, 0, 0, 0, 0, 0, 0, 0, CLR_GRAY, gen_enum)

/* dummy object[0] -- description [2nd arg] *must* be NULL */
OBJECT(OBJ("strange object", NoDes, "奇怪的物体", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0),
       0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, STRANGE_OBJECT),
/* slots [1] through [MAXOCLASSES-1] are indexed by class; some are
   used for display purposes, most aren't used; none are actual objects;
   note that 'real' strange object is in slot [0] but ILLOBJ_CLASS is 1
   so we add a dummy for it in slot [1] to simplify accessing the rest;
   there isn't any entry for RANDOM_CLASS (0) */
GENERIC("strange",    "奇怪"  , ILLOBJ_CLASS,  GENERIC_ILLOBJ),  /* [1] */
GENERIC("weapon",     "武器"  , WEAPON_CLASS,  GENERIC_WEAPON),  /* [2] */
GENERIC("armor",      "盔甲"  , ARMOR_CLASS,   GENERIC_ARMOR),   /* [3] */
GENERIC("ring",       "戒指"  , RING_CLASS,    GENERIC_RING),    /* [4] */
GENERIC("amulet",     "护符"  , AMULET_CLASS,  GENERIC_AMULET),  /* [5] */
GENERIC("tool",       "工具"  , TOOL_CLASS,    GENERIC_TOOL),    /* [6] */
GENERIC("food",       "食物"  , FOOD_CLASS,    GENERIC_FOOD),    /* [7] */
GENERIC("potion",     "药水"  , POTION_CLASS,  GENERIC_POTION),  /* [8] */
GENERIC("scroll",     "卷轴"  , SCROLL_CLASS,  GENERIC_SCROLL),  /* [9] */
GENERIC("spellbook",  "魔法书", SPBOOK_CLASS,  GENERIC_SPBOOK),  /* [10] */
GENERIC("wand",       "魔杖"  , WAND_CLASS,    GENERIC_WAND),    /* [11] */
GENERIC("coin",       "金币"  , COIN_CLASS,    GENERIC_COIN),    /* [12] */
GENERIC("gem",        "宝石"  , GEM_CLASS,     GENERIC_GEM),     /* [13] */
GENERIC("large rock", "大石头", ROCK_CLASS,    GENERIC_ROCK),    /* [14] bldr+statue */
GENERIC("iron ball",  "铁球"  , BALL_CLASS,    GENERIC_BALL),    /* [15] */
GENERIC("iron chain", "铁链"  , CHAIN_CLASS,   GENERIC_CHAIN),   /* [16] */
GENERIC("venom",      "毒液"  , VENOM_CLASS,   GENERIC_VENOM),   /* [17] */
#undef GENERIC
/* FIRST_OBJECT: it would be simpler just to use MARKER(FIRST_OBJECT,ARROW)
   below but that is vulnerable to neglecting to update the marker enum
   after inserting something in front of arrow */
MARKER(LAST_GENERIC, GENERIC_VENOM)
MARKER(FIRST_OBJECT, LAST_GENERIC + 1)
/* this definition of FIRST_OBJECT advances the default value for next enum;
   backtrack to fix that, otherwise ARROW and the rest would be off by 1 */
MARKER(OBJCLASS_HACK, FIRST_OBJECT - 1)

/* weapons ... */
#define WEAPON(ename,edesc,name,desc,kn,mg,bi,prob,wt,                          \
               cost,sdam,ldam,hitbon,typ,sub,metal,color,sn)        \
    OBJECT(OBJ(ename,edesc,name,desc),                                          \
           BITS(kn, mg, 1, 0, 0, 1, 0, 0, bi, 0, typ, sub, metal),  \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color,sn)
#define PROJECTILE(ename,edesc,name,desc,kn,prob,wt,                            \
                   cost,sdam,ldam,hitbon,metal,sub,color,sn)        \
    OBJECT(OBJ(ename,edesc,name,desc),                                          \
           BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, PIERCE, sub, metal), \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, sdam, ldam, hitbon, 0, wt, color, sn)
#define BOW(ename,edesc,name,desc,kn,prob,wt,cost,hitbon,metal,sub,color,sn)    \
    OBJECT(OBJ(ename,edesc,name,desc),                                          \
           BITS(kn, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, sub, metal),      \
           0, WEAPON_CLASS, prob, 0, wt,                            \
           cost, 2, 2, hitbon, 0, wt, color, sn)

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
   the extra damage is added on in weapon.c, not here! */

/* weapon strike mode overloads the oc_dir field */
#define P PIERCE
#define S SLASH
#define B WHACK

/* missiles; materiel reflects the arrowhead, not the shaft */
PROJECTILE("arrow", NoDes, "箭", NoDes,
           1, 55, 1, 2, 6, 6, 0,        IRON, -P_BOW, HI_METAL,
                                                        ARROW),
PROJECTILE("elven arrow", "runed arrow", "精灵箭", "符文箭",
           0, 20, 1, 2, 7, 6, 0,        WOOD, -P_BOW, HI_WOOD,
                                                        ELVEN_ARROW),
PROJECTILE("orcish arrow", "crude arrow", "兽人箭", "粗糙箭",
           0, 20, 1, 2, 5, 6, 0,        IRON, -P_BOW, CLR_BLACK,
                                                        ORCISH_ARROW),
PROJECTILE("silver arrow", NoDes, "银箭", NoDes,
           1, 12, 1, 5, 6, 6, 0,        SILVER, -P_BOW, HI_SILVER,
                                                        SILVER_ARROW),
PROJECTILE("ya", "bamboo arrow", "矢", "竹箭",
           0, 15, 1, 4, 7, 7, 1,        METAL, -P_BOW, HI_METAL, YA),
PROJECTILE("crossbow bolt", NoDes, "弩箭", NoDes,
           1, 55, 1, 2, 4, 6, 0,        IRON, -P_CROSSBOW, HI_METAL,
                                                        CROSSBOW_BOLT),

/* missiles that don't use a launcher */
WEAPON("dart", NoDes, "飞镖", NoDes,
       1, 1, 0, 60,   1,   2,  3,  2, 0, P,   -P_DART, IRON, HI_METAL,
                                                        DART),
WEAPON("shuriken", "throwing star", "手里剑", "投掷镖",
       0, 1, 0, 35,   1,   5,  8,  6, 2, P,   -P_SHURIKEN, IRON, HI_METAL,
                                                        SHURIKEN),
WEAPON("boomerang", NoDes, "回旋镖", NoDes,
       1, 1, 0, 15,   5,  20,  9,  9, 0, 0,   -P_BOOMERANG, WOOD, HI_WOOD,
                                                        BOOMERANG),

/* spears [note: javelin used to have a separate skill from spears,
   because the latter are primarily stabbing weapons rather than
   throwing ones; but for playability, they've been merged together
   under spear skill and spears can now be thrown like javelins] */
WEAPON("spear", NoDes, "矛", NoDes,
       1, 1, 0, 50,  30,   3,  6,  8, 0, P,   P_SPEAR, IRON, HI_METAL,
                                                        SPEAR),
WEAPON("elven spear", "runed spear", "精灵矛", "符文矛",
       0, 1, 0, 10,  30,   3,  7,  8, 0, P,   P_SPEAR, WOOD, HI_WOOD,
                                                        ELVEN_SPEAR),
WEAPON("orcish spear", "crude spear", "兽人矛", "粗糙矛",
       0, 1, 0, 13,  30,   3,  5,  8, 0, P,   P_SPEAR, IRON, CLR_BLACK,
                                                        ORCISH_SPEAR),
WEAPON("dwarvish spear", "stout spear", "矮人矛", "结实矛",
       0, 1, 0, 12,  35,   3,  8,  8, 0, P,   P_SPEAR, IRON, HI_METAL,
                                                        DWARVISH_SPEAR),
WEAPON("silver spear", NoDes, "银矛", NoDes,
       1, 1, 0,  2,  36,  40,  6,  8, 0, P,   P_SPEAR, SILVER, HI_SILVER,
                                                        SILVER_SPEAR),
WEAPON("javelin", "throwing spear", "标枪", "投掷矛",
       0, 1, 0, 10,  20,   3,  6,  6, 0, P,   P_SPEAR, IRON, HI_METAL,
                                                        JAVELIN),

/* spearish; doesn't stack, not intended to be thrown */
WEAPON("trident", NoDes, "三叉矛", NoDes,
       1, 0, 0,  8,  25,   5,  6,  4, 0, P,   P_TRIDENT, IRON, HI_METAL,
                                                        TRIDENT),
        /* +1 small, +2d4 large */

/* blades; all stack */
WEAPON("dagger", NoDes, "匕首", NoDes,
       1, 1, 0, 30,  10,   4,  4,  3, 2, P,   P_DAGGER, IRON, HI_METAL,
                                                        DAGGER),
WEAPON("elven dagger", "runed dagger", "精灵匕首", "符文匕首",
       0, 1, 0, 10,  10,   4,  5,  3, 2, P,   P_DAGGER, WOOD, HI_WOOD,
                                                        ELVEN_DAGGER),
WEAPON("orcish dagger", "crude dagger", "兽人匕首", "粗糙匕首",
       0, 1, 0, 12,  10,   4,  3,  3, 2, P,   P_DAGGER, IRON, CLR_BLACK,
                                                        ORCISH_DAGGER),
WEAPON("silver dagger", NoDes, "银匕首", NoDes,
       1, 1, 0,  3,  12,  40,  4,  3, 2, P,   P_DAGGER, SILVER, HI_SILVER,
                                                        SILVER_DAGGER),
WEAPON("athame", NoDes, "仪式刀", NoDes,
       1, 1, 0,  0,  10,   4,  4,  3, 2, S,   P_DAGGER, IRON, HI_METAL,
                                                        ATHAME),
WEAPON("scalpel", NoDes, "手术刀", NoDes,
       1, 1, 0,  0,   5,   6,  3,  3, 2, S,   P_KNIFE, METAL, HI_METAL,
                                                        SCALPEL),
WEAPON("knife", NoDes, "小刀", NoDes,
       1, 1, 0, 20,   5,   4,  3,  2, 0, P|S, P_KNIFE, IRON, HI_METAL,
                                                        KNIFE),
WEAPON("stiletto", NoDes, "小剑", NoDes,
       1, 1, 0,  5,   5,   4,  3,  2, 0, P|S, P_KNIFE, IRON, HI_METAL,
                                                        STILETTO),
/* 3.6: worm teeth and crysknives now stack;
   when a stack of teeth is enchanted at once, they fuse into one crysknife;
   when a stack of crysknives drops, the whole stack reverts to teeth */
/* 5.0: change crysknife from MINERAL to BONE and worm tooth from 0 to BONE */
WEAPON("worm tooth", NoDes, "蠕虫齿", NoDes,
       1, 1, 0,  0,  20,   2,  2,  2, 0, 0,   P_KNIFE, BONE, CLR_WHITE,
                                                        WORM_TOOTH),
WEAPON("crysknife", NoDes, "迅捷小刀", NoDes,
       1, 1, 0,  0,  20, 100, 10, 10, 3, P,   P_KNIFE, BONE, CLR_WHITE,
                                                        CRYSKNIFE),

/* axes */
WEAPON("axe", NoDes, "斧头", NoDes,
       1, 0, 0, 40,  60,   8,  6,  4, 0, S,   P_AXE, IRON, HI_METAL,
                                                        AXE),
WEAPON("battle-axe", "double-headed axe", "战斧", "双头斧",      /* "double-bitted"? */
       0, 0, 1, 10, 120,  40,  8,  6, 0, S,   P_AXE, IRON, HI_METAL,
                                                        BATTLE_AXE),

/* swords */
WEAPON("short sword", NoDes, "短剑", NoDes,
       1, 0, 0,  8,  30,  10,  6,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL,
                                                        SHORT_SWORD),
WEAPON("elven short sword", "runed short sword", "精灵短剑", "符文短剑",
       0, 0, 0,  2,  30,  10,  8,  8, 0, P,   P_SHORT_SWORD, WOOD, HI_WOOD,
                                                        ELVEN_SHORT_SWORD),
WEAPON("orcish short sword", "crude short sword", "兽人短剑", "粗糙短剑", 
       0, 0, 0,  3,  30,  10,  5,  8, 0, P,   P_SHORT_SWORD, IRON, CLR_BLACK,
                                                        ORCISH_SHORT_SWORD),
WEAPON("dwarvish short sword", "broad short sword", "矮人短剑", "宽阔短剑",
       0, 0, 0,  2,  30,  10,  7,  8, 0, P,   P_SHORT_SWORD, IRON, HI_METAL,
                                                        DWARVISH_SHORT_SWORD),
WEAPON("scimitar", "curved sword", "短弯刀", "弯刀",
       0, 0, 0, 15,  40,  15,  8,  8, 0, S,   P_SABER, IRON, HI_METAL,
                                                        SCIMITAR),
WEAPON("silver saber", NoDes, "银剑", NoDes,
       1, 0, 0,  6,  40,  75,  8,  8, 0, S,   P_SABER, SILVER, HI_SILVER,
                                                        SILVER_SABER),
WEAPON("broadsword", NoDes, "阔剑", NoDes,
       1, 0, 0,  8,  70,  10,  4,  6, 0, S,   P_BROAD_SWORD, IRON, HI_METAL,
                                                        BROADSWORD),
        /* +d4 small, +1 large */
WEAPON("elven broadsword", "runed broadsword", "精灵阔剑", "符文阔剑",
       0, 0, 0,  4,  70,  10,  6,  6, 0, S,   P_BROAD_SWORD, WOOD, HI_WOOD,
                                                        ELVEN_BROADSWORD),
        /* +d4 small, +1 large */
WEAPON("long sword", NoDes, "长剑", NoDes,
       1, 0, 0, 50,  40,  15,  8, 12, 0, S,   P_LONG_SWORD, IRON, HI_METAL,
                                                        LONG_SWORD),
WEAPON("two-handed sword", NoDes, "双手剑", NoDes,
       1, 0, 1, 22, 150,  50, 12,  6, 0, S,   P_TWO_HANDED_SWORD,
                                                            IRON, HI_METAL,
                                                        TWO_HANDED_SWORD),
        /* +2d6 large */
WEAPON("katana", "samurai sword", "武士刀", "日本刀",
       0, 0, 0,  4,  40,  80, 10, 12, 1, S,   P_LONG_SWORD, IRON, HI_METAL,
                                                        KATANA),
/* special swords set up for artifacts */
WEAPON("tsurugi", "long samurai sword", "武士剑", "武士长剑",
       0, 0, 1,  0,  60, 500, 16,  8, 2, S,   P_TWO_HANDED_SWORD,
                                                            METAL, HI_METAL,
                                                        TSURUGI),
        /* +2d6 large */
WEAPON("runesword", "runed broadsword", "符文剑", "符文阔剑",
       0, 0, 0,  0,  40, 300,  4,  6, 0, S,   P_BROAD_SWORD, IRON, CLR_BLACK,
                                                        RUNESWORD),
        /* +d4 small, +1 large; Stormbringer: +5d2 +d8 from level drain */

/* polearms */
/* spear-type */
WEAPON("partisan", "vulgar polearm", "戟", "粗俗长柄武器",
       0, 0, 1,  5,  80,  10,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL,
                                                        PARTISAN),
        /* +1 large */
WEAPON("ranseur", "hilted polearm", "三叉戟", "大长柄武器",
       0, 0, 1,  5,  50,   6,  4,  4, 0, P,   P_POLEARMS, IRON, HI_METAL,
                                                        RANSEUR),
        /* +d4 both */
WEAPON("spetum", "forked polearm", "大战戟", "长柄叉",
       0, 0, 1,  5,  50,   5,  6,  6, 0, P,   P_POLEARMS, IRON, HI_METAL,
                                                        SPETUM),
        /* +1 small, +d6 large */
WEAPON("glaive", "single-edged polearm", "剑刃戟", "单刃长柄武器",
       0, 0, 1,  8,  75,   6,  6, 10, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        GLAIVE),
/* axe-type */
WEAPON("halberd", "angled poleaxe", "斧枪", "成角的战斧",
       0, 0, 1,  8, 150,  10, 10,  6, 0, P|S, P_POLEARMS, IRON, HI_METAL,
                                                        HALBERD),
        /* +1d6 large */
WEAPON("bardiche", "long poleaxe", "大战斧", "长战斧",
       0, 0, 1,  4, 120,   7,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        BARDICHE),
        /* +1d4 small, +2d4 large */
WEAPON("voulge", "pole cleaver", "长斧", "极切肉刀",
       0, 0, 1,  4, 125,   5,  4,  4, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        VOULGE),
        /* +d4 both */
/* curved/hooked */
WEAPON("fauchard", "pole sickle", "斩矛", "极镰刀", 
       0, 0, 1,  6,  60,   5,  6,  8, 0, P|S, P_POLEARMS, IRON, HI_METAL,
                                                        FAUCHARD),
WEAPON("guisarme", "pruning hook", "长勾刀", "修枝刀",
       0, 0, 1,  6,  80,   5,  4,  8, 0, S,   P_POLEARMS, IRON, HI_METAL,
                                                        GUISARME),
        /* +1d4 small */
WEAPON("bill-guisarme", "hooked polearm", "倒勾刀", "弯曲长柄武器",
       0, 0, 1,  4, 120,   7,  4, 10, 0, P|S, P_POLEARMS, IRON, HI_METAL,
                                                        BILL_GUISARME),
        /* +1d4 small */
/* other */
WEAPON("lucern hammer", "pronged polearm", "苜蓿锤", "分叉长柄武器",
       0, 0, 1,  5, 150,   7,  4,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL,
                                                        LUCERN_HAMMER),
        /* +1d4 small */
WEAPON("bec de corbin", "beaked polearm", "鸦啄战锤", "喙长柄武器",
       0, 0, 1,  4, 100,   8,  8,  6, 0, B|P, P_POLEARMS, IRON, HI_METAL,
                                                        BEC_DE_CORBIN),

/* formerly grouped with the polearms but don't use polearms skill;
   lance isn't even two-handed */
WEAPON("dwarvish mattock", "broad pick", "矮人鹤嘴锄", "宽阔锄头",
       0, 0, 1, 13, 120,  50, 12,  8, -1, B,  P_PICK_AXE, IRON, HI_METAL,
                                                        DWARVISH_MATTOCK),
WEAPON("lance", NoDes, "长戟", NoDes,
       1, 0, 0,  4, 180,  10,  6,  8, 0, P,   P_LANCE, IRON, HI_METAL,
                                                        LANCE),
        /* +2d10 when jousting with lance as primary weapon,
           +2d2 when jousting with it as secondary when dual wielding */

/* bludgeons */
WEAPON("mace", NoDes, "钉头锤", NoDes,
       1, 0, 0, 40,  30,   5,  6,  6, 0, B,   P_MACE, IRON, HI_METAL,
                                                        MACE),
        /* +1 small */
WEAPON("silver mace", NoDes, "银钉头锤", NoDes,
       1, 0, 0,  2,  36,  60,  6,  6, 0, B,   P_MACE, SILVER, HI_SILVER,
                                                        SILVER_MACE),
        /* +1 small */
WEAPON("morning star", NoDes, "流星锤", NoDes,
       1, 0, 0, 12, 120,  10,  4,  6, 0, B,   P_MORNING_STAR, IRON, HI_METAL,
                                                        MORNING_STAR),
        /* +d4 small, +1 large */
WEAPON("war hammer", NoDes, "战锤", NoDes,
       1, 0, 0, 15,  50,   5,  4,  4, 0, B,   P_HAMMER, IRON, HI_METAL,
                                                        WAR_HAMMER),
        /* +1 small */
WEAPON("club", NoDes, "棍棒", NoDes,
       1, 0, 0, 12,  30,   3,  6,  3, 0, B,   P_CLUB, WOOD, HI_WOOD,
                                                        CLUB),
WEAPON("rubber hose", NoDes, "橡胶管", NoDes,
       1, 0, 0,  0,  20,   3,  4,  3, 0, B,   P_WHIP, PLASTIC, CLR_BROWN,
                                                        RUBBER_HOSE),
WEAPON("quarterstaff", "staff", "铁头木棒", "棒子",
       0, 0, 1, 11,  40,   5,  6,  6, 0, B,   P_QUARTERSTAFF, WOOD, HI_WOOD,
                                                        QUARTERSTAFF),
/* two-piece */
WEAPON("aklys", "thonged club", "链棒", "皮带棍棒",
       0, 0, 0,  8,  15,   4,  6,  3, 0, B,   P_CLUB, IRON, HI_METAL,
                                                        AKLYS),
WEAPON("flail", NoDes, "连枷", NoDes,
       1, 0, 0, 40,  15,   4,  6,  4, 0, B,   P_FLAIL, IRON, HI_METAL,
                                                        FLAIL),
        /* +1 small, +1d4 large */

/* misc */
WEAPON("bullwhip", NoDes, "牛鞭", NoDes,
       1, 0, 0,  2,  20,   4,  2,  1, 0, 0,   P_WHIP, LEATHER, CLR_BROWN,
                                                        BULLWHIP),

/* bows */
BOW("bow", NoDes, "弓", NoDes,                     1, 24, 30, 60, 0, WOOD, P_BOW, HI_WOOD,
                                                        BOW),
BOW("elven bow", "runed bow", "精灵弓", "符文弓",   0, 12, 30, 60, 0, WOOD, P_BOW, HI_WOOD,
                                                        ELVEN_BOW),
BOW("orcish bow", "crude bow", "兽人弓", "粗糙弓",  0, 12, 30, 60, 0, WOOD, P_BOW, CLR_BLACK,
                                                        ORCISH_BOW),
BOW("yumi", "long bow", "和弓", "长弓",               0,  0, 30, 60, 0, WOOD, P_BOW, HI_WOOD,
                                                        YUMI),
BOW("sling", NoDes, "投石器", NoDes,                1, 40,  3, 20, 0, LEATHER, P_SLING, HI_LEATHER,
                                                        SLING),
BOW("crossbow", NoDes, "弩", NoDes,             1, 45, 50, 40, 0, WOOD, P_CROSSBOW, HI_WOOD,
                                                        CROSSBOW),

#undef P
#undef S
#undef B

#undef WEAPON
#undef PROJECTILE
#undef BOW

/* armor ... */
        /* IRON denotes ferrous metals, including steel.
         * Only IRON weapons and armor can rust.
         * Only COPPER (including brass) corrodes.
         * Some creatures are vulnerable to SILVER.
         */
#define ARMOR(ename,edesc,name,desc,kn,mgc,blk,power,prob,delay,wt,  \
              cost,ac,can,sub,metal,c,sn)                   \
    OBJECT(OBJ(ename, edesc, name, desc),                                         \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, blk, 0, 0, sub, metal),  \
           power, ARMOR_CLASS, prob, delay, wt,                     \
           cost, 0, 0, 10 - ac, can, wt, c, sn)
#define HELM(ename,edesc,name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(ename, edesc, name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_HELM, metal, c, sn)
#define CLOAK(ename,edesc,name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(ename, edesc, name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_CLOAK, metal, c,sn)
#define SHIELD(ename,edesc,name,desc,kn,mgc,blk,pow,prob,delay,wt,cost,ac,can,metal,c,sn) \
    ARMOR(ename, edesc, name, desc, kn, mgc, blk, pow, prob, delay, wt, \
          cost, ac, can, ARM_SHIELD, metal, c,sn)
#define GLOVES(ename,edesc,name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(ename, edesc, name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_GLOVES, metal, c,sn)
#define BOOTS(ename,edesc,name,desc,kn,mgc,power,prob,delay,wt,cost,ac,can,metal,c,sn)  \
    ARMOR(ename, edesc, name, desc, kn, mgc, 0, power, prob, delay, wt,  \
          cost, ac, can, ARM_BOOTS, metal, c,sn)

/* helmets */
HELM("elven leather helm", "leather hat", "精灵皮帽", "皮帽",
     0, 0,           0,  6, 1,  3,  8,  9, 0, LEATHER, HI_LEATHER,
                                                        ELVEN_LEATHER_HELM),
HELM("orcish helm", "iron skull cap", "兽人头盔", "铁骷髅帽",
     0, 0,           0,  6, 1, 30, 10,  9, 0, IRON, CLR_BLACK,
                                                        ORCISH_HELM),
HELM("dwarvish iron helm", "hard hat", "矮人铁头盔", "安全帽",
     0, 0,           0,  6, 1, 40, 20,  8, 0, IRON, HI_METAL,
                                                        DWARVISH_IRON_HELM),
HELM("fedora", NoDes, "软呢帽", NoDes,
     1, 0,           0,  0, 0,  3,  1, 10, 0, CLOTH, CLR_BROWN,
                                                        FEDORA),
HELM("cornuthaum", "conical hat", "巫师帽", "圆锥形帽",
     0, 1, CLAIRVOYANT,  5, 1,  4, 80, 10, 1, CLOTH, CLR_BLUE,
        /* name coined by devteam; confers clairvoyance for wizards,
           blocks clairvoyance if worn by role other than wizard */
                                                        CORNUTHAUM),
HELM("dunce cap", "conical hat", "愚人帽", "圆锥形帽",
     0, 1,           0,  5, 1,  4,  1, 10, 0, CLOTH, CLR_BLUE,
        /* sets Int and Wis to fixed value of 6, so actually provides
           protection against death caused by Int being drained below 3 */
                                                        DUNCE_CAP),
HELM("dented pot", NoDes, "瘪锅", NoDes,
     1, 0,           0,  2, 0, 10,  8,  9, 0, IRON, CLR_BLACK,
                                                        DENTED_POT),
HELM("helm of brilliance", "crystal helmet", "卓越头盔", "水晶头盔",
     0, 1,           0,  6, 1, 40, 50,  9, 0, GLASS, CLR_WHITE,
        /* used to be iron and shuffled as "etched helmet" but required
           special case for the effect of iron armor on spell casting */
                                                        HELM_OF_BRILLIANCE),
/* with shuffled appearances... */
HELM("helmet", "plumed helmet", "钢盔", "羽饰头盔",
     0, 0,           0, 10, 1, 30, 10,  9, 0, IRON, HI_METAL,
                                                        HELMET),
HELM("helm of caution", "etched helmet", "警觉头盔", "蚀刻头盔",
     0, 1,     WARNING,  6, 1, 50, 50,  9, 0, IRON, CLR_GREEN,
                                                        HELM_OF_CAUTION),
HELM("helm of opposite alignment", "crested helmet", "敌对阵营头盔", "羽冠头盔",
     0, 1,           0, 10, 1, 50, 50,  9, 0, IRON, HI_METAL,
                                                 HELM_OF_OPPOSITE_ALIGNMENT),
HELM("helm of telepathy", "visored helmet", "感知头盔", "檐帽头盔",
     0, 1,     TELEPAT,  4, 1, 50, 50,  9, 0, IRON, HI_METAL,
                                                 HELM_OF_TELEPATHY),

/* suits of armor */
/*
 * There is code in polyself.c that assumes (1) and (2).
 * There is code in obj.h, objnam.c, mon.c, read.c that assumes (2).
 *      (1) The dragon scale mails and the dragon scales are together.
 *      (2) That the order of the dragon scale mail and dragon scales
 *          is the same as order of dragons defined in monst.c.
 */
#define DRGN_ARMR(ename,name,mgc,power,cost,ac,color,snam)  \
    ARMOR(ename, NoDes, name, NoDes, 1, mgc, 1, power, 0, 5, 40,  \
          cost, ac, 0, ARM_SUIT, DRAGON_HIDE, color,snam)
/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
   needed to create them */
DRGN_ARMR("gray dragon scale mail", "灰龙鳞甲",    1, ANTIMAGIC,  1200, 1, CLR_GRAY,
                                                    GRAY_DRAGON_SCALE_MAIL),
    /* gold DSM is a light source; there's no property for that */
DRGN_ARMR("gold dragon scale mail", "金龙鳞甲",    1, 0,           900, 1, HI_GOLD,
                                                    GOLD_DRAGON_SCALE_MAIL),
DRGN_ARMR("silver dragon scale mail", "银龙鳞甲",  1, REFLECTING, 1200, 1, DRAGON_SILVER,
                                                    SILVER_DRAGON_SCALE_MAIL),
#if 0 /* DEFERRED */
DRGN_ARMR("shimmering dragon scale mail", 1, DISPLACED, 1200, 1, CLR_CYAN,
                                                SHIMMERING_DRAGON_SCALE_MAIL),
#endif
DRGN_ARMR("red dragon scale mail", "红龙鳞甲",     1, FIRE_RES,    900, 1, CLR_RED,
                                                    RED_DRAGON_SCALE_MAIL),
DRGN_ARMR("white dragon scale mail", "白龙鳞甲",   1, COLD_RES,    900, 1, CLR_WHITE,
                                                    WHITE_DRAGON_SCALE_MAIL),
DRGN_ARMR("orange dragon scale mail", "橙龙鳞甲",  1, SLEEP_RES,   900, 1, CLR_ORANGE,
                                                    ORANGE_DRAGON_SCALE_MAIL),
DRGN_ARMR("black dragon scale mail", "黑龙鳞甲",   1, DISINT_RES, 1200, 1, CLR_BLACK,
                                                    BLACK_DRAGON_SCALE_MAIL),
DRGN_ARMR("blue dragon scale mail", "蓝龙鳞甲",    1, SHOCK_RES,   900, 1, CLR_BLUE,
                                                    BLUE_DRAGON_SCALE_MAIL),
DRGN_ARMR("green dragon scale mail", "绿龙鳞甲",   1, POISON_RES,  900, 1, CLR_GREEN,
                                                    GREEN_DRAGON_SCALE_MAIL),
DRGN_ARMR("yellow dragon scale mail", "黄龙鳞甲",  1, ACID_RES,    900, 1, CLR_YELLOW,
                                                    YELLOW_DRAGON_SCALE_MAIL),
/* For now, only dragons leave these. */
/* 3.4.1: dragon scales left classified as "non-magic"; they confer magical
   properties but are produced "naturally"; affects use as polypile fodder */
DRGN_ARMR("gray dragon scales", "灰龙鳞",        0, ANTIMAGIC,   700, 7, CLR_GRAY,
                                                        GRAY_DRAGON_SCALES),
DRGN_ARMR("gold dragon scales", "金龙鳞",        0, 0,           500, 7, HI_GOLD,
                                                        GOLD_DRAGON_SCALES),
DRGN_ARMR("silver dragon scales", "银龙鳞",      0, REFLECTING,  700, 7, DRAGON_SILVER,
                                                        SILVER_DRAGON_SCALES),
#if 0 /* DEFERRED */
DRGN_ARMR("shimmering dragon scales",  0, DISPLACED,   700, 7, CLR_CYAN,
                                                    SHIMMERING_DRAGON_SCALES),
#endif
DRGN_ARMR("red dragon scales", "红龙鳞",         0, FIRE_RES,    500, 7, CLR_RED,
                                                        RED_DRAGON_SCALES),
DRGN_ARMR("white dragon scales", "白龙鳞",       0, COLD_RES,    500, 7, CLR_WHITE,
                                                        WHITE_DRAGON_SCALES),
DRGN_ARMR("orange dragon scales", "橙龙鳞",      0, SLEEP_RES,   500, 7, CLR_ORANGE,
                                                        ORANGE_DRAGON_SCALES),
DRGN_ARMR("black dragon scales", "黑龙鳞",       0, DISINT_RES,  700, 7, CLR_BLACK,
                                                        BLACK_DRAGON_SCALES),
DRGN_ARMR("blue dragon scales", "蓝龙鳞",        0, SHOCK_RES,   500, 7, CLR_BLUE,
                                                        BLUE_DRAGON_SCALES),
DRGN_ARMR("green dragon scales", "绿龙鳞",       0, POISON_RES,  500, 7, CLR_GREEN,
                                                        GREEN_DRAGON_SCALES),
DRGN_ARMR("yellow dragon scales", "黄龙鳞",      0, ACID_RES,    500, 7, CLR_YELLOW,
                                                        YELLOW_DRAGON_SCALES),
#undef DRGN_ARMR
/* other suits */
ARMOR("plate mail", NoDes, "板甲", NoDes,
      1, 0, 1,  0, 40, 5, 450, 600,  3, 2,  ARM_SUIT, IRON, HI_METAL,
                                                        PLATE_MAIL),
ARMOR("crystal plate mail", NoDes, "水晶板甲", NoDes,
      1, 0, 1,  0, 10, 5, 415, 820,  3, 2,  ARM_SUIT, GLASS, CLR_WHITE,
                                                        CRYSTAL_PLATE_MAIL),
ARMOR("bronze plate mail", NoDes, "黄铜板甲", NoDes,
      1, 0, 1,  0, 23, 5, 450, 400,  4, 1,  ARM_SUIT, COPPER, HI_COPPER,
                                                        BRONZE_PLATE_MAIL),
ARMOR("splint mail", NoDes, "板条甲", NoDes,
      1, 0, 1,  0, 57, 5, 400,  80,  4, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        SPLINT_MAIL),
ARMOR("banded mail", NoDes, "带链甲", NoDes,
      1, 0, 1,  0, 66, 5, 350,  90,  4, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        BANDED_MAIL),
ARMOR("dwarvish mithril-coat", NoDes, "矮人秘银胶衣", NoDes,
      1, 0, 0,  0, 10, 1, 150, 240,  4, 2,  ARM_SUIT, MITHRIL, HI_SILVER,
                                                        DWARVISH_MITHRIL_COAT),
ARMOR("elven mithril-coat", NoDes, "精灵秘银胶衣", NoDes,
      1, 0, 0,  0, 15, 1, 150, 240,  5, 2,  ARM_SUIT, MITHRIL, HI_SILVER,
                                                        ELVEN_MITHRIL_COAT),
ARMOR("chain mail", NoDes, "锁子甲", NoDes,
      1, 0, 0,  0, 66, 5, 300,  75,  5, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        CHAIN_MAIL),
ARMOR("orcish chain mail", "crude chain mail", "兽人锁子甲", "粗糙锁子甲",
      0, 0, 0,  0, 19, 5, 300,  75,  6, 1,  ARM_SUIT, IRON, CLR_BLACK,
                                                        ORCISH_CHAIN_MAIL),
ARMOR("scale mail", NoDes, "鳞甲", NoDes,
      1, 0, 0,  0, 66, 5, 250,  45,  6, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        SCALE_MAIL),
ARMOR("studded leather armor", NoDes, "嵌皮甲", NoDes,
      1, 0, 0,  0, 66, 3, 200,  15,  7, 1,  ARM_SUIT, LEATHER, HI_LEATHER,
                                                        STUDDED_LEATHER_ARMOR),
ARMOR("ring mail", NoDes, "锁环甲", NoDes,
      1, 0, 0,  0, 66, 5, 250, 100,  7, 1,  ARM_SUIT, IRON, HI_METAL,
                                                        RING_MAIL),
ARMOR("orcish ring mail", "crude ring mail", "兽人锁环甲", "粗糙锁环甲",
      0, 0, 0,  0, 19, 5, 250,  80,  8, 1,  ARM_SUIT, IRON, CLR_BLACK,
                                                        ORCISH_RING_MAIL),
ARMOR("leather armor", NoDes, "皮甲", NoDes,
      1, 0, 0,  0, 75, 3, 150,   5,  8, 1,  ARM_SUIT, LEATHER, HI_LEATHER,
                                                        LEATHER_ARMOR),
ARMOR("leather jacket", NoDes, "皮夹克", NoDes,
      1, 0, 0,  0, 11, 0,  30,  10,  9, 0,  ARM_SUIT, LEATHER, CLR_BLACK,
                                                        LEATHER_JACKET),

/* shirts */
ARMOR("Hawaiian shirt", NoDes, "夏威夷衬衫", NoDes,
      1, 0, 0,  0,  8, 0,   5,   3, 10, 0,  ARM_SHIRT, CLOTH, CLR_MAGENTA,
                                                        HAWAIIAN_SHIRT),
ARMOR("T-shirt", NoDes, "T恤衫", NoDes,
      1, 0, 0,  0,  2, 0,   5,   2, 10, 0,  ARM_SHIRT, CLOTH, CLR_WHITE,
                                                        T_SHIRT),

/* cloaks */
CLOAK("mummy wrapping", NoDes, "木乃伊绷带", NoDes,
      1, 0,          0,  0, 0,  3,  2, 10, 1,  CLOTH, CLR_GRAY,
                                                        MUMMY_WRAPPING),
        /* worn mummy wrapping blocks invisibility */
CLOAK("elven cloak", "faded pall", "精灵斗篷", "褪色的斗篷",
      0, 1,    STEALTH,  8, 0, 10, 60,  9, 1,  CLOTH, CLR_BLACK, ELVEN_CLOAK),
CLOAK("orcish cloak", "coarse mantelet", "兽人斗篷", "粗糙的小斗蓬",
      0, 0,          0,  8, 0, 10, 40, 10, 1,  CLOTH, CLR_BLACK,
                                                        ORCISH_CLOAK),
CLOAK("dwarvish cloak", "hooded cloak", "矮人斗篷", "带帽斗篷",
      0, 0,          0,  8, 0, 10, 50, 10, 1,  CLOTH, HI_CLOTH,
                                                        DWARVISH_CLOAK),
CLOAK("oilskin cloak", "slippery cloak", "油布斗篷", "湿滑的斗篷",
      0, 0,          0,  8, 0, 10, 50,  9, 2,  CLOTH, HI_CLOTH,
                                                        OILSKIN_CLOAK),
CLOAK("robe", NoDes, "长袍", NoDes,
      1, 1,          0,  6, 0, 15, 50,  8, 2,  CLOTH, CLR_RED, ROBE),
        /* robe was adopted from slash'em, where it's worn as a suit
           rather than as a cloak and there are several variations */
CLOAK("alchemy smock", "apron", "炼金术罩衫", "围裙",
      0, 1, POISON_RES, 11, 0, 10, 50,  9, 1,  CLOTH, CLR_WHITE,
                                                        ALCHEMY_SMOCK),
CLOAK("leather cloak", NoDes, "皮斗篷", NoDes,
      1, 0,          0,  8, 0, 15, 40,  9, 1,  LEATHER, CLR_BROWN,
                                                        LEATHER_CLOAK),
/* with shuffled appearances... */
CLOAK("cloak of protection", "tattered cape", "保护斗篷", "破烂的斗篷",
      0, 1, PROTECTION, 11, 0, 10, 50,  7, 3,  CLOTH, HI_CLOTH,
                                                        CLOAK_OF_PROTECTION),
        /* cloak of protection is now the only item conferring MC 3 */
CLOAK("cloak of invisibility", "opera cloak", "隐身斗篷", "夜礼服斗篷",
      0, 1,      INVIS, 12, 0, 10, 60,  9, 1,  CLOTH, CLR_BRIGHT_MAGENTA,
                                                        CLOAK_OF_INVISIBILITY),
CLOAK("cloak of magic resistance", "ornamental cope", "魔法抗性斗篷", "装饰性长袍",
      0, 1,  ANTIMAGIC,  6, 0, 10, 60,  9, 1,  CLOTH, CLR_WHITE,
                                                   CLOAK_OF_MAGIC_RESISTANCE),
        /*  'cope' is not a spelling mistake... leave it be */
CLOAK("cloak of displacement", "piece of cloth", "幻影斗篷", "布块",
      0, 1,  DISPLACED, 12, 0, 10, 50,  9, 1,  CLOTH, HI_CLOTH,
                                                        CLOAK_OF_DISPLACEMENT),

/* shields */
SHIELD("small shield", "wooden shield", "小盾牌", "木制盾牌",
       0, 0, 0,          0,  6, 0,  30,  3, 9, 0,  WOOD, HI_WOOD,
                                                        SMALL_SHIELD),
SHIELD("shield of drain resistance", "wooden shield", "抗等级吸收之盾", "木制盾牌",
       0, 1, 0,  DRAIN_RES, 12, 0,  30, 50, 9, 0,  WOOD, HI_WOOD,
                                                  SHIELD_OF_DRAIN_RESISTANCE),
SHIELD("shield of shock resistance", "wooden shield", "抗电击之盾", "木制盾牌",
       0, 1, 0,  SHOCK_RES, 12, 0,  30, 50, 9, 0,  WOOD, HI_WOOD,
                                                  SHIELD_OF_SHOCK_RESISTANCE),
SHIELD("elven shield", "blue and green shield", "精灵盾", "蓝绿盾",
       0, 0, 0,          0,  2, 0,  40,  7, 8, 0,  WOOD, CLR_GREEN,
                                                        ELVEN_SHIELD),
SHIELD("Uruk-hai shield", "white-handed shield", "强兽人盾", "白色手盾",
       0, 0, 0,          0,  2, 0,  50,  7, 9, 0,  IRON, HI_METAL,
                                                        URUK_HAI_SHIELD),
SHIELD("orcish shield", "red-eyed shield", "兽人盾", "红眼盾",
       0, 0, 0,          0,  2, 0,  50,  7, 9, 0,  IRON, CLR_RED,
                                                        ORCISH_SHIELD),
SHIELD("large shield", NoDes, "大盾牌", NoDes,
       1, 0, 1,          0,  4, 0, 100, 10, 8, 0,  IRON, HI_METAL,
                                                        LARGE_SHIELD),
SHIELD("dwarvish roundshield", "large round shield", "矮人圆盾", "大圆盾",
       0, 0, 0,          0,  3, 0, 100, 10, 8, 0,  IRON, HI_METAL,
                                                        DWARVISH_ROUNDSHIELD),
SHIELD("shield of reflection", "polished silver shield", "反射之盾", "抛光银盾",
       0, 1, 0, REFLECTING,  7, 0,  50, 50, 8, 0,  SILVER, HI_SILVER,
                                                        SHIELD_OF_REFLECTION),

/* gloves */
/* These have their color but not material shuffled, so the IRON must
 * stay CLR_BROWN (== HI_LEATHER) even though it's normally either
 * HI_METAL or CLR_BLACK.  All have shuffled descriptions.
 */
GLOVES("leather gloves", "old gloves", "皮手套", "残破的手套",
       0, 0,        0, 15, 1, 10,  8, 9, 0,  LEATHER, HI_LEATHER,
                                                        LEATHER_GLOVES),
GLOVES("gauntlets of fumbling", "padded gloves", "笨拙手套", "加衬手套",
       0, 1, FUMBLING,  8, 1, 10, 50, 9, 0,  LEATHER, HI_LEATHER,
                                                    GAUNTLETS_OF_FUMBLING),
GLOVES("gauntlets of power", "riding gloves", "力量手套", "骑手手套",
       0, 1,        0,  8, 1, 30, 50, 9, 0,  IRON, CLR_BROWN,
                                                    GAUNTLETS_OF_POWER),
GLOVES("gauntlets of dexterity", "fencing gloves", "敏捷手套", "击剑手套",
       0, 1,        0,  8, 1, 10, 50, 9, 0,  LEATHER, HI_LEATHER,
                                                    GAUNTLETS_OF_DEXTERITY),

/* boots */
BOOTS("low boots", "walking shoes", "低跟鞋", "步行鞋",
      0, 0,          0, 23, 2, 10,  8, 9, 0, LEATHER, HI_LEATHER, LOW_BOOTS),
BOOTS("iron shoes", "hard shoes", "铁鞋", "硬底鞋",
      0, 0,          0,  7, 2, 50, 16, 8, 0, IRON, HI_METAL, IRON_SHOES),
BOOTS("high boots", "jackboots", "高筒靴", "长筒靴",
      0, 0,          0, 14, 2, 20, 12, 8, 0, LEATHER, HI_LEATHER, HIGH_BOOTS),
/* with shuffled appearances... */
BOOTS("speed boots", "combat boots", "速度靴", "战斗靴",
      0, 1,       FAST, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER, SPEED_BOOTS),
BOOTS("water walking boots", "jungle boots", "水上步靴", "丛林靴",
      0, 1,   WWALKING, 12, 2, 15, 50, 9, 0, LEATHER, HI_LEATHER,
                                                        WATER_WALKING_BOOTS),
BOOTS("jumping boots", "hiking boots", "跳跃靴", "登山靴",
      0, 1,    JUMPING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER,
                                                        JUMPING_BOOTS),
BOOTS("elven boots", "mud boots", "精灵靴", "泥靴",
      0, 1,    STEALTH, 12, 2, 15,  8, 9, 0, LEATHER, HI_LEATHER,
                                                        ELVEN_BOOTS),
BOOTS("kicking boots", "buckled boots", "踢靴", "带扣靴",
      0, 1,          0, 12, 2, 50,  8, 9, 0, IRON, CLR_BROWN,
                                                        KICKING_BOOTS),
        /* CLR_BROWN for same reason as gauntlets of power */
BOOTS("fumble boots", "riding boots", "笨拙靴", "马靴",
      0, 1,   FUMBLING, 12, 2, 20, 30, 9, 0, LEATHER, HI_LEATHER,
                                                        FUMBLE_BOOTS),
BOOTS("levitation boots", "snow boots", "飘浮靴", "雪地靴",
      0, 1, LEVITATION, 12, 2, 15, 30, 9, 0, LEATHER, HI_LEATHER,
                                                        LEVITATION_BOOTS),
#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

/* rings ... */
#define RING(ename,estone,name,stone,power,cost,mgc,spec,mohs,metal,color,sn) \
    OBJECT(OBJ(ename, estone, name, stone),                                          \
           BITS(0, 0, spec, 0, mgc, spec, 0, 0, 0,                    \
                HARDGEM(mohs), 0, P_NONE, metal),                     \
           power, RING_CLASS, 1, 0, 3, cost, 0, 0, 0, 0, 15, color,sn)
RING("adornment", "wooden", "装饰品", "木制",
     ADORNED,                  100, 1, 1, 2, WOOD, HI_WOOD, RIN_ADORNMENT),
RING("gain strength", "granite", "增加力量", "花岗石",
     0,                        150, 1, 1, 7, MINERAL, HI_MINERAL,
                                                            RIN_GAIN_STRENGTH),
RING("gain constitution", "opal", "增加体质", "蛋白石",
     0,                        150, 1, 1, 7, MINERAL, HI_MINERAL,
                                                        RIN_GAIN_CONSTITUTION),
RING("increase accuracy", "clay", "增加精确", "黏土",
     0,                        150, 1, 1, 4, MINERAL, CLR_RED,
                                                        RIN_INCREASE_ACCURACY),
RING("increase damage", "coral", "增加伤害", "珊瑚",
     0,                        150, 1, 1, 4, MINERAL, CLR_ORANGE,
                                                        RIN_INCREASE_DAMAGE),
RING("protection", "black onyx", "保护", "黑玛瑙",
     PROTECTION,               100, 1, 1, 7, MINERAL, CLR_BLACK,
                                                        RIN_PROTECTION),
        /* 'PROTECTION' intrinsic enhances MC from worn armor by +1,
           regardless of ring's enchantment; wearing a second ring of
           protection (or even one ring of protection combined with
           cloak of protection) doesn't give a second MC boost */
RING("regeneration", "moonstone", "再生", "月石",
     REGENERATION,             200, 1, 0,  6, MINERAL, HI_MINERAL,
                                                        RIN_REGENERATION),
RING("searching", "tiger eye", "搜索", "虎眼石",
     SEARCHING,                200, 1, 0,  6, GEMSTONE, CLR_BROWN,
                                                        RIN_SEARCHING  ),
RING("stealth", "jade", "潜行", "翡翠",
     STEALTH,                  100, 1, 0,  6, GEMSTONE, CLR_GREEN,
                                                        RIN_STEALTH),
RING("sustain ability", "bronze", "维持能力", "青铜",
     FIXED_ABIL,               100, 1, 0,  4, COPPER, HI_COPPER,
                                                        RIN_SUSTAIN_ABILITY),
RING("levitation", "agate", "飘浮", "玛瑙",
     LEVITATION,               200, 1, 0,  7, GEMSTONE, CLR_RED,
                                                        RIN_LEVITATION),
RING("hunger", "topaz", "饥饿", "黄宝石",
     HUNGER,                   100, 1, 0,  8, GEMSTONE, CLR_CYAN,
                                                        RIN_HUNGER),
RING("aggravate monster", "sapphire", "激怒怪物", "蓝宝石",
     AGGRAVATE_MONSTER,        150, 1, 0,  9, GEMSTONE, CLR_BLUE,
                                                        RIN_AGGRAVATE_MONSTER),
RING("conflict", "ruby", "冲突", "红宝石",
     CONFLICT,                 300, 1, 0,  9, GEMSTONE, CLR_RED,
                                                        RIN_CONFLICT),
RING("warning", "diamond", "警报", "钻石",
     WARNING,                  100, 1, 0, 10, GEMSTONE, CLR_WHITE,
                                                        RIN_WARNING),
RING("poison resistance", "pearl", "毒抗", "珍珠",
     POISON_RES,               150, 1, 0,  4, BONE, CLR_WHITE,
                                                        RIN_POISON_RESISTANCE),
RING("fire resistance", "iron", "火抗", "铁",
     FIRE_RES,                 200, 1, 0,  5, IRON, HI_METAL,
                                                        RIN_FIRE_RESISTANCE),
RING("cold resistance", "brass", "寒抗", "黄铜",
     COLD_RES,                 150, 1, 0,  4, COPPER, HI_COPPER,
                                                        RIN_COLD_RESISTANCE),
RING("shock resistance", "copper", "电抗", "铜",
     SHOCK_RES,                150, 1, 0,  3, COPPER, HI_COPPER,
                                                        RIN_SHOCK_RESISTANCE),
RING("free action", "twisted", "自由行动", "扭曲的",
     FREE_ACTION,              200, 1, 0,  6, IRON, HI_METAL,
                                                        RIN_FREE_ACTION),
RING("slow digestion", "steel", "慢消化", "钢铁",
     SLOW_DIGESTION,           200, 1, 0,  8, IRON, HI_METAL,
                                                        RIN_SLOW_DIGESTION),
RING("teleportation", "silver", "传送", "银",
     TELEPORT,                 200, 1, 0,  3, SILVER, HI_SILVER,
                                                        RIN_TELEPORTATION),
RING("teleport control", "gold", "传送控制", "金",
     TELEPORT_CONTROL,         300, 1, 0,  3, GOLD, HI_GOLD,
                                                        RIN_TELEPORT_CONTROL),
RING("polymorph", "ivory", "变形", "象牙",
     POLYMORPH,                300, 1, 0,  4, BONE, CLR_WHITE,
                                                        RIN_POLYMORPH),
RING("polymorph control", "emerald", "变形控制", "祖母绿",
     POLYMORPH_CONTROL,        300, 1, 0,  8, GEMSTONE, CLR_BRIGHT_GREEN,
                                                        RIN_POLYMORPH_CONTROL),
RING("invisibility", "wire", "隐身", "金属",
     INVIS,                    150, 1, 0,  5, IRON, HI_METAL,
                                                        RIN_INVISIBILITY),
RING("see invisible", "engagement", "看见隐形", "订婚",
     SEE_INVIS,                150, 1, 0,  5, IRON, HI_METAL,
                                                        RIN_SEE_INVISIBLE),
RING("protection from shape changers", "shiny", "怪物现形", "闪耀的",
     PROT_FROM_SHAPE_CHANGERS, 100, 1, 0,  5, IRON, CLR_BRIGHT_CYAN,
                                               RIN_PROTECTION_FROM_SHAPE_CHAN),
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(ename,edesc,name,desc,power,prob,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                            \
           BITS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, P_NONE, IRON),        \
           power, AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL, sn)
AMULET("amulet of ESP",                "circular", "感知护身符", "圆形", TELEPAT, 120,
                                                        AMULET_OF_ESP),
MARKER(FIRST_AMULET, AMULET_OF_ESP)
AMULET("amulet of life saving",       "spherical", "复活护身符", "球形",LIFESAVED, 75,
                                                        AMULET_OF_LIFE_SAVING),
AMULET("amulet of strangulation",          "oval", "窒息护身符", "椭圆形", STRANGLED, 115,
                                                      AMULET_OF_STRANGULATION),
AMULET("amulet of restful sleep",    "triangular", "深度睡眠护身符", "三角形", SLEEPY, 115,
                                                      AMULET_OF_RESTFUL_SLEEP),
AMULET("amulet versus poison",        "pyramidal", "毒抗护身符", "锥状", POISON_RES, 115,
                                                        AMULET_VERSUS_POISON),
AMULET("amulet of change",               "square", "变性护身符", "方形", 0, 115,
                                                        AMULET_OF_CHANGE),
AMULET("amulet of unchanging",          "concave", "阻止变形护身符", "凹形", UNCHANGING, 60,
                                                        AMULET_OF_UNCHANGING),
AMULET("amulet of reflection",        "hexagonal", "反射护身符", "六角形", REFLECTING, 75,
                                                        AMULET_OF_REFLECTION),
AMULET("amulet of magical breathing", "octagonal", "魔法呼吸护身符", "八角形", MAGICAL_BREATHING, 75,
                                                  AMULET_OF_MAGICAL_BREATHING),
        /* +2 AC and +2 MC; +2 takes naked hero past 'warded' to 'guarded' */
AMULET("amulet of guarding",         "perforated", "保护护身符", "带孔", PROTECTION, 75,
                                                        AMULET_OF_GUARDING),
        /* cubical: some descriptions are already three dimensional and
           parallelogrammatical (real word!) would be way over the top */
AMULET("amulet of flying",              "cubical", "飞行护身符", "立方体", FLYING, 60,
                                                        AMULET_OF_FLYING),
/* fixed descriptions; description duplication is deliberate;
 * fake one must come before real one because selection for
 * description shuffling stops when a non-magic amulet is encountered
 */
OBJECT(OBJ("cheap plastic imitation of the Amulet of Yendor",
           "Amulet of Yendor", "岩德护身符的廉价塑料仿制品", "岩德护身符"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, PLASTIC),
       0, AMULET_CLASS, 0, 0, 20, 0, 0, 0, 0, 0, 1, HI_METAL,
                                                FAKE_AMULET_OF_YENDOR),
OBJECT(OBJ("Amulet of Yendor", /* note: description == name */
           "Amulet of Yendor", "岩德护身符", "岩德护身符"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, MITHRIL),
       0, AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL,
                                                AMULET_OF_YENDOR),
MARKER(LAST_AMULET, AMULET_OF_YENDOR)
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOL(ename,edesc,name,desc,kn,mrg,mgc,chg,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(kn, mrg, chg, 0, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat), \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define CONTAINER(ename,edesc,name,desc,kn,mgc,chg,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(kn, 0, chg, 1, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat),   \
           0, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define EYEWEAR(ename,edesc,name,desc,kn,prop,prob,wt,cost,mat,color,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(kn, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, mat),         \
           prop, TOOL_CLASS, prob, 0, wt, cost, 0, 0, 0, 0, wt, color, sn)
#define WEPTOOL(ename,edesc,name,desc,kn,mgc,bi,prob,wt,cost,sdam,ldam,hitbon,sub, \
                mat,clr,sn)                                             \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(kn, 0, 1, 0, mgc, 1, 0, 0, bi, 0, hitbon, sub, mat),    \
           0, TOOL_CLASS, prob, 0, wt, cost, sdam, ldam, hitbon, 0, wt, \
           clr, sn)
/* containers */
CONTAINER("large box",       NoDes, "大箱子", NoDes, 1, 0, 0, 40, 350,   8, WOOD, HI_WOOD,
                                                                LARGE_BOX),
CONTAINER("chest",           NoDes, "箱子", NoDes, 1, 0, 0, 35, 600,  16, WOOD, HI_WOOD,
                                                                CHEST),
CONTAINER("ice box",         NoDes, "冰盒子", NoDes, 1, 0, 0,  5, 900,  42, PLASTIC, CLR_WHITE,
                                                                ICE_BOX),
CONTAINER("sack",           "bag", "布袋", "袋子", 0, 0, 0, 35,  15,   2, CLOTH, HI_CLOTH,
                                                                SACK),
CONTAINER("oilskin sack",   "bag", "防水袋", "袋子", 0, 0, 0,  5,  15, 100, CLOTH, HI_CLOTH,
                                                                OILSKIN_SACK),
CONTAINER("bag of holding", "bag", "次元袋", "袋子", 0, 1, 0, 20,  15, 100, CLOTH, HI_CLOTH,
                                                               BAG_OF_HOLDING),
CONTAINER("bag of tricks",  "bag", "魔术袋", "袋子", 0, 1, 1, 20,  15, 100, CLOTH, HI_CLOTH,
                                                                BAG_OF_TRICKS),
#undef CONTAINER

/* lock opening tools */
TOOL("skeleton key",       "key", "万能钥匙", "钥匙", 0, 0, 0, 0, 80,  3, 10, IRON, HI_METAL,
                                                                SKELETON_KEY),
TOOL("lock pick",           NoDes, "开锁器", NoDes, 1, 0, 0, 0, 60,  4, 20, IRON, HI_METAL,
                                                                LOCK_PICK),
TOOL("credit card",         NoDes, "信用卡", NoDes, 1, 0, 0, 0, 15,  1, 10, PLASTIC, CLR_WHITE,
                                                                CREDIT_CARD),
/* light sources */
TOOL("tallow candle",   "candle", "牛油烛", "蜡烛", 0, 1, 0, 0, 20,  2, 10, WAX, CLR_WHITE,
                                                                TALLOW_CANDLE),
TOOL("wax candle",      "candle", "蜡状蜡烛", "蜡烛", 0, 1, 0, 0,  5,  2, 20, WAX, CLR_WHITE,
                                                                WAX_CANDLE),
TOOL("brass lantern",       NoDes, "黄铜灯笼", NoDes, 1, 0, 0, 0, 30, 30, 12, COPPER, CLR_YELLOW,
                                                                BRASS_LANTERN),
TOOL("oil lamp",          "lamp", "油灯", "灯", 0, 0, 0, 0, 45, 20, 10, COPPER, CLR_YELLOW,
                                                                OIL_LAMP),
TOOL("magic lamp",        "lamp", "神灯", "灯", 0, 0, 1, 0, 15, 20, 50, COPPER, CLR_YELLOW,
                                                                MAGIC_LAMP),
/* other tools */
TOOL("expensive camera",    NoDes, "高档相机", NoDes, 1, 0, 0, 1, 15, 12,200, PLASTIC, CLR_BLACK,
                                                            EXPENSIVE_CAMERA),
TOOL("mirror",   "looking glass", "反光镜", "镜子", 0, 0, 0, 0, 45, 13, 10, GLASS, HI_SILVER,
                                                                MIRROR),
TOOL("crystal ball", "glass orb", "水晶球", "玻璃球", 0, 0, 1, 1, 15,150, 60, GLASS, HI_GLASS,
                                                                CRYSTAL_BALL),
/* eyewear - tools which can be worn on the face; (!mrg, !chg, !mgc)
   worn lenses don't confer the Blinded property, blindfolds and towels do;
   wet towel can be used as a weapon but is not a weptool and uses obj->spe
   differently from weapons and weptools */
EYEWEAR("lenses",           NoDes, "眼镜", NoDes, 1,       0,  5,  3, 80, GLASS, HI_GLASS,
                                                                LENSES),
EYEWEAR("blindfold",        NoDes, "眼罩", NoDes, 1, BLINDED, 50,  2, 20, CLOTH, CLR_BLACK,
                                                                BLINDFOLD),
EYEWEAR("towel",            NoDes, "毛巾", NoDes, 1, BLINDED, 50,  5, 50, CLOTH, CLR_MAGENTA,
                                                                TOWEL),
#undef EYEWEAR

/* still other tools */
TOOL("saddle",              NoDes, "鞍", NoDes, 1, 0, 0, 0,  5,200,150, LEATHER, HI_LEATHER,
                                                                SADDLE),
TOOL("leash",               NoDes, "狗链", NoDes, 1, 0, 0, 0, 65, 12, 20, LEATHER, HI_LEATHER,
                                                                LEASH),
TOOL("stethoscope",         NoDes, "听诊器", NoDes, 1, 0, 0, 0, 25,  4, 75, IRON, HI_METAL,
                                                                STETHOSCOPE),
TOOL("tinning kit",         NoDes, "装罐器", NoDes, 1, 0, 0, 1, 15,100, 30, IRON, HI_METAL,
                                                                TINNING_KIT),
TOOL("tin opener",          NoDes, "开罐器", NoDes, 1, 0, 0, 0, 35,  4, 30, IRON, HI_METAL,
                                                                TIN_OPENER),
TOOL("can of grease",       NoDes, "涂油罐", NoDes, 1, 0, 0, 1, 15, 15, 20, IRON, HI_METAL,
                                                                CAN_OF_GREASE),
TOOL("figurine",            NoDes, "小雕像", NoDes, 1, 0, 1, 0, 25, 50, 80, MINERAL, HI_MINERAL,
                                                                FIGURINE),
        /* monster type specified by obj->corpsenm */
TOOL("magic marker",        NoDes, "魔笔", NoDes, 1, 0, 1, 1, 15,  2, 50, PLASTIC, CLR_RED,
                                                                MAGIC_MARKER),
/* traps */
TOOL("land mine",           NoDes, "地雷", NoDes, 1, 0, 0, 0, 0, 200,180, IRON, CLR_RED,
                                                                LAND_MINE),
TOOL("beartrap",            NoDes, "捕兽夹", NoDes, 1, 0, 0, 0, 0, 200, 60, IRON, HI_METAL,
                                                                BEARTRAP),
/* instruments;
   "If tin whistles are made out of tin, what do they make foghorns out of?" */
TOOL("tin whistle",    "whistle", "六孔哨", "口哨", 0, 0, 0, 0,100, 3, 10, METAL, HI_METAL,
                                                                TIN_WHISTLE),
TOOL("magic whistle",  "whistle", "魔法口哨", "口哨", 0, 0, 1, 0, 30, 3, 10, METAL, HI_METAL,
                                                                MAGIC_WHISTLE),
TOOL("wooden flute",     "flute", "木笛", "长笛", 0, 0, 0, 0,  4, 5, 12, WOOD, HI_WOOD,
                                                                WOODEN_FLUTE),
TOOL("magic flute",      "flute", "魔笛", "长笛", 0, 0, 1, 1,  2, 5, 36, WOOD, HI_WOOD,
                                                                MAGIC_FLUTE),
TOOL("tooled horn",       "horn", "加工号角", "号角", 0, 0, 0, 0,  5, 18, 15, BONE, CLR_WHITE,
                                                                TOOLED_HORN),
TOOL("frost horn",        "horn", "冰霜号角", "号角", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE,
                                                                FROST_HORN),
TOOL("fire horn",         "horn", "火焰号角", "号角", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE,
                                                                FIRE_HORN),
TOOL("horn of plenty",    "horn", "丰饶之角", "号角", 0, 0, 1, 1,  2, 18, 50, BONE, CLR_WHITE,
                                                            HORN_OF_PLENTY),
        /* horn, but not an instrument */
TOOL("wooden harp",       "harp", "木竖琴", "竖琴", 0, 0, 0, 0,  4, 30, 50, WOOD, HI_WOOD,
                                                                WOODEN_HARP),
TOOL("magic harp",        "harp", "魔幻竖琴", "竖琴", 0, 0, 1, 1,  2, 30, 50, WOOD, HI_WOOD,
                                                                MAGIC_HARP),
TOOL("bell",                NoDes,  "铃", NoDes, 1, 0, 0, 0,  2, 30, 50, COPPER, HI_COPPER,
                                                                BELL),
TOOL("bugle",               NoDes, "军号", NoDes, 1, 0, 0, 0,  4, 10, 15, COPPER, HI_COPPER,
                                                                BUGLE),
TOOL("leather drum",      "drum", "皮革鼓", "鼓", 0, 0, 0, 0,  4, 25, 25, LEATHER, HI_LEATHER,
                                                                LEATHER_DRUM),
TOOL("drum of earthquake","drum", "地震鼓", "鼓", 0, 0, 1, 1,  2, 25, 25, LEATHER, HI_LEATHER,
                                                          DRUM_OF_EARTHQUAKE),
/* tools useful as weapons */
WEPTOOL("pick-axe", NoDes, "鹤嘴锄", NoDes,
        1, 0, 0, 20, 100,  50,  6,  3, WHACK,  P_PICK_AXE, IRON, HI_METAL,
                                                                PICK_AXE),
WEPTOOL("grappling hook", NoDes, "爪钩", "铁钩",
        1, 0, 0,  5,  30,  50,  2,  6, WHACK,  P_FLAIL,    IRON, HI_METAL,
                                                             GRAPPLING_HOOK),
WEPTOOL("unicorn horn", NoDes, "独角兽的角", NoDes,
        1, 1, 1,  0,  20, 100, 12, 12, PIERCE, P_UNICORN_HORN,
                                                           BONE, CLR_WHITE,
                                                                UNICORN_HORN),
        /* 3.4.1: unicorn horn left classified as "magic" */
/* two unique tools;
 * not artifacts, despite the comment which used to be here
 */
OBJECT(OBJ("Candelabrum of Invocation", "candelabrum", "祈祷烛台", "烛台"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, GOLD),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 200, HI_GOLD,
                                                   CANDELABRUM_OF_INVOCATION),
OBJECT(OBJ("Bell of Opening", "silver bell", "开启之铃", "银铃"),
       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, P_NONE, SILVER),
       0, TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 50, HI_SILVER,
                                                   BELL_OF_OPENING),
#undef TOOL
#undef WEPTOOL

/* Comestibles ... */
#define FOOD(ename, name, prob, delay, wt, unk, tin, nutrition, color, sn) \
    OBJECT(OBJ(ename, NoDes, name, NoDes),                                            \
           BITS(1, 1, unk, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, tin), 0,     \
           FOOD_CLASS, prob, delay, wt, nutrition / 20 + 5, 0, 0, 0, 0, \
           nutrition, color, sn)
/* All types of food (except tins & corpses) must have a delay of at least 1.
 * Delay on corpses is computed and is weight dependent.
 * Domestic pets prefer tripe rations above all others.
 * Fortune cookies can be read, using them up without ingesting them.
 * Carrots improve your vision.
 * +0 tins contain monster meat.
 * +1 tins (of spinach) make you stronger (like Popeye).
 * Meatballs/sticks/rings are only created from objects via stone to flesh.
 */
/* meat */
FOOD("tripe ration", "牛肚",        140,  2, 10, 0, FLESH, 200, CLR_BROWN,
                                                        TRIPE_RATION),
FOOD("corpse", "尸体",                0,  1,  0, 0, FLESH,   0, CLR_BROWN,
                                                        CORPSE),
FOOD("egg", "蛋",                  85,  1,  1, 1, FLESH,  80, CLR_WHITE,
                                                        EGG),
FOOD("meatball", "肉丸",              0,  1,  1, 0, FLESH,   5, CLR_BROWN,
                                                        MEATBALL),
FOOD("meat stick", "肉棍",            0,  1,  1, 0, FLESH,   5, CLR_BROWN,
                                                        MEAT_STICK),
/* formerly "huge chunk of meat" */
FOOD("enormous meatball", "大块肉",     0, 20,400, 0, FLESH,2000, CLR_BROWN,
                                                        ENORMOUS_MEATBALL),
/* special case because it's not mergeable */
OBJECT(OBJ("meat ring", NoDes, "肉环", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
       0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, CLR_BROWN, MEAT_RING),
/* pudding 'corpses' will turn into these and combine;
   must be in same order as the pudding monsters */
FOOD("glob of gray ooze", "灰色软泥团",     0,  2, 20, 0, FLESH,  20, CLR_GRAY,
                                                       GLOB_OF_GRAY_OOZE),
FOOD("glob of brown pudding", "棕色布丁团", 0,  2, 20, 0, FLESH,  20, CLR_BROWN,
                                                       GLOB_OF_BROWN_PUDDING),
FOOD("glob of green slime", "绿色黏液团",   0,  2, 20, 0, FLESH,  20, CLR_GREEN,
                                                       GLOB_OF_GREEN_SLIME),
FOOD("glob of black pudding", "黑色布丁团", 0,  2, 20, 0, FLESH,  20, CLR_BLACK,
                                                       GLOB_OF_BLACK_PUDDING),

/* fruits & veggies */
FOOD("kelp frond",  "海藻叶子",           0,  1,  1, 0, VEGGY,  30, CLR_GREEN, KELP_FROND),
FOOD("eucalyptus leaf",  "桉叶",      3,  1,  1, 0, VEGGY,   1, CLR_GREEN,
                                                          EUCALYPTUS_LEAF),
FOOD("apple",  "苹果",               15,  1,  2, 0, VEGGY,  50, CLR_RED, APPLE),
FOOD("orange",  "橙子",              10,  1,  2, 0, VEGGY,  80, CLR_ORANGE, ORANGE),
FOOD("pear",  "梨",                10,  1,  2, 0, VEGGY,  50, CLR_BRIGHT_GREEN,
                                                          PEAR),
FOOD("melon", "甜瓜",                10,  1,  5, 0, VEGGY, 100, CLR_BRIGHT_GREEN,
                                                          MELON),
FOOD("banana", "香蕉",               10,  1,  2, 0, VEGGY,  80, CLR_YELLOW, BANANA),
FOOD("carrot", "胡萝卜",               15,  1,  2, 0, VEGGY,  50, CLR_ORANGE, CARROT),
FOOD("sprig of wolfsbane", "附子草枝",    7,  1,  1, 0, VEGGY,  40, CLR_GREEN,
                                                          SPRIG_OF_WOLFSBANE),
FOOD("clove of garlic", "蒜瓣",       7,  1,  1, 0, VEGGY,  40, CLR_WHITE,
                                                          CLOVE_OF_GARLIC),
/* name of slime mold is changed based on player's OPTION=fruit:something
   and bones data might have differently named ones from prior games */
FOOD("slime mold", "黏菌",           75,  1,  5, 0, VEGGY, 250, HI_ORGANIC,
                                                          SLIME_MOLD),

/* people food */
FOOD("lump of royal jelly", "蜂王浆",   0,  1,  2, 0, VEGGY, 200, CLR_YELLOW,
                                                        LUMP_OF_ROYAL_JELLY),
FOOD("cream pie", "奶油派",             25,  1, 10, 0, VEGGY, 100, CLR_WHITE, CREAM_PIE),
FOOD("candy bar", "条形糖果",            13,  1,  2, 0, VEGGY, 100, CLR_BRIGHT_BLUE,
                                                                CANDY_BAR),
FOOD("fortune cookie", "幸运饼干",       55,  1,  1, 0, VEGGY,  40, CLR_YELLOW,
                                                              FORTUNE_COOKIE),
FOOD("pancake", "煎饼",              25,  2,  2, 0, VEGGY, 200, CLR_YELLOW, PANCAKE),
FOOD("lembas wafer", "兰巴斯片",         20,  2,  5, 0, VEGGY, 800, CLR_WHITE,
                                                                LEMBAS_WAFER),
FOOD("cram ration", "压缩口粮",          20,  3, 15, 0, VEGGY, 600, HI_ORGANIC,
                                                                CRAM_RATION),
FOOD("food ration", "口粮",         380,  5, 20, 0, VEGGY, 800, HI_ORGANIC,
                                                                FOOD_RATION),
FOOD("K-ration", "K-口粮",              0,  1, 10, 0, VEGGY, 400, HI_ORGANIC, K_RATION),
FOOD("C-ration", "C-口粮",             0,  1, 10, 0, VEGGY, 300, HI_ORGANIC, C_RATION),
/* tins have type specified by obj->spe (+1 for spinach, other implies
   flesh; negative specifies preparation method {homemade,boiled,&c})
   and by obj->corpsenm (type of monster flesh) */
FOOD("tin", "罐头",                  75,  0, 10, 1, METAL,   0, HI_METAL, TIN),
#undef FOOD

/* potions ... */
#define POTION(ename,edesc,name,desc,mgc,power,prob,cost,color,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, GLASS),      \
           power, POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color, sn)
POTION("gain ability",           "ruby", "增强能力", "深红色",  1, 0, 40, 300, CLR_RED,
                                                        POT_GAIN_ABILITY),
POTION("restore ability",        "pink", "恢复能力", "粉红色",  1, 0, 40, 100, CLR_BRIGHT_MAGENTA,
                                                        POT_RESTORE_ABILITY),
POTION("confusion",            "orange", "混乱", "橙色",  1, CONFUSION, 40, 100, CLR_ORANGE,
                                                        POT_CONFUSION),
POTION("blindness",            "yellow", "失明", "黄色",  1, BLINDED, 30, 150, CLR_YELLOW,
                                                        POT_BLINDNESS),
POTION("paralysis",           "emerald", "麻痹", "翠绿色",  1, 0, 40, 300, CLR_BRIGHT_GREEN,
                                                        POT_PARALYSIS),
POTION("speed",            "dark green", "加速", "深绿色",  1, FAST, 40, 200, CLR_GREEN,
                                                        POT_SPEED),
POTION("levitation",             "cyan", "飘浮", "蓝绿色",  1, LEVITATION, 40, 200, CLR_CYAN,
                                                        POT_LEVITATION),
POTION("hallucination",      "sky blue", "幻觉", "天蓝色",  1, HALLUC, 30, 100, CLR_CYAN,
                                                        POT_HALLUCINATION),
POTION("invisibility", "brilliant blue", "隐身", "亮蓝色",  1, INVIS, 40, 150, CLR_BRIGHT_BLUE,
                                                        POT_INVISIBILITY),
POTION("see invisible",       "magenta", "看见隐形", "洋红色",  1, SEE_INVIS, 40, 50, CLR_MAGENTA,
                                                        POT_SEE_INVISIBLE),
POTION("healing",          "purple-red", "治愈", "紫红色",  1, 0, 115, 20, CLR_MAGENTA,
                                                        POT_HEALING),
POTION("extra healing",          "puce", "强力治愈", "深褐色",  1, 0, 45, 100, CLR_RED,
                                                        POT_EXTRA_HEALING),
POTION("gain level",            "milky", "升级", "乳白色",  1, 0, 20, 300, CLR_WHITE,
                                                        POT_GAIN_LEVEL),
POTION("enlightenment",        "swirly", "启蒙", "涡旋形",  1, 0, 20, 200, CLR_BROWN,
                                                        POT_ENLIGHTENMENT),
POTION("monster detection",    "bubbly", "怪物探测", "多泡的",  1, 0, 40, 150, CLR_WHITE,
                                                        POT_MONSTER_DETECTION),
POTION("object detection",      "smoky", "物品探测", "冒烟的",  1, 0, 40, 150, CLR_GRAY,
                                                        POT_OBJECT_DETECTION),
POTION("gain energy",          "cloudy", "获得能量", "混浊的",  1, 0, 40, 150, CLR_WHITE,
                                                        POT_GAIN_ENERGY),
POTION("sleeping",       "effervescent", "沉睡", "沸腾的",  1, 0, 40, 100, CLR_GRAY,
                                                        POT_SLEEPING),
POTION("full healing",          "black", "完全治愈", "黑色的",  1, 0, 10, 200, CLR_BLACK,
                                                        POT_FULL_HEALING),
POTION("polymorph",            "golden", "变形", "金色",  1, 0, 10, 200, CLR_YELLOW,
                                                        POT_POLYMORPH),
POTION("booze",                 "brown", "酒", "棕色",  0, 0, 40,  50, CLR_BROWN,
                                                        POT_BOOZE),
POTION("sickness",              "fizzy", "疾病", "起泡的",  0, 0, 40,  50, CLR_CYAN,
                                                        POT_SICKNESS),
POTION("fruit juice",            "dark", "果汁", "深色的",  0, 0, 40,  50, CLR_BLACK,
                                                        POT_FRUIT_JUICE),
POTION("acid",                  "white", "酸", "白色的",  0, 0, 10, 250, CLR_WHITE,
                                                        POT_ACID),
POTION("oil",                   "murky", "油", "黑暗的",  0, 0, 30, 250, CLR_BROWN,
                                                        POT_OIL),
/* fixed description
 */
POTION("water",                 "clear", "水", "清澈的",  0, 0, 80, 100, CLR_CYAN,
                                                        POT_WATER),
#undef POTION

/* scrolls ... */
#define SCROLL(ename, etext, name,text,mgc,prob,cost,sn) \
    OBJECT(OBJ(ename, etext, name, text),                                           \
           BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),    \
           0, SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, \
           HI_PAPER, sn)
SCROLL("enchant armor",              "ZELGO MER", "防具附魔", "ZELGO MER",  1,  63,  80,
                                                        SCR_ENCHANT_ARMOR),
SCROLL("destroy armor",         "JUYED AWK YACC", "防具毁坏", "JUYED AWK YACC",  1,  45, 100,
                                                        SCR_DESTROY_ARMOR),
SCROLL("confuse monster",                 "NR 9", "混乱怪物", "NR 9",  1,  53, 100,
                                                        SCR_CONFUSE_MONSTER),
SCROLL("scare monster",   "XIXAXA XOXAXA XUXAXA", "恐吓怪物", "XIXAXA XOXAXA XUXAXA",  1,  35, 100,
                                                        SCR_SCARE_MONSTER),
SCROLL("remove curse",             "PRATYAVAYAH", "解除诅咒", "PRATYAVAYAH",  1,  65,  80,
                                                        SCR_REMOVE_CURSE),
SCROLL("enchant weapon",         "DAIYEN FOOELS", "武器附魔", "DAIYEN FOOELS",  1,  80,  60,
                                                        SCR_ENCHANT_WEAPON),
SCROLL("create monster",       "LEP GEX VEN ZEA", "制造怪物", "LEP GEX VEN ZEA",  1,  45, 200,
                                                        SCR_CREATE_MONSTER),
SCROLL("taming",                   "PRIRUTSENIE", "驯化", "PRIRUTSENIE",  1,  15, 200,
                                                        SCR_TAMING),
SCROLL("genocide",                  "ELBIB YLOH", "灭绝", "ELBIB YLOH",  1,  15, 300,
                                                        SCR_GENOCIDE),
SCROLL("light",                 "VERR YED HORRE", "光亮", "VERR YED HORRE",  1,  90,  50,
                                                        SCR_LIGHT),
SCROLL("teleportation",        "VENZAR BORGAVVE", "传送", "VENZAR BORGAVVE",  1,  55, 100,
                                                        SCR_TELEPORTATION),
SCROLL("gold detection",                 "THARR", "金钱探测", "THARR",  1,  33, 100,
                                                        SCR_GOLD_DETECTION),
SCROLL("food detection",               "YUM YUM", "食物探测", "YUM YUM",  1,  25, 100,
                                                        SCR_FOOD_DETECTION),
SCROLL("identify",                  "KERNOD WEL", "鉴定", "KERNOD WEL",  1, 180,  20,
                                                        SCR_IDENTIFY),
SCROLL("magic mapping",              "ELAM EBOW", "魔法地图", "ELAM EBOW",  1,  45, 100,
                                                        SCR_MAGIC_MAPPING),
SCROLL("amnesia",                   "DUAM XNAHT", "失忆", "DUAM XNAHT",  1,  35, 200,
                                                        SCR_AMNESIA),
SCROLL("fire",                  "ANDOVA BEGARIN", "火", "ANDOVA BEGARIN",  1,  30, 100,
                                                        SCR_FIRE),
SCROLL("earth",                          "KIRJE", "大地", "KIRJE",  1,  18, 200,
                                                        SCR_EARTH),
SCROLL("punishment",            "VE FORBRYDERNE", "惩罚", "VE FORBRYDERNE",  1,  15, 300,
                                                        SCR_PUNISHMENT),
SCROLL("charging",                "HACKEM MUCHE", "充能", "HACKEM MUCHE",  1,  15, 300,
                                                        SCR_CHARGING),
SCROLL("stinking cloud",             "VELOX NEB", "臭云", "VELOX NEB",  1,  15, 300,
                                                        SCR_STINKING_CLOUD),
    /* Extra descriptions, shuffled into use at start of new game.
     * Code in win/share/tilemap.c depends on SCR_STINKING_CLOUD preceding
     * these and on how many of them there are.  If a real scroll gets added
     * after stinking cloud or the number of extra descriptions changes,
     * tilemap.c must be modified to match.  Mgc,Prob,Cost are superfluous.
     * SC values must be distinct but are only used by 'nethack --dumpenums'.
     */
#define XTRA_SCROLL_LABEL(text, sn) SCROLL(NoDes, text, NoDes, text, 1, 0, 100, sn)
XTRA_SCROLL_LABEL(     "FOOBIE BLETCH", SC01),
XTRA_SCROLL_LABEL(             "TEMOV", SC02),
XTRA_SCROLL_LABEL(        "GARVEN DEH", SC03),
XTRA_SCROLL_LABEL(           "READ ME", SC04),
XTRA_SCROLL_LABEL(     "ETAOIN SHRDLU", SC05),
XTRA_SCROLL_LABEL(       "LOREM IPSUM", SC06),
XTRA_SCROLL_LABEL(             "FNORD", SC07), /* Illuminati */
XTRA_SCROLL_LABEL(           "KO BATE", SC08), /* Kurd Lasswitz */
XTRA_SCROLL_LABEL(     "ABRA KA DABRA", SC09), /* traditional incantation */
XTRA_SCROLL_LABEL(      "ASHPD SODALG", SC10), /* Portal */
XTRA_SCROLL_LABEL(           "ZLORFIK", SC11), /* Zak McKracken */
XTRA_SCROLL_LABEL(     "GNIK SISI VLE", SC12), /* Zak McKracken */
XTRA_SCROLL_LABEL(   "HAPAX LEGOMENON", SC13),
XTRA_SCROLL_LABEL( "EIRIS SAZUN IDISI", SC14), /* Merseburg Incantations */
XTRA_SCROLL_LABEL(   "PHOL ENDE WODAN", SC15), /* Merseburg Incantations */
XTRA_SCROLL_LABEL(             "GHOTI", SC16), /* pronounced as 'fish',
                                                * George Bernard Shaw */
XTRA_SCROLL_LABEL("MAPIRO MAHAMA DIROMAT", SC17), /* Wizardry */
XTRA_SCROLL_LABEL( "VAS CORP BET MANI", SC18), /* Ultima */
XTRA_SCROLL_LABEL(           "XOR OTA", SC19), /* Aarne Haapakoski */
XTRA_SCROLL_LABEL("STRC PRST SKRZ KRK", SC20), /* Czech and Slovak
                                                * tongue-twister */
#undef XTRA_SCROLL_LABEL
    /* These must come last because they have special fixed descriptions.
     */
#ifdef MAIL_STRUCTURES
SCROLL("mail",          "stamped", "邮件", "有邮戳的",  0,   0,   0, SCR_MAIL),
#endif
SCROLL("blank paper", "unlabeled", "空白", "无标签的",  0,  28,  60, SCR_BLANK_PAPER),
#undef SCROLL

/* spellbooks ... */
    /* Expanding beyond 52 spells would require changes in spellcasting
     * or imposition of a limit on number of spells hero can know because
     * they are currently assigned successive letters, a-zA-Z, when learned.
     * [The existing spell sorting capability could conceivably be extended
     * to enable moving spells from beyond Z to within it, bumping others
     * out in the process, allowing more than 52 spells be known but keeping
     * only 52 be castable at any given time.]
     */
#define SPELL(ename,edesc,name,desc,sub,prob,delay,level,mgc,dir,color,sn)  \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(0, 0, 0, 0, mgc, 0, 0, 0, 0, 0, dir, sub, PAPER),       \
           0, SPBOOK_CLASS, prob, delay, 50, level * 100,               \
           0, 0, 0, level, 20, color, sn)
/* Spellbook description normally refers to book covers (primarily color).
   Parchment and vellum would never be used for such, but rather than
   eliminate those, finagle their definitions to refer to the pages
   rather than the cover.  They are made from animal skin (typically of
   a goat or sheep) and books using them for pages generally need heavy
   covers with straps or clamps to tightly close the book in order to
   keep the pages flat.  (However, a wooden cover might itself be covered
   by a sheet of parchment, making this become less of an exception.  Also,
   changing the internal composition from paper to leather makes eating a
   parchment or vellum spellbook break vegetarian conduct, as it should.) */
#define PAPER LEATHER /* override enum for use in SPELL() expansion */
SPELL("dig",             "parchment", "挖掘", "羊皮纸",
      P_MATTER_SPELL,      20,  6, 5, 1, RAY, HI_LEATHER, SPE_DIG),
MARKER(FIRST_SPELL, SPE_DIG)
/* magic missile ... finger of death must be in this order; see buzz() */
SPELL("magic missile",   "vellum", "魔法飞弹", "牛皮纸",
      P_ATTACK_SPELL,      45,  2, 2, 1, RAY, HI_LEATHER, SPE_MAGIC_MISSILE),
#undef PAPER /* revert to normal material */
SPELL("fireball",        "ragged", "火球", "粗糙的",
      P_ATTACK_SPELL,      20,  4, 4, 1, RAY, HI_PAPER, SPE_FIREBALL),
SPELL("cone of cold",    "dog eared", "冰锥", "卷边",
      P_ATTACK_SPELL,      10,  7, 4, 1, RAY, HI_PAPER, SPE_CONE_OF_COLD),
SPELL("sleep",           "mottled", "沉睡", "斑驳的",
      P_ENCHANTMENT_SPELL, 30,  1, 3, 1, RAY, HI_PAPER, SPE_SLEEP),
SPELL("finger of death", "stained", "死亡之指", "褪色的",
      P_ATTACK_SPELL,       5, 10, 7, 1, RAY, HI_PAPER, SPE_FINGER_OF_DEATH),
SPELL("light",           "cloth", "光亮", "布",
      P_DIVINATION_SPELL,  45,  1, 1, 1, NODIR, HI_CLOTH, SPE_LIGHT),
SPELL("detect monsters", "leathery", "探测怪物", "坚韧的",
      P_DIVINATION_SPELL,  43,  1, 1, 1, NODIR, HI_LEATHER,
                                                        SPE_DETECT_MONSTERS),
SPELL("healing",         "white", "治愈", "白色的",
      P_HEALING_SPELL,     40,  2, 1, 1, IMMEDIATE, CLR_WHITE,
                                                        SPE_HEALING),
SPELL("knock",           "pink", "敲击", "粉红的",
      P_MATTER_SPELL,      25,  1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA,
                                                        SPE_KNOCK),
SPELL("force bolt",      "red", "力冲击", "红色的",
      P_ATTACK_SPELL,      30,  2, 1, 1, IMMEDIATE, CLR_RED,
                                                        SPE_FORCE_BOLT),
SPELL("confuse monster", "orange", "混乱怪物", "橙色的",
      P_ENCHANTMENT_SPELL, 49,  2, 1, 1, IMMEDIATE, CLR_ORANGE,
                                                        SPE_CONFUSE_MONSTER),
SPELL("cure blindness",  "yellow", "治疗失明", "黄色的",
      P_HEALING_SPELL,     25,  2, 2, 1, IMMEDIATE, CLR_YELLOW,
                                                        SPE_CURE_BLINDNESS),
SPELL("drain life",      "velvet", "吸血", "天鹅绒",
      P_ATTACK_SPELL,      10,  2, 2, 1, IMMEDIATE, CLR_MAGENTA,
                                                        SPE_DRAIN_LIFE),
SPELL("slow monster",    "light green", "减慢怪物", "浅绿色",
      P_ENCHANTMENT_SPELL, 30,  2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN,
                                                        SPE_SLOW_MONSTER),
SPELL("wizard lock",     "dark green", "巫师锁", "深绿色",
      P_MATTER_SPELL,      25,  3, 2, 1, IMMEDIATE, CLR_GREEN,
                                                        SPE_WIZARD_LOCK),
SPELL("create monster",  "turquoise", "制造怪物", "蓝绿色",
      P_CLERIC_SPELL,      35,  3, 2, 1, NODIR, CLR_BRIGHT_CYAN,
                                                        SPE_CREATE_MONSTER),
SPELL("detect food",     "cyan", "探测食物", "青色的",
      P_DIVINATION_SPELL,  30,  3, 2, 1, NODIR, CLR_CYAN,
                                                        SPE_DETECT_FOOD),
SPELL("cause fear",      "light blue", "造成恐惧", "淡蓝色",
      P_ENCHANTMENT_SPELL, 25,  3, 3, 1, NODIR, CLR_BRIGHT_BLUE,
                                                        SPE_CAUSE_FEAR),
SPELL("clairvoyance",    "dark blue", "千里眼", "深蓝色",
      P_DIVINATION_SPELL,  15,  3, 3, 1, NODIR, CLR_BLUE,
                                                        SPE_CLAIRVOYANCE),
SPELL("cure sickness",   "indigo", "治疗疾病", "靛蓝色",
      P_HEALING_SPELL,     32,  3, 3, 1, NODIR, CLR_BLUE,
                                                        SPE_CURE_SICKNESS),
SPELL("charm monster",   "magenta", "魅惑怪物", "洋红色",
      P_ENCHANTMENT_SPELL, 20,  3, 5, 1, IMMEDIATE, CLR_MAGENTA,
                                                        SPE_CHARM_MONSTER),
SPELL("haste self",      "purple", "自我加速", "紫色的",
      P_ESCAPE_SPELL,      33,  4, 3, 1, NODIR, CLR_MAGENTA,
                                                        SPE_HASTE_SELF),
SPELL("detect unseen",   "violet", "探测隐形", "紫罗兰",
      P_DIVINATION_SPELL,  20,  4, 3, 1, NODIR, CLR_MAGENTA,
                                                        SPE_DETECT_UNSEEN),
SPELL("levitation",      "tan", "飘浮", "棕褐色",
      P_ESCAPE_SPELL,      20,  4, 4, 1, NODIR, CLR_BROWN,
                                                        SPE_LEVITATION),
SPELL("extra healing",   "plaid", "强力治愈", "带格子",
      P_HEALING_SPELL,     27,  5, 3, 1, IMMEDIATE, CLR_GREEN,
                                                        SPE_EXTRA_HEALING),
SPELL("restore ability", "light brown", "恢复能力", "浅棕色",
      P_HEALING_SPELL,     25,  5, 4, 1, NODIR, CLR_BROWN,
                                                        SPE_RESTORE_ABILITY),
SPELL("invisibility",    "dark brown", "隐身", "深棕色",
      P_ESCAPE_SPELL,      20,  5, 4, 1, NODIR, CLR_BROWN,
                                                        SPE_INVISIBILITY),
SPELL("detect treasure", "gray", "探测宝藏", "灰色的",
      P_DIVINATION_SPELL,  20,  5, 4, 1, NODIR, CLR_GRAY,
                                                        SPE_DETECT_TREASURE),
SPELL("remove curse",    "wrinkled", "解除诅咒", "皱的",
      P_CLERIC_SPELL,      25,  5, 3, 1, NODIR, HI_PAPER,
                                                        SPE_REMOVE_CURSE),
SPELL("magic mapping",   "dusty", "魔法地图", "浅灰色的",
      P_DIVINATION_SPELL,  18,  7, 5, 1, NODIR, HI_PAPER,
                                                        SPE_MAGIC_MAPPING),
SPELL("identify",        "bronze", "鉴定", "青铜色的",
      P_DIVINATION_SPELL,  20,  6, 3, 1, NODIR, HI_COPPER,
                                                        SPE_IDENTIFY),
SPELL("turn undead",     "copper", "超度", "紫铜色的",
      P_CLERIC_SPELL,      16,  8, 6, 1, IMMEDIATE, HI_COPPER,
                                                        SPE_TURN_UNDEAD),
SPELL("polymorph",       "silver", "变形", "银色的",
      P_MATTER_SPELL,      10,  8, 6, 1, IMMEDIATE, HI_SILVER,
                                                        SPE_POLYMORPH),
SPELL("teleport away",   "gold", "传送", "金色的",
      P_ESCAPE_SPELL,      15,  6, 6, 1, IMMEDIATE, HI_GOLD,
                                                        SPE_TELEPORT_AWAY),
SPELL("create familiar", "glittering", "生成宠物", "辉煌的",
      P_CLERIC_SPELL,      10,  7, 6, 1, NODIR, CLR_WHITE,
                                                        SPE_CREATE_FAMILIAR),
SPELL("cancellation",    "shining", "消除", "闪烁的",
      P_MATTER_SPELL,      15,  8, 7, 1, IMMEDIATE, CLR_WHITE,
                                                        SPE_CANCELLATION),
SPELL("protection",      "dull", "保护", "枯燥的",
      P_CLERIC_SPELL,      18,  3, 1, 1, NODIR, HI_PAPER,
                                                        SPE_PROTECTION),
SPELL("jumping",         "thin", "跳跃", "薄的",
      P_ESCAPE_SPELL,      20,  3, 1, 1, IMMEDIATE, HI_PAPER,
                                                        SPE_JUMPING),
SPELL("stone to flesh",  "thick", "点石成肉", "厚的",
      P_HEALING_SPELL,     15,  1, 3, 1, IMMEDIATE, HI_PAPER,
                                                        SPE_STONE_TO_FLESH),
SPELL("chain lightning", "checkered", "连锁闪电", "方格花纹的",
      P_ATTACK_SPELL,      25,  4, 2, 1, NODIR, CLR_GRAY,
                                                        SPE_CHAIN_LIGHTNING),

#if 0 /* DEFERRED */
/* from slash'em, create a tame critter which explodes when attacking,
   damaging adjacent creatures--friend or foe--and dying in the process */
SPELL("flame sphere",    "canvas", "火焰球", "油画布",
      P_MATTER_SPELL,      20,  2, 1, 1, NODIR, CLR_BROWN,
                                                        SPE_FLAME_SPHERE),
SPELL("freeze sphere",   "hardcover", "冷冻球", "精装本",
      P_MATTER_SPELL,      20,  2, 1, 1, NODIR, CLR_BROWN,
                                                        SPE_FREEZE_SPHERE),
#endif
/* books with fixed descriptions
 */
SPELL("blank paper", "plain", "白纸", "空白", P_NONE, 18, 0, 0, 0, 0, HI_PAPER,
                                                        SPE_BLANK_PAPER),
/* LAST_SPELL is used to calculate MAXSPELL, allocation size of spl_book[];
   by including blank paper, which has no actual spell, we ensure that
   even if hero learns every spell, spl_book[] will have at least one
   unused slot at end; an unused slot is needed for use as terminator */
MARKER(LAST_SPELL, SPE_BLANK_PAPER)
/* tribute book added in 3.6 */
OBJECT(OBJ("novel", "paperback", "小说", "平装本"),
       BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 1, 0, 10, 20, 0, 0, 0, 1, 20, CLR_BRIGHT_BLUE,
                                                        SPE_NOVEL),
/* a special, one of a kind, spellbook */
OBJECT(OBJ("Book of the Dead", "papyrus", "死亡之书", "莎草纸"),
       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, P_NONE, PAPER),
       0, SPBOOK_CLASS, 0, 0, 50, 10000, 0, 0, 0, 7, 20, HI_PAPER,
                                                        SPE_BOOK_OF_THE_DEAD),
#undef SPELL

/* wands ... */
#define WAND(ename,etyp,name,typ,prob,cost,mgc,dir,metal,color,sn) \
    OBJECT(OBJ(ename, etyp, name, typ),                                              \
           BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, P_NONE, metal),    \
           0, WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color, sn)
WAND("light",           "glass", "光亮", "玻璃", 95, 100, 1, NODIR, GLASS, HI_GLASS,
                                                            WAN_LIGHT),
WAND("secret door detection",
                        "balsa", "暗门探测", "巴沙木", 50, 150, 1, NODIR, WOOD, HI_WOOD,
                                                    WAN_SECRET_DOOR_DETECTION),
WAND("enlightenment", "crystal", "启蒙", "水晶", 15, 150, 1, NODIR, GLASS, HI_GLASS,
                                                    WAN_ENLIGHTENMENT),
WAND("create monster",  "maple", "制造怪物", "枫木", 50, 200, 1, NODIR, WOOD, HI_WOOD,
                                                    WAN_CREATE_MONSTER),
WAND("wishing",          "pine", "许愿", "松木",  5, 500, 1, NODIR, WOOD, HI_WOOD,
                                                    WAN_WISHING),
WAND("stasis",        "redwood", "停滞", "红木", 45, 150, 1, NODIR, WOOD, CLR_RED,
                                                    WAN_STASIS),
WAND("nothing",           "oak", "无", "橡木", 25, 100, 0, IMMEDIATE, WOOD, HI_WOOD,
                                                    WAN_NOTHING),
WAND("striking",        "ebony", "冲击", "乌木", 30, 150, 1, IMMEDIATE, WOOD, HI_WOOD,
                                                    WAN_STRIKING),
WAND("make invisible", "marble", "隐身", "大理石", 45, 150, 1, IMMEDIATE, MINERAL, HI_MINERAL,
                                                    WAN_MAKE_INVISIBLE),
WAND("slow monster",      "tin", "减慢怪物", "锡制", 50, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_SLOW_MONSTER),
WAND("speed monster",   "brass", "加速怪物", "黄铜", 50, 150, 1, IMMEDIATE, COPPER, HI_COPPER,
                                                    WAN_SPEED_MONSTER),
WAND("undead turning", "copper", "超度", "铜制", 50, 150, 1, IMMEDIATE, COPPER, HI_COPPER,
                                                    WAN_UNDEAD_TURNING),
WAND("polymorph",      "silver", "变形", "银制", 45, 200, 1, IMMEDIATE, SILVER, HI_SILVER,
                                                    WAN_POLYMORPH),
WAND("cancellation", "platinum", "消除", "白金", 45, 200, 1, IMMEDIATE, PLATINUM, CLR_WHITE,
                                                    WAN_CANCELLATION),
WAND("teleportation", "iridium", "传送", "铱金", 45, 200, 1, IMMEDIATE, METAL,
                                     CLR_BRIGHT_CYAN, WAN_TELEPORTATION),
WAND("opening",          "zinc", "解锁", "锌制", 30, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_OPENING),
WAND("locking",      "aluminum", "上锁", "铝制", 30, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_LOCKING),
WAND("probing",       "uranium", "侦查", "铀制", 30, 150, 1, IMMEDIATE, METAL, HI_METAL,
                                                    WAN_PROBING),
WAND("digging",          "iron", "挖掘", "铁制", 40, 150, 1, RAY, IRON, HI_METAL,
                                                    WAN_DIGGING),
/* magic missile ... lightning must be in this order; see buzz() */
WAND("magic missile",   "steel", "魔法飞弹", "钢铁", 50, 150, 1, RAY, IRON, HI_METAL,
                                                    WAN_MAGIC_MISSILE),
WAND("fire",        "hexagonal", "火焰", "六角形", 40, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_FIRE),
WAND("cold",            "short", "寒冷", "短", 40, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_COLD),
WAND("sleep",           "runed", "沉睡", "符文", 50, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_SLEEP),
WAND("death",            "long", "死亡", "长",  5, 500, 1, RAY, IRON, HI_METAL,
                                                    WAN_DEATH),
WAND("lightning",      "curved", "闪电", "弧形", 40, 175, 1, RAY, IRON, HI_METAL,
                                                    WAN_LIGHTNING),
/* extra descriptions, shuffled into use at start of new game */
WAND(NoDes,             "forked", NoDes, "分叉",  0, 150, 1, 0, WOOD, HI_WOOD, WAN1),
WAND(NoDes,             "spiked", NoDes, "尖顶",  0, 150, 1, 0, IRON, HI_METAL, WAN2),
WAND(NoDes,            "jeweled", NoDes, "宝石",  0, 150, 1, 0, IRON, HI_MINERAL, WAN3),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(ename,name,prob,metal,worth,sn) \
    OBJECT(OBJ(ename, NoDes, name, NoDes),                                         \
           BITS(1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, metal),    \
           0, COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, HI_GOLD, sn)
COIN("gold piece", "金币", 1000, GOLD, 1, GOLD_PIECE),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(ename,edesc,name,desc,prob,wt,gval,nutr,mohs,glass,color,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(0, 1, 0, 0, 0, 0, 0, 0, 0,                              \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, 3, 3, 0, 0, nutr, color, sn)
#define ROCK(ename,edesc,name,desc,kn,prob,wt,gval,sdam,ldam,mgc,nutr,mohs,glass,colr,sn) \
    OBJECT(OBJ(ename, edesc, name, desc),                                             \
           BITS(kn, 1, 0, 0, mgc, 0, 0, 0, 0,                           \
                HARDGEM(mohs), 0, -P_SLING, glass),                     \
           0, GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr, colr, sn)
GEM("dilithium crystal", "white", "双锂水晶", "白色的",  2, 1, 4500, 15,  5, GEMSTONE, CLR_WHITE,
                                                        DILITHIUM_CRYSTAL),
MARKER(FIRST_REAL_GEM, DILITHIUM_CRYSTAL)
GEM("diamond",           "white", "钻石", "白色的",  3, 1, 4000, 15, 10, GEMSTONE, CLR_WHITE,
                                                        DIAMOND),
GEM("ruby",                "red", "红宝石", "红色的",  4, 1, 3500, 15,  9, GEMSTONE, CLR_RED,
                                                        RUBY),
GEM("jacinth",          "orange", "红锆石", "橙色的",  3, 1, 3250, 15,  9, GEMSTONE, CLR_ORANGE,
                                                        JACINTH),
GEM("sapphire",           "blue", "蓝宝石", "蓝色的",  4, 1, 3000, 15,  9, GEMSTONE, CLR_BLUE,
                                                        SAPPHIRE),
GEM("black opal",        "black", "黑蛋白石", "黑色的",  3, 1, 2500, 15,  8, GEMSTONE, CLR_BLACK,
                                                        BLACK_OPAL),
GEM("emerald",           "green", "祖母绿", "绿色的",  5, 1, 2500, 15,  8, GEMSTONE, CLR_GREEN,
                                                        EMERALD),
GEM("turquoise",         "green", "绿松石", "绿色的",  6, 1, 2000, 15,  6, GEMSTONE, CLR_GREEN,
                                                        TURQUOISE),
GEM("citrine",          "yellow", "黄水晶", "黄色的",  4, 1, 1500, 15,  6, GEMSTONE, CLR_YELLOW,
                                                        CITRINE),
GEM("aquamarine",        "green", "海蓝宝石", "绿色的",  6, 1, 1500, 15,  8, GEMSTONE, CLR_GREEN,
                                                        AQUAMARINE),
GEM("amber",   "yellowish brown", "琥珀", "杏色的",  8, 1, 1000, 15,  2, GEMSTONE, CLR_BROWN,
                                                        AMBER),
GEM("topaz",   "yellowish brown", "黄宝石", "杏色的", 10, 1,  900, 15,  8, GEMSTONE, CLR_BROWN,
                                                        TOPAZ),
GEM("jet",               "black", "黑玉", "黑色的",  6, 1,  850, 15,  7, GEMSTONE, CLR_BLACK,
                                                        JET),
GEM("opal",              "white", "蛋白石", "白色的", 12, 1,  800, 15,  6, GEMSTONE, CLR_WHITE,
                                                        OPAL),
GEM("chrysoberyl",      "yellow", "金绿玉", "黄色的",  8, 1,  700, 15,  5, GEMSTONE, CLR_YELLOW,
                                                        CHRYSOBERYL),
GEM("garnet",              "red", "石榴石", "红色的", 12, 1,  700, 15,  7, GEMSTONE, CLR_RED,
                                                        GARNET),
GEM("amethyst",         "violet", "紫水晶", "紫色的", 14, 1,  600, 15,  7, GEMSTONE, CLR_MAGENTA,
                                                        AMETHYST),
GEM("jasper",              "red", "碧玉", "红色的", 15, 1,  500, 15,  7, GEMSTONE, CLR_RED,
                                                        JASPER),
GEM("fluorite",         "violet", "萤石", "紫色的", 15, 1,  400, 15,  4, GEMSTONE, CLR_MAGENTA,
                                                        FLUORITE),
GEM("obsidian",          "black", "黑曜石", "黑色的",  9, 1,  200, 15,  6, GEMSTONE, CLR_BLACK,
                                                        OBSIDIAN),
GEM("agate",            "orange", "玛瑙", "橙色的", 12, 1,  200, 15,  6, GEMSTONE, CLR_ORANGE,
                                                        AGATE),
GEM("jade",              "green", "翡翠", "绿色的", 10, 1,  300, 15,  6, GEMSTONE, CLR_GREEN,
                                                        JADE),
MARKER(LAST_REAL_GEM, JADE)
GEM("worthless piece of white glass", "white", "毫无价值的白色玻璃碎片", "白色的",
    77, 1, 0, 6, 5, GLASS, CLR_WHITE, WORTHLESS_WHITE_GLASS),
MARKER(FIRST_GLASS_GEM, WORTHLESS_WHITE_GLASS)
GEM("worthless piece of blue glass", "blue", "毫无价值的蓝色玻璃碎片", "蓝色的",
    77, 1, 0, 6, 5, GLASS, CLR_BLUE, WORTHLESS_BLUE_GLASS),
GEM("worthless piece of red glass", "red", "毫无价值的红色玻璃碎片", "红色的",
    77, 1, 0, 6, 5, GLASS, CLR_RED, WORTHLESS_RED_GLASS),
GEM("worthless piece of yellowish brown glass", "yellowish brown", "毫无价值的杏色玻璃碎片", "杏色的",
    77, 1, 0, 6, 5, GLASS, CLR_BROWN, WORTHLESS_YELLOWBROWN_GLASS),
GEM("worthless piece of orange glass", "orange", "毫无价值的橙色玻璃碎片", "橙色的",
    76, 1, 0, 6, 5, GLASS, CLR_ORANGE, WORTHLESS_ORANGE_GLASS),
GEM("worthless piece of yellow glass", "yellow", "毫无价值的黄色玻璃碎片", "黄色的",
    77, 1, 0, 6, 5, GLASS, CLR_YELLOW, WORTHLESS_YELLOW_GLASS),
GEM("worthless piece of black glass", "black", "毫无价值的黑色玻璃碎片", "黑色的",
    76, 1, 0, 6, 5, GLASS, CLR_BLACK, WORTHLESS_BLACK_GLASS),
GEM("worthless piece of green glass", "green", "毫无价值的绿色玻璃碎片", "绿色的",
    77, 1, 0, 6, 5, GLASS, CLR_GREEN, WORTHLESS_GREEN_GLASS),
GEM("worthless piece of violet glass", "violet", "毫无价值的紫色玻璃碎片", "紫色的",
    77, 1, 0, 6, 5, GLASS, CLR_MAGENTA, WORTHLESS_VIOLET_GLASS),
MARKER(LAST_GLASS_GEM, WORTHLESS_VIOLET_GLASS)

/* Placement note: there is a wishable subrange for
 * "gray stones" in the o_ranges[] array in objnam.c
 * that is currently everything between luckstones and flint
 * (inclusive).
 */
ROCK("luckstone", "gray", "幸运石", "灰色的",  0,  10,  10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY,
                                                                    LUCKSTONE),
ROCK("loadstone", "gray", "负重石", "灰色的",  0,  10, 500,  1, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY,
                                                                    LOADSTONE),
ROCK("touchstone", "gray", "试金石", "灰色的", 0,   8,  10, 45, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY,
                                                                  TOUCHSTONE),
ROCK("flint", "gray", "打火石", "灰色的",      0,  10,  10,  1, 6, 6, 0, 10, 7, MINERAL, CLR_GRAY,
                                                                    FLINT),
ROCK("rock", NoDes, "岩石", NoDes,         1, 100,  10,  0, 3, 3, 0, 10, 7, MINERAL, CLR_GRAY,
                                                                    ROCK),
#undef GEM
#undef ROCK

/* miscellaneous ... */
/* Note: boulders and rocks are not normally created at random; the
 * probabilities only come into effect when you try to polymorph them.
 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
 * on a specific type and may act as containers (both affect weight).
 */
OBJECT(OBJ("boulder", NoDes, "巨石", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 100, 0, 6000, 0, 20, 20, 0, 0, 2000, HI_MINERAL, BOULDER),
OBJECT(OBJ("statue", NoDes, "雕像", NoDes),
       BITS(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, P_NONE, MINERAL), 0,
       ROCK_CLASS, 900, 0, 2500, 0, 20, 20, 0, 0, 2500, CLR_WHITE, STATUE),

OBJECT(OBJ("heavy iron ball", NoDes, "沉重的铁球", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       BALL_CLASS, 1000, 0, 480, 10, 25, 25, 0, 0, 200, HI_METAL,
                                                            HEAVY_IRON_BALL),
        /* +d4 when "very heavy" */
OBJECT(OBJ("iron chain", NoDes, "铁链", NoDes),
       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
       CHAIN_CLASS, 1000, 0, 120, 0, 4, 4, 0, 0, 200, HI_METAL, IRON_CHAIN),
        /* +1 both l & s */

/* Venom is normally a transitory missile (spit by various creatures)
 * but can be wished for in wizard mode so could occur in bones data.
 */
OBJECT(OBJ("splash of blinding venom", "splash of venom", "致盲毒液", "飞溅的毒液"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_ORGANIC, BLINDING_VENOM),
OBJECT(OBJ("splash of acid venom", "splash of venom", "酸性毒液", "飞溅的毒液"),
       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_ORGANIC, ACID_VENOM),
        /* +d6 small or large */

#if defined(OBJECTS_DESCR_INIT) || defined(OBJECTS_INIT)
/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
OBJECT(OBJ(NoDes, NoDes, NoDes, NoDes),
       BITS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0), 0,
       ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
#undef BITS
#endif

#undef OBJ
#undef OBJECT
#undef MARKER
#undef HARDGEM
#undef NoDes

/*objects.c*/
