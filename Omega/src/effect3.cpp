/* omega copyright (C) by Laurence Raphael Brothers, 1987,1988,1989 */
/* effect3.c */

#include "glob.h"

/* if know id, then summon that monster; else (if < 0) get one. */
void summon (int blessing, int id)
{
    int i,x,y;
    MonsterList* tml;

    if (id < 0)
    {
        if (blessing > 0)
        {
            id = monsterlist();
            xredraw();
        }

        /* for (id ==0) case, see below -- get a "fair" monster */
        else if (blessing < 0)
            id = random_range(NUMMONSTERS);
    }

    for (i = 0; i < 8; ++i)
    {
        x = Player.x+Dirs[0][i];
        y = Player.y+Dirs[1][i];

        if (!inbounds(x,y)) continue;
        if (Level->site[x][y].locchar != FLOOR) continue;
        if (Level->site[x][y].creature) continue;

        break;
    }

    if (i < 8)
    {
        if ((blessing == 0) && (id < 0))
            Level->site[x][y].creature = m_create(x,y,WANDERING,difficulty());
        else
            Level->site[x][y].creature = make_creature(id);

        Level->site[x][y].creature->x = x;
        Level->site[x][y].creature->y = y;
        tml = (MonsterList*) checkmalloc(sizeof(MonsterList));
        tml->monster = Level->site[x][y].creature;

        if (blessing > 0)
            m_status_reset(tml->monster,HOSTILE);
        else if (blessing < 0)
            m_status_set(tml->monster,HOSTILE);

        tml->next = Level->mlist;
        Level->mlist = tml;
    }
}


int itemlist(int itemindex, int num)
{
    int i,itemno;

    print2("Show ID list? ");
    if (ynq2() == 'y') {
        menuclear();
        for(i=0; i<num; i++) {
            menunumprint(i+1);
            menuprint(":");
            menuprint(Objects[i+itemindex].truename);
            menuprint("\n");
        }
        showmenu();
    }
    itemno = (int) parsenum("Item ID? ")-1;
    if ((itemno >= num)||(itemno<0)) itemno = ABORT;
    return(itemno);
}

int monsterlist(void)
{
    int i,itemno;
    print2("Show ID list? ");
    if (ynq2() == 'y')
        do {
            clearmsg();
            menuclear();
            for(i=0; i<NUMMONSTERS; i++) {
                menunumprint(i+1);
                menuprint(":");
                menuprint(Monsters[i].name);
                menuprint("\n");
            }
            showmenu();
            itemno = (int) parsenum("Summon monster: ")-1;
            if ((itemno < 0) || (itemno > NUMMONSTERS-1)) {
                print3("How about trying a real monster?");
                morewait();
            }
        } while ((itemno < 0) || (itemno > NUMMONSTERS-1));
    else
        do {
            itemno = (int) parsenum("Summon monster: ")-1;
        } while ((itemno < 0) || (itemno > NUMMONSTERS-1));
    return(itemno);
}


/* uncurse all items, cure diseases, and neutralize poison */
void cleanse(int blessing)
{
    int i;

    if (blessing > -1) {
        if (blessing > 0)
            for(i=0; i<MAXITEMS; i++)
                if (Player.possessions[i] != NULL) {
                    if ((Player.possessions[i]->used) &&
                            (Player.possessions[i]->blessing < 0)) {
                        Player.possessions[i]->used = false;
                        item_use(Player.possessions[i]);
                        Player.possessions[i]->blessing = 0;
                        Player.possessions[i]->used = true;
                        item_use(Player.possessions[i]);
                    }
                }

        if (Player.status[POISONED] > 0) {
            Player.status[POISONED] = 0;
        }
        if (Player.status[DISEASED] > 0) {
            Player.status[DISEASED] = 0;
        }
        showflags();
        mprint("You feel radiant!");
    }
    else {
        Player.status[POISONED] += 10;
        Player.status[DISEASED] += 10;
        mprint("You feel besmirched!");
        showflags();
    }
}

void annihilate(int blessing)
{
    MonsterList* ml;
    int i;

    if (blessing == 0) {
        mprint("Lightning strikes flash all around you!!!");
        for(i=0; i<9; i++)
            if (Level->site[Player.x+Dirs[0][i]][Player.y+Dirs[1][i]].creature !=
                    NULL)
                Level->site[Player.x+Dirs[0][i]][Player.y+Dirs[1][i]].creature->m_death();
    }
    if (blessing > 0) {
        if (Current_Environment == E_COUNTRYSIDE) {
            clearmsg();
            print1("Bolts of lightning flash down for as far as you can see!!!");
            morewait();
            print1("There is a rain of small birds and insects from the sky, and you");
            print2("notice that you can't hear any animal noises around here any more...");
            Player.alignment -= 3;
        }
        else {
            mprint("Thousands of bolts of lightning flash throughout the level!!!");
            for(ml=Level->mlist; ml!=NULL; ml=ml->next)
                if (ml->monster != NULL && ml->monster->hp > 0)
                    ml->monster->m_death();
        }
    }
    else {
        mprint("You are hit by a bolt of mystic lightning!");
        p_death("self-annihilation");
    }
}


void sleep_monster(int blessing)
{
    /* MonsterList* ml; */
    int x=Player.x,y=Player.y;
    Monster *target;

    if (blessing == 0) setspot(&x,&y);

    if (blessing < 0)
        sleep_player(abs(blessing)+2);
    /*
    else if (blessing > 0) {
      mprint("A silence pervades the area.");
      for (ml=Level->mlist;ml!=NULL;ml=ml->next) {
        m_status_reset(ml->monster,AWAKE);
        ml->monster->wakeup = 0;
      }
    }
    */
    else {
        target = Level->site[x][y].creature;
        if (target != NULL) {
            if (target->uniqueness == COMMON) {
                strcpy(Str1,"The ");
                strcat(Str1,target->name);
            }
            else strcpy(Str1,target->name);
            if ( (! m_immunityp(target,SLEEP)) || (blessing > 0)) {
                strcat(Str1," seems to have fallen asleep.");
                m_status_reset(target,AWAKE);
                target->wakeup = 0;
            }
            else strcat(Str1," is bright eyed, and bushy tailed!");
            mprint(Str1);
        }
        else mprint("Nothing to sleep there!");
    }
}

void sleep_player(int amount)
{
    if (Player.status[SLEPT] == 0) { /* prevent player from sleeping forever */
        mprint("You feel sleepy...");
        if (! p_immune(SLEEP)) {
            Player.status[SLEPT] += random_range(amount*2)+2;
        }
        else mprint("but you shrug off the momentary lassitude.");
    }
}


void hide(int x, int y)
{
    if (inbounds(x,y)) {
        lset(x,y,SECRET);
        lset(x, y, CHANGED);
        putspot(x, y, WALL);
        mprint("You feel sneaky.");
    }
}

void clairvoyance(int vision)
{
    int i,j;
    int x = Player.x, y = Player.y;
    mprint("Clairvoyance... ");
    setspot(&x,&y);
    for(i=x-vision; i<x+vision+1; i++)
        for(j=y-vision; j<y+vision+1; j++) {
            if (inbounds(i,j)) {
                Level->site[i][j].showchar = SPACE;
                lreset(i,j,SECRET);
                lset(i, j, CHANGED);
                dodrawspot(i,j);
            }
        }
    levelrefresh();
}

void aggravate(void)
{
    MonsterList* tm;

    for (tm=Level->mlist; tm!=NULL; tm=tm->next) {
        m_status_set(tm->monster,AWAKE);
        m_status_set(tm->monster,HOSTILE);
    }
}

void learnspell(int blessing)
{
    int i,spell,done=false;
    if (blessing < 0) {
        for(i=NUMSPELLS; ((i>-1) && (! done)); i--)
            if (Spells[i].known) {
                done = true;
                Objects[OB_SPELLS_SCROLL].known = true;
                mprint("You feel forgetful.");
                Spells[i].known = false;
            }
        if (i == ABORT)
            mprint("You feel fortunate.");
    }
    else {
        Objects[OB_SPELLS_SCROLL].known = true;
        spell = random_range(NUMSPELLS);
        print1("Spell Research");
        if ((random_range(4*Spells[spell].powerdrain)+
                Spells[spell].powerdrain) <
                (4*Player.iq+8*Player.level)) {
            nprint1(" -- Research successful: ");
            nprint1(spellid(spell));
            if (Spells[spell].known) {
                print2("...is now easier to cast.");
                Spells[spell].powerdrain = ((int) ((Spells[spell].powerdrain+1)/2));
            }
            else {
                print2("...is added to your repertoire");
                Spells[spell].known = true;
                gain_experience(Spells[spell].powerdrain*10);
            }
        }
        else nprint1(" -- Research unsuccessful.");
    }
}


void amnesia(void)
{
    int i,j;
    for (j=0; j<Level->level_length; j++)
        for (i=0; i<Level->level_width; i++)
            lreset(i,j,SEEN);

    erase_level();
    drawvision(Player.x,Player.y);
}


