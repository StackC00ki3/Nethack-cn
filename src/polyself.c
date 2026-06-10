/* NetHack 5.0	polyself.c	$NHDT-Date: 1772101811 2026/02/26 02:30:11 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.227 $ */
/*      Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Polymorph self routine.
 *
 * Note:  the light source handling code assumes that gy.youmonst.m_id
 * always remains 1 and gy.youmonst.mx will always remain 0 when it handles
 * the case of the player polymorphed into a light-emitting monster.
 *
 * Transformation sequences:
 *              /-> polymon                 poly into monster form
 *    polyself =
 *              \-> newman -> polyman       fail to poly, get human form
 *
 *    rehumanize -> polyman                 return to original form
 *
 *    polymon (called directly)             usually golem petrification
 */

#include "hack.h"

staticfn void check_strangling(boolean);
staticfn void polyman(const char *, const char *);
staticfn void dropp(struct obj *);
staticfn void break_armor(void);
staticfn void drop_weapon(int);
staticfn int armor_to_dragon(int);
staticfn void newman(void);
staticfn void polysense(void);

static const char no_longer_petrify_resistant[] =
    "你不再免疫石化,然后你";

/* update the gy.youmonst.data structure pointer and intrinsics */
void
set_uasmon(void)
{
    struct permonst *mdat = &mons[u.umonnum];
    boolean was_vampshifter = valid_vampshiftform(gy.youmonst.cham, u.umonnum);

    set_mon_data(&gy.youmonst, mdat);
    gy.youmonst.m_id = 1;

    if (Protection_from_shape_changers)
        gy.youmonst.cham = NON_PM;
    else if (is_vampire(gy.youmonst.data))
        gy.youmonst.cham = gy.youmonst.mnum;
    /* assume hero-as-chameleon/doppelganger/sandestin doesn't change shape */
    else if (!was_vampshifter)
        gy.youmonst.cham = NON_PM;
    u.mcham = gy.youmonst.cham; /* for save/restore since youmonst isn't */

#define PROPSET(PropIndx, ON)                          \
    do {                                               \
        if (ON)                                        \
            u.uprops[PropIndx].intrinsic |= FROMFORM;  \
        else                                           \
            u.uprops[PropIndx].intrinsic &= ~FROMFORM; \
    } while (0)
#define resist_from_form(MRtyp) ((gy.youmonst.data->mresists & (MRtyp)) != 0)

    PROPSET(FIRE_RES, resist_from_form(MR_FIRE));
    PROPSET(COLD_RES, resist_from_form( MR_COLD));
    PROPSET(SLEEP_RES, resist_from_form(MR_SLEEP));
    PROPSET(DISINT_RES, resist_from_form(MR_DISINT));
    PROPSET(SHOCK_RES, resist_from_form(MR_ELEC));
    PROPSET(POISON_RES, resist_from_form(MR_POISON));
    PROPSET(ACID_RES, resist_from_form(MR_ACID));
    PROPSET(STONE_RES, resist_from_form(MR_STONE));
    {
        /* resists_drli() takes wielded weapon into account; suppress it */
        struct obj *save_uwep = uwep;

        uwep = 0;
        PROPSET(DRAIN_RES, resists_drli(&gy.youmonst));
        uwep = save_uwep;
    }
    /* resists_magm() takes wielded, worn, and carried equipment into
       into account; cheat and duplicate its monster-specific part */
    PROPSET(ANTIMAGIC, (dmgtype(mdat, AD_MAGM)
                        || mdat == &mons[PM_BABY_GRAY_DRAGON]
                        || dmgtype(mdat, AD_RBRE)));
    PROPSET(SICK_RES, (mdat->mlet == S_FUNGUS || mdat == &mons[PM_GHOUL]));

    PROPSET(STUNNED, (mdat == &mons[PM_STALKER] || is_bat(mdat)));
    PROPSET(HALLUC_RES, dmgtype(mdat, AD_HALU));
    PROPSET(SEE_INVIS, perceives(mdat));
    PROPSET(TELEPAT, telepathic(mdat));
    /* note that Infravision uses mons[race] rather than usual mons[role] */
    PROPSET(INFRAVISION, infravision(Upolyd ? mdat : &mons[gu.urace.mnum]));
    PROPSET(INVIS, pm_invisible(mdat));
    PROPSET(TELEPORT, can_teleport(mdat));
    PROPSET(TELEPORT_CONTROL, control_teleport(mdat));
    PROPSET(LEVITATION, is_floater(mdat));
    /* floating eye is the only 'floater'; it is also flagged as a 'flyer';
       suppress flying for it so that enlightenment doesn't confusingly
       show latent flight capability always blocked by levitation */
    PROPSET(FLYING, (is_flyer(mdat) && !is_floater(mdat)));
    PROPSET(SWIMMING, is_swimmer(mdat));
    /* [don't touch MAGICAL_BREATHING here; both Amphibious and Breathless
       key off of it but include different monster forms...] */
    PROPSET(PASSES_WALLS, passes_walls(mdat));
    PROPSET(REGENERATION, regenerates(mdat));
    PROPSET(REFLECTING, (mdat == &mons[PM_SILVER_DRAGON]));
    PROPSET(BLINDED, !haseyes(mdat));
    PROPSET(BLND_RES, (dmgtype_fromattack(mdat, AD_BLND, AT_EXPL)
                       || dmgtype_fromattack(mdat, AD_BLND, AT_GAZE)));
#undef PROPSET
#undef resist_from_form

    /* whether the player is flying/floating depends on their steed,
       which won't be known during the restore process: but BFlying
       and BStealth should be set correctly already in that case, so
       there's nothing to do */
    if (!program_state.restoring)
        float_vs_flight(); /* maybe toggle (BFlying & I_SPECIAL) */
    polysense();

#ifdef STATUS_HILITES
    if (VIA_WINDOWPORT())
        status_initialize(REASSESS_ONLY);
#endif
    /* we can reset this now, having just done what it is meant to trigger */
    gw.were_changes = 0L;
}

/* Levitation overrides Flying; set or clear BFlying|I_SPECIAL */
void
float_vs_flight(void)
{
    boolean stuck_in_floor = (u.utrap && u.utraptype != TT_PIT);

    /* floating overrides flight; so does being trapped in the floor */
    if ((HLevitation || ELevitation)
        || ((HFlying || EFlying) && stuck_in_floor))
        BFlying |= I_SPECIAL;
    else
        BFlying &= ~I_SPECIAL;
    /* being trapped on the ground (bear trap, web, molten lava survived
       with fire resistance, former lava solidified via cold, tethered
       to a buried iron ball) overrides floating--the floor is reachable */
    if ((HLevitation || ELevitation) && stuck_in_floor)
        BLevitation |= I_SPECIAL;
    else
        BLevitation &= ~I_SPECIAL;

    /* riding blocks stealth unless hero+steed fly, so a change in flying
       might cause a change in stealth */
    steed_vs_stealth();

    disp.botl = TRUE;
}

/* riding blocks stealth unless hero+steed fly */
void
steed_vs_stealth(void)
{
    if (u.usteed && !Flying && !Levitation)
        BStealth |= FROMOUTSIDE;
    else
        BStealth &= ~FROMOUTSIDE;
}

