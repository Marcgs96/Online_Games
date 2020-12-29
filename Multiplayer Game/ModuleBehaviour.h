#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour* addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Player* addPlayer(GameObject *parentGameObject);
	Projectile* addProjectile(BehaviourType behaviourType, GameObject *parentGameObject);
	DeathGhost* addDeathGhost(GameObject* parentGameObject);
	Weapon* addWeapon(GameObject* parentGameObject);
	Spell* addSpell(BehaviourType behaviourType, GameObject* parentGameObject);

	std::list<Player> GetPlayersList();

private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Player players[MAX_CLIENTS];
	Projectile* projectiles[MAX_GAME_OBJECTS];
	DeathGhost deathGhosts[MAX_CLIENTS];
	Weapon weapons[MAX_CLIENTS];
	Spell* spells[MAX_CLIENTS];
};

