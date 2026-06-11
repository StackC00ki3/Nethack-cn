/* NetHack 5.0	sit.c	$NHDT-Date: 1718136168 2024/06/11 20:02:48 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.95 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2012. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

staticfn void throne_sit_effect(void);
staticfn int lay_an_egg(void);

/* take away the hero's money */
void
take_gold(void)
{
    struct obj *otmp, *nobj;
    int lost_money = 0;

    for (otmp = gi.invent; otmp; otmp = nobj) {
        nobj = otmp->nobj;
        if (otmp->oclass == COIN_CLASS) {
            lost_money = 1;
            remove_worn_item(otmp, FALSE);
            delobj(otmp);
        }
    }
    if (!lost_money) {
        You_feel("有点奇怪.");
    } else {
        You("注意到自己身上没有金币了!");
        disp.botl = TRUE;
    }
}

staticfn void special_throne_effect(int effect);

/* maybe do something when hero sits on a throne */
staticfn void
throne_sit_effect(void)
{
    coordxy tx = u.ux, ty = u.uy;

    boolean special_throne = !!In_V_tower(&u.uz);

    if (rnd(6) > 4) { /* [why so convoluted? it's the same as '!rn2(3)'] */
        int effect = rnd(13);

        if (wizard && !iflags.debug_fuzzer) {
            char buf[BUFSZ];
            int which;

            buf[0] = '\0';
            getlin("选择王座效果(1..13)[0=随机]", buf);
            if (buf[0] == '\033') {
                pline("%s", Never_mind);
                return; /* caller will still cause a move to elapse */
            }
            which = atoi(buf);
            if (which >= 1 && which <= 13)
                effect = which;
        }

        if (special_throne) {
            special_throne_effect(effect);
            return;
        }

        switch (effect) {
        case 1:
            (void) adjattrib(rn2(A_MAX), -rn1(4, 3), FALSE);
            losehp(rnd(10), "王座被诅咒", KILLED_BY_AN);
            break;
        case 2:
            (void) adjattrib(rn2(A_MAX), 1, FALSE);
            break;
        case 3:
            pline("一股%s电流冲击穿透了你的身体!",
                  (Shock_resistance) ? "" : "巨大的");
            losehp(Shock_resistance ? rnd(6) : rnd(30), "椅子放电",
                   KILLED_BY_AN);
            exercise(A_CON, FALSE);
            break;
        case 4:
            You_feel("好,好多了!");
            if (Upolyd) {
                if (u.mh >= (u.mhmax - 5))
                    u.mhmax += 4;
                u.mh = u.mhmax;
            }
            if (u.uhp >= (u.uhpmax - 5)) {
                u.uhpmax += 4;
                if (u.uhpmax > u.uhppeak)
                    u.uhppeak = u.uhpmax;
            }
            u.uhp = u.uhpmax;
            u.ucreamed = 0;
            make_blinded(0L, TRUE);
            make_sick(0L, (char *) 0, FALSE, SICK_ALL);
            heal_legs(0);
            disp.botl = TRUE;
            break;
        case 5:
            take_gold();
            break;
        case 6:
            if (u.uluck + rn2(5) < 0) {
                You_feel("你的运气在变化.");
                change_luck(1);
            } else
                makewish();
            break;
        case 7:
            {
                int cnt = rnd(10);

                /* Magical voice not affected by deafness */
                pline("一个声音回荡道:");
                SetVoice((struct monst *) 0, 0, 80, voice_throne);
                verbalize("尔客至矣,%s!",
                          flags.female ? "夫人" : "主公");
                while (cnt--)
                    (void) makemon(courtmon(), tx, ty, NO_MM_FLAGS);
                break;
            }
        case 8:
            /* Magical voice not affected by deafness */
            pline("一个声音回荡道:");
            SetVoice((struct monst *) 0, 0, 80, voice_throne);
            verbalize("钦承%s威命...",
                      flags.female ? "夫人" : "主公");
            do_genocide(5); /* REALLY|ONTHRONE, see do_genocide() */
            break;
        case 9:
            /* Magical voice not affected by deafness */
            pline("一个声音回荡道:");
            SetVoice((struct monst *) 0, 0, 80, voice_throne);
            verbalize(
                 "乱臣贼子,敢坐于此无上圣座,当遭天谴!");
            if (Luck > 0) {
                make_blinded(BlindedTimeout + rn1(100, 250), TRUE);
                change_luck((Luck > 1) ? -rnd(2) : -1);
            } else
                rndcurse();
            break;
        case 10:
            if (Luck < 0 || (HSee_invisible & INTRINSIC)) {
                if (svl.level.flags.nommap) {
                    pline("一种可怕的嗡嗡声充斥你的大脑!");
                    make_confused((HConfusion & TIMEOUT) + (long) rnd(30),
                                  FALSE);
                } else {
                    pline("一个图像在你心中显现.");
                    do_mapping();
                }
            } else {
                /* avoid "vision clears" if hero can't see */
                if (!Blind) {
                    Your("视觉变得清晰了.");
                } else {
                    int num_of_eyes = eyecount(gy.youmonst.data);
                    const char *eye = body_part(EYE);

                    /* note: 1 eye case won't actually happen--can't
                       sit on throne when poly'd into always-levitating
                       floating eye and can't polymorph into Cyclops */
                    switch (num_of_eyes) { /* 2, 1, or 0 */
                    default:
                    case 2: /* more than 1 eye */
                        eye = makeplural(eye);
                        FALLTHROUGH;
                        /*FALLTHRU*/
                    case 1: /* one eye (Cyclops, floating eye) */
                        Your("%s在%s...", eye, vtense(eye, "刺痛"));
                        break;
                    case 0: /* no eyes */
                        Your("%s中有一种非常奇怪的感觉.",
                             body_part(HEAD));
                        break;
                    }
                }
                HSee_invisible |= FROMOUTSIDE;
                newsym(u.ux, u.uy);
            }
            break;
        case 11:
            if (Luck < 0) {
                You_feel("被威胁了.");
                aggravate();
            } else {
                You_feel("一阵扭曲.");
                tele(); /* teleport him */
            }
            break;
        case 12:
            You("被赐予一次启示机会!");
            if (gi.invent) {
                /* rn2(5) agrees w/seffects() */
                identify_pack(rn2(5), FALSE);
            }
            break;
        case 13:
            Your("心变成了一块椒盐卷饼!");
            make_confused((HConfusion & TIMEOUT) + (long) rn1(7, 16),
                          FALSE);
            break;
        default:
            impossible("throne effect");
            break;
        }
    } else {
        if (is_prince(gy.youmonst.data) || u.uevent.uhand_of_elbereth)
            You_feel("这里非常舒服.");
        else
            You_feel("有点格格不入...");
    }

    /* 5.0: when the random chance for removal is hit, ask for confirmation
       if in wizard mode, and remove the throne even if hero was teleported
       away from it.  [This used to remove a throne at hero's current
       location if there happened to be one, so for the teleport case that
       only happened when teleporting back to the same point where hero
       started from.]  "Analyzing a throne" doesn't really make any sense
       but if the answer is yes than it will vanish in a puff of logic. */
    if (!special_throne &&
        !rn2(3) && (!wizard || y_n("分析王座吗?") == 'y')) {
        levl[tx][ty].typ = ROOM, levl[tx][ty].flags = 0;
        map_background(tx, ty, FALSE);
        newsym_force(tx, ty);
        /* "[God] promptly vanishes in a puff of logic" is from
           Douglas Adams' _The_Hitchhiker's_Guide_to_the_Galaxy_. */
        pline_The("王座在一团逻辑烟雾中%s.",
                  cansee(tx, ty) ? "消失" : "已经消失");
    }
}

