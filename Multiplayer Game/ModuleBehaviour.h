#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Player * addPlayer(GameObject *parentGameObject);
	Projectile * addProjectile(GameObject *parentGameObject);
	AxeProjectile * addAxeProjectile(GameObject *parentGameObject);
	StaffProjectile * addStaffProjectile(GameObject *parentGameObject);
	BowProjectile * addBowProjectile(GameObject *parentGameObject);
	DeathGhost* addDeathGhost(GameObject* parentGameObject);
	Weapon* addWeapon(GameObject* parentGameObject);
	AxeSpell* addAxeSpell(GameObject* parentGameObject);
	StaffSpell* addStaffSpell(GameObject* parentGameObject);
	BowSpell* addBowSpell(GameObject* parentGameObject);

	std::list<Player> GetPlayersList();
private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Player players[MAX_CLIENTS];
	Projectile projectiles[MAX_GAME_OBJECTS];
	AxeProjectile axeProjectiles[MAX_GAME_OBJECTS];
	StaffProjectile staffProjectiles[MAX_GAME_OBJECTS];
	BowProjectile bowProjectiles[MAX_GAME_OBJECTS];
	DeathGhost deathGhosts[MAX_CLIENTS];
	Weapon weapons[MAX_CLIENTS];
	AxeSpell axeSpells[MAX_CLIENTS];
	StaffSpell staffSpells[MAX_CLIENTS];
	BowSpell bowSpells[MAX_CLIENTS];
};