/* for changing into form that's immune to strangulation */
staticfn void
check_strangling(boolean on)
{
    /* on -- maybe resume strangling */
    if (on) {
        boolean was_strangled = (Strangled != 0L);

        /* when Strangled is already set, polymorphing from one
           vulnerable form into another causes the counter to be reset */
        if (uamul && uamul->otyp == AMULET_OF_STRANGULATION
            && can_be_strangled(&gy.youmonst)) {
            Strangled = 6L;
            disp.botl = TRUE;
            Your("%s%s你的%s!", simpleonames(uamul),
                 was_strangled ? "仍然在勒住" : "开始勒住",
                 body_part(NECK)); /* "throat" */
            makeknown(AMULET_OF_STRANGULATION);
        }

    /* off -- maybe block strangling */
    } else {
        if (Strangled && !can_be_strangled(&gy.youmonst)) {
            Strangled = 0L;
            disp.botl = TRUE;
            You("不再被窒息了.");
        }
    }
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* make a (new) human out of the player */
staticfn void
polyman(const char *fmt, const char *arg)
{
    boolean sticking = (sticks(gy.youmonst.data) && u.ustuck && !u.uswallow),
            was_mimicking = (U_AP_TYPE != M_AP_NOTHING);
    boolean was_blind = !!Blind,
            had_see_invis = !!See_invisible;

    if (Upolyd) {
        u.acurr = u.macurr; /* restore old attribs */
        u.amax = u.mamax;
        u.umonnum = u.umonster;
        flags.female = u.mfemale;
    }
    set_uasmon();

    u.mh = u.mhmax = 0;
    u.mtimedone = 0;
    skinback(FALSE);
    u.uundetected = 0;

    if (sticking)
        uunstick();
    find_ac();
    if (was_mimicking) {
        if (gm.multi < 0)
            unmul("");
        gy.youmonst.m_ap_type = M_AP_NOTHING;
        gy.youmonst.mappearance = 0;
    }

    newsym(u.ux, u.uy);

    urgent_pline(fmt, arg);
    /* check whether player foolishly genocided self while poly'd */
    if (ugenocided()) {
        /* intervening activity might have clobbered genocide info */
        struct kinfo *kptr = find_delayed_killer(POLYMORPH);

        if (kptr != (struct kinfo *) 0 && kptr->name[0]) {
            svk.killer.format = kptr->format;
            Strcpy(svk.killer.name, kptr->name);
        } else {
            svk.killer.format = KILLED_BY;
            Strcpy(svk.killer.name, "自我灭绝");
        }
        dealloc_killer(kptr);
        done(GENOCIDED);
    }

    if (!!See_invisible ^ had_see_invis)
        set_mimic_blocking(); /* See_invisible just toggled */

    if (u.twoweap && !could_twoweap(gy.youmonst.data))
        untwoweapon();

    if (u.utrap && u.utraptype == TT_PIT) {
        set_utrap(rn1(6, 2), TT_PIT); /* time to escape resets */
    }
    if (was_blind && !Blind) { /* reverting from eyeless */
        set_itimeout(&HBlinded, 1L);
        make_blinded(0L, TRUE); /* remove blindness */
    }
    check_strangling(TRUE);

    if (!Levitation && !u.ustuck && is_pool_or_lava(u.ux, u.uy))
        spoteffects(TRUE);

    see_monsters();
}

RESTORE_WARNING_FORMAT_NONLITERAL

void
change_sex(void)
{
    /* Some monsters are always of one sex and their sex can't be changed;
     * Succubi/incubi can change, but are handled below.
     *
     * !Upolyd check necessary because is_male() and is_female()
     * may be true for certain roles
     */
    if (!Upolyd
        || (!is_male(gy.youmonst.data) && !is_female(gy.youmonst.data)
            && !is_neuter(gy.youmonst.data)))
        flags.female = !flags.female;
    if (Upolyd) /* poly'd: also change saved sex */
        u.mfemale = !u.mfemale;
    max_rank_sz(); /* [this appears to be superfluous] */
    if ((Upolyd ? u.mfemale : flags.female) && gu.urole.name.f)
        Strcpy(svp.pl_character, gu.urole.name.f);
    else
        Strcpy(svp.pl_character, gu.urole.name.m);
    if (!Upolyd) {
        u.umonnum = u.umonster;
    } else if (u.umonnum == PM_AMOROUS_DEMON) {
        flags.female = !flags.female;
#if 0
        /* change monster type to match new sex; disabled with
           PM_AMOROUS_DEMON */
        u.umonnum = (u.umonnum == PM_SUCCUBUS) ? PM_INCUBUS : PM_SUCCUBUS;
#endif
        set_uasmon();
    }
}

/* log a message if non-poly'd hero's gender has changed */
void
livelog_newform(boolean viapoly, int oldgend, int newgend)
{
    char buf[BUFSZ];
    const char *oldrole, *oldrank, *newrole, *newrank;

    /*
     * TODO?
     *  Give other logging feedback here instead of in newman().
     */

    if (!Upolyd) {
        if (newgend != oldgend) {
            oldrole = (oldgend && gu.urole.name.f) ? gu.urole.name.f
                                                  : gu.urole.name.m;
            newrole = (newgend && gu.urole.name.f) ? gu.urole.name.f
                                                  : gu.urole.name.m;
            oldrank = rank_of(u.ulevel, Role_switch, oldgend);
            newrank = rank_of(u.ulevel, Role_switch, newgend);
            Sprintf(buf, "%.10s %.30s", genders[flags.female].adj, newrank);
            livelog_printf(LL_MINORAC, "%s成了%s",
                           viapoly ? "变形" : "转变", /* 注:transform=性别转变, polymorph=变形 */
                           an(strcmp(newrole, oldrole) ? newrole
                              : strcmp(newrank, oldrank) ? newrank
                                : buf));
        }
    }
}

staticfn void
newman(void)
{
    const char *newform;
    int i, oldlvl, newlvl, oldgend, newgend, hpmax, enmax;

    oldlvl = u.ulevel;
    newlvl = oldlvl + rn1(5, -2);     /* new = old + {-2,-1,0,+1,+2} */
    if (newlvl > 127 || newlvl < 1) { /* level went below 0? */
        goto dead; /* old level is still intact (in case of lifesaving) */
    }
    if (newlvl > MAXULEV)
        newlvl = MAXULEV;
    /* If your level goes down, your peak level goes down by
       the same amount so that you can't simply use blessed
       full healing to undo the decrease.  But if your level
       goes up, your peak level does *not* undergo the same
       adjustment; you might end up losing out on the chance
       to regain some levels previously lost to other causes. */
    if (newlvl < oldlvl)
        u.ulevelmax -= (oldlvl - newlvl);
    if (u.ulevelmax < newlvl)
        u.ulevelmax = newlvl;
    u.ulevel = newlvl;

    oldgend = poly_gender();
    if (gs.sex_change_ok && !rn2(10))
        change_sex();

    adjabil(oldlvl, (int) u.ulevel);

    /* random experience points for the new experience level */
    u.uexp = rndexp(FALSE);

    /* set up new attribute points (particularly Con) */
    redist_attr();

    /*
     * New hit points:
     *  remove "level gain"-based HP from any extra HP accumulated
     *  (the "extra" might actually be negative);
     *  modify the extra, retaining {80%, 90%, 100%, or 110%};
     *  add in newly generated set of level-gain HP.
     *
     * (This used to calculate new HP in direct proportion to old HP,
     * but that was subject to abuse:  accumulate a large amount of
     * extra HP, drain level down to 1, then polyself to level 2 or 3
     * [lifesaving capability needed to handle level 0 and -1 cases]
     * and the extra got multiplied by 2 or 3.  Repeat the level
     * drain and polyself steps until out of lifesaving capability.)
     */
    hpmax = u.uhpmax;
    for (i = 0; i < oldlvl; i++)
        hpmax -= (int) u.uhpinc[i];
    /* hpmax * rn1(4,8) / 10; 0.95*hpmax on average */
    hpmax = rounddiv((long) hpmax * (long) rn1(4, 8), 10);
    for (i = 0; (u.ulevel = i) < newlvl; i++)
        hpmax += newhp();
    if (hpmax < u.ulevel)
        hpmax = u.ulevel; /* min of 1 HP per level */
    /* retain same proportion for current HP; u.uhp * hpmax / u.uhpmax */
    u.uhp = rounddiv((long) u.uhp * (long) hpmax, u.uhpmax);
    setuhpmax(hpmax, TRUE); /* might reduce u.uhp */
    /*
     * Do the same for spell power.
     */
    enmax = u.uenmax;
    for (i = 0; i < oldlvl; i++)
        enmax -= (int) u.ueninc[i];
    enmax = rounddiv((long) enmax * (long) rn1(4, 8), 10);
    for (i = 0; (u.ulevel = i) < newlvl; i++)
        enmax += newpw();
    if (enmax < u.ulevel)
        enmax = u.ulevel;
    u.uen = rounddiv((long) u.uen * (long) enmax,
                     ((u.uenmax < 1) ? 1 : u.uenmax));
    u.uenmax = enmax;
    /* [should alignment record be tweaked too?] */

    u.uhunger = rn1(500, 500);
    if (Sick)
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
    if (Stoned)
        make_stoned(0L, (char *) 0, 0, (char *) 0);
    if (u.uhp <= 0) {
        if (Polymorph_control) { /* even when Stunned || Unaware */
            if (u.uhp <= 0)
                u.uhp = 1;
        } else {
 dead:      /* we come directly here if experience level went to 0 or less */
            urgent_pline(
                     "你的新形态看上去健康状况不那么好,难以存活.");
            svk.killer.format = KILLED_BY_AN;
            Strcpy(svk.killer.name, "不成功的变形");
            done(DIED);
            /* must have been life-saved to get here */
            newuhs(FALSE);
            encumber_msg(); /* used to be done by redist_attr() */
            return; /* lifesaved */
        }
    }
    newuhs(FALSE);
    /* use saved gender we're about to revert to, not current */
    newform = ((Upolyd ? u.mfemale : flags.female) && gu.urace.individual.f)
                ? gu.urace.individual.f
                : (gu.urace.individual.m)
                   ? gu.urace.individual.m
                   : gu.urace.noun;
    polyman("你感觉你像一个新%s!", newform);

    newgend = poly_gender();
    /* note: newman() bypasses achievements for new ranks attained and
       doesn't log "new <form>" when that isn't accompanied by level change */
    if (newlvl != oldlvl)
        livelog_printf(LL_MINORAC, "像一个新%s一样升到了%d级",
                       newform, newlvl); /*修改语序:newlvl, newform);*/
    else
        livelog_newform(TRUE, oldgend, newgend);

    if (Slimed) {
        Your("身体变形了,但你身上仍然有黏液.");
        make_slimed(10L, (const char *) 0);
    }

    disp.botl = TRUE;
    see_monsters();
    encumber_msg();

    retouch_equipment(2);
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
}

void
polyself(int psflags)
{
    char buf[BUFSZ];
    int old_light, new_light, mntmp, class, tryct, gvariant = NEUTRAL;
    boolean forcecontrol = ((psflags & POLY_CONTROLLED) != 0),
            low_control = ((psflags & POLY_LOW_CTRL) != 0),
            monsterpoly = ((psflags & POLY_MONSTER) != 0),
            formrevert = ((psflags & POLY_REVERT) != 0),
            draconian = (uarm && Is_dragon_armor(uarm)),
            iswere = (ismnum(u.ulycn)),
            isvamp = (is_vampire(gy.youmonst.data)
                      || is_vampshifter(&gy.youmonst)),
            controllable_poly = Polymorph_control && !(Stunned || Unaware);

    if (Unchanging) {
        You("变形失败了!");
        return;
    }
    /* being Stunned|Unaware doesn't negate this aspect of Poly_control */
    if (!Polymorph_control && !forcecontrol && !draconian && !iswere
        && !isvamp) {
        if (rn2(20) > ACURR(A_CON)) {
            You1(shudder_for_moment);
            losehp(rnd(30), "系统休克", KILLED_BY_AN);
            exercise(A_CON, FALSE);
            return;
        }
    }
    old_light = emits_light(gy.youmonst.data);
    mntmp = NON_PM;

    if (formrevert) {
        mntmp = gy.youmonst.cham;
        monsterpoly = TRUE;
        controllable_poly = FALSE;
    }

    if (forcecontrol && low_control
        && (draconian || monsterpoly || isvamp || iswere))
        forcecontrol = FALSE;

    if (monsterpoly && isvamp)
        goto do_vampyr;

    if (controllable_poly || forcecontrol) {
        buf[0] = '\0';
        tryct = 5;

        do {
            mntmp = NON_PM;
            getlin("变成哪种怪物? [输入名称]", buf);
            (void) mungspaces(buf);
            if (*buf == '\033') {
                /* user is cancelling controlled poly */
                if (forcecontrol) { /* wizard mode #polyself */
                    pline1(Never_mind);
                    return;
                }
                Strcpy(buf, "*"); /* resort to random */
            }
            if (!strcmp(buf, "*") || !strcmp(buf, "random")) {
                /* explicitly requesting random result */
                tryct = 0; /* will skip thats_enough_tries */
                continue;  /* end do-while(--tryct > 0) loop */
            }
            class = 0;
            mntmp = name_to_mon(buf, &gvariant);
            if (mntmp < LOW_PM) {
 by_class:
                class = name_to_monclass(buf, &mntmp);
                if (class && mntmp == NON_PM)
                    mntmp = (draconian && class == S_DRAGON)
                            ? armor_to_dragon(uarm->otyp)
                            : mkclass_poly(class);

            /* placeholder monsters are for corpses and all flagged
               M2_NOPOLY but they are reasonable polymorph targets;
               pick a suitable substitute (which might be geno'd) */
            } else if (is_placeholder(&mons[mntmp])
                       /* when your own race, fall to !polyok() case */
                       && !your_race(&mons[mntmp])
                       /* same for generic human, even if hero isn't human */
                       && mntmp != PM_HUMAN) {
                /* far less general than mkclass() */
                if (mntmp == PM_ORC)
                    mntmp = rn2(3) ? PM_HILL_ORC : PM_MORDOR_ORC;
                else if (mntmp == PM_ELF)
                    mntmp = rn2(3) ? PM_GREEN_ELF : PM_GREY_ELF;
                else if (mntmp == PM_GIANT)
                    mntmp = rn2(3) ? PM_STONE_GIANT : PM_HILL_GIANT;
                /* note: PM_DWARF and PM_GNOME are ordinary monsters and
                   no longer flagged no-poly so have no need for placeholder
                   handling; PM_HUMAN is a placeholder without a suitable
                   substitute so gets handled differently below */
            }

            if (mntmp < LOW_PM) {
                if (!class)
                    pline("我从未听说过这种怪物.");
                else
                    You_cant("变形为这些中的任何一个.");
            } else if (wizard && Upolyd
                       && (mntmp == u.umonster
                           /* "priest" and "priestess" match the monster
                              rather than the role; override that unless
                              the text explicitly contains "aligned" */
                           || (u.umonster == PM_CLERIC
                               && mntmp == PM_ALIGNED_CLERIC
                               && !strstri(buf, "aligned")))) { /*危险:我不知道怎么改*/
                /* in wizard mode, picking own role while poly'd reverts to
                   normal without newman()'s chance of level or sex change */
                rehumanize();
                old_light = 0; /* rehumanize() extinguishes u-as-mon light */
                goto made_change;
            } else if (iswere && (were_beastie(mntmp) == u.ulycn
                                  || mntmp == counter_were(u.ulycn)
                                  || (Upolyd && mntmp == PM_HUMAN))) {
                goto do_shift;
            } else if (!polyok(&mons[mntmp])
                       /* Note:  humans are illegal as monsters, but an
                          illegal monster forces newman(), which is what
                          we want if they specified a human.... (unless
                          they specified a unique monster) */
                       && !(mntmp == PM_HUMAN
                            || (your_race(&mons[mntmp])
                                && (mons[mntmp].geno & G_UNIQ) == 0)
                            || mntmp == gu.urole.mnum)) {
                const char *pm_name;

                /* mkclass_poly() can pick a !polyok()
                   candidate; if so, usually try again */
                if (class) {
                    if (rn2(3) || --tryct > 0)
                        goto by_class;
                    /* no retries left; put one back on counter
                       so that end of loop decrement will yield
                       0 and trigger thats_enough_tries message */
                    ++tryct;
                }
                pm_name = pmname(&mons[mntmp], flags.female ? FEMALE : MALE);
                if (the_unique_pm(&mons[mntmp]))
                    pm_name = the(pm_name);
                else if (!type_is_pname(&mons[mntmp]))
                    pm_name = an(pm_name);
                You_cant("变形为%s.", pm_name);
            } else
                break;
        } while (--tryct > 0);

        if (!tryct)
            pline1(thats_enough_tries);
        /* allow skin merging, even when polymorph is controlled */
        if (draconian && (tryct <= 0 || mntmp == armor_to_dragon(uarm->otyp)))
            goto do_merge;
        if (isvamp && (tryct <= 0 || mntmp == PM_WOLF || mntmp == PM_FOG_CLOUD
                       || is_bat(&mons[mntmp])))
            goto do_vampyr;
    } else if (draconian || iswere || isvamp) {
        /* special changes that don't require polyok() */
        if (draconian) {
 do_merge:
            mntmp = armor_to_dragon(uarm->otyp);
            if (!(svm.mvitals[mntmp].mvflags & G_GENOD)) {
                unsigned was_lit = uarm->lamplit;
                int arm_light = artifact_light(uarm) ? arti_light_radius(uarm)
                                                     : 0;

                /* allow G_EXTINCT */
                if (Is_dragon_scales(uarm)) {
                    /* dragon scales remain intact as uskin */
                    You("和你的鳞甲融合了.");
                } else { /* dragon scale mail reverts to scales */
                    /* similar to noarmor(invent.c),
                       shorten to "<color> scale mail" */
                    Strcpy(buf, simpleonames(uarm));
                    strsubst(buf, "龙", ""); /*危险:strsubst(buf, " dragon ", " ");*/
                    /* tricky phrasing; dragon scale mail is singular, dragon
                       scales are plural (note: we don't use "set of scales",
                       which usually overrides the distinction, here) */
                    Your("在融合的时候,%s恢复成了鳞!", buf);
                    /* uarm->spe enchantment remains unchanged;
                       re-converting scales to mail poses risk
                       of evaporation due to over enchanting */
                    uarm->otyp += GRAY_DRAGON_SCALES - GRAY_DRAGON_SCALE_MAIL;
                    observe_object(uarm);
                    disp.botl = TRUE; /* AC is changing */
                }
                uskin = uarm;
                uarm = (struct obj *) 0;
                /* save/restore hack */
                uskin->owornmask |= I_SPECIAL;
                if (was_lit)
                    maybe_adjust_light(uskin, arm_light);
                update_inventory();
            }
        } else if (iswere) {
 do_shift:
            if (Upolyd && were_beastie(mntmp) != u.ulycn)
                mntmp = PM_HUMAN; /* Illegal; force newman() */
            else
                mntmp = u.ulycn;
        } else if (isvamp) {
 do_vampyr:
            if (mntmp < LOW_PM || (mons[mntmp].geno & G_UNIQ)) {
                mntmp = (gy.youmonst.data == &mons[PM_VAMPIRE_LEADER]
                         && !rn2(10)) ? PM_WOLF
                                      : !rn2(4) ? PM_FOG_CLOUD
                                                : PM_VAMPIRE_BAT;
                if (ismnum(gy.youmonst.cham)
                    && !is_vampire(gy.youmonst.data) && !rn2(2))
                    mntmp = gy.youmonst.cham;
            }
            if (controllable_poly) {
                Sprintf(buf, "变成%s?",
                        an(pmname(&mons[mntmp], gvariant)));
                if (y_n(buf) != 'y')
                    return;
            }
        }
        /* if polymon fails, "you feel" message has been given
           so don't follow up with another polymon or newman;
           sex_change_ok left disabled here */
        if (mntmp == PM_HUMAN)
            newman(); /* werecritter */
        else
            (void) polymon(mntmp);
        goto made_change; /* maybe not, but this is right anyway */
    }

    if (mntmp < LOW_PM) {
        tryct = 200;
        do {
            /* randomly pick an "ordinary" monster */
            mntmp = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
            if (polyok(&mons[mntmp]) && !is_placeholder(&mons[mntmp]))
                break;
        } while (--tryct > 0);
    }

    /* The below polyok() fails either if everything is genocided, or if
     * we deliberately chose something illegal to force newman().
     */
    gs.sex_change_ok++;
    if (!polyok(&mons[mntmp]) || (!forcecontrol && !rn2(5))
        || your_race(&mons[mntmp])) {
        newman();
    } else {
        (void) polymon(mntmp);
    }
    gs.sex_change_ok--; /* reset */

 made_change:
    new_light = emits_light(gy.youmonst.data);
    if (old_light != new_light) {
        if (old_light)
            del_light_source(LS_MONSTER, monst_to_any(&gy.youmonst));
        if (new_light == 1)
            ++new_light; /* otherwise it's undetectable */
        if (new_light)
            new_light_source(u.ux, u.uy, new_light, LS_MONSTER,
                             monst_to_any(&gy.youmonst));
    }
}

/* (try to) make a mntmp monster out of the player; return 1 if successful */
int
polymon(int mntmp)
{
    char buf[BUFSZ], ustuckNam[BUFSZ];
    boolean sticking = sticks(gy.youmonst.data) && u.ustuck && !u.uswallow,
            was_blind = !!Blind, dochange = FALSE, was_expelled = FALSE,
            was_hiding_under = u.uundetected && hides_under(gy.youmonst.data);
    int mlvl, newMaxStr;

    if (svm.mvitals[mntmp].mvflags & G_GENOD) { /* allow G_EXTINCT */
        You_feel("相当地%s化.",
                 pmname(&mons[mntmp], flags.female ? FEMALE : MALE));
        exercise(A_WIS, TRUE);
        return 0;
    }

    /* KMH, conduct */
    if (!u.uconduct.polyselfs++)
        livelog_printf(LL_CONDUCT,
                       "第一次变形,变成了%s",
                       an(pmname(&mons[mntmp], flags.female ? FEMALE : MALE)));

    /* exercise used to be at the very end but only Wis was affected
       there since the polymorph was always in effect by then */
    exercise(A_CON, FALSE);
    exercise(A_WIS, TRUE);

    if (!Upolyd) {
        /* Human to monster; save human stats */
        u.macurr = u.acurr;
        u.mamax = u.amax;
        u.mfemale = flags.female;
    } else {
        /* Monster to monster; restore human stats, to be
         * immediately changed to provide stats for the new monster
         */
        u.acurr = u.macurr;
        u.amax = u.mamax;
        flags.female = u.mfemale;
    }

    /* if stuck mimicking gold, stop immediately */
    if (gm.multi < 0 && U_AP_TYPE == M_AP_OBJECT
        && gy.youmonst.data->mlet != S_MIMIC)
        unmul("");
    /* if becoming a non-mimic, stop mimicking anything */
    if (mons[mntmp].mlet != S_MIMIC) {
        /* as in polyman() */
        gy.youmonst.m_ap_type = M_AP_NOTHING;
        gy.youmonst.mappearance = 0;
    }
    if (is_male(&mons[mntmp])) {
        if (flags.female)
            dochange = TRUE;
    } else if (is_female(&mons[mntmp])) {
        if (!flags.female)
            dochange = TRUE;
    } else if (!is_neuter(&mons[mntmp]) && mntmp != u.ulycn) {
        if (gs.sex_change_ok && !rn2(10))
            dochange = TRUE;
    }

    Strcpy(ustuckNam, u.ustuck ? Some_Monnam(u.ustuck) : "");

    Strcpy(buf, (u.umonnum != mntmp) ? "" : "新");
    if (dochange) {
        flags.female = !flags.female;
        Strcat(buf, (is_male(&mons[mntmp]) || is_female(&mons[mntmp]))
                       ? "" : flags.female ? "女的" : "男的");
    }
    Strcat(buf, pmname(&mons[mntmp], flags.female ? FEMALE : MALE));
    You("%s%s!", (u.umonnum != mntmp) ? "变成了" : "感觉像是", an(buf));

    if (Stoned && poly_when_stoned(&mons[mntmp])) {
        /* poly_when_stoned already checked stone golem genocide */
        mntmp = PM_STONE_GOLEM;
        make_stoned(0L, "你变成了石头!", 0, (char *) 0);
    }

    u.mtimedone = rn1(500, 500);
    u.umonnum = mntmp;
    set_uasmon();

    /* New stats for monster, to last only as long as polymorphed.
     * Currently only strength gets changed.
     */
    newMaxStr = uasmon_maxStr();
    if (strongmonst(&mons[mntmp])) {
        ABASE(A_STR) = AMAX(A_STR) = (schar) newMaxStr;
    } else {
        /* not a strongmonst(); if hero has exceptional strength, remove it
           (note: removal is temporary until returning to original form);
           we don't attempt to enforce lower maximum for wimpy forms;
           unlike for strongmonst, current strength does not get set to max */
        AMAX(A_STR) = (schar) newMaxStr;
        /* make sure current is not higher than max (strip exceptional Str) */
        if (ABASE(A_STR) > AMAX(A_STR))
            ABASE(A_STR) = AMAX(A_STR);
    }

    if (Stone_resistance && Stoned) { /* parnes@eniac.seas.upenn.edu */
        make_stoned(0L, "你不再怕石化了.", 0,
                    (char *) 0);
    }
    if (Sick_resistance && Sick) {
        make_sick(0L, (char *) 0, FALSE, SICK_ALL);
        You("不再感到生病.");
    }
    if (Slimed) {
        if (flaming(gy.youmonst.data)) {
            make_slimed(0L, "黏液烧光了!");
        } else if (mntmp == PM_GREEN_SLIME) {
            /* do it silently */
            make_slimed(0L, (char *) 0);
        }
    }
    check_strangling(FALSE); /* maybe stop strangling */
    if (nohands(gy.youmonst.data))
        make_glib(0);

    /*
    mlvl = adj_lev(&mons[mntmp]);
     * We can't do the above, since there's no such thing as an
     * "experience level of you as a monster" for a polymorphed character.
     */
    mlvl = (int) mons[mntmp].mlevel;
    if (gy.youmonst.data->mlet == S_DRAGON && mntmp >= PM_GRAY_DRAGON) {
        u.mhmax = In_endgame(&u.uz) ? (8 * mlvl) : (4 * mlvl + d(mlvl, 4));
    } else if (is_golem(gy.youmonst.data)) {
        u.mhmax = golemhp(mntmp);
    } else {
        if (!mlvl)
            u.mhmax = rnd(4);
        else
            u.mhmax = d(mlvl, 8);
        if (is_home_elemental(&mons[mntmp]))
            u.mhmax *= 3;
    }
    u.mh = u.mhmax;

    if (u.ulevel < mlvl) {
        /* Low level characters can't become high level monsters for long */
#ifdef DUMB
        /* DRS/NS 2.2.6 messes up -- Peter Kendell */
        int mtd = u.mtimedone, ulv = u.ulevel;

        u.mtimedone = mtd * ulv / mlvl;
#else
        u.mtimedone = u.mtimedone * u.ulevel / mlvl;
#endif
    }

    if (uskin && mntmp != armor_to_dragon(uskin->otyp))
        skinback(FALSE);
    break_armor();
    drop_weapon(1);
    find_ac(); /* (repeated below) */
    /* if hiding under something and can't hide anymore, unhide now;
       but don't auto-hide when not already hiding-under */
    if (was_hiding_under)
        (void) hideunder(&gy.youmonst);

    if (u.utrap && u.utraptype == TT_PIT) {
        set_utrap(rn1(6, 2), TT_PIT); /* time to escape resets */
    }
    if (was_blind && !Blind) { /* previous form was eyeless */
        set_itimeout(&HBlinded, 1L);
        make_blinded(0L, TRUE); /* remove blindness */
    }
    newsym(u.ux, u.uy); /* Change symbol */

    /* you now know what an egg of your type looks like; [moved from
       below in case expels() -> spoteffects() drops hero onto any eggs] */
    if (lays_eggs(gy.youmonst.data)) {
        learn_egg_type(u.umonnum);
        /* make queen bees recognize killer bee eggs */
        learn_egg_type(egg_type_from_parent(u.umonnum, TRUE));
    }

    if (u.uswallow) {
        uchar usiz;

        /* if new form can't be swallowed, make engulfer expel hero */
        if (unsolid(gy.youmonst.data)
            /* subset of engulf_target() */
            || (usiz = gy.youmonst.data->msize) >= MZ_HUGE
            || (u.ustuck->data->msize < usiz && !is_whirly(u.ustuck->data))) {
            boolean expels_mesg = TRUE;

            if (unsolid(gy.youmonst.data)) {
                if (canspotmon(u.ustuck)) /* [see below for explanation] */
                    Strcpy(ustuckNam, Monnam(u.ustuck));
                pline("%s再也无法容纳你了.", ustuckNam);
                expels_mesg = FALSE;
            }
            expels(u.ustuck, u.ustuck->data, expels_mesg);
            was_expelled = TRUE;
            /* FIXME? if expels() triggered rehumanize then we should
               return early */
        }

    /* [note:  this 'sticking' handling is only sufficient for changing from
       grabber to engulfer or vice versa because engulfing by poly'd hero
       always ends immediately so won't be in effect during a polymorph] */
    } else if (u.ustuck && !sticking /* && !u.uswallow */
               /* being held; if now capable of holding, make holder
                  release so that hero doesn't automagically start holding
                  it; or, release if no longer capable of being held */
               && (sticks(gy.youmonst.data) || unsolid(gy.youmonst.data))) {
        /* u.ustuck name was saved above in case we're changing from can-see
           to can't-see; but might have changed from can't-see to can-see so
           override here if hero knows who u.ustuck is */
        if (canspotmon(u.ustuck))
            Strcpy(ustuckNam, Monnam(u.ustuck));
        set_ustuck((struct monst *) 0);
        pline("%s松开了对你的抓握.", ustuckNam);
    } else if (sticking && !sticks(gy.youmonst.data)) {
        /* was holding onto u.ustuck but no longer capable of that */
        uunstick();
    }

    if (u.usteed) {
        if (touch_petrifies(u.usteed->data) && !Stone_resistance && rnl(3)) {
            pline("%s碰到了%s.", no_longer_petrify_resistant,
                  mon_nam(u.usteed));
            Sprintf(buf, "骑乘%s",
                    an(pmname(u.usteed->data, Mgender(u.usteed))));
            instapetrify(buf);
        }
        if (!can_ride(u.usteed))
            dismount_steed(DISMOUNT_POLY);
    }

    find_ac();
    if (((!Levitation && !u.ustuck && !Flying && is_pool_or_lava(u.ux, u.uy))
         || (Underwater && !Swimming))
        /* if expelled above, expels() already called spoteffects() */
        && !was_expelled) {
        spoteffects(TRUE);
        /* FIXME? if spoteffects() triggered rehumanize then we should
           return early */
    }
    if (Passes_walls && u.utrap
        && (u.utraptype == TT_INFLOOR || u.utraptype == TT_BURIEDBALL)) {
        if (u.utraptype == TT_INFLOOR) {
            pline_The("岩石似乎不再困住你了.");
        } else {
            pline_The("掩埋的球不再束缚你了.");
            buried_ball_to_freedom();
        }
        reset_utrap(TRUE);
    } else if (likes_lava(gy.youmonst.data) && u.utrap
               && u.utraptype == TT_LAVA) {
        pline_The("%s感觉变得舒缓了.", hliquid("熔岩"));
        reset_utrap(TRUE);
    }
    if (amorphous(gy.youmonst.data) || is_whirly(gy.youmonst.data)
        || unsolid(gy.youmonst.data)) {
        if (Punished) {
            You("滑脱出了铁链.");
            unpunish();
        } else if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            You("滑脱出了掩埋的球和链.");
            buried_ball_to_freedom();
        }
    }
    if (u.utrap && (u.utraptype == TT_WEB || u.utraptype == TT_BEARTRAP)
        && (amorphous(gy.youmonst.data) || is_whirly(gy.youmonst.data)
            || unsolid(gy.youmonst.data)
            || (gy.youmonst.data->msize <= MZ_SMALL
                && u.utraptype == TT_BEARTRAP))) {
        You("不再卡在%s中.",
            u.utraptype == TT_WEB ? "蜘蛛网" : "捕兽夹");
        /* probably should burn webs too if PM_FIRE_ELEMENTAL */
        reset_utrap(TRUE);
    }
    if (webmaker(gy.youmonst.data) && u.utrap && u.utraptype == TT_WEB) {
        You("适应了网.");
        reset_utrap(TRUE);
    }
    check_strangling(TRUE); /* maybe start strangling */

    disp.botl = TRUE;
    gv.vision_full_recalc = 1;
    see_monsters();
    encumber_msg();

    retouch_equipment(2);
    /* this might trigger a recursive call to polymon() [stone golem
       wielding cockatrice corpse and hit by stone-to-flesh, becomes
       flesh golem above, now gets transformed back into stone golem;
       fortunately neither form uses #monster] */
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);

    /* the explanation of '#monster' used to be shown sooner, but there are
       possible fatalities above and it isn't useful unless hero survives */
    if (flags.verbose) {
        static const char use_thec[] = "使用#%s以%s.";
        static const char monsterc[] = "monster";
        struct permonst *uptr = gy.youmonst.data;
        boolean might_hide = (is_hider(uptr) || hides_under(uptr));

        if (can_breathe(uptr))
            pline(use_thec, monsterc, "使用你的呼气攻击");
        if (attacktype(uptr, AT_SPIT))
            pline(use_thec, monsterc, "吐出毒液");
        if (uptr->mlet == S_NYMPH)
            pline(use_thec, monsterc, "移除铁球");
        if (attacktype(uptr, AT_GAZE))
            pline(use_thec, monsterc, "注视怪物");
        if (might_hide && webmaker(uptr))
            pline(use_thec, monsterc, "藏在网里,或者织网");
        else if (might_hide)
            pline(use_thec, monsterc, "躲藏");
        else if (webmaker(uptr))
            pline(use_thec, monsterc, "织网");
        if (is_were(uptr))
            pline(use_thec, monsterc, "召唤同伴帮助");
        if (u.umonnum == PM_GREMLIN)
            pline(use_thec, monsterc, "在泉水里分裂");
        if (is_unicorn(uptr))
            pline(use_thec, monsterc, "用你的号攻击");
        if (is_mind_flayer(uptr))
            pline(use_thec, monsterc, "释放精神冲击波");
        if (uptr->msound == MS_SHRIEK) /* worthless, actually */
            pline(use_thec, monsterc, "尖叫");
        if (is_vampire(uptr) || is_vampshifter(&gy.youmonst))
            pline(use_thec, monsterc, "改变形态");

        if (lays_eggs(uptr) && flags.female
            && !(uptr == &mons[PM_GIANT_EEL]
                 || uptr == &mons[PM_ELECTRIC_EEL]))
            pline(use_thec, "sit",
                  eggs_in_water(uptr) ? "在水中繁殖" : "下蛋");
    }
    return 1;
}

