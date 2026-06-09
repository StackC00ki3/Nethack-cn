/* NetHack 5.0	engrave.c	$NHDT-Date: 1737345573 2025/01/19 19:59:33 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.165 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* doengrave() data */
struct _doengrave_ctx {
    boolean dengr;    /* TRUE if we wipe out the current engraving */
    boolean doblind;  /* TRUE if engraving blinds the player */
    boolean doknown;  /* TRUE if we identify the stylus */
    boolean eow;      /* TRUE if we are overwriting oep */
    boolean jello;    /* TRUE if we are engraving in slime */
    boolean ptext;    /* TRUE if we must prompt for engrave text */
    boolean teleengr; /* TRUE if we move the old engraving */
    boolean zapwand;  /* TRUE if we remove a wand charge */
    boolean disprefresh; /* TRUE if the display needs a refresh */
    boolean frosted;  /* TRUE if engraving on ice */
    boolean adding;   /* TRUE if adding to existing engraving */

    int ret;          /* doengrave return value */
    int type;         /* Type of engraving made */
    int oetype;       /* will be set to type of current engraving */

    struct obj *otmp; /* Object selected with which to engrave */
    struct engr* oep; /* The current engraving */

    char buf[BUFSZ];          /* Buffer for final/poly engraving text */
    char ebuf[BUFSZ];         /* Buffer for initial engraving text */
    char fbuf[BUFSZ];         /* Buffer for "your fingers" */
    char qbuf[QBUFSZ];        /* Buffer for query text */
    char post_engr_text[BUFSZ]; /* Text displayed after engraving prompt */
    char *writer;      /* text of item used for writing */
    const char *everb; /* Present tense of engraving type */
    const char *eloc;  /* Where the engraving is (ie dust/floor/...) */

    size_t len;          /* # of nonspace chars of new engraving text */
};
#ifndef SFCTOOL
staticfn int stylus_ok(struct obj *);
staticfn boolean u_can_engrave(void);
staticfn void doengrave_ctx_init(struct _doengrave_ctx *);
staticfn void doengrave_sfx_item_WAN(struct _doengrave_ctx *);
staticfn boolean doengrave_sfx_item(struct _doengrave_ctx *);
staticfn void doengrave_ctx_verb(struct _doengrave_ctx *);
staticfn int engrave(void);
staticfn const char *blengr(void);

char *
random_engraving(char *outbuf, char *pristine_copy)
{
    const char *rumor;

    /* a random engraving may come from the "rumors" file,
       or from the "engrave" file (formerly in an array here) */
    if (!rn2(4) || !(rumor = getrumor(0, pristine_copy, TRUE)) || !*rumor)
        (void) get_rnd_text(ENGRAVEFILE, pristine_copy, rn2, MD_PAD_RUMORS);

    Strcpy(outbuf, pristine_copy);
    wipeout_text(outbuf, (int) (strlen(outbuf) / 4), 0);
    return outbuf;
}

/* Partial rubouts for engraving characters. -3. */
static const struct {
    char wipefrom;
    const char *wipeto;
} rubouts[] = { { 'A', "^" },
                { 'B', "Pb[" },
                { 'C', "(" },
                { 'D', "|)[" },
                { 'E', "|FL[_" },
                { 'F', "|-" },
                { 'G', "C(" },
                { 'H', "|-" },
                { 'I', "|" },
                { 'K', "|<" },
                { 'L', "|_" },
                { 'M', "|" },
                { 'N', "|\\" },
                { 'O', "C(" },
                { 'P', "F" },
                { 'Q', "C(" },
                { 'R', "PF" },
                { 'T', "|" },
                { 'U', "J" },
                { 'V', "/\\" },
                { 'W', "V/\\" },
                { 'Z', "/" },
                { 'b', "|" },
                { 'd', "c|" },
                { 'e', "c" },
                { 'g', "c" },
                { 'h', "n" },
                { 'j', "i" },
                { 'k', "|" },
                { 'l', "|" },
                { 'm', "nr" },
                { 'n', "r" },
                { 'o', "c" },
                { 'q', "c" },
                { 'w', "v" },
                { 'y', "v" },
                { ':', "." },
                { ';', ",:" },
                { ',', "." },
                { '=', "-" },
                { '+', "-|" },
                { '*', "+" },
                { '@', "0" },
                { '0', "C(" },
                { '1', "|" },
                { '6', "o" },
                { '7', "/" },
                { '8', "3o" } };

/* degrade some of the characters in a string */
void
wipeout_text(
    char *engr,    /* engraving text */
    int cnt,       /* number of chars to degrade */
    unsigned seed) /* for semi-controlled randomization */
{
    char *s;
    int i, j, nxt, use_rubout;
    unsigned lth = (unsigned) strlen(engr);

    if (lth && cnt > 0) {
        while (cnt--) {
            /* pick next character */
            if (!seed) {
                /* random */
                nxt = rn2((int) lth);
                use_rubout = rn2(4);
            } else {
                /* predictable; caller can reproduce the same sequence by
                   supplying the same arguments later, or a pseudo-random
                   sequence by varying any of them */
                nxt = seed % lth;
                seed *= 31, seed %= (BUFSZ - 1);
                use_rubout = seed & 3;
            }
            s = &engr[nxt];
            if (*s == ' ')
                continue;

            /* rub out unreadable & small punctuation marks */
            if (strchr("?.,'`-|_", *s)) {
                *s = ' ';
                continue;
            }

            if (!use_rubout) {
                i = SIZE(rubouts);
            } else {
                for (i = 0; i < SIZE(rubouts); i++)
                    if (*s == rubouts[i].wipefrom) {
                        unsigned ln = (unsigned) strlen(rubouts[i].wipeto);
                        /*
                         * Pick one of the substitutes at random.
                         */
                        if (!seed) {
                            j = rn2((int) ln);
                        } else {
                            seed *= 31, seed %= (BUFSZ - 1);
                            j = seed % ln;
                        }
                        *s = rubouts[i].wipeto[j];
                        break;
                    }
            }

            /* didn't pick rubout; use '?' for unreadable character */
            if (i == SIZE(rubouts))
                *s = '?';
        }
    }

    /* trim trailing spaces */
    while (lth && engr[lth - 1] == ' ')
        engr[--lth] = '\0';
}

/* check whether hero can reach something at ground level */
boolean
can_reach_floor(boolean check_pit)
{
    struct trap *t;

    if (u.uswallow
        || (u.ustuck && !sticks(gy.youmonst.data)
            /* assume that arms are pinned rather than that the hero
               has been lifted up above the floor [doesn't explain
               how hero can attack the creature holding him or her;
               that's life in nethack...] */
            && attacktype(u.ustuck->data, AT_HUGS))
        || (Levitation && !(Is_airlevel(&u.uz) || Is_waterlevel(&u.uz))))
        return FALSE;
    /* Restricted/unskilled riders can't reach the floor */
    if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
        return FALSE;
    if (u.uundetected && ceiling_hider(gy.youmonst.data))
        return FALSE;

    if (Flying || gy.youmonst.data->msize >= MZ_HUGE)
        return TRUE;

    if (check_pit && (t = t_at(u.ux, u.uy)) != 0
        && (uteetering_at_seen_pit(t) || uescaped_shaft(t)))
        return FALSE;

    return TRUE;
}

/* give a message after caller has determined that hero can't reach */
void
cant_reach_floor(coordxy x, coordxy y, boolean up,
                 boolean check_pit, boolean wand_engraving)
{
    pline("%s够不到%s.",
          wand_engraving
              ? "魔杖不再有任何动静, 且魔杖的尖端"
              : "你",
          up  ? ceiling(x, y)
              : (check_pit && can_reach_floor(FALSE)) ? "坑底"
                                                      : surface(x, y));
}