/*affects player only */
void level_drain(int levels, char *source)
{
    long exp_loss;
    int decrement = ((int) (Player.maxhp / (Player.level+1)));
    /* lost experience is the delta between what is needed for the */
    /* current level and for the target level -DAG */
    exp_loss = expval(Player.level) - expval( Player.level-levels );

    Player.level -= levels;
    Player.xp -= exp_loss;
    Player.maxhp -= (levels * decrement);
    /* should pro-rate hp loss? - DAG */
    Player.hp -= (levels * decrement);

    if ((Player.hp < 1) || (Player.level < 0))
        p_death(source);
}


void disrupt(int x, int y, int amount)
{
    Monster *target;

    if ((x ==Player.x) && (y==Player.y)) {
        mprint("You feel disrupted!");
        p_damage(amount,NORMAL_DAMAGE,"magical disruption");
    }
    else {
        target = Level->site[x][y].creature;
        if (target != NULL) {
            if (target->uniqueness == COMMON) {
                strcpy(Str1,"The ");
                strcat(Str1,target->name);
            }
            else strcpy(Str1,target->name);
            if (! m_immunityp(target,NORMAL_DAMAGE)) {
                strcat(Str1," was blasted!");
                mprint(Str1);
                target->m_damage(amount,NORMAL_DAMAGE);
                target->wakeup = 0;
            }
            else {
                strcat(Str1," does not seem affected.");
                mprint(Str1);
            }
        }
    }
}


void disintegrate(int x, int y)
{
    Monster *target;
    if (! inbounds(x,y)) mprint("You feel a sense of wastage.");
    else if ((x==Player.x)&&(y==Player.y)) {
        if (Player.possessions[O_CLOAK] != NULL) {
            mprint("Your cloak disintegrates!");
            dispose_lost_objects(1,Player.possessions[O_CLOAK]);
        }
        else if (Player.possessions[O_ARMOR] != NULL) {
            mprint("Your armor disintegrates!");
            dispose_lost_objects(1,Player.possessions[O_ARMOR]);
        }
        else {
            mprint("Uh, oh....");
            mprint("Zzzap! You've been disintegrated!");
            p_damage(250,UNSTOPPABLE,"disintegration");
        }
    }
    else {
        if (!view_los_p(Player.x, Player.y, x, y))
            State.setSuppressPrinting( true );
        if ((target = Level->site[x][y].creature) != NULL) {
            if (target->uniqueness == COMMON) {
                strcpy(Str1,"The ");
                strcat(Str1,target->name);
            }
            else strcpy(Str1,target->name);
            strcat(Str1," disintegrates!");
            mprint(Str1);
            target->m_damage(100,UNSTOPPABLE);
            if (target->hp > 0) mprint("It was partially protected by its armor.");
        }
        else if (Level->site[x][y].locchar == ALTAR) {
            mprint("Zzzzap! the altar seems unaffected...");
            mprint("But an angry deity retaliates....");
            disintegrate(Player.x,Player.y);
        }
        else if (Level->site[x][y].p_locf == L_TRAP_PIT) {
            if (Current_Environment == Current_Dungeon) {
                mprint("A hole is blasted in the base of the pit!");
                Level->site[x][y].locchar = TRAP;
                Level->site[x][y].p_locf = L_TRAP_DOOR;
                Level->site[x][y].aux = S_DISINTEGRATE;
                lset(x, y, CHANGED);
            }
            else mprint("The hole just gets deeper....");
        }
        else if (Level->site[x][y].locchar == FLOOR) {
            mprint("You zap a hole in the floor!");
            Level->site[x][y].locchar = TRAP;
            Level->site[x][y].p_locf = L_TRAP_PIT;
            lset(x, y, CHANGED);
        }
        else if ((Level->site[x][y].locchar == WALL) ||
                 (Level->site[x][y].locchar == OPEN_DOOR) ||
                 (Level->site[x][y].locchar == CLOSED_DOOR) ||
                 (Level->site[x][y].locchar == PORTCULLIS) ||
                 (Level->site[x][y].locchar == STATUE)) {
            mprint("The site is reduced to rubble!");
            if (Level->site[x][y].locchar == WALL)
                tunnelcheck();
            Level->site[x][y].p_locf = L_RUBBLE;
            Level->site[x][y].locchar = RUBBLE;
            lreset(x,y,SECRET);
            lset(x, y, CHANGED);
        }
        else if ((Level->site[x][y].locchar == RUBBLE) ||
                 (Level->site[x][y].locchar == TRAP)) {
            mprint("The site is blasted clear!");
            Level->site[x][y].p_locf = L_NO_OP;
            Level->site[x][y].locchar = FLOOR;
            lreset(x,y,SECRET);
            lset(x, y, CHANGED);
        }
        else if (Level->site[x][y].locchar == HEDGE) {
            if (Level->site[x][y].p_locf == L_TRIFID) {
                mprint("The trifid screams as it disintgrates!");
                gain_experience(50);
                Level->site[x][y].p_locf = L_NO_OP;
                Level->site[x][y].locchar = FLOOR;
                lreset(x,y,SECRET);
                lset(x, y, CHANGED);
            }
            else {
                mprint("The hedge is blasted away!");
                Level->site[x][y].p_locf = L_NO_OP;
                Level->site[x][y].locchar = FLOOR;
                lreset(x,y,SECRET);
                lset(x, y, CHANGED);
            }
        }
        else mprint("The blast has no effect.");
        if (!view_los_p(Player.x, Player.y, x, y))
            State.setSuppressPrinting( false );
        else
            plotspot(x, y, true);
    }
}

void acid_cloud(void)
{
    mprint("You are caught in an acid cloud!  ");
    if (Player.possessions[O_CLOAK] != NULL) {
        (void) damage_item(Player.possessions[O_CLOAK]);
        mprint("You are burned by acid.");
        p_damage(3,ACID,"an acid cloud");
    }
    else if (Player.possessions[O_ARMOR] != NULL) {
        mprint("You are burned by acid.");
        p_damage(3,ACID,"an acid cloud");
        (void) damage_item(Player.possessions[O_ARMOR]);
    }
    else if (p_immune(ACID))
    {
        mprint("You resist the effects!");
        return;
    }
    else {
        mprint("The acid eats away at your bare skin!");
        p_damage(25,ACID,"an acid cloud");
    }
}


/* teleport player */
void p_teleport(int type)
{
    int x=Player.x,y=Player.y;
    drawspot(x,y);
    if (type < 0) {
        x = random_range(Level->level_width);
        y = random_range(Level->level_length);
        if ((Level->site[x][y].locchar != FLOOR) &&
                (Level->site[x][y].locchar != OPEN_DOOR)) {
            mprint("You teleported into a solid object....");
            mprint("You are dead!");
            p_death("teleportation into a solid object");
        }
        else {
            Player.x = x;
            Player.y = y;
        }
    }
    else if (type == 0)
        findspace(&(Player.x),&(Player.y),-1);
    else {
        setspot(&Player.x,&Player.y);
        if ((Level->site[Player.x][Player.y].locchar != FLOOR) ||
                (Level->site[Player.x][Player.y].creature != NULL)) {
            mprint("You feel deflected.");
            p_teleport(0);
        }
    }
    Player.status[IMMOBILE] = 0;
    screencheck(Player.x,Player.y);
    roomcheck();
}


void p_poison(int toxicity)
{
    mprint("You feel sick.");
    if (! p_immune(POISON))
        Player.status[POISONED]+=toxicity;
    else mprint("The sickness fades!");
    showflags();
}

void apport(int blessing)
{
    int i,index,x=Player.x,y=Player.y;
    if (blessing > -1) {
        mprint("Apport from:");
        setspot(&x,&y);
        if (Level->site[x][y].things != NULL) {
            pickup_at(x,y);
            plotspot(x, y, true);
        }
        else mprint("There's nothing there to apport!");
    }
    else {
        mprint("You have a sense of loss.");
        for(i=0; i<abs(blessing); i++) {
            index = random_item();
            if (index != ABORT) {
                drop_at(x,y,Player.possessions[index]);
                dispose_lost_objects(Player.possessions[index]->number,
                                     Player.possessions[index]);
            }
        }
    }
}


