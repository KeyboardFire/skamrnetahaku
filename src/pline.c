/* NetHack 3.6	pline.c	$NHDT-Date: 1432512770 2015/05/25 00:12:50 $  $NHDT-Branch: master $:$NHDT-Revision: 1.42 $ */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#define NEED_VARARGS /* Uses ... */ /* comment line for pre-compiled headers \
                                       */
#include "hack.h"

static boolean no_repeat = FALSE;
static char prevmsg[BUFSZ];

static char *FDECL(You_buf, (int));

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vpline, (const char *, va_list));

void pline
VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, char *);
    vpline(line, VA_ARGS);
    VA_END();
}

# ifdef USE_STDARG
static void
vpline(const char *line, va_list the_args)
# else
static void
vpline(line, the_args)
const char *line;
va_list the_args;
# endif

#else /* USE_STDARG | USE_VARARG */

# define vpline pline

void pline
VA_DECL(const char *, line)
#endif /* USE_STDARG | USE_VARARG */
{       /* start of vpline() or of nested block in USE_OLDARG's pline() */
    char pbuf[3 * BUFSZ];
    int ln;
    xchar msgtyp;
    /* Do NOT use VA_START and VA_END in here... see above */

    if (!line || !*line)
        return;
#ifdef HANGUPHANDLING
    if (program_state.done_hup)
        return;
#endif
    if (program_state.wizkit_wishing)
        return;

    if (index(line, '%')) {
        Vsprintf(pbuf, line, VA_ARGS);
        line = pbuf;
    }
    if ((ln = (int) strlen(line)) > BUFSZ - 1) {
        if (line != pbuf)                          /* no '%' was present */
            (void) strncpy(pbuf, line, BUFSZ - 1); /* caveat: unterminated */
        /* truncate, preserving the final 3 characters:
           "___ extremely long text" -> "___ extremely l...ext"
           (this may be suboptimal if overflow is less than 3) */
        (void) strncpy(pbuf + BUFSZ - 1 - 6, "...", 3);
        /* avoid strncpy; buffers could overlap if excess is small */
        pbuf[BUFSZ - 1 - 3] = line[ln - 3];
        pbuf[BUFSZ - 1 - 2] = line[ln - 2];
        pbuf[BUFSZ - 1 - 1] = line[ln - 1];
        pbuf[BUFSZ - 1] = '\0';
        line = pbuf;
    }
    if (!iflags.window_inited) {
        raw_print(line);
        iflags.last_msg = PLNMSG_UNKNOWN;
        return;
    }
#ifndef MAC
    if (no_repeat && !strcmp(line, toplines))
        return;
#endif /* MAC */
    if (vision_full_recalc)
        vision_recalc(0);
    if (u.ux)
        flush_screen(1); /* %% */
    msgtyp = msgtype_type(line);
    if (msgtyp == MSGTYP_NOSHOW) return;
    if (msgtyp == MSGTYP_NOREP && !strcmp(line, prevmsg)) return;
    putstr(WIN_MESSAGE, 0, line);
    /* this gets cleared after every pline message */
    iflags.last_msg = PLNMSG_UNKNOWN;
    strncpy(prevmsg, line, BUFSZ);
    if (msgtyp == MSGTYP_STOP) display_nhwindow(WIN_MESSAGE, TRUE); /* --more-- */

#if !(defined(USE_STDARG) || defined(USE_VARARGS))
    /* provide closing brace for the nested block
       which immediately follows USE_OLDARGS's VA_DECL() */
    VA_END();
#endif
}

/*VARARGS1*/
void Norep
VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, const char *);
    no_repeat = TRUE;
    vpline(line, VA_ARGS);
    no_repeat = FALSE;
    VA_END();
    return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char *
You_buf(siz)
int siz;
{
    if (siz > you_buf_siz) {
        if (you_buf)
            free((genericptr_t) you_buf);
        you_buf_siz = siz + 10;
        you_buf = (char *) alloc((unsigned) you_buf_siz);
    }
    return you_buf;
}

void
free_youbuf()
{
    if (you_buf)
        free((genericptr_t) you_buf), you_buf = (char *) 0;
    you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer, prefix, text) \
    Strcpy((pointer = You_buf((int) (strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer, prefix, text) \
    strcat((YouPrefix(pointer, prefix, text), pointer), text)

/*VARARGS1*/
void You
VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, ".i do ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void Your
VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, ".i lo do ", line), VA_ARGS);
    VA_END();
}

