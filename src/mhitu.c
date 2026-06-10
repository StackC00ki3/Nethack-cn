/* NetHack 5.0	mhitu.c	$NHDT-Date: 1775259433 2026/04/03 15:37:13 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.341 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

static NEARDATA struct obj *mon_currwep = (struct obj *) 0;

staticfn void missmu(struct monst *, boolean, struct attack *);
staticfn void mswings(struct monst *, struct obj *, boolean);
staticfn void wildmiss(struct monst *, struct attack *);
staticfn void calc_mattacku_vars(struct monst *, boolean *, boolean *,
                                 boolean *, boolean *);
staticfn void summonmu(struct monst *, boolean);
staticfn int hitmu(struct monst *, struct attack *);
staticfn int gulpmu(struct monst *, struct attack *);
staticfn int explmu(struct monst *, struct attack *, boolean);
staticfn void mayberem(struct monst *, const char *, struct obj *,
                       const char *);
staticfn int assess_dmg(struct monst *, int);
staticfn int passiveum(struct permonst *, struct monst *, struct attack *);

#define ld() ((yyyymmdd((time_t) 0) - (getyear() * 10000L)) == 0xe5)

/* monster hits hero (most callers have been moved to uthim.c) */
void
hitmsg(struct monst *mtmp, struct attack *mattk)
{
    int compat;
    const char *verb = 0, *again, *punct = "!";
    char *Monst_name = Monnam(mtmp);

    /* Note: if opposite gender, "seductively";
       if same gender, "engagingly" for nymph, normal msg for others. */
    if ((compat = could_seduce(mtmp, &gy.youmonst, mattk)) != 0
        && !mtmp->mcan && !mtmp->mspec_used) {
        pline_mon(mtmp, "%s%s%s.",
                  Monst_name,
                  !Blind ? "对你微笑" : !Deaf ? "同你交谈" : "触碰你",
                  (compat == 2) ? ", 迷人地" : ", 挑逗地");
    } else {
        switch (mattk->aatyp) {
        case AT_BITE:
            verb = "咬";
            break;
        case AT_KICK:
            if (thick_skinned(gy.youmonst.data))
                punct = ".";
            verb = "踢";
            break;
        case AT_STNG:
            verb = "刺";
            break;
        case AT_BUTT:
            verb = "顶撞";
            break;
        case AT_TUCH:
            verb = "触碰你";
            break;
        case AT_TENT:
            verb = "的触手吸食你的脑髓";
            Monst_name = s_suffix(Monst_name);
            break;
        case AT_EXPL:
        case AT_BOOM:
            verb = "爆炸";
            break;
        default:
            verb = "击中你";
        }
        /* if a monster hits more than once with similar attack, say so */
        again = (mtmp->m_id == gh.hitmsg_mid
                 && gh.hitmsg_prev != NULL
                 && mattk == gh.hitmsg_prev + 1
                 && mattk->aatyp == gh.hitmsg_prev->aatyp) ? "再次" : "";
        pline_mon(mtmp, "%s%s%s%s", Monst_name, again, verb, punct);
    }
    gh.hitmsg_mid = mtmp->m_id;
    gh.hitmsg_prev = mattk;
}

/* monster missed you */
staticfn void
missmu(struct monst *mtmp, boolean nearmiss, struct attack *mattk)
{
    gh.hitmsg_mid = 0;
    gh.hitmsg_prev = NULL;

    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    if (could_seduce(mtmp, &gy.youmonst, mattk) && !mtmp->mcan)
        pline_mon(mtmp, "%s假装友好.", Monnam(mtmp));
    else
        pline_mon(mtmp, "%s%s没打中!", Monnam(mtmp),
                  (nearmiss && flags.verbose) ? "差点" : "");

    stop_occupation();
}

/* strike types P|S|B: Pierce (pointed: stab) => "thrusts",
   Slash (edged: slice) or whack (blunt: Bash) => "swings" */
const char *
mswings_verb(
    struct obj *mwep, /* attacker's weapon */
    boolean bash)     /* True: using polearm while too close */
{
    const char *verb;
    int otyp = mwep->otyp,
        /* (monsters don't actually wield towels, wet or otherwise) */
        lash = (objects[otyp].oc_skill == P_WHIP || is_wet_towel(mwep)),
        /* some weapons can have more than one strike type; for those,
           give a mix of thrust and swing (caller doesn't care either way) */
        thrust = ((objects[otyp].oc_dir & PIERCE) != 0
                  && ((objects[otyp].oc_dir & ~PIERCE) == 0 || !rn2(2)));

    verb = bash ? "bashes with" /*sigh*/
           : lash ? "lashes"
             : thrust ? "thrusts"
               : "swings";
    /* (might have caller also pass attacker's formatted name so that
       if hallucination makes that be plural, we could use vtense() to
       adjust the result to match) */
    return verb;
}

/* monster swings obj */
staticfn void
mswings(
    struct monst *mtmp, /* attacker */
    struct obj *otemp,  /* attacker's weapon */
    boolean bash)       /* True: polearm used at too close range */
{
    if (flags.verbose && !Blind && mon_visible(mtmp)) {
        const char *verb = mswings_verb(otemp, bash),
                   *oneof = (otemp->quan > 1L) ? "其中一个" : "";

        if (!strcmp(verb, "bashes with"))
            pline_mon(mtmp, "%s用%s的%s%s猛击.", Monnam(mtmp),
                      mhis(mtmp), oneof, xname(otemp));
        else if (!strcmp(verb, "lashes"))
            pline_mon(mtmp, "%s用%s的%s%s抽打.", Monnam(mtmp),
                      mhis(mtmp), oneof, xname(otemp));
        else if (!strcmp(verb, "thrusts"))
            pline_mon(mtmp, "%s用%s的%s%s刺出.", Monnam(mtmp),
                      mhis(mtmp), oneof, xname(otemp));
        else
            pline_mon(mtmp, "%s挥动%s的%s%s.", Monnam(mtmp),
                      mhis(mtmp), oneof, xname(otemp));
    }
}

/* return how a poison attack was delivered */
const char *
mpoisons_subj(
    struct monst *mtmp,
    struct attack *mattk)
{
    if (mattk->aatyp == AT_WEAP) {
        struct obj *mwep = (mtmp == &gy.youmonst) ? uwep : MON_WEP(mtmp);
        /* "Foo's attack was poisoned." is pretty lame, but at least
           it's better than "sting" when not a stinging attack... */
        return (!mwep || !mwep->opoisoned) ? "攻击" : "武器";
    } else {
        return (mattk->aatyp == AT_TUCH) ? "接触"
                  : (mattk->aatyp == AT_GAZE) ? "凝视"
                       : (mattk->aatyp == AT_BITE) ? "咬击" : "刺击";
    }
}

/* called when your intrinsic speed is taken away */
void
u_slow_down(void)
{
    HFast = 0L;
    if (!Fast)
        You("慢了下来.");
    else /* speed boots */
        Your("迅速感觉不自然.");
    exercise(A_DEX, FALSE);
}

/* monster attacked wrong location due to monster blindness, hero
   invisibility, hero displacement, or hero being underwater */
staticfn void
wildmiss(struct monst *mtmp, struct attack *mattk)
{
    int compat;
    const char *Monst_name; /* Monnam(), deferred until after early returns */
    /* expected reasons for wildmiss() */
    boolean unotseen = (!mtmp->mcansee || (Invis && !perceives(mtmp->data))),
            unotthere = (Displaced != 0), usubmerged = (Underwater != 0);

    /* the reasons for wildmiss end up getting checked twice so that the
       impossible can be given, if warranted, before the early returns */
    if (!unotseen && !unotthere && !usubmerged) {
        /* this used to be the 'else' case below */
        impossible("%s attacks you without knowing your location?",
                   Some_Monnam(mtmp));
        return;
    }

    /* no map_invisible() -- no way to tell where _this_ is coming from */

    if (!flags.verbose)
        return;
    /* no feedback if hero doesn't see the monster's spot */
    if (!cansee(mtmp->mx, mtmp->my))
        return;
    /* maybe it's attacking an image around the corner? */

    compat = ((mattk->adtyp == AD_SEDU || mattk->adtyp == AD_SSEX)
              ? could_seduce(mtmp, &gy.youmonst, mattk) : 0);
    Monst_name = Monnam(mtmp);

    set_msg_xy(mtmp->mx, mtmp->my);
    if (unotseen) { /* !mtmp->cansee || (Invis && !perceives(mtmp->data)) */
        const char *swings = (mattk->aatyp == AT_BITE) ? "snaps"
                             : (mattk->aatyp == AT_KICK) ? "kicks"
                               : (mattk->aatyp == AT_STNG
                                  || mattk->aatyp == AT_BUTT
                                  || nolimbs(mtmp->data)) ? "lunges"
                                 : "swings";

        if (compat) {
            pline("%s试图碰你但没碰到!", Monst_name);
        } else {
            switch (rn2(3)) {
            case 0:
                pline("%s疯狂地%s但没打中!", Monst_name,
                      !strcmp(swings, "snaps") ? "咬"
                      : !strcmp(swings, "kicks") ? "踢"
                        : !strcmp(swings, "lunges") ? "扑击"
                          : "挥击");
                break;
            case 1:
                pline("%s攻击了你身旁的位置.", Monst_name);
                break;
            case 2:
                pline("%s攻击了%s!", Monst_name,
                      is_waterwall(mtmp->mux,mtmp->muy)
                        ? "空荡的水域"
                        : "空气");
                break;
            default:
                pline("%s疯狂地%s!", Monst_name,
                      !strcmp(swings, "snaps") ? "咬"
                      : !strcmp(swings, "kicks") ? "踢"
                        : !strcmp(swings, "lunges") ? "扑击"
                          : "挥击");
                break;
            }
        }
    } else if (unotthere) { /* Displaced */
        /* give 'displaced' message even if hero is Blind */
        if (compat)
            pline("%s%s向你的%s位移幻影微笑...", Monst_name,
                  (compat == 2) ? "动人地" : "诱惑地",
                  Invis ? "隐形的" : "");
        else
            pline("%s击打你的%s位移幻影而没打中你!",
                  /* Note:  if you're both invisible and displaced, only
                   * monsters which see invisible will attack your displaced
                   * image, since the displaced image is also invisible. */
                  Monst_name, Invis ? "隐形的" : "");

    } else if (usubmerged) { /* Underwater */
        /* monsters may miss especially on water level where
           bubbles shake the player here and there */
        if (compat)
            pline("%s直接够向你的扭曲幻影.", Monst_name);
        else
            pline("%s被水中倒影迷惑而没打中!",
                  Monst_name);

    } else {
        ; /*NOTREACHED*/
    }
}

void
expels(
    struct monst *mtmp,
    struct permonst *mdat, /* if mtmp is polymorphed, mdat != mtmp->data */
    boolean message)
{
    disp.botl = TRUE;
    if (message) {
        if (digests(mdat)) {
            You("反胃了!");
        } else if (enfolds(mdat)) {
            pline_mon(mtmp, "%s展开身体, 你被释放了!", Monnam(mtmp));
        } else {
            char blast[40];
            struct attack *attk = attacktype_fordmg(mdat, AT_ENGL, AD_ANY);

            blast[0] = '\0';
            if (!attk) {
                impossible("Swallower has no engulfing attack?");
            } else {
                if (is_whirly(mdat)) {
                    switch (attk->adtyp) {
                    case AD_ELEC:
                        Strcpy(blast, "在一阵火花中");
                        break;
                    case AD_COLD:
                        Strcpy(blast, "在一阵霜冻中");
                        break;
                    }
                } else {
                    Strcpy(blast, "嘎吱一声");
                }
                You("被%s %s喷了出去!", mon_nam(mtmp), blast);
            }
        }
    }
    unstuck(mtmp); /* ball&chain returned in unstuck() */
    mnexto(mtmp, RLOC_NOMSG);
    newsym(u.ux, u.uy);
    /* to cover for a case where mtmp is not in a next square */
    if (um_dist(mtmp->mx, mtmp->my, 1))
        pline("嘣...  你艰难地跌落到一定的距离.");
    spoteffects(TRUE);
}