struct engr *
engr_at(coordxy x, coordxy y)
{
    struct engr *ep = head_engr;

    while (ep) {
        if (x == ep->engr_x && y == ep->engr_y)
            return ep;
        ep = ep->nxt_engr;
    }
    return (struct engr *) 0;
}

/* Decide whether a particular string is engraved at a specified
 * location; a case-insensitive substring match is used.
 * Ignore headstones, in case the player names herself "Elbereth".
 *
 * If strict checking is requested, the word is only considered to be
 * present if it is intact and is the entire content of the engraving.
 */
struct engr *
sengr_at(const char *s, coordxy x, coordxy y, boolean strict)
{
    struct engr *ep = engr_at(x, y);

    if (ep && ep->engr_type != HEADSTONE && ep->engr_time <= svm.moves) {
        if (strict ? !strcmpi(ep->engr_txt[actual_text], s)
                   : (strstri(ep->engr_txt[actual_text], s) != 0))
            return ep;
    }
    return (struct engr *) NULL;
}

void
u_wipe_engr(int cnt)
{
    if (can_reach_floor(TRUE))
        wipe_engr_at(u.ux, u.uy, cnt, FALSE);
}

void
wipe_engr_at(coordxy x, coordxy y, xint16 cnt, boolean magical)
{
    struct engr *ep = engr_at(x, y);

    /* Headstones and some specially marked engravings are indelible */
    if (ep && ep->engr_type != HEADSTONE && !ep->nowipeout) {
        debugpline1("asked to erode %d characters", cnt);
        if (ep->engr_type != BURN || is_ice(x, y) || (magical && !rn2(2))) {
            if (ep->engr_type != DUST && ep->engr_type != ENGR_BLOOD) {
                cnt = rn2(1 + 50 / (cnt + 1)) ? 0 : 1;
                debugpline1("actually eroding %d characters", cnt);
            }
            wipeout_text(ep->engr_txt[actual_text], (int) cnt, 0);
            while (ep->engr_txt[actual_text][0] == ' ')
                ep->engr_txt[actual_text]++;
            if (!ep->engr_txt[actual_text][0])
                del_engr(ep);
        }
    }
}

/*
 * Returns:
 *    non-zero if it can be felt
 */
boolean
engr_can_be_felt(struct engr *ep)
{
    boolean canfeel = FALSE;

    switch (ep->engr_type) {
        case ENGRAVE:
        case HEADSTONE:
        case BURN:
            canfeel = TRUE;
            break;
        case DUST:
        case MARK:
        case ENGR_BLOOD:
        default:
            canfeel = FALSE;
            break;
    }
    return canfeel;
}

void
read_engr_at(coordxy x, coordxy y)
{
    struct engr *ep = engr_at(x, y);
    const char *eloc = surface(x, y);
    int sensed = 0;

    /* Sensing an engraving does not require sight for some engraving types,
     * nor does it necessarily imply comprehension (literacy).
     */
    if (ep && ep->engr_txt[actual_text][0]) {
        switch (ep->engr_type) {
        case DUST:
            if (!Blind) {
                sensed = 1;
                pline("%s写在%s里.", Something,
                      is_ice(x, y) ? "霜" : "灰尘");
            }
            break;
        case ENGRAVE:
        case HEADSTONE:
            if (!Blind || can_reach_floor(TRUE)) {
                sensed = 1;
                pline("%s被刻在%s上.", Something, eloc);
            }
            break;
        case BURN:
            if (!Blind || can_reach_floor(TRUE)) {
                sensed = 1;
                pline("一些文字已经%s在%s里.",
                      is_ice(x, y) ? "融" : "烧", eloc);
            }
            break;
        case MARK:
            if (!Blind) {
                sensed = 1;
                pline("这里有一些涂鸦在%s上.", eloc);
            }
            break;
        case ENGR_BLOOD:
            /* "It's a message!  Scrawled in blood!"
             * "What's it say?"
             * "It says... `See you next Wednesday.'" -- Thriller
             */
            if (!Blind) {
                sensed = 1;
                You_see("一条消息写在血液里.");
            }
            break;
        default:
            impossible("%s is written in a very strange way.", Something);
            sensed = 1;
        }

        if (sensed) {
            char *et, buf[BUFSZ];
            const char *endpunct;
            int maxelen = (int) (sizeof buf
                                 /* sizeof "literal" counts terminating \0 */
                                 - sizeof "You feel the words: \"\"."),
                elen = (int) strlen(ep->engr_txt[actual_text]),
                off = (int) (ep->engr_txt[actual_text] - engr_text_space(ep));

            if (elen > maxelen) {
                (void) strncpy(buf, ep->engr_txt[actual_text], maxelen);
                buf[maxelen] = '\0';
                et = buf;
                elen = maxelen;
            } else {
                et = ep->engr_txt[actual_text];
            }
            endpunct = "";
            if (elen < 2
                /* only skip if punctuation is original, not degraded char */
                || !((ep->engr_txt[pristine_text][off + elen - 1]
                      == et[elen - 1])
                     && strchr(".!?", et[elen - 1]))) {
                endpunct = ".";
            }
            You("%s: \"%s\"%s", (Blind) ? "感觉到文字" : "读到", et,
                endpunct);
            Strcpy(ep->engr_txt[remembered_text], ep->engr_txt[actual_text]);
            ep->eread = 1;
            ep->erevealed = 1;
            if (svc.context.run > 0)
                nomul(0);
        }
    }
}

void
make_engr_at(
    coordxy x, coordxy y,
    const char *s,
    const char *pristine_s,
    long e_time,
    int e_type)
{
    int i;
    struct engr *ep;
    unsigned smem = Strlen(s) + 1;
    boolean havepristine = FALSE;

    if (pristine_s != NULL) {
        unsigned prmem = Strlen(pristine_s) + 1;
        if (prmem > smem)
            smem = prmem;
        havepristine = TRUE;
    }
    if ((ep = engr_at(x, y)) != 0)
        del_engr(ep);

    ep = newengr(smem * 3);
    (void) memset((genericptr_t) ep, 0, (smem * 3) + sizeof (struct engr));
    ep->nxt_engr = head_engr;
    head_engr = ep;
    ep->engr_x = x;
    ep->engr_y = y;
    ep->engr_txt[actual_text] = engr_text_space(ep);
    ep->engr_txt[remembered_text] = ep->engr_txt[actual_text] + smem;
    ep->engr_txt[pristine_text] = ep->engr_txt[remembered_text] + smem;
    for(i = 0; i < text_states; ++i)
        Strcpy(ep->engr_txt[i], s);
    if (havepristine)
        Strcpy(ep->engr_txt[pristine_text], pristine_s);
    if (!strcmp(s, "Elbereth")) {
        /* engraving "Elbereth":  if done when making a level, it creates
           an old-style Elbereth that deters monsters when any objects are
           present; otherwise (done by the player), exercises wisdom */
        if (gi.in_mklev)
            ep->guardobjects = 1;
        else
            exercise(A_WIS, TRUE);
    }
    ep->engr_time = e_time;
    ep->engr_type = (xint8) ((e_type > 0) ? e_type : rnd(N_ENGRAVE - 1));
    ep->engr_szeach = smem;
    ep->engr_alloc = smem * 3;
    /* we do not set ep->eread or ep->erevealed;
     * the caller will need to if required */
}

/* delete any engraving at location <x,y> */
void
del_engr_at(coordxy x, coordxy y)
{
    struct engr *ep = engr_at(x, y);

    if (ep)
        del_engr(ep);
}

/*
 * freehand - returns true if player has a free hand
 */