/* determine hero's temporary strength value used while polymorphed;
   hero poly'd into M2_STRONG monster usually gets 18/100 strength but
   there are exceptions; non-M2_STRONG get maximum strength set to 18 */
schar
uasmon_maxStr(void)
{
    const struct Race *R;
    int newMaxStr;
    int mndx = u.umonnum;
    struct permonst *ptr = &mons[mndx];

    if (is_orc(ptr)) {
        if (mndx != PM_URUK_HAI && mndx != PM_ORC_CAPTAIN)
            mndx = PM_ORC;
    } else if (is_elf(ptr)) {
        mndx = PM_ELF;
    } else if (is_dwarf(ptr)) {
        mndx = PM_DWARF;
    } else if (is_gnome(ptr)) {
        mndx = PM_GNOME;
#if 0   /* use the mons[] value for humans */
    } else if (is_human(ptr)) {
        mndx = PM_HUMAN;
#endif
    }
    R = character_race(mndx);

    if (strongmonst(ptr)) {
        /* ettins, titans and minotaurs don't pass the is_giant() test;
           giant mummies and giant zombies do but we throttle those */
        boolean live_H = is_giant(ptr) && !is_undead(ptr);

        /* hero orcs are limited to 18/50 for maximum strength, so treat
           hero poly'd into an orc the same; goblins, orc shamans, and orc
           zombies don't have strongmonst() attribute so won't get here;
           hobgoblins and orc mummies do get here and are limited to 18/50
           like normal orcs; however, orc captains and Uruk-hai retain 18/100
           strength; hero gnomes are also limited to 18/50; hero elves are
           limited to 18/00 regardless of whether they're strongmonst, but
           the two strongmonst types (monarchs and nobles) have current
           strength set to 18 [by polymon()], the others don't */
        newMaxStr = R ? R->attrmax[A_STR] : live_H ? STR19(19) : STR18(100);
    } else {
        newMaxStr = R ? R->attrmax[A_STR] : 18; /* 18 is same as STR18(0) */
    }
    return (schar) newMaxStr;
}

