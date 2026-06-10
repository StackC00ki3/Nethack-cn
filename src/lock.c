/* NetHack 5.0	lock.c	$NHDT-Date: 1741793439 2025/03/12 07:30:39 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.145 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/*-Copyright (c) Robert Patrick Rankin, 2011. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* occupation callbacks */
staticfn int picklock(void);
staticfn int forcelock(void);

staticfn const char *lock_action(void);
staticfn boolean obstructed(coordxy, coordxy, boolean);
staticfn void chest_shatter_msg(struct obj *);

boolean
picking_lock(coordxy *x, coordxy *y)
{
    if (go.occupation == picklock) {
        *x = u.ux + u.dx;
        *y = u.uy + u.dy;
        return TRUE;
    } else {
        *x = *y = 0;
        return FALSE;
    }
}

boolean
picking_at(coordxy x, coordxy y)
{
    return (boolean) (go.occupation == picklock
                      && gx.xlock.door == &levl[x][y]);
}

/* produce an occupation string appropriate for the current activity */
staticfn const char *
lock_action(void)
{
    /* if the target is currently unlocked, we're trying to lock it now */
    if (gx.xlock.door && !(gx.xlock.door->doormask & D_LOCKED))
        return "锁上门";
    else if (gx.xlock.box && !gx.xlock.box->olocked)
        return gx.xlock.box->otyp == CHEST ? "锁上箱子" : "锁上盒子";
    /* otherwise we're trying to unlock it */
    else if (gx.xlock.picktyp == LOCK_PICK)
        return "撬锁";
    else if (gx.xlock.picktyp == CREDIT_CARD)
        return "撬锁"; /* same as lock_pick */
    else if (gx.xlock.door)
        return "打开门锁";
    else if (gx.xlock.box)
        return gx.xlock.box->otyp == CHEST ? "打开箱子锁" : "打开盒子锁";
    else
        return "撬锁";
}

/* try to open/close a lock */
staticfn int
picklock(void)
{
    if (gx.xlock.box) {
        if (gx.xlock.box->where != OBJ_FLOOR
            || gx.xlock.box->ox != u.ux || gx.xlock.box->oy != u.uy) {
            return ((gx.xlock.usedtime = 0)); /* you or it moved */
        }
    } else { /* door */
        if (gx.xlock.door != &(levl[u.ux + u.dx][u.uy + u.dy])) {
            return ((gx.xlock.usedtime = 0)); /* you moved */
        }
        switch (gx.xlock.door->doormask) {
        case D_NODOOR:
            pline("这个门口没有门.");
            return ((gx.xlock.usedtime = 0));
        case D_ISOPEN:
            You("不能锁打开的门.");
            return ((gx.xlock.usedtime = 0));
        case D_BROKEN:
            pline("这扇门坏了.");
            return ((gx.xlock.usedtime = 0));
        }
    }

    if (gx.xlock.usedtime++ >= 50 || nohands(gy.youmonst.data)) {
        You("放弃了尝试%s.", lock_action());
        exercise(A_DEX, TRUE); /* even if you don't succeed */
        return ((gx.xlock.usedtime = 0));
    }

    if (rn2(100) >= gx.xlock.chance)
        return 1; /* still busy */

    /* using the Master Key of Thievery finds traps if its bless/curse
       state is adequate (non-cursed for rogues, blessed for others;
       checked when setting up 'xlock') */
    if ((!gx.xlock.door ? (int) gx.xlock.box->otrapped
                       : (gx.xlock.door->doormask & D_TRAPPED) != 0)
        && gx.xlock.magic_key) {
        gx.xlock.chance += 20; /* less effort needed next time */
        if (!gx.xlock.door) {
            if (!gx.xlock.box->tknown)
                You("发现一个陷阱!");
            gx.xlock.box->tknown = 1;
        }
        if (y_n("要尝试解除它吗?") == 'y') {
            const char *what;
            boolean alreadyunlocked;

            /* disarming while using magic key always succeeds */
            if (gx.xlock.door) {
                gx.xlock.door->doormask &= ~D_TRAPPED;
                what = "门";
                alreadyunlocked = !(gx.xlock.door->doormask & D_LOCKED);
            } else {
                gx.xlock.box->otrapped = 0;
                gx.xlock.box->tknown = 0;
                what = (gx.xlock.box->otyp == CHEST) ? "箱子" : "盒子";
                alreadyunlocked = !gx.xlock.box->olocked;
            }
            You("成功解除了陷阱. %s仍然%s.",
                what, alreadyunlocked ? "没有上锁" : "锁着");
            exercise(A_WIS, TRUE);
        } else {
            You("停下了%s.", lock_action());
            exercise(A_WIS, FALSE);
        }
        return ((gx.xlock.usedtime = 0));
    }

    You("成功地%s.", lock_action());
    if (gx.xlock.door) {
        if (gx.xlock.door->doormask & D_TRAPPED) {
            b_trapped("门", FINGER);
            gx.xlock.door->doormask = D_NODOOR;
            unblock_point(u.ux + u.dx, u.uy + u.dy);
            if (*in_rooms(u.ux + u.dx, u.uy + u.dy, SHOPBASE))
                add_damage(u.ux + u.dx, u.uy + u.dy, SHOP_DOOR_COST);
            newsym(u.ux + u.dx, u.uy + u.dy);
        } else if (gx.xlock.door->doormask & D_LOCKED)
            gx.xlock.door->doormask = D_CLOSED;
        else
            gx.xlock.door->doormask = D_LOCKED;
    } else {
        gx.xlock.box->olocked = !gx.xlock.box->olocked;
        gx.xlock.box->lknown = 1;
        if (gx.xlock.box->otrapped)
            (void) chest_trap(gx.xlock.box, FINGER, FALSE);
    }
    exercise(A_DEX, TRUE);
    return ((gx.xlock.usedtime = 0));
}

void
breakchestlock(struct obj *box, boolean destroyit)
{
    if (!destroyit) { /* bill for the box but not for its contents */
        struct obj *hide_contents = box->cobj;

        box->cobj = 0;
        costly_alteration(box, COST_BRKLCK);
        box->cobj = hide_contents;
        box->olocked = 0;
        box->obroken = 1;
        box->lknown = 1;
    } else { /* #force has destroyed this box (at <u.ux,u.uy>) */
        struct obj *otmp;
        struct monst *shkp = (*u.ushops && costly_spot(u.ux, u.uy))
                                 ? shop_keeper(*u.ushops)
                                 : 0;
        boolean costly = (boolean) (shkp != 0),
                peaceful_shk = costly && (boolean) shkp->mpeaceful;
        long loss = 0L;

        pline("事实上, 你完全破坏了%s.", the(xname(box)));
        /* Put the contents on ground at the hero's feet. */
        while ((otmp = box->cobj) != 0) {
            obj_extract_self(otmp);
            if (!rn2(3) || otmp->oclass == POTION_CLASS) {
                chest_shatter_msg(otmp);
                if (costly)
                    loss += stolen_value(otmp, u.ux, u.uy, peaceful_shk,
                                         TRUE);
                if (otmp->quan == 1L) {
                    obfree(otmp, (struct obj *) 0);
                    continue;
                }
                /* this works because we're sure to have at least 1 left;
                   otherwise it would fail since otmp is not in inventory */
                useup(otmp);
            }
            if (box->otyp == ICE_BOX && otmp->otyp == CORPSE) {
                otmp->age = svm.moves - otmp->age; /* actual age */
                start_corpse_timeout(otmp);
            }
            place_object(otmp, u.ux, u.uy);
            stackobj(otmp);
        }
        if (costly)
            loss += stolen_value(box, u.ux, u.uy, peaceful_shk, TRUE);
        if (loss)
            You("因物品被毁, 欠 %ld %s.", loss, currency(loss));
        delobj(box);
    }
}

/* try to force a locked chest */
staticfn int
forcelock(void)
{
    if ((gx.xlock.box->ox != u.ux) || (gx.xlock.box->oy != u.uy))
        return ((gx.xlock.usedtime = 0)); /* you or it moved */

    if (gx.xlock.usedtime++ >= 50 || !uwep || nohands(gy.youmonst.data)) {
        You("放弃了撬锁的尝试.");
        if (gx.xlock.usedtime >= 50) /* you made the effort */
            exercise((gx.xlock.picktyp) ? A_DEX : A_STR, TRUE);
        return ((gx.xlock.usedtime = 0));
    }

    if (gx.xlock.picktyp) { /* blade */
        if (rn2(1000 - (int) uwep->spe) > (992 - greatest_erosion(uwep) * 10)
            && !uwep->cursed && !obj_resists(uwep, 0, 99)) {
            /* for a +0 weapon, probability that it survives an unsuccessful
             * attempt to force the lock is (.992)^50 = .67
             */
            pline("%s %s 打破了!", (uwep->quan > 1L) ? "你的一个" : "你的",
                  xname(uwep));
            useup(uwep);
            You("放弃了强行开锁的尝试.");
            exercise(A_DEX, TRUE);
            return ((gx.xlock.usedtime = 0));
        }
    } else             /* blunt */
        wake_nearby(FALSE); /* due to hammering on the container */

    if (rn2(100) >= gx.xlock.chance)
        return 1; /* still busy */

    You("成功地撬开了锁.");
    exercise(gx.xlock.picktyp ? A_DEX : A_STR, TRUE);
    /* breakchestlock() might destroy xlock.box; if so, xlock context will
       be cleared (delobj -> obfree -> maybe_reset_pick); but it might not,
       so explicitly clear that manually */
    breakchestlock(gx.xlock.box, (boolean) (!gx.xlock.picktyp && !rn2(3)));
    reset_pick(); /* lock-picking context is no longer valid */

    return 0;
}

void
reset_pick(void)
{
    gx.xlock.usedtime = gx.xlock.chance = gx.xlock.picktyp = 0;
    gx.xlock.magic_key = FALSE;
    gx.xlock.door = (struct rm *) 0;
    gx.xlock.box = (struct obj *) 0;
}

/* level change or object deletion; context may no longer be valid */
void
maybe_reset_pick(struct obj *container) /* passed from obfree() */
{
    /*
     * If a specific container, only clear context if it is for that
     * particular container (which is being deleted).  Other stuff on
     * the current dungeon level remains valid.
     * However if 'container' is Null, clear context if not carrying
     * gx.xlock.box (which might be Null if context is for a door).
     * Used for changing levels, where a floor container or a door is
     * being left behind and won't be valid on the new level but a
     * carried container will still be.  There might not be any context,
     * in which case redundantly clearing it is harmless.
     */
    if (container ? (container == gx.xlock.box)
                  : (!gx.xlock.box || !carried(gx.xlock.box)))
        reset_pick();
}

/* pick a tool for autounlock */
struct obj *
autokey(boolean opening) /* True: key, pick, or card; False: key or pick */
{
    struct obj *o, *key, *pick, *card, *akey, *apick, *acard;

    /* mundane item or regular artifact or own role's quest artifact */
    key = pick = card = (struct obj *) 0;
    /* other role's quest artifact (Rogue's Key or Tourist's Credit Card) */
    akey = apick = acard = (struct obj *) 0;
    for (o = gi.invent; o; o = o->nobj) {
        if (any_quest_artifact(o) && !is_quest_artifact(o)) {
            switch (o->otyp) {
            case SKELETON_KEY:
                if (!akey)
                    akey = o;
                break;
            case LOCK_PICK:
                if (!apick)
                    apick = o;
                break;
            case CREDIT_CARD:
                if (!acard)
                    acard = o;
                break;
            default:
                break;
            }
        } else {
            switch (o->otyp) {
            case SKELETON_KEY:
                if (!key || is_magic_key(&gy.youmonst, o))
                    key = o;
                break;
            case LOCK_PICK:
                if (!pick)
                    pick = o;
                break;
            case CREDIT_CARD:
                if (!card)
                    card = o;
                break;
            default:
                break;
            }
        }
    }
    if (!opening)
        card = acard = 0;
    /* only resort to other role's quest artifact if no other choice */
    if (!key && !pick && !card)
        key = akey;
    if (!pick && !card)
        pick = apick;
    if (!card)
        card = acard;
    return key ? key : pick ? pick : card ? card : 0;
}

DISABLE_WARNING_FORMAT_NONLITERAL

/* for doapply(); if player gives a direction or resumes an interrupted
   previous attempt then it usually costs hero a move even if nothing
   ultimately happens; when told "can't do that" before being asked for
   direction or player cancels with ESC while giving direction, it doesn't */
#define PICKLOCK_LEARNED_SOMETHING (-1) /* time passes */
#define PICKLOCK_DID_NOTHING 0          /* no time passes */
#define PICKLOCK_DID_SOMETHING 1

/* player is applying a key, lock pick, or credit card */
int
pick_lock(
    struct obj *pick,
    coordxy rx, coordxy ry, /* coordinates of door/container, for autounlock:
                             * doesn't prompt for direction if these are set */
    struct obj *container)  /* container, for autounlock */
{
    struct obj dummypick;
    int picktyp, c, ch;
    coord cc;
    struct rm *door;
    struct obj *otmp;
    char qbuf[QBUFSZ];
    boolean autounlock = (rx != 0 || container != NULL);

    /* 'pick' might be Null [called by do_loot_cont() for AUTOUNLOCK_UNTRAP] */
    if (!pick) {
        dummypick = cg.zeroobj;
        pick = &dummypick; /* pick->otyp will be STRANGE_OBJECT */
    }
    picktyp = pick->otyp;