int
freehand(void)
{
    return (!uwep || !welded(uwep)
            || (!bimanual(uwep) && (!uarms || !uarms->cursed)));
}

/* getobj callback for an object to engrave with */
staticfn int
stylus_ok(struct obj *obj)
{
    if (!obj)
        return GETOBJ_SUGGEST;

    /* Potential extension: exclude weapons that don't make any sense (such as
     * bullwhips) and downplay rings and gems that wouldn't be good to write
     * with (such as glass and non-gem rings) */
    if (obj->oclass == WEAPON_CLASS || obj->oclass == WAND_CLASS
        || obj->oclass == GEM_CLASS || obj->oclass == RING_CLASS)
        return GETOBJ_SUGGEST;

    /* Only markers and towels are recommended tools. */
    if (obj->oclass == TOOL_CLASS
        && (obj->otyp == TOWEL || obj->otyp == MAGIC_MARKER))
        return GETOBJ_SUGGEST;

    return GETOBJ_DOWNPLAY;
}

/* can hero engrave at all (at their location)? */
staticfn boolean
u_can_engrave(void)
{
    int levtyp = SURFACE_AT(u.ux, u.uy);

    if (u.uswallow) {
        if (is_animal(u.ustuck->data)) {
            pline("你想写什么?  \"约拿到此一游\"?");
            return FALSE;
        } else if (is_whirly(u.ustuck->data)) {
            cant_reach_floor(u.ux, u.uy, FALSE, FALSE, FALSE);
            return FALSE;
        }
        /* Note: for amorphous engulfers, writing attempt is allowed here
           but yields the 'jello' result in doengrave() */
    } else if (is_lava(u.ux, u.uy)) {
        You_cant("在%s上写字!", surface(u.ux, u.uy));
        return FALSE;
    } else if (is_pool(u.ux, u.uy) || IS_FOUNTAIN(levtyp)) {
        You_cant("在%s上写!", surface(u.ux, u.uy));
        return FALSE;
    } else if (IS_AIR(levtyp)) {
        /* airlevel or inside bubble on waterlevel */
        You_cant("在%s里写字!",
                 (levtyp == CLOUD) ? "云气" : "稀薄空气");
        return FALSE;
    } else if (!ACCESSIBLE(levtyp)) {
        /* stone, tree, wall, secret corridor, pool, lava, bars */
        You_cant("在这里写.");
        return FALSE;
    }

    if (cantwield(gy.youmonst.data)) {
        You_cant("手持任何东西!");
        return FALSE;
    }
    if (check_capacity((char *) 0))
        return FALSE;
    return TRUE;
}

/* initialize the doengrave data */
staticfn void
doengrave_ctx_init(struct _doengrave_ctx *de)
{
    de->dengr = FALSE;
    de->doblind = FALSE;
    de->doknown = FALSE;
    de->eow = FALSE;
    de->ptext = TRUE;
    de->teleengr = FALSE;
    de->zapwand = FALSE;
    de->disprefresh = FALSE;
    de->adding = FALSE;

    de->ret = ECMD_OK;
    de->type = DUST;
    de->oetype = 0;

    de->otmp = (struct obj *) 0;
    de->oep = engr_at(u.ux, u.uy);

    de->buf[0] = (char) 0;
    de->ebuf[0] = (char) 0;
    de->fbuf[0] = (char) 0;
    de->qbuf[0] = (char) 0;
    de->post_engr_text[0] = (char) 0;
    de->writer = (char *) 0;

    if (de->oep)
        de->oetype = de->oep->engr_type;
    if (is_demon(gy.youmonst.data) || is_vampire(gy.youmonst.data))
        de->type = ENGR_BLOOD;

    de->jello = (u.uswallow && !(is_animal(u.ustuck->data)
                                 || is_whirly(u.ustuck->data)));
    de->frosted = is_ice(u.ux, u.uy);
}

/* special engraving effects for WAND objects */
staticfn void
doengrave_sfx_item_WAN(struct _doengrave_ctx *de)
{
    switch (de->otmp->otyp) {
        /* DUST wands */
    default:
        break;
        /* NODIR wands */
    case WAN_LIGHT:
    case WAN_SECRET_DOOR_DETECTION:
    case WAN_STASIS:
    case WAN_CREATE_MONSTER:
    case WAN_WISHING:
    case WAN_ENLIGHTENMENT:
        zapnodir(de->otmp);
        break;
        /* IMMEDIATE wands */
        /* If wand is "IMMEDIATE", remember to affect the
         * previous engraving even if turning to dust.
         */
    case WAN_STRIKING:
        Strcpy(de->post_engr_text,
               "魔杖没能反抗你的刻写!");
        break;
    case WAN_SLOW_MONSTER:
        if (!Blind) {
            Sprintf(de->post_engr_text, "%s上的臭虫速度变慢了!",
                    surface(u.ux, u.uy));
        }
        break;
    case WAN_SPEED_MONSTER:
        if (!Blind) {
            Sprintf(de->post_engr_text, "%s上的臭虫速度加快了!",
                    surface(u.ux, u.uy));
        }
        break;
    case WAN_POLYMORPH:
        if (de->oep) {
            if (!Blind) {
                de->type = (xint16) 0; /* random */
                (void) random_engraving(de->buf, de->ebuf);
            } else {
                /* keep the same type so that feels don't
                   change and only the text is altered,
                   but you won't know anyway because
                   you're a _blind writer_ */
                if (de->oetype)
                    de->type = de->oetype;
                xcrypt(blengr(), de->buf);
            }
            de->dengr = TRUE;
        }
        break;
    case WAN_NOTHING:
    case WAN_UNDEAD_TURNING:
    case WAN_OPENING:
    case WAN_LOCKING:
    case WAN_PROBING:
        break;
        /* RAY wands */
    case WAN_MAGIC_MISSILE:
        de->ptext = TRUE;
        if (!Blind) {
            Sprintf(de->post_engr_text,
                    "%s上都是弹孔!",
                    surface(u.ux, u.uy));
        }
        break;
        /* can't tell sleep from death - Eric Backus */
    case WAN_SLEEP:
    case WAN_DEATH:
        if (!Blind) {
            Sprintf(de->post_engr_text, "%s上的臭虫停止了移动!",
                    surface(u.ux, u.uy));
        }
        break;
    case WAN_COLD:
        if (!Blind)
            Strcpy(de->post_engr_text,
                   "一些冰块从魔杖上掉下来了.");
        if (!de->oep || (de->oep->engr_type != BURN))
            break;
        FALLTHROUGH;
        /*FALLTHRU*/
    case WAN_CANCELLATION:
    case WAN_MAKE_INVISIBLE:
        if (de->oep && de->oep->engr_type != HEADSTONE) {
            if (!Blind)
                pline_The("%s上的刻字消失了!",
                          surface(u.ux, u.uy));
            de->dengr = TRUE;
        }
        break;
    case WAN_TELEPORTATION:
        if (de->oep && de->oep->engr_type != HEADSTONE) {
            if (!Blind)
                pline_The("%s上的刻字消失了!",
                          surface(u.ux, u.uy));
            de->teleengr = TRUE;
        }
        break;
        /* type = ENGRAVE wands */
    case WAN_DIGGING:
        de->ptext = TRUE;
        de->type = ENGRAVE;
        if (!objects[de->otmp->otyp].oc_name_known) {
            if (flags.verbose)
                pline("这个%s是一把挖掘魔杖!", xname(de->otmp));
            de->doknown = TRUE;
        }
        Strcpy(de->post_engr_text,
               (Blind && !Deaf)
                ? "你听到了钻孔声!"    /* Deaf-aware */
                : Blind
                   ? "你感到震动."
                   : IS_GRAVE(levl[u.ux][u.uy].typ)
                      ? "碎片从墓碑上飞出."
                      : de->frosted
                         ? "冰屑从冰面飞起!"
                         : (svl.level.locations[u.ux][u.uy].typ
                           == DRAWBRIDGE_DOWN)
                            ? "木屑从桥上飞起."
                            : "碎石从地面飞起.");
        break;
        /* type = BURN wands */
    case WAN_FIRE:
        de->ptext = TRUE;
        de->type = BURN;
        if (!objects[de->otmp->otyp].oc_name_known) {
            if (flags.verbose)
                pline("这个%s是一把火焰魔杖!", xname(de->otmp));
            de->doknown = TRUE;
        }
        Strcpy(de->post_engr_text, Blind ? "你感到魔杖热了起来."
                                         : "火焰从魔杖中飞出.");
        break;
    case WAN_LIGHTNING:
        de->ptext = TRUE;
        de->type = BURN;
        if (!objects[de->otmp->otyp].oc_name_known) {
            if (flags.verbose)
                pline("这根%s是一根闪电魔杖!", xname(de->otmp));
            de->doknown = TRUE;
        }
        if (!Blind) {
            Strcpy(de->post_engr_text, "闪电从魔杖中呈弧形射出.");
            de->doblind = TRUE;
        } else {
            Strcpy(de->post_engr_text, !Deaf
                   ? "你听到噼啪声!"     /* Deaf-aware */
                   : "你的头发竖起来了!");
        }
        break;
        /* type = MARK wands */
        /* type = ENGR_BLOOD wands */
    }
}