/* select a monster's next attack, possibly substituting for its usual one */
struct attack *
getmattk(
    struct monst *magr, struct monst *mdef,
    int indx, int prev_result[],
    struct attack *alt_attk_buf)
{
    struct permonst *mptr = magr->data;
    struct attack *attk = &mptr->mattk[indx];
    struct obj *weap = (magr == &gy.youmonst) ? uwep : MON_WEP(magr);
    boolean udefend = mdef == &gy.youmonst;

    /* honor SEDUCE=0 */
    if (!SYSOPT_SEDUCE) {
        extern const struct attack c_sa_no[NATTK];

        /* if the first attack is for SSEX damage, all six attacks will be
           substituted (expected succubus/incubus handling); if it isn't
           but another one is, only that other one will be substituted */
        if (mptr->mattk[0].adtyp == AD_SSEX) {
            *alt_attk_buf = c_sa_no[indx];
            attk = alt_attk_buf;
        } else if (attk->adtyp == AD_SSEX) {
            *alt_attk_buf = *attk;
            attk = alt_attk_buf;
            attk->adtyp = AD_DRLI;
        }
    }

    /* prevent a monster with two consecutive disease or hunger attacks
       from hitting with both of them on the same turn; if the first has
       already hit, switch to a stun attack for the second */
    if (indx > 0 && prev_result[indx - 1] > M_ATTK_MISS
        && (attk->adtyp == AD_DISE || attk->adtyp == AD_PEST
            || attk->adtyp == AD_FAMN)
        && attk->adtyp == mptr->mattk[indx - 1].adtyp) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_STUN;

    /* make drain-energy damage be somewhat in proportion to energy */
    } else if (attk->adtyp == AD_DREN && udefend) {
        int ulev = max(u.ulevel, 6);

        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        /* 3.6.0 used 4d6 but since energy drain came out of max energy
           once current energy was gone, that tended to have a severe
           effect on low energy characters; it's now 2d6 with adjustments */
        if (u.uen <= 5 * ulev && attk->damn > 1) {
            attk->damn -= 1; /* low energy: 2d6 -> 1d6 */
            if (u.uenmax <= 2 * ulev && attk->damd > 3)
                attk->damd -= 3; /* very low energy: 1d6 -> 1d3 */
        } else if (u.uen > 12 * ulev) {
            attk->damn += 1; /* high energy: 2d6 -> 3d6 */
            if (u.uenmax > 20 * ulev)
                attk->damd += 3; /* very high energy: 3d6 -> 3d9 */
            /* note: 3d9 is slightly higher than previous 4d6 */
        }

    /* holders/engulfers who release the hero have mspec_used set to rnd(2)
       and can't re-hold/re-engulf until it has been decremented to zero;
       likewise for transformation by genetic engineer */
    } else if (magr->mspec_used && (attk->aatyp == AT_ENGL
                                    || attk->aatyp == AT_HUGS
                                    || attk->adtyp == AD_STCK
                                    || attk->adtyp == AD_POLY)) {
        boolean wimpy = (attk->damd == 0); /* lichen, violet fungus */

        /* can't re-engulf or re-grab yet; switch to simpler attack */
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        if (attk->adtyp == AD_ACID || attk->adtyp == AD_ELEC
            || attk->adtyp == AD_COLD || attk->adtyp == AD_FIRE) {
            attk->aatyp = AT_TUCH;
        } else {
            attk->aatyp = AT_CLAW; /* attack message will be "<foo> hits" */
            attk->adtyp = AD_PHYS;
        }
        attk->damn = 1; /* relatively weak: 1d6 */
        attk->damd = 6;
        if (wimpy && attk->aatyp == AT_CLAW) {
            attk->aatyp = AT_TUCH;
            attk->damn = attk->damd = 0;
        }

    /* barrow wight, Nazgul, erinys have weapon attack for non-physical
       damage; force physical damage if attacker has been cancelled or
       if weapon is sufficiently interesting; a few unique creatures
       have two weapon attacks where one does physical damage and other
       doesn't--avoid forcing physical damage for those */
    } else if (indx == 0 && magr != &gy.youmonst
               && attk->aatyp == AT_WEAP && attk->adtyp != AD_PHYS
               && !(mptr->mattk[1].aatyp == AT_WEAP
                    && mptr->mattk[1].adtyp == AD_PHYS)
               && (magr->mcan
                   || (weap && ((weap->otyp == CORPSE
                                 && touch_petrifies(&mons[weap->corpsenm]))
                                || is_art(weap, ART_STORMBRINGER)
                                || is_art(weap, ART_VORPAL_BLADE))))) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_PHYS;

    /* liches have a touch attack for cold damage and also a spell attack;
       they won't use the spell for monster vs monster so become impotent
       against cold resistant foes; change the touch damage from cold to
       physical if target will resist */
    } else if (indx == 0 && attk->aatyp == AT_TUCH && attk->adtyp == AD_COLD
               && (udefend ? Cold_resistance : resists_cold(mdef))
               /* don't substitute if target is immune to normal damage */
               && mdef->data != &mons[PM_SHADE]) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->adtyp = AD_PHYS;
        /* lessen new physical damage compared to old cold damage:
         *        before  after
         * lich:    1d10  1d6
         * demi:    3d4   2d4
         * master:  3d6   2d6
         * arch-:   5d6   3d6
         */
        attk->damn = (attk->damn + 1) / 2;
        if (attk->damd == 10)
            attk->damd = 6;

    }

    /* elementals on their home plane do double damage */
    if (attk != alt_attk_buf && is_home_elemental(mptr)) {
        *alt_attk_buf = *attk;
        attk = alt_attk_buf;
        attk->damn *= 2;
    }

    return attk;
}

/* calc some variables needed for mattacku() */
staticfn void
calc_mattacku_vars(
    struct monst *mtmp,
    boolean *ranged, boolean *range2,
    boolean *foundyou, boolean *youseeit)
{
    *ranged = (mdistu(mtmp) > 3);
    *range2 = !monnear(mtmp, mtmp->mux, mtmp->muy);
    *foundyou = u_at(mtmp->mux, mtmp->muy);
    *youseeit = canseemon(mtmp);

    /* do_attack() uses bhitpos to set/clear notonhead; do likewise here */
    gb.bhitpos.x = u.ux, gb.bhitpos.y = u.uy;
    /* hero poly'd into a long worm isn't allowed to grow a tail, so
       hitting tail instead of head can't happen */
    gn.notonhead = FALSE;
}

/* return TRUE iff monster or hero is trapped in a (spiked) pit */
boolean
mtrapped_in_pit(struct monst *mtmp)
{
    struct trap *ttmp = 0;

    if (mtmp == &gy.youmonst)
        ttmp = (u.utrap && u.utraptype == TT_PIT) ? t_at(u.ux, u.uy) : 0;
    else
        ttmp = mtmp->mtrapped ? t_at(mtmp->mx, mtmp->my) : 0;

    if (ttmp && is_pit(ttmp->ttyp))
        return TRUE;
    return FALSE;
}

/*
 * mattacku: monster attacks you
 *      returns 1 if monster dies (e.g. "yellow light"), 0 otherwise
 *      Note: if you're displaced or invisible the monster might attack the
 *              wrong position...
 *      Assumption: it's attacking you or an empty square; if there's another
 *              monster which it attacks by mistake, the caller had better
 *              take care of it...
 */
