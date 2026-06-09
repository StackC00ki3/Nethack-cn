/* NetHack 5.0	insight.c	$NHDT-Date: 1777004419 2026/04/23 20:20:19 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.134 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Enlightenment and Conduct+Achievements and Vanquished+Extinct+Geno'd
 * and stethoscope/probing feedback.
 *
 * Most code used to reside in cmd.c, presumably because ^X was originally
 * a wizard mode command and the majority of those are in that file.
 * Some came from end.c where it is used during end of game disclosure.
 * And some came from priest.c that had once been in pline.c.
 */

#include "hack.h"

staticfn void enlght_out(const char *);
staticfn void enlght_line(const char *, const char *, const char *,
                          const char *);
staticfn char *enlght_combatinc(const char *, int, int, char *);
staticfn void enlght_halfdmg(int, int);
staticfn boolean walking_on_water(void);
staticfn boolean cause_known(int);
staticfn char *attrval(int, int, char *);
staticfn char *fmt_elapsed_time(char *, int);
staticfn char *N_times(long, char *) NONNULL NONNULLARG2;
staticfn void background_enlightenment(int, int);
staticfn void basics_enlightenment(int, int);
staticfn void characteristics_enlightenment(int, int);
staticfn void one_characteristic(int, int, int);
staticfn void status_enlightenment(int, int);
staticfn void weapon_insight(int);
staticfn void attributes_enlightenment(int, int);
staticfn void show_achievements(int);
staticfn int QSORTCALLBACK vanqsort_cmp(const genericptr, const genericptr);
staticfn int num_extinct(void);
staticfn int num_gone(int, int *);
staticfn char *size_str(int);
staticfn void item_resistance_message(int, const char *, int);

extern const char *const hu_stat[];  /* hunger status from eat.c */
extern const char *const enc_stat[]; /* encumbrance status from botl.c */

static const char You_[] = "你", are[] = "是", were[] = "曾是",
                  have[] = "拥有", had[] = "曾拥有", can[] = "可以",
                  could[] = "曾可以";

/* for livelogging: */
struct ll_achieve_msg {
    long llflag;
    const char *msg;
};
/* ordered per 'enum achievements' in you.h */
/* take care to keep them in sync! */
static struct ll_achieve_msg achieve_msg [] = {
    { 0, "" }, /* actual achievements are numbered from 1 */
    { LL_ACHIEVE, "取得了开门之铃" },
    { LL_ACHIEVE, "进入了 Gehennom" },
    { LL_ACHIEVE, "取得了祈祷烛台" },
    { LL_ACHIEVE, "取得了亡者之书" },
    { LL_ACHIEVE, "进行了祈祷仪式" },
    { LL_ACHIEVE, "取得了 Yendor 护符" },
    { LL_ACHIEVE, "进入了元素位面" },
    { LL_ACHIEVE, "进入了星界位面" },
    { LL_ACHIEVE, "飞升了" },
    /* if the type of item isn't discovered yet, disclosing the event
       via #chronicle would be a spoiler (particularly for gray stone);
       the ID'd name for the type of item will be appended to the next
       two messages, for display via livelog and/or dumplog */
    { LL_ACHIEVE | LL_SPOILER, "取得了矿坑尽头的" }, /* " luckstone" */
    { LL_ACHIEVE | LL_SPOILER, "取得了推箱子的" }, /* " <item>" */
    { LL_ACHIEVE | LL_UMONST, "杀死了美杜莎" },
     /* these two are not logged */
    { 0, "英雄一直失明" },
    { 0, "英雄从未穿戴盔甲" },
     /* */
    { LL_MINORAC | LL_DUMP, "进入了侏儒矿坑" },
    { LL_ACHIEVE, "到达了矿镇" }, /* probably minor, but dnh logs it */
    { LL_MINORAC, "进入了一家商店" },
    { LL_MINORAC, "进入了一座神殿" },
    { LL_ACHIEVE, "咨询了先知" }, /* minor, but rare enough */
    { LL_MINORAC | LL_DUMP, "读了一本 Discworld 小说" }, /* even more so */
    { LL_ACHIEVE, "进入了推箱子关" }, /* keep as major for turn comparison
                                        * with completed sokoban */
    { LL_ACHIEVE, "进入了大房间" },
    /* The following 8 are for advancing through the ranks
       and messages differ by role so are created on the fly;
       rank 0 (Xp 1 and 2) isn't an achievement */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 3 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 6 */
    { LL_MINORAC | LL_DUMP, "" }, /* Xp 10 */
    { LL_ACHIEVE, "" }, /* Xp 14, so able to attempt the quest */
    { LL_ACHIEVE, "" }, /* Xp 18 */
    { LL_ACHIEVE, "" }, /* Xp 22 */
    { LL_ACHIEVE, "" }, /* Xp 26 */
    { LL_ACHIEVE, "" }, /* Xp 30 */
    { LL_MINORAC, "学会了城堡吊桥的曲调" }, /* achievement #31 */
    { 0, "" } /* keep this one at the end */
};

/* macros to simplify output of enlightenment messages; also used by
   conduct and achievements */
#define enl_msg(prefix, present, past, suffix, ps) \
    enlght_line((prefix), final ? (past) : (present), (suffix), (ps))
#define you_are(attr, ps) enl_msg(You_, are, were, (attr), (ps))
#define you_have(attr, ps) enl_msg(You_, have, had, (attr), (ps))
#define you_can(attr, ps) enl_msg(You_, can, could, (attr), (ps))
#define you_have_been(goodthing) \
    enl_msg(You_, "", "", (goodthing), "")
#define you_have_never(badthing) \
    enl_msg(You_, "从未", "从未", (badthing), "")
#define you_have_X(something) \
    enl_msg(You_, "", (const char *) "", (something), "")

staticfn void
enlght_out(const char *buf)
{
    if (ge.en_via_menu) {
        add_menu_str(ge.en_win, buf);
    } else
        putstr(ge.en_win, 0, buf);
}

staticfn void
enlght_line(
    const char *start,
    const char *middle,
    const char *end,
    const char *ps)
{
#ifndef NO_ENLGHT_CONTRACTIONS
    static const struct contrctn {
        const char *twowords, *contrctn;
    } contra[] = {
        { " are not ", " aren't " },
        { " were not ", " weren't " },
        { " have not ", " haven't " },
        { " had not ", " hadn't " },
        { " can not ", " can't " },
        { " could not ", " couldn't " },
    };
    int i;
#endif
    char buf[BUFSZ];

    Sprintf(buf, "  %s%s%s%s.", start, middle, end, ps);
#ifndef NO_ENLGHT_CONTRACTIONS
    if (strstri(buf, " not ")) { /* TODO: switch to libc strstr() */
        for (i = 0; i < SIZE(contra); ++i)
            (void) strsubst(buf, contra[i].twowords, contra[i].contrctn);
    }
#endif
    enlght_out(buf);
}

/* format increased chance to hit or damage or defense (Protection) */
staticfn char *
enlght_combatinc(
    const char *inctyp, /* "to hit" or "damage" or "defense" */
    int incamt,         /* amount of increment (negative if decrement) */
    int final,          /* ENL_{GAMEINPROGRESS,GAMEOVERALIVE,GAMEOVERDEAD} */
    char *outbuf)
{
    const char *modif, *bonus, *inctyp_cn;
    int absamt;

    absamt = abs(incamt);
    /* Protection amount is typically larger than damage or to-hit;
       reduce magnitude by a third in order to stretch modifier ranges
       (small:1..5, moderate:6..10, large:11..19, huge:20+) */
    if (!strcmp(inctyp, "defense"))
        absamt = (absamt * 2) / 3;

    if (absamt <= 3)
        modif = "少量";
    else if (absamt <= 6)
        modif = "中等";
    else if (absamt <= 12)
        modif = "大量";
    else
        modif = "巨量";

    modif = !incamt ? "无" : modif; /* ("no" case shouldn't happen) */
    bonus = (incamt >= 0) ? "加值" : "惩罚";
    if (!strcmp(inctyp, "to hit"))
        inctyp_cn = "命中";
    else if (!strcmp(inctyp, "damage"))
        inctyp_cn = "伤害";
    else if (!strcmp(inctyp, "defense"))
        inctyp_cn = "防御";
    else
        inctyp_cn = inctyp;

    Sprintf(outbuf, "%s%s%s", modif, inctyp_cn, bonus);
    if (final || wizard)
        Sprintf(eos(outbuf), " (%s%d)", (incamt > 0) ? "+" : "", incamt);

    return outbuf;
}

/* report half physical or half spell damage */
staticfn void
enlght_halfdmg(int category, int final)
{
    const char *category_name;
    char buf[BUFSZ];

    switch (category) {
    case HALF_PHDAM:
        category_name = "物理";
        break;
    case HALF_SPDAM:
        category_name = "法术";
        break;
    default:
        category_name = "未知";
        break;
    }
    Sprintf(buf, "承受%s%s伤害", (final || wizard) ? "一半的" : "减少的",
            category_name);
    enl_msg(You_, "", "", buf, from_what(category));
}

/* is hero actively using water walking capability on water (or lava)? */
staticfn boolean
walking_on_water(void)
{
    if (u.uinwater || Levitation || Flying)
        return FALSE;
    return (boolean) (Wwalking && is_pool_or_lava(u.ux, u.uy));
}

/* describe u.utraptype; used by status_enlightenment() and self_lookat() */
char *
trap_predicament(char *outbuf, int final, boolean wizxtra)
{
    struct trap *t;

    /* caller has verified u.utrap */
    *outbuf = '\0';
    switch (u.utraptype) {
    case TT_BURIEDBALL:
        Strcpy(outbuf, "被拴在埋藏的东西上");
        break;
    case TT_LAVA:
        Sprintf(outbuf, "沉入%s", final ? "熔岩" : hliquid("熔岩"));
        break;
    case TT_INFLOOR:
        Sprintf(outbuf, "被困在 %s", the(surface(u.ux, u.uy)));
        break;
    default: /* TT_BEARTRAP, TT_PIT, or TT_WEB */
        Strcpy(outbuf, "被困住了");
        if ((t = t_at(u.ux, u.uy)) != 0) /* should never be null */
            Sprintf(eos(outbuf), "在 %s", an(trapname(t->ttyp, FALSE)));
        break;
    }
    if (wizxtra) { /* give extra information for wizard mode enlightenment */
        /* curly braces: u.utrap is an escape attempt counter rather than a
           turn timer so use different ornamentation than usual parentheses */
        Sprintf(eos(outbuf), " {%u}", u.utrap);
    }
    return outbuf;
}

/* check whether hero is wearing something that player definitely knows
   confers the target property; item must have been seen and its type
   discovered but it doesn't necessarily have to be fully identified */
staticfn boolean
cause_known(
    int propindx) /* index of a property which can be conveyed by worn item */
{
    struct obj *o;
    long mask = W_ARMOR | W_AMUL | W_RING | W_TOOL;

    /* simpler than from_what()/what_gives(); we don't attempt to
       handle artifacts and we deliberately ignore wielded items */
    for (o = gi.invent; o; o = o->nobj) {
        if (!(o->owornmask & mask))
            continue;
        if ((int) objects[o->otyp].oc_oprop == propindx
            && objects[o->otyp].oc_name_known && o->dknown)
            return TRUE;
    }
    return FALSE;
}

/* format a characteristic value, accommodating Strength's strangeness */
staticfn char *
attrval(
    int attrindx,
    int attrvalue,
    char resultbuf[]) /* should be at least [7] to hold "18/100\0" */
{
    if (attrindx != A_STR || attrvalue <= 18)
        Sprintf(resultbuf, "%d", attrvalue);
    else if (attrvalue > STR18(100)) /* 19 to 25 */
        Sprintf(resultbuf, "%d", attrvalue - 100);
    else /* simplify "18/\**" to be "18/100" */
        Sprintf(resultbuf, "18/%02d", attrvalue - 18);
    return resultbuf;
}

/* format urealtime.realtime as
      " D days, H hours, M minutes and S seconds"
   with any fields having a value of 0 omitted:
      0-00:00:20 => " 20 seconds"
      0-00:15:05 => " 15 minutes and 5 seconds"
      0-00:16:00 => " 16 minutes"
      0-01:15:10 => " 1 hour, 15 minutes and 10 seconds"
      0-02:00:01 => " 2 hours and 1 second"
      3-00:25:40 => " 3 days, 25 minutes and 40 seconds"
   (note: for a list of more than two entries, nethack usually includes the
   [style-wise] optional comma before "and" but in this instance it does not)
 */
staticfn char *
fmt_elapsed_time(char *outbuf, int final)
{
    int fieldcnt;
    long edays, ehours, eminutes, eseconds;
    /* for a game that's over, reallydone() has updated urealtime.realtime
       to its final value before calling us during end of game disclosure;
       for a game that's still in progress, it holds the amount of elapsed
       game time from previous sessions up through most recent save/restore
       (or up through latest level change when 'checkpoint' is On);
       '.start_timing' has a non-zero value even if '.realtime' is 0 */
    long etim = urealtime.realtime;

    if (!final)
        etim += timet_delta(getnow(), urealtime.start_timing);
    /* we could use localtime() to convert the value into a 'struct tm'
       to get date and time fields but this is simple and straightforward */
    eseconds = etim % 60L, etim /= 60L;
    eminutes = etim % 60L, etim /= 60L;
    ehours = etim % 24L;
    edays = etim / 24L;
    fieldcnt = !!edays + !!ehours + !!eminutes + !!eseconds;

    Strcpy(outbuf, fieldcnt ? "" : " 无"); /* 'none' should never happen */
    if (edays) {
        Sprintf(eos(outbuf), " %ld 天", edays);
        if (fieldcnt > 1) /* hours and/or minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? " 和" : ",");
        --fieldcnt; /* edays has been processed */
    }
    if (ehours) {
        Sprintf(eos(outbuf), " %ld 小时", ehours);
        if (fieldcnt > 1) /* minutes and/or seconds to follow */
            Strcat(outbuf, (fieldcnt == 2) ? " 和" : ",");
        --fieldcnt; /* ehours has been processed */
    }
    if (eminutes) {
        Sprintf(eos(outbuf), " %ld 分钟", eminutes);
        if (fieldcnt > 1) /* seconds to follow */
            Strcat(outbuf, "和");
        /* eminutes has been processed but no need to decrement fieldcnt */
    }
    if (eseconds)
        Sprintf(eos(outbuf), " %ld秒", eseconds);
    return outbuf;
}

/* "once" vs "twice" vs "17 times", used in several places */
staticfn char *
N_times(long n, char *outbuf)
{
    switch (n) {
    case 0:
    default:
        Sprintf(outbuf, "%ld次", n);
        break;
    case 1:
        Strcpy(outbuf, "一次");
        break;
    case 2:
        Strcpy(outbuf, "两次");
        break;
    case 3:
        Strcpy(outbuf, "三次");
        break;
    }
    return outbuf;
}