/* special engraving effects for all objects */
staticfn boolean
doengrave_sfx_item(struct _doengrave_ctx *de)
{
    switch (de->otmp->oclass) {
    default:
    case AMULET_CLASS:
    case CHAIN_CLASS:
    case POTION_CLASS:
    case COIN_CLASS:
        break;
    case RING_CLASS:
        /* "diamond" rings and others should work */
    case GEM_CLASS:
        /* diamonds & other hard gems should work */
        if (objects[de->otmp->otyp].oc_tough) {
            de->type = ENGRAVE;
            break;
        }
        break;
    case ARMOR_CLASS:
        if (is_boots(de->otmp)) {
            de->type = DUST;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    /* Objects too large to engrave with */
    case BALL_CLASS:
    case ROCK_CLASS:
        You_cant("用大东西来刻写!");
        de->ptext = FALSE;
        break;
    /* Objects too silly to engrave with */
    case FOOD_CLASS:
    case SCROLL_CLASS:
    case SPBOOK_CLASS:
        pline("%s会变得%s.", Yname2(de->otmp),
              de->frosted ? "结霜" : "太脏");
        de->ptext = FALSE;
        break;
    case RANDOM_CLASS: /* This should mean fingers */
        break;

    /* The charge is removed from the wand before prompting for
     * the engraving text, because all kinds of setup decisions
     * and pre-engraving messages are based upon knowing what type
     * of engraving the wand is going to do.  Also, the player
     * will have potentially seen "You wrest .." message, and
     * therefore will know they are using a charge.
     */
    case WAND_CLASS:
        if (zappable(de->otmp)) {
            check_unpaid(de->otmp);
            if (de->otmp->cursed && !rn2(WAND_BACKFIRE_CHANCE)) {
                wand_explode(de->otmp, 0);
                de->ret = ECMD_TIME;
                return FALSE;
            }
            de->zapwand = TRUE;
            if (!can_reach_floor(TRUE))
                de->ptext = FALSE;
            doengrave_sfx_item_WAN(de);
        } else { /* end if zappable */
            /* failing to wrest one last charge takes time */
            de->ptext = FALSE; /* use "early exit" below, return 1 */
            /* give feedback here if we won't be getting the
               "can't reach floor" message below */
            if (can_reach_floor(TRUE)) {
                /* cancelled wand turns to dust */
                if (de->otmp->spe < 0)
                    de->zapwand = TRUE;
                /* empty wand just doesn't write */
                else
                    pline_The("魔杖磨损过度, 无法刻写.");
            }
        }
        break;

    case WEAPON_CLASS:
        if (is_art(de->otmp, ART_FIRE_BRAND)) {
            de->type = BURN; /* doesn't dull weapon */
        } else if (is_blade(de->otmp)) {
            /* if non-blade or welded or too dull, engraving type stays set
               to DUST; feedback for that is only given for bladed weapons */
            if (welded(de->otmp))
                pline("%s只能刮擦%s.",
                      Yname2(de->otmp), surface(u.ux, u.uy));
            else if ((int) de->otmp->spe <= -3)
                pline("%s太钝, 无法刻字.",
                      Yobjnam2(de->otmp, "是"));
            else
                de->type = ENGRAVE;
        }
        break;

    case TOOL_CLASS:
        if (de->otmp == ublindf) {
            pline(
                "用那个刻东西有点困难, 你不觉得吗?");
            de->ret = ECMD_FAIL;
            return FALSE;
        }
        switch (de->otmp->otyp) {
        case MAGIC_MARKER:
            if (de->otmp->spe <= 0)
                Your("标记笔已经干涸了.");
            else
                de->type = MARK;
            break;
        case TOWEL:
            /* Can't really engrave with a towel */
            de->ptext = FALSE;
            if (de->oep) {
                if (de->oep->engr_type == DUST
                    || de->oep->engr_type == ENGR_BLOOD
                    || de->oep->engr_type == MARK) {
                    if (is_wet_towel(de->otmp))
                        dry_a_towel(de->otmp, -1, TRUE);
                    if (!Blind)
                        You("抹掉了这里的消息.");
                    else
                        pline("%s %s.", Yobjnam2(de->otmp, "变得"),
                              de->frosted ? "冰冷的" : "布满灰尘的");
                    de->dengr = TRUE;
                } else {
                    pline("%s无法擦除这个雕刻.",
                          Yname2(de->otmp));
                }
            } else {
                pline("%s %s.", Yobjnam2(de->otmp, "变得"),
                      de->frosted ? "冰冷" : "布满灰尘");
            }
            break;
        default:
            break;
        }
        break;

    case VENOM_CLASS:
        /* this used to be ``if (wizard)'' and fall through to ILLOBJ_CLASS
           for normal play, but splash of venom isn't "illegal" because it
           could occur in normal play via wizard mode bones */
        pline("你在写一封毒笔信吗?");
        break;

    case ILLOBJ_CLASS:
        impossible("You're engraving with an illegal object!");
        break;
    }

    return TRUE;
}

/* which verb phrasing to use for engraving */
staticfn void
doengrave_ctx_verb(struct _doengrave_ctx *de)
{
    switch (de->type) {
    default:
        de->everb = de->adding ? "添加奇怪字迹到"
                               : "奇怪地写到";
        break;
    case DUST:
        de->everb = de->adding ? "添加字迹到" : "写到";
        de->eloc = de->frosted ? "霜" : "灰尘";
        break;
    case HEADSTONE:
        de->everb = de->adding ? "添加墓志铭到" : "刻到";
        break;
    case ENGRAVE:
        de->everb = de->adding ? "添加刻字到" : "刻到";
        break;
    case BURN:
        de->everb = de->adding ? (de->frosted ? "添加融入的文字到"
                                  : "添加烧入的文字到")
                       : (de->frosted ? "融入" : "烧入");
        break;
    case MARK:
        de->everb = de->adding ? "添加涂鸦到" : "涂写到";
        break;
    case ENGR_BLOOD:
        de->everb = de->adding ? "添加血字到" : "用血潦草写到";
        break;
    }
}

/* Mohs' Hardness Scale:
 *  1 - Talc             6 - Orthoclase
 *  2 - Gypsum           7 - Quartz
 *  3 - Calcite          8 - Topaz
 *  4 - Fluorite         9 - Corundum
 *  5 - Apatite         10 - Diamond
 *
 * Since granite is an igneous rock hardness ~ 7, anything >= 8 should
 * probably be able to scratch the rock.
 * Devaluation of less hard gems is not easily possible because obj struct
 * does not contain individual oc_cost currently. 7/91
 *
 * steel      - 5-8.5   (usu. weapon)
 * diamond    - 10                      * jade       -  5-6      (nephrite)
 * ruby       -  9      (corundum)      * turquoise  -  5-6
 * sapphire   -  9      (corundum)      * opal       -  5-6
 * topaz      -  8                      * glass      - ~5.5
 * emerald    -  7.5-8  (beryl)         * dilithium  -  4-5??
 * aquamarine -  7.5-8  (beryl)         * iron       -  4-5
 * garnet     -  7.25   (var. 6.5-8)    * fluorite   -  4
 * agate      -  7      (quartz)        * brass      -  3-4
 * amethyst   -  7      (quartz)        * gold       -  2.5-3
 * jasper     -  7      (quartz)        * silver     -  2.5-3
 * onyx       -  7      (quartz)        * copper     -  2.5-3
 * moonstone  -  6      (orthoclase)    * amber      -  2-2.5
 */

/* the #engrave command */
int
doengrave(void)
{
    char *sp;         /* Place holder for space count of engr text */
    struct _doengrave_ctx *de;
    int retval;
    boolean initial_msg_given = FALSE;

    /* Can the adventurer engrave at all? */
    if (!u_can_engrave())
        return ECMD_FAIL;

    de = (struct _doengrave_ctx *) alloc(sizeof (struct _doengrave_ctx));
    doengrave_ctx_init(de);

    gm.multi = 0;              /* moves consumed */
    gn.nomovemsg = (char *) 0; /* occupation end message */

    /* One may write with finger, or weapon, or wand, or..., or...
     * Edited by GAN 10/20/86 so as not to change weapon wielded.
     */

    de->otmp = getobj("写字用", stylus_ok, GETOBJ_PROMPT);
    if (!de->otmp) {/* otmp == &hands_obj if fingers */
        de->ret = ECMD_CANCEL;
        goto doengr_exit;
    }

    if (de->otmp == &hands_obj) {
        Strcat(strcpy(de->fbuf, "你的"), body_part(FINGERTIP));
        de->writer = de->fbuf;
    } else {
        de->writer = yname(de->otmp);
    }

    /* There's no reason you should be able to write with a wand
     * while both your hands are tied up.
     */
    if (!freehand() && de->otmp != uwep && !de->otmp->owornmask) {
        You("没有空%s 来写!", body_part(HAND));
        goto doengr_exit;
    }

    if (de->jello) {
        You("用%s给%s挠痒.", mon_nam(u.ustuck), de->writer);
        Your("信息分散了...");
        goto doengr_exit;
    }
    if (!can_reach_floor(TRUE)) {
        if (de->otmp->oclass != WAND_CLASS) {
            cant_reach_floor(u.ux, u.uy, FALSE, TRUE, FALSE);
            goto doengr_exit;
        } else {
            You("挥动魔杖, 指向下方的%s.",
                surface(u.ux, u.uy));
            initial_msg_given = TRUE;
        }
    }
    if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
        if (!initial_msg_given)
            You("用%s向祭坛做手势.", de->writer);
        altar_wrath(u.ux, u.uy);
        goto doengr_exit;
    }
    if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
        if (de->otmp == &hands_obj) { /* using only finger */
            You("只能在%s上弄出一个小污点.",
                surface(u.ux, u.uy));
            goto doengr_exit;
        } else if (!levl[u.ux][u.uy].disturbed) {
            /* disturb the grave: summon a ghoul, same as sometimes
               happens when kicking; sets levl[ux][uy]->disturbed so
               that it'll only happen once */
            disturb_grave(u.ux, u.uy);
            goto doengr_exit;
        }
    }

    /* SPFX for items */
    if (!doengrave_sfx_item(de))
        goto doengr_exit;

    if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
        if (de->type == ENGRAVE || de->type == 0) {
            de->type = HEADSTONE;
        } else {
            /* ensures the "cannot wipe out" case */
            de->type = DUST;
            de->dengr = FALSE;
            de->teleengr = FALSE;
            de->buf[0] = '\0';
        }
    }

    /*
     * End of implement setup
     */

    /* Identify stylus */
    if (de->doknown) {
        learnwand(de->otmp);
        if (objects[de->otmp->otyp].oc_name_known)
            more_experienced(0, 10);
    }
    if (de->teleengr) {
        rloc_engr(de->oep);
        de->oep->eread = 0;
        de->oep->erevealed = 0;
        de->disprefresh = TRUE;
        de->oep = (struct engr *) 0;
    }
    if (de->dengr) {
        del_engr(de->oep);
        de->oep = (struct engr *) 0;
        de->disprefresh = TRUE;
    }
    /* Something has changed the engraving here */
    if (*de->buf) {
        struct engr *tmp_ep;

        make_engr_at(u.ux, u.uy, de->buf, de->ebuf, svm.moves, de->type);
        tmp_ep = engr_at(u.ux, u.uy);
        if (!Blind) {
            if (tmp_ep != 0) {
                pline_The("现在刻字显示为: \"%s\".", de->buf);
                tmp_ep->eread = 1;
                tmp_ep->erevealed = 1;
                de->disprefresh = TRUE;
            }
        }
        de->ptext = FALSE;
    }
    if (de->zapwand && (de->otmp->spe < 0)) {
        pline("%s %s化为了尘土.", The(xname(de->otmp)),
              Blind ? "" : "剧烈发光, 然后 ");
        if (!IS_GRAVE(levl[u.ux][u.uy].typ))
            You(
    "你用自己的灰尘在%s上写字是徒劳的.",
                de->frosted ? "霜" : "灰尘");
        useup(de->otmp);
        de->otmp = 0; /* wand is now gone */
        de->ptext = FALSE;
    }
    /* Early exit for some implements. */
    if (!de->ptext) {
        if (de->otmp && de->otmp->oclass == WAND_CLASS
            && !can_reach_floor(TRUE))
            cant_reach_floor(u.ux, u.uy, FALSE, TRUE, TRUE);
        de->ret = ECMD_TIME;
        goto doengr_exit;
    }
    /*
     * Special effects should have deleted the current engraving (if
     * possible) by now.
     */
    if (de->oep) {
        char c = 'n';

        /* Give player the choice to add to engraving. */
        if (de->type == HEADSTONE) {
            /* no choice, only append */
            c = 'y';
        } else if (de->type == de->oep->engr_type
                   && (!Blind || de->oep->engr_type == BURN
                       || de->oep->engr_type == ENGRAVE)) {
            c = yn_function("你想在当前刻字上添加内容吗?",
                            ynqchars, 'y', TRUE);
            if (c == 'q') {
                pline1(Never_mind);
                goto doengr_exit;
            }
        }

        if (c == 'n' || Blind) {
            if (de->oep->engr_type == DUST
                || de->oep->engr_type == ENGR_BLOOD
                || de->oep->engr_type == MARK) {
                if (!Blind) {
                    You("抹去了此处%s的信息.",
                        (de->oep->engr_type == DUST)
                            ? (de->frosted
                                ? "写在霜上"
                                : "写在灰尘上")
                            : (de->oep->engr_type == ENGR_BLOOD)
                                ? "用血写就"
                                : "写下的");
                    del_engr(de->oep);
                    de->oep = (struct engr *) 0;
                    de->disprefresh = TRUE;
                } else {
                    /* defer deletion until after we *know* we're engraving */
                    de->eow = TRUE;
                }
            } else if (de->type == DUST || de->type == MARK
                       || de->type == ENGR_BLOOD) {
                You("无法抹去%s此处%s的信息.",
                    (de->oep->engr_type == BURN)
                        ? (de->frosted ? "融化在" : "烧在")
                        : "刻在",
                    surface(u.ux, u.uy));
                de->ret = ECMD_TIME;
                goto doengr_exit;
            } else if (de->type != de->oep->engr_type || c == 'n') {
                if (!Blind || can_reach_floor(TRUE))
                    You("将覆盖当前的信息.");
                de->eow = TRUE;
            }
        } else if (de->oep
                   && Strlen(de->oep->engr_txt[actual_text]) >= BUFSZ - 1) {
            There("没有额外的空间来添加其他内容了.");
            de->ret = ECMD_TIME;
            goto doengr_exit;
        }
    }

    de->eloc = surface(u.ux, u.uy);
    de->adding = (de->oep && !de->eow);
    doengrave_ctx_verb(de);

    /* Tell adventurer what is going on */
    if (de->otmp != &hands_obj)
        You("用%s%s%s%s.",
            /* since doname() yields "N items" when quantity is more than
               one, match that by using "1 of" rather than "one of" when
               informing the player that the stack will be split */
            (de->type == ENGRAVE && de->otmp->quan > 1L) ? "1个 " : "",
            doname(de->otmp), de->everb, de->eloc);
    else
        You("用你的%s%s%s.",
            body_part(FINGERTIP), de->everb, de->eloc);

    /* Prompt for engraving! */
    Sprintf(de->qbuf, "你想在这里%s%s什么?", de->everb, de->eloc);
    getlin(de->qbuf, de->ebuf);
    /* convert tabs to spaces and condense consecutive spaces to one */
    mungspaces(de->ebuf);

    /* Count the actual # of chars engraved not including spaces */
    de->len = strlen(de->ebuf);
    for (sp = de->ebuf; *sp; sp++)
        if (*sp == ' ')
            de->len -= 1;

    if (de->len == 0 || strchr(de->ebuf, '\033')) {
        if (de->zapwand) {
            if (!Blind)
                pline("%s, 然后%s.", Tobjnam(de->otmp, "发光"),
                      otense(de->otmp, "消退"));
            de->ret = ECMD_TIME;
            goto doengr_exit;
        } else {
            pline1(Never_mind);
            goto doengr_exit;
        }
    }

    /* A single `x' is the traditional signature of an illiterate person */
    if (de->len != 1 || (!strchr(de->ebuf, 'x') && !strchr(de->ebuf, 'X')))
        if (!u.uconduct.literate++)
            livelog_printf(LL_CONDUCT, "通过刻写\"%s\"变得识字",
                           de->ebuf);

    /* Mix up engraving if surface or state of mind is unsound.
       Note: this won't add or remove any spaces. */
    for (sp = de->ebuf; *sp; sp++) {
        if (*sp == ' ')
            continue;
        if (((de->type == DUST || de->type == ENGR_BLOOD) && !rn2(25))
            || (Blind && !rn2(11)) || (Confusion && !rn2(7))
            || (Stunned && !rn2(4)) || (Hallucination && !rn2(2)))
            *sp = ' ' + rnd(96 - 2); /* ASCII '!' thru '~'
                                        (excludes ' ' and DEL) */
    }

    /* Previous engraving is overwritten */
    if (de->eow) {
        del_engr(de->oep);
        de->oep = (struct engr *) 0;
        de->disprefresh = TRUE;
    }

    Strcpy(svc.context.engraving.text, de->ebuf);
    svc.context.engraving.nextc = svc.context.engraving.text;
    svc.context.engraving.stylus = de->otmp;
    svc.context.engraving.type = de->type;
    svc.context.engraving.pos.x = u.ux;
    svc.context.engraving.pos.y = u.uy;
    svc.context.engraving.actionct = 0;
    set_occupation(engrave, "刻写", 0);

    if (de->post_engr_text[0])
        pline("%s", de->post_engr_text);
    if (de->doblind && !resists_blnd(&gy.youmonst)) {
        You("被闪光致盲了!");
        make_blinded((long) rnd(50), FALSE);
        if (!Blind)
            Your1(vision_clears);
    }

    /* Engraving will always take at least one action via being run as an
       occupation, so do not count this setup as taking time. */
 doengr_exit:
    if (de->disprefresh)
        newsym(u.ux, u.uy);
    retval = de->ret;
    free(de);
    return retval;
}