/* dropx() jacket for break_armor() */
staticfn void
dropp(struct obj *obj)
{
    struct obj *otmp;

    /*
     * Dropping worn armor while polymorphing might put hero into water
     * (loss of levitation boots or water walking boots that the new
     * form can't wear), where emergency_disrobe() could remove it from
     * inventory.  Without this, dropx() could trigger an 'object lost'
     * panic.  Right now, boots are the only armor which might encounter
     * this situation, but handle it for all armor.
     *
     * Hypothetically, 'obj' could have merged with something (not
     * applicable for armor) and no longer be a valid pointer, so scan
     * inventory for it instead of trusting obj->where.
     */
    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        if (otmp == obj) {
            dropx(obj);
            /* Note that otmp->nobj is pointing at fobj now,
             * as a result of:
             * dropx() -> dropy() -> dropz() -> place_object(),
             * and no longer pointing at the next obj in inventory.
             * That would be an issue if this loop were allowed
             * to continue, but the break statement that
             * follows prevents the loop from continuing on with
             * objects on the floor.
             */
            break;
        }
    }
}

staticfn void
break_armor(void)
{
    struct obj *otmp;
    struct permonst *uptr = gy.youmonst.data;

    if (breakarm(uptr)) {
        if ((otmp = uarm) != 0) {
            if (donning(otmp))
                cancel_don();
            /* for gold DSM, we don't want Armor_gone() to report that it
               stops shining _after_ we've been told that it is destroyed */
            if (otmp->lamplit)
                end_burn(otmp, FALSE);

            You("把你的铠甲挤破了!");
            exercise(A_STR, FALSE);
            (void) Armor_gone();
            useup(otmp);
        }
        if ((otmp = uarmc) != 0
            /* mummy wrapping adapts to small and very big sizes */
            && (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(uptr))) {
            if (otmp->otyp == MUMMY_WRAPPING) {
                /* doesn't have a clasp to break open */
                Your("%s被撕裂了!", cloak_simple_name(otmp));
                (void) Cloak_off();
                useup(otmp);
            } else if (otmp->otyp == ALCHEMY_SMOCK) {
                pline_The("你%s上的结被拉开了!", cloak_simple_name(otmp));
                (void) Cloak_off();
                dropp(otmp);
            } else {
                pline_The("你%s上的搭扣断裂开了!", cloak_simple_name(otmp));
                (void) Cloak_off();
                dropp(otmp);
            }
        }
        if (uarmu) {
            Your("衬衫被撕成了碎片!");
            useup(uarmu);
        }
    } else if (sliparm(uptr)) {
        if ((otmp = uarm) != 0 && racial_exception(&gy.youmonst, otmp) < 1) {
            if (donning(otmp))
                cancel_don();
            Your("盔甲掉在你的旁边!");
            /* [note: _gone() instead of _off() dates to when life-saving
               could force fire resisting armor back on if hero burned in
               hell (3.0, predating Gehennom); the armor isn't actually
               gone here but also isn't available to be put back on] */
            (void) Armor_gone();
            dropp(otmp);
        }
        if ((otmp = uarmc) != 0
            /* mummy wrapping adapts to small and very big sizes */
            && (otmp->otyp != MUMMY_WRAPPING || !WrappingAllowed(uptr))) {
            if (is_whirly(uptr))
                Your("%s失去肉体支撑,掉到了地上!", cloak_simple_name(otmp));
            else
                You("的体型太小,从你的%s中缩出!", cloak_simple_name(otmp));
            (void) Cloak_off();
            dropp(otmp);
        }
        if ((otmp = uarmu) != 0) {
            if (is_whirly(uptr))
                You("渗出了你的衬衫!");
            else
                You("变得太小,从你的衬衫中缩出!");
            setworn((struct obj *) 0, otmp->owornmask & W_ARMU);
            dropp(otmp);
        }
    }
    if (has_horns(uptr)) {
        if ((otmp = uarmh) != 0) {
            if (is_flimsy(otmp) && !donning(otmp)) {
                char hornbuf[BUFSZ];

                /* Future possibilities: This could damage/destroy helmet */
                Sprintf(hornbuf, "角%s", plur(num_horns(uptr)));
                Your("%s%s过了%s.", hornbuf, vtense(hornbuf, "钻"),
                     yname(otmp));
            } else {
                if (donning(otmp))
                    cancel_don();
                Your("%s掉到了%s上!", helm_simple_name(otmp),
                     surface(u.ux, u.uy));
                (void) Helmet_off();
                dropp(otmp);
            }
        }
    }
    if (nohands(uptr) || verysmall(uptr)) {
        if ((otmp = uarmg) != 0) {
            if (donning(otmp))
                cancel_don();
            /* Drop weapon along with gloves */
            You("掉下了你的手套%s!", uwep ? "和武器" : "");
            drop_weapon(0);
            (void) Gloves_off();
            /* Glib manipulation (ends immediately) handled by Gloves_off */
            dropp(otmp);
        }
        if ((otmp = uarms) != 0) {
            You("没法用手拿你的盾牌了!");
            (void) Shield_off();
            dropp(otmp);
        }
        if ((otmp = uarmh) != 0) {
            if (donning(otmp))
                cancel_don();
            Your("%s掉到了%s上!", helm_simple_name(otmp),
                 surface(u.ux, u.uy));
            (void) Helmet_off();
            dropp(otmp);
        }
    }
    if (nohands(uptr) || verysmall(uptr)
        || slithy(uptr) || uptr->mlet == S_CENTAUR) {
        if ((otmp = uarmf) != 0) {
            if (donning(otmp))
                cancel_don();
            if (is_whirly(uptr))
                Your("靴子掉了下来!");
            else
                Your("靴子从%s的双脚上%s!",
                     verysmall(uptr) ? "滑出" : "被蹬开");
            (void) Boots_off();
            dropp(otmp);
        }
    }
    /* not armor, but eyewear shouldn't stay worn without a head to wear
       it/them on (should also come off if head is too tiny or too huge,
       but putting accessories on doesn't reject those cases [yet?]);
       amulet stays worn */
    if ((otmp = ublindf) != 0 && !has_head(uptr)) {
        int l;
        const char *eyewear = simpleonames(otmp); /* blindfold|towel|lenses */

        if (!strncmp(eyewear, "pair of ", l = 8)) /* lenses */
            eyewear += l;
        Your("%s%s了!", eyewear, vtense(eyewear, "掉落"));
        (void) Blindf_off((struct obj *) 0); /* Null: skip usual off mesg */
        dropp(otmp);
    }
    /* rings stay worn even when no hands */
}