void
enlightenment(
    int mode,  /* BASICENLIGHTENMENT | MAGICENLIGHTENMENT (| both) */
    int final) /* ENL_GAMEINPROGRESS:0, ENL_GAMEOVERALIVE, ENL_GAMEOVERDEAD */
{
    char buf[BUFSZ], tmpbuf[BUFSZ];

    ge.en_win = create_nhwindow(NHW_MENU);
    ge.en_via_menu = !final;
    if (ge.en_via_menu)
        start_menu(ge.en_win, MENU_BEHAVE_STANDARD);

    Strcpy(tmpbuf, svp.plname);
    *tmpbuf = highc(*tmpbuf); /* same adjustment as bottom line */
    /* as in background_enlightenment, when poly'd we need to use the saved
       gender in u.mfemale rather than the current you-as-monster gender */
    Snprintf(buf, sizeof(buf), "%s(%s)的属性:", tmpbuf,
             ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
                ? gu.urole.name.f
                : gu.urole.name.m);

    /* title */
    enlght_out(buf); /* "Conan the Archeologist's attributes:" */
    /* background and characteristics; ^X or end-of-game disclosure */
    if (mode & BASICENLIGHTENMENT) {
        /* role, race, alignment, deities, dungeon level, time, experience */
        background_enlightenment(mode, final);
        /* hit points, energy points, armor class, gold */
        basics_enlightenment(mode, final);
        /* strength, dexterity, &c */
        characteristics_enlightenment(mode, final);
    }
    /* expanded status line information, including things which aren't
       included there due to space considerations;
       shown for both basic and magic enlightenment */
    status_enlightenment(mode, final);
    /* remaining attributes; shown for potion,&c or wizard mode and
       explore mode ^X or end of game disclosure */
    if (mode & MAGICENLIGHTENMENT) {
        /* intrinsics and other traditional enlightenment feedback */
        attributes_enlightenment(mode, final);
    }

    enlght_out(""); /* separator */
    enlght_out("杂项:");
    /* reminder to player and/or information for dumplog */
    if ((mode & BASICENLIGHTENMENT) != 0 && (wizard || discover || final)) {
        if (wizard || discover) {
            Sprintf(buf, "以%s模式运行", wizard ? "调试" : "探索");
            you_are(buf, "");
        }

        if (!flags.bones) {
            /* mention not saving bones iff hero just died */
            Sprintf(buf, "已禁用加载%s骨头关卡",
                    (final == ENL_GAMEOVERDEAD) ? "和存储" : "");
            you_have_X(buf);
        } else if (!u.uroleplay.numbones) {
            enl_msg(You_, "尚未遭遇", "未遭遇", "任何骨头层", "");
        } else {
            Sprintf(buf, "遭遇了 %ld 个骨头层", u.uroleplay.numbones);
            you_have_X(buf);
        }
    }
    (void) fmt_elapsed_time(buf, final);
    enl_msg("总游戏耗时", "为", "为", buf, "");

    if (!ge.en_via_menu) {
        display_nhwindow(ge.en_win, TRUE);
    } else {
        menu_item *selected = 0;

        end_menu(ge.en_win, (char *) 0);
        if (select_menu(ge.en_win, PICK_NONE, &selected) > 0)
            free((genericptr_t) selected);
        ge.en_via_menu = FALSE;
    }
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*ARGSUSED*/
/* display role, race, alignment and such to en_win */
staticfn void
background_enlightenment(int unused_mode UNUSED, int final)
{
    const char *role_titl, *rank_titl;
    int innategend, difgend, difalgn;
    char buf[BUFSZ], tmpbuf[BUFSZ];

    /* note that if poly'd, we need to use u.mfemale instead of flags.female
       to access hero's saved gender-as-human/elf/&c rather than current */
    innategend = (Upolyd ? u.mfemale : flags.female) ? 1 : 0;
    role_titl = (innategend && gu.urole.name.f) ? gu.urole.name.f
                                                : gu.urole.name.m;
    rank_titl = rank_of(u.ulevel, Role_switch, innategend);

    enlght_out(""); /* separator after title */
    enlght_out("背景:");

    /* if polymorphed, report current shape before underlying role;
       will be repeated as first status: "you are transformed" and also
       among various attributes: "you are in beast form" (after being
       told about lycanthropy) or "you are polymorphed into <a foo>"
       (with countdown timer appended for wizard mode); we really want
       the player to know he's not a samurai at the moment... */
    if (Upolyd) {
        char anbuf[20]; /* includes trailing space; [4] suffices */
        struct permonst *uasmon = gy.youmonst.data;
        boolean altphrasing = vampshifted(&gy.youmonst);

        tmpbuf[0] = '\0';
        /* here we always use current gender, not saved role gender */
        if (!is_male(uasmon) && !is_female(uasmon) && !is_neuter(uasmon))
            Sprintf(tmpbuf, "%s ", genders[flags.female ? 1 : 0].adj);
        if (altphrasing)
            Sprintf(eos(tmpbuf), "%s的",
                    pmname(&mons[gy.youmonst.cham],
                           flags.female ? FEMALE : MALE));
        Snprintf(buf, sizeof(buf), "%s%s%s%s形态",
                 !final ? "当前" : "",
                 altphrasing ? just_an(anbuf, tmpbuf) : "处于",
                 tmpbuf, pmname(uasmon, flags.female ? FEMALE : MALE));
        you_are(buf, "");
    }

    /* report role; omit gender if it's redundant (eg, "female priestess") */
    tmpbuf[0] = '\0';
    if (!gu.urole.name.f
        && ((gu.urole.allow & ROLE_GENDMASK) == (ROLE_MALE | ROLE_FEMALE)
            || innategend != flags.initgend))
        Sprintf(tmpbuf, "%s ", genders[innategend].adj);
    buf[0] = '\0';
    if (Upolyd)
        Strcpy(buf, "实际上 "); /* "You are actually a ..." */
    if (!strcmpi(rank_titl, role_titl)) {
        /* omit role when rank title matches it */
        Sprintf(eos(buf), "一位%s, 等级%d %s%s", an(rank_titl), u.ulevel,
                tmpbuf, gu.urace.noun);
    } else {
        Sprintf(eos(buf), "一位%s, 等级%d %s%s%s", an(rank_titl), u.ulevel,
                tmpbuf, gu.urace.adj, role_titl);
    }
    you_are(buf, "");

    /* report alignment (bypass you_are() in order to omit ending period);
       adverb is used to distinguish between temporary change (helm of opp.
       alignment), permanent change (one-time conversion), and original */
    Sprintf(buf, "  %s%s%s阵营, %s肩负着%s的使命",
            You_, !final ? "是" : "曾是",
            align_str(u.ualign.type),
            /* helm of opposite alignment (might hide conversion) */
            (u.ualign.type != u.ualignbase[A_CURRENT])
               /* what's the past tense of "currently"? if we used "formerly"
                  it would sound like a reference to the original alignment */
               ? (!final ? "当前" : "临时")
               /* permanent conversion */
               : (u.ualign.type != u.ualignbase[A_ORIGINAL])
                  /* and what's the past tense of "now"? certainly not "then"
                     in a context like this...; "belatedly" == weren't that
                     way sooner (in other words, didn't start that way) */
                  ? (!final ? "现在" : "最近才")
                  /* atheist (ignored in very early game) */
                  : (!u.uconduct.gnostic && svm.moves > 1000L)
                     ? "名义上"
                     /* lastly, normal case */
                     : "",
            u_gname());
    enlght_out(buf);
    /* show the rest of this game's pantheon (finishes previous sentence)
       [appending "also Moloch" at the end would allow for straightforward
       trailing "and" on all three aligned entries but looks too verbose] */
    Sprintf(buf, "  %s敌对的是", !final ? "正" : "曾");
    if (u.ualign.type != A_LAWFUL)
        Sprintf(eos(buf), "%s(%s)和", align_gname(A_LAWFUL),
                align_str(A_LAWFUL));
    if (u.ualign.type != A_NEUTRAL)
        Sprintf(eos(buf), "%s(%s)%s", align_gname(A_NEUTRAL),
                align_str(A_NEUTRAL),
                (u.ualign.type != A_CHAOTIC) ? "和" : "");
    if (u.ualign.type != A_CHAOTIC)
        Sprintf(eos(buf), "%s(%s)", align_gname(A_CHAOTIC),
                align_str(A_CHAOTIC));
    Strcat(buf, "."); /* terminate sentence */
    enlght_out(buf);

    /* show original alignment,gender,race,role if any have been changed;
       giving separate message for temporary alignment change bypasses need
       for tricky phrasing otherwise necessitated by possibility of having
       helm of opposite alignment mask a permanent alignment conversion */
    difgend = (innategend != flags.initgend);
    difalgn = (((u.ualign.type != u.ualignbase[A_CURRENT]) ? 1 : 0)
               + ((u.ualignbase[A_CURRENT] != u.ualignbase[A_ORIGINAL])
                  ? 2 : 0));
    if (difalgn & 1) { /* have temporary alignment so report permanent one */
        Sprintf(buf, "实际上是%s阵营", align_str(u.ualignbase[A_CURRENT]));
        you_are(buf, "");
        difalgn &= ~1; /* suppress helm from "started out <foo>" message */
    }
    if (difgend || difalgn) { /* sex change or perm align change or both */
        Sprintf(buf, "  你最开始是%s%s%s.",
                difgend ? genders[flags.initgend].adj : "",
                (difgend && difalgn) ? "和" : "",
                difalgn ? align_str(u.ualignbase[A_ORIGINAL]) : "");
        enlght_out(buf);
    }

    /* "You are left-handed." won't work well if polymorphed into something
       without hands; use "You are normally left-handed." in that situation */
    Sprintf(buf, "%s惯用%s手",
            !strcmp(body_part(HANDED), "有手") ? "" : "通常",
            URIGHTY ? "右" : "左");
    you_are(buf, "");

    /* As of 3.6.2: dungeon level, so that ^X really has all status info as
       claimed by the comment below; this reveals more information than
       the basic status display, but that's one of the purposes of ^X;
       similar information is revealed by #overview; the "You died in
       <location>" given by really_done() is more rudimentary than this */
    *buf = *tmpbuf = '\0';
    if (In_endgame(&u.uz)) {
        int egdepth = observable_depth(&u.uz);

        (void) endgamelevelname(tmpbuf, egdepth);
        Snprintf(buf, sizeof(buf), "位于终局的%s%s",
                 !strncmp(tmpbuf, "Plane", 5) ? "元素" : "", tmpbuf);
    } else if (Is_knox(&u.uz)) {
        /* this gives away the fact that the knox branch is only 1 level */
        Sprintf(buf, "在%s层", svd.dungeons[u.uz.dnum].dname);
        /* TODO? maybe phrase it differently when actually inside the fort,
           if we're able to determine that (not trivial) */
    } else {
        char dgnbuf[QBUFSZ];

        Strcpy(dgnbuf, svd.dungeons[u.uz.dnum].dname);
        if (!strncmpi(dgnbuf, "The ", 4))
            *dgnbuf = lowc(*dgnbuf);
        Sprintf(tmpbuf, "%d层",
                In_quest(&u.uz) ? dunlev(&u.uz) : depth(&u.uz));
        /* TODO? maybe extend this bit to include various other automatic
           annotations from the dungeon overview code */
        if (Is_rogue_level(&u.uz))
            Strcat(tmpbuf, ", 一个原始的区域");
        else if (Is_bigroom(&u.uz) && !Blind)
            Strcat(tmpbuf, ", 一个非常大的房间");
        Snprintf(buf, sizeof(buf), "位于%s的%s", dgnbuf, tmpbuf);
    }
    you_are(buf, "");

    /* this is shown even if the 'time' option is off */
    if (svm.moves == 1L) {
        you_have_X("刚刚开始冒险");
    } else {
        /* 'turns' grates on the nerves in this context... */
        Sprintf(buf, "%ld 回合前进入地牢", svm.moves);
        /* same phrasing for current and final: "entered" is unconditional */
        enlght_line(You_, "", buf, "");
    }

    /* for gameover, these have been obtained in really_done() so that they
       won't vary if user leaves a disclosure prompt or --More-- unanswered
       long enough for the dynamic value to change between then and now */
    if (final ? iflags.at_midnight : midnight()) {
        enl_msg("现在", "是", "曾是", "午夜时分", "");
    } else if (final ? iflags.at_night : night()) {
        enl_msg("现在", "是", "曾是", "夜间", "");
    }
    /* other environmental factors */
    if (flags.moonphase == FULL_MOON || flags.moonphase == NEW_MOON) {
        /* [This had "tonight" but has been changed to "in effect".
           There is a similar issue to Friday the 13th--it's the value
           at the start of the current session but that session might
           have dragged on for an arbitrary amount of time.  We want to
           report the values that currently affect play--or affected
           play when game ended--rather than actual outside situation.] */
        Sprintf(buf, "%s月%s生效",
                (flags.moonphase == FULL_MOON) ? "满月"
                : (flags.moonphase == NEW_MOON) ? "新月"
                  /* showing these would probably just lead to confusion
                     since they have no effect on game play... */
                  : (flags.moonphase < FULL_MOON) ? "上弦月"
                    : "下弦月",
                /* we don't have access to 'how' here--aside from survived
                   vs died--so settle for general platitude */
                final ? "在你的冒险结束时" : "");
        enl_msg("当时", "有", "曾有", buf, "");
    }
    if (flags.friday13) {
        /* let player know that friday13 penalty is/was in effect;
           we don't say "it is/was Friday the 13th" because that was at
           the start of the session and it might be past midnight (or
           days later if the game has been paused without save/restore),
           so phrase this similar to the start up message */
        Sprintf(buf, "  坏事%s13号星期五.",
                !final ? "会发生在"
                : (final == ENL_GAMEOVERALIVE) ? "会发生在"
                  /* there's no may to tell whether -1 Luck made a
                     difference but hero has died... */
                  : "发生在");
        enlght_out(buf);
    }

    if (!Upolyd) {
        int ulvl = (int) u.ulevel;
        /* [flags.showexp currently does not matter; should it?] */

        /* experience level is already shown above */
        Sprintf(buf, "%-1ld 经验点", u.uexp);
        /* TODO?
         *  Remove wizard-mode restriction since patient players can
         *  determine the numbers needed without resorting to spoilers
         *  (even before this started being disclosed for 'final';
         *  just enable 'showexp' and look at normal status lines
         *  after drinking gain level potions or eating wraith corpses
         *  or being level-drained by vampires).
         */
        if (ulvl < 30 && (final || wizard)) {
            long nxtlvl = newuexp(ulvl), delta = nxtlvl - u.uexp;

            Sprintf(eos(buf), ", 还需 %ld 经验点%s等级%d",
                    delta, (ulvl < 18) ? "以达到" : "用于", (ulvl + 1));
        }
        you_have(buf, "");
    }
#ifdef SCORE_ON_BOTL
    if (flags.showscore) {
        /* describes what's shown on status line, which is an approximation;
           only show it here if player has the 'showscore' option enabled */
        Sprintf(buf, "%ld%s", botl_score(),
                !final ? "" : " (未计入游戏结束调整)");
        enl_msg("你的分数", "是", "曾是", buf, "");
    }
#endif
}

/* hit points, energy points, armor class -- essential information which
   doesn't fit very well in other categories */
/*ARGSUSED*/
staticfn void
basics_enlightenment(int mode UNUSED, int final)
{
    static char Power[] = "能量点(法力)";
    char buf[BUFSZ];
    int pw = u.uen, hp = (Upolyd ? u.mh : u.uhp),
        pwmax = u.uenmax, hpmax = (Upolyd ? u.mhmax : u.uhpmax);

    enlght_out(""); /* separator after background */
    enlght_out("基础:");

    if (hp < 0)
        hp = 0;
    /* "1 out of 1" rather than "all" if max is only 1; should never happen */
    if (hp == hpmax && hpmax > 1)
        Sprintf(buf, "全部 %d 点生命值", hpmax);
    else
        Sprintf(buf, "%d / %d 点生命值", hp, hpmax);
    you_have(buf, "");

    /* low max energy is feasible, so handle couple of extra special cases */
    if (pwmax == 0 || (pw == pwmax && pwmax == 2)) /* both: not "all 2" */
        Sprintf(buf, "%s%s", !pwmax ? "无" : "全部 ", Power);
    else if (pw == pwmax && pwmax > 2)
        Sprintf(buf, "%d %s", pwmax, Power);
    else
        Sprintf(buf, "%d/%d %s", pw, pwmax, Power);
    you_have(buf, "");

    if (Upolyd) {
        switch (mons[u.umonnum].mlevel) {
        case 0:
            /* status line currently being explained shows "HD:0" */
            Strcpy(buf, "0 生命骰 (实际1/2)");
            break;
        case 1:
            Strcpy(buf, "1 生命骰");
            break;
        default:
            Sprintf(buf, "%d 生命骰", mons[u.umonnum].mlevel);
            break;
        }
        you_have(buf, "");
    }

    find_ac(); /* enforces AC_MAX cap */
    Sprintf(buf, "%d", u.uac);
    if (abs(u.uac) == AC_MAX)
        Sprintf(eos(buf), ", 可能达到的 %s",
                (u.uac < 0) ? "最佳" : "最差");
    enl_msg("你的护甲等级", "是", "曾是", buf, "");

    /* gold; similar to doprgold (#showgold) but without shop billing info;
       includes container contents, unlike status line but like doprgold */
    {
        long umoney = money_cnt(gi.invent), hmoney = hidden_gold(final);

        if (!umoney) {
            Sprintf(buf, " 你的钱包%s空的", !final ? "是" : "曾是");
        } else {
            Sprintf(buf, " 你的钱包装%s %ld %s", !final ? "着" : "了",
                    umoney, currency(umoney));
        }
        /* terminate the wallet line if appropriate, otherwise add an
           introduction to subsequent continuation; output now either way */
        Strcat(buf, !hmoney ? "." : !umoney ? ", 但是" : ", 并且");
        enlght_out(buf);

        /* put contained gold on its own line to avoid excessive width; it's
           phrased as a continuation of the wallet line so not capitalized */
        if (hmoney) {
            Sprintf(buf, "%ld %s 藏在你的背包里",
                    hmoney, umoney ? "更多" : currency(hmoney));
            enl_msg("你", "还有", "曾还有", buf, "");
        }
    }

    if (flags.pickup) {
        char ocl[MAXOCLASSES + 1];

        Strcpy(buf, "开着");
        if (costly_spot(u.ux, u.uy)) {
            /* being in a shop inhibits autopickup, even 'pickup_thrown' */
            Strcat(buf, ", 但在商店内暂时禁用");
        } else {
            oc_to_str(flags.pickup_types, ocl);
            Sprintf(eos(buf), ", 拾取%s%s%s", *ocl ? "'" : "",
                    *ocl ? ocl : "所有类型", *ocl ? "'" : "");
            if (flags.pickup_thrown && *ocl)
                Strcat(buf, "以及投掷"); /* show when not 'all types' */
            if (ga.apelist)
                Strcat(buf, ", 除了例外");
        }
    } else
        Strcpy(buf, "关着");
    enl_msg("自动拾取", "是", "曾是", buf, "");
}

/* characteristics: expanded version of bottom line strength, dexterity, &c */
staticfn void
characteristics_enlightenment(int mode, int final)
{
    char buf[BUFSZ];

    enlght_out("");
    Sprintf(buf, "%s属性:", !final ? "" : "最终");
    enlght_out(buf);

    /* bottom line order */
    one_characteristic(mode, final, A_STR); /* strength */
    one_characteristic(mode, final, A_DEX); /* dexterity */
    one_characteristic(mode, final, A_CON); /* constitution */
    one_characteristic(mode, final, A_INT); /* intelligence */
    one_characteristic(mode, final, A_WIS); /* wisdom */
    one_characteristic(mode, final, A_CHA); /* charisma */
}

/* display one attribute value for characteristics_enlightenment() */
staticfn void
one_characteristic(int mode, int final, int attrindx)
{
    extern const char *const attrname[]; /* attrib.c */
    boolean hide_innate_value = FALSE, interesting_alimit;
    int acurrent, abase, apeak, alimit;
    const char *paren_pfx;
    char subjbuf[BUFSZ], valubuf[BUFSZ], valstring[32];

    /* being polymorphed or wearing certain cursed items prevents
       hero from reliably tracking changes to characteristics so
       we don't show base & peak values then; when the items aren't
       cursed, hero could take them off to check underlying values
       and we show those in such case so that player doesn't need
       to actually resort to doing that */
    if (Upolyd) {
        hide_innate_value = TRUE;
    } else if (Fixed_abil) {
        if (stuck_ring(uleft, RIN_SUSTAIN_ABILITY)
            || stuck_ring(uright, RIN_SUSTAIN_ABILITY))
            hide_innate_value = TRUE;
    }
    switch (attrindx) {
    case A_STR:
        if (uarmg && uarmg->otyp == GAUNTLETS_OF_POWER && uarmg->cursed)
            hide_innate_value = TRUE;
        break;
    case A_DEX:
        break;
    case A_CON:
        if (u_wield_art(ART_OGRESMASHER) && uwep->cursed)
            hide_innate_value = TRUE;
        break;
    case A_INT:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_WIS:
        if (uarmh && uarmh->otyp == DUNCE_CAP && uarmh->cursed)
            hide_innate_value = TRUE;
        break;
    case A_CHA:
        break;
    default:
        return; /* impossible */
    };
    /* note: final disclosure includes MAGICENLIGHTENTMENT */
    if ((mode & MAGICENLIGHTENMENT) && !Upolyd)
        hide_innate_value = FALSE;

    acurrent = ACURR(attrindx);
    (void) attrval(attrindx, acurrent, valubuf); /* Sprintf(valubuf,"%d",) */
    Sprintf(subjbuf, "你的%s", attrname[attrindx]);

    if (!hide_innate_value) {
        /* show abase, amax, and/or attrmax if acurr doesn't match abase
           (a magic bonus or penalty is in effect) or abase doesn't match
           amax (some points have been lost to poison or exercise abuse
           and are restorable) or attrmax is different from normal human
           (while game is in progress; trying to reduce dependency on
           spoilers to keep track of such stuff) or attrmax was different
           from abase (at end of game; this attribute wasn't maxed out) */
        abase = ABASE(attrindx);
        apeak = AMAX(attrindx);
        alimit = ATTRMAX(attrindx);
        /* criterium for whether the limit is interesting varies */
        interesting_alimit =
            final ? TRUE /* was originally `(abase != alimit)' */
                  : (alimit != (attrindx != A_STR ? 18 : STR18(100)));
        paren_pfx = final ? " (" : " (当前; ";
        if (acurrent != abase) {
            Sprintf(eos(valubuf), "%s基础:%s", paren_pfx,
                    attrval(attrindx, abase, valstring));
            paren_pfx = ", ";
        }
        if (abase != apeak) {
            Sprintf(eos(valubuf), "%s最高:%s", paren_pfx,
                    attrval(attrindx, apeak, valstring));
            paren_pfx = ", ";
        }
        if (interesting_alimit) {
            Sprintf(eos(valubuf), "%s%s极限:%s", paren_pfx,
                    /* more verbose if exceeding 'limit' due to magic bonus */
                    (acurrent > alimit) ? "天生 " : "",
                    attrval(attrindx, alimit, valstring));
            /* paren_pfx = ", "; */
        }
        if (acurrent != abase || abase != apeak || interesting_alimit)
            Strcat(valubuf, ")");
    }
    enl_msg(subjbuf, "是", "曾是", valubuf, "");
}

/* status: selected obvious capabilities, assorted troubles */
staticfn void
status_enlightenment(int mode, int final)
{
    boolean magic = (mode & MAGICENLIGHTENMENT) ? TRUE : FALSE;
    int cap;
    char buf[BUFSZ], youtoo[BUFSZ], heldmon[BUFSZ];
    boolean Riding = (u.usteed
                      /* if hero dies while dismounting, u.usteed will still
                         be set; we want to ignore steed in that situation */
                      && !(final == ENL_GAMEOVERDEAD
                           && !strcmp(svk.killer.name, "riding accident")));
    const char *steedname = (!Riding ? (char *) 0
                      : x_monnam(u.usteed,
                                 u.usteed->mtame ? ARTICLE_YOUR : ARTICLE_THE,
                                 (char *) 0,
                                 (SUPPRESS_SADDLE | SUPPRESS_HALLUCINATION),
                                 FALSE));

    /*\
     * Status (many are abbreviated on bottom line; others are or
     *     should be discernible to the hero hence to the player)
    \*/
    enlght_out(""); /* separator after title or characteristics */
    enlght_out(final ? "最终状态:" : "状态:");

    Strcpy(youtoo, You_);
    /* not a traditional status but inherently obvious to player; more
       detail given below (attributes section) for magic enlightenment */
    if (Upolyd) {
        Strcpy(buf, "已经变形");
        if (ugenocided())
            Sprintf(eos(buf), "且内心%s", udeadinside());
        you_are(buf, "");
    }
    /* not a trouble, but we want to display riding status before maybe
       reporting steed as trapped or hero stuck to cursed saddle */
    if (Riding) {
        Sprintf(buf, "骑着%s", steedname);
        you_are(buf, "");
        Sprintf(eos(youtoo), "和%s", steedname);
    }
    /* other movement situations that hero should always know */
    if (Levitation) {
        if (Lev_at_will && magic)
            you_are("随意漂浮", "");
        else
            enl_msg(youtoo, are, were, "漂浮", from_what(LEVITATION));
    } else if (Flying) { /* can only fly when not levitating */
        enl_msg(youtoo, are, were, "飞行", from_what(FLYING));
    }
    if (Underwater) {
        you_are("在水下", "");
    } else if (u.uinwater) {
        you_are(Swimming ? "游泳" : "在水中", from_what(SWIMMING));
    } else if (walking_on_water()) {
        /* show active Wwalking here, potential Wwalking elsewhere */
        Sprintf(buf, "走在%s上",
                is_pool(u.ux, u.uy) ? "水面"
                : is_lava(u.ux, u.uy) ? "熔岩"
                  : surface(u.ux, u.uy)); /* catchall; shouldn't happen */
        you_are(buf, from_what(WWALKING));
    }
    if (Upolyd && (u.uundetected || U_AP_TYPE != M_AP_NOTHING))
        youhiding(TRUE, final);

    /* internal troubles, mostly in the order that prayer ranks them */
    if (Stoned) {
        if (final && (Stoned & I_SPECIAL))
            enlght_out(" 你变成了石头.");
        else
            you_are("正在变成石头", "");
    }
    if (Slimed) {
        if (final && (Slimed & I_SPECIAL))
            enlght_out(" 你变成了黏液.");
        else
            you_are("正在变成黏液", "");
    }
    if (Strangled) {
        if (u.uburied) {
            you_are("被埋着", "");
        } else {
            if (final && (Strangled & I_SPECIAL)) {
                enlght_out(" 你死于窒息.");
            } else {
                Strcpy(buf, "被窒息");
                if (wizard)
                    Sprintf(eos(buf), " (%ld)", (Strangled & TIMEOUT));
                you_are(buf, from_what(STRANGLED));
            }
        }
    }
    if (Sick) {
        /* the two types of sickness are lumped together; hero can be
           afflicted by both but there is only one timeout; botl status
           puts TermIll before FoodPois and death due to timeout reports
           terminal illness if both are in effect, so do the same here */
        if (final && (Sick & I_SPECIAL)) {
            Sprintf(buf, "  %s死于%s.", You_, /* has trailing space */
                    (u.usick_type & SICK_NONVOMITABLE)
                    ? "疾病晚期" : "食物中毒");
            enlght_out(buf);
        } else {
            /* unlike death due to sickness, report the two cases separately
               because it is possible to cure one without curing the other */
            if (u.usick_type & SICK_NONVOMITABLE)
                you_are("因疾病身患绝症", "");
            if (u.usick_type & SICK_VOMITABLE)
                you_are("因食物中毒身患绝症", "");
        }
    }
    if (Vomiting)
        you_are("恶心", "");
    if (Stunned)
        you_are("眩晕", "");
    if (Confusion)
        you_are("混乱", "");
    if (Hallucination)
        you_are("产生幻觉", "");
    if (Blind) {
        /* check the reasons in same order as from_what() */
        Sprintf(buf, "%s失明",
                (HBlinded & FROMOUTSIDE) != 0L ? "永久"
                : (HBlinded & FROMFORM) ? "天生"
                  /* better phrasing desperately wanted... */
                  : Blindfolded_only ? "故意"
                    /* timed, possibly combined with blindfold */
                    : "暂时");
        if (wizard && (HBlinded == BlindedTimeout && !Blindfolded))
            Sprintf(eos(buf), " (%ld)", BlindedTimeout);
        /* !haseyes: avoid "you are innately blind innately" */
        you_are(buf, !haseyes(gy.youmonst.data) ? "" : from_what(BLINDED));
    }
    if (Deaf)
        you_are("失聪", from_what(DEAF));

    /* external troubles, more or less */
    if (Punished) {
        if (uball) {
            Sprintf(buf, "被拴在 %s 上", ansimpleoname(uball));
        } else {
            impossible("Punished without uball?");
            Strcpy(buf, "拴着埋着的什么东西");
        }
        you_are(buf, "");
    }
    if (u.utrap) {
        char predicament[BUFSZ];
        boolean anchored = (u.utraptype == TT_BURIEDBALL);

        (void) trap_predicament(predicament, final, wizard);
        if (u.usteed) { /* not `Riding' here */
            Sprintf(buf, "%s%s", anchored ? "你和" : "", steedname);
            enl_msg(buf, (anchored ? "是" : "是"),
                    (anchored ? "曾是" : "曾是"), predicament, "");
        } else
            you_are(predicament, "");
    } /* (u.utrap) */
    heldmon[0] = '\0'; /* lint suppression */
    if (u.ustuck) { /* includes u.uswallow */
        Strcpy(heldmon, a_monnam(u.ustuck));
        if (!strcmp(heldmon, "it")
            && (!has_mgivenname(u.ustuck)
                || strcmp(MGIVENNAME(u.ustuck), "it") != 0))
            Strcpy(heldmon, "一个看不见的生物");
    }
    if (u.uswallow) {
        assert(u.ustuck != NULL); /* implied by u.uswallow */
        Snprintf(buf, sizeof buf, "被%s%s",
                digests(u.ustuck->data) ? "吞下并消化于" : "吞没于",
                heldmon);
        if (dmgtype(u.ustuck->data, AD_DGST)) {
            /* if final, death via digestion can be deduced by u.uswallow
               still being True and u.uswldtim having been decremented to 0 */
            if (final && !u.uswldtim)
                Strcat(buf, " 并完全被消化了");
            else
                Sprintf(eos(buf), "并且%s正在被消化",
                        final ? "曾" : "");
        }
        if (wizard)
            Sprintf(eos(buf), " (%u)", u.uswldtim);
        you_are(buf, "");
    } else if (u.ustuck) {
        boolean ustick = (Upolyd && sticks(gy.youmonst.data));
        int dx = u.ustuck->mx - u.ux, dy = u.ustuck->my - u.uy;

        Snprintf(buf, sizeof buf, "%s%s (%s)",
                 ustick ? "正抓住" : "被抓住于",
                 heldmon, dxdy_to_dist_descr(dx, dy, TRUE));
        you_are(buf, "");
    }
    if (Riding) {
        struct obj *saddle = which_armor(u.usteed, W_SADDLE);

        if (saddle && saddle->cursed) {
            Sprintf(buf, "粘在%s的%s上", s_suffix(steedname),
                    simpleonames(saddle));
            you_are(buf, "");
        }
    }
    if (Wounded_legs) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long whichleg = (EWounded_legs & BOTH_SIDES);
        const char *bp = u.usteed ? mbodypart(u.usteed, LEG) : body_part(LEG),
            *article = "", /* precedes "wounded", so never "an " */
            *leftright = "";

        if (whichleg == BOTH_SIDES)
            bp = makeplural(bp), article = "";
        else
            leftright = (whichleg == LEFT_SIDE) ? "左" : "右";
        Sprintf(buf, "%s受伤的%s%s", article, leftright, bp);

        /* when mounted, Wounded_legs applies to steed rather than to
           hero; we only report steed's wounded legs in wizard mode */
        if (u.usteed) { /* not `Riding' here */
            if (wizard && steedname) {
                char steednambuf[BUFSZ];

                Strcpy(steednambuf, steedname);
                enl_msg(steednambuf, "有", "曾有", buf, "");
            }
        } else {
            you_have(buf, "");
        }
    }
    if (Glib) {
        Sprintf(buf, "%s很滑", fingers_or_gloves(TRUE));
        if (wizard)
            Sprintf(eos(buf), " (%ld)", (Glib & TIMEOUT));
        you_have(buf, "");
    }
    if (Fumbling) {
        if (magic || cause_known(FUMBLING))
            enl_msg(You_, "", "", "笨手笨脚", from_what(FUMBLING));
    }
    if (Sleepy) {
        if (magic || cause_known(SLEEPY)) {
            Strcpy(buf, from_what(SLEEPY));
            if (wizard)
                Sprintf(eos(buf), " (%ld)", (HSleepy & TIMEOUT));
            enl_msg(You_, "会", "曾会", "无法控制地睡着", buf);
        }
    }
    /* hunger/nutrition */
    if (Hunger) {
        if (magic || cause_known(HUNGER))
            enl_msg(You_, "会", "曾会", "快速饥饿",
                    from_what(HUNGER));
    }
    Strcpy(buf, hu_stat[u.uhs]); /* hunger status; omitted if "normal" */
    mungspaces(buf);             /* strip trailing spaces */
    /* status line doesn't show hunger when state is "not hungry", we do;
       needed for wizard mode's reveal of u.uhunger but add it for everyone */
    if (!*buf)
        Strcpy(buf, "不饿");
    if (*buf) { /* (since "not hungry" was added, this will always be True) */
        *buf = lowc(*buf); /* override capitalization */
        if (!strcmp(buf, "weak"))
            Strcat(buf, "因过度饥饿");
        else if (!strncmp(buf, "faint", 5)) /* fainting, fainted */
            Strcat(buf, " 因饥饿");
        if (wizard)
            Sprintf(eos(buf), " <%d>", u.uhunger);
        you_are(buf, "");
    }
    /* encumbrance */
    if ((cap = near_capacity()) > UNENCUMBERED) {
        const char *adj = "?_?"; /* (should always get overridden) */

        Strcpy(buf, enc_stat[cap]);
        *buf = lowc(*buf);
        switch (cap) {
        case SLT_ENCUMBER:
            adj = "略微减慢";
            break; /* burdened */
        case MOD_ENCUMBER:
            adj = "中度减慢";
            break; /* stressed */
        case HVY_ENCUMBER:
            adj = "严重减慢";
            break; /* strained */
        case EXT_ENCUMBER:
            adj = "极度减慢";
            break; /* overtaxed */
        case OVERLOADED:
            adj = "无法进行";
            break;
        }
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        Sprintf(eos(buf), "; 移动%s%s", !final ? "" : "曾", adj);
        you_are(buf, "");
    } else {
        /* last resort entry, guarantees Status section is non-empty
           (no longer needed for that purpose since weapon status added;
           still useful though) */
        Strcpy(buf, "未负重");
        if (wizard)
            Sprintf(eos(buf), " <%d>", inv_weight());
        you_are(buf, "");
    }
    /* current weapon(s) and corresponding skill level(s) */
    weapon_insight(final);
    /* unlike ring of increase accuracy's effect, the monk's suit penalty
       is too blatant to be restricted to magical enlightenment */
    if (iflags.tux_penalty && !Upolyd) {
        (void) enlght_combatinc("to hit", -gu.urole.spelarmr, final, buf);
        /* if from_what() ever gets extended from wizard mode to normal
           play, it could be adapted to handle this */
        Sprintf(eos(buf), " 因你的%s", suit_simple_name(uarm));
        you_have(buf, "");
    }
    /* report 'nudity' */
    if (!uarm && !uarmu && !uarmc && !uarms && !uarmg && !uarmf && !uarmh) {
        if (u.uroleplay.nudist)
            enl_msg(You_, "", "", "不穿任何盔甲", "");
        else
            you_are("没有穿任何盔甲", "");
    }
}