/* occupation callback for engraving some text */
staticfn int
engrave(void)
{
    struct engr *oep;
    char buf[BUFSZ]; /* holds the post-this-action engr text, including
                      * anything already there */
    const char *finishverb; /* "You finish [foo]." */
    struct obj * stylus; /* shorthand for svc.context.engraving.stylus */
    boolean firsttime = (svc.context.engraving.actionct == 0);
    int rate = 10; /* # characters that can be engraved in this action */
    boolean truncate = FALSE;
    boolean neweng = (svc.context.engraving.actionct == 0);

    boolean carving = (svc.context.engraving.type == ENGRAVE
                       || svc.context.engraving.type == HEADSTONE);
    boolean dulling_wep, marker;
    char *endc; /* points at character 1 beyond the last character to engrave
                 * this action */
    int i, space_left;

    if (svc.context.engraving.pos.x != u.ux
        || svc.context.engraving.pos.y != u.uy) { /* teleported? */
        You("无法继续刻字.");
        return 0;
    }
    /* Stylus might have been taken out of inventory and destroyed somehow.
     * Not safe to dereference stylus until after this. */
    if (svc.context.engraving.stylus == &hands_obj) { /* bare finger */
        stylus = (struct obj *) 0;
    } else {
        for (stylus = gi.invent; stylus; stylus = stylus->nobj) {
            if (stylus == svc.context.engraving.stylus)
                break;
        }
        if (!stylus) {
            You("无法继续雕刻.");
            return 0;
        }
    }

    dulling_wep = (carving && stylus && stylus->oclass == WEAPON_CLASS
                   && (stylus->otyp != ATHAME || stylus->cursed));
    marker = (stylus && stylus->otyp == MAGIC_MARKER
              && svc.context.engraving.type == MARK);

    svc.context.engraving.actionct++;

    /* sanity checks */
    if (dulling_wep && !is_blade(stylus)) {
        impossible("carving with non-bladed weapon");
    } else if (svc.context.engraving.type == MARK && !marker) {
        impossible("making graffiti with non-marker stylus");
    }

    /* Step 1: Compute rate. */
    if (carving && stylus
        && (dulling_wep || stylus->oclass == RING_CLASS
            || stylus->oclass == GEM_CLASS)) {
        /* slow engraving methods */
        rate = 1;
    } else if (marker) {
        /* one charge / 2 letters */
        rate = min(rate, stylus->spe * 2);
    }

    /* Step 2: Compute last character that can be engraved this action. */
    i = rate;
    for (endc = svc.context.engraving.nextc; *endc && i > 0; endc++) {
        if (*endc != ' ') {
            i--;
        }
    }

    /* Step 3: affect stylus from engraving - it might wear out. */
    if (dulling_wep) {
        boolean splitstack = FALSE, dulled = FALSE;

        /* 'dulling_wep' guarantees that 'stylus' is a weapon which is
           not welded to the hero's hand(s) */
        if (stylus->quan > 1L) {
            if (firsttime)
                pline("其中一把%s变钝了.", yname(stylus));
            stylus = svc.context.engraving.stylus = splitobj(stylus, 1L);
            /* if stack is wielded or quivered, the split-off one isn't */
            stylus->owornmask = 0L;
            splitstack = TRUE;
        } else {
            /* normal case: stylus->quan==1 */
            if (firsttime)
                pline("%s变钝了.", Yname2(stylus));
        }
        /* Dull the weapon at a rate of -1 enchantment per 2 characters,
         * rounding down.
         * The number of characters obtainable given starting enchantment:
         * -2 => 3, -1 => 5, 0 => 7, +1 => 9, +2 => 11
         * Note: this does not allow a +0 anything (except an athame) to
         * engrave "Elbereth" all at once.
         * However, you can engrave "Elb", then "ere", then "th", by taking
         * advantage of the rounding down. */
        if (svc.context.engraving.actionct % 2 == 1) { /* 1st,3rd,... action */
            /* deduct a point on 1st, 3rd, 5th, ... turns, unless this is the
             * last character being engraved (a rather convoluted way to round
             * down), but always deduct a point on the 1st turn to prevent
             * zero-cost engravings.
             * Check for truncation *before* deducting a point - otherwise,
             * attempting to e.g. engrave 3 characters with a -2 weapon will
             * stop at the 1st. */
            if (stylus->spe <= -3) {
                if (firsttime) {
                    impossible("<= -3 weapon valid for engraving");
                }
                truncate = TRUE;
            } else if (*endc || svc.context.engraving.actionct == 1) {
                stylus->spe -= 1;
                dulled = TRUE;
            }
        }
        if (splitstack) {
            obj_extract_self(stylus);
            stylus = hold_another_object(stylus, "你掉落了一个%s!",
                                          doname(stylus), (char *) NULL);
            nhUse(stylus);
        } else if (dulled && stylus->known) {
            /* reflect change in stylus->spe; not needed for splitstack
               since hold_another_object() does this */
            prinv((char *) NULL, stylus, 1L);
            update_inventory();
        }
    } else if (marker) {
        int ink_cost = max(rate / 2, 1); /* Prevent infinite graffiti */

        if (stylus->spe < ink_cost) {
            impossible("overly dry marker valid for graffiti?");
            ink_cost = stylus->spe;
            truncate = TRUE;
        }
        stylus->spe -= ink_cost;
        update_inventory();
        if (stylus->spe == 0) {
            /* can't engrave any further; truncate the string */
            Your("标记笔干了.");
            truncate = TRUE;
        }
    }

    switch (svc.context.engraving.type) {
    default:
        finishverb = "你的奇怪刻字";
        break;
    case DUST:
        finishverb = is_ice(u.ux, u.uy) ? "霜中的字迹"
                     : "灰尘中的字迹";
        break;
    case HEADSTONE:
    case ENGRAVE:
        finishverb = "刻字";
        break;
    case BURN:
        finishverb = is_ice(u.ux, u.uy) ? "把信息融入冰面"
                     : "把信息烧入地面";
        break;
    case MARK:
        finishverb = "涂污地下城";
        break;
    case ENGR_BLOOD:
        finishverb = "潦草书写";
    }

    /* actions that happen at the end of every engraving action go here */

    buf[0] = '\0';
    oep = engr_at(u.ux, u.uy);
    if (oep) /* add to existing engraving */
        Strcpy(buf, oep->engr_txt[actual_text]);

    space_left = (int) (sizeof buf - strlen(buf) - 1U);
    if (endc - svc.context.engraving.nextc > space_left) {
        You("写不下了.");
        endc = svc.context.engraving.nextc + space_left;
        truncate = TRUE;
    }

    /* If the stylus did wear out mid-engraving, truncate the input so that we
     * can't go any further. */
    if (truncate && *endc != '\0') {
        *endc = '\0';
        You("只能写下\"%s\".", svc.context.engraving.text);
    } else {
        /* input was not truncated; stylus may still have worn out on the last
         * character, though */
        truncate = FALSE;
    }

    (void) strncat(buf, svc.context.engraving.nextc,
                   min(space_left, endc - svc.context.engraving.nextc));
    make_engr_at(u.ux, u.uy, buf, NULL, svm.moves - gm.multi,
                 svc.context.engraving.type);
    oep = engr_at(u.ux, u.uy);
    if (oep) {
        oep->eread = 1;
        oep->erevealed = 1;
    }

    if (*endc) {
        svc.context.engraving.nextc = endc;
        if (neweng) {
            newsym(svc.context.engraving.pos.x, svc.context.engraving.pos.y);
        }
        return 1; /* not yet finished this turn */
    } else { /* finished engraving */
        /* actions that happen after the engraving is finished go here */

        if (truncate) {
            /* Now that "You are only able to write 'foo'" also prints at the
             * end of engraving, this might be redundant. */
            You("不能再写更多了.");
        } else if (!firsttime) {
            /* only print this if engraving took multiple actions */
            You("完成了%s.", finishverb);
        }
        svc.context.engraving.text[0] = '\0';
        svc.context.engraving.nextc = (char *) 0;
        svc.context.engraving.stylus = (struct obj *) 0;
    }
    if (neweng)
        newsym(svc.context.engraving.pos.x, svc.context.engraving.pos.y);
    return 0;
}

/* while loading bones, clean up text which might accidentally
   or maliciously disrupt player's terminal when displayed */
void
sanitize_engravings(void)
{
    struct engr *ep;

    for (ep = head_engr; ep; ep = ep->nxt_engr) {
        sanitize_name(ep->engr_txt[actual_text]);
    }
}

/* mark all engravings as not-discovered/not-read when saving bones */
void
forget_engravings(void)
{
    struct engr *ep;

    for (ep = head_engr; ep; ep = ep->nxt_engr) {
        ep->erevealed = ep->eread = 0;

        /* Note: engr_txt[actual_text], engr_txt[rememberd_text], and
         * engr_txt[pristine_text] retain their original text rather
         * than get updated to reflect each engraving's current text.
         * Does it matter? */
    }
}

void
engraving_sanity_check(void)
{
    struct engr *ep;
    int levtyp;

    if (head_engr && (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz))) {
        impossible("engraving sanity: on plane of air/water");
        return;
    }

    for (ep = head_engr; ep; ep = ep->nxt_engr) {
        coordxy x = ep->engr_x, y = ep->engr_y;

        if (!isok(x, y)) {
            impossible("engraving sanity: !isok <%i,%i>", x, y);
            continue;
        }
        levtyp = SURFACE_AT(x, y);
        if (is_pool_or_lava(x, y) || IS_AIR(levtyp) || !ACCESSIBLE(levtyp)) {
            impossible("engraving sanity: illegal surface (%d: \"%s\")",
                       levtyp, surface(x, y));
            continue;
        }
    }
}

void
save_engravings(NHFILE *nhfp)
{
    struct engr *ep, *ep2;
    unsigned no_more_engr = 0, engr_alloc;
    unsigned szeach;

    for (ep = head_engr; ep; ep = ep2) {
        ep2 = ep->nxt_engr;
        if (ep->engr_alloc
            && ep->engr_txt[actual_text][0] && update_file(nhfp)) {
            engr_alloc = (unsigned) ep->engr_alloc;
            szeach = ep->engr_szeach;
            Sfo_unsigned(nhfp, &engr_alloc, "engraving-engr_alloc");
            Sfo_engr(nhfp, ep, "engraving");
            ep->engr_txt[actual_text] = engr_text_space(ep);
            ep->engr_txt[remembered_text] = ep->engr_txt[actual_text] + szeach;
            ep->engr_txt[pristine_text] = ep->engr_txt[remembered_text] + szeach;
            Sfo_char(nhfp, ep->engr_txt[actual_text], "engraving-actual_text", szeach);
            Sfo_char(nhfp, ep->engr_txt[remembered_text], "engraving-remembered_text", szeach);
            Sfo_char(nhfp, ep->engr_txt[pristine_text], "engraving-pristine_text", szeach);
        }
        if (release_data(nhfp))
            dealloc_engr(ep);
    }
    if (update_file(nhfp)) {
        Sfo_unsigned(nhfp, &no_more_engr, "engraving-engr_alloc");
    }
    if (release_data(nhfp))
        head_engr = 0;
}
#endif /* !SFCTOOL */