staticfn void
drop_weapon(int alone)
{
    struct obj *otmp;
    const char *what, *which, *whichtoo;
    boolean candropwep, candropswapwep, updateinv = TRUE;

    if (uwep) {
        /* !alone check below is currently superfluous but in the
         * future it might not be so if there are monsters which cannot
         * wear gloves but can wield weapons
         */
        if (!alone || cantwield(gy.youmonst.data)) {
            candropwep = canletgo(uwep, "");
            candropswapwep = !u.twoweap || canletgo(uswapwep, "");
            if (alone) {
                what = (candropwep && candropswapwep) ? "放下" : "放下";
                which = is_sword(uwep) ? "剑" : weapon_descr(uwep);
                if (u.twoweap) {
                    whichtoo =
                        is_sword(uswapwep) ? "剑" : weapon_descr(uswapwep);
                    if (strcmp(which, whichtoo))
                        which = "武器";
                }
                if (uwep->quan != 1L || u.twoweap)
                    which = makeplural(which);

                You("发现你必须%s%s%s!", what,
                    the_your[!!strncmp(which, "尸体", 6)], which);
            }
            /* if either uwep or wielded uswapwep is flagged as 'in_use'
               then don't drop it or explicitly update inventory; leave
               those actions to caller (or caller's caller, &c) */
            if (u.twoweap) {
                otmp = uswapwep;
                uswapwepgone();
                if (otmp->in_use)
                    updateinv = FALSE;
                else if (candropswapwep)
                    dropx(otmp);
            }
            otmp = uwep;
            uwepgone();
            if (otmp->in_use)
                updateinv = FALSE;
            else if (candropwep)
                dropx(otmp);
            /* [note: dropp vs dropx -- if heart of ahriman is wielded, we
               might be losing levitation by dropping it; but that won't
               happen until the drop, unlike Boots_off() dumping hero into
               water and triggering emergency_disrobe() before dropx()] */

            if (updateinv)
                update_inventory();
        } else if (!could_twoweap(gy.youmonst.data)) {
            untwoweapon();
        }
    }
}

/* return to original form, usually either due to polymorph timing out
   or dying from loss of hit points while being polymorphed */