/* extracted from status_enlightenment() to reduce clutter there */
staticfn void
weapon_insight(int final)
{
    char buf[BUFSZ];
    int wtype;

    /* report being weaponless; distinguish whether gloves are worn
       [perhaps mention silver ring(s) when not wearing gloves?] */
    if (!uwep) {
        you_are(empty_handed(), "");

    /* two-weaponing implies hands and
       a weapon or wep-tool (not other odd stuff) in each hand */
    } else if (u.twoweap) {
        you_are("同时挥舞两件武器", "");

    /* report most weapons by their skill class (so a katana will be
       described as a long sword, for instance; mattock, hook, and aklys
       are exceptions), or wielded non-weapon item by its object class */
    } else {
        const char *what = weapon_descr(uwep);

        /* [what about other silver items?] */
        if (uwep->otyp == SHIELD_OF_REFLECTION)
            what = shield_simple_name(uwep); /* silver|smooth shield */
        else if (is_wet_towel(uwep))
            what = /* (uwep->spe < 3) ? "潮湿毛巾" : */ "湿毛巾";

        if (!strcmpi(what, "armor") || !strcmpi(what, "food")
            || !strcmpi(what, "venom"))
            Sprintf(buf, "手持着一些 %s", what);
        else
            /* [maybe include known blessed?] */
            Sprintf(buf, "挥舞%s",
                    (uwep->quan == 1L) ? an(what) : makeplural(what));
        you_are(buf, "");
    }

    /*
     * Skill with current weapon.  Might help players who've never
     * noticed #enhance or decided that it was pointless.
     */
    if ((wtype = weapon_type(uwep)) != P_NONE && (!uwep || !is_ammo(uwep))) {
        char sklvlbuf[20];
        int sklvl = P_SKILL(wtype);
        boolean hav = (sklvl != P_UNSKILLED && sklvl != P_SKILLED);

        if (sklvl == P_ISRESTRICTED)
            Strcpy(sklvlbuf, "无");
        else
            (void) lcase(skill_level_name(wtype, sklvlbuf));
        /* "you have no/basic/expert/master/grand-master skill with <skill>"
           or "you are unskilled/skilled in <skill>" */
        Sprintf(buf, "%s %s %s", sklvlbuf,
                hav ? "技能" : "于", skill_name(wtype));

        if (!u.twoweap) {
            if (can_advance(wtype, FALSE))
                Sprintf(eos(buf), ", 并且%s提升",
                        !final ? "可以" : "本可以");
            if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

        } else { /* two-weapon */
            static const char also_[] = "也";
            char pfx[QBUFSZ], sfx[QBUFSZ],
                sknambuf2[20], sklvlbuf2[20], twobuf[20];
            const char *also = "", *also2 = "", *also3 = (char *) 0,
                       *verb_present, *verb_past;
            int wtype2 = weapon_type(uswapwep),
                sklvl2 = P_SKILL(wtype2),
                twoskl = P_SKILL(P_TWO_WEAPON_COMBAT);
            boolean a1, a2, ab,
                    hav2 = (sklvl2 != P_UNSKILLED && sklvl2 != P_SKILLED);

            /* normally hero must have access to two-weapon skill in
               order to initiate u.twoweap, but not if polymorphed into
               a form which has multiple weapon attacks, so we need to
               avoid getting bitten by unexpected skill value */
            if (twoskl == P_ISRESTRICTED) {
                twoskl = P_UNSKILLED;
                /* restricted is the same as unskilled as far as bonus
                   or penalty goes, and it isn't ordinarily seen so
                   skill_level_name() returns "Unknown" for it */
                Strcpy(twobuf, "受限");
            } else {
                (void) lcase(skill_level_name(P_TWO_WEAPON_COMBAT, twobuf));
            }

            /* keep buf[] from above in case skill levels match */
            pfx[0] = sfx[0] = '\0';
            if (twoskl < sklvl) {
                /* twoskil won't be restricted so sklvl is at least basic */
                Sprintf(pfx, "你在%s方面的技能", skill_name(wtype));
                Sprintf(sfx, "受限于%s双武器战斗技能", twobuf);
                also = also_;
            } else if (twoskl > sklvl) {
                /* sklvl might be restricted */
                Strcpy(pfx, "你的双武器技能");
                Strcpy(sfx, "受限于");
                if (sklvl > P_ISRESTRICTED)
                    Sprintf(eos(sfx), "%s等级", sklvlbuf);
                else
                    Sprintf(eos(sfx), "没有技能");
                Sprintf(eos(sfx), "的%s", skill_name(wtype));
                also2 = also_;
            } else {
                Strcat(buf, "和双武器战斗");
                also3 = also_;
            }
            if (*pfx)
                enl_msg(pfx, "是", "曾是", sfx, "");
            else if (hav)
                you_have(buf, "");
            else
                you_are(buf, "");

            /* skip comparison between secondary and two-weapons if it is
               identical to the comparison between primary and twoweap */
            if (wtype2 != wtype) {
                Strcpy(sknambuf2, skill_name(wtype2));
                (void) lcase(skill_level_name(wtype2, sklvlbuf2));
                verb_present = "是", verb_past = "曾是";
                pfx[0] = sfx[0] = buf[0] = '\0';
                if (twoskl < sklvl2) {
                    /* twoskil is at least unskilled, sklvl2 at least basic */
                    Sprintf(pfx, "你在%s上的技能", sknambuf2);
                    Sprintf(sfx, "%s因%s双武器战斗而受限",
                            also, twobuf);
                } else if (twoskl > sklvl2) {
                    /* sklvl2 might be restricted */
                    Strcpy(pfx, "你的双武器技能");
                    Sprintf(sfx, "%s受限于", also2);
                    if (sklvl2 > P_ISRESTRICTED)
                        Sprintf(eos(sfx), "%s等级", sklvlbuf2);
                    else
                        Strcat(eos(sfx), "没有技能");
                    Sprintf(eos(sfx), "的%s", sknambuf2);
                } else {
                    /* equal; two-weapon is at least unskilled, so sklvl2 is
                       too; "you [also] have basic/expert/master/grand-master
                       skill with <skill>" or "you [also] are unskilled/
                       skilled in <skill> */
                    Sprintf(buf, "%s %s %s", sklvlbuf2,
                            hav2 ? "的掌握" : "方面", sknambuf2);
                    Strcat(buf, "和双武器战斗");
                    if (also3) {
                        Strcpy(pfx, "你也");
                        Snprintf(sfx, sizeof(sfx), "%s", buf), buf[0] = '\0';
                        verb_present = hav2 ? "拥有" : "是";
                        verb_past = hav2 ? "曾拥有" : "曾是";
                    }
                }
                if (*pfx)
                    enl_msg(pfx, verb_present, verb_past, sfx, "");
                else if (hav2)
                    you_have(buf, "");
                else
                    you_are(buf, "");
            } /* wtype2 != wtype */

            /* if training and available skill credits already allow
               #enhance for any of primary, secondary, or two-weapon,
               tell the player; avoid attempting figure out whether
               spending skill credits enhancing one might make either
               or both of the others become ineligible for enhancement */
            a1 = can_advance(wtype, FALSE);
            a2 = (wtype2 != wtype) ? can_advance(wtype2, FALSE) : FALSE;
            ab = can_advance(P_TWO_WEAPON_COMBAT, FALSE);
            if (a1 || a2 || ab) {
                static const char also_wik_[] = "以及";

                /* for just one, the conditionals yield
                   1) "skill with <that one>"; for more than one:
                   2) "skills with <primary> and also with <secondary>" or
                   3) "skills with <primary> and also with two-weapons" or
                   4) "skills with <secondary> and also with two-weapons" or
                   5) "skills with <primary>, <secondary>, and two-weapons"
                   (no 'also's or extra 'with's for case 5); when primary
                   and secondary use the same skill, only cases 1 and 3 are
                   possible because 'a2' gets forced to False above */
                Sprintf(sfx, "提升%s%s%s%s%s的技能",
                        a1 ? skill_name(wtype) : "",
                        ((a1 && a2 && ab) ? ", "
                         : (a1 && (a2 || ab)) ? also_wik_ : ""),
                        a2 ? skill_name(wtype2) : "",
                        ((a1 && a2 && ab) ? ", "
                         : (a2 && ab) ? also_wik_ : ""),
                        ab ? "双武器" : "");
                enl_msg(You_, "可以", "本可以", sfx, "");
            }
        } /* two-weapon */
    } /* skill applies */
}