void strategic_teleport(int blessing)
{
    int new_env;

    /* WDT HACK: Game balance issue: the star gem is supposed to be the only
     * way out of the astral plane (including the Circle of Sorcerors).  However,
     * Hy Magic offers the Location wish, and some artifacts grant this
     * as well.  Seems to me that Hy Magic ought to allow it, and nothing
     * else (aside from the Star Gem, of course). */
    if ((Current_Environment == E_CIRCLE || Current_Environment == E_ASTRAL) &&
            !State.isCheater())
    {
        mprint("Some property of this eerie place interferes with the magic!\n");
        return;
    }
    mprint("Magic portals open up all around you!");
    if (blessing < 0) {
        morewait();
        mprint("You are dragged into one!");
        change_environment(E_COUNTRYSIDE);
        do {
            setPlayerXY( random_range(Level->level_width), random_range(Level->level_length) );
        } while(Country[Player.x][Player.y].base_terrain_type == CHAOS_SEA);
    }
    else {
        mprint("Below each portal is a caption. Enter which one:");
        menuclear();
        menuprint("a: Rampart\n");
        menuprint("b: Village of Star View\n");
        menuprint("c: Village of Woodmere\n");
        menuprint("d: Village of Stormwatch\n");
        menuprint("e: Village of Thaumaris\n");
        menuprint("f: Village of Skorch\n");
        menuprint("g: Village of Whorfen\n");
        menuprint("h: Temple of the Noose\n");
        menuprint("i: The Parthenon\n");
        menuprint("j: Temple of the Black Hand\n");
        menuprint("k: Temple of the Hidden Moon\n");
        menuprint("l: WoodHenge\n");
        menuprint("m: Temple of Destiny\n");
        menuprint("n: HellWell Volcano\n");
        menuprint("o: Ruined Palace\n");
        if (State.isCheater())
            menuprint("z: Anywhere\n");
        menuprint("ANYTHING ELSE: Avoid entering a portal.");
        showmenu();
        switch((char) mcigetc()) {
        case 'a':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 27, 19 );
            break;
        case 'b':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 56, 5 );
            break;
        case 'c':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 35, 11 );
            break;
        case 'd':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 10, 40 );
            break;
        case 'e':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 7, 6 );
            break;
        case 'f':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 41, 43 );
            break;
        case 'g':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 20, 41 );
            break;
        case 'h':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 22, 30 );
            break;
        case 'i':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 51, 11 );
            break;
        case 'j':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 45, 45 );
            break;
        case 'k':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 19, 46 );
            break;
        case 'l':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 32, 5 );
            break;
        case 'm':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 49, 59 );
            break;
        case 'n':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 30, 58 );
            break;
        case 'o':
            change_environment(E_COUNTRYSIDE);
            setPlayerXY( 51, 51 );
            break;
        default:
            if (State.isCheater()) {
                new_env = (int) parsenum("Enter environment number: ");
                change_environment(new_env);
            }
        }
        xredraw();
        if (State.isLost()) {
            print1("You know where you are now.");
            State.setLost( false );
            Precipitation = 0;
        }
    }
    setlastxy(Player.x, Player.y);
    screencheck(Player.x,Player.y);
    drawvision(Player.x,Player.y);
    if (Current_Environment == E_COUNTRYSIDE)
        terrain_check(false);
}


void hero(int blessing)
{
    if (blessing > -1) {
        mprint("You feel super!");
        Player.status[HERO] += random_range(5)+1+blessing;
        calc_melee();
    }
    else {
        Player.status[HERO]=0;
        calc_melee();
        mprint("You feel cowardly.");
        level_drain(abs(blessing),"a potion of cowardice");
    }
}


void levitate(int blessing)
{
    if (blessing > -1) {
        if (State.isMounted())
            mprint("You have a strange feeling of lightness in your saddle.");
        else {
            mprint("You start to float a few inches above the floor.");
            mprint("You discover you can easily control your altitude...");
            mprint("(Note use of '@' command may be useful while levitating)");
            Player.status[LEVITATING] += random_range(5)+1+blessing;
        }
    }
    else mprint("Nothing much happens.");
}


/* has effect of switching between 1st level and deepest level attained */
void level_return(void)
{
    if (Current_Environment == Current_Dungeon) {
        mprint("The vortex of mana carries you off!");
        if (Level->depth > 1)
            change_level(Level->depth,1,false);
        else change_level(Level->depth,deepest[Current_Environment],false);
    }
    else if (Current_Environment == E_COUNTRYSIDE) {
        mprint("A mysterious force wafts you back home!");
        Player.x = 27;
        Player.y = 19;
        screencheck(Player.x,Player.y);
        drawvision(Player.x,Player.y);
        locprint("Back Outside Rampart.");
    }
    else mprint("A feeble vortex of magic swirls by and has no further effect.");
}


void cure(int blessing)
{
    int happened = false;
    if (blessing > -1) {
        if (Player.status[DISEASED]) {
            Player.status[DISEASED]=0;
            mprint("You feel hygienic!");
            happened = true;
        }
        if (Player.status[POISONED]) {
            Player.status[POISONED] -= 5+blessing*10;
            if (Player.status[POISONED] > 0)
                mprint("The effect of the poison has been reduced.");
            else {
                Player.status[POISONED] = 0;
                mprint("The poison has been purged from your system.");
            }
            happened = true;
        }
        if (Player.status[BLINDED]) {
            Player.status[BLINDED]=0;
            happened = true;
            mprint("Cobwebs clear from before your eyes.");
        }
        if (! happened) mprint("Nothing much happens.");
    }
    else disease(12);
    showflags();
}

void disease(int amount)
{
    mprint("You feel ill.");
    if (! Player.immunity[INFECTION]) {
        mprint("You begin to shiver with ague.");
        Player.status[DISEASED]+=random_range(amount*2)+1;
    }
    else mprint("The illness fades.");
}

void truesight(int blessing)
{
    if (blessing > -1) {
        Player.status[TRUESIGHT]+=random_range(10)+1;
        mprint("You feel sharp.");
    }
    else {
        Player.status[BLINDED]+=random_range(10)+1;
        mprint("You've been blinded!");
    }
}


void dispel(int blessing)
{
    int i,x=Player.x,y=Player.y;
    Object* o;
    if (blessing > -1) {
        setspot(&x,&y);
        if ((x==Player.x)&&(y==Player.y)) {
            for(i=0; i<MAXITEMS; i++) {
                o = Player.possessions[i];
                if (o != NULL)
                    if ((o->used) && (o->blessing < 0)) {
                        if (blessing+1 + o->blessing >=0) {
                            o->used = false;
                            State.setSuppressPrinting( true );
                            item_use(o);
                            State.setSuppressPrinting( false );
                            mprint("You hear a sighing sound from");
                            mprint(itemid(o));
                            o->blessing = 0;
                            o->used = true;
                            State.setSuppressPrinting( true );
                            item_use(o);
                            State.setSuppressPrinting( false );
                        }
                        else {
                            mprint("You hear dark laughter from");
                            mprint(itemid(o));
                        }
                    }
            }
        }
        else if (Level->site[x][y].creature != NULL) {
            if (Level->site[x][y].creature->level < blessing * 3) {
                Level->site[x][y].creature->specialf = M_NO_OP;
                if (Level->site[x][y].creature->meleef != M_NO_OP)
                    Level->site[x][y].creature->meleef = M_MELEE_NORMAL;
                Level->site[x][y].creature->strikef = M_NO_OP;
                Level->site[x][y].creature->immunity=0;
                m_status_reset(Level->site[x][y].creature,M_INVISIBLE);
                m_status_reset(Level->site[x][y].creature,INTANGIBLE);
            }
            else mprint("The monster ignores the effect!");
        }
        else if ((Level->site[x][y].p_locf == L_TRAP_FIRE) ||
                 (Level->site[x][y].p_locf == L_STATUE_WAKE) ||
                 (Level->site[x][y].p_locf == L_TRAP_TELEPORT) ||
                 (Level->site[x][y].p_locf == L_TRAP_DISINTEGRATE)) {
            Level->site[x][y].p_locf = L_NO_OP;
            if (Level->site[x][y].locchar == TRAP)
                Level->site[x][y].locchar = FLOOR;
            lset(x, y, CHANGED);
        }
        else if (Level->site[x][y].p_locf == L_MAGIC_POOL)
            Level->site[x][y].p_locf = L_WATER;
        else mprint("Nothing much seems to happen.");
    }
    else {
        mprint("A smell of ozone and positive ions fills the air..");
        if (Player.status[ACCURACY] && (Player.status[ACCURACY] < 1000))
            Player.status[ACCURACY]=1;
        if (Player.status[DISPLACED]&&(Player.status[DISPLACED] < 1000))
            Player.status[DISPLACED]=1;
        if (Player.status[HASTED]&&(Player.status[HASTED] < 1000))
            Player.status[HASTED]=1;
        if (Player.status[BREATHING]&&(Player.status[BREATHING] < 1000))
            Player.status[BREATHING]=1;
        if (Player.status[INVISIBLE]&&(Player.status[INVISIBLE] < 1000))
            Player.status[INVISIBLE]=1;
        if (Player.status[REGENERATING]&&(Player.status[REGENERATING] < 1000))
            Player.status[REGENERATING]=1;
        if (Player.status[ALERT]&&(Player.status[ALERT] < 1000))
            Player.status[ALERT]=1;
        if (Player.status[HERO]&&(Player.status[HERO] < 1000))
            Player.status[HERO]=1;
        if (Player.status[LEVITATING]&&(Player.status[LEVITATING] < 1000))
            Player.status[LEVITATING]=1;
        if (Player.status[ACCURATE]&&(Player.status[ACCURATE] < 1000))
            Player.status[ACCURATE]=1;
        if (Player.status[TRUESIGHT]&&(Player.status[TRUESIGHT] < 1000))
            Player.status[TRUESIGHT]=1;
        tenminute_status_check();
    }
}