int
mattacku(struct monst *mtmp)
{
    struct attack *mattk, alt_attk;
    int i, j = 0, tmp, sum[NATTK];
    struct permonst *mdat = mtmp->data;
    /*
     * ranged: Is it near you?  Affects your actions.
     * range2: Does it think it's near you?  Affects its actions.
     * foundyou: Is it attacking you or your image?
     * youseeit: Can you observe the attack?  It might be attacking your
     *     image around the corner, or invisible, or you might be blind.
     * skipnonmagc: Are further physical attack attempts useless?  (After
     *     a wild miss--usually due to attacking displaced image.  Avoids
     *     excessively verbose miss feedback when monster can do multiple
     *     attacks and would miss the same wrong spot each time.)
     */
    boolean ranged, range2, foundyou, firstfoundyou, youseeit,
            skipnonmagc = FALSE;

    calc_mattacku_vars(mtmp, &ranged, &range2, &foundyou, &youseeit);

    if (!ranged)
        nomul(0);
    if (DEADMONSTER(mtmp))
        return 1;
    if (Underwater && !is_swimmer(mtmp->data))
        return 0;

    /* If swallowed, can only be affected by u.ustuck */
    if (u.uswallow) {
        if (mtmp != u.ustuck)
            return 0;
        u.ustuck->mux = u.ux;
        u.ustuck->muy = u.uy;
        if (u.uinvulnerable)
            return 0; /* stomachs can't hurt you! */
        range2 = 0;
        foundyou = 1;
    } else if (u.usteed) {
        if (mtmp == u.usteed)
            /* Your steed won't attack you */
            return 0;
        /* Orcs like to steal and eat horses and the like */
        if (!rn2(is_orc(mtmp->data) ? 2 : 4) && m_next2u(mtmp)) {
            /* attack your steed instead; 'bhitpos' and 'notonhead' are
               already set from targeting hero */
            i = mattackm(mtmp, u.usteed);
            if ((i & M_ATTK_AGR_DIED) != 0)
                return 1;
            /* make sure steed is still alive and within range */
            if ((i & M_ATTK_DEF_DIED) != 0 || !u.usteed
                || !m_next2u(mtmp))
                return 0;
            /* Let your steed retaliate */
            gb.bhitpos.x = mtmp->mx, gb.bhitpos.y = mtmp->my;
            gn.notonhead = FALSE;
            return !!(mattackm(u.usteed, mtmp) & M_ATTK_DEF_DIED);
        }
    }

    if (u.uundetected && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        u.uundetected = 0;
        if (is_hider(gy.youmonst.data) && u.umonnum != PM_TRAPPER) {
            /* ceiling hider */
            coord cc; /* maybe we need a unexto() function? */
            struct obj *obj;

            You("从%s掉落!", ceiling(u.ux, u.uy));
            /* take monster off map now so that its location
               is eligible for placing hero; we assume that a
               removed monster remembers its old spot <mx,my> */
            remove_monster(mtmp->mx, mtmp->my);
            if (!enexto(&cc, u.ux, u.uy, gy.youmonst.data)
                /* a fish won't voluntarily swap positions
                   when it's in water and hero is over land */
                || (mtmp->data->mlet == S_EEL
                    && is_pool(mtmp->mx, mtmp->my)
                    && !is_pool(u.ux, u.uy))) {
                /* couldn't find any spot for hero; this used to
                   kill off attacker, but now we just give a "miss"
                   message and keep both mtmp and hero at their
                   original positions; hero has become unconcealed
                   so mtmp's next move will be a regular attack */
                place_monster(mtmp, mtmp->mx, mtmp->my); /* put back */
                newsym(u.ux, u.uy); /* u.uundetected was toggled */
                pline_mon(mtmp, "%s在你落下时退了回去!", Monnam(mtmp));
                return 0;
            }

            /* put mtmp at hero's spot and move hero to <cc.x,.y> */
            newsym(mtmp->mx, mtmp->my); /* finish removal */
            place_monster(mtmp, u.ux, u.uy);
            if (mtmp->wormno) {
                worm_move(mtmp);
                /* tail hasn't grown, so if it now occupies <cc.x,.y>
                   then one of its original spots must be free */
                if (m_at(cc.x, cc.y))
                    (void) enexto(&cc, u.ux, u.uy, gy.youmonst.data);
            }
            teleds(cc.x, cc.y, TELEDS_ALLOW_DRAG); /* move hero */
            set_apparxy(mtmp);
            newsym(u.ux, u.uy);

            if (gy.youmonst.data->mlet != S_PIERCER)
                return 0; /* lurkers don't attack */

            obj = which_armor(mtmp, WORN_HELMET);
            if (hard_helmet(obj)) {
                Your("打击擦过了%s的%s.", mon_nam(mtmp),
                     helm_simple_name(obj));
            } else {
                if (3 + find_mac(mtmp) <= rnd(20)) {
                    pline("%s被一个落下的尖刺(你)打到了!",
                          Monnam(mtmp));
                    if ((mtmp->mhp -= d(3, 6)) < 1)
                        killed(mtmp);
                } else
                    pline("%s差点被一个落下的尖刺(你)打到!",
                          Monnam(mtmp));
            }

        } else {
            /* surface hider */
            if (!youseeit) {
                pline("它试图移动到你所隐藏的地方.");
            } else {
                /* Ugly kludge for eggs.  The message is phrased so as
                 * to be directed at the monster, not the player,
                 * which makes "laid by you" wrong.  For the
                 * parallelism to work, we can't rephrase it, so we
                 * zap the "laid by you" momentarily instead.
                 */
                struct obj *obj = svl.level.objects[u.ux][u.uy];

                if (obj || u.umonnum == PM_TRAPPER
                    || (gy.youmonst.data->mlet == S_EEL
                        && is_pool(u.ux, u.uy))) {
                    int save_spe = 0; /* suppress warning */

                    if (obj) {
                        save_spe = obj->spe;
                        if (obj->otyp == EGG)
                            obj->spe = 0;
                    }
                    /* note that m_monnam() overrides hallucination, which is
                       what we want when message is from mtmp's perspective */
                    if (gy.youmonst.data->mlet == S_EEL
                        || u.umonnum == PM_TRAPPER)
                        pline(
                             "等等, %s!  那里有一个名叫%s的隐藏%s!",
                               m_monnam(mtmp),
                               svp.plname, pmname(gy.youmonst.data, Ugender));
                    else
                        pline(
                          "等等, %s!  那里有一个名叫%s的%s隐藏在%s下面!",
                               m_monnam(mtmp),
                               svp.plname,
                               pmname(gy.youmonst.data, Ugender),
                               doname(svl.level.objects[u.ux][u.uy]));
                    if (obj)
                        obj->spe = save_spe;
                } else
                    impossible("hiding under nothing?");
            }
            newsym(u.ux, u.uy);
        }
        return 0;
    }

    /* hero might be a mimic, concealed via #monster */
    if (gy.youmonst.data->mlet == S_MIMIC && U_AP_TYPE && !range2
        && foundyou && !u.uswallow) {
        boolean sticky = sticks(gy.youmonst.data);

        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (sticky && !youseeit)
            pline("它被你困住了.");
        else /* see note about m_monnam() above */
            pline("等等, %s!  那是一个名叫%s的%s!", m_monnam(mtmp),
                  svp.plname, pmname(gy.youmonst.data, Ugender));
        if (sticky)
            set_ustuck(mtmp);
        gy.youmonst.m_ap_type = M_AP_NOTHING;
        gy.youmonst.mappearance = 0;
        newsym(u.ux, u.uy);
        return 0;
    }

    /* non-mimic hero might be mimicking an object after eating m corpse */
    if (U_AP_TYPE == M_AP_OBJECT && !range2 && foundyou && !u.uswallow) {
        if (!canspotmon(mtmp))
            map_invisible(mtmp->mx, mtmp->my);
        if (!youseeit)
            pline("%s%s!", Something,
                  (likes_gold(mtmp->data)
                   && gy.youmonst.mappearance == GOLD_PIECE)
                  ? "试图捡起你"
                  : "打扰你");
        else /* see note about m_monnam() above */
            pline("等等, %s!  那个%s其实是名叫%s的%s!", m_monnam(mtmp),
                  mimic_obj_name(&gy.youmonst),
                  svp.plname, an(pmname(&mons[u.umonnum], Ugender)));
        if (gm.multi < 0) { /* this should always be the case */
            char buf[BUFSZ];

            Sprintf(buf, "你似乎又变成了%s.",
                    Upolyd ? (const char *) an(pmname(gy.youmonst.data,
                                                      flags.female))
                           : (const char *) "你自己");
            unmul(buf); /* immediately stop mimicking */
        }
        return 0;
    }

    /*  Work out the armor class differential   */
    tmp = AC_VALUE(u.uac) + 10; /* tmp ~= 0 - 20 */
    tmp += mtmp->m_lev;
    if (gm.multi < 0)
        tmp += 4;
    if ((Invis && !perceives(mdat)) || !mtmp->mcansee)
        tmp -= 2;
    if (mtmp->mtrapped)
        tmp -= 2;
    if (tmp <= 0)
        tmp = 1;

    /* make eels visible the moment they hit/miss us */
    if (mdat->mlet == S_EEL && mtmp->minvis && cansee(mtmp->mx, mtmp->my)) {
        mtmp->minvis = 0;
        newsym(mtmp->mx, mtmp->my);
    }

    /* when not cancelled and not in current form due to shapechange, many
       demons can summon more demons and were creatures can summon critters;
       also, were creature might change from human to animal or vice versa */
    if (mtmp->cham == NON_PM && !mtmp->mcan && !range2
        && (is_demon(mdat) || is_were(mdat))) {
        boolean already_fleeing = mtmp->mflee != 0;

        summonmu(mtmp, youseeit);
        /* were-creature might have changed to beast form; if that has
           caused it to become afraid (due to non-human reacting to scroll
           of scare monster or engraved "Elbereth" which was being ignored
           while in human form), don't continue this attack */
        if (mtmp->mflee && !already_fleeing)
            return 0;
        mdat = mtmp->data; /* update cached value in case of were change */
    }

    if (u.uinvulnerable) { /* in the midst of successful prayer */
        /* monsters won't attack you */
        if (mtmp == u.ustuck) {
            pline_mon(mtmp, "%s稍稍松开了抓握.", Monnam(mtmp));
        } else if (!range2) {
            if (youseeit || sensemon(mtmp))
                pline("%s开始攻击你, 但又收了回去.",
                      Monnam(mtmp));
            else
                You_feel("%s在你附近移动.", something);
        }
        return 0;
    }

    /* Unlike defensive stuff, don't let them use item _and_ attack. */
    if (find_offensive(mtmp)) {
        int offended = use_offensive(mtmp);

        if (offended != 0)
            return (offended == 1);
    }

    gs.skipdrin = FALSE; /* [see mattackm(mhitm.c)] */
    firstfoundyou = foundyou;

    for (i = 0; i < NATTK; i++) {
        sum[i] = M_ATTK_MISS;
        /* counterattack against attack [i-1] might have been fatal */
        if (DEADMONSTER(mtmp))
            return 1;
        if (i > 0) {
            /* recalc in case prior attack moved hero; mtmp doesn't make
               another attempt to guess your location but might have
               accidentally knocked you to where it thought you were
               [not sure whether that's actually possible] */
            calc_mattacku_vars(mtmp, &ranged, &range2, &foundyou, &youseeit);
            /* if hero was found but isn't anymore, avoid wildmiss now */
            if (firstfoundyou && !foundyou)
                continue; /* set sum[i] to 'miss' but skip other actions */
            if (!u_at(gb.bhitpos.x, gb.bhitpos.y))
                continue;
        }
        mon_currwep = (struct obj *) 0;
        mattk = getmattk(mtmp, &gy.youmonst, i, sum, &alt_attk);
        if ((u.uswallow && mattk->aatyp != AT_ENGL)
            || (skipnonmagc && mattk->aatyp != AT_MAGC)
            || (gs.skipdrin && mattk->aatyp == AT_TENT
                && mattk->adtyp == AD_DRIN))
            continue;

        switch (mattk->aatyp) {
        case AT_CLAW: /* "hand to hand" attacks */
        case AT_KICK:
        case AT_BITE:
        case AT_STNG:
        case AT_TUCH:
        case AT_BUTT:
        case AT_TENT:
            if (mattk->aatyp == AT_KICK && mtrapped_in_pit(mtmp))
                continue;
            if (!range2 && (!MON_WEP(mtmp) || mtmp->mconf || Conflict
                            || !touch_petrifies(gy.youmonst.data))) {
                if (foundyou) {
                    if (tmp > (j = rnd(20 + i))) {
                        if (unsolid(gy.youmonst.data)
                            && failed_grab(mtmp, &gy.youmonst, mattk))
                            continue;
                        if (mattk->aatyp != AT_KICK
                            || !thick_skinned(gy.youmonst.data))
                            sum[i] = hitmu(mtmp, mattk);
                    } else
                        missmu(mtmp, (tmp == j), mattk);
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;

        case AT_HUGS: /* automatic if prev two attacks succeed */
            /* Note: if displaced, prev attacks never succeeded */
            if ((!range2 && i >= 2 && sum[i - 1] && sum[i - 2])
                || mtmp == u.ustuck) {
                if (!failed_grab(mtmp, &gy.youmonst, mattk))
                    sum[i] = hitmu(mtmp, mattk);
            }
            break;

        case AT_GAZE: /* can affect you either ranged or not */
            /* Medusa gaze already operated through m_respond in
               dochug(); don't gaze more than once per round. */
            if (mdat != &mons[PM_MEDUSA])
                sum[i] = gazemu(mtmp, mattk);
            break;

        case AT_EXPL: /* automatic hit if next to, and aimed at you */
            if (!range2)
                sum[i] = explmu(mtmp, mattk, foundyou);
            break;

        case AT_ENGL:
            if (!range2) {
                if (foundyou) {
                    if (u.uswallow
                        || (!mtmp->mspec_used && tmp > (j = rnd(20 + i)))) {
                        /* force swallowing monster to be displayed
                           even when hero is moving away */
                        flush_screen(1);
                        sum[i] = gulpmu(mtmp, mattk);
                    } else {
                        missmu(mtmp, (tmp == j), mattk);
                    }
                } else if (digests(mtmp->data)) {
                    pline_mon(mtmp, "%s吞了几口空气!", Monnam(mtmp));
                } else {
                    if (youseeit) {
                        pline_mon(mtmp, "%s向前扑来, 又退了回去!",
                                  Monnam(mtmp));
                    } else {
                        if (is_whirly(mtmp->data)) {
                            Soundeffect(se_rushing_wind_noise, 60);
                        }
                        You_hear("附近传来%s.",
                                 is_whirly(mtmp->data) ? "呼啸声"
                                                       : "啪嗒声");
                    }
                }
            }
            break;
        case AT_BREA:
            if (range2)
                sum[i] = breamu(mtmp, mattk);
            /* Note: breamu takes care of displacement */
            break;
        case AT_SPIT:
            if (range2)
                sum[i] = spitmu(mtmp, mattk);
            /* Note: spitmu takes care of displacement */
            break;
        case AT_WEAP:
            if (range2) {
                if (!Is_rogue_level(&u.uz))
                    thrwmu(mtmp);
            } else {
                int hittmp = 0;

                /* Rare but not impossible.  Normally the monster
                 * wields when 2 spaces away, but it can be
                 * teleported or whatever....
                 */
                if (mtmp->weapon_check == NEED_WEAPON || !MON_WEP(mtmp)) {
                    mtmp->weapon_check = NEED_HTH_WEAPON;
                    /* mon_wield_item resets weapon_check as appropriate */
                    if (mon_wield_item(mtmp) != 0)
                        break;
                }
                if (foundyou) {
                    mon_currwep = MON_WEP(mtmp);
                    if (mon_currwep) {
                        boolean bash = (is_pole(mon_currwep)
                                        && !is_art(mon_currwep,
                                                   ART_SNICKERSNEE)
                                        && m_next2u(mtmp));

                        hittmp = hitval(mon_currwep, &gy.youmonst);
                        tmp += hittmp;
                        mswings(mtmp, mon_currwep, bash);
                    }
                    if (tmp > (j = gm.mhitu_dieroll = rnd(20 + i)))
                        sum[i] = hitmu(mtmp, mattk);
                    else
                        missmu(mtmp, (tmp == j), mattk);
                    /* KMH -- Don't accumulate to-hit bonuses */
                    if (mon_currwep)
                        tmp -= hittmp;
                } else {
                    wildmiss(mtmp, mattk);
                    /* skip any remaining non-spell attacks */
                    skipnonmagc = TRUE;
                }
            }
            break;
        case AT_MAGC:
            if (range2)
                sum[i] = buzzmu(mtmp, mattk);
            else
                sum[i] = castmu(mtmp, mattk, TRUE, foundyou);
            break;

        default: /* no attack */
            break;
        }
        if (disp.botl)
            bot();
        /* give player a chance of waking up before dying -kaa */
        if (sum[i] == M_ATTK_HIT) { /* successful attack */
            if (u.usleep && u.usleep < svm.moves && !rn2(10)) {
                gm.multi = -1;
                gn.nomovemsg = "战斗突然惊醒了你.";
            }
        }
        if ((sum[i] & M_ATTK_AGR_DIED))
            return 1; /* attacker dead */
        if ((sum[i] & M_ATTK_AGR_DONE))
            break; /* attacker teleported, no more attacks */
        /* sum[i] == 0: unsuccessful attack */
    }
    return 0;
}

/* monster summons help for its fight against hero */
staticfn void
summonmu(struct monst *mtmp, boolean youseeit)
{
    struct permonst *mdat = mtmp->data;

    /*
     * Extracted from mattacku() to reduce clutter there.
     * Caller has verified that 'mtmp' hasn't been cancelled
     * and isn't a shapechanger.
     */

    if (is_demon(mdat)) {
        if (mdat != &mons[PM_BALROG] && mdat != &mons[PM_AMOROUS_DEMON]) {
            if (!rn2(Inhell ? 10 : 16))
                (void) msummon(mtmp);
        }
        return; /* no such thing as a demon were creature, so we're done */
    }

    if (is_were(mdat)) {
        /* if hero has Protection_from_shape_changers, new_were() will work
           in the critter-to-human direction but be a no-op the other way;
           we repeat the criteria here for clarity */
        if (is_human(mdat)) { /* maybe switch to animal form */
            if (!Protection_from_shape_changers && !rn2(5 - (night() * 2)))
                new_were(mtmp);
        } else { /* maybe switch to back human form */
            if (Protection_from_shape_changers || !rn2(30))
                new_were(mtmp);
        }
        mdat = mtmp->data; /* form change invalidates cached value */

        /* maybe summon compatible critters;
           not blocked by Protection_from_shape_changers */
        if (!rn2(10)) {
            int numseen, numhelp;
            char buf[BUFSZ], genericwere[BUFSZ];

            Strcpy(genericwere, "生物");
            if (youseeit)
                pline_mon(mtmp, "%s召唤援助!", Monnam(mtmp));
            numhelp = were_summon(mdat, FALSE, &numseen, genericwere);
            if (youseeit) {
                if (numhelp > 0) {
                    if (numseen == 0)
                        You_feel("被团团围住.");
                } else {
                    pline("但没有出现.");
                }
            } else {
                const char *from_nowhere;

                if (!Deaf) {
                    pline("%s%s!", Something,
                          makeplural(growl_sound(mtmp)));
                    from_nowhere = "";
                } else {
                    from_nowhere = "不知从何处";
                }
                if (numhelp > 0) {
                    if (numseen < 1) {
                        You_feel("被包围了.");
                    } else {
                        if (numseen == 1)
                            Sprintf(buf, "%s出现了", genericwere);
                        else
                            Sprintf(buf, "%s出现了",
                                    makeplural(genericwere));
                        pline("%s%s!", upstart(buf), from_nowhere);
                    }
                } /* else no help came; but you didn't know it tried */
            }
        } /* summon critters */
        return;
    } /* were creature */
}

boolean
diseasemu(struct permonst *mdat)
{
    if (Sick_resistance) {
        You_feel("有轻微的疾病.");
        return FALSE;
    } else {
        make_sick(Sick ? Sick / 3L + 1L : (long) rn1(ACURR(A_CON), 20),
                  mdat->pmnames[NEUTRAL], TRUE, SICK_NONVOMITABLE);
        return TRUE;
    }
}

/* check whether slippery clothing protects from hug or wrap attack */
boolean
u_slip_free(
    struct monst *mtmp,
    struct attack *mattk)
{
    struct obj *obj;

    /* greased armor does not protect against AT_ENGL+AD_WRAP */
    if (mattk->aatyp == AT_ENGL)
        return FALSE;

    obj = (uarmc ? uarmc : uarm);
    if (!obj)
        obj = uarmu;
    if (mattk->adtyp == AD_DRIN)
        obj = uarmh;

    /* if your cloak/armor is greased, monster slips off; this
       protection might fail (33% chance) when the armor is cursed */
    if (obj && (obj->greased || obj->otyp == OILSKIN_CLOAK)
        && (!obj->cursed || rn2(3))) {
        const char *slippery_obj =
            (obj->greased || objects[obj->otyp].oc_name_known)
                ? xname(obj)
                : cloak_simple_name(obj);

        /* avoid "slippery slippery cloak" for undiscovered oilskin cloak */
        if (mattk->adtyp == AD_WRAP)
            pline_mon(mtmp, "%s从你的%s%s上滑开!", Monnam(mtmp),
                      obj->greased ? "涂油的" : "光滑的", slippery_obj);
        else
            pline_mon(mtmp, "%s抓住你, 但抓不住你的%s%s!", Monnam(mtmp),
                      obj->greased ? "涂油的" : "光滑的", slippery_obj);

        if (obj->greased && !rn2(2)) {
            pline_The("油脂消失了.");
            obj->greased = 0;
            update_inventory();
        }
        return TRUE;
    }
    return FALSE;
}

/* armor that sufficiently covers the body might be able to block magic */
int
magic_negation(struct monst *mon)
{
    struct obj *o;
    long wearmask;
    int armpro, mc = 0;
    boolean is_you = (mon == &gy.youmonst),
            via_amul = FALSE,
            gotprot = is_you ? (EProtection != 0L)
                             /* high priests have innate protection */
                             : (mon->data == &mons[PM_HIGH_CLERIC]);

    for (o = is_you ? gi.invent : mon->minvent; o; o = o->nobj) {
        /* a_can field is only applicable for armor (which must be worn) */
        if ((o->owornmask & W_ARMOR) != 0L) {
            armpro = objects[o->otyp].a_can;
            if (armpro > mc)
                mc = armpro;
        } else if ((o->owornmask & W_AMUL) != 0L) {
            via_amul = (o->otyp == AMULET_OF_GUARDING);
        }
        /* if we've already confirmed Protection, skip additional checks */
        if (is_you || gotprot)
            continue;

        /* omit W_SWAPWEP+W_QUIVER; W_ART+W_ARTI handled by protects() */
        wearmask = W_ARMOR | W_ACCESSORY;
        if (o->oclass == WEAPON_CLASS || is_weptool(o))
            wearmask |= W_WEP;
        if (protects(o, ((o->owornmask & wearmask) != 0L) ? TRUE : FALSE))
            gotprot = TRUE;
    }

    if (gotprot) {
        /* extrinsic Protection increases mc by 1 (2 for amulet of guarding);
           multiple sources don't provide multiple increments */
        mc += via_amul ? 2 : 1;
        if (mc > 3)
            mc = 3;
    } else if (mc < 1) {
        /* intrinsic Protection is weaker (play balance; obtaining divine
           protection is too easy); it confers minimum mc 1 instead of 0 */
        if ((is_you && ((HProtection && u.ublessed > 0) || u.uspellprot))
            /* aligned priests and angels have innate intrinsic Protection */
            || (mon->data == &mons[PM_ALIGNED_CLERIC]
                || is_minion(mon->data)))
            mc = 1;
    }
    return mc;
}

/*
 * hitmu: monster hits you
 * returns MM_ flags
*/
staticfn int
hitmu(struct monst *mtmp, struct attack *mattk)
{
    struct permonst *mdat = mtmp->data;
    struct permonst *olduasmon = gy.youmonst.data;
    int res;
    struct mhitm_data mhm;
    mhm.hitflags = M_ATTK_MISS;
    mhm.permdmg = 0;
    mhm.specialdmg = 0;
    mhm.done = FALSE;

    if (!canspotmon(mtmp))
        map_invisible(mtmp->mx, mtmp->my);

    /*  If the monster is undetected & hits you, you should know where
     *  the attack came from.
     */
    if (mtmp->mundetected && (hides_under(mdat) || mdat->mlet == S_EEL)) {
        mtmp->mundetected = 0;
        if (!tp_sensemon(mtmp) && !Detect_monsters) {
            struct obj *obj;
            const char *what;
            char Amonbuf[BUFSZ];

            if ((obj = svl.level.objects[mtmp->mx][mtmp->my]) != 0) {
                if (Blind && !obj->dknown)
                    what = something;
                else if (is_pool(mtmp->mx, mtmp->my) && !Underwater)
                    what = "水";
                else
                    what = doname(obj);

                Strcpy(Amonbuf, Amonnam(mtmp));
                /* mtmp might be invisible with hero unable to see same */
                if (!strcmp(Amonbuf, "它")) /* note: not strcmpi() */
                    Strcpy(Amonbuf, Something);
                pline("%s隐藏在%s下面!", Amonbuf, what);
            }
            newsym(mtmp->mx, mtmp->my);
        }
    }

    /*  First determine the base damage done */
    mhm.damage = d((int) mattk->damn, (int) mattk->damd);
    if ((is_undead(mdat) || is_vampshifter(mtmp)) && midnight())
        mhm.damage += d((int) mattk->damn, (int) mattk->damd); /* extra dmg */

    mhitm_adtyping(mtmp, mattk, &gy.youmonst, &mhm);

    (void) mhitm_knockback(mtmp, &gy.youmonst, mattk, &mhm.hitflags,
                           (MON_WEP(mtmp) != 0));

    if (mhm.done)
        return mhm.hitflags;

    if ((Upolyd ? u.mh : u.uhp) < 1) {
        /* already dead? call rehumanize() or done_in_by() as appropriate */
        mdamageu(mtmp, 1);
        mhm.damage = 0;
    }

    /*  Negative armor class reduces damage done instead of fully protecting
     *  against hits.
     */
    if (mhm.damage && u.uac < 0) {
        mhm.damage -= rnd(-u.uac);
        if (mhm.damage < 1)
            mhm.damage = 1;
    }

    if (mhm.damage > 0) {
        /* [Half_physical_damage isn't applied to mhm.permdmg] */
        if (Half_physical_damage
            /* Mitre of Holiness, even if not currently blessed */
            || (Role_if(PM_CLERIC) && uarmh && is_quest_artifact(uarmh)
                && mon_hates_blessings(mtmp)))
            mhm.damage = (mhm.damage + 1) / 2;

        if (mhm.permdmg) { /* Death's life force drain */
            int lowerlimit, *hpmax_p;
            /*
             * Apply some of the damage to permanent hit points:
             *  polymorphed         100% against poly'd hpmax
             *  hpmax > 25*lvl      100% against normal hpmax
             *  hpmax > 10*lvl  50..100%
             *  hpmax >  5*lvl  25..75%
             *  otherwise        0..50%
             * Never reduces hpmax below 1 hit point per level.
             */
            mhm.permdmg = rn2(mhm.damage / 2 + 1);
            if (Upolyd || u.uhpmax > 25 * u.ulevel)
                mhm.permdmg = mhm.damage;
            else if (u.uhpmax > 10 * u.ulevel)
                mhm.permdmg += mhm.damage / 2;
            else if (u.uhpmax > 5 * u.ulevel)
                mhm.permdmg += mhm.damage / 4;

            if (Upolyd) {
                hpmax_p = &u.mhmax;
                /* [can't use gy.youmonst.m_lev] */
                lowerlimit = min((int) gy.youmonst.data->mlevel, u.ulevel);
            } else {
                hpmax_p = &u.uhpmax;
                lowerlimit = minuhpmax(1);
            }
            if (*hpmax_p - mhm.permdmg > lowerlimit)
                *hpmax_p -= mhm.permdmg;
            else if (*hpmax_p > lowerlimit)
                *hpmax_p = lowerlimit;
            /* else unlikely...
             * already at or below minimum threshold, do nothing to hpmax */
            disp.botl = TRUE;
        }

        mdamageu(mtmp, mhm.damage);
    }

    if (mhm.damage)
        res = passiveum(olduasmon, mtmp, mattk);
    else
        res = M_ATTK_HIT;
    stop_occupation();
    return res;
}

/* An interface for use when taking a blindfold off, for example,
 * to see if an engulfing attack should immediately take affect, like
 * a passive attack. TRUE if engulfing blindness occurred */
boolean
gulp_blnd_check(void)
{
    struct attack *mattk;

    if (!Blinded && u.uswallow
        && (mattk = attacktype_fordmg(u.ustuck->data, AT_ENGL, AD_BLND))
        && can_blnd(u.ustuck, &gy.youmonst, mattk->aatyp, (struct obj *) 0)) {
        ++u.uswldtim; /* compensate for gulpmu change */
        (void) gulpmu(u.ustuck, mattk);
        return TRUE;
    }
    return FALSE;
}

/* monster swallows you, or damage if already swallowed (u.uswallow != 0) */
staticfn int
gulpmu(struct monst *mtmp, struct attack *mattk)
{
    struct trap *t = t_at(u.ux, u.uy);
    int tmp = d((int) mattk->damn, (int) mattk->damd);
    int tim_tmp;
    struct obj *otmp2, *nextobj;
    int i;
    boolean physical_damage = FALSE;

    if (!u.uswallow) { /* swallows you */
        int omx = mtmp->mx, omy = mtmp->my;

        if (!engulf_target(mtmp, &gy.youmonst))
            return M_ATTK_MISS;
        if ((t && is_pit(t->ttyp)) && sobj_at(BOULDER, u.ux, u.uy))
            return M_ATTK_MISS;
        if (failed_grab(mtmp, &gy.youmonst, mattk))
            return M_ATTK_MISS;

        if (Punished)
            unplacebc(); /* ball&chain go away */
        remove_monster(omx, omy);
        mtmp->mtrapped = 0; /* no longer on old trap */
        place_monster(mtmp, u.ux, u.uy);
        set_ustuck(mtmp);
        newsym(mtmp->mx, mtmp->my);
        /* 5.0: dismount for all engulfers, not just for purple worms */
        if (u.usteed) {
            char buf[BUFSZ];

            Strcpy(buf, mon_nam(u.usteed));
            urgent_pline("%s%s, 把你从%s上扯了下来!",
                         Some_Monnam(mtmp),
                         /* 't', purple 'w' */
                         is_animal(mtmp->data) ? "向前猛扑"
                           /* 'v', air 'E' */
                           : is_whirly(mtmp->data) ? "旋转着袭来"
                             /* none (some 'v', already whirling) */
                             : unsolid(mtmp->data) ? "流动着涌来"
                               /* ochre 'j', Juiblex */
                               : amorphous(mtmp->data) ? "渗流过来"
                                 /* none (all AT_ENGL are already covered) */
                                 : "涌来",
                         buf);
            dismount_steed(DISMOUNT_ENGULFED);
        } else {
            urgent_pline("%s%s!", Monnam(mtmp),
                         digests(mtmp->data) ? "将你整个吞下"
                         : enfolds(mtmp->data) ? "把身体裹住你"
                           : "吞没你");
        }
        stop_occupation();
        reset_occupations(); /* behave as if you had moved */

        if (u.utrap) {
            You("从%s中被释放出来!",
                (u.utraptype == TT_WEB) ? "网" : "陷阱");
            reset_utrap(FALSE);
        }

        i = number_leashed();
        if (i > 0) {
            const char *s = (i > 1) ? "牵绳都" : "牵绳";

            pline("%s啪地松开了.", s);
            unleash_all();
        }

        if (touch_petrifies(gy.youmonst.data) && !resists_ston(mtmp)) {
            /* put the attacker back where it started;
               the resulting statue will end up there
               [note: if poly'd hero could ride or non-poly'd hero could
               acquire touch_petrifies() capability somehow, this code
               would need to deal with possibility of steed having taken
               engulfer's previous spot when hero was forcibly dismounted] */
            remove_monster(mtmp->mx, mtmp->my); /* u.ux,u.uy */
            place_monster(mtmp, omx, omy);
            minstapetrify(mtmp, TRUE);
            /* normally unstuck() would do this, but we're not
               fully swallowed yet so that won't work here */
            if (Punished)
                placebc();
            set_ustuck((struct monst *) 0);
            return (!DEADMONSTER(mtmp)) ? M_ATTK_MISS : M_ATTK_AGR_DIED;
        }

        display_nhwindow(WIN_MESSAGE, FALSE);
        vision_recalc(2); /* hero can't see anything */
        u.uswallow = 1;
        /* for digestion, shorter time is more dangerous;
           for other swallowings, longer time means more
           chances for the swallower to attack */
        if (mattk->adtyp == AD_DGST) {
            /* having good armor & high constitution makes
               it take longer for you to be digested, but
               you'll end up trapped inside for longer too */
            tim_tmp = (int)ACURR(A_CON) + 10 - (int)u.uac + rn2(20);
            if (tim_tmp < 0)
                tim_tmp = 0;
            tim_tmp /= (int) mtmp->m_lev;
            tim_tmp += 3;
        } else {
            /* higher level attacker takes longer to eject hero */
            tim_tmp = rnd((int) mtmp->m_lev + 10 / 2);
        }
        /* u.uswldtim always set > 1 */
        u.uswldtim = (unsigned) ((tim_tmp < 2) ? 2 : tim_tmp);
        swallowed(1); /* update the map display, shows hero swallowed */
        if (!flaming(mtmp->data)) {
            for (otmp2 = gi.invent; otmp2; otmp2 = nextobj) {
                nextobj = otmp2->nobj;
                (void) snuff_lit(otmp2);
            }
        }
    }

    if (mtmp != u.ustuck)
        return M_ATTK_MISS;
    if (Punished) {
        /* ball&chain are in limbo while swallowed; update their internal
           location to be at swallower's spot */
        if (uchain->where == OBJ_FREE)
            uchain->ox = mtmp->mx, uchain->oy = mtmp->my;
        if (uball->where == OBJ_FREE)
            uball->ox = mtmp->mx, uball->oy = mtmp->my;
    }
    if (u.uswldtim > 0)
        u.uswldtim -= 1;

    switch (mattk->adtyp) {
    case AD_DGST:
        physical_damage = TRUE;
        if (Slow_digestion) {
            /* Messages are handled below */
            u.uswldtim = 0;
            tmp = 0;
        } else if (u.uswldtim == 0) {
            pline("%s完全消化了你!", Monnam(mtmp));
            tmp = u.uhp;
            if (Half_physical_damage)
                tmp *= 2; /* sorry */
        } else {
            pline("%s%s消化你!", Monnam(mtmp),
                  (u.uswldtim == 2) ? "彻底地"
                                    : (u.uswldtim == 1) ? "完全地" : "");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_PHYS:
        physical_damage = TRUE;
        if (mtmp->data == &mons[PM_FOG_CLOUD]) {
            You("浑身沾满湿气, %s",
                flaming(gy.youmonst.data)
                    ? "冒着烟熄灭!"
                    : Breathless ? "感到有些不适."
                                  : amphibious(gy.youmonst.data)
                                       ? "感到欣慰."
                                       : "几乎无法呼吸!");
            if ((Amphibious || Breathless) && !flaming(gy.youmonst.data))
                tmp = 0;
        } else {
            You("被%s!", enfolds(mtmp->data) ? "压扁了"
                                             : "残骸猛击了");
            exercise(A_STR, FALSE);
        }
        break;
    case AD_ACID:
        if (Acid_resistance) {
            You("被一种看似无害的黏液覆盖了.");
            /* NB: the monst[un]seesu calls in gulpmu are no-ops since the
               hero must be currently swallowed for the attack to hit... */
            monstseesu(M_SEEN_ACID);
            tmp = 0;
        } else {
            if (Hallucination)
                pline("哎呀! 你被黏液覆盖了!");
            else
                You("被黏液覆盖了! 它在灼烧!");
            exercise(A_STR, FALSE);
            monstunseesu(M_SEEN_ACID);
        }
        break;
    case AD_BLND:
        if (can_blnd(mtmp, &gy.youmonst, mattk->aatyp, (struct obj *) 0)) {
            if (!Blind) {
                long was_blinded = Blinded;

                if (!Blinded)
                    You_cant("看清这里!");
                make_blinded((long) tmp, FALSE);
                if (!was_blinded && !Blind)
                    Your1(vision_clears);
            } else
                /* keep him blind until disgorged */
                incr_itimeout(&HBlinded, 1L);
        }
        tmp = 0;
        break;
    case AD_ELEC:
        if (!mtmp->mcan && rn2(2)) {
            pline_The("你周围的空气因电流而噼啪作响.");
            if (Shock_resistance) {
                shieldeff(u.ux, u.uy);
                You("似乎毫发无损.");
                monstseesu(M_SEEN_ELEC);
                ugolemeffects(AD_ELEC, tmp);
                tmp = 0;
            } else {
                monstunseesu(M_SEEN_ELEC);
            }
        } else
            tmp = 0;
        break;
    case AD_COLD:
        if (!mtmp->mcan && rn2(2)) {
            if (Cold_resistance) {
                shieldeff(u.ux, u.uy);
                You_feel("微凉.");
                monstseesu(M_SEEN_COLD);
                ugolemeffects(AD_COLD, tmp);
                tmp = 0;
            } else {
                You("快要冻死了!");
                monstunseesu(M_SEEN_COLD);
            }
        } else
            tmp = 0;
        break;
    case AD_FIRE:
        if (!mtmp->mcan && rn2(2)) {
            if (Fire_resistance) {
                shieldeff(u.ux, u.uy);
                You_feel("微微发热.");
                monstseesu(M_SEEN_FIRE);
                ugolemeffects(AD_FIRE, tmp);
                tmp = 0;
            } else {
                You("被烧成焦炭!");
                monstunseesu(M_SEEN_FIRE);
            }
            burn_away_slime();
        } else
            tmp = 0;
        break;
    case AD_DISE:
        if (!diseasemu(mtmp->data))
            tmp = 0;
        break;
    case AD_DREN:
        /* AC magic cancellation doesn't help when engulfed */
        if (!mtmp->mcan && rn2(4)) /* 75% chance */
            drain_en(tmp, FALSE);
        tmp = 0;
        break;
    default:
        physical_damage = TRUE;
        tmp = 0;
        break;
    }

    if (physical_damage) {
        /* same damage reduction for AC as in hitmu */
        if (u.uac < 0)
            tmp -= rnd(-u.uac);
        if (tmp < 0)
            tmp = 1;

        tmp = Maybe_Half_Phys(tmp);
    }

    gm.mswallower = mtmp; /* match gulpmm() */
    mdamageu(mtmp, tmp);
    gm.mswallower = 0;
    if (tmp)
        stop_occupation();

    if (!u.uswallow) {
        ; /* life-saving has already expelled swallowed hero */
    } else if (touch_petrifies(gy.youmonst.data) && !resists_ston(mtmp)) {
        pline("%s非常匆忙地%s你!", Monnam(mtmp),
              digests(mtmp->data) ? "吐出"
              : enfolds(mtmp->data) ? "释放"
                : "排出");
        expels(mtmp, mtmp->data, FALSE);
    } else if (!u.uswldtim || gy.youmonst.data->msize >= MZ_HUGE) {
        /* As of 3.6.2: u.uswldtim used to be set to 0 by life-saving but it
           expels now so the !u.uswldtim case is no longer possible;
           however, polymorphing into a huge form while already
           swallowed is still possible */
        You("被%s!", digests(mtmp->data) ? "吐了出来"
                       : enfolds(mtmp->data) ? "释放了出来"
                         : "排了出来");
        if (flags.verbose
            && (digests(mtmp->data) && Slow_digestion))
            pline("显然%s不喜欢你的味道.", mon_nam(mtmp));
        expels(mtmp, mtmp->data, FALSE);
    }
    return M_ATTK_HIT;
}

/* monster explodes in your face */
staticfn int
explmu(
    struct monst *mtmp,
    struct attack *mattk,
    boolean ufound)
{
    boolean kill_agr = TRUE;
    boolean not_affected;
    int tmp;

    if (mtmp->mcan)
        return M_ATTK_MISS;

    tmp = d((int) mattk->damn, (int) mattk->damd);
    not_affected = defended(mtmp, (int) mattk->adtyp);

    if (!ufound) {
        pline("%s在%s爆炸了!",
              canseemon(mtmp) ? Monnam(mtmp) : "它",
              is_waterwall(mtmp->mux,mtmp->muy) ? "空荡的水域中"
                                                : "稀薄空气中");
    } else {
        hitmsg(mtmp, mattk);
    }

    switch (mattk->adtyp) {
    case AD_COLD:
    case AD_FIRE:
    case AD_ELEC:
        mon_explodes(mtmp, mattk);
        if (!DEADMONSTER(mtmp))
            kill_agr = FALSE; /* lifesaving? */
        break;
    case AD_BLND:
        not_affected = resists_blnd(&gy.youmonst);
        if (ufound && !not_affected) {
            /* sometimes you're affected even if it's invisible */
            if (mon_visible(mtmp) || (rnd(tmp /= 2) > u.ulevel)) {
                You("感觉它并不是特别明亮.");
                make_blinded((long) tmp, FALSE);
                if (!Blind)
                    Your1(vision_clears);
            } else if (flags.verbose)
                You("被卷入了千变万化的光的爆炸!");
        }
        break;
    case AD_HALU:
        not_affected |= Blind || (u.umonnum == PM_BLACK_LIGHT
                                  || u.umonnum == PM_VIOLET_FUNGUS
                                  || dmgtype(gy.youmonst.data, AD_STUN));
        if (ufound && !not_affected) {
            boolean chg;
            if (!Hallucination)
                You("被一阵万花筒般的光芒击中了!");
            /* avoid hallucinating the black light as it dies */
            mondead(mtmp);    /* remove it from map now */
            kill_agr = FALSE; /* already killed (maybe lifesaved) */
            chg =
                make_hallucinated(HHallucination + (long) tmp, FALSE, 0L);
            You("%s.", chg ? "被吓坏了" : "似乎不受影响");
        }
        break;
    default:
        impossible("unknown exploder damage type %d", mattk->adtyp);
        break;
    }
    if (not_affected) {
        You("似乎不受其影响.");
        ugolemeffects((int) mattk->adtyp, tmp);
    }
    if (kill_agr && !DEADMONSTER(mtmp))
        mondead(mtmp);
    wake_nearto(mtmp->mx, mtmp->my, 7 * 7);
    return (!DEADMONSTER(mtmp)) ? M_ATTK_MISS : M_ATTK_AGR_DIED;
}

/* monster gazes at you */
int
gazemu(struct monst *mtmp, struct attack *mattk)
{
    static const char *const reactions[] = {
        "混乱",              /* [0] */
        "眩晕",              /* [1] */
        "迷惑",  "目眩",     /* [2,3] */
        "恼怒",  "发炎",     /* [4,5] */
        "疲倦",              /* [6] */
        "迟钝",              /* [7] */
    };
    int react = -1;
    boolean is_medusa, reflectable,
            cancelled = (mtmp->mcan != 0), already = FALSE,
            mcanseeu = (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
                        && mtmp->mcansee);

    if (m_seenres(mtmp, cvt_adtyp_to_mseenres(mattk->adtyp)))
        return M_ATTK_MISS;

    is_medusa = (mtmp->data == &mons[PM_MEDUSA]);
    reflectable = (Reflecting && couldsee(mtmp->mx, mtmp->my) && is_medusa);
    /* assumes that hero has to see monster's gaze in order to be
       affected, rather than monster just having to look at hero;
       Unaware:  asleep or unconscious => not blind but won't see;
       when hallucinating, hero's brain doesn't register what
       it's seeing correctly so the gaze is usually ineffective
       [this could be taken a lot farther and select a gaze effect
       appropriate to what's currently being displayed, giving
       ordinary monsters a gaze attack when hero thinks he or she
       is facing a gazing creature, but let's not go that far...] */
    if ((Hallucination && rn2(4)) || (Unaware && !reflectable))
        cancelled = TRUE;

    switch (mattk->adtyp) {
    case AD_STON:
        /* note: Medusa is the only monster with stoning gaze, so
           'is_medusa' will always be True here */
        if (cancelled || !mtmp->mcansee) {
            if (!canseemon(mtmp))
                break; /* silently */
            if (Unaware) { /* can't see attacker even though not blind */
                react = is_medusa ? 4 : 2; /* irritated or puzzled */
                break;
            }
            if (is_medusa && Hallucination && !rn2(3))
                pline("某人似乎该挨蛇剪了.");
            else
                pline_mon(mtmp, "%s%s.", Monnam(mtmp),
                      (is_medusa && mtmp->mcan && !react)
                          ? "看起来并没有那么丑"
                          : "无效地凝视着");
            break;
         }
        if (reflectable) {
            /* hero has line of sight to Medusa and she's not blind */
            boolean useeit = canseemon(mtmp);

            if (useeit)
                (void) ureflects("%s的凝视被你的%s反射了.",
                                 Monnam(mtmp));
            if (mon_reflects(mtmp, !useeit ? (char *) 0
                                  : "凝视被%s的%s反射开了!"))
                break;
            if (!m_canseeu(mtmp)) { /* probably you're invisible */
                if (useeit)
                    pline(
                      "%s似乎没有注意到%s的凝视被反射了.",
                          Monnam(mtmp), mhis(mtmp));
                break;
            }
            if (useeit)
                pline_mon(mtmp, "%s变成了石头!", Monnam(mtmp));
            gs.stoned = TRUE;
            killed(mtmp);

            if (!DEADMONSTER(mtmp))
                break;
            return M_ATTK_AGR_DIED;
        }
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)
            && !Stone_resistance && !Unaware) {
            You("迎上了%s的凝视.", mon_nam(mtmp));
            stop_occupation();
            if (poly_when_stoned(gy.youmonst.data) && polymon(PM_STONE_GOLEM))
                break;
            urgent_pline("你变成了石头...");
            svk.killer.format = KILLED_BY;
            Strcpy(svk.killer.name, pmname(mtmp->data, Mgender(mtmp)));
            done(STONING);
        }
        break;
    case AD_CONF:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 0; /* "confused" */
                already = (mtmp->mconf != 0);
            } else {
                int conf = d(3, 4);

                mtmp->mspec_used = mtmp->mspec_used + (conf + rn2(6));
                if (!Confusion)
                    pline_mon(mtmp, "%s的凝视让你混乱!",
                              Monnam(mtmp));
                else
                    You("越来越困惑了.");
                make_confused(HConfusion + conf, FALSE);
                stop_occupation();
            }
        }
        break;
    case AD_STUN:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = 1; /* "stunned" */
                already = (mtmp->mstun != 0);
            } else {
                int stun = d(2, 6);

                mtmp->mspec_used = mtmp->mspec_used + (stun + rn2(6));
                pline_mon(mtmp, "%s锐利地盯着你!", Monnam(mtmp));
                make_stunned((HStun & TIMEOUT) + (long) stun, TRUE);
                stop_occupation();
            }
        }
        break;
    case AD_BLND:
        if (canseemon(mtmp) && !resists_blnd(&gy.youmonst)
            && mdistu(mtmp) <= BOLT_LIM * BOLT_LIM) {
            if (cancelled) {
                react = rn1(2, 2); /* "puzzled" || "dazzled" */
                already = (mtmp->mcansee == 0);
                /* Archons gaze every round; we don't want cancelled ones
                   giving the "seems puzzled/dazzled" message that often */
                if (mtmp->mcan && mtmp->data == &mons[PM_ARCHON] && rn2(5))
                    react = -1;
            } else {
                int blnd = d((int) mattk->damn, (int) mattk->damd);

                You("被%s的光芒致盲了!", mon_nam(mtmp));
                make_blinded((long) blnd, FALSE);
                stop_occupation();
                /* not blind at this point implies you're wearing
                   the Eyes of the Overworld; make them block this
                   particular stun attack too */
                if (!Blind) {
                    Your1(vision_clears);
                } else {
                    long oldstun = (HStun & TIMEOUT), newstun = (long) rnd(3);

                    /* we don't want to increment stun duration every time
                       or sighted hero will become incapacitated */
                    make_stunned(max(oldstun, newstun), TRUE);
                }
            }
        }
        break;
    case AD_FIRE:
        if (mcanseeu && !mtmp->mspec_used && rn2(5)) {
            if (cancelled) {
                react = rn1(2, 4); /* "irritated" || "inflamed" */
            } else {
                int dmg = d(2, 6), orig_dmg = dmg, lev = (int) mtmp->m_lev;

                pline_mon(mtmp, "%s用灼热的凝视攻击你!",
                          Monnam(mtmp));
                stop_occupation();
                if (Fire_resistance) {
                    shieldeff(u.ux, u.uy);
                    pline_The("火焰并不觉得热!");
                    monstseesu(M_SEEN_FIRE);
                    ugolemeffects(AD_FIRE, d(12, 6));
                    dmg = 0;
                } else {
                    monstunseesu(M_SEEN_FIRE);
                }
                burn_away_slime();
                if (lev > rn2(20))
                    (void) burnarmor(&gy.youmonst);
                if (lev > rn2(20)) {
                    (void) destroy_items(&gy.youmonst, AD_FIRE, orig_dmg);
                    ignite_items(gi.invent);
                }
                if (dmg)
                    mdamageu(mtmp, dmg);
            }
        }
        break;