staticfn void
item_resistance_message(
    int adtyp,
    const char *prot_message,
    int final)
{
    int protection = u_adtyp_resistance_obj(adtyp);

    if (protection) {
        boolean somewhat = protection < 99;

        enl_msg("你的物品",
                somewhat ? "一定程度上" : "",
                somewhat ? "曾一定程度上" : "曾",
                prot_message, item_what(adtyp));
    }
}

/* attributes: intrinsics and the like, other non-obvious capabilities */
staticfn void
attributes_enlightenment(
    int unused_mode UNUSED,
    int final)
{
    static NEARDATA const char
        if_surroundings_permitted[] = "如果周围环境允许";
    int ltmp, armpro, warnspecies;
    char buf[BUFSZ];

    /*\
     *  Attributes
    \*/
    enlght_out("");
    enlght_out(final ? "最终特质:" : "特质:");

    if (u.uevent.uhand_of_elbereth) {
        static const char *const hofe_titles[3] = { "埃尔贝雷斯之手",
                                                    "平衡使者",
                                                    "阿里奥克之荣光" };
        you_are(hofe_titles[u.uevent.uhand_of_elbereth - 1], "");
    }

    Sprintf(buf, "%s", piousness(TRUE, "遵守阵营"));
    if (u.ualign.record >= 0)
        you_are(buf, "");
    else
        you_have(buf, "");

    if (wizard) {
        Sprintf(buf, " %d", u.ualign.record);
        enl_msg("你的阵营记录", "是", "曾是", buf, "");
    }

    /*** Resistances to troubles ***/
    if (Invulnerable)
        you_are("无敌", from_what(INVULNERABLE));
    if (Antimagic)
        you_are("受魔法保护", from_what(ANTIMAGIC));
    if (Fire_resistance)
        you_are("抗火", from_what(FIRE_RES));
    item_resistance_message(AD_FIRE, "受火焰保护", final);
    if (Cold_resistance)
        you_are("抗寒", from_what(COLD_RES));
    item_resistance_message(AD_COLD, "受寒冷保护", final);
    if (Sleep_resistance)
        you_are("抗睡眠", from_what(SLEEP_RES));
    if (Disint_resistance)
        you_are("抗解离", from_what(DISINT_RES));
    item_resistance_message(AD_DISN, "受解离保护", final);
    if (Shock_resistance)
        you_are("抗电击", from_what(SHOCK_RES));
    item_resistance_message(AD_ELEC, "受电击保护",
                            final);
    if (Poison_resistance)
        you_are("抗毒", from_what(POISON_RES));
    if (Acid_resistance) {
        Sprintf(buf, "%.20s%.30s",
                temp_resist(ACID_RES) ? "临时 " : "",
                "抗酸");
        you_are(buf, from_what(ACID_RES));
    }
    item_resistance_message(AD_ACID, "受酸液保护", final);
    if (Drain_resistance)
        you_are("抗等级吸取", from_what(DRAIN_RES));
    if (Sick_resistance)
        you_are("免疫疾病", from_what(SICK_RES));
    if (Stone_resistance) {
        Sprintf(buf, "%.20s%.30s",
                temp_resist(STONE_RES) ? "暂时 " : "",
                "防石化");
        you_are(buf, from_what(STONE_RES));
    }
    if (Halluc_resistance)
        enl_msg(You_, "抵抗", "曾抵抗", "幻觉",
                from_what(HALLUC_RES));
    if (u.uedibility)
        you_can("辨认有害食物", "");

    /*** Vision and senses ***/
    if ((HBlinded || EBlinded) && BBlinded) /* blind w/ blindness blocked */
        you_can("看见", from_what(-BLINDED)); /* Eyes of the Overworld */
    if (Blnd_resist && !Blind) /* skip if no eyes or blindfolded */
        you_are("不受光致失明影响",
                from_what(BLND_RES));
    if (See_invisible) {
        if (!Blind)
            enl_msg(You_, "能看见", "曾能看见", "隐形", from_what(SEE_INVIS));
        else if (!PermaBlind)
            enl_msg(You_, "在不失明时会看见", "在不失明时本会看见",
                    "隐形", "");
        else
            enl_msg(You_, "若未失明会看见", "若未失明本会看见",
                    "隐形", "");
    }
    if (Blind_telepat)
        you_have("心灵感应", from_what(TELEPAT));
    if (Warning)
        you_are("受到警告", from_what(WARNING));
    if (Warn_of_mon && svc.context.warntype.obj) {
        Sprintf(buf, "察觉到 %s 的存在",
                (svc.context.warntype.obj & M2_ORC) ? "兽人"
                : (svc.context.warntype.obj & M2_ELF) ? "精灵"
                  : (svc.context.warntype.obj & M2_DEMON) ? "恶魔"
                    : something);
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Warn_of_mon && svc.context.warntype.polyd) {
        Sprintf(buf, "察觉到%s的存在",
                ((svc.context.warntype.polyd & (M2_HUMAN | M2_ELF))
                 == (M2_HUMAN | M2_ELF)) ? "人类和精灵"
                    : (svc.context.warntype.polyd & M2_HUMAN) ? "人类"
                      : (svc.context.warntype.polyd & M2_ELF) ? "精灵"
                        : (svc.context.warntype.polyd & M2_ORC) ? "兽人"
                          : (svc.context.warntype.polyd & M2_DEMON) ? "恶魔"
                            : "某些怪物");
        you_are(buf, "");
    }
    warnspecies =  svc.context.warntype.speciesidx;
    if (Warn_of_mon && ismnum(warnspecies)) {
        Sprintf(buf, "能感知到 %s 的存在",
                makeplural(mons[warnspecies].pmnames[NEUTRAL]));
        you_are(buf, from_what(WARN_OF_MON));
    }
    if (Undead_warning)
        you_are("能感知亡灵", from_what(WARN_UNDEAD));
    if (Searching)
        you_have("自动搜索", from_what(SEARCHING));
    if (Clairvoyant) {
        you_are("拥有千里眼", from_what(CLAIRVOYANT));
    } else if ((HClairvoyant || EClairvoyant) && BClairvoyant) {
        Strcpy(buf, from_what(-CLAIRVOYANT));
        (void) strsubst(buf, "是因为", "若不是因为");
        enl_msg(You_, "本可以", "本可以", "拥有千里眼", buf);
    }
    if (Infravision)
        you_have("红外视觉", from_what(INFRAVISION));
    if (Detect_monsters) {
        Strcpy(buf, "感知到怪物存在");
        if (wizard) {
            long detectmon_timeout = (HDetect_monsters & TIMEOUT);

            if (detectmon_timeout)
                Sprintf(eos(buf), " (%ld)", detectmon_timeout);
        }
        you_are(buf, "");
    }
    if (u.umconf) { /* 'u.umconf' is a counter rather than a timeout */
        Strcpy(buf, "击中怪物时使怪物混乱");
        if (wizard && !final) {
            if (u.umconf == 1)
                Strcat(buf, " (仅下一次击中)");
            else /* u.umconf > 1 */
                Sprintf(eos(buf), " (接下来%u次击中)", u.umconf);
        }
        enl_msg(You_, "将", "本会", buf, "");
    }

    /*** Appearance and behavior ***/
    if (Adornment) {
        int adorn = 0;

        if (uleft && uleft->otyp == RIN_ADORNMENT)
            adorn += uleft->spe;
        if (uright && uright->otyp == RIN_ADORNMENT)
            adorn += uright->spe;
        /* the sum might be 0 (+0 ring or two which negate each other);
           that yields "you are charismatic" (which isn't pointless
           because it potentially impacts seduction attacks) */
        Sprintf(buf, "具有%s魅力",
                (adorn > 0) ? "更多的" : (adorn < 0) ? "更少的" : "");
        you_are(buf, from_what(ADORNED));
    }
    if (Invisible)
        you_are("隐形", from_what(INVIS));
    else if (Invis)
        you_are("对他者隐形", from_what(INVIS));
    /* ordinarily "visible" is redundant; this is a special case for
       the situation when invisibility would be an expected attribute */
    else if ((HInvis || EInvis) && BInvis)
        you_are("可见", from_what(-INVIS));
    if (Displaced)
        you_are("移位", from_what(DISPLACED));
    if (Stealth) {
        you_are("潜行", from_what(STEALTH));
    } else if (BStealth && (HStealth || EStealth)) {
        Sprintf(buf, "潜行%s",
                (BStealth == FROMOUTSIDE) ? " 若未骑乘" : "");
        enl_msg(You_, "本会", "本会", buf, "");
    }
    if (Aggravate_monster)
        enl_msg(You_, "会惊扰", "曾惊扰", "怪物",
                from_what(AGGRAVATE_MONSTER));
    if (Conflict)
        enl_msg(You_, "会引发", "曾引发", "冲突", from_what(CONFLICT));

    /*** Transportation ***/
    if (Jumping)
        you_can("跳跃", from_what(JUMPING));
    if (Teleportation)
        you_can("传送", from_what(TELEPORT));
    if (Teleport_control)
        you_have("传送控制", from_what(TELEPORT_CONTROL));
    /* actively levitating handled earlier as a status condition */
    if (BLevitation) { /* levitation is blocked */
        long save_BLev = BLevitation;

        BLevitation = 0L;
        if (Levitation) {
            /* either trapped in the floor or inside solid rock
               (or both if chained to buried iron ball and have
               moved one step into solid rock somehow) */
            boolean trapped = (save_BLev & I_SPECIAL) != 0L,
                    terrain = (save_BLev & FROMOUTSIDE) != 0L;

            Sprintf(buf, "%s%s%s",
                    trapped ? "如果未被陷阱困住" : "",
                    (trapped && terrain) ? "并且" : "",
                    terrain ? if_surroundings_permitted : "");
            enl_msg(You_, "本会漂浮", "本会漂浮", buf, "");
        }
        BLevitation = save_BLev;
    }
    /* actively flying handled earlier as a status condition */
    if (BFlying) { /* flight is blocked */
        long save_BFly = BFlying;

        BFlying = 0L;
        if (Flying) {
            enl_msg(You_, "本会飞行", "本会飞行",
                    /* wording quibble: for past tense, "hadn't been"
                       would sound better than "weren't" (and
                       "had permitted" better than "permitted"), but
                       "weren't" and "permitted" are adequate so the
                       extra complexity to handle that isn't worth it */
                    Levitation
                       ? "如果没有漂浮"
                       : (save_BFly == I_SPECIAL)
                          /* this is an oversimplification; being trapped
                             might also be blocking levitation so flight
                             would still be blocked after escaping trap */
                          ? "如果没有被困"
                          : (save_BFly == FROMOUTSIDE)
                             ? if_surroundings_permitted
                             /* two or more of levitation, surroundings,
                                and being trapped in the floor */
                             : "如果情况允许",
                    "");
        }
        BFlying = save_BFly;
    }
    /* including this might bring attention to the fact that ceiling
       clinging has inconsistencies... */
    if (is_clinger(gy.youmonst.data)) {
        boolean has_lid = has_ceiling(&u.uz);

        if (has_lid && !u.uinwater) {
            you_can("攀附天花板", "");
        } else {
            Sprintf(buf, "攀附天花板, 如果%s%s%s",
                    !has_lid ? "有天花板" : "",
                    (!has_lid && u.uinwater) ? "并且" : "",
                    u.uinwater ? (Underwater ? "你没有在水下"
                                  : "你没有在水中") : "");
            /* past tense is applicable for death while Unchanging */
            enl_msg(You_, "本可以", "本可以", buf, "");
        }
    }
    /* actively walking on water handled earlier as a status condition */
    if (Wwalking && !walking_on_water())
        you_can("水上行走", from_what(WWALKING));
    /* actively swimming (in water but not under it) handled earlier */
    if (Swimming && (Underwater || !u.uinwater))
        you_can("游泳", from_what(SWIMMING));
    if (Breathless)
        you_can("不呼吸也能生存", from_what(MAGICAL_BREATHING));
    else if (Amphibious)
        you_can("在水中呼吸", from_what(MAGICAL_BREATHING));
    if (Passes_walls)
        you_can("穿墙而行", from_what(PASSES_WALLS));

    /*** Physical attributes ***/
    if (Regeneration)
        enl_msg(You_, "会", "曾会", "再生", from_what(REGENERATION));
    if (Slow_digestion)
        you_have("较慢消化", from_what(SLOW_DIGESTION));
    if (u.uhitinc) {
        (void) enlght_combatinc("to hit", u.uhitinc, final, buf);
        if (iflags.tux_penalty && !Upolyd)
            Sprintf(eos(buf), " %s 你的套装惩罚",
                    (u.uhitinc < 0) ? "增加"
                    : (u.uhitinc < 4 * gu.urole.spelarmr / 5)
                      ? "部分抵消"
                      : (u.uhitinc < gu.urole.spelarmr) ? "几乎抵消"
                        : "克服");
        you_have(buf, "");
    }
    if (u.udaminc)
        you_have(enlght_combatinc("damage", u.udaminc, final, buf), "");
    if (u.uspellprot || Protection) {
        int prot = 0;

        if (uleft && uleft->otyp == RIN_PROTECTION)
            prot += uleft->spe;
        if (uright && uright->otyp == RIN_PROTECTION)
            prot += uright->spe;
        if (uamul && uamul->otyp == AMULET_OF_GUARDING)
            prot += 2;
        if (HProtection & INTRINSIC)
            prot += u.ublessed;
        prot += u.uspellprot;
        if (prot)
            you_have(enlght_combatinc("defense", prot, final, buf), "");
    }
    if ((armpro = magic_negation(&gy.youmonst)) > 0) {
        /* magic cancellation factor, conferred by worn armor */
        static const char *const mc_types[] = {
            "" /*ordinary*/, "受护佑", "受守护", "受保护",
        };
        /* sanity check */
        if (armpro >= SIZE(mc_types))
            armpro = SIZE(mc_types) - 1;
        you_are(mc_types[armpro], "");
    }
    if (Half_physical_damage)
        enlght_halfdmg(HALF_PHDAM, final);
    if (Half_spell_damage)
        enlght_halfdmg(HALF_SPDAM, final);
    if (Half_gas_damage)
        enl_msg(You_, "", "", "承受减少的毒气伤害", "");
    if (spellid(0) > NO_SPELL) { /* skip if no spells are known yet */
        /* greatly simplified edition of percent_success(spell.c)--may need
           to be suppressed if oversimplification leads to player confusion */
        char cast_adj[QBUFSZ];
        boolean suit = uarm && is_metallic(uarm),
                robe = uarmc && uarmc->otyp == ROBE;

        *cast_adj = '\0';
        if (suit) /* omit "wearing" to shorten the text */
            Sprintf(cast_adj, "受金属盔甲妨碍%s",
                    robe ? ", 被你的长袍缓解" : "");
        else if (robe)
            Strcpy(cast_adj, "因穿着长袍而增强");

        if (*cast_adj)
            enl_msg("你的施法", "是", "曾是", cast_adj, "");
    }
    /* polymorph and other shape change */
    if (Protection_from_shape_changers)
        you_are("受变形怪保护",
                from_what(PROT_FROM_SHAPE_CHANGERS));
    if (Unchanging) {
        const char *what = 0;

        if (!Upolyd) /* Upolyd handled below after current form */
            enl_msg(You_, "", "", "不能改变当前形态",
                    from_what(UNCHANGING));
        /* blocked shape changes */
        if (Polymorph)
            what = !final ? "变形" : "变形";
        else if (ismnum(u.ulycn))
            what = !final ? "改变形态" : "改变形态";
        if (what) {
            Sprintf(buf, "将周期性地%s", what);
            /* omit from_what(UNCHANGING); too verbose */
            enl_msg(You_, "", "", buf,
                    "");
        }
    } else if (Polymorph) {
        you_are("周期性变形", from_what(POLYMORPH));
    }
    if (Polymorph_control)
        you_have("变形控制", from_what(POLYMORPH_CONTROL));
    if (Upolyd && u.umonnum != u.ulycn
        /* if we've died from turning into slime, we're polymorphed
           right now but don't want to list it as a temporary attribute
           [we need a more reliable way to detect this situation] */
        && !(final == ENL_GAMEOVERDEAD
             && u.umonnum == PM_GREEN_SLIME && !Unchanging)) {
        /* foreign shape (except were-form which is handled below) */
        if (!vampshifted(&gy.youmonst))
            Sprintf(buf, "已变形为%s",
                    an(pmname(gy.youmonst.data,
                              flags.female ? FEMALE : MALE)));
        else
            Sprintf(buf, "变形为%s的%s形态",
                    an(pmname(&mons[gy.youmonst.cham],
                              flags.female ? FEMALE : MALE)),
                    pmname(gy.youmonst.data, flags.female ? FEMALE : MALE));
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.mtimedone);
        you_are(buf, "");
    }
    if (lays_eggs(gy.youmonst.data) && flags.female) /* Upolyd */
        you_can("产卵", "");
    if (ismnum(u.ulycn)) {
        /* "you are a werecreature [in beast form]" */
        Strcpy(buf, an(pmname(&mons[u.ulycn],
               flags.female ? FEMALE : MALE)));
        if (u.umonnum == u.ulycn) {
            Strcat(buf, "是野兽形态");
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.mtimedone);
        }
        you_are(buf, "");
    }
    if (Unchanging && Upolyd) /* !Upolyd handled above */
        enl_msg(You_, "", "", "不能改变当前形态", from_what(UNCHANGING));
    if (Hate_silver)
        you_are("受银伤害", "");
    /* movement and non-armor-based protection */
    if (Fast)
        you_are(Very_fast ? "非常快" : "快", from_what(FAST));
    if (Reflecting)
        you_have("反射", from_what(REFLECTING));
    if (Free_action)
        you_have("行动自如", from_what(FREE_ACTION));
    if (Fixed_abil)
        you_have("固定能力", from_what(FIXED_ABIL));
    if (Lifesaved)
        enl_msg("你的生命", "将被", "本会被", "拯救", "");

    /*** Miscellany ***/
    if (Luck) {
        ltmp = abs((int) Luck);
        Sprintf(buf, "%s%s幸运",
                ltmp >= 10 ? "极其" : ltmp >= 5 ? "非常" : "",
                Luck < 0 ? "不" : "");
        if (wizard)
            Sprintf(eos(buf), " (%d)", Luck);
        you_are(buf, "");
    } else if (wizard)
        enl_msg("你的幸运值", "是", "曾是", "零", "");
    if (u.moreluck > 0)
        you_have("额外幸运", "");
    else if (u.moreluck < 0)
        you_have("降低的幸运", "");
    if (carrying(LUCKSTONE) || stone_luck(TRUE)) {
        ltmp = stone_luck(FALSE);
        if (ltmp <= 0)
            enl_msg("你的坏运", "不会", "未曾", "随时间消退", "");
        if (ltmp >= 0)
            enl_msg("你的好运", "不会", "未曾", "随时间消退", "");
    }

    if (u.ugangr) {
        Sprintf(buf, "%s生气",
                u.ugangr > 6 ? "极其" : u.ugangr > 3 ? "非常" : "");
        if (wizard)
            Sprintf(eos(buf), " (%d)", u.ugangr);
        enl_msg(u_gname(), "对你", "曾对你", buf, "");
    } else {
        /*
         * We need to suppress this when the game is over, because death
         * can change the value calculated by can_pray(), potentially
         * resulting in a false claim that you could have prayed safely.
         */
        if (!final) {
#if 0
            /* "can [not] safely pray" vs "could [not] have safely prayed" */
            Sprintf(buf, "%s平安地祈祷", can_pray(FALSE) ? "能" : "不能");
#else
            Sprintf(buf, "%s平安地祈祷", can_pray(FALSE) ? "能" : "不能");
#endif
            if (wizard)
                Sprintf(eos(buf), " (%d)", u.ublesscnt);
            enl_msg(You_, "", "", buf, "");
        }
    }

#ifdef DEBUG
    /* named fruit debugging (doesn't really belong here...); to enable,
       include 'fruit' in DEBUGFILES list (even though it isn't a file...) */
    if (wizard && explicitdebug("fruit")) {
        struct fruit *f;

        reorder_fruit(TRUE); /* sort by fruit index, from low to high;
                              * this modifies the gf.ffruit chain, so could
                              * possibly mask or even introduce a problem,
                              * but it does useful sanity checking */
        for (f = gf.ffruit; f; f = f->nextf) {
            Sprintf(buf, "水果 #%d ", f->fid);
            enl_msg(buf, "是", "曾是", f->fname, "");
        }
        enl_msg("当前水果", "是", "曾是", svp.pl_fruit, "");
        Sprintf(buf, "%d", flags.made_fruit);
        enl_msg("造出水果标志", "是", "曾是", buf, "");
    }
#endif

    {
        const char *p;

        buf[0] = '\0';
        if (final < 2) { /* still in progress, or quit/escaped/ascended */
            p = "被杀后幸存";
            if (!u.umortality)
                p = !final ? (char *) 0 : "幸存";
            else
                (void) N_times((long) u.umortality, buf);
        } else { /* game ended in character's death */
            p = "已死亡";
            switch (u.umortality) {
            case 0:
                impossible("dead without dying?");
                FALLTHROUGH;
                /* FALLTHRU */
            case 1:
                break; /* just "are dead" */
            default:
                Sprintf(buf, " (%d%s次!)", u.umortality,
                        ordin(u.umortality));
                break;
            }
        }
        if (p)
            enl_msg(You_, "", "", p, buf, "");
    }
}

