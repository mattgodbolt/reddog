#include "RedDog.h"
#include "command.h"
#include "process.h"
#include "input.h"
#include "view.h"
#include "camera.h"
#include "Player.h"

#define ENERGY_RECHARGE 0.1f

void PickMeUp(STRAT *strat, int type, int amount)
{
	int pn;

	pn = strat->flag2 & 3;

	switch (type)
	{
		case BOMB:
			if (player[pn].PlayerSecondaryWeapon.type == BOMB)
			{
				if (player[pn].PlayerArmageddon == 1)
				{
					amount++;
				}
			}
			PlaySound(player[pn].Player, 5, GENERIC_PICKUP_WEAPON_3, 1.0, 0, 0, 0, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
			break;
		case ENERGY:
			if (player[pn].PlayerWeaponEnergy <= 1.5f)
				player[pn].PlayerWeaponEnergy += 0.5f;
			else
				player[pn].PlayerWeaponEnergy = 2.0;
			PlaySound(player[pn].Player, 5, GENERIC_PICKUP_ENERGY, 1.0, 0, 0, 0, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
			return;
		case ARMOUR:
			if (player[pn].Player->health > 0.0f)
			{
				if (player[pn].Player->health < player[pn].maxHealth - amount)
					player[pn].Player->health += amount;
				else
					player[pn].Player->health = player[pn].maxHealth;
				PlaySound(player[pn].Player, 5, GENERIC_PICKUP_WEAPON_4, 1.0, 0, 0, 0, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
			}
			return;
		case CLOAKING:
			player[pn].cloaked = amount;
			PlaySound(player[pn].Player, 5, GENERIC_PICKUP_WEAPON_3, 1.0, 0, 0, 0, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
			break;
		case ROCKET:
			player[pn].targetting = 2;
			player[pn].playerTargetTime = 0;
			player[pn].playerTargetNumber = -1;
			PlaySound(player[pn].Player, 5, GENERIC_PICKUP_WEAPON_1, 1.0, 0, 0, 0, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
			break;
		case RDHM:
		case VULCAN:
		case HOMING_BULLET:
		case ALL_ROUND_SHOCKWAVE:
		case LASER:			
		case ELECTRO:		
		case STORMING_SHIELD:
		case FLAMER:
			if (!Multiplayer)
				PlaySound(player[pn].Player, 5, GENERIC_PICKUP_WEAPON_3, 1.0, 0, 0, 0, SBFLAG_NOREVERB|SBFLAG_NODOPPLER);
			break;
	}

	
	player[pn].PlayerSecondaryWeapon.type = type;
	player[pn].PlayerSecondaryWeapon.amount = amount;
	if (type != CLOAKING)
		player[pn].cloaked = 0;
}