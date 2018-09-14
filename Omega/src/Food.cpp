#include "glob.h"
#include "Food.h"

Food::Food(void)
{
	weight = 20;
	plus = charge = dmg = hit = fragility = blessing = 0;

	/* how full the food makes you feel */
	aux = 8;
	
	number = 1;
	basevalue = 2;

	uniqueness = COMMON;
	level = 0;
	objchar = FOOD;
	usef = I_FOOD;
	objstr = "food ration";
	truename = "food ration";
	cursestr = "food ration";
	initialize();
}

Food::~Food(void)
{
}

void Food::initialize()
{
}

// Consume the food. Destroying object takes place in eat command: command2.c/eat()
void Food::use()
{
	// Adjust player's hunger
	Player.food = max(0, Player.food + aux);

	// Print flavor message
    switch (random_range(6))
    {
    case 0:
        mprint("That tasted horrible!");
        break;
    case 1:
        mprint("Yum!");
        break;
    case 2:
        mprint("How nauseous!");
        break;
    case 3:
        mprint("Can I have some more? Please?");
        break;
    case 4:
        mprint("Your mouth feels like it is growing hair!");
        break;
    case 5:
        mprint("Tastes like home cooking!");
        break;
    }
}

Food::Food(Object *o) {
    initialize();

    weight = o->weight;
    plus = o->plus;
    charge = o->charge;
    dmg = o->dmg;
    hit = o->hit;
    aux = o->aux;
    number = o->number;
    fragility = o->fragility;
    basevalue = o->basevalue;
    known = o->known;
    used = o->used;
    blessing = o->blessing;
    type = o->type;
    uniqueness = o->uniqueness;
    usef = o->usef;
    level = o->level;
    objchar = o->objchar;
    if(o->objstr != NULL) objstr = o->objstr;
    if(o->truename != NULL) truename = o->truename;
    if(o->cursestr != NULL) cursestr = o->cursestr;
}