/* special throne in Vlad's tower: effect is 1 to 13 inclusive */
staticfn void
special_throne_effect(int effect) {
    coordxy tx = u.ux, ty = u.uy;

    switch (effect) {
    case 1:
    case 2:
    case 3:
    case 4:
        /* 4 chances of a wish, but then the throne disappears.

           This is the only way the throne can disappear from sitting
           on it, so if you sit on it enough (enduring the negative
           effects) you are guaranteed an eventual wish. */
        makewish();
        levl[tx][ty].typ = ROOM, levl[tx][ty].flags = 0;
        map_background(tx, ty, FALSE);
        newsym_force(tx, ty);
        pline_The("王座耗尽了力量,瓦解了.");
        break;
    case 5:
        /* permanent level drain */
        pline("坐在王座上真是糟糕透顶.");
        if (!Drain_resistance) {
            losexp("坐在王座上的糟糕经历");
            if (u.ulevelmax > u.ulevel)
                u.ulevelmax -= 1;
        }
        break;
    case 6:
    {
        /* grease hands and inventory

           Same rules for which items can be affected as grease_ok in apply.c */
        struct obj *otmp;

        pline("一股油腻的液体喷了你一身!");
        for (otmp = gi.invent; otmp; otmp = otmp->nobj)
            if (otmp->oclass != COIN_CLASS)
                otmp->greased = 1;
        make_glib(rn1(101, 100));
        update_inventory();
        break;
    }
    case 7:
        /* lose an intrinsic */
        attrcurse();
        pline_The("王座似乎被逗乐了.");
        break;
    case 8:
    {
        /* level teleport to Vibrating Square level */
        d_level vs_level;
        find_hell(&vs_level);
        vs_level.dlevel = svd.dungeons[vs_level.dnum].num_dunlevs - 1;
        if (u.uhave.amulet)
            You_feel("极其迷失了一刹那.");
        else
            schedule_goto(
                &vs_level, UTOTYPE_NONE, (char *) 0,
                "你感觉十分格格不入.");
        break;
    }
    case 9:
    {
        /* summon demons; a NULL argument to msummon summons demons as
           though they were summoned by the Wizard of Yendor */
        pline_The("王座似乎在呼救!");
        msummon(NULL);
        msummon(NULL);
        msummon(NULL);
        break;
    }
    case 10:
    {
        /* confused blessed remove curse effect */
        struct obj fake_spellbook;
        long save_confusion = HConfusion;

        fake_spellbook = cg.zeroobj;
        fake_spellbook.otyp = SPE_REMOVE_CURSE;
        fake_spellbook.oclass = SPBOOK_CLASS;
        fake_spellbook.blessed = 1;
        HConfusion = 1L;
        (void) seffects(&fake_spellbook);
        HConfusion = save_confusion;
        break;
    }
    case 11:
        /* polymorph effect (not blocked by magic resistance, but other things
           that protect from polymorphs work) */
        if (is_vampire(gy.youmonst.data)) {
            You_feel("不值得.");
        } else {
            pline("这个王座不是为你量身定制的!");
            You_feel("一种变化正在你身上发生.");
            polyself(POLY_NOFLAGS);
        }
        break;
    case 12:
        /* acid damage */
        pline("酸液从王座中渗出来!");
        losehp(Acid_resistance ? rnd(16) : rnd(80), "椅子喷出酸液",
               KILLED_BY_AN);
        exercise(A_CON, FALSE);
        break;
    case 13:
    {
        /* ability shuffle */
        int ability;
        pline("当你坐在王座上,你的身体和心智开始扭曲.");
        for (ability = 0; ability < A_MAX; ++ability) {
            adjattrib(ability, rn2(5) - 2, -1);
        }
        break;
    }
    }
}

/* hero lays an egg */
staticfn int
lay_an_egg(void)
{
    struct obj *uegg;

    if (!flags.female) {
        pline("%s不能下蛋!",
              Hallucination
              ? "你可能认为你是一个鸭嘴兽, 但雄性仍然"
              : "雄性");
        return ECMD_OK;
    } else if (u.uhunger < (int) objects[EGG].oc_nutrition) {
        You("没有足够的能量来下蛋.");
        return ECMD_OK;
    } else if (eggs_in_water(gy.youmonst.data)) {
        if (!(Underwater || Is_waterlevel(&u.uz))) {
            pline("你又不是溅水灯鱼.");
            return ECMD_OK;
        }
        if (Upolyd
            && (gy.youmonst.data == &mons[PM_GIANT_EEL]
                || gy.youmonst.data == &mons[PM_ELECTRIC_EEL])) {
            You("渴望马尾藻海.");
            return ECMD_OK;
        }
    }
    uegg = mksobj(EGG, FALSE, FALSE);
    uegg->spe = 1;
    uegg->quan = 1L;
    uegg->owt = weight(uegg);
    /* this sets hatch timers if appropriate */
    set_corpsenm(uegg, egg_type_from_parent(u.umonnum, FALSE));
    uegg->known = 1;
    observe_object(uegg);
    You("%s.", eggs_in_water(gy.youmonst.data) ? "下了一个蛋" : "下了一个蛋");
    dropy(uegg);
    stackobj(uegg);
    morehungry((int) objects[EGG].oc_nutrition);
    return ECMD_TIME;
}