#ifdef PM_BEHOLDER /* work in progress */
    case AD_SLEE:
        if (mcanseeu && gm.multi >= 0 && !rn2(5) && !Sleep_resistance) {
            if (cancelled) {
                react = 6;                      /* "tired" */
                already = (mtmp->mfrozen != 0); /* can't happen... */
            } else {
                fall_asleep(-rnd(10), TRUE);
                pline("%s的凝视让你非常困倦...",
                      Monnam(mtmp));
                monstunseesu(M_SEEN_SLEEP);
            }
        }
        break;
    case AD_SLOW:
        if (mcanseeu
            && (HFast & (INTRINSIC | TIMEOUT)) && !defended(mtmp, AD_SLOW)
            && !rn2(4)) {
            if (cancelled) {
                react = 7; /* "dulled" */
                already = (mtmp->mspeed == MSLOW);
            } else {
                u_slow_down();
                stop_occupation();
            }
        }
        break;
#endif /* BEHOLDER */
    default:
        impossible("Gaze attack %d?", mattk->adtyp);
        break;
    }
    if (react >= 0) {
        if (Hallucination && rn2(3))
            react = rn2(SIZE(reactions));
        /* cancelled/hallucinatory feedback; monster might look "confused",
           "stunned",&c but we don't actually set corresponding attribute */
        pline_mon(mtmp, "%s看起来%s%s.", Monnam(mtmp),
              !rn2(3) ? "" : already ? "相当"
                                     : (!rn2(2) ? "有点" : "略微"),
              reactions[react]);
    }
    return M_ATTK_MISS;
}

/* mtmp hits you for n points damage */
void
mdamageu(struct monst *mtmp, int n)
{
    if (n < 0) {
        impossible("mdamageu for negative damage? (%d)", n);
        n = 0;
    }

    disp.botl = TRUE;
    if (Upolyd) {
        u.mh -= n;
        showdamage(n);
        /* caller might have reduced mhmax before calling mdamageu() */
        if (u.mh > u.mhmax)
            u.mh = u.mhmax;
        if (u.mh < 1)
            rehumanize();
    } else {
        u.uhp -= n;
        showdamage(n);
        /* caller might have reduced uhpmax before calling mdamageu() */
        if (u.uhp > u.uhpmax)
            u.uhp = u.uhpmax;
        if (u.uhp < 1)
            done_in_by(mtmp, DIED);
    }
}

/* returns 0 if seduction impossible,
 *         1 if fine,
 *         2 if wrong gender for nymph
 */
int
could_seduce(
    struct monst *magr, struct monst *mdef,
    struct attack *mattk) /* non-Null: current atk; Null: genrl capability */
{
    struct permonst *pagr;
    boolean agrinvis, defperc;
    xint16 genagr, gendef;
    int adtyp;

    if (is_animal(magr->data))
        return 0;
    if (magr == &gy.youmonst) {
        pagr = gy.youmonst.data;
        agrinvis = (Invis != 0);
        genagr = poly_gender();
    } else {
        pagr = magr->data;
        agrinvis = magr->minvis;
        genagr = gender(magr);
    }
    if (mdef == &gy.youmonst) {
        defperc = (See_invisible != 0);
        gendef = poly_gender();
    } else {
        defperc = perceives(mdef->data);
        gendef = gender(mdef);
    }