void
rehumanize(void)
{
    boolean was_flying = (Flying != 0);

    /* You can't revert back while unchanging */
    if (Unchanging) {
        if (u.mh < 1) {
            svk.killer.format = NO_KILLER_PREFIX;
            Strcpy(svk.killer.name, "卡在变形中时被杀死");
            done(DIED);
            /* can get to here if declining to die in explore or wizard
               mode; since we're wearing an amulet of unchanging we can't
               be wearing an amulet of life-saving */
            return; /* don't rehumanize after all */
        } else if (uamul && uamul->otyp == AMULET_OF_UNCHANGING) {
            Your("%s%s了!", simpleonames(uamul), otense(uamul, "失效"));
            observe_object(uamul);
            makeknown(AMULET_OF_UNCHANGING);
        }
    }

    /*
     * Right now, dying while being a shifted vampire (bat, cloud, wolf)
     * reverts to human rather than to vampire.
     */

    if (emits_light(gy.youmonst.data))
        del_light_source(LS_MONSTER, monst_to_any(&gy.youmonst));
    polyman("你变回了%s形态!", gu.urace.adj);

    if (u.uhp < 1) {
        /* can only happen if some bit of code reduces u.uhp
           instead of u.mh while poly'd */
        Your("旧形态不够健康,难以存活.");
        Sprintf(svk.killer.name, "恢复到不健康的%s形态",
                gu.urace.adj);
        svk.killer.format = KILLED_BY;
        done(DIED);
    }
    nomul(0);

    disp.botl = TRUE;
    gv.vision_full_recalc = 1;
    encumber_msg();
    update_inventory();
    if (was_flying && !Flying && u.usteed)
        You("和%s轻轻地回到%s.",
            mon_nam(u.usteed), surface(u.ux, u.uy));
    retouch_equipment(2);
    if (!uarmg)
        selftouch(no_longer_petrify_resistant);
}

int
dobreathe(void)
{
    struct attack *mattk;

    if (Strangled) {
        You_cant("呼吸. 抱歉.");
        return ECMD_OK;
    }
    if (u.uen < 15) {
        You("没有足够的能量来呼吸!");
        return ECMD_OK;
    }
    u.uen -= 15;
    disp.botl = TRUE;

    if (!getdir((char *) 0))
        return ECMD_CANCEL;

    mattk = attacktype_fordmg(gy.youmonst.data, AT_BREA, AD_ANY);
    if (!mattk)
        impossible("bad breath attack?"); /* mouthwash needed... */
    else if (!u.dx && !u.dy && !u.dz)
        ubreatheu(mattk);
    else
        ubuzz(BZ_U_BREATH(BZ_OFS_AD(mattk->adtyp)), (int) mattk->damn);
    return ECMD_TIME;
}

int
dospit(void)
{
    struct obj *otmp;
    struct attack *mattk;

    if (!getdir((char *) 0))
        return ECMD_CANCEL;
    mattk = attacktype_fordmg(gy.youmonst.data, AT_SPIT, AD_ANY);
    if (!mattk) {
        impossible("bad spit attack?");
    } else {
        switch (mattk->adtyp) {
        case AD_BLND:
        case AD_DRST:
            otmp = mksobj(BLINDING_VENOM, TRUE, FALSE);
            break;
        default:
            impossible("bad attack type in dospit");
            FALLTHROUGH;
            /*FALLTHRU*/
        case AD_ACID:
            otmp = mksobj(ACID_VENOM, TRUE, FALSE);
            break;
        }
        otmp->spe = 1; /* to indicate it's yours */
        throwit(otmp, 0L, FALSE, (struct obj *) 0);
    }
    return ECMD_TIME;
}

int
doremove(void)
{
    if (!Punished) {
        if (u.utrap && u.utraptype == TT_BURIEDBALL) {
            pline_The("铁球和铁链被坚固地埋在%s里.",
                      surface(u.ux, u.uy));
            return ECMD_OK;
        }
        You("没有被拴在任何东西上!");
        return ECMD_OK;
    }
    unpunish();
    return ECMD_TIME;
}

int
dospinweb(void)
{
    coordxy x = u.ux, y = u.uy;
    struct trap *ttmp = t_at(x, y);
    /* disallow webs on water, lava, air & cloud */
    boolean reject_terrain = is_pool_or_lava(x, y) || IS_AIR(levl[x][y].typ);

    /* [at the time this was written, it was not possible to be both a
       webmaker and a flyer, but with the advent of amulet of flying that
       became a possibility; at present hero can spin a web while flying] */
    if (Levitation || reject_terrain) {
        You("必须在%s地面上织网.",
            reject_terrain ? "坚实的" : "");
        return ECMD_OK;
    }
    if (u.uswallow) {
        You("在%s里释放出丝浆.", mon_nam(u.ustuck));
        if (is_animal(u.ustuck->data)) {
            expels(u.ustuck, u.ustuck->data, TRUE);
            return ECMD_OK;
        }
        if (is_whirly(u.ustuck->data)) {
            int i;

            for (i = 0; i < NATTK; i++)
                if (u.ustuck->data->mattk[i].aatyp == AT_ENGL)
                    break;
            if (i == NATTK)
                impossible("Swallower has no engulfing attack?");
            else {
                char sweep[30];

                sweep[0] = '\0';
                switch (u.ustuck->data->mattk[i].adtyp) {
                case AD_FIRE:
                    Strcpy(sweep, "烧掉");
                    break;
                case AD_ELEC:
                    Strcpy(sweep, "电击掉");
                    break;
                case AD_COLD:
                    Strcpy(sweep, "冻裂掉");
                    break;
                }
                pline_The("网被%s了!", sweep);
            }
            return ECMD_OK;
        } /* default: a nasty jelly-like creature */
        pline_The("网被%s溶解了.", mon_nam(u.ustuck));
        return ECMD_OK;
    }
    if (u.utrap) {
        You("不能在卡在陷阱里时织网.");
        return ECMD_OK;
    }
    exercise(A_DEX, TRUE);
    if (ttmp) {
        switch (ttmp->ttyp) {
        case PIT:
        case SPIKED_PIT:
            You("织了一张网,盖住了坑.");
            deltrap(ttmp);
            bury_objs(x, y);
            newsym(x, y);
            return ECMD_TIME;
        case SQKY_BOARD:
            pline_The("嘎吱作响的板子被卡住了.");
            deltrap(ttmp);
            newsym(x, y);
            return ECMD_TIME;
        case TELEP_TRAP:
        case LEVEL_TELEP:
        case MAGIC_PORTAL:
        case VIBRATING_SQUARE:
            Your("网消失了!");
            return ECMD_OK;
        case WEB:
            You("让网变得更厚.");
            return ECMD_TIME;
        case HOLE:
        case TRAPDOOR:
            You("把网织在%s上.",
                (ttmp->ttyp == TRAPDOOR) ? "陷阱门" : "洞");
            deltrap(ttmp);
            newsym(x, y);
            return ECMD_TIME;
        case ROLLING_BOULDER_TRAP:
            You("织的网干扰了触发器.");
            deltrap(ttmp);
            newsym(x, y);
            return ECMD_TIME;
        case ARROW_TRAP:
        case DART_TRAP:
        case BEAR_TRAP:
        case ROCKTRAP:
        case FIRE_TRAP:
        case LANDMINE:
        case SLP_GAS_TRAP:
        case RUST_TRAP:
        case MAGIC_TRAP:
        case ANTI_MAGIC:
        case POLY_TRAP:
            You("触发了一个陷阱!");
            dotrap(ttmp, NO_TRAP_FLAGS);
            return ECMD_TIME;
        default:
            impossible("Webbing over trap type %d?", ttmp->ttyp);
            return ECMD_OK;
        }
    } else if (On_stairs(x, y)) {
        /* cop out: don't let them hide the stairs */
        Your("网没有挡住去%s的路.",
             (levl[x][y].typ == STAIRS) ? "楼梯" : "梯子");
        return ECMD_TIME;
    }
    ttmp = maketrap(x, y, WEB);
    if (ttmp) {
        You("你吐出一张网.");
        ttmp->madeby_u = 1;
        feeltrap(ttmp);
        if (*in_rooms(x, y, SHOPBASE))
            add_damage(x, y, SHOP_WEB_COST);
    }
    return ECMD_TIME;
}

int
dosummon(void)
{
    int placeholder;
    if (u.uen < 10) {
        You("没有足够的能量来发出求救的呼唤!");
        return ECMD_OK;
    }
    u.uen -= 10;
    disp.botl = TRUE;

    You("呼唤你的同胞寻求帮助!");
    exercise(A_WIS, TRUE);
    if (!were_summon(gy.youmonst.data, TRUE, &placeholder, (char *) 0))
        pline("但是什么都没有来.");
    return ECMD_TIME;
}

int
dogaze(void)
{
    struct monst *mtmp;
    int looked = 0;
    char qbuf[QBUFSZ];
    int i;
    uchar adtyp = 0;

    for (i = 0; i < NATTK; i++) {
        if (gy.youmonst.data->mattk[i].aatyp == AT_GAZE) {
            adtyp = gy.youmonst.data->mattk[i].adtyp;
            break;
        }
    }
    if (adtyp != AD_CONF && adtyp != AD_FIRE) {
        impossible("gaze attack %d?", adtyp);
        return ECMD_OK;
    }

    if (Blind) {
        You_cant("注视任何东西.");
        return ECMD_OK;
    } else if (Hallucination) {
        You_cant("注视任何你能看到的东西.");
        return ECMD_OK;
    }
    if (u.uen < 15) {
        You("没有足够的能量来使用你的注视攻击!");
        return ECMD_OK;
    }
    u.uen -= 15;
    disp.botl = TRUE;

    for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
        if (DEADMONSTER(mtmp))
            continue;
        if (canseemon(mtmp) && couldsee(mtmp->mx, mtmp->my)) {
            looked++;
            if (Invis && !perceives(mtmp->data)) {
                pline("%s似乎没有注意到你的注视.", Monnam(mtmp));
            } else if (mtmp->minvis && !See_invisible) {
                You_cant("看清该朝哪里注视%s.", Monnam(mtmp));
            } else if (M_AP_TYPE(mtmp) == M_AP_FURNITURE
                       || M_AP_TYPE(mtmp) == M_AP_OBJECT) {
                looked--;
                continue;
            } else if (flags.safe_dog && mtmp->mtame && !Confusion) {
                You("避免注视%s.", y_monnam(mtmp));
            } else {
                if (flags.confirm && mtmp->mpeaceful && !Confusion) {
                    Sprintf(qbuf, "确定%s%s?",
                            (adtyp == AD_CONF) ? "混乱" : "攻击",
                            mon_nam(mtmp));
                    if (y_n(qbuf) != 'y')
                        continue;
                }
                setmangry(mtmp, TRUE);
                if (helpless(mtmp) || mtmp->mstun
                    || !mtmp->mcansee || !haseyes(mtmp->data)) {
                    looked--;
                    continue;
                }
                /* No reflection check for consistency with when a monster
                 * gazes at *you*--only medusa gaze gets reflected then.
                 */
                if (adtyp == AD_CONF) {
                    if (!mtmp->mconf)
                        Your("注视混乱了%s!", mon_nam(mtmp));
                    else
                        pline("%s越来越混乱了.",
                              Monnam(mtmp));
                    mtmp->mconf = 1;
                } else if (adtyp == AD_FIRE) {
                    int dmg = d(2, 6), orig_dmg = dmg, lev = (int) u.ulevel;

                    You("用一种炽热的注视攻击%s!", mon_nam(mtmp));
                    if (resists_fire(mtmp)) {
                        pline_The("%s没有着火!", mon_nam(mtmp));
                        dmg = 0;
                    }
                    if (lev > rn2(20)) {
                        dmg += destroy_items(mtmp, AD_FIRE, orig_dmg);
                        ignite_items(mtmp->minvent);
                    }
                    if (dmg)
                        mtmp->mhp -= dmg;
                    if (DEADMONSTER(mtmp))
                        killed(mtmp);
                }
                /* For consistency with passive() in uhitm.c, this only
                 * affects you if the monster is still alive.
                 */
                if (DEADMONSTER(mtmp))
                    continue;

                if (mtmp->data == &mons[PM_FLOATING_EYE] && !mtmp->mcan) {
                    if (!Free_action) {
                        You("被%s的注视定住了!",
                            s_suffix(mon_nam(mtmp)));
                        nomul((u.ulevel > 6 || rn2(4))
                                  ? -d((int) mtmp->m_lev + 1,
                                       (int) mtmp->data->mattk[0].damd)
                                  : -200);
                        gm.multi_reason = "被怪物的注视定住";
                        gn.nomovemsg = 0;
                        return ECMD_TIME;
                    } else
                        You("在%s的注视下僵住了一刹那.",
                            s_suffix(mon_nam(mtmp)));
                }
                /* Technically this one shouldn't affect you at all because
                 * the Medusa gaze is an active monster attack that only
                 * works on the monster's turn, but for it to *not* have an
                 * effect would be too weird.
                 */
                if (mtmp->data == &mons[PM_MEDUSA] && !mtmp->mcan) {
                    pline("注视睁着眼的%s不是一个非常好的主意.",
                          l_monnam(mtmp));
                    /* as if gazing at a sleeping anything is fruitful... */
                    urgent_pline("你变成了石头...");
                    svk.killer.format = KILLED_BY;
                    Strcpy(svk.killer.name,
                           "故意正视美杜莎的眼睛");
                    done(STONING);
                }
            }
        }
    }
    if (!looked)
        You("尤其注视不到任何地方.");
    return ECMD_TIME;
}