void polymorph(int blessing)
{
    int x=Player.x,y=Player.y,newmonster;
    Monster *m;
    int rnd;
    /* for the copy */
    int ohp,olvl;
    long oimmunity, oxpv;

    setspot(&x,&y);
    clearmsg();
    if ((x==Player.x)&&(y==Player.y)) {
        rnd = random_range(NUMMONSTERS);
        strcpy(Str2,"You enjoy your new life as ");
        strcat(Str2,getarticle(Monsters[rnd].name));
        mprint(Str2);
        mprint(Monsters[rnd].name);
        mprint("But your game is over....");
        p_death("autopolymorphosis");
    }
    else if ((m=Level->site[x][y].creature) == NULL)
        mprint("Nothing happens.");
    else {
        if (m_immunityp(m,OTHER_MAGIC) || (m->level > random_range(12))) {
            strcpy(Str1,"The ");
            strcat(Str1,m->name);
            strcat(Str1," resists the change!");
            m_status_set(m,HOSTILE);
        }
        else {
            if (blessing < 0) {
                do newmonster = random_range(NUMMONSTERS);
                while ((newmonster == NPC) ||
                        (newmonster == MAST_THIEF) ||
                        (Monsters[newmonster].level <= m->level) ||
                        (Monsters[newmonster].uniqueness != COMMON));
            }
            else {
                do newmonster = random_range(NUMMONSTERS);
                while ((newmonster == NPC) ||
                        (newmonster == MAST_THIEF) ||
                        (Monsters[newmonster].uniqueness != COMMON));
            }

            ohp = m->hp;
            olvl = m->level;
            oimmunity = m->immunity;
            oxpv = m->xpv;
            *m = Monsters[newmonster];
            m->hp = max(m->hp, ohp);
            m->level = max(m->level, olvl);
            m->immunity |= oimmunity;
            m->xpv = max(m->xpv, oxpv);
            m_status_set(m,HOSTILE);
        }
    }
}


void hellfire(int x, int y, int blessing)
{
    Monster *m;
    if ((x==Player.x)&&(y==Player.y)) {
        mprint("You have been completely annihilated. Congratulations.");
        p_death("hellfire");
    }
    else if ((m=Level->site[x][y].creature) == NULL) {
        mprint("The gods are angry over your waste of power...");
        level_drain(5,"indiscriminate use of hellfire");
    }
    else {
        mprint("The monster writhes in the flames...");
        if (blessing < 0) {
            mprint("...and appears stronger.");
            morewait();
            mprint("Much stronger.");
            m->hp += 1000;
            m->hit +=20;
            m->dmg += 100;
            m_status_set(m,HOSTILE);
        }
        else {
            if (m->uniqueness == COMMON) {
                mprint("and is utterly annihilated. Only a greasy spot remains...");
                m->corpseString = "a greasy spot";
                m->id = 0;
                free_objlist(m->possessions);
                m->possessions = NULL;
            }
            else
                mprint("and dies, cursing your name and the uncaring gods....");
            m->m_death();
        }
    }
}

void drain(int blessing)
{
    int x=Player.x,y=Player.y;
    Monster *m;
    setspot(&x,&y);
    mprint("You begin to drain energy...");
    if ((x==Player.x)&&(y==Player.y)) {
        mprint("You drain your own energy....");
        mprint("Uh, oh, positive feedback....");
        level_drain(Player.level,"self-vampirism");
    }
    else if ((m=Level->site[x][y].creature) != NULL) {
        if ((blessing > -1) && (! m_immunityp(m,NEGENERGY))) {
            mprint("The monster seems weaker...");
            m->m_damage(m->level*m->level,NEGENERGY);
            m->hit = max(m->hit - m->level, 1);
            m->dmg = max(m->dmg - m->level*m->level, 1);
            m->ac = max(m->ac - m->level, 1);
            m->level = max(1,m->level-1);
            mprint("You feel stronger...");
            gain_experience(m->level*5);
            Player.hp+=(m->level*m->level / 2);
        }
        else {
            mprint("The effect reverses itself!");
            mprint("The monster seems stronger...");
            m->hp+=Player.level*Player.level;
            m->hit += Player.level;
            m->dmg += Player.level*Player.level;
            m->ac += Player.level;
            m->level++;
            mprint("You feel weaker...");
            Player.mana = min(0,Player.level*Player.level);
            level_drain(m->level,"negative energy conflict");
        }
    }
    else if (blessing < 0) {
        mprint("You seem to lose energy, instead of gaining it!");
        level_drain(3,"reversed energy drain");
    }
    else if (Level->site[x][y].locchar == ALTAR) {
        mprint("The altar collapses in on itself....");
        Level->site[x][y].locchar = ABYSS;
        Level->site[x][y].p_locf = L_ABYSS;
        lset(x, y, CHANGED);
        if (! Player.patron) {
            mprint("You drain some theurgic energy from the altar....");
            gain_experience(40);
            Player.hp += 20;
            Player.pow+=2;
        }
        if (Level->site[x][y].aux == Player.patron) {
            mprint("Your deity is enraged.");
            mprint("You are struck by godsfire.");
            p_damage(Player.hp-1,UNSTOPPABLE,"godsfire");
            mprint("You feel atheistic.");
            Player.patron = -1;
            Player.rank[PRIESTHOOD] = 0;
        }
        else {
            mprint("You feel the wrath of a god....");
            p_damage(random_range(Player.level*10),UNSTOPPABLE,"divine wrath");
            if (Player.patron != 0) {
                mprint("Your deity doesn't seem to mind your action, though.");
                gain_experience(100);
            }
        }
    }
    else {
        mprint("You drain some energy from the ambient megaflow.");
        Player.hp++;
    }
}