/* ^X command */
int
doattributes(void)
{
    int mode = BASICENLIGHTENMENT;

    /* show more--as if final disclosure--for wizard and explore modes */
    if (wizard || discover)
        mode |= MAGICENLIGHTENMENT;

    enlightenment(mode, ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

void
youhiding(boolean via_enlghtmt, /* enlightenment line vs topl message */
          int msgflag)          /* for variant message phrasing */
{
    char *bp, buf[BUFSZ];

    Strcpy(buf, "隐藏");
    if (U_AP_TYPE != M_AP_NOTHING) {
        /* mimic; hero is only able to mimic a strange object or gold
           or hallucinatory alternative to gold, so we skip the details
           for the hypothetical furniture and monster cases */
        bp = eos(strcpy(buf, "在模拟"));
        if (U_AP_TYPE == M_AP_OBJECT) {
            Sprintf(bp, " %s", an(simple_typename(gy.youmonst.mappearance)));
        } else if (U_AP_TYPE == M_AP_FURNITURE) {
            Strcpy(bp, "什么东西");
        } else if (U_AP_TYPE == M_AP_MONSTER) {
            Strcpy(bp, "什么怪");
        } else {
            ; /* something unexpected; leave 'buf' as-is */
        }
    } else if (u.uundetected) {
        bp = eos(buf); /* points past "hiding" */
        if (gy.youmonst.data->mlet == S_EEL) {
            if (is_pool(u.ux, u.uy))
                Sprintf(bp, "在%s里", waterbody_name(u.ux, u.uy));
        } else if (hides_under(gy.youmonst.data)) {
            struct obj *o = svl.level.objects[u.ux][u.uy];

            if (o)
                Sprintf(bp, "在%s下面", ansimpleoname(o));
        } else if (is_clinger(gy.youmonst.data) || Flying) {
            /* Flying: 'lurker above' hides on ceiling but doesn't cling */
            Sprintf(bp, "在%s上", ceiling(u.ux, u.uy));
        } else {
            /* on floor; is_hider() but otherwise not special: 'trapper' */
            if (u.utrap && u.utraptype == TT_PIT) {
                struct trap *t = t_at(u.ux, u.uy);

                Sprintf(bp, "在一个%s坑里",
                        (t && t->ttyp == SPIKED_PIT) ? "有钉子的" : "");
            } else
                Sprintf(bp, " 在%s上", surface(u.ux, u.uy));
        }
    } else {
        ; /* shouldn't happen; will result in generic "you are hiding" */
    }

    if (via_enlghtmt) {
        int final = msgflag; /* 'final' is used by you_are() macro */

        you_are(buf, "");
    } else {
        /* for dohide(), when player uses '#monster' command */
        You("%s%s.", msgflag ? "已经" : "现在", buf);
    }
}

/* #conduct command [KMH]; shares enlightenment's tense handling */
int
doconduct(void)
{
    show_conduct(ENL_GAMEINPROGRESS);
    return ECMD_OK;
}

/* display conducts; for doconduct(), also disclose() and dump_everything() */
void
show_conduct(int final)
{
    char buf[BUFSZ], bufN[40];
    int ngenocided;

    /* Create the conduct window */
    ge.en_win = create_nhwindow(NHW_MENU);
    putstr(ge.en_win, 0, "自愿挑战:");

    /* rerolling; "You <this or that>" is about the character, rerolling
       is about the player so phrase it differently;
       also, always use past tense since the chance to do something with it
       is gone by time player can issue #conduct command or see disclosure */
    if (!u.uroleplay.reroll)
        Strcpy(buf, " 角色重掷未启用.");
    else if (!u.uroleplay.numrerolls)
        Strcpy(buf, " 你的角色没有被重掷.");
    else
        Sprintf(buf, " 你的角色被重掷了%s.",
                N_times(u.uroleplay.numrerolls, bufN));
    enlght_out(buf);

    if (u.uroleplay.blind)
        you_have_been("先天失明");
    if (u.uroleplay.deaf)
        you_have_been("先天失聪");
    /* note: we don't report "you are without possessions" unless the
       game started with the pauper option set */
    if (u.uroleplay.pauper)
        enl_msg(You_, gi.invent ? "起始时" : "现在", "起始时",
                "没有财产", "");
    /* nudist is far more than a subset of possessionless, and a much
       more impressive accomplishment, but showing "started out without
       possessions" before "faithfully nudist" looks more logical */
    if (u.uroleplay.nudist)
        you_have_been("一直坚持裸体主义");

    if (!u.uconduct.food)
        enl_msg(You_, "一直", "曾经", "不吃食物", "");
        /* but beverages are okay */
    else if (!u.uconduct.unvegan)
        you_have_X("遵循严格纯素饮食");
    else if (!u.uconduct.unvegetarian)
        you_have_been("保持素食");

    if (!u.uconduct.gnostic)
        you_have_been("保持无神论");

    if (!u.uconduct.weaphit) {
        you_have_never("用持握武器命中过");
    } else if (wizard) {
        Sprintf(buf, "用持握武器命中过 %ld 次", u.uconduct.weaphit);
        you_have_X(buf);
    }
    if (!u.uconduct.killer)
        you_have_been("保持和平主义");

    if (!u.uconduct.literate) {
        you_have_been("保持文盲");
    } else if (wizard) {
        Sprintf(buf, "阅读物品或铭刻过 %ld 次", u.uconduct.literate);
        you_have_X(buf);
    }

    if (!u.uconduct.pets)
        you_have_never("拥有过宠物");

    ngenocided = num_genocides();
    if (ngenocided == 0) {
        you_have_never("灭绝过任何怪物");
    } else {
        Sprintf(buf, "灭绝了 %d 类怪物", ngenocided);
        you_have_X(buf);
    }

    if (!u.uconduct.polypiles) {
        you_have_never("变形过物品");
    } else if (wizard) {
        Sprintf(buf, "变形了 %ld 件物品", u.uconduct.polypiles);
        you_have_X(buf);
    }

    if (!u.uconduct.polyselfs) {
        you_have_never("改变过形态");
    } else if (wizard) {
        Sprintf(buf, "改变形态 %ld 次", u.uconduct.polyselfs);
        you_have_X(buf);
    }

    if (!u.uconduct.wishes) {
        you_have_X("没有使用愿望");
    } else {
        Sprintf(buf, "使用了 %ld 个愿望", u.uconduct.wishes);
        if (u.uconduct.wisharti) {
            /* if wisharti == wishes
             *  1 wish (for an artifact)
             *  2 wishes (both for artifacts)
             *  N wishes (all for artifacts)
             * else (N is at least 2 in order to get here; M < N)
             *  N wishes (1 for an artifact)
             *  N wishes (M for artifacts)
             */
            if (u.uconduct.wisharti == u.uconduct.wishes)
                Sprintf(eos(buf), " (%s",
                        (u.uconduct.wisharti > 2L) ? "全是"
                          : (u.uconduct.wisharti == 2L) ? "都是" : "");
            else
                Sprintf(eos(buf), " (%ld ", u.uconduct.wisharti);

            Sprintf(eos(buf), "%s)",
                    (u.uconduct.wisharti == 1L) ? "1 神器"
                                                : "神器");
        }
        you_have_X(buf);

        if (!u.uconduct.wisharti)
            enl_msg(You_, "没有", "没有", "许愿获取任何神器", "");
    }

    /* only report Sokoban conduct if the Sokoban branch has been entered */
    if (sokoban_in_play()) {
        const char *presentverb = "违反了", *pastverb = "违反了";

        if (!u.uconduct.sokocheat) {
            presentverb = "没有违反";
            pastverb = "没有违反";
            Strcpy(buf, "任何特殊的推箱子规则");
        } else {
            Strcpy(buf, "特殊的推箱子规则 ");
            Strcat(buf, N_times(u.uconduct.sokocheat, bufN));
        }
        enl_msg(You_, presentverb, pastverb, buf, "");
    }

    show_achievements(final);

    /* Pop up the window and wait for a key */
    display_nhwindow(ge.en_win, TRUE);
    destroy_nhwindow(ge.en_win);
    ge.en_win = WIN_ERR;
}

/*
 *      Achievements (see 'enum achievements' in you.h).
 */

staticfn void
show_achievements(
    int final) /* 'final' is used "behind the curtain" by enl_foo() macros */
{
    int i, achidx, absidx, acnt;
    char title[QBUFSZ], buf[QBUFSZ];
    winid awin = WIN_ERR;

    /* unfortunately we can't show the achievements (at least not all of
       them) while the game is in progress because it would give away the
       ID of luckstone (at Mine's End) and of real Amulet of Yendor */
    if (!final && !wizard)
        return;

    /* first, figure whether any achievements have been accomplished
       so that we don't show the header for them if the resulting list
       below it would be empty */
    if ((acnt = count_achievements()) == 0)
        return;

    if (ge.en_win != WIN_ERR) {
        awin = ge.en_win; /* end of game disclosure window */
        putstr(awin, 0, "");
    } else {
        awin = create_nhwindow(NHW_MENU);
    }
    Sprintf(title, "成就:");
    putstr(awin, 0, title);

    /* display achievements in the order in which they were recorded;
       lone exception is to defer the Amulet if we just ascended;
       it warrants alternate wording when given away during ascension,
       but the Amulet achievement is always attained before entering
       endgame and the alternate wording looks strange if shown before
       "reached endgame" and "reached Astral" */
    if (remove_achievement(ACH_UWIN)) { /* UWIN == Ascended! */
        /* for ascension, force it to be last and Amulet next to last
           by taking them out and then adding them back */
        if (remove_achievement(ACH_AMUL)) /* should always be True here */
            record_achievement(ACH_AMUL);
        record_achievement(ACH_UWIN);
    }
    for (i = 0; i < acnt; ++i) {
        achidx = u.uachieved[i];
        absidx = abs(achidx);

        switch (absidx) {
        case ACH_BLND:
            enl_msg(You_, "在", "曾在", "无法看见的情况下探索", "");
            break;
        case ACH_NUDE:
            enl_msg(You_, "一直", "曾经", "不穿任何盔甲", "");
            break;
        case ACH_MINE:
            you_have_X("进入过侏儒矿坑");
            break;
        case ACH_TOWN:
            you_have_X("到达过矿镇");
            break;
        case ACH_SHOP:
            you_have_X("进入过商店");
            break;
        case ACH_TMPL:
            you_have_X("进入过神殿");
            break;
        case ACH_ORCL:
            you_have_X("咨询过德尔菲先知");
            break;
        case ACH_NOVL:
            you_have_X("读过 Discworld 小说");
            break;
        case ACH_SOKO:
            you_have_X("进入过推箱子关");
            break;
        case ACH_SOKO_PRIZE: /* hard to reach guaranteed bag or amulet */
            you_have_X("完成了推箱子关");
            break;
        case ACH_MINE_PRIZE: /* hidden guaranteed luckstone */
            you_have_X("完成了侏儒矿坑");
            break;
        case ACH_BGRM:
            you_have_X("进入过大房间");
            break;
        case ACH_MEDU:
            you_have_X("击败了美杜莎");
            break;
        case ACH_TUNE:
            you_have_X(
                "学会了开闭城堡吊桥的曲调");
            break;
        case ACH_BELL:
            /* alternate phrasing for present vs past and also for
               possessing the item vs once held it */
            enl_msg(You_,
                    u.uhave.bell ? "拥有" : "曾经持有",
                    u.uhave.bell ? "曾拥有" : "曾经持有",
                    "开门之铃", "");
            break;
        case ACH_HELL:
            enl_msg(You_, "", "", "进入过 Gehennom", "");
            break;
        case ACH_CNDL:
            enl_msg(You_,
                    u.uhave.menorah ? "拥有" : "曾经持有",
                    u.uhave.menorah ? "曾拥有" : "曾经持有",
                    "祈祷烛台", "");
            break;
        case ACH_BOOK:
            enl_msg(You_,
                    u.uhave.book ? "拥有" : "曾经持有",
                    u.uhave.book ? "曾拥有" : "曾经持有",
                    "亡者之书", "");
            break;
        case ACH_INVK:
            you_have_X("进入过 Moloch 的至圣所");
            break;
        case ACH_AMUL:
            /* alternate wording for ascended (always past tense) since
               hero had it until #offer forced it to be relinquished */
            enl_msg(You_,
                    u.uhave.amulet ? "拥有" : "取得了",
                    u.uevent.ascended ? "献上了"
                     : u.uhave.amulet ? "曾拥有" : "曾取得",
                    "Yendor 护符", "");
            break;

        /* reaching Astral makes feedback about reaching the Planes
           be redundant and ascending makes both be redundant, but
           we display all that apply */
        case ACH_ENDG:
            you_have_X("到达过元素位面");
            break;
        case ACH_ASTR:
            you_have_X("到达过星界位面");
            break;
        case ACH_UWIN:
            /* the ultimate achievement... */
            enlght_out(" 你飞升了!");
            break;

        /* rank 0 is the starting condition, not an achievement; 8 is Xp 30 */
        case ACH_RNK1: case ACH_RNK2: case ACH_RNK3: case ACH_RNK4:
        case ACH_RNK5: case ACH_RNK6: case ACH_RNK7: case ACH_RNK8:
            Sprintf(buf, "达到了%s的称号",
                    rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                            Role_switch, (achidx < 0) ? TRUE : FALSE));
            you_have_X(buf);
            break;

        default:
            Sprintf(buf, " [意外的成就 #%d.]", achidx);
            enlght_out(buf);
            break;
        } /* switch */
    } /* for */

    if (awin != ge.en_win) {
        display_nhwindow(awin, TRUE);
        destroy_nhwindow(awin);
    }
}

/* record an achievement (add at end of list unless already present) */
void
record_achievement(schar achidx)
{
    int i, absidx;
    int repeat_achievement = 0;

    absidx = abs(achidx);
    /* valid achievements range from 1 to N_ACH-1; however, ranks can be
       stored as the complement (ie, negative) to track gender */
    if ((achidx < 1 && (absidx < ACH_RNK1 || absidx > ACH_RNK8))
        || achidx >= N_ACH) {
        impossible("Achievement #%d is out of range.", achidx);
        return;
    }

    /* the list has an extra slot so there is always at least one 0 at
       its end (more than one unless all N_ACH-1 possible achievements
       have been recorded); find first empty slot or achievement #achidx;
       an attempt to duplicate an achievement can happen if any of Bell,
       Candelabrum, Book, or Amulet is dropped then picked up again */
    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == absidx) {
            repeat_achievement = 1;
            break;
        }

    /*
     * We do the sound for an achievement, even if it has already been
     * achieved before. Some players might have set up level-based
     * theme music or something. We do let the sound interface know
     * that it's not the original achievement though.
     */
    SoundAchievement(achidx, 0, repeat_achievement);

    if (repeat_achievement)
        return; /* already recorded, don't duplicate it */
    u.uachieved[i] = achidx;

    /* avoid livelog for achievements recorded during final disclosure:
       nudist and blind-from-birth; also ascension which is suppressed
       by this gets logged separately in really_done() */
    if (program_state.gameover)
        return;

    if (absidx >= ACH_RNK1 && absidx <= ACH_RNK8) {
        livelog_printf(achieve_msg[absidx].llflag,
                       "达到了%s称号(等级 %d)",
                       rank_of(rank_to_xlev(absidx - (ACH_RNK1 - 1)),
                               Role_switch, (achidx < 0) ? TRUE : FALSE),
                       u.ulevel);
    } else if (achidx == ACH_SOKO_PRIZE
               || achidx == ACH_MINE_PRIZE) {
        /* need to supply extra information for these two */
        short otyp = ((achidx == ACH_SOKO_PRIZE)
                      ? svc.context.achieveo.soko_prize_otyp
                      : svc.context.achieveo.mines_prize_otyp);

        /* note: OBJ_NAME() works here because both "bag of holding" and
           "amulet of reflection" are fully named in their objects[] entry
           but that's not true in the general case */
        livelog_printf(achieve_msg[achidx].llflag, "%s %s",
                       achieve_msg[achidx].msg, OBJ_NAME(objects[otyp]));
    } else {
        livelog_printf(achieve_msg[absidx].llflag, "%s",
                       achieve_msg[absidx].msg);
    }
}

/* discard a recorded achievement; return True if removed, False otherwise */
boolean
remove_achievement(schar achidx)
{
    int i;

    for (i = 0; u.uachieved[i]; ++i)
        if (abs(u.uachieved[i]) == abs(achidx))
            break; /* stop when found */
    if (!u.uachieved[i]) /* not found */
        return FALSE;
    /* list is 0 terminated so any beyond the removed one move up a slot */
    do {
        u.uachieved[i] = u.uachieved[i + 1];
    } while (u.uachieved[++i]);
    return TRUE;
}

/* used to decide whether there are any achievements to display */
int
count_achievements(void)
{
    int i, acnt = 0;

    for (i = 0; u.uachieved[i]; ++i)
        ++acnt;
    return acnt;
}

/* convert a rank index to an achievement number; encode it when female
   in order to subsequently report gender-specific ranks accurately */
schar
achieve_rank(int rank) /* 1..8 */
{
    schar achidx = (schar) ((rank - 1) + ACH_RNK1);

    if (flags.female)
        achidx = -achidx;
    return achidx;
}

/* return True if sokoban branch has been entered, False otherwise */
boolean
sokoban_in_play(void)
{
    int achidx;

    /* TODO? move this to dungeon.c and test furthest level reached of the
       sokoban branch instead of relying on the entered-sokoban achievement */

    for (achidx = 0; u.uachieved[achidx]; ++achidx)
        if (u.uachieved[achidx] == ACH_SOKO)
            return TRUE;
    return FALSE;
}

/* #chronicle command */
int
do_gamelog(void)
{
#ifdef CHRONICLE
    if (gg.gamelog) {
        show_gamelog(ENL_GAMEINPROGRESS);
    } else {
        pline("没有记录的事件.");
    }
#else
    pline("编译时已关闭事件记录.");
#endif /* !CHRONICLE */
    return ECMD_OK;
}

/* 'major' events for dumplog; inclusion or exclusion here may need tuning */
#define LL_majors (0L \
                   | LL_WISH            \
                   | LL_ACHIEVE         \
                   | LL_UMONST          \
                   | LL_DIVINEGIFT      \
                   | LL_LIFESAVE        \
                   | LL_ARTIFACT        \
                   | LL_GENOCIDE        \
                   | LL_DUMP) /* explicitly for dumplog */
#define majorevent(llmsg) (((llmsg)->flags & LL_majors) != 0)
#define spoilerevent(llmsg) (((llmsg)->flags & LL_SPOILER) != 0)

/* #chronicle details */
void
show_gamelog(int final)
{
#ifdef CHRONICLE
    struct gamelog_line *llmsg;
    winid win;
    char buf[BUFSZ];
    int eventcnt = 0;

    win = create_nhwindow(NHW_TEXT);
    Sprintf(buf, "%s事件:", final ? "重大" : "已记录");
    putstr(win, 0, buf);
    for (llmsg = gg.gamelog; llmsg; llmsg = llmsg->next) {
        if (final && !majorevent(llmsg))
            continue;
        if (!final && !wizard && spoilerevent(llmsg))
            continue;
        if (!eventcnt++)
            putstr(win, 0, " 回合");
        Snprintf(buf, sizeof buf, "%5ld: %s", llmsg->turn, llmsg->text);
        putstr(win, 0, buf);
    }
    /* since start of game is logged as a major event, 'eventcnt' should
       never end up as 0; for 'final', end of game is a major event too */
    if (!eventcnt)
        putstr(win, 0, " 无");

    display_nhwindow(win, TRUE);
    destroy_nhwindow(win);
#else
    nhUse(final);
#endif /* !CHRONICLE */
    return;
}