    adtyp = mattk ? mattk->adtyp
            : dmgtype(pagr, AD_SSEX) ? AD_SSEX
              : dmgtype(pagr, AD_SEDU) ? AD_SEDU
                : AD_PHYS;
    if (adtyp == AD_SSEX && !SYSOPT_SEDUCE)
        adtyp = AD_SEDU;

    if (agrinvis && !defperc && adtyp == AD_SEDU)
        return 0;

    /* nymphs have two attacks, one for steal-item damage and the other
       for seduction, both pass the could_seduce() test;
       incubi/succubi have three attacks, their claw attacks for damage
       don't pass the test */
    if ((pagr->mlet != S_NYMPH && pagr != &mons[PM_AMOROUS_DEMON])
        || (adtyp != AD_SEDU && adtyp != AD_SSEX && adtyp != AD_SITM))
        return 0;

    return (genagr == 1 - gendef) ? 1 : (pagr->mlet == S_NYMPH) ? 2 : 0;
}

/* returns 1 if monster teleported (or hero leaves monster's vicinity) */
int
doseduce(struct monst *mon)
{
    struct obj *ring, *nring;
    boolean fem = (mon->data == &mons[PM_AMOROUS_DEMON]
                   && Mgender(mon) == FEMALE); /* otherwise incubus */
    boolean seewho, naked; /* True iff no armor */
    int attr_tot, tried_gloves = 0;
    char qbuf[QBUFSZ], Who[QBUFSZ];

    if (mon->mcan || mon->mspec_used) {
        pline_mon(mon, "%s表现得好像头%s疼.", Monnam(mon),
                  mon->mcan ? "很" : "");
        return 0;
    }
    if (unresponsive()) {
        pline_mon(mon, "%s似乎因你毫无反应而沮丧.",
                  Monnam(mon));
        return 0;
    }
    seewho = canseemon(mon);
    if (!seewho)
        pline("有人爱抚你...");
    else
        You_feel("很受%s的吸引.", mon_nam(mon));
    /* cache the seducer's name in a local buffer */
    Strcpy(Who, (!seewho ? (fem ? "她" : "他") : Monnam(mon)));

    /* if in the process of putting armor on or taking armor off,
       interrupt that activity now */
    (void) stop_donning((struct obj *) 0);
    /* don't try to take off gloves if cursed weapon blocks them */
    if (welded(uwep))
        tried_gloves = 1;

    for (ring = gi.invent; ring; ring = nring) {
        nring = ring->nobj;
        if (ring->otyp != RIN_ADORNMENT)
            continue;
        if (fem) {
            if (ring->owornmask && uarmg) {
                /* don't take off worn ring if gloves are in the way */
                if (!tried_gloves++)
                    mayberem(mon, Who, uarmg, "手套");
                if (uarmg)
                    continue; /* next ring might not be worn */
            }
            /* confirmation prompt when charisma is high bypassed if deaf */
            if (!Deaf && rn2(20) < ACURR(A_CHA)) {
                (void) safe_qbuf(qbuf, "\"那个",
                                 "看起来很漂亮. 可以给我吗?\"", ring,
                                 xname, simpleonames, "戒指");
                makeknown(RIN_ADORNMENT);
                SetVoice(mon, 0, 80, 0);
                if (y_n(qbuf) == 'n')
                    continue;
            } else
                pline("%s决定要%s, 然后拿走了它.",
                      Who, yname(ring));
            makeknown(RIN_ADORNMENT);
            /* might be in left or right ring slot or weapon/alt-wep/quiver */
            if (ring->owornmask)
                remove_worn_item(ring, FALSE);
            freeinv(ring);
            (void) mpickobj(mon, ring);
        } else {
            if (uleft && uright && uleft->otyp == RIN_ADORNMENT
                && uright->otyp == RIN_ADORNMENT)
                break;
            if (ring == uleft || ring == uright)
                continue;
            if (uarmg) {
                /* don't put on ring if gloves are in the way */
                if (!tried_gloves++)
                    mayberem(mon, Who, uarmg, "手套");
                if (uarmg)
                    break; /* no point trying further rings */
            }
            /* confirmation prompt when charisma is high bypassed if deaf */
            if (!Deaf && rn2(20) < ACURR(A_CHA)) {
                (void) safe_qbuf(qbuf, "\"那个",
                                "看起来很漂亮. 你愿意为我戴上它吗?\"",
                                 ring, xname, simpleonames, "戒指");
                makeknown(RIN_ADORNMENT);
                SetVoice(mon, 0, 80, 0);
                if (y_n(qbuf) == 'n')
                    continue;
            } else {
                pline("%s决定你戴上%s会更漂亮,",
                      Who, yname(ring));
                pline("并把它戴到你的手指上.");
            }
            makeknown(RIN_ADORNMENT);
            if (!uright) {
                pline("%s把%s戴到你的右%s.",
                      Who, the(xname(ring)), body_part(HAND));
                setworn(ring, RIGHT_RING);
            } else if (!uleft) {
                pline("%s把%s戴到你的左%s.",
                      Who, the(xname(ring)), body_part(HAND));
                setworn(ring, LEFT_RING);
            } else if (uright && uright->otyp != RIN_ADORNMENT) {
                /* note: the "replaces" message might be inaccurate if
                   hero's location changes and the process gets interrupted,
                   but trying to figure that out in advance in order to use
                   alternate wording is not worth the effort */
                pline("%s用%s替换了%s.",
                      Who, yname(uright), yname(ring));
                Ring_gone(uright);
                /* ring removal might cause loss of levitation which could
                   drop hero onto trap that transports hero somewhere else */
                if (u.utotype || !m_next2u(mon))
                    return 1;
                setworn(ring, RIGHT_RING);
            } else if (uleft && uleft->otyp != RIN_ADORNMENT) {
                /* see "replaces" note above */
                pline("%s用%s替换了%s.",
                      Who, yname(uleft), yname(ring));
                Ring_gone(uleft);
                if (u.utotype || !m_next2u(mon))
                    return 1;
                setworn(ring, LEFT_RING);
            } else
                impossible("ring replacement");
            Ring_on(ring);
            prinv((char *) 0, ring, 0L);
        }
    }

    naked = (!uarmc && !uarmf && !uarmg && !uarms && !uarmh && !uarmu);
    urgent_pline("%s%s%s.", Who,
                 Deaf ? "似乎在你耳边低语"
                 : naked ? "在你耳边喃喃情话"
                   : "在你耳边低语",
                 naked ? "" : ", 同时帮你宽衣");
    mayberem(mon, Who, uarmc, cloak_simple_name(uarmc));
    if (!uarmc)
        mayberem(mon, Who, uarm, suit_simple_name(uarm));
    mayberem(mon, Who, uarmf, "靴子");
    if (!tried_gloves)
        mayberem(mon, Who, uarmg, "手套");
    mayberem(mon, Who, uarms, "盾牌");
    mayberem(mon, Who, uarmh, helm_simple_name(uarmh));
    if (!uarmc && !uarm)
        mayberem(mon, Who, uarmu, "衬衫");

    /* removing armor (levitation boots, or levitation ring to make
       room for adornment ring with incubus case) might result in the
       hero falling through a trap door or landing on a teleport trap
       and changing location, so hero might not be adjacent to seducer
       any more (mayberem() has its own adjacency test so we don't need
       to check after each potential removal) */
    if (u.utotype || !m_next2u(mon))
        return 1;

    if (uarm || uarmc) {
        if (!Deaf) {
            if (!(ld() && mon->female)) {
                SetVoice(mon, 0, 80, 0);
                verbalize("你真是个%s; 我真希望...",
                          flags.female ? "甜美的女士" : "好男人");
            } else {
                struct obj *yourgloves = u_carried_gloves();

                /* have her call your gloves by their correct
                   name, possibly revealing them to you */
                if (yourgloves)
                    observe_object(yourgloves);
                verbalize("那么, 你欠我%s%s!",
                          yourgloves ? yname(yourgloves)
                                     : "十二双手套",
                          yourgloves ? "和另外十一双手套"
                                     : "");
            }
        } else if (seewho)
            pline_mon(mon, "%s似乎叹了口气.", Monnam(mon));
        /* else no regret message if can't see or hear seducer */

        if (!tele_restrict(mon))
            (void) rloc(mon, RLOC_MSG);
        return 1;
    }
    if (u.ualign.type == A_CHAOTIC)
        adjalign(1);

    /* by this point you have discovered mon's identity, blind or not... */
    urgent_pline(
             "时间静止了, 你和%s躺在彼此怀中...",
                 noit_mon_nam(mon));
    /* 3.6.1: a combined total for charisma plus intelligence of 35-1
       used to guarantee successful outcome; now total maxes out at 32
       as far as deciding what will happen; chance for bad outcome when
       Cha+Int is 32 or more is 2/35, a bit over 5.7% */
    attr_tot = ACURR(A_CHA) + ACURR(A_INT);
    if (rn2(35) > min(attr_tot, 32)) {
        /* Don't bother with mspec_used here... it didn't get tired! */
        pline("%s似乎比你更为享受...",
              noit_Monnam(mon));
        switch (rn2(5)) {
        case 0:
            You_feel("耗尽了能量.");
            u.uen = 0;
            u.uenmax -= rnd(Half_physical_damage ? 5 : 10);
            exercise(A_CON, FALSE);
            if (u.uenmax < 0)
                u.uenmax = 0;
            break;
        case 1:
            You("情绪低落.");
            (void) adjattrib(A_CON, -1, TRUE);
            exercise(A_CON, FALSE);
            disp.botl = TRUE;
            break;
        case 2:
            Your("感官迟钝.");
            (void) adjattrib(A_WIS, -1, TRUE);
            exercise(A_WIS, FALSE);
            disp.botl = TRUE;
            break;
        case 3:
            if (!resists_drli(&gy.youmonst)) {
                You_feel("不成样子.");
                losexp("过度劳累");
            } else {
                You("有一种古怪的感觉...");
            }
            exercise(A_CON, FALSE);
            exercise(A_DEX, FALSE);
            exercise(A_WIS, FALSE);
            break;
        case 4: {
            int tmp;

            You_feel("疲惫.");
            exercise(A_STR, FALSE);
            tmp = rn1(10, 6);
            losehp(Maybe_Half_Phys(tmp), "精疲力竭", KILLED_BY);
            break;
        } /* case 4 */
        } /* switch */
    } else {
        mon->mspec_used = rnd(100); /* monster is worn out */
        You("似乎比%s更为享受...", noit_mon_nam(mon));
        switch (rn2(5)) {
        case 0:
            You_feel("唤起了你的全部潜能.");
            exercise(A_CON, TRUE);
            u.uen = (u.uenmax += rnd(5));
            if (u.uenmax > u.uenpeak)
                u.uenpeak = u.uenmax;
            break;
        case 1:
            You_feel("好到足以再来一次.");
            (void) adjattrib(A_CON, 1, TRUE);
            exercise(A_CON, TRUE);
            disp.botl = TRUE;
            break;
        case 2:
            You("会永远记住%s...", noit_mon_nam(mon));
            (void) adjattrib(A_WIS, 1, TRUE);
            exercise(A_WIS, TRUE);
            disp.botl = TRUE;
            break;
        case 3:
            pline("这是一个非常有教育性的经验.");
            pluslvl(FALSE);
            exercise(A_WIS, TRUE);
            break;
        case 4:
            You_feel("恢复了健康!");
            u.uhp = u.uhpmax;
            if (Upolyd)
                u.mh = u.mhmax;
            exercise(A_STR, TRUE);
            disp.botl = TRUE;
            break;
        }
    }

    if (mon->mtame) { /* don't charge */
        ;
    } else if (rn2(20) < ACURR(A_CHA)) {
        pline("%s要求你付钱给%s, 但你拒绝了...",
              noit_Monnam(mon), noit_mhim(mon));
    } else if (u.umonnum == PM_LEPRECHAUN) {
        pline_mon(mon, "%s试图拿走你的金币, 但失败了...",
                  noit_Monnam(mon));
    } else {
        long cost;
        long umoney = money_cnt(gi.invent);

        if (umoney > (long) LARGEST_INT - 10L)
            cost = (long) rnd(LARGEST_INT) + 500L;
        else
            cost = (long) rnd((int) umoney + 10) + 500L;
        if (mon->mpeaceful) {
            cost /= 5L;
            if (!cost)
                cost = 1L;
        }
        if (cost > umoney)
            cost = umoney;
        if (!cost) {
            if (!Deaf) {
                SetVoice(mon, 0, 80, 0);
                verbalize("这次免费!");
            } else {
                pline("免费.");
            }
        } else {
            pline_mon(mon, "%s拿走%ld%s作为服务报酬!",
                      noit_Monnam(mon), cost, currency(cost));
            money2mon(mon, cost);
            disp.botl = TRUE;
        }
    }
    if (!rn2(25))
        mon->mcan = 1; /* monster is worn out */
    if (!tele_restrict(mon))
        (void) rloc(mon, RLOC_MSG);
    return 1;
}

