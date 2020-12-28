#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Player * addPlayer(GameObject *parentGameObject);
	Projectile * addProjectile(GameObject *parentGameObject);
	DeathGhost* addDeathGhost(GameObject* parentGameObject);
	Weapon* addWeapon(GameObject* parentGameObject);

private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Player players[MAX_CLIENTS];
	Projectile projectiles[MAX_GAME_OBJECTS];
	DeathGhost deathGhosts[MAX_CLIENTS];
	Weapon weapons[MAX_CLIENTS];
};