void
rest_engravings(NHFILE *nhfp)
{
    struct engr *ep;
    unsigned lth = 0;
    unsigned szeach;

    head_engr = 0;
    while (1) {
        Sfi_unsigned(nhfp, &lth, "engraving-engr_alloc");
        if (lth == 0)
            return;
        ep = newengr(lth);
        Sfi_engr(nhfp, ep, "engraving");
        szeach = ep->engr_szeach;
        ep->nxt_engr = head_engr;
        head_engr = ep;
        ep->engr_txt[actual_text] = engr_text_space(ep); /* Andreas Bormann */
        ep->engr_txt[remembered_text] = ep->engr_txt[actual_text] + szeach;
        ep->engr_txt[pristine_text] = ep->engr_txt[remembered_text] + szeach;
        Sfi_char(nhfp, ep->engr_txt[actual_text],
                 "engraving-actual_text", (int) szeach);
        Sfi_char(nhfp, ep->engr_txt[remembered_text],
                 "engraving-remembered_text", (int) szeach);
        Sfi_char(nhfp, ep->engr_txt[pristine_text],
                 "engraving-pristine_text", (int) szeach);

        while (ep->engr_txt[actual_text][0] == ' ')
            ep->engr_txt[actual_text]++;
        while (ep->engr_txt[remembered_text][0] == ' ')
            ep->engr_txt[remembered_text]++;
        /* mark as finished for bones levels -- no problem for
         * normal levels as the player must have finished engraving
         * to be able to move again */
        ep->engr_time = svm.moves;
    }
}

#ifndef SFCTOOL
DISABLE_WARNING_FORMAT_NONLITERAL

/* to support '#stats' wizard-mode command */
void
engr_stats(
    const char *hdrfmt,
    char *hdrbuf,
    long *count,
    long *size)
{
    struct engr *ep;

    Sprintf(hdrbuf, hdrfmt, (long) sizeof (struct engr));
    *count = *size = 0L;
    for (ep = head_engr; ep; ep = ep->nxt_engr) {
        ++*count;
        *size += (long) sizeof *ep + (long) ep->engr_alloc;
    }
}

RESTORE_WARNING_FORMAT_NONLITERAL

void
del_engr(struct engr *ep)
{
    if (ep == head_engr) {
        head_engr = ep->nxt_engr;
    } else {
        struct engr *ept;

        for (ept = head_engr; ept; ept = ept->nxt_engr)
            if (ept->nxt_engr == ep) {
                ept->nxt_engr = ep->nxt_engr;
                break;
            }
        if (!ept) {
            impossible("Error in del_engr?");
            return;
        }
    }
    dealloc_engr(ep);
}

/* randomly relocate an engraving */
void
rloc_engr(struct engr *ep)
{
    int tx, ty, tryct = 200;

    do {
        if (--tryct < 0)
            return;
        tx = rn1(COLNO - 3, 2);
        ty = rn2(ROWNO);
    } while (engr_at(tx, ty) || !goodpos(tx, ty, (struct monst *) 0, 0));

    ep->engr_x = tx;
    ep->engr_y = ty;
    newsym(tx, ty);  /* caller took care of the old location */
}

/* Create a headstone at the given location.
 * The caller is responsible for newsym(x, y).
 */
void
make_grave(coordxy x, coordxy y, const char *str)
{
    char buf[BUFSZ];

    /* Can we put a grave here? */
    if ((levl[x][y].typ != ROOM && levl[x][y].typ != GRAVE) || t_at(x, y))
        return;
    /* Make the grave */
    if (!set_levltyp(x, y, GRAVE))
        return;
    /* Engrave the headstone */
    del_engr_at(x, y);
    if (!str)
        str = get_rnd_text(EPITAPHFILE, buf, rn2, MD_PAD_RUMORS);
    make_engr_at(x, y, str, NULL, 0L, HEADSTONE);
    return;
}

/* called when kicking or engraving on a grave's headstone */
void
disturb_grave(coordxy x, coordxy y)
{
    struct rm *lev = &levl[x][y];

    if (!IS_GRAVE(lev->typ)) {
        impossible("Disturbing grave that isn't a grave? (%d)", lev->typ);
    } else if (lev->disturbed) {
        impossible("Disturbing already disturbed grave?");
    } else {
        You("打扰了亡灵!");
        lev->disturbed = 1;
        (void) makemon(&mons[PM_GHOUL], x, y, NO_MM_FLAGS);
        exercise(A_WIS, FALSE);
    }
}

void
see_engraving(struct engr *ep)
{
    newsym(ep->engr_x, ep->engr_y);
}

/* like see_engravings() but overrides vision, but only for some types
   of engravings that can be felt  [this isn't actually used anywhere?] */
void
feel_engraving(struct engr *ep)
{
    if (engr_can_be_felt(ep)) {
        ep->eread = 1;
        ep->erevealed = 1;
        map_engraving(ep, 1);
        /* in case it's beneath something, redisplay the something */
        newsym(ep->engr_x, ep->engr_y);
    }
}

static const char blind_writing[][21] = {
    {0x44, 0x66, 0x6d, 0x69, 0x62, 0x65, 0x22, 0x45, 0x7b, 0x71,
     0x65, 0x6d, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
    {0x51, 0x67, 0x60, 0x7a, 0x7f, 0x21, 0x40, 0x71, 0x6b, 0x71,
     0x6f, 0x67, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x49, 0x6d, 0x73, 0x69, 0x62, 0x65, 0x22, 0x4c, 0x61, 0x7c,
     0x6d, 0x67, 0x24, 0x42, 0x7f, 0x69, 0x6c, 0x77, 0x67, 0x7e, 0x00},
    {0x4b, 0x6d, 0x6c, 0x66, 0x30, 0x4c, 0x6b, 0x68, 0x7c, 0x7f,
     0x6f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x51, 0x67, 0x70, 0x7a, 0x7f, 0x6f, 0x67, 0x68, 0x64, 0x71,
     0x21, 0x4f, 0x6b, 0x6d, 0x7e, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x4c, 0x63, 0x76, 0x61, 0x71, 0x21, 0x48, 0x6b, 0x7b, 0x75,
     0x67, 0x63, 0x24, 0x45, 0x65, 0x6b, 0x6b, 0x65, 0x00, 0x00, 0x00},
    {0x4c, 0x67, 0x68, 0x6b, 0x78, 0x68, 0x6d, 0x76, 0x7a, 0x75,
     0x21, 0x4f, 0x71, 0x7a, 0x75, 0x6f, 0x77, 0x00, 0x00, 0x00, 0x00},
    {0x44, 0x66, 0x6d, 0x7c, 0x78, 0x21, 0x50, 0x65, 0x66, 0x65,
     0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    {0x44, 0x66, 0x73, 0x69, 0x62, 0x65, 0x22, 0x56, 0x7d, 0x63,
     0x69, 0x76, 0x6b, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

staticfn const char *
blengr(void)
{
    return ROLL_FROM(blind_writing);
}
#endif /* !SFCTOOL */
/*engrave.c*/
