#include "Networks.h"
#include "ModuleBehaviour.h"

bool ModuleBehaviour::update()
{
	for (Player &behaviour : players)
	{
		handleBehaviourLifeCycle(&behaviour);
	}
	
	for (Projectile &behaviour : projectiles)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (DeathGhost& behaviour : deathGhosts)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	return true;
}

Behaviour *ModuleBehaviour::addBehaviour(BehaviourType behaviourType, GameObject *parentGameObject)
{
	switch (behaviourType)
	{
	case BehaviourType::Player:
		return addPlayer(parentGameObject);
	case BehaviourType::Projectile:
		return addProjectile(parentGameObject);
	case BehaviourType::DeathGhost:
		return addDeathGhost(parentGameObject);
	default:
		return nullptr;
	}
}

Player* ModuleBehaviour::addPlayer(GameObject *parentGameObject)
{
	for (Player &behaviour : players)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

Projectile*ModuleBehaviour::addProjectile(GameObject *parentGameObject)
{
	for (Projectile &behaviour : projectiles)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

DeathGhost* ModuleBehaviour::addDeathGhost(GameObject* parentGameObject)
{
	for (DeathGhost& behaviour : deathGhosts)
	{
		if (behaviour.gameObject == nullptr)
		{
			behaviour = {};
			behaviour.gameObject = parentGameObject;
			parentGameObject->behaviour = &behaviour;
			return &behaviour;
		}
	}

	ASSERT(false);
	return nullptr;
}

void ModuleBehaviour::handleBehaviourLifeCycle(Behaviour *behaviour)
{
	GameObject *gameObject = behaviour->gameObject;

	if (gameObject != nullptr)
	{
		switch (gameObject->state)
		{
		case GameObject::STARTING:
			behaviour->start();
			break;
		case GameObject::UPDATING:
			behaviour->update();
			break;
		case GameObject::DESTROYING:
			behaviour->destroy();
			gameObject->behaviour = nullptr;
			behaviour->gameObject = nullptr;
			break;
		default:;
		}
	}
}
