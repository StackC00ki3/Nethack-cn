/* NetHack 5.0	write.c	$NHDT-Date: 1702023275 2023/12/08 08:14:35 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.41 $ */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

staticfn int cost(struct obj *) NONNULLARG1;
staticfn int write_ok(struct obj *) NO_NNARGS;
staticfn char *new_book_description(int, char *) NONNULL NONNULLPTRS;

/*
 * returns base cost of a scroll or a spellbook
 */
staticfn int
cost(struct obj *otmp)
{
    if (otmp->oclass == SPBOOK_CLASS)
        return (10 * objects[otmp->otyp].oc_level);

    switch (otmp->otyp) {
#ifdef MAIL_STRUCTURES
    case SCR_MAIL:
        return 2;
#endif
    case SCR_LIGHT:
    case SCR_GOLD_DETECTION:
    case SCR_FOOD_DETECTION:
    case SCR_MAGIC_MAPPING:
    case SCR_AMNESIA:
    case SCR_FIRE:
    case SCR_EARTH:
        return 8;
    case SCR_DESTROY_ARMOR:
    case SCR_CREATE_MONSTER:
    case SCR_PUNISHMENT:
        return 10;
    case SCR_CONFUSE_MONSTER:
        return 12;
    case SCR_IDENTIFY:
        return 14;
    case SCR_ENCHANT_ARMOR:
    case SCR_REMOVE_CURSE:
    case SCR_ENCHANT_WEAPON:
    case SCR_CHARGING:
        return 16;
    case SCR_SCARE_MONSTER:
    case SCR_STINKING_CLOUD:
    case SCR_TAMING:
    case SCR_TELEPORTATION:
        return 20;
    case SCR_GENOCIDE:
        return 30;
    case SCR_BLANK_PAPER:
    default:
        impossible("You can't write such a weird scroll!");
    }
    return 1000;
}

/* getobj callback for object to write on */
staticfn int
write_ok(struct obj *obj)
{
    if (!obj || (obj->oclass != SCROLL_CLASS && obj->oclass != SPBOOK_CLASS))
        return GETOBJ_EXCLUDE;

    if (obj->otyp == SCR_BLANK_PAPER || obj->otyp == SPE_BLANK_PAPER)
        return GETOBJ_SUGGEST;

    return GETOBJ_DOWNPLAY;
}

/* write -- applying a magic marker */
int
dowrite(struct obj *pen)
{
    struct obj *paper;
    char namebuf[BUFSZ] = DUMMY, *nm, *bp;
    struct obj *new_obj;
    int basecost, actualcost;
    int curseval;
    char qbuf[QBUFSZ];
    int first, last, i, deferred, deferralchance, real;
    boolean by_descr = FALSE;
    const char *typeword;
    int spell_knowledge;

    if (nohands(gy.youmonst.data)) {
        You("需要有手才能写!");
        return ECMD_OK;
    } else if (Glib) {
        pline("%s从你的%s上滑落.", Tobjnam(pen, ""),
              fingers_or_gloves(FALSE));
        dropx(pen);
        return ECMD_TIME;
    }

    /* get paper to write on */
    paper = getobj("写在什么上", write_ok, GETOBJ_NOFLAGS);
    if (!paper)
        return ECMD_CANCEL;
    /* can't write on a novel (unless/until it's been converted into a blank
       spellbook), but we want messages saying so to avoid "spellbook" */
    typeword = (paper->otyp == SPE_NOVEL) ? "书"
               : (paper->oclass == SPBOOK_CLASS) ? "魔法书"
                 : "卷轴";
    if (Blind) {
        if (!paper->dknown) {
            You("不知道那个%s是否空白.", typeword);
            return ECMD_OK;
        } else if (paper->oclass == SPBOOK_CLASS) {
            /* can't write a magic book while blind */
            pline("%s不能写盲文.",
                  upstart(ysimple_name(pen)));
            return ECMD_OK;
        }
    }
    observe_object(paper);
    if (paper->otyp != SCR_BLANK_PAPER && paper->otyp != SPE_BLANK_PAPER) {
        pline("那个%s不是空白的!", typeword);
        exercise(A_WIS, FALSE);
        return ECMD_TIME;
    }
    makeknown(SCR_BLANK_PAPER);

    /* what to write */
    Sprintf(qbuf, "你想要写哪种%s?", typeword);
    getlin(qbuf, namebuf);
    (void) mungspaces(namebuf); /* remove any excess whitespace */
    if (namebuf[0] == '\033' || !namebuf[0])
        return ECMD_TIME;
    nm = namebuf;
    if (!strncmpi(nm, "scroll ", 7))
        nm += 7;
    else if (!strncmpi(nm, "spellbook ", 10))
        nm += 10;
    else if (!strncmp(nm, "卷轴", sizeof "卷轴" - 1))
        nm += (sizeof "卷轴" - 1);
    else if (!strncmp(nm, "魔法书", sizeof "魔法书" - 1))
        nm += (sizeof "魔法书" - 1);
    if (*nm == ' ')
        nm++;
    if (!strncmpi(nm, "of ", 3))
        nm += 3;

    if ((bp = strstri(nm, " armour")) != 0) {
        memcpy(bp, " armor ", 7);
        (void) mungspaces(bp + 1);        /* remove the extra space */
    }

    deferred = real = 0; /* not any scroll or book */
    deferralchance = 0;  /* incremented for each oc_uname match */
    first = svb.bases[(int) paper->oclass];
    last = svb.bases[(int) paper->oclass + 1] - 1;
    /* first loop: look for match with name/description */
    for (i = first; i <= last; i++) {
        /* extra shufflable descr not representing a real object */
        if (!OBJ_NAME(objects[i]))
            continue;

        if (!strcmpi(OBJ_NAME(objects[i]), nm)) {
            if (objects[i].oc_name_known
                /* spellbooks can only be written by_name, so no need to
                   hold out for a 'better' by_descr match */
                || paper->oclass == SPBOOK_CLASS) {
                goto found;
            } else {
                /* save item in case there are no better by_descr matches */
                real = deferred = i;
                break;
            }
        }

        if (!strcmpi(OBJ_DESCR(objects[i]), nm)) {
            by_descr = TRUE;
            goto found;
        }
    }
    /* second loop: look for match with user-assigned name */
    /* we will get here if 'nm' isn't a real scroll name/descr, or is the name
     * of a real scroll that hasn't been formally IDed. */
    for (i = first; i <= last; i++) {
        /* player might assign same name multiple times and if so,
           we choose one of those matches randomly */
        if (objects[i].oc_uname && !strcmpi(objects[i].oc_uname, nm)
            /* prefer attempting to write the real scroll type if
               the typename clobbers a real scroll and is known to
               be incorrect */
            && !(real && objects[i].oc_name_known)
            /*
             * First match: chance incremented to 1,
             *   !rn2(1) is 1, we remember i;
             * second match: chance incremented to 2,
             *   !rn2(2) has 1/2 chance to replace i;
             * third match: chance incremented to 3,
             *   !rn2(3) has 1/3 chance to replace i
             *   and 2/3 chance to keep previous 50:50
             *   choice; so on for higher match counts.
             */
            && !rn2(++deferralchance)) {
            deferred = i;
            /* writing by user-assigned name is same as by description:
               fails for books, works for scrolls (having an assigned
               type name guarantees presence on discoveries list) */
            by_descr = TRUE;
        }
    }

    if (deferred) {
        i = deferred;
        goto found;
    }

    There("没有这种%s!", typeword);
    return ECMD_TIME;
 found:

    if (i == SCR_BLANK_PAPER || i == SPE_BLANK_PAPER) {
        You_cant("写那个!");
        pline("这太不像话了!");
        return ECMD_TIME;
    } else if (i == SPE_NOVEL) {
        boolean fanfic = !rn2(3), tearup = !rn2(3);

        if (!fanfic) {
            You("%s写伟大的岩德利亚人小说,但%s.",
                !tearup ? "准备" : "尝试",
                !Hallucination ? "缺乏灵感" : "灵感太多了写不下");
        } else {
            You("%s创作出真正%s的同人小说.",
                !tearup ? "开始" : "",
                !Hallucination ? "糟糕的" : "精彩的");
        }
        if (!tearup) {
            You("放弃这个想法.");
        } else {
            You("把它撕碎.");
            useup(paper);
        }
        return ECMD_TIME;
    } else if (i == SPE_BOOK_OF_THE_DEAD) {
        pline("普通地下城冒险者可写不出那个.");
        return ECMD_TIME;
    } else if (by_descr && paper->oclass == SPBOOK_CLASS
               && !objects[i].oc_name_known) {
        /* can't write unknown spellbooks by description */
        pline("不幸的是,你没有足够的知识继续写.");
        return ECMD_TIME;
    }

    /* KMH, conduct */
    if (!u.uconduct.literate++)
        livelog_printf(LL_CONDUCT,
                       "因写%s而脱离文盲", an(typeword));

    new_obj = mksobj(i, FALSE, FALSE);
    new_obj->bknown = (paper->bknown && pen->bknown);

    /* shk imposes a flat rate per use, not based on actual charges used */
    check_unpaid(pen);

    /* see if there's enough ink */
    basecost = cost(new_obj);
    if (pen->spe < basecost / 2) {
        Your("魔笔太干了,写不了!");
        obfree(new_obj, (struct obj *) 0);
        return ECMD_TIME;
    }

    /* we're really going to write now, so calculate cost
     */
    actualcost = rn1(basecost / 2, basecost / 2);
    curseval = bcsign(pen) + bcsign(paper);
    exercise(A_WIS, TRUE);
    /* dry out marker */
    if (pen->spe < actualcost) {
        pen->spe = 0;
        Your("魔笔干了!");
        /* scrolls disappear, spellbooks don't */
        if (paper->oclass == SPBOOK_CLASS) {
            pline_The("魔法书没写完,而且你写的字消失了.");
            update_inventory(); /* pen charges */
        } else {
            pline_The("卷轴现在已经没用了,然后消失了!");
            useup(paper);
        }
        obfree(new_obj, (struct obj *) 0);
        return ECMD_TIME;
    }
    pen->spe -= actualcost;

    /*
     * Writing by name requires that the hero knows the scroll or
     * book type.  One has previously been read (and its effect
     * was evident) or been ID'd via scroll/spell/throne (or skill
     * for Wizards) and it will be on the discoveries list.
     * Unknown spellbooks can also be written by name if the hero
     * has fresh knowledge of the spell, or if the spell is almost
     * forgotten and the hero is Lucky (with a greater chance than
     * if the spell is unknown or forgotten).
     * (Previous versions allowed scrolls and books to be written
     * by type name if they were on the discoveries list via being
     * given a user-assigned name, even though doing the latter
     * doesn't--and shouldn't--make the actual type become known.)
     *
     * Writing by description requires that the hero knows the
     * description (a scroll's label, that is, since books by_descr
     * are rejected above).  This is done by checking to see if a
     * scroll with the same description has been encountered.
     *
     * Normal requirements can be overridden if hero is Lucky.
     */

    if (paper->oclass == SPBOOK_CLASS) {
        spell_knowledge = known_spell(new_obj->otyp);
    } else {
        spell_knowledge = spe_Unknown;
    }
    /* if known, then either by-name or by-descr works */
    if (!objects[new_obj->otyp].oc_name_known
        /* else if named, then only by-descr works */
        && !(by_descr && objects[new_obj->otyp].oc_encountered)
        /* else fresh knowledge of the spell works */
        && spell_knowledge != spe_Fresh
        /* and Luck might override after previous checks have failed */
        && rnl(((Role_if(PM_WIZARD) && paper->oclass != SPBOOK_CLASS)
                || spell_knowledge == spe_GoingStale)
               ? 5 : 15)) {
        You("%s那个.", by_descr ? "没能成功写出" : "不知道怎么写");
        /* scrolls disappear, spellbooks don't */
        if (paper->oclass == SPBOOK_CLASS) {
            You(
      "写上你最好的字:\"我的日记\",但它很快消退了.");
            update_inventory(); /* pen charges */
        } else {
            if (by_descr) {
                Strcpy(namebuf, OBJ_DESCR(objects[new_obj->otyp]));
                wipeout_text(namebuf, (6 + MAXULEV - u.ulevel) / 6, 0);
            } else
                Sprintf(namebuf, "%s到此一游!", svp.plname);
            You("写上\"%s\",然后卷轴消失了.", namebuf);
            useup(paper);
        }
        obfree(new_obj, (struct obj *) 0);
        return ECMD_TIME;
    }
    /* can write scrolls when blind, but requires luck too;
       attempts to write books when blind are caught above */
    if (Blind && rnl(3)) {
        /* writing while blind usually fails regardless of
           whether the target scroll is known; even if we
           have passed the write-an-unknown scroll test
           above we can still fail this one, so it's doubly
           hard to write an unknown scroll while blind */
        You("无法正确地写卷轴,然后它消失了.");
        useup(paper);
        obfree(new_obj, (struct obj *) 0);
        return ECMD_TIME;
    }

    /* use up old scroll / spellbook */
    useup(paper);

    /* success */
    if (new_obj->oclass == SPBOOK_CLASS) {
        /* acknowledge the change in the object's description... */
        pline_The("魔法书奇怪地扭曲,然后变%s.",
                  new_book_description(new_obj->otyp, namebuf));
    }
    new_obj->blessed = (curseval > 0);
    new_obj->cursed = (curseval < 0);
#ifdef MAIL_STRUCTURES
    if (new_obj->otyp == SCR_MAIL)
        /* 0: delivered in-game via external event (or randomly for fake mail);
           1: from bones or wishing; 2: written with marker */
        new_obj->spe = 2;
#endif
    /* unlike alchemy, for example, a successful result yields the
       specifically chosen item so hero recognizes it even if blind;
       the exception is for being lucky writing an undiscovered scroll,
       where the label associated with the type-name isn't known yet;
       but if writing by description, the description is always known */
    new_obj->dknown = FALSE;
    if (objects[new_obj->otyp].oc_name_known || by_descr)
        observe_object(new_obj);

    new_obj = hold_another_object(new_obj, "哦不!%s出了你的手中!",
                                  The(aobjnam(new_obj, "滑落")),
                                  (const char *) 0);
    nhUse(new_obj); /* try to avoid complaint about dead assignment */
    return ECMD_TIME;
}

/* most book descriptions refer to cover appearance, so we can issue a
   message for converting a plain book into one of those with something
   like "the spellbook turns red" or "the spellbook turns ragged";
   but some descriptions refer to composition and "the book turns vellum"
   looks funny, so we want to insert "into " prior to such descriptions;
   even that's rather iffy, indicating that such descriptions probably
   ought to be eliminated (especially "cloth"!) */
staticfn char *
new_book_description(int booktype, char *outbuf)
{
    /* subset of description strings from objects.c; if it grows
       much, we may need to add a new flag field to objects[] instead */
    static const char *const compositions[] = {
        "羊皮纸",
        "牛皮纸",
        "布",
#if 0
        "平装", "精装", /* not used */
        "莎草纸", /* not applicable--can't be produced via writing */
#endif /*0*/
        0
    };
    const char *descr, *const *comp_p;

    descr = OBJ_DESCR(objects[booktype]);
    for (comp_p = compositions; *comp_p; ++comp_p)
        if (!strcmpi(descr, *comp_p))
            break;

    Sprintf(outbuf, "%s%s", *comp_p ? "成" : "", descr);
    return outbuf;
}

/*write.c*/