// LOJBAN TRANSLATION NOTE
// "Feel" is an ambiguous word. REMOVE ALL CALLS TO YOU_FEEL AND REPLACE THEM
// WITH CALLS TO YOU. This function will be removed / commented out once that
// is done.
/*VARARGS1*/
void You_feel
VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    if (Unaware)
        YouPrefix(tmp, "You dream that you feel ", line);
    else
        YouPrefix(tmp, "You feel ", line);
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_cant
VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, ".i do na kakne ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void pline_The
VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, ".i lo ", line), VA_ARGS);
    VA_END();
}

// LOJBAN TRANSLATION NOTE
// This simply doesn't translate properly. REMOVE ALL CALLS TO THERE AND
// REPLACE THEM WITH CALLS TO PLINE. This function will be removed / commented
// out once that is done.
/*VARARGS1*/
void There
VA_DECL(const char *, line)
{
    char *tmp;
    VA_START(line);
    VA_INIT(line, const char *);
    vpline(YouMessage(tmp, "There ", line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_hear
VA_DECL(const char *, line)
{
    char *tmp;

    if (Deaf || !flags.acoustics)
        return;
    VA_START(line);
    VA_INIT(line, const char *);
    if (Underwater)
        YouPrefix(tmp, ".i do ja'aru'e tirna ", line);
    else if (Unaware)
        YouPrefix(tmp, ".i do senva lonu tirna ", line);
    else
        YouPrefix(tmp, ".i do tirna ", line);
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/*VARARGS1*/
void You_see
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    if (Unaware)
        YouPrefix(tmp, ".i do senva lonu viska ", line);
    else if (Blind) /* caller should have caught this... */
        YouPrefix(tmp, ".i do ganse ", line);
    else
        YouPrefix(tmp, ".i do viska ", line);
    vpline(strcat(tmp, line), VA_ARGS);
    VA_END();
}

/* Print a message inside double-quotes.
 * The caller is responsible for checking deafness.
 * Gods can speak directly to you in spite of deafness.
 */
/*VARARGS1*/
void verbalize
VA_DECL(const char *, line)
{
    char *tmp;

    VA_START(line);
    VA_INIT(line, const char *);
    tmp = You_buf((int) strlen(line) + sizeof "lu  li'u");
    Strcpy(tmp, "lu ");
    Strcat(tmp, line);
    Strcat(tmp, " li'u");
    vpline(tmp, VA_ARGS);
    VA_END();
}

/*VARARGS1*/
/* Note that these declarations rely on knowledge of the internals
 * of the variable argument handling stuff in "tradstdc.h"
 */

#if defined(USE_STDARG) || defined(USE_VARARGS)
static void FDECL(vraw_printf, (const char *, va_list));

void raw_printf
VA_DECL(const char *, line)
{
    VA_START(line);
    VA_INIT(line, char *);
    vraw_printf(line, VA_ARGS);
    VA_END();
}

# ifdef USE_STDARG
static void
vraw_printf(const char *line, va_list the_args)
# else
static void
vraw_printf(line, the_args)
const char *line;
va_list the_args;
# endif

#else /* USE_STDARG | USE_VARARG */

void raw_printf
VA_DECL(const char *, line)
#endif
{
    char pbuf[3 * BUFSZ];
    int ln;
    /* Do NOT use VA_START and VA_END in here... see above */

    if (index(line, '%')) {
        Vsprintf(pbuf, line, VA_ARGS);
        line = pbuf;
    }
    if ((ln = (int) strlen(line)) > BUFSZ - 1) {
        if (line != pbuf)
            line = strncpy(pbuf, line, BUFSZ - 1);
        /* unlike pline, we don't futz around to keep last few chars */
        pbuf[BUFSZ - 1] = '\0'; /* terminate strncpy or truncate vsprintf */
    }
    raw_print(line);
#if !(defined(USE_STDARG) || defined(USE_VARARGS))
    VA_END(); /* (see vpline) */
#endif
}

/*VARARGS1*/
void impossible
VA_DECL(const char *, s)
{
    char pbuf[2 * BUFSZ];
    VA_START(s);
    VA_INIT(s, const char *);
    if (program_state.in_impossible)
        panic("impossible called impossible");

    program_state.in_impossible = 1;
    Vsprintf(pbuf, s, VA_ARGS);
    pbuf[BUFSZ - 1] = '\0'; /* sanity */
    paniclog("impossible", pbuf);
    pline("%s", pbuf);
    pline(".i kalsa samyzilkei .i lonu do ciska zoi gy. #quit .gy cu xamgu cu cumki");  // LOJTODO extended commands
    program_state.in_impossible = 0;
    VA_END();
}

const char *
align_str(alignment)
aligntyp alignment;
{
    switch ((int) alignment) {
    case A_CHAOTIC:
        return "palci";
    case A_NEUTRAL:
        return "nutli";
    case A_LAWFUL:
        return "vrude";
    case A_NONE:
        return "caurmarde";
    }
    return "caurju'o";
}

void
mstatusline(mtmp)
register struct monst *mtmp;
{
    aligntyp alignment = mon_aligntyp(mtmp);
    char info[BUFSZ], monnambuf[BUFSZ];

    info[0] = 0;
    if (mtmp->mtame) {
        Strcat(info, " gi'e tolcilce");
        if (wizard) {
            Sprintf(eos(info), " to %d", mtmp->mtame); // LOJTODO numbers
            if (!mtmp->isminion)
                Sprintf(eos(info), " gi'e xagji %ld gi'e klagaudji %d",
                        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
            Strcat(info, " toi");
        }
    } else if (mtmp->mpeaceful)
        Strcat(info, " gi'e nalcilce");
    if (mtmp->cham >= LOW_PM && mtmp->data != &mons[mtmp->cham])
        /* don't reveal the innate form (chameleon, vampire, &c),
           just expose the fact that this current form isn't it */
        Strcat(info, " gi'e ficybi'o");
    /* pets eating mimic corpses mimic while eating, so this comes first */
    if (mtmp->meating)
        Strcat(info, " gi'e citka");
    /* a stethoscope exposes mimic before getting here so this
       won't be relevant for it, but wand of probing doesn't */
    if (mtmp->m_ap_type)
        Sprintf(eos(info), " gi'e simsa %s",
                (mtmp->m_ap_type == M_AP_FURNITURE)
                    ? an(defsyms[mtmp->mappearance].explanation)
                    : (mtmp->m_ap_type == M_AP_OBJECT)
                          ? ((mtmp->mappearance == GOLD_PIECE)
                                 ? "lo solji"
                                 : an(simple_typename(mtmp->mappearance)))
                          : (mtmp->m_ap_type == M_AP_MONSTER)
                                ? an(mons[mtmp->mappearance].mname)
                                : something); /* impossible... */
    if (mtmp->mcan)
        Strcat(info, " gi'e selvi'umakfa");
    if (mtmp->mconf)
        Strcat(info, " gi'e selfi'u");
    if (mtmp->mblinded || !mtmp->mcansee)
        Strcat(info, " gi'e vistolka'e");
    if (mtmp->mstun)
        Strcat(info, " gi'e seljenca");
    if (mtmp->msleeping)
        Strcat(info, " gi'e sipna");
#if 0 /* unfortunately mfrozen covers temporary sleep and being busy \
         (donning armor, for instance) as well as paralysis */
	else if (mtmp->mfrozen)	  Strcat(info, ", paralyzed");
#else
    else if (mtmp->mfrozen || !mtmp->mcanmove)
        Strcat(info, " gi'e muvtolka'e");
#endif
    /* [arbitrary reason why it isn't moving] */
    else if (mtmp->mstrategy & STRAT_WAITMASK)
        Strcat(info, " gi'e tsapei");
    if (mtmp->mflee)
        Strcat(info, " gi'e terpa");
    if (mtmp->mtrapped)
        Strcat(info, " gi'e selkavbu");
    if (mtmp->mspeed)
        Strcat(info, mtmp->mspeed == MFAST ? " gi'e sutra" : mtmp->mspeed == MSLOW
                                                            ? " gi'e masno"
                                                            : ", ???? speed"); // LOJTODO
    if (mtmp->mundetected)
        Strcat(info, " gi'e zilmipri");
    if (mtmp->minvis)
        Strcat(info, " gi'e selvisnalka'e");
    if (mtmp == u.ustuck)
        Strcat(info, sticks(youmonst.data)
                         ? " gi'e seljai do"
                         : !u.uswallow ? " gi'e jgari do"
                                       : attacktype_fordmg(u.ustuck->data,
                                                           AT_ENGL, AD_DGST)
                                             ? " gi'e djaruntygau do"
                                             : is_animal(u.ustuck->data)
                                                   ? " gi'e tulcti do"
                                                   : " gi'e tunlo do");
    if (mtmp == u.usteed)
        Strcat(info, " gi'e marce do");

    /* avoid "Status of the invisible newt ..., invisible" */
    /* and unlike a normal mon_nam, use "saddled" even if it has a name */
    Strcpy(monnambuf, x_monnam(mtmp, ARTICLE_THE, (char *) 0,
                               (SUPPRESS_IT | SUPPRESS_INVISIBLE), FALSE));

    // LOJTODO numbers
    pline(".i %s to %s toi zo'u %d crena'u gi'e %d to %d toi ka'orna'u gi'e %d dabycakna'u%s", monnambuf,
          align_str(alignment), mtmp->m_lev, mtmp->mhp, mtmp->mhpmax,
          find_mac(mtmp), info);
}

void
ustatusline()
{
    char info[BUFSZ];

    info[0] = '\0';
    if (Sick) {
        Strcat(info, " gi'e zilcatra");
        if (u.usick_type & SICK_VOMITABLE)
            Strcat(info, " lo djavindu");
        if (u.usick_type & SICK_NONVOMITABLE) {
            if (u.usick_type & SICK_VOMITABLE)
                Strcat(info, " .e");
            Strcat(info, " lo kambi'a");
        }
    }
    if (Stoned)
        Strcat(info, " gi'e rokybi'o");
    if (Slimed)
        Strcat(info, " gi'e pexybi'o");
    if (Strangled)
        Strcat(info, " gi'e vaxseldicra");
    if (Vomiting)
        Strcat(info, " gi'e rigbi'a"); /* !"nauseous" */
    if (Confusion)
        Strcat(info, " gi'e selfi'u");
    if (Blind) {
        Strcat(info, " gi'e vistolka'e");
        if (u.ucreamed) {
            Strcat(info, " ri'a");
            if ((long) u.ucreamed < Blinded || Blindfolded
                || !haseyes(youmonst.data))
                Strcat(info, " loza'i selgai");
            Strcat(info, " lo nipypesxu");
        } /* note: "goop" == "glop"; variation is intentional */
    }
    if (Stunned)
        Strcat(info, " gi'e seljenca");
    if (!u.usteed && Wounded_legs) {
        const char *what = body_part(LEG);
        if ((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
            what = makeplural(what);
        Sprintf(eos(info), " gi'e selxai sedi'o %s", what);
    }
    if (Glib)
        Sprintf(eos(info), " gi'e salsfe sedi'o %s", makeplural(body_part(HAND)));
    if (u.utrap)
        Strcat(info, " gi'e selkavbu");
    if (Fast)
        Strcat(info, Very_fast ? " gi'e mutce sutra" : " gi'e sutra");
    if (u.uundetected)
        Strcat(info, " gi'e zilmipri");
    if (Invis)
        Strcat(info, " gi'e selvisnalka'e");
    if (u.ustuck) {
        if (sticks(youmonst.data))
            Strcat(info, " gi'e jgari ");
        else
            Strcat(info, " gi'e seljai ");
        Strcat(info, mon_nam(u.ustuck));
    }

    pline(".i %s to %s%s toi zo'u %d crena'u gi'e %d to %d toi ka'orna'u gi'e %d dabycakna'u%s", plname,
          (u.ualign.record >= 20)
              ? "traji censa "
              : (u.ualign.record > 13)
                    ? "mutce censa "
                    : (u.ualign.record > 8)
                          ? "censa "
                          : (u.ualign.record > 3)
                                ? "milxe censa "
                                : (u.ualign.record == 3)
                                      ? ""
                                      : (u.ualign.record >= 1)
                                            ? "no'e censa "
                                            : (u.ualign.record == 0)
                                                  ? "na'e censa "
                                                  : "to'e censa ",
          align_str(u.ualign.type),
          Upolyd ? mons[u.umonnum].mlevel : u.ulevel, Upolyd ? u.mh : u.uhp,
          Upolyd ? u.mhmax : u.uhpmax, u.uac, info);
}

void
self_invis_message()
{
    pline("%s %s",
          Hallucination ? ".i .uecaisaisai .ianaicaisai .o'ecu'i do" : ".i .ue do suska",
          See_invisible ? "kakne lonu pa'o viska do"
                        : "na kakne lonu viska do");
}

void
pudding_merge_message(otmp, otmp2)
struct obj *otmp;
struct obj *otmp2;
{
    boolean visible =
        cansee(otmp->ox, otmp->oy) || cansee(otmp2->ox, otmp2->oy);
    boolean onfloor = otmp->where == OBJ_FLOOR || otmp2->where == OBJ_FLOOR;
    boolean inpack = carried(otmp) || carried(otmp2);

    /* the player will know something happened inside his own inventory */
    if ((!Blind && visible) || inpack) {
        if (Hallucination) {
            if (onfloor) {
                You_see("lomu'e lo pagbu be lo loldi cu runme");
            } else if (inpack) {
                Your("bakfu ze'o tcena gi'e jgari da");
            }
            /* even though we can see where they should be,
             * they'll be out of our view (minvent or container)
             * so don't actually show anything */
        } else if (onfloor || inpack) {
            pline(".i lo %s jorne zi'o%s", makeplural(obj_typename(otmp->otyp)),
                  inpack ? " di'o lo do bakfu" : "");
        }
    } else {
        You_hear("lo smaji sploici zei sance");
    }
}

/*pline.c*/
