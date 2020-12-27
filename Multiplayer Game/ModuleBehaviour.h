#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Player * addPlayer(GameObject *parentGameObject);
	Laser * addLaser(GameObject *parentGameObject);
	DeathGhost* addDeathGhost(GameObject* parentGameObject);

private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Player players[MAX_CLIENTS];
	Laser lasers[MAX_GAME_OBJECTS];
	DeathGhost deathGhosts[MAX_CLIENTS];
};