/* #sit command */
int
dosit(void)
{
    static const char sit_message[] = "坐在%s上.";
    struct trap *trap = t_at(u.ux, u.uy);
    int typ = levl[u.ux][u.uy].typ;

    if (u.usteed) {
        You("已经坐在%s身上了.", mon_nam(u.usteed));
        return ECMD_OK;
    }
    if (u.uundetected && is_hider(gy.youmonst.data)
        && u.umonnum != PM_TRAPPER) /* trapper can stay hidden on floor */
        u.uundetected = 0; /* no longer on the ceiling */

    if (!can_reach_floor(FALSE)) {
        if (u.uswallow)
            There("没有座位!");
        else if (Levitation)
            You("在空中翻滚.");
        else
            You("坐在空中.");
        return ECMD_OK;
    } else if (u.ustuck && !sticks(gy.youmonst.data)) {
        /* holding monster is next to hero rather than beneath, but
           hero is in no condition to actually sit at has/her own spot */
        if (humanoid(u.ustuck->data))
            pline("%s不会让你坐在%s膝上.", Monnam(u.ustuck),
                  mhis(u.ustuck));
        else
            pline("%s没有膝盖.", Monnam(u.ustuck));
        return ECMD_OK;
    } else if (is_pool(u.ux, u.uy) && !Underwater) { /* water walking */
        goto in_water;
    } else if (Upolyd && u.umonnum == PM_GREMLIN
               && (levl[u.ux][u.uy].typ == FOUNTAIN || is_pool(u.ux, u.uy))) {
        goto in_water;
    }

    if (OBJ_AT(u.ux, u.uy)
        /* ensure we're not standing on the precipice */
        && !(uteetering_at_seen_pit(trap) || uescaped_shaft(trap))) {
        struct obj *obj;

        obj = svl.level.objects[u.ux][u.uy];
        if (gy.youmonst.data->mlet == S_DRAGON && obj->oclass == COIN_CLASS) {
            You("盘腿绕在你的%s积蓄旁.",
                (obj->quan + money_cnt(gi.invent) < u.ulevel * 1000)
                ? "微薄" : "");
        } else if (obj->otyp == TOWEL) {
            pline("现在恐怕不适合野餐...");
        } else {
            if (slithy(gy.youmonst.data))
                You("盘腿绕在%s旁.", the(xname(obj)));
            else
                You("坐在%s上.", the(xname(obj)));
            if (obj->otyp == CORPSE && amorphous(&mons[obj->corpsenm]))
                pline("它软绵绵的...");
            else if (obj->otyp == CREAM_PIE) {
                 if (!Deaf) {
                   Soundeffect(se_squelch, 30);
                   pline("扑哧!");
                }
                useupf(obj, obj->quan);
            } else if (!(Is_box(obj)
                         || objects[obj->otyp].oc_material == CLOTH))
                pline("那不是很舒服...");
        }
    } else if (trap != 0 || (u.utrap && (u.utraptype >= TT_LAVA))) {
        if (u.utrap) {
            exercise(A_WIS, FALSE); /* you're getting stuck longer */
            if (u.utraptype == TT_BEARTRAP) {
                You_cant("在你的%s陷在捕兽夹时坐下.",
                         body_part(FOOT));
                u.utrap++;
            } else if (u.utraptype == TT_PIT) {
                if (trap && trap->ttyp == SPIKED_PIT) {
                    You("坐在尖刺上.哎哟!");
                    losehp(Half_physical_damage ? rn2(2) : 1,
                           "坐在尖刺上", KILLED_BY);
                    exercise(A_STR, FALSE);
                } else
                    You("坐在坑里.");
                u.utrap += rn2(5);
            } else if (u.utraptype == TT_WEB) {
                You("坐在蜘蛛网里,被缠得更紧了!");
                u.utrap += rn1(10, 5);
            } else if (u.utraptype == TT_LAVA) {
                /* Must have fire resistance or they'd be dead already */
                You("坐在%s里!", hliquid("熔岩"));
                if (Slimed)
                    burn_away_slime();
                u.utrap += rnd(4);
                losehp(d(2, 10), "坐在熔岩里",
                       KILLED_BY); /* lava damage */
            } else if (u.utraptype == TT_INFLOOR
                       || u.utraptype == TT_BURIEDBALL) {
                You_cant("挪动来坐下!");
                u.utrap++;
            }
        } else {
            /* when flying, "you land" might need some refinement; it sounds
               as if you're staying on the ground but you will immediately
               take off again unless you become stuck in a holding trap */
            You("%s了.", Flying ? "着陆" : "坐下");
            dotrap(trap, VIASITTING);
        }
    } else if ((Underwater || Is_waterlevel(&u.uz))
                && !eggs_in_water(gy.youmonst.data)) {
        if (Is_waterlevel(&u.uz))
            There("附近没有坐垫漂浮着.");
        else
            You("坐在泥泞的底部.");
    } else if (is_pool(u.ux, u.uy) && !eggs_in_water(gy.youmonst.data)) {
 in_water:
        You("坐在%s里.", hliquid("水"));
        if (Upolyd && u.umonnum == PM_GREMLIN) {
            if (split_mon(&gy.youmonst, (struct monst *) 0)) {
                if (levl[u.ux][u.uy].typ == FOUNTAIN)
                    dryup(u.ux, u.uy, TRUE);
            }
            /* splitting--or failing to do so--protects gear from the water */
        } else {
            if (!rn2(10) && uarm)
                (void) water_damage(uarm, "盔甲", TRUE);
            if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
                (void) water_damage(uarm, "盔甲", TRUE);
        }
    } else if (IS_SINK(typ)) {
        You(sit_message, defsyms[S_sink].explanation);
        Your("%s打湿了.",
             humanoid(gy.youmonst.data) ? "臀部" : "下部");
    } else if (IS_ALTAR(typ)) {
        You(sit_message, defsyms[S_altar].explanation);
        altar_wrath(u.ux, u.uy);
    } else if (IS_GRAVE(typ)) {
        You(sit_message, defsyms[S_grave].explanation);
    } else if (typ == STAIRS) {
        You(sit_message, "楼梯");
    } else if (typ == LADDER) {
        You(sit_message, "梯子");
    } else if (is_lava(u.ux, u.uy)) {
        /* must be WWalking */
        You(sit_message, hliquid("熔岩"));
        burn_away_slime();
        if (likes_lava(gy.youmonst.data)) {
            pline_The("%s感觉很暖和.", hliquid("熔岩"));
            return ECMD_TIME;
        }
        pline_The("%s烧伤了你!", hliquid("熔岩"));
        losehp(d((Fire_resistance ? 2 : 10), 10), /* lava damage */
               "坐在熔岩里", KILLED_BY);
    } else if (is_ice(u.ux, u.uy)) {
        You(sit_message, defsyms[S_ice].explanation);
        if (!Cold_resistance)
            pline_The("冰感觉很冷.");
    } else if (typ == DRAWBRIDGE_DOWN) {
        You(sit_message, "吊桥");
    } else if (IS_THRONE(typ)) {
        You(sit_message, defsyms[S_throne].explanation);
        throne_sit_effect();
    } else if (lays_eggs(gy.youmonst.data)) {
        return lay_an_egg();
    } else {
        pline("坐在%s上可没意思.", surface(u.ux, u.uy));
    }
    return ECMD_TIME;
}