/*
 *      Vanquished monsters.
 */

/* the two uppercase choices are implemented but suppressed from menu.
   also used in options.c */
const char *const vanqorders[NUM_VANQ_ORDER_MODES][3] = {
    { "t", "传统: 按怪物等级",
           "传统: 按怪物等级, 同等级按内部怪物索引" },
    { "d", "按怪物难度",
           "按怪物难度, 同难度按内部怪物索引" },
    { "a", "按字母序, 唯一怪物分开",
           "按字母序, 先列唯一怪物, 再列其他怪物" },
    { "A", "按字母序, 唯一怪物混排",
           "按字母序, 唯一怪物与其他怪物混排" },
    { "C", "按怪物类别, 类内等级从高到低",
           "按怪物类别, 类内等级从高到低" },
    { "c", "按怪物类别, 类内等级从低到高",
           "按怪物类别, 类内等级从低到高" },
    { "n", "按数量从高到低",
           "按数量从高到低, 数量相同时按内部索引" },
    { "z", "按数量从低到高",
           "按数量从低到高, 数量相同时按内部索引" },
};

staticfn int QSORTCALLBACK
vanqsort_cmp(
    const genericptr vptr1,
    const genericptr vptr2)
{
    int indx1 = *(short *) vptr1, indx2 = *(short *) vptr2,
        mlev1, mlev2, mstr1, mstr2, uniq1, uniq2, died1, died2, res;
    const char *name1, *name2, *punct;
    schar mcls1, mcls2;

    switch (flags.vanq_sortmode) {
    default:
    case VANQ_MLVL_MNDX:
        /* sort by monster level */
        mlev1 = mons[indx1].mlevel;
        mlev2 = mons[indx2].mlevel;
        res = mlev2 - mlev1; /* mlevel high to low */
        break;
    case VANQ_MSTR_MNDX:
        /* sort by monster toughness */
        mstr1 = mons[indx1].difficulty;
        mstr2 = mons[indx2].difficulty;
        res = mstr2 - mstr1; /* monstr high to low */
        break;
    case VANQ_ALPHA_SEP:
        uniq1 = ((mons[indx1].geno & G_UNIQ) && indx1 != PM_HIGH_CLERIC);
        uniq2 = ((mons[indx2].geno & G_UNIQ) && indx2 != PM_HIGH_CLERIC);
        if (uniq1 ^ uniq2) { /* one or other uniq, but not both */
            res = uniq2 - uniq1;
            break;
        } /* else both unique or neither unique */
        FALLTHROUGH;
        /*FALLTHRU*/
    case VANQ_ALPHA_MIX:
        name1 = mons[indx1].pmnames[NEUTRAL];
        name2 = mons[indx2].pmnames[NEUTRAL];
        res = strcmpi(name1, name2); /* caseblind alpha, low to high */
        break;
    case VANQ_MCLS_HTOL:
    case VANQ_MCLS_LTOH:
        /* mons[].mlet is a small integer, 1..N, of type plain char;
           if 'char' happens to be unsigned, (mlet1 - mlet2) would yield
           an inappropriate result when mlet2 is greater than mlet1,
           so force our copies (mcls1, mcls2) to be signed */
        mcls1 = (schar) mons[indx1].mlet;
        mcls2 = (schar) mons[indx2].mlet;
        /* S_ANT through S_ZRUTY correspond to lowercase monster classes,
           S_ANGEL through S_ZOMBIE correspond to uppercase, and various
           punctuation characters are used for classes beyond those */
        if (mcls1 > S_ZOMBIE && mcls2 > S_ZOMBIE) {
            /* force a specific order to the punctuation classes that's
               different from the internal order;
               internal order is ok if neither or just one is punctuation
               since letters have lower values so come out before punct */
            static const char punctclasses[] = {
                S_LIZARD, S_EEL, S_GOLEM, S_GHOST, S_DEMON, S_HUMAN, '\0'
            };

            if ((punct = strchr(punctclasses, mcls1)) != 0)
                mcls1 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
            if ((punct = strchr(punctclasses, mcls2)) != 0)
                mcls2 = (schar) (S_ZOMBIE + 1 + (int) (punct - punctclasses));
        }
        res = mcls1 - mcls2; /* class */
        if (res == 0) {
            /* Riders are in the same class as major demons, yielding res==0
               above when both mcls1 and mcls2 are either Riders or demons or
               one of each; force Riders to be sorted before demons */
            res = is_rider(&mons[indx2]) - is_rider(&mons[indx1]);
            /* res -1 => #1 is a Rider, #2 isn't;
                    0 => both Riders or neither;
                   +1 => #2 is a Rider, #1 isn't */
            if (res)
                break;
            mlev1 = mons[indx1].mlevel;
            mlev2 = mons[indx2].mlevel;
            res = mlev1 - mlev2; /* mlevel low to high */
            if (flags.vanq_sortmode == VANQ_MCLS_HTOL)
                res = -res; /* mlevel high to low */
        }
        break;
    case VANQ_COUNT_H_L:
    case VANQ_COUNT_L_H:
        died1 = svm.mvitals[indx1].died;
        died2 = svm.mvitals[indx2].died;
        res = died2 - died1; /* dead count high to low */
        if (flags.vanq_sortmode == VANQ_COUNT_L_H)
            res = -res; /* dead count low to high */
        break;
    }
    /* tiebreaker: internal mons[] index */
    if (res == 0)
        res = indx1 - indx2; /* mndx low to high */
    return res;
}

/* returns -1 if cancelled via ESC */
int
set_vanq_order(boolean for_vanq)
{
    winid tmpwin;
    menu_item *selected;
    anything any;
    char buf[BUFSZ];
    const char *desc;
    int i, n, choice,
        clr = NO_COLOR;

    tmpwin = create_nhwindow(NHW_MENU);
    start_menu(tmpwin, MENU_BEHAVE_STANDARD);
    any = cg.zeroany; /* zero out all bits */
    for (i = 0; i < SIZE(vanqorders); i++) {
        if (i == VANQ_ALPHA_MIX || i == VANQ_MCLS_HTOL) /* skip these */
            continue;
        /* suppress some orderings if this menu if for 'm #genocided' */
        if (!for_vanq && (i == VANQ_COUNT_H_L || i == VANQ_COUNT_L_H))
            continue;
        desc = vanqorders[i][2];
        /* unique monsters can't be genocided so "alpha, unique separate"
           and "alpha, unique intermixed" are confusing descriptions when
           this menu is for #genocided rather than for #vanquished */
        if (!for_vanq && i == VANQ_ALPHA_SEP)
            desc = "按字母序";
        any.a_int = i + 1;
        add_menu(tmpwin, &nul_glyphinfo, &any, *vanqorders[i][0], 0,
                 ATR_NONE, clr, desc,
                 (i == flags.vanq_sortmode) ? MENU_ITEMFLAGS_SELECTED
                                            : MENU_ITEMFLAGS_NONE);
    }
    Sprintf(buf, "%s的排序顺序",
            for_vanq ? "已击杀怪物计数（含灭绝类型）"
                     : "已灭绝怪物类型（含击杀计数）");
    end_menu(tmpwin, buf);

    n = select_menu(tmpwin, PICK_ONE, &selected);
    destroy_nhwindow(tmpwin);
    if (n > 0) {
        choice = selected[0].item.a_int - 1;
        /* skip preselected entry if we have more than one item chosen */
        if (n > 1 && choice == flags.vanq_sortmode)
            choice = selected[1].item.a_int - 1;
        free((genericptr_t) selected);
        flags.vanq_sortmode = choice;
    }
    return (n < 0) ? -1 : flags.vanq_sortmode;
}

/* #vanquished command */
int
dovanquished(void)
{
    list_vanquished(iflags.menu_requested ? 'A' : 'y', FALSE);
    iflags.menu_requested = FALSE;
    return ECMD_OK;
}

/* high priests aren't unique but are flagged as such to simplify something */
#define UniqCritterIndx(mndx) \
    ((mons[mndx].geno & G_UNIQ) != 0 && mndx != PM_HIGH_CLERIC)

#define done_stopprint program_state.stopprint

/* used for #vanquished and end of game disclosure and end of game dumplog */
void
list_vanquished(char defquery, boolean ask)
{
    int i;
    int pfx, nkilled;
    unsigned ntypes, ni;
    long total_killed = 0L;
    winid klwin;
    short mindx[NUMMONS];
    char c, buf[BUFSZ], buftoo[BUFSZ];
    /* 'A' is only supplied by 'm #vanquished'; 'd' is only supplied by
       dump_everything() when writing dumplog, so won't happen if built
       without '#define DUMPLOG' but there's no need for conditionals here */
    boolean force_sort = (defquery == 'A'),
            dumping = (defquery == 'd');

    /* normally we don't ask about sort order for the vanquished list unless
       it contains at least two entries; however, if player has used explicit
       'm #vanquished', choose order no matter what it contains so far */
    if (force_sort) { /* iflags.menu_requested via dovanquished() */
        /* choose value for vanq_sortmode via menu; ESC cancels choosing
           sort order but continues with vanquishd monsters display */
        (void) set_vanq_order(TRUE);
    }
    if (dumping || force_sort) {
        /* switch from 'A' or 'd' to 'y'; 'ask' is already False for the
           cases that might supply 'A' or 'd' */
        defquery = 'y';
        ask = FALSE; /* redundant */
    }

    /* get totals first */
    ntypes = 0;
    for (i = LOW_PM; i < NUMMONS; i++) {
        if ((nkilled = (int) svm.mvitals[i].died) == 0)
            continue;
        mindx[ntypes++] = i;
        total_killed += (long) nkilled;
    }

    /* vanquished creatures list;
     * includes all dead monsters, not just those killed by the player
     */
    if (ntypes != 0) {
        char mlet, prev_mlet = 0; /* used as small integer, not character */
        boolean class_header, uniq_header, Rider,
                was_uniq = FALSE, special_hdr = FALSE;

        if (ask) {
            char allow_yn[10];

            if (ntypes > 1) {
                Strcpy(allow_yn, ynaqchars);
            } else {
                Strcpy(allow_yn, ynqchars); /* don't include 'a', but */
                Strcat(allow_yn, "\033a");  /* allow user to answer 'a' */
                if (defquery == 'a') /* potential default from 'disclose' */
                    defquery = 'y';
            }
            c = yn_function("你想查看已击败生物的统计吗?",
                            allow_yn, defquery, TRUE);
        } else {
            c = defquery;
        }

        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            if (c == 'a' && ntypes > 1) { /* ask user to choose sort order */
                /* choose value for vanq_sortmode via menu; ESC cancels list
                   of vanquished monsters but does not set 'done_stopprint' */
                if (set_vanq_order(TRUE) < 0)
                    return;
            }
            uniq_header = (flags.vanq_sortmode == VANQ_ALPHA_SEP);
            class_header = ((flags.vanq_sortmode == VANQ_MCLS_LTOH
                             || flags.vanq_sortmode == VANQ_MCLS_HTOL)
                            && ntypes > 1);

            klwin = create_nhwindow(NHW_MENU);
            putstr(klwin, 0, "已击败的生物:");
            if (!dumping)
                putstr(klwin, 0, "");

            qsort((genericptr_t) mindx, ntypes, sizeof *mindx, vanqsort_cmp);
            for (ni = 0; ni < ntypes; ni++) {
                i = mindx[ni];
                nkilled = svm.mvitals[i].died;
                Rider = is_rider(&mons[i]);
                mlet = mons[i].mlet;
                if (class_header
                    && (mlet != prev_mlet || (special_hdr && !Rider))) {
                    if (!Rider) {
                        Strcpy(buf, def_monsyms[(int) mlet].explain);
                        special_hdr = FALSE;
                    } else {
                        Strcpy(buf, "骑手");
                        special_hdr = TRUE;
                    }
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                if (UniqCritterIndx(i)) {
                    Sprintf(buf, "%s%s",
                            !type_is_pname(&mons[i]) ? "" : "",
                            mons[i].pmnames[NEUTRAL]);
                    if (nkilled > 1)
                        Sprintf(eos(buf), " (%s)",
                                N_times((long) nkilled, buftoo));
                    was_uniq = TRUE;
                } else {
                    if (uniq_header && was_uniq) {
                        putstr(klwin, 0, "");
                        was_uniq = FALSE;
                    }
                    /* trolls or undead might have come back,
                       but we don't keep track of that */
                    if (nkilled == 1)
                        Strcpy(buf, an(mons[i].pmnames[NEUTRAL]));
                    else
                        Sprintf(buf, "%3d %s", nkilled,
                                makeplural(mons[i].pmnames[NEUTRAL]));
                }
                /* number of leading spaces to match 3 digit prefix */
                pfx = !strncmpi(buf, "the ", 4) ? 0
                      : !strncmpi(buf, "an ", 3) ? 1
                        : !strncmpi(buf, "a ", 2) ? 2
                          : !digit(buf[2]) ? 4 : 0;
                if (class_header)
                    ++pfx;
                Snprintf(buftoo, sizeof buftoo, "%*s%s", pfx, "", buf);
                putstr(klwin, 0, buftoo);
            }
            /*
             * if (Hallucination)
             *     putstr(klwin, 0, "以及梨树上的一只鹧鸪");
             */
            if (ntypes > 1) {
                if (!dumping)
                    putstr(klwin, 0, "");
                Sprintf(buf, "击败了 %ld 个生物.", total_killed);
                putstr(klwin, 0, buf);
            }
            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /*
     * For end-of-game disclosure, we're only called when some monsters
     * were vanquished and won't reach these 'else-if's.
     *
     * If no monsters have been vanquished, we're either called for game
     * still in progress, so use present tense via pline(), or for dumplog
     * which needs putstr() and past tense.
     */
    } else if (!program_state.gameover) {
        /* #vanquished rather than final disclosure, so pline() is ok */
        pline("还没有任何生物被击败.");
#ifdef DUMPLOG
    } else if (dumping) {
        putstr(0, 0, "没有生物被击败."); /* not pline() */
#endif
    }
}

/* number of monster species which have been genocided */
int
num_genocides(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (svm.mvitals[i].mvflags & G_GENOD) {
            ++n;
            if (UniqCritterIndx(i))
                impossible("unique creature '%d: %s' genocided?",
                           i, mons[i].pmnames[NEUTRAL]);
        }
    }
    return n;
}

/* return a count of the number of extinct species */
staticfn int
num_extinct(void)
{
    int i, n = 0;

    for (i = LOW_PM; i < NUMMONS; ++i) {
        if (UniqCritterIndx(i))
            continue;
        if ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT)
            ++n;
    }
    return n;
}

/* collect both genocides and extinctions, skipping uniques */
staticfn int
num_gone(int mvflags, int *mindx)
{
    uchar mflg = (uchar) mvflags;
    int i, n = 0;

    (void) memset((genericptr_t) mindx, 0, NUMMONS * sizeof *mindx);

    for (i = LOW_PM; i < NUMMONS; ++i) {
        /* uniques can't be genocided but can become extinct;
           however, they're never reported as extinct, so skip them */
        if (UniqCritterIndx(i))
            continue;

        if ((svm.mvitals[i].mvflags & mflg) != 0)
            mindx[n++] = i;
    }
    return n;
}

/* show genocided and extinct monster types for final disclosure/dumplog
   or for the #genocided command */