    /* check whether we're resuming an interrupted previous attempt */
    if (gx.xlock.usedtime && picktyp == gx.xlock.picktyp) {
        static char no_longer[] = "很遗憾, 你再也无法%s%s了.";

        if (nohands(gy.youmonst.data)) {
            const char *what = (picktyp == LOCK_PICK) ? "撬锁工具" : "钥匙";

            if (picktyp == CREDIT_CARD)
                what = "卡片";
            pline(no_longer, "握住", what);
            reset_pick();
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (u.uswallow || (gx.xlock.box && !can_reach_floor(TRUE))) {
            pline(no_longer, "够到", "锁");
            reset_pick();
            return PICKLOCK_LEARNED_SOMETHING;
        } else {
            const char *action = lock_action();

            You("继续尝试%s.", action);
            gx.xlock.magic_key = is_magic_key(&gy.youmonst, pick);
            set_occupation(picklock, action, 0);
            return PICKLOCK_DID_SOMETHING;
        }
    }

    if (nohands(gy.youmonst.data)) {
        You_cant("拿着%s -- 你没有手!", doname(pick));
        return PICKLOCK_DID_NOTHING;
    } else if (u.uswallow) {
        You_cant("%s%s.", (picktyp == CREDIT_CARD) ? "解锁" : "锁上或解锁",
                 mon_nam(u.ustuck));
        return PICKLOCK_DID_NOTHING;
    }

    if (pick != &dummypick && picktyp != SKELETON_KEY
        && picktyp != LOCK_PICK && picktyp != CREDIT_CARD) {
        impossible("picking lock with object %d?", picktyp);
        return PICKLOCK_DID_NOTHING;
    }
    ch = 0; /* lint suppression */

    if (rx != 0) { /* autounlock; caller has provided coordinates */
        cc.x = rx;
        cc.y = ry;
    } else if (!get_adjacent_loc((char *) 0, "无效位置!",
                                 u.ux, u.uy, &cc)) {
        return PICKLOCK_DID_NOTHING;
    }

    if (u_at(cc.x, cc.y)) { /* pick lock on a container */
        const char *verb;
        char qsfx[QBUFSZ];
        boolean it;
        int count;

        if (u.dz < 0 && !autounlock) { /* beware stale u.dz value */
            pline("%s上方没有任何种类的锁.",
                  Levitation ? "这里" : "那里");
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (is_lava(u.ux, u.uy)) {
            pline("那样做可能会熔化%s.", yname(pick));
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (is_pool(u.ux, u.uy) && !Underwater) {
            pline_The("%s没有锁.", hliquid("水"));
            return PICKLOCK_LEARNED_SOMETHING;
        }

        count = 0;
        c = 'n'; /* in case there are no boxes here */
        for (otmp = svl.level.objects[cc.x][cc.y]; otmp;
             otmp = otmp->nexthere) {
            /* autounlock on boxes: only the one that was just discovered to
               be locked; don't include any other boxes which might be here */
            if (autounlock && otmp != container)
                continue;
            if (Is_box(otmp)) {
                ++count;
                if (!can_reach_floor(TRUE)) {
                    You_cant("从高处够到%s.", the(xname(otmp)));
                    return PICKLOCK_LEARNED_SOMETHING;
                }
                it = 0;
                if (otmp->obroken)
                    verb = "修理";
                else if (!otmp->olocked)
                    verb = "锁上", it = 1;
                else if (picktyp != LOCK_PICK)
                    verb = "打开", it = 1;
                else
                    verb = "撬开";

                if (autounlock && (flags.autounlock & AUTOUNLOCK_UNTRAP) != 0
                    && could_untrap(FALSE, TRUE)
                    && (c = otmp->tknown ? (otmp->otrapped ? 'y' : 'n')
                            : ynq(safe_qbuf(qbuf, "检查", "是否有陷阱?",
                                          otmp, yname, ysimple_name, "该物品")))
                       != 'n') {
                    if (c == 'q')
                        return PICKLOCK_DID_NOTHING; /* c == 'q' */
                    /* c == 'y' */
                    untrap(FALSE, 0, 0, otmp);
                    return PICKLOCK_DID_SOMETHING; /* even if no trap found */
                } else if (autounlock
                          && (flags.autounlock & AUTOUNLOCK_APPLY_KEY) != 0) {
                    c = 'q';
                    if (pick != &dummypick) {
                        Sprintf(qbuf, "用%s解锁它?", yname(pick));
                        c = ynq(qbuf);
                    }
                    if (c != 'y')
                        return PICKLOCK_DID_NOTHING;
                } else {
                    /* "There is <a box> here; <verb> <it|its lock>?" */
                    Sprintf(qsfx, "; %s%s?", verb, it ? "它" : "锁");
                    (void) safe_qbuf(qbuf, "这里有", qsfx, otmp, doname,
                                     ansimpleoname, "一个盒子");
                    otmp->lknown = 1;

                    c = ynq(qbuf);
                    if (c == 'q')
                        return PICKLOCK_DID_NOTHING;
                    if (c == 'n')
                        continue; /* try next box */
                }

                if (otmp->obroken) {
                    You_cant("用%s做那个.",
                             ansimpleoname(pick));
                    return PICKLOCK_LEARNED_SOMETHING;
                } else if (picktyp == CREDIT_CARD && !otmp->olocked) {
                    /* credit cards are only good for unlocking */
                    You_cant("用%s做不到.",
                             an(simple_typename(picktyp)));
                    return PICKLOCK_LEARNED_SOMETHING;
                } else if (autounlock
                           && !touch_artifact(pick, &gy.youmonst)) {
                    /* note: for !autounlock, apply already did touch check */
                    return PICKLOCK_DID_SOMETHING;
                }
                switch (picktyp) {
                case CREDIT_CARD:
                    ch = ACURR(A_DEX) + 20 * Role_if(PM_ROGUE);
                    break;
                case LOCK_PICK:
                    ch = 4 * ACURR(A_DEX) + 25 * Role_if(PM_ROGUE);
                    break;
                case SKELETON_KEY:
                    ch = 75 + ACURR(A_DEX);
                    break;
                default:
                    ch = 0;
                }
                if (otmp->cursed)
                    ch /= 2;

                gx.xlock.box = otmp;
                gx.xlock.door = 0;
                break;
            }
        }
        if (c != 'y') {
            if (!count)
                There("似乎没有任何类型的锁.");
            return PICKLOCK_LEARNED_SOMETHING; /* decided against all boxes */
        }

    /* not the hero's location; pick the lock in an adjacent door */
    } else {
        struct monst *mtmp;

        if (u.utrap && u.utraptype == TT_PIT) {
            You_cant("越过坑的边缘.");
            /* this used to return PICKLOCK_LEARNED_SOMETHING but the
               #open command doesn't use a turn for similar situation */
            return PICKLOCK_DID_NOTHING;
        }

        door = &levl[cc.x][cc.y];
        mtmp = m_at(cc.x, cc.y);
        if (mtmp && canseemon(mtmp) && M_AP_TYPE(mtmp) != M_AP_FURNITURE
            && M_AP_TYPE(mtmp) != M_AP_OBJECT) {
            if (picktyp == CREDIT_CARD
                && (mtmp->isshk || mtmp->data == &mons[PM_ORACLE])) {
                SetVoice(mtmp, 0, 80, 0);
                verbalize("不收支票, 不赊账, 没问题.");
            } else {
                pline("我觉得 %s 不会喜欢那样.",
                      mon_nam(mtmp));
            }
            return PICKLOCK_LEARNED_SOMETHING;
        } else if (mtmp && is_door_mappear(mtmp)) {
            /* "The door actually was a <mimic>!" */
            stumble_onto_mimic(mtmp);
            /* mimic might keep the key (50% chance, 10% for PYEC or MKoT) */
            maybe_absorb_item(mtmp, pick, 50, 10);
            return PICKLOCK_LEARNED_SOMETHING;
        }
        if (!IS_DOOR(door->typ)) {
            int res = PICKLOCK_DID_NOTHING, oldglyph = door->glyph;
            schar oldlastseentyp = update_mapseen_for(cc.x, cc.y);

            /* this is probably only relevant when blind */
            feel_location(cc.x, cc.y);
            if (door->glyph != oldglyph
                || svl.lastseentyp[cc.x][cc.y] != oldlastseentyp)
                res = PICKLOCK_LEARNED_SOMETHING;

            if (is_drawbridge_wall(cc.x, cc.y) >= 0)
                You("%s吊桥上没有锁.", Blind ? "感到" : "看到");
            else
                You("%s那里没有门.", Blind ? "感觉" : "看到");
            return res;
        }
        switch (door->doormask) {
        case D_NODOOR:
            pline("这个门口没有门.");
            return PICKLOCK_LEARNED_SOMETHING;
        case D_ISOPEN:
            You("不能锁打开的门.");
            return PICKLOCK_LEARNED_SOMETHING;
        case D_BROKEN:
            pline("这扇门坏了.");
            return PICKLOCK_LEARNED_SOMETHING;
        default:
            if ((flags.autounlock & AUTOUNLOCK_UNTRAP) != 0
                && could_untrap(FALSE, FALSE)
                && (c = ynq("检查这扇门是否有陷阱?")) != 'n') {
                if (c == 'q')
                    return PICKLOCK_DID_NOTHING;
                /* c == 'y' */
                untrap(FALSE, cc.x, cc.y, (struct obj *) 0);
                return PICKLOCK_DID_SOMETHING; /* even if no trap found */
            }
            /* credit cards are only good for unlocking */
            if (picktyp == CREDIT_CARD && !(door->doormask & D_LOCKED)) {
                You_cant("用信用卡锁门.");
                return PICKLOCK_LEARNED_SOMETHING;
            }

            if (autounlock)
                Sprintf(qbuf, "用%s%s它?", yname(pick),
                        (door->doormask & D_LOCKED) ? "解锁" : "锁上");
            else
                Sprintf(qbuf, "%s它?",
                        (door->doormask & D_LOCKED) ? "解锁" : "锁上");
            c = ynq(qbuf);
            if (c != 'y')
                return PICKLOCK_DID_NOTHING;

            /* note: for !autounlock, 'apply' already did touch check */
            if (autounlock && !touch_artifact(pick, &gy.youmonst))
                return PICKLOCK_DID_SOMETHING;

            switch (picktyp) {
            case CREDIT_CARD:
                ch = 2 * ACURR(A_DEX) + 20 * Role_if(PM_ROGUE);
                break;
            case LOCK_PICK:
                ch = 3 * ACURR(A_DEX) + 30 * Role_if(PM_ROGUE);
                break;
            case SKELETON_KEY:
                ch = 70 + ACURR(A_DEX);
                break;
            default:
                ch = 0;
            }
            gx.xlock.door = door;
            gx.xlock.box = 0;
        }
    }
    svc.context.move = 0;
    gx.xlock.chance = ch;
    gx.xlock.picktyp = picktyp;
    gx.xlock.magic_key = is_magic_key(&gy.youmonst, pick);
    gx.xlock.usedtime = 0;
    set_occupation(picklock, lock_action(), 0);
    return PICKLOCK_DID_SOMETHING;
}

/* is hero wielding a weapon that can #force? */
boolean
u_have_forceable_weapon(void)
{
    if (!uwep /* proper type test */
        || ((uwep->oclass == WEAPON_CLASS || is_weptool(uwep))
            ? (objects[uwep->otyp].oc_skill < P_DAGGER
               || objects[uwep->otyp].oc_skill == P_FLAIL
               || objects[uwep->otyp].oc_skill > P_LANCE)
            : uwep->oclass != ROCK_CLASS))
        return FALSE;
    return TRUE;
}

RESTORE_WARNING_FORMAT_NONLITERAL

/* the #force command - try to force a chest with your weapon */
int
doforce(void)
{
    struct obj *otmp;
    int c, picktyp;
    char qbuf[QBUFSZ];

    /*
     * TODO?
     *  allow force with edged weapon to be performed on doors.
     */

    if (u.uswallow) {
        You_cant("在里面强行做任何事情.");
        return ECMD_OK;
    }
    if (!u_have_forceable_weapon()) {
        boolean use_plural = uwep && uwep->quan > 1;

        You_cant("在%s武器%s的情况下用力推任何东西.",
                 !uwep ? "未装备"
                 : (uwep->oclass != WEAPON_CLASS && !is_weptool(uwep))
                   ? (use_plural ? "没有合适的" : "没有合适的")
                   : (use_plural ? "使用那些" : "使用那个"),
                 use_plural ? "" : "");
        return ECMD_OK;
    }
    if (!can_reach_floor(TRUE)) {
        cant_reach_floor(u.ux, u.uy, FALSE, TRUE, FALSE);
        return ECMD_OK;
    }

    picktyp = is_blade(uwep) && !is_pick(uwep);
    if (gx.xlock.usedtime && gx.xlock.box && picktyp == gx.xlock.picktyp) {
        You("重新开始尝试撬锁.");
        set_occupation(forcelock, "强行开锁", 0);
        return ECMD_TIME;
    }

    /* A lock is made only for the honest man, the thief will break it. */
    gx.xlock.box = (struct obj *) 0;
    for (otmp = svl.level.objects[u.ux][u.uy]; otmp; otmp = otmp->nexthere)
        if (Is_box(otmp)) {
            if (otmp->obroken || !otmp->olocked) {
                /* force doname() to omit known "broken" or "unlocked"
                   prefix so that the message isn't worded redundantly;
                   since we're about to set lknown, there's no need to
                   remember and then reset its current value */
                otmp->lknown = 0;
                There("有%s, 但是它的锁已经%s.",
                      doname(otmp), otmp->obroken ? "坏了" : "开了");
                otmp->lknown = 1;
                continue;
            }
            (void) safe_qbuf(qbuf, "这里有", "; 强行撬锁?",
                             otmp, doname, ansimpleoname, "一个盒子");
            otmp->lknown = 1;

            c = ynq(qbuf);
            if (c == 'q')
                return ECMD_OK;
            if (c == 'n')
                continue;

            if (picktyp)
                You("把%s伸进裂缝中然后使劲地撬动.", yname(uwep));
            else
                You("用%s使劲地砸.", yname(uwep));
            gx.xlock.box = otmp;
            gx.xlock.chance = objects[uwep->otyp].oc_wldam * 2;
            gx.xlock.picktyp = picktyp;
            gx.xlock.magic_key = FALSE;
            gx.xlock.usedtime = 0;
            break;
        }

    if (gx.xlock.box)
        set_occupation(forcelock, "强行开锁", 0);
    else
        You("决定不硬来.");
    return ECMD_TIME;
}

boolean
stumble_on_door_mimic(coordxy x, coordxy y)
{
    struct monst *mtmp;

    if ((mtmp = m_at(x, y)) && is_door_mappear(mtmp)
        && !Protection_from_shape_changers) {
        stumble_onto_mimic(mtmp);
        return TRUE;
    }
    return FALSE;
}

/* the #open command - try to open a door */
int
doopen(void)
{
    return doopen_indir(0, 0);
}

/* try to open a door in direction u.dx/u.dy */
int
doopen_indir(coordxy x, coordxy y)
{
    coord cc;
    struct rm *door;
    boolean portcullis;
    const char *dirprompt;
    int res = ECMD_OK;

    if (nohands(gy.youmonst.data)) {
        You_cant("打开任何东西 -- 你没有手!");
        return ECMD_OK;
    }

    dirprompt = NULL; /* have get_adjacent_loc() -> getdir() use default */
    if (u.utrap && u.utraptype == TT_PIT && container_at(u.ux, u.uy, FALSE))
        dirprompt = "打开哪里? [.>]";

    if (x > 0 && y >= 0) {
        /* nonzero <x,y> is used when hero in amorphous form tries to
           flow under a closed door at <x,y>; the test here was using
           'y > 0' but that would give incorrect results if doors are
           ever allowed to be placed on the top row of the map */
        cc.x = x;
        cc.y = y;
    } else if (!get_adjacent_loc(dirprompt, (char *) 0, u.ux, u.uy, &cc)) {
        return ECMD_OK;
    }

    /* open at yourself/up/down: switch to loot unless there is a closed
       door here (possible with Passes_walls) and direction isn't 'down' */
    if (u_at(cc.x, cc.y) && (u.dz > 0 || !closed_door(u.ux, u.uy)))
        return doloot();

    /* this used to be done prior to get_adjacent_loc() but doing so was
       incorrect once open at hero's spot became an alternate way to loot */
    if (u.utrap && u.utraptype == TT_PIT) {
        You_cant("够到坑边缘.");
        return ECMD_OK;
    }

    if (stumble_on_door_mimic(cc.x, cc.y))
        return ECMD_TIME;

    /* when choosing a direction is impaired, use a turn
       regardless of whether a door is successfully targeted */
    if (Confusion || Stunned)
        res = ECMD_TIME;

    door = &levl[cc.x][cc.y];
    portcullis = (is_drawbridge_wall(cc.x, cc.y) >= 0);
    /* this used to be 'if (Blind)' but using a key skips that so we do too */
    {
        int oldglyph = door->glyph;
        schar oldlastseentyp = update_mapseen_for(cc.x, cc.y);

        newsym(cc.x, cc.y);
        if (door->glyph != oldglyph
            || svl.lastseentyp[cc.x][cc.y] != oldlastseentyp)
            res = ECMD_TIME; /* learned something */
    }

    if (portcullis || !IS_DOOR(door->typ)) {
        /* closed portcullis or spot that opened bridge would span */
        if (is_db_wall(cc.x, cc.y) || door->typ == DRAWBRIDGE_UP)
            There("没有明显的方式来打开吊桥.");
        else if (portcullis || door->typ == DRAWBRIDGE_DOWN)
            pline_The("吊桥已经开了.");
        else if (container_at(cc.x, cc.y, TRUE))
            pline("%s那里有可搜刮的东西.",
                  Blind ? "感觉" : "好像");
        else
            You("%s那里没有门.", Blind ? "感觉" : "看到");
        return res;
    }

    if (!(door->doormask & D_CLOSED)) {
        const char *mesg;
        boolean locked = FALSE;

        switch (door->doormask) {
        case D_BROKEN:
            mesg = "坏了";
            break;
        case D_NODOOR:
            mesg = "口没有门";
            break;
        case D_ISOPEN:
            mesg = "已经打开了";
            break;
        default:
            mesg = "锁着";
            locked = TRUE;
            break;
        }
        set_msg_xy(cc.x, cc.y);
        pline("这扇门%s.", mesg);
        if (locked && flags.autounlock) {
            struct obj *unlocktool;

            u.dz = 0; /* should already be 0 since hero moved toward door */
            if ((flags.autounlock & AUTOUNLOCK_APPLY_KEY) != 0
                && (unlocktool = autokey(TRUE)) != 0) {
                res = pick_lock(unlocktool, cc.x, cc.y,
                                (struct obj *) 0) ? ECMD_TIME : ECMD_OK;
            } else if ((flags.autounlock & AUTOUNLOCK_KICK) != 0
                       && !u.usteed /* kicking is different when mounted */
                       && ynq("踢它?") == 'y') {
                cmdq_add_ec(CQ_CANNED, dokick);
                cmdq_add_dir(CQ_CANNED,
                             sgn(cc.x - u.ux), sgn(cc.y - u.uy), 0);
                /* this was 'ECMD_TIME', but time shouldn't elapse until
                   the canned kick takes place */
                res = ECMD_OK;
            }
        }
        return res;
    }

    if (verysmall(gy.youmonst.data)) {
        pline("你太小了, 拉不开门.");
        return res;
    }

    /* door is known to be CLOSED */
    if (rnl(20) < (ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 3) {
        set_msg_xy(cc.x, cc.y);
        pline_The("门开了.");
        if (door->doormask & D_TRAPPED) {
            b_trapped("门", FINGER);
            door->doormask = D_NODOOR;
            if (*in_rooms(cc.x, cc.y, SHOPBASE))
                add_damage(cc.x, cc.y, SHOP_DOOR_COST);
        } else
            door->doormask = D_ISOPEN;
        feel_newsym(cc.x, cc.y); /* the hero knows she opened it */
        recalc_block_point(cc.x, cc.y); /* vision: new see through there */
    } else {
        exercise(A_STR, TRUE);
        set_msg_xy(cc.x, cc.y);
        pline_The("门纹丝不动!");
    }

    return ECMD_TIME;
}

staticfn boolean
obstructed(coordxy x, coordxy y, boolean quietly)
{
    struct monst *mtmp = m_at(x, y);

    if (mtmp && M_AP_TYPE(mtmp) != M_AP_FURNITURE) {
        if (M_AP_TYPE(mtmp) == M_AP_OBJECT)
            goto objhere;
        if (!quietly) {
            char *Mn = Some_Monnam(mtmp); /* Monnam, Someone or Something */

            if ((mtmp->mx != x || mtmp->my != y) && canspotmon(mtmp))
                /* s_suffix() returns a modifiable buffer */
                Mn = strcat(s_suffix(Mn), "的尾巴");

            pline("%s挡住了路!", Mn);
        }
        if (!canspotmon(mtmp))
            map_invisible(x, y);
        return TRUE;
    }
    if (OBJ_AT(x, y)) {
 objhere:
        if (!quietly)
            pline("%s挡住了路.", Something);
        return TRUE;
    }
    return FALSE;
}

/* the #close command - try to close a door */
int
doclose(void)
{
    coordxy x, y;
    struct rm *door;
    boolean portcullis;
    int res = ECMD_OK;

    if (nohands(gy.youmonst.data)) {
        You_cant("关闭任何东西 -- 你没有手!");
        return ECMD_OK;
    }

    if (u.utrap && u.utraptype == TT_PIT) {
        You_cant("够到坑的边缘.");
        return ECMD_OK;
    }

    if (!getdir((char *) 0))
        return ECMD_CANCEL;

    x = u.ux + u.dx;
    y = u.uy + u.dy;
    if (u_at(x, y) && !Passes_walls) {
        You("挡在路上!");
        return ECMD_TIME;
    }

    if (!isok(x, y))
        goto nodoor;

    if (stumble_on_door_mimic(x, y))
        return ECMD_TIME;

    /* when choosing a direction is impaired, use a turn
       regardless of whether a door is successfully targeted */
    if (Confusion || Stunned)
        res = ECMD_TIME;

    door = &levl[x][y];
    portcullis = (is_drawbridge_wall(x, y) >= 0);
    if (Blind) {
        int oldglyph = door->glyph;
        schar oldlastseentyp = update_mapseen_for(x, y);

        feel_location(x, y);
        if (door->glyph != oldglyph
            || svl.lastseentyp[x][y] != oldlastseentyp)
            res = ECMD_TIME; /* learned something */
    }

    if (portcullis || !IS_DOOR(door->typ)) {
        /* is_db_wall: closed portcullis */
        if (is_db_wall(x, y) || door->typ == DRAWBRIDGE_UP)
            pline_The("吊桥已经关闭了.");
        else if (portcullis || door->typ == DRAWBRIDGE_DOWN)
            There("没有明显的方式来关闭吊桥.");
        else {
 nodoor:
            You("%s那里没有门.", Blind ? "感觉" : "看到");
        }
        return res;
    }

    if (door->doormask == D_NODOOR) {
        pline("这个门洞没有门.");
        return res;
    } else if (obstructed(x, y, FALSE)) {
        return res;
    } else if (door->doormask == D_BROKEN) {
        pline("这扇门坏了.");
        return res;
    } else if (door->doormask & (D_CLOSED | D_LOCKED)) {
        pline("你太小了, 不能拉上门.");
        return res;
    }

    if (door->doormask == D_ISOPEN) {
        if (verysmall(gy.youmonst.data) && !u.usteed) {
            pline("你太小了, 无法把门推上.");
            return res;
        }
        if (u.usteed
            || rn2(25) < (ACURRSTR + ACURR(A_DEX) + ACURR(A_CON)) / 3) {
            pline_The("门在抵抗!");
            door->doormask = D_CLOSED;
            feel_newsym(x, y); /* the hero knows she closed it */
            block_point(x, y); /* vision:  no longer see there */
        } else {
            exercise(A_STR, TRUE);
            pline_The("门纹丝不动!");
        }
    }

    return ECMD_TIME;
}

/* box obj was hit with spell or wand effect otmp;
   returns true if something happened */
boolean
boxlock(struct obj *obj, struct obj *otmp) /* obj *is* a box */
{
    boolean res = 0;

    switch (otmp->otyp) {
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        if (!obj->olocked) { /* lock it; fix if broken */
            Soundeffect(se_klunk, 50);
            pline("咔嗒!");
            obj->olocked = 1;
            obj->obroken = 0;
            if (Role_if(PM_WIZARD))
                obj->lknown = 1;
            else
                obj->lknown = 0;
            res = 1;
        } /* else already closed and locked */
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (obj->olocked) { /* unlock; isn't broken so doesn't need fixing */
            Soundeffect(se_klick, 50);
            pline("咔玲!");
            obj->olocked = 0;
            res = 1;
            if (Role_if(PM_WIZARD))
                obj->lknown = 1;
            else
                obj->lknown = 0;
        } else /* silently fix if broken */
            obj->obroken = 0;
        break;
    case WAN_POLYMORPH:
    case SPE_POLYMORPH:
        /* maybe start unlocking chest, get interrupted, then zap it;
           we must avoid any attempt to resume unlocking it */
        if (gx.xlock.box == obj)
            reset_pick();
        break;
    }
    return res;
}

/* Door/secret door was hit with spell or wand effect otmp;
   returns true if something happened */
boolean
doorlock(struct obj *otmp, coordxy x, coordxy y)
{
    struct rm *door = &levl[x][y];
    boolean res = TRUE;
    int loudness = 0;
    const char *msg = (const char *) 0;
    const char *dustcloud = "一团尘云";
    const char *quickly_dissipates = "迅速消散";
    boolean mysterywand = (otmp->oclass == WAND_CLASS && !otmp->dknown);

    if (door->typ == SDOOR) {
        switch (otmp->otyp) {
        case WAN_OPENING:
        case SPE_KNOCK:
        case WAN_STRIKING:
        case SPE_FORCE_BOLT:
            door->typ = DOOR;
            door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
            newsym(x, y);
            if (cansee(x, y))
                pline("墙里出现了一道门!");
            if (otmp->otyp == WAN_OPENING || otmp->otyp == SPE_KNOCK)
                return TRUE;
            break; /* striking: continue door handling below */
        case WAN_LOCKING:
        case SPE_WIZARD_LOCK:
        default:
            return FALSE;
        }
    }

    switch (otmp->otyp) {
    case WAN_LOCKING:
    case SPE_WIZARD_LOCK:
        if (Is_rogue_level(&u.uz)) {
            boolean vis = cansee(x, y);

            /* Can't have real locking in Rogue, so just hide doorway */
            if (vis) {
                pline("%s在更古老, 更原始的门口涌现.",
                      dustcloud);
            } else {
                Soundeffect(se_swoosh, 25);
                You_hear("哗哗响.");
            }
            if (obstructed(x, y, mysterywand)) {
                if (vis)
                    pline_The("尘云%s.", quickly_dissipates);
                return FALSE;
            }
            block_point(x, y);
            door->typ = SDOOR, door->doormask = D_NODOOR;
            if (vis)
                pline_The("门口消失了!");
            newsym(x, y);
            return TRUE;
        }
        if (obstructed(x, y, mysterywand))
            return FALSE;
        /* Don't allow doors to close over traps.  This is for pits */
        /* & trap doors, but is it ever OK for anything else? */
        if (t_at(x, y)) {
            /* maketrap() clears doormask, so it should be NODOOR */
            pline("%s在门口涌现, 但是%s.", dustcloud,
                  quickly_dissipates);
            return FALSE;
        }

        switch (door->doormask & ~D_TRAPPED) {
        case D_CLOSED:
            msg = "门锁上了!";
            break;
        case D_ISOPEN:
            msg = "门砰地关上并锁住了!";
            break;
        case D_BROKEN:
            msg = "坏门重新组合并锁住了!";
            break;
        case D_NODOOR:
            msg =
               "一团尘云升起, 组合成了一扇门!";
            break;
        default:
            res = FALSE;
            break;
        }
        block_point(x, y);
        door->doormask = D_LOCKED | (door->doormask & D_TRAPPED);
        newsym(x, y);
        break;
    case WAN_OPENING:
    case SPE_KNOCK:
        if (door->doormask & D_LOCKED) {
            msg = "门解锁了!";
            door->doormask = D_CLOSED | (door->doormask & D_TRAPPED);
        } else
            res = FALSE;
        break;
    case WAN_STRIKING:
    case SPE_FORCE_BOLT:
        if (door->doormask & (D_LOCKED | D_CLOSED)) {
            /* sawit: closed door location is more visible than open */
            boolean sawit, seeit;

            if (door->doormask & D_TRAPPED) {
                struct monst *mtmp = m_at(x, y);

                sawit = mtmp ? canseemon(mtmp) : cansee(x, y);
                door->doormask = D_NODOOR;
                unblock_point(x, y);
                newsym(x, y);
                seeit = mtmp ? canseemon(mtmp) : cansee(x, y);
                if (mtmp) {
                    (void) mb_trapped(mtmp, sawit || seeit);
                } else {
                    /* for mtmp, mb_trapped() does is own wake_nearto() */
                    loudness = 40;
                    if (flags.verbose) {
                        Soundeffect(se_kaboom_door_explodes, 75);
                        if ((sawit || seeit) && !Unaware) {
                            pline("轰隆!! 你看到一扇门爆炸了.");
                        } else if (!Deaf) {
                            Soundeffect(se_explosion, 75);
                            You_hear("一声%s爆炸.",
                                     (distu(x, y) > 7 * 7) ? "远处"
                                                           : "附近");
                        }
                    }
                }
                break;
            }
            sawit = cansee(x, y);
            door->doormask = D_BROKEN;
            recalc_block_point(x, y);
            seeit = cansee(x, y);
            newsym(x, y);
            if (flags.verbose) {
                if ((sawit || seeit) && !Unaware) {
                    pline_The("门崩溃打开了!");
                } else if (!Deaf) {
                    Soundeffect(se_crashing_sound, 100);
                    You_hear("崩溃声.");
                }
            }
            /* force vision recalc before printing more messages */
            if (gv.vision_full_recalc)
                vision_recalc(0);
            loudness = 20;
        } else
            res = FALSE;
        break;
    default:
        impossible("magic (%d) attempted on door.", otmp->otyp);
        break;
    }
    if (msg && cansee(x, y))
        pline1(msg);
    if (loudness > 0) {
        /* door was destroyed */
        wake_nearto(x, y, loudness);
        if (*in_rooms(x, y, SHOPBASE))
            add_damage(x, y, 0L);
    }

    if (res && picking_at(x, y)) {
        /* maybe unseen monster zaps door you're unlocking */
        stop_occupation();
        reset_pick();
    }
    return res;
}

staticfn void
chest_shatter_msg(struct obj *otmp)
{
    const char *disposition;
    const char *thing;
    long save_HBlinded, save_BBlinded;

    if (otmp->oclass == POTION_CLASS) {
        You("%s%s碎裂了!", Blind ? "听到" : "看到", an(bottlename()));
        if (!breathless(gy.youmonst.data) || haseyes(gy.youmonst.data))
            potionbreathe(otmp);
        return;
    }
    /* We have functions for distant and singular names, but not one */
    /* which does _both_... */
    save_HBlinded = HBlinded,  save_BBlinded = BBlinded;
    HBlinded = 1L,  BBlinded = 0L;
    thing = singular(otmp, xname);
    HBlinded = save_HBlinded,  BBlinded = save_BBlinded;
    switch (objects[otmp->otyp].oc_material) {
    case PAPER:
        disposition = "被撕成碎片";
        break;
    case WAX:
        disposition = "被压碎";
        break;
    case VEGGY:
        disposition = "被捣成浆";
        break;
    case FLESH:
        disposition = "被捣烂";
        break;
    case GLASS:
        disposition = "碎裂了";
        break;
    case WOOD:
        disposition = "裂成碎片";
        break;
    default:
        disposition = "被毁掉了";
        break;
    }
    pline("%s%s!", An(thing), disposition);
}

/*lock.c*/
