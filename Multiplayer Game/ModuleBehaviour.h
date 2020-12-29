#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Player * addPlayer(GameObject *parentGameObject);
	Projectile * addProjectile(GameObject *parentGameObject, BehaviourType behaviourType);
	DeathGhost* addDeathGhost(GameObject* parentGameObject);
	Weapon* addWeapon(GameObject* parentGameObject);
	AxeSpell* addAxeSpell(GameObject* parentGameObject);
	StaffSpell* addStaffSpell(GameObject* parentGameObject);
	BowSpell* addBowSpell(GameObject* parentGameObject);

	std::list<Player> GetPlayersList();
private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Player players[MAX_CLIENTS];
	Projectile* projectiles[MAX_GAME_OBJECTS];
	DeathGhost deathGhosts[MAX_CLIENTS];
	Weapon weapons[MAX_CLIENTS];
	AxeSpell axeSpells[MAX_CLIENTS];
	StaffSpell staffSpells[MAX_CLIENTS];
	BowSpell bowSpells[MAX_CLIENTS];
};