void sanctuary(void)
{
    if (Level->environment == E_TEMPLE)
        mprint("Odd, the spell has no effect. I wonder why.");
    else {
        mprint("You're standing on sacred ground!");
        Player.sx = Player.x;
        Player.sy = Player.y;
    }
}

void shadowform(void)
{
    /* WDT HACK: this fix might work, but it seems like the immunity
     * will be FAR too short.  It's obviously better than the old
     * situation, though... */
    /* DAG reply: this is not too short, SHADOWFORM gets decremented in
       ten minute intervals (tenminute_check), not one minute intervals. */
    if (!Player.status[SHADOWFORM]) {
        mprint("You feel like a shadow.");
        Player.immunity[NORMAL_DAMAGE]++;
        Player.immunity[ACID]++;
        Player.immunity[THEFT]++;
        Player.immunity[INFECTION]++;
        Player.status[SHADOWFORM]+=Player.level;
    }
    else {
        mprint("You feel even more shadowy.");
        Player.status[SHADOWFORM]+=Player.level;
    }
}

void illuminate(int blessing)
{
    int r=Level->site[Player.x][Player.y].roomnumber;
    if (blessing > -1) {
        if (r > ROOMBASE) {
            if (loc_statusp(Player.x,Player.y,LIT))
                mprint("A glow surrounds you.");
            else {
                mprint("The room lights up!");
                Player.status[ILLUMINATION]+=blessing+3;
                spreadroomlight(Player.x,
                                Player.y,
                                Level->site[Player.x][Player.y].roomnumber);
            }
        }
        else mprint("You see a faint glimmer of light which quickly fades.");
    }
    else {
        if (r > ROOMBASE) {
            if (! loc_statusp(Player.x,Player.y,LIT))
                mprint("Nothing much happens.");
            else {
                mprint("The room darkens!");
                spreadroomdark(Player.x,
                               Player.y,
                               Level->site[Player.x][Player.y].roomnumber);
            }
        }
        else mprint("The gloom thickens for a moment.");
    }
}


void drain_life(int amount)
{
    amount = abs(amount);
    mprint("You feel cold!");
    if (p_immune(NEGENERGY))
        mprint("... but the feeling quickly fades.");
    else {
        if (random_range(2)) {
            mprint("The coldness spreads throughout your body...");
            Player.str-=amount;
            Player.con-=amount;
            if ((Player.str < 3) || (Player.con < 3)) {
                mprint("You suffer a fatal heart attack!!!");
                Player.hp = 0;
                strcpy(Str2,"a coronary");
                p_death(Str2);
            }
        }
        else {
            mprint("The coldness saps your very soul...");
            strcpy(Str2,"soul destruction");
            level_drain(amount,Str2);
        }
    }
}


void inflict_fear(int x, int y)
{
    Monster *m;
    if ((Player.x == x) && (Player.y == y)) {
        mprint("You shudder with otherworldly dread.");
        if (Player.immunity[FEAR] > 0)
            mprint("You brace up and face your fear like a hero!");
        else {
            mprint("You panic!");
            Player.status[AFRAID]+=10;
        }
    }
    else if ((m = Level->site[x][y].creature) != NULL) {
        if (m->uniqueness == COMMON) {
            strcpy(Str2,"The ");
            strcat(Str2,m->name);
        }
        else strcpy(Str2,m->name);
        m->speed = max(2,m->speed-1);
        if (m_immunityp(m,FEAR))
            strcat(Str2,"seems enraged!");
        else {
            strcat(Str2,"is terrorized!");
            m->m_dropstuff();
            if (m_statusp(m,MOBILE))
                m->movef = M_MOVE_SCAREDY;
        }
    }
    else mprint("A thrill of fear tickles your spine ... and passes.");
}


/*Turns on deflection status for the player */
void deflection(int blessing)
{
    if (blessing > -1) {
        mprint("You feel buffered.");
        Player.status[DEFLECTION] = blessing + random_range(6);
    }
    else {
        mprint("You feel vulnerable");
        Player.status[VULNERABLE] += random_range(6) - blessing;
    }
}