/* 'mon' tries to remove a piece of hero's armor */
staticfn void
mayberem(struct monst *mon,
         const char *seducer, /* only used for alternate message */
         struct obj *obj, const char *str)
{
    char qbuf[QBUFSZ];

    if (!obj || !obj->owornmask)
        return;
    /* removal of a previous item might have sent the hero elsewhere
       (loss of levitation that leads to landing on a transport trap) */
    if (u.utotype || !m_next2u(mon))
        return;

    /* being deaf overrides confirmation prompt for high charisma */
    if (Deaf) {
        pline("%s脱掉了你的%s.", seducer, str);
    } else if (rn2(20) < ACURR(A_CHA)) {
        SetVoice(mon, 0, 80, 0); /* y_n aka yn_function is set up for this */
        Sprintf(qbuf, "\"要我脱掉你的%s, %s?\"", str,
                (!rn2(2) ? "情人" : !rn2(2) ? "亲爱的" : "甜心"));
        if (y_n(qbuf) == 'n')
            return;
    } else {
        char hairbuf[BUFSZ];

        Sprintf(hairbuf, "让我用手指抚过你的%s",
                body_part(HAIR));
        SetVoice(mon, 0, 80, 0);
        verbalize("脱掉你的%s; %s.", str,
                  (obj == uarm)
                     ? "让我们再靠近一点"
                     : (obj == uarmc || obj == uarms)
                        ? "它碍事了"
                        : (obj == uarmf)
                           ? "让我揉揉你的脚"
                           : (obj == uarmg)
                              ? "它们太笨重了"
                              : (obj == uarmu)
                                 ? "让我给你按摩"
                                 /* obj == uarmh */
                                 : hairbuf);
    }
    remove_worn_item(obj, TRUE);
}

