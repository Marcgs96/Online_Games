#pragma once

#include "Behaviours.h"

class ModuleBehaviour : public Module
{
public:

	bool update() override;

	Behaviour * addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject);
	Player * addSpaceship(GameObject *parentGameObject);
	Laser     * addLaser(GameObject *parentGameObject);

private:

	void handleBehaviourLifeCycle(Behaviour * behaviour);

	Player spaceships[MAX_CLIENTS];
	Laser lasers[MAX_GAME_OBJECTS];
};