/* curse a few inventory items at random! */
void
rndcurse(void)
{
    int nobj = 0;
    int cnt, onum;
    struct obj *otmp;
    static const char mal_aura[] = "感到一股邪恶的光晕环绕着%s.";

    if (u_wield_art(ART_MAGICBANE) && rn2(20)) {
        You(mal_aura, "吸收魔法的刀锋");
        return;
    }

    if (Antimagic) {
        shieldeff(u.ux, u.uy);
    }

    You(mal_aura, "你");

    for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
        /* gold isn't subject to being cursed or blessed */
        if (otmp->oclass == COIN_CLASS)
            continue;
        nobj++;
    }
    cnt = rnd(6 / ((!!Antimagic) + (!!Half_spell_damage) + 1));
    if (nobj) {
        for (; cnt > 0; cnt--) {
            onum = rnd(nobj);
            for (otmp = gi.invent; otmp; otmp = otmp->nobj) {
                /* as above */
                if (otmp->oclass == COIN_CLASS)
                    continue;
                if (--onum == 0)
                    break; /* found the target */
            }
            /* the !otmp case should never happen; picking an already
               cursed item happens--avoid "resists" message in that case */
            if (!otmp || otmp->cursed)
                continue; /* next target */

            if (otmp->oartifact && spec_ability(otmp, SPFX_INTEL)
                && rn2(10) < 8) {
                pline("%s!", Tobjnam(otmp, "在抵抗你"));
                continue;
            }

            if (otmp->blessed)
                unbless(otmp);
            else
                curse(otmp);
        }
        update_inventory();
    }

    /* treat steed's saddle as extended part of hero's inventory */
    if (u.usteed && !rn2(4) && (otmp = which_armor(u.usteed, W_SADDLE)) != 0
        && !otmp->cursed) { /* skip if already cursed */
        if (otmp->blessed)
            unbless(otmp);
        else
            curse(otmp);
        if (!Blind) {
            pline("%s%s.", Yobjnam2(otmp, "在发光"),
                  hcolor(otmp->cursed ? NH_BLACK : (const char *) "棕色"));
            otmp->bknown = Hallucination ? 0 : 1; /* bypass set_bknown() */
        } else {
            otmp->bknown = 0; /* bypass set_bknown() */
        }
    }
}