/* called by domonability() for #monster */
int
dohide(void)
{
    boolean ismimic = gy.youmonst.data->mlet == S_MIMIC,
            on_ceiling = is_clinger(gy.youmonst.data) || Flying;

    /* can't hide while being held (or holding) or while trapped
       (except for floor hiders [trapper or mimic] in pits) */
    if (u.ustuck || (u.utrap && (u.utraptype != TT_PIT || on_ceiling))) {
        You_cant("在你%s的时候躲藏.",
                 !u.ustuck ? "受困"
                   : u.uswallow ? (digests(u.ustuck->data) ? "被吞咽"
                                                           : "被吞没")
                     : !sticks(gy.youmonst.data) ? "被抓住"
                       : (humanoid(u.ustuck->data) ? "抓住某人"
                                                   : "抓住那个生物"));
        if (u.uundetected || (ismimic && U_AP_TYPE != M_AP_NOTHING)) {
            u.uundetected = 0;
            gy.youmonst.m_ap_type = M_AP_NOTHING;
            newsym(u.ux, u.uy);
        }
        return ECMD_OK;
    }
    /* note: hero-as-eel handling is incomplete but unnecessary;
       such critters aren't offered the option of hiding via #monster */
    if (gy.youmonst.data->mlet == S_EEL && !is_pool(u.ux, u.uy)) {
        if (IS_FOUNTAIN(levl[u.ux][u.uy].typ))
            pline_The("喷泉不够深,藏不进去.");
        else
            There("没有%s可以藏进去.", hliquid("水"));
        u.uundetected = 0;
        return ECMD_OK;
    }
    if (hides_under(gy.youmonst.data)) {
        long ct = 0L;
        struct obj *otmp, *otop = svl.level.objects[u.ux][u.uy];

        if (!otop) {
            There("没有东西可以藏进去.");
            u.uundetected = 0;
            return ECMD_OK;
        }
        for (otmp = otop;
             otmp && otmp->otyp == CORPSE
                  && touch_petrifies(&mons[otmp->corpsenm]);
             otmp = otmp->nexthere)
            ct += otmp->quan;
        /* otmp will be Null iff the entire pile consists of 'trice corpses */
        if (!otmp && !Stone_resistance) {
            char kbuf[BUFSZ];
            const char *corpse_name = cxname(otop);

            /* for the plural case, we'll say "cockatrice corpses" or
               "chickatrice corpses" depending on the top of the pile
               even if both types are present */
            if (ct == 1)
                corpse_name = an(corpse_name);
            /* no need to check poly_when_stoned(); no hide-underers can
               turn into stone golems instead of becoming petrified */
            pline("躲在%s%s下面是个致命的错误...",
                  corpse_name, plur(ct));
            Sprintf(kbuf, "躲在%s%s下面", corpse_name, plur(ct));
            instapetrify(kbuf);
            /* only reach here if life-saved */
            u.uundetected = 0;
            return ECMD_TIME;
        }
    }
    /* Planes of Air and Water */
    if (on_ceiling && !has_ceiling(&u.uz)) {
        There("上面无处可藏.");
        u.uundetected = 0;
        return ECMD_OK;
    }
    if ((is_hider(gy.youmonst.data) && !Flying) /* floor hider */
        && (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz))) {
        There("下面无处可藏.");
        u.uundetected = 0;
        return ECMD_OK;
    }
    /* TODO? inhibit floor hiding at furniture locations, or
     * else make youhiding() give smarter messages at such spots.
     */

    if (u.uundetected || (ismimic && U_AP_TYPE != M_AP_NOTHING)) {
        youhiding(FALSE, 1); /* "you are already hiding" */
        return ECMD_OK;
    }

    if (ismimic) {
        /* should bring up a dialog "what would you like to imitate?" */
        gy.youmonst.m_ap_type = M_AP_OBJECT;
        gy.youmonst.mappearance = STRANGE_OBJECT;
    } else
        u.uundetected = 1;
    newsym(u.ux, u.uy);
    youhiding(FALSE, 0); /* "you are now hiding" */
    return ECMD_TIME;
}

int
dopoly(void)
{
    struct permonst *savedat = gy.youmonst.data;

    if (is_vampire(gy.youmonst.data) || is_vampshifter(&gy.youmonst)) {
        polyself(POLY_MONSTER);
        if (savedat != gy.youmonst.data) {
            You("转变成了%s.",
                an(pmname(gy.youmonst.data, Ugender)));
            newsym(u.ux, u.uy);
        }
    }
    return ECMD_TIME;
}

/* #monster for hero-as-mind_flayer giving psychic blast */
int
domindblast(void)
{
    struct monst *mtmp, *nmon;
    int dmg;

    if (u.uen < 10) {
        You("集中注意力,但你的能量不足以保持这么做.");
        return ECMD_OK;
    }
    u.uen -= 10;
    disp.botl = TRUE;

    You("集中了注意力.");
    pline("一股精神能量涌了出来.");
    for (mtmp = fmon; mtmp; mtmp = nmon) {
        int u_sen;

        nmon = mtmp->nmon;
        if (DEADMONSTER(mtmp))
            continue;
        if (mdistu(mtmp) > BOLT_LIM * BOLT_LIM)
            continue;
        if (mtmp->mpeaceful)
            continue;
        if (mindless(mtmp->data))
            continue;
        u_sen = telepathic(mtmp->data) && !mtmp->mcansee;
        if (u_sen || (telepathic(mtmp->data) && rn2(2)) || !rn2(10)) {
            dmg = rnd(15);
            /* wake it up first, to bring hidden monster out of hiding;
               but in case it is currently peaceful, don't make it hostile
               unless it will survive the psychic blast, otherwise hero
               would avoid the penalty for killing it while peaceful */
            wakeup(mtmp, (dmg > mtmp->mhp) ? TRUE : FALSE);
            You("锁定于%s的%s.", s_suffix(mon_nam(mtmp)),
                u_sen ? "心灵感应"
                : telepathic(mtmp->data) ? "潜在心灵感应"
                  : "心灵");
            mtmp->mhp -= dmg;
            if (DEADMONSTER(mtmp))
                killed(mtmp);
        }
    }
    return ECMD_TIME;
}

void
uunstick(void)
{
    struct monst *mtmp = u.ustuck;

    if (!mtmp) {
        impossible("uunstick: no ustuck?");
        return;
    }
    set_ustuck((struct monst *) 0); /* before pline() */
    pline("%s不再被你控制.", Monnam(mtmp));
}

void
skinback(boolean silently)
{
    if (uskin) {
        int old_light = arti_light_radius(uskin);

        if (!silently)
            Your("皮肤变回了原来的样子.");
        uarm = uskin;
        uskin = (struct obj *) 0;
        /* undo save/restore hack */
        uarm->owornmask &= ~I_SPECIAL;

        if (artifact_light(uarm))
            maybe_adjust_light(uarm, old_light);
    }
}