void
list_genocided(char defquery, boolean ask)
{
    int i, mndx;
    int ngenocided, nextinct, ngone, mvflags, mindx[NUMMONS];
    char c;
    winid klwin;
    char buf[BUFSZ];
    boolean genoing, /* prompting for genocide or class genocide */
            dumping; /* for DUMPLOG; doesn't need to be conditional */
    boolean both = (program_state.gameover || wizard || discover);

    dumping = (defquery == 'd');
    genoing = (defquery == 'g');
    if (dumping || genoing)
        defquery = 'y';
    if (genoing)
        both = FALSE; /* genocides only, not extinctions */

    /* this goes through the whole monster list up to three times but will
       happen rarely and is simpler than a more general single pass check;
       extinctions are only revealed during end of game disclosure or when
       running in wizard or explore mode */
    ngenocided = num_genocides();
    nextinct = both ? num_extinct() : 0;
    mvflags = G_GENOD | (both ? G_EXTINCT : 0);
    ngone = num_gone(mvflags, mindx);

    /* genocided or extinct species list */
    if (ngone > 0) {
        Sprintf(buf, "你想查看%s物种%s%s列表吗?",
                (nextinct && !ngenocided) ? "绝种的" : "",
                (ngenocided) ? "灭绝的" : "",
                (nextinct && ngenocided) ? "和绝种的" : "");
        c = ask ? yn_function(buf, (ngone > 1) ? "ynaq" : "ynq\033a",
                              defquery, TRUE)
                : defquery;
        if (c == 'q')
            done_stopprint++;
        if (c == 'y' || c == 'a') {
            int save_sortmode;
            char mlet, prev_mlet = 0;
            boolean class_header = FALSE;

            if (ngone > 1) {
                if (c == 'a') { /* ask player to choose sort order */
                    /* #genocided shares #vanquished's sort order */
                    if (set_vanq_order(FALSE) < 0)
                        return;
                }
                /* sort orderings count-high-to-low or count-low-to-high
                   don't make sense for genocides; if the preferred order
                   to set to either of those, use alphabetical instead;
                   note: the tie breaker for by-class is level-high-to-low
                   or level-low-to-high rather than count so is ok as-is */
                save_sortmode = flags.vanq_sortmode;
                if (flags.vanq_sortmode == VANQ_COUNT_H_L
                    || flags.vanq_sortmode == VANQ_COUNT_L_H)
                    flags.vanq_sortmode = VANQ_ALPHA_MIX;
                qsort((genericptr_t) mindx, ngone,
                      sizeof *mindx, vanqsort_cmp);
                class_header = (flags.vanq_sortmode == VANQ_MCLS_LTOH
                                || flags.vanq_sortmode == VANQ_MCLS_HTOL);
                flags.vanq_sortmode = save_sortmode;
            }

            klwin = create_nhwindow(NHW_MENU);
            Sprintf(buf, "%s%s物种:",
                    (ngenocided) ? "灭绝的" : "绝种的",
                    (nextinct && ngenocided) ? "或绝种的" : "");
            putstr(klwin, 0, buf);
            if (!dumping)
                putstr(klwin, 0, "");

            for (i = 0; i < ngone; ++i) {
                mndx = mindx[i];
                mlet = mons[mndx].mlet;
                if (class_header && mlet != prev_mlet) {
                    Strcpy(buf, def_monsyms[(int) mlet].explain);
                    /* 'ask' implies final disclosure, where highlighting
                       of various header lines is suppressed */
                    putstr(klwin, ask ? ATR_NONE : iflags.menu_headings.attr,
                           upstart(buf));
                    prev_mlet = mlet;
                }
                Sprintf(buf, " %s", makeplural(mons[mndx].pmnames[NEUTRAL]));
                /*
                 * "Extinct" is unfortunate terminology.  A species
                 * is marked extinct when its birth limit is reached,
                 * but there might be members of the species still
                 * alive, contradicting the meaning of the word.
                 *
                 * We only append "(extinct)" if the G_GENOD bit is
                 * clear.  During normal play, 'mndx' won't be in the
                 * collected list unless that bit is set.
                 */
                if ((svm.mvitals[mndx].mvflags & G_GONE) == G_EXTINCT)
                    Strcat(buf, " (已绝种)");
                putstr(klwin, 0, buf);
            }
            if (!dumping)
                putstr(klwin, 0, "");
            if (ngenocided > 0) {
                Sprintf(buf, "%d 个物种已灭绝.", ngenocided);
                putstr(klwin, 0, buf);
            }
            if (nextinct > 0) {
                Sprintf(buf, "%d 个物种已绝种.", nextinct);
                putstr(klwin, 0, buf);
            }

            display_nhwindow(klwin, TRUE);
            destroy_nhwindow(klwin);
        }

    /* See the comment for similar code near the end of list_vanquished(). */
    } else if (!program_state.gameover) {
        /* #genocided rather than final disclosure, so pline() is ok and
           extinction has been ignored */
        pline("%1$s没有生物被灭绝.", genoing ? "还" : "");
#ifdef DUMPLOG
    } else if (dumping) { /* 'gameover' is True if we make it here */
        putstr(0, 0, "没有物种被灭绝或绝种.");
#endif
    }
}

/* M-g - #genocided command */
int
dogenocided(void)
{
    list_genocided(iflags.menu_requested ? 'a' : 'y', FALSE);
    return ECMD_OK;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* #wizborn extended command */
int
doborn(void)
{
    static const char fmt[] = "%4i %4i %c %-30s";
    int i;
    winid datawin = create_nhwindow(NHW_TEXT);
    char buf[BUFSZ];
    int nborn = 0, ndied = 0;

    putstr(datawin, 0, "死亡 出生");
    for (i = LOW_PM; i < NUMMONS; i++)
        if (svm.mvitals[i].born || svm.mvitals[i].died
            || (svm.mvitals[i].mvflags & G_GONE) != 0) {
            Sprintf(buf, fmt,
                    svm.mvitals[i].died, svm.mvitals[i].born,
                    ((svm.mvitals[i].mvflags & G_GONE) == G_EXTINCT) ? 'E'
                    : ((svm.mvitals[i].mvflags & G_GONE) == G_GENOD) ? 'G'
                      : ((svm.mvitals[i].mvflags & G_GONE) != 0) ? 'X'
                        : ' ',
                    mons[i].pmnames[NEUTRAL]);
            putstr(datawin, 0, buf);
            nborn += svm.mvitals[i].born;
            ndied += svm.mvitals[i].died;
        }

    putstr(datawin, 0, "");
    Sprintf(buf, fmt, ndied, nborn, ' ', "");

    display_nhwindow(datawin, FALSE);
    destroy_nhwindow(datawin);

    return ECMD_OK;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/*
 * align_str(), piousness(), mstatusline() and ustatusline() once resided
 * in pline.c, then got moved to priest.c just to be out of there.  They
 * fit better here.
 */

const char *
align_str(aligntyp alignment)
{
    switch ((int) alignment) {
    case A_CHAOTIC:
        return "混沌";
    case A_NEUTRAL:
        return "中立";
    case A_LAWFUL:
        return "守序";
    case A_NONE:
        return "无阵营";
    }
    return "不明";
}

staticfn char *
size_str(int msize)
{
    static char outbuf[40];

    switch (msize) {
    case MZ_TINY:
        Strcpy(outbuf, "极小");
        break;
    case MZ_SMALL:
        Strcpy(outbuf, "小");
        break;
    case MZ_MEDIUM:
        Strcpy(outbuf, "中等");
        break;
    case MZ_LARGE:
        Strcpy(outbuf, "大");
        break;
    case MZ_HUGE:
        Strcpy(outbuf, "巨大");
        break;
    case MZ_GIGANTIC:
        Strcpy(outbuf, "巨型");
        break;
    default:
        Sprintf(outbuf, "未知尺寸 (%d)", msize);
        break;
    }
    return outbuf;
}

/* used for self-probing */
char *
piousness(boolean showneg, const char *suffix)
{
    static char buf[32]; /* bigger than "insufficiently neutral" */
    const char *pio;

    /* note: piousness 20 matches MIN_QUEST_ALIGN (quest.h) */
    if (u.ualign.record >= 20)
        pio = "虔诚地";
    else if (u.ualign.record > 13)
        pio = "虔敬地";
    else if (u.ualign.record > 8)
        pio = "热忱地";
    else if (u.ualign.record > 3)
        pio = "坚定地";
    else if (u.ualign.record == 3)
        pio = "";
    else if (u.ualign.record > 0)
        pio = "勉强地";
    else if (u.ualign.record == 0)
        pio = "名义上";
    else if (!showneg)
        pio = "不足以";
    else if (u.ualign.record >= -3)
        pio = "已偏离阵营";
    else if (u.ualign.record >= -8)
        pio = "已犯下罪行";
    else
        pio = "已严重违背阵营";

    Sprintf(buf, "%s", pio);
    if (suffix && (!showneg || u.ualign.record >= 0)) {
        if (u.ualign.record != 3)
            Strcat(buf, "");
        Strcat(buf, suffix);
    }
    return buf;
}

/* stethoscope or probing applied to monster -- one-line feedback */
void
mstatusline(struct monst *mtmp)
{
    aligntyp alignment = mon_aligntyp(mtmp);
    char info[BUFSZ], monnambuf[BUFSZ];

    info[0] = 0;
    if (mtmp->mtame) {
        Strcat(info, ", 驯服");
        if (wizard) {
            Sprintf(eos(info), " (%d", mtmp->mtame);
            if (!mtmp->isminion)
                Sprintf(eos(info), "; 饥饿度 %ld; 叼取值 %d",
                        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
            Strcat(info, ")");
        }
    } else if (mtmp->mpeaceful)
        Strcat(info, ", 和平的");

    if (mtmp->data == &mons[PM_LONG_WORM]) {
        int segndx, nsegs = count_wsegs(mtmp);

        /* the worm code internals don't consider the head to be one of
           the worm's segments, but we count it as such when presenting
           worm feedback to the player */
        if (!nsegs) {
            Strcat(info, ", 单节");
        } else {
            ++nsegs; /* include head in the segment count */
            segndx = wseg_at(mtmp, gb.bhitpos.x, gb.bhitpos.y);
            Sprintf(eos(info), ", %d 节的%d%s",
                    nsegs, segndx, ordin(segndx));
        }
    }
    if (ismnum(mtmp->cham) && mtmp->data != &mons[mtmp->cham])
        /* don't reveal the innate form (chameleon, vampire, &c),
           just expose the fact that this current form isn't it */
        Strcat(info, ", 变形生物");
    /* pets eating mimic corpses mimic while eating, so this comes first */
    if (mtmp->meating)
        Strcat(info, ", 在吃");
    /* a stethoscope exposes mimic before getting here so this
       won't be relevant for it, but wand of probing doesn't */
    if (mtmp->mundetected || mtmp->m_ap_type
        || visible_region_at(gb.bhitpos.x, gb.bhitpos.y))
        mhidden_description(mtmp,
                       MHID_PREFIX | MHID_ARTICLE | MHID_ALTMON | MHID_REGION,
                            eos(info));
    if (mtmp->mcan)
        Strcat(info, ", 被取消");
    if (mtmp->mconf)
        Strcat(info, ", 混乱");
    if (mtmp->mblinded || !mtmp->mcansee)
        Strcat(info, ", 盲目");
    if (mtmp->mstun)
        Strcat(info, ", 眩晕");
    if (mtmp->msleeping)
        Strcat(info, ", 睡着");
#if 0 /* unfortunately mfrozen covers temporary sleep and being busy
       * (donning armor, for instance) as well as paralysis */
    else if (mtmp->mfrozen)
        Strcat(info, ", 麻痹");
#else
    else if (mtmp->mfrozen || !mtmp->mcanmove)
        Strcat(info, ", 不能移动");
#endif
    /* [arbitrary reason why it isn't moving] */
    else if ((mtmp->mstrategy & STRAT_WAITMASK) != 0)
        Strcat(info, ", 沉思");
    if (mtmp->mflee)
        Strcat(info, ", 害怕");
    if (mtmp->mtrapped)
        Strcat(info, ", 被困住");
    if (mtmp->mspeed)
        Strcat(info, (mtmp->mspeed == MFAST) ? ", 快速"
                      : (mtmp->mspeed == MSLOW) ? ", 缓慢"
                         : ", [? 速度]");
    if (mtmp->minvis)
        Strcat(info, ", 隐形");
    if (mtmp == u.ustuck) {
        struct permonst *pm = u.ustuck->data;

        /* being swallowed/engulfed takes priority over sticks(youmonst);
           this used to have that backwards and checked sticks() first */
        Strcat(info, u.uswallow ? (digests(pm)
                                   ? ", 正在消化你"
                                   /* note: the "swallowing you" case won't
                                      happen because all animal engulfers
                                      either digest their victims (purple
                                      worm) or enfold them (trappers and
                                      lurkers above) */
                                   : (is_animal(pm) && !enfolds(pm))
                                     ? ", 正在吞咽你"
                                     : ", 正在吞噬你")
                     /* !u.uswallow; if both youmonst and ustuck are holders,
                        youmonst wins */
                     : (!sticks(gy.youmonst.data) ? ", 正在抓住你"
                                                 : ", 被你抓住"));
    }
    if (mtmp == u.usteed) {
        Strcat(info, ", 带着你");
        if (Wounded_legs) {
            /* EWounded_legs is used to track left/right/both rather than
               some form of extrinsic impairment; HWounded_legs is used for
               timeout; both apply to steed instead of hero when mounted */
            long legs = (EWounded_legs & BOTH_SIDES);
            const char *what = mbodypart(mtmp, LEG);

            if (legs == BOTH_SIDES)
                what = makeplural(what);
            Sprintf(eos(info), ", %s受伤了", what);
        }
    }
    if (mtmp->mleashed)
        Strcat(info, ", 被拴住");

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_YOUR, (char *) 0,
                               (SUPPRESS_IT | SUPPRESS_INVISIBLE), FALSE));

    pline("%s 的状态(%s, %s): 等级 %d  HP %d(%d)  护甲 %d%s.",
          monnambuf, align_str(alignment), size_str(mtmp->data->msize),
          mtmp->m_lev, mtmp->mhp, mtmp->mhpmax, find_mac(mtmp), info);
}

/* stethoscope or probing applied to hero -- one-line feedback */
void
ustatusline(void)
{
    NhRegion *reg;
    char info[BUFSZ];
    size_t ln;

    info[0] = '\0';
    if (Sick) {
        Strcat(info, ", 垂死于");
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, "食物中毒");
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, "和");
            Strcat(info, "疾病");
        }
    }
    if (Stoned)
        Strcat(info, ", 固化");
    if (Slimed)
        Strcat(info, ", 黏液化");
    if (Strangled)
        Strcat(info, ", 被窒息");
    if (Vomiting)
        Strcat(info, ", 作呕"); /* !"nauseous" */
    if (Confusion)
        Strcat(info, ", 混乱");
    if (Blind) {
        Strcat(info, ", 失明");
        if (u.ucreamed) {
            if ((long) u.ucreamed < BlindedTimeout || Blindfolded
                || !haseyes(gy.youmonst.data))
                Strcat(info, ", 满身都是");
            Strcat(info, "黏糊糊的东西");
        } /* note: "goop" == "glop"; variation is intentional */
    }
    if (Stunned)
        Strcat(info, ", 眩晕");
    if (Wounded_legs && !u.usteed) {
        /* EWounded_legs is used to track left/right/both rather than some
           form of extrinsic impairment; HWounded_legs is used for timeout;
           both apply to steed instead of hero when mounted */
        long legs = (EWounded_legs & BOTH_SIDES);
        const char *what = body_part(LEG);

        if (legs == BOTH_SIDES)
            what = makeplural(what);
        /* when it's just one leg, ^X reports which, left or right;
           ustatusline() doesn't, in order to keep the output a bit shorter */
        Sprintf(eos(info), ", %s受伤了", what);
    }
    if (Glib)
        Sprintf(eos(info), ", 很滑的%s", fingers_or_gloves(TRUE));
    if (u.utrap)
        Strcat(info, ", 被困");
    if (Fast)
        Strcat(info, Very_fast ? ", 非常快" : ", 快");
    if (u.uundetected)
        Strcat(info, ", 隐蔽的");
    else if (U_AP_TYPE != M_AP_NOTHING)
        Strcat(info, ", 伪装");
    if (Invis)
        Strcat(info, ", 隐身");
    if (u.ustuck) {
        if (u.uswallow)
            Strcat(info, digests(u.ustuck->data) ? ", 正在被...消化"
                                                 : ", 被...吞噬");
        else if (!sticks(gy.youmonst.data))
            Strcat(info, ", 受牵制于");
        else
            Strcat(info, ", 牵制着");
        /* FIXME? a_monnam() uses x_monnam() which has a special case that
           forces "the" instead of "a" when formatting u.ustuck while hero
           is swallowed; we don't really want that here but it isn't worth
           fiddling with just for self-probing while engulfed */
        Strcat(info, a_monnam(u.ustuck));
    }
    if (!u.uswallow
        && (reg = visible_region_at(u.ux, u.uy)) != 0
        && (ln = strlen(info)) < sizeof info)
        Snprintf(eos(info), sizeof info - ln, ", 在%s云雾中",
                 reg_damg(reg) ? "毒气" : "蒸汽");

    pline("状态: %s(%s): 等级 %d  HP %d(%d)  AC %d%s.", svp.plname,
          piousness(FALSE, align_str(u.ualign.type)),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel, Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax, u.uac, info);
}

/* for 'onefile' processing where end of this file isn't necessarily the
   end of the source code seen by the compiler */
#undef enl_msg
#undef you_are
#undef you_have
#undef you_can
#undef you_have_been
#undef you_have_never
#undef you_have_X
#undef LL_majors
#undef majorevent
#undef spoilerevent
#undef UniqCritterIndx
#undef done_stopprint

/*insight.c*/