staticfn int
assess_dmg(struct monst *mtmp, int tmp)
{
    if ((mtmp->mhp -= tmp) <= 0) {
        pline_mon(mtmp, "%s死了!", Monnam(mtmp));
        xkilled(mtmp, XKILL_NOMSG);
        if (!DEADMONSTER(mtmp))
            return M_ATTK_HIT;
        return M_ATTK_AGR_DIED;
    }
    return M_ATTK_HIT;
}

#if 0
/* returns True if monster has a range attack in its repertoire
   that it will actually utilize. Caller provides the assessment
   callback (optional). Callback returns 0 if the attack is
   active */

boolean
ranged_attk_assessed(
    struct monst *mtmp,
    boolean (*assessfunc)(struct monst *, int))
{
    int i;
    struct permonst *ptr = mtmp->data;

    for (i = 0; i < NATTK; i++)
        if (DISTANCE_ATTK_TYPE(ptr->mattk[i].aatyp)) {
            if (!assessfunc || (*assessfunc)(mtmp, i) == 0)
                return TRUE;
        }
    return FALSE;
}
#endif

/* can be used as ranged_attk_assessed() callback.
   Returns TRUE if monster is avoiding use of this attack */
boolean
mon_avoiding_this_attack(
    struct monst *mtmp,
    int attkidx)
{
    struct permonst *ptr = mtmp->data;
    int typ = -1;

    if (attkidx >= 0
        && (typ = get_atkdam_type(ptr->mattk[attkidx].adtyp)) >= 0
        && m_seenres(mtmp, cvt_adtyp_to_mseenres(typ)))
        return TRUE;
    return FALSE;
}

/*
 * This would be equivalent to:
 *     ranged_attk_assessed(mtmp, mon_avoiding_this_attack)
 * but without the added assessment function call overhead.
 */
boolean
ranged_attk_available(struct monst *mtmp)
{
    int i, typ = -1;
    struct permonst *ptr = mtmp->data;

    for (i = 0; i < NATTK; i++) {
        if (DISTANCE_ATTK_TYPE(ptr->mattk[i].aatyp)
            && (typ = get_atkdam_type(ptr->mattk[i].adtyp)) >= 0
                && m_seenres(mtmp, cvt_adtyp_to_mseenres(typ)) == 0)
                return TRUE;
    }
    return FALSE;
}

/* FIXME:
 *  sequencing issue:  a monster's attack might cause poly'd hero
 *  to revert to normal form.  The messages for passive counterattack
 *  would look better if they came before reverting form, but we need
 *  to know whether hero reverted in order to decide whether passive
 *  damage applies.
 */
staticfn int
passiveum(
    struct permonst *olduasmon,
    struct monst *mtmp,
    struct attack *mattk)
{
    int i, tmp;
    struct attack *oldu_mattk = 0;

    /*
     * mattk      == mtmp's attack that hit you;
     * oldu_mattk == your passive counterattack (even if mtmp's attack
     *               has already caused you to revert to normal form).
     */
    for (i = 0; !oldu_mattk; i++) {
        if (i >= NATTK)
            return M_ATTK_HIT;
        if (olduasmon->mattk[i].aatyp == AT_NONE
            || olduasmon->mattk[i].aatyp == AT_BOOM)
            oldu_mattk = &olduasmon->mattk[i];
    }
    if (oldu_mattk->damn)
        tmp = d((int) oldu_mattk->damn, (int) oldu_mattk->damd);
    else if (oldu_mattk->damd)
        tmp = d((int) olduasmon->mlevel + 1, (int) oldu_mattk->damd);
    else
        tmp = 0;

    /* These affect the enemy even if you were "killed" (rehumanized) */
    switch (oldu_mattk->adtyp) {
    case AD_ACID:
        if (!rn2(2)) {
            pline_mon(mtmp, "%s被%s%s溅到了!", Monnam(mtmp),
                  /* temporary? hack for sequencing issue:  "your acid"
                     looks strange coming immediately after player has
                     been told that hero has reverted to normal form */
                  !Upolyd ? "" : "你的", hliquid("酸"));
            if (resists_acid(mtmp)) {
                pline_mon(mtmp, "%s不受影响.", Monnam(mtmp));
                tmp = 0;
            }
        } else
            tmp = 0;
        if (!rn2(30))
            erode_armor(mtmp, ERODE_CORRODE);
        if (!rn2(6))
            acid_damage(MON_WEP(mtmp));
        return assess_dmg(mtmp, tmp);
    case AD_STON: /* cockatrice */
    {
        long protector = attk_protection((int) mattk->aatyp),
             wornitems = mtmp->misc_worn_check;

        /* wielded weapon gives same protection as gloves here */
        if (MON_WEP(mtmp) != 0)
            wornitems |= W_ARMG;

        if (!resists_ston(mtmp)
            && (protector == 0L
                || (protector != ~0L
                    && (wornitems & protector) != protector))) {
            if (poly_when_stoned(mtmp->data)) {
                mon_to_stone(mtmp);
                return 1;
            }
            pline_mon(mtmp, "%s变成了石头!", Monnam(mtmp));
            gs.stoned = 1;
            xkilled(mtmp, XKILL_NOMSG);
            if (!DEADMONSTER(mtmp))
                return M_ATTK_HIT;
            return M_ATTK_AGR_DIED;
        }
        return M_ATTK_HIT;
    }
    case AD_ENCH: /* KMH -- remove enchantment (disenchanter) */
        if (mon_currwep) {
            /* by_you==True: passive counterattack to hero's action
               is hero's fault */
            (void) drain_item(mon_currwep, TRUE);
            /* No message */
        }
        return M_ATTK_HIT;
    default:
        break;
    }
    if (!Upolyd)
        return M_ATTK_HIT;

    /* These affect the enemy only if you are still a monster */
    if (rn2(3))
        switch (oldu_mattk->adtyp) {
        case AD_PHYS:
            if (oldu_mattk->aatyp == AT_BOOM) {
                You("爆炸!");
                /* KMH, balance patch -- this is okay with unchanging */
                rehumanize();
                return assess_dmg(mtmp, tmp);
            }
            break;
        case AD_PLYS: /* Floating eye */
            if (tmp > 127)
                tmp = 127;
            if (u.umonnum == PM_FLOATING_EYE) {
                if (!rn2(4))
                    tmp = 127;
                if (mtmp->mcansee && haseyes(mtmp->data) && rn2(3)
                    && (perceives(mtmp->data) || !Invis)) {
                    if (Blind) {
                        pline("作为失明的%s, 你不能保护自己.",
                              pmname(gy.youmonst.data,
                                     flags.female ? FEMALE : MALE));
                    } else {
                        if (mon_reflects(mtmp,
                                         "你的凝视被%s的%s反射了."))
                            return 1;
                        pline_mon(mtmp, "%s被你的凝视冻住了!",
                                  Monnam(mtmp));
                        paralyze_monst(mtmp, tmp);
                        return M_ATTK_AGR_DONE;
                    }
                }
            } else { /* gelatinous cube */
                pline_mon(mtmp, "%s被你冻住了.", Monnam(mtmp));
                paralyze_monst(mtmp, tmp);
                return M_ATTK_AGR_DONE;
            }
            return M_ATTK_HIT;
        case AD_COLD: /* Brown mold or blue jelly */
            if (resists_cold(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline_mon(mtmp, "%s感到微微发冷.", Monnam(mtmp));
                golemeffects(mtmp, AD_COLD, tmp);
                tmp = 0;
                break;
            }
            pline_mon(mtmp, "%s突然变得非常寒冷!", Monnam(mtmp));
            u.mh += (tmp + rn2(2)) / 2;
            if (u.mhmax < u.mh)
                u.mhmax = u.mh;
            if (u.mhmax > (((int) gy.youmonst.data->mlevel + 1) * 8))
                (void) split_mon(&gy.youmonst, mtmp);
            break;
        case AD_STUN: /* Yellow mold */
            if (!mtmp->mstun) {
                mtmp->mstun = 1;
                pline_mon(mtmp, "%s%s.", Monnam(mtmp),
                      makeplural(stagger(mtmp->data, "蹒跚")));
            }
            tmp = 0;
            break;
        case AD_FIRE: /* Red mold */
            if (resists_fire(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline_mon(mtmp, "%s感到微微发热.", Monnam(mtmp));
                golemeffects(mtmp, AD_FIRE, tmp);
                tmp = 0;
                break;
            }
            pline_mon(mtmp, "%s突然变得非常灼热!", Monnam(mtmp));
            break;
        case AD_ELEC:
            if (resists_elec(mtmp)) {
                shieldeff(mtmp->mx, mtmp->my);
                pline_mon(mtmp, "%s感到轻微刺痛.", Monnam(mtmp));
                golemeffects(mtmp, AD_ELEC, tmp);
                tmp = 0;
                break;
            }
            pline_mon(mtmp, "%s被你的电流猛击!",
                      Monnam(mtmp));
            break;
        default:
            tmp = 0;
            break;
        }
    else
        tmp = 0;

    return assess_dmg(mtmp, tmp);
}

struct monst *
cloneu(void)
{
    struct monst *mon;
    int mndx = monsndx(gy.youmonst.data);

    if (u.mh <= 1)
        return (struct monst *) 0;
    if (svm.mvitals[mndx].mvflags & G_EXTINCT)
        return (struct monst *) 0;
    mon = makemon(gy.youmonst.data, u.ux, u.uy,
                  NO_MINVENT | MM_EDOG | MM_NOMSG);
    if (!mon)
        return NULL;
    mon->mcloned = 1;
    mon = christen_monst(mon, svp.plname);
    initedog(mon, TRUE);
    mon->m_lev = gy.youmonst.data->mlevel;
    mon->mhpmax = u.mhmax;
    mon->mhp = u.mh / 2;
    u.mh -= mon->mhp;
    disp.botl = TRUE;
    return mon;
}

#undef ld

/*mhitu.c*/