const char *
mbodypart(struct monst *mon, int part)
{
    static NEARDATA const char
        *humanoid_parts[] = { "胳膊",       "眼睛",  "脸",         "手指",
                              "指尖", "脚", "手",         "手",
                              "头",      "腿",  "头晕", "脖子",
                              "脊椎",     "脚趾",  "头发",         "血液",
                              "肺",      "鼻子", "胃" },
        *jelly_parts[] = { "伪足", "黑点", "正面",
                           "延长的伪足", "伪足末端",
                           "伪足根", "控制", "控制",
                           "脑区", "下伪足", "粘性",
                           "中部", "表面", "伪足末端",
                           "波纹", "汁液", "表面", "感官",
                           "胃" },
        *animal_parts[] = { "前肢",  "眼睛",           "脸",
                            "前爪",  "爪尖",      "后爪",
                            "前爪",  "爪",        "头",
                            "后肢", "头晕",  "脖子",
                            "脊椎",     "后爪尖", "毛",
                            "血液",     "肺",          "鼻子",
                            "胃" },
        *bird_parts[] = { "翅膀",     "眼睛",  "脸",         "翅膀",
                          "翼梢", "脚", "翅膀",         "翅膀",
                          "头",     "腿",  "头晕", "脖子",
                          "脊椎",    "脚趾",  "羽毛",     "血液",
                          "肺",     "喙", "胃" },
        *horse_parts[] = { "前腿",  "眼睛",           "脸",
                           "前蹄", "蹄尖",      "后蹄",
                           "前蹄", "蹄",        "头",
                           "后腿", "头晕",  "脖子",
                           "脊骨", "后蹄尖", "鬃毛",
                           "血液",    "肺",          "鼻子",
                           "胃" },
        *sphere_parts[] = { "附肢", "视神经", "身体", "触手",
                            "触手尖", "下附肢", "触手",
                            "触手", "身体", "下触手",
                            "旋转", "赤道面", "身体",
                            "下触手尖", "纤毛", "生命力",
                            "视网膜", "嗅神经", "内部" },
        *fungus_parts[] = { "菌丝", "视觉中枢", "正面",
                            "菌丝",    "菌丝",       "根",
                            "纤维",   "纤维",    "盖区",
                            "根茎",  "孢子",  "茎",
                            "根",     "根茎尖", "孢子",
                            "汁液",   "菌褶",        "菌褶",
                            "内部" },
        *vortex_parts[] = { "部位",        "眼睛",           "正面",
                            "小流动", "小流动", "下流动",
                            "漩涡",         "漩涡",       "中心核",
                            "下流动", "混乱",        "中心",
                            "流动",      "边缘",          "流动",
                            "生命力",    "中心",        "前缘",
                            "内部" },
        *snake_parts[] = { "退化的脚", "眼睛", "脸", "大鳞片",
                           "大鳞片尖", "尾部区域", "鳞间隙",
                           "鳞间隙", "头", "尾部区域",
                           "头晕", "脖子", "长体", "尾部鳞片",
                           "鳞片", "血液", "肺", "分叉舌",
                           "胃" },
        *worm_parts[] = { "前体节", "感光细胞",
                          "环带", "茸毛", "茸毛", "后体节",
                          "体节", "体节", "前体节",
                          "后端", "过度拉伸", "环带",
                          "长体", "后茸毛", "茸毛", "血液",
                          "皮肤", "口前叶", "胃" },
        *spider_parts[] = { "触肢", "眼睛", "脸", "触肢", "跗节",
                            "爪", "触肢", "触肢", "头胸部",
                            "腿", "吐丝", "头胸部", "腹部",
                            "爪", "毛", "血淋巴", "书肺",
                            "唇基", "消化道" },
        *fish_parts[] = { "鳍", "眼睛", "前颌骨", "骨盆腋",
                          "腹鳍", "臀鳍", "胸鳍", "鳍",
                          "头", "梗节", "衰竭", "鳃",
                          "背鳍", "尾鳍", "鳞", "血液",
                          "鳃", "鼻孔", "胃" };
    /* claw attacks are overloaded in mons[]; most humanoids with
       such attacks should still reference hands rather than claws */
    static const char not_claws[] = {
        S_HUMAN,     S_MUMMY,   S_ZOMBIE, S_ANGEL, S_NYMPH, S_LEPRECHAUN,
        S_QUANTMECH, S_VAMPIRE, S_ORC,    S_GIANT, /* quest nemeses */
        '\0' /* string terminator; assert( S_xxx != 0 ); */
    };
    struct permonst *mptr = mon->data;

    if (part <= NO_PART) {
        impossible("mbodypart: bad part %d", part);
        return "mystery part";
    }

    /* some special cases */
    if (mptr->mlet == S_DOG || mptr->mlet == S_FELINE
        || mptr->mlet == S_RODENT || mptr == &mons[PM_OWLBEAR]) {
        switch (part) {
        case HAND:
            return "爪子";
        case HANDED:
            return "爪子";
        case FOOT:
            return "后爪";
        case ARM:
        case LEG:
            return horse_parts[part]; /* "foreleg", "rear leg" */
        default:
            break; /* for other parts, use animal_parts[] below */
        }
    } else if (mptr->mlet == S_YETI) { /* excl. owlbear due to 'if' above */
        /* opposable thumbs, hence "hands", "arms", "legs", &c */
        return humanoid_parts[part]; /* yeti/sasquatch, monkey/ape */
    }
    if ((part == HAND || part == HANDED)
        && (humanoid(mptr) && attacktype(mptr, AT_CLAW)
            && !strchr(not_claws, mptr->mlet) && mptr != &mons[PM_STONE_GOLEM]
            && mptr != &mons[PM_AMOROUS_DEMON]))
        return (part == HAND) ? "爪" : "爪";
    if ((mptr == &mons[PM_MUMAK] || mptr == &mons[PM_MASTODON])
        && part == NOSE)
        return "象鼻";
    if (mptr == &mons[PM_SHARK] && part == HAIR)
        return "皮肤"; /* sharks don't have scales */
    if ((mptr == &mons[PM_JELLYFISH] || mptr == &mons[PM_KRAKEN])
        && (part == ARM || part == FINGER || part == HAND || part == FOOT
            || part == TOE))
        return "触手";
    if (mptr == &mons[PM_FLOATING_EYE] && part == EYE)
        return "角膜";
    if (humanoid(mptr) && (part == ARM || part == FINGER || part == FINGERTIP
                           || part == HAND || part == HANDED))
        return humanoid_parts[part];
    if (mptr->mlet == S_COCKATRICE)
        return (part == HAIR) ? snake_parts[part] : bird_parts[part];
    if (mptr == &mons[PM_RAVEN])
        return bird_parts[part];
    if (mptr->mlet == S_CENTAUR || mptr->mlet == S_UNICORN
        || mptr == &mons[PM_KI_RIN]
        || (mptr == &mons[PM_ROTHE] && part != HAIR))
        return horse_parts[part];
    if (mptr->mlet == S_LIGHT) {
        if (part == HANDED)
            return "光线";
        else if (part == ARM || part == FINGER || part == FINGERTIP
                 || part == HAND)
            return "光线";
        else
            return "光束";
    }
    if (mptr == &mons[PM_STALKER] && part == HEAD)
        return "头";
    if (mptr->mlet == S_EEL && mptr != &mons[PM_JELLYFISH])
        return fish_parts[part];
    if (mptr->mlet == S_WORM)
        return worm_parts[part];
    if (mptr->mlet == S_SPIDER)
        return spider_parts[part];
    if (slithy(mptr) || (mptr->mlet == S_DRAGON && part == HAIR))
        return snake_parts[part];
    if (mptr->mlet == S_EYE)
        return sphere_parts[part];
    if (mptr->mlet == S_JELLY || mptr->mlet == S_PUDDING
        || mptr->mlet == S_BLOB || mptr == &mons[PM_JELLYFISH])
        return jelly_parts[part];
    if (mptr->mlet == S_VORTEX || mptr->mlet == S_ELEMENTAL)
        return vortex_parts[part];
    if (mptr->mlet == S_FUNGUS)
        return fungus_parts[part];
    if (humanoid(mptr))
        return humanoid_parts[part];
    return animal_parts[part];
}

const char *
body_part(int part)
{
    return mbodypart(&gy.youmonst, part);
}

int
poly_gender(void)
{
    /* Returns gender of polymorphed player;
     * 0/1=same meaning as flags.female, 2=none.
     */
    if (is_neuter(gy.youmonst.data) || !humanoid(gy.youmonst.data))
        return 2;
    return flags.female;
}

void
ugolemeffects(int damtype, int dam)
{
    int heal = 0;

    /* We won't bother with "slow"/"haste" since players do not
     * have a monster-specific slow/haste so there is no way to
     * restore the old velocity once they are back to human.
     */
    if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
        return;
    switch (damtype) {
    case AD_ELEC:
        if (u.umonnum == PM_FLESH_GOLEM)
            heal = (dam + 5) / 6; /* Approx 1 per die */
        break;
    case AD_FIRE:
        if (u.umonnum == PM_IRON_GOLEM)
            heal = dam;
        break;
    }
    if (heal && (u.mh < u.mhmax)) {
        u.mh += heal;
        if (u.mh > u.mhmax)
            u.mh = u.mhmax;
        disp.botl = TRUE;
        pline("奇怪的是,你感觉比以前好了.");
        exercise(A_STR, TRUE);
    }
}

staticfn int
armor_to_dragon(int atyp)
{
    switch (atyp) {
    case GRAY_DRAGON_SCALE_MAIL:
    case GRAY_DRAGON_SCALES:
        return PM_GRAY_DRAGON;
    case SILVER_DRAGON_SCALE_MAIL:
    case SILVER_DRAGON_SCALES:
        return PM_SILVER_DRAGON;
    case GOLD_DRAGON_SCALE_MAIL:
    case GOLD_DRAGON_SCALES:
        return PM_GOLD_DRAGON;
#if 0 /* DEFERRED */
    case SHIMMERING_DRAGON_SCALE_MAIL:
    case SHIMMERING_DRAGON_SCALES:
        return PM_SHIMMERING_DRAGON;
#endif
    case RED_DRAGON_SCALE_MAIL:
    case RED_DRAGON_SCALES:
        return PM_RED_DRAGON;
    case ORANGE_DRAGON_SCALE_MAIL:
    case ORANGE_DRAGON_SCALES:
        return PM_ORANGE_DRAGON;
    case WHITE_DRAGON_SCALE_MAIL:
    case WHITE_DRAGON_SCALES:
        return PM_WHITE_DRAGON;
    case BLACK_DRAGON_SCALE_MAIL:
    case BLACK_DRAGON_SCALES:
        return PM_BLACK_DRAGON;
    case BLUE_DRAGON_SCALE_MAIL:
    case BLUE_DRAGON_SCALES:
        return PM_BLUE_DRAGON;
    case GREEN_DRAGON_SCALE_MAIL:
    case GREEN_DRAGON_SCALES:
        return PM_GREEN_DRAGON;
    case YELLOW_DRAGON_SCALE_MAIL:
    case YELLOW_DRAGON_SCALES:
        return PM_YELLOW_DRAGON;
    default:
        return NON_PM;
    }
}

/* some species have awareness of other species */
staticfn void
polysense(void)
{
    short warnidx = NON_PM;

    svc.context.warntype.speciesidx = NON_PM;
    svc.context.warntype.species = 0;
    svc.context.warntype.polyd = 0;
    HWarn_of_mon &= ~FROMRACE;

    switch (u.umonnum) {
    case PM_PURPLE_WORM:
    case PM_BABY_PURPLE_WORM:
        warnidx = PM_SHRIEKER;
        break;
    case PM_VAMPIRE:
    case PM_VAMPIRE_LEADER:
        svc.context.warntype.polyd = M2_HUMAN | M2_ELF;
        HWarn_of_mon |= FROMRACE;
        return;
    }
    if (ismnum(warnidx)) {
        svc.context.warntype.speciesidx = warnidx;
        svc.context.warntype.species = &mons[warnidx];
        HWarn_of_mon |= FROMRACE;
    }
}

/* True iff hero's role or race has been genocided */
boolean
ugenocided(void)
{
    return ((svm.mvitals[gu.urole.mnum].mvflags & G_GENOD)
            || (svm.mvitals[gu.urace.mnum].mvflags & G_GENOD));
}

/* how hero feels "inside" after self-genocide of role or race */
const char *
udeadinside(void)
{
    /* self-genocide used to always say "you feel dead inside" but that
       seems silly when you're polymorphed into something undead;
       monkilled() distinguishes between living (killed) and non (destroyed)
       for monster death message; we refine the nonliving aspect a bit */
    return !nonliving(gy.youmonst.data)
             ? "一片死寂"          /* living, including demons */
             : !weirdnonliving(gy.youmonst.data)
                 ? "审判来临" /* undead plus manes */
                 : "一片空虚";    /* golems plus vortices */
}

/*polyself.c*/