/* remove a random INTRINSIC ability from hero.
   returns the intrinsic property which was removed,
   or 0 if nothing was removed. */
int
attrcurse(void)
{
    int ret = 0;

    switch (rnd(11)) {
    case 1:
        if (HFire_resistance & INTRINSIC) {
            HFire_resistance &= ~INTRINSIC;
            You_feel("更热了.");
            ret = FIRE_RES;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 2:
        if (HTeleportation & INTRINSIC) {
            HTeleportation &= ~INTRINSIC;
            You_feel("你的位置不那么不稳定了.");
            ret = TELEPORT;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 3:
        if (HPoison_resistance & INTRINSIC) {
            HPoison_resistance &= ~INTRINSIC;
            You_feel("有点不适!");
            ret = POISON_RES;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 4:
        if (HTelepat & INTRINSIC) {
            HTelepat &= ~INTRINSIC;
            if (Blind && !Blind_telepat)
                see_monsters(); /* Can't sense mons anymore! */
            Your("心灵感应失效了!");
            ret = TELEPAT;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 5:
        if (HCold_resistance & INTRINSIC) {
            HCold_resistance &= ~INTRINSIC;
            You_feel("更冷了.");
            ret = COLD_RES;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 6:
        if (HInvis & INTRINSIC) {
            HInvis &= ~INTRINSIC;
            You_feel("更偏执了.");
            ret = INVIS;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 7:
        if (HSee_invisible & INTRINSIC) {
            HSee_invisible &= ~INTRINSIC;
            if (!See_invisible) {
                set_mimic_blocking();
                see_monsters();
                /* might not be able to see self anymore */
                newsym(u.ux, u.uy);
            }
            You("%s!", Hallucination ? "瞄见了一只大猫"
                                     : "以为你看到了什么东西");
            ret = SEE_INVIS;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 8:
        if (HFast & INTRINSIC) {
            HFast &= ~INTRINSIC;
            You_feel("你的速度变慢了.");
            ret = FAST;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 9:
        if (HStealth & INTRINSIC) {
            HStealth &= ~INTRINSIC;
            You_feel("更笨拙了.");
            ret = STEALTH;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 10:
        /* intrinsic protection is just disabled, not set back to 0 */
        if (HProtection & INTRINSIC) {
            HProtection &= ~INTRINSIC;
            You_feel("更易受伤害了.");
            ret = PROTECTION;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    case 11:
        if (HAggravate_monster & INTRINSIC) {
            HAggravate_monster &= ~INTRINSIC;
            You_feel("不那么引人注目了.");
            ret = AGGRAVATE_MONSTER;
            break;
        }
        FALLTHROUGH;
        /*FALLTHRU*/
    default:
        break;
    }
    return ret;
}

/*sit.c*/
