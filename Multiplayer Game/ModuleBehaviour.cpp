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

	for (AxeProjectile& behaviour : axeProjectiles)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (StaffProjectile& behaviour : staffProjectiles)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (BowProjectile& behaviour : bowProjectiles)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (DeathGhost& behaviour : deathGhosts)
	{
		handleBehaviourLifeCycle(&behaviour);
	}

	for (Weapon& behaviour : weapons) 
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
	case BehaviourType::AxeProjectile:
		return addAxeProjectile(parentGameObject);
	case BehaviourType::StaffProjectile:
		return addStaffProjectile(parentGameObject);
	case BehaviourType::BowProjectile:
		return addBowProjectile(parentGameObject);
	case BehaviourType::DeathGhost:
		return addDeathGhost(parentGameObject);	
	case BehaviourType::Weapon:
		return addWeapon(parentGameObject);
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

AxeProjectile* ModuleBehaviour::addAxeProjectile(GameObject* parentGameObject)
{
	for (AxeProjectile& behaviour : axeProjectiles)
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

StaffProjectile* ModuleBehaviour::addStaffProjectile(GameObject* parentGameObject)
{
	for (StaffProjectile& behaviour : staffProjectiles)
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

BowProjectile* ModuleBehaviour::addBowProjectile(GameObject* parentGameObject)
{
	for (BowProjectile& behaviour : bowProjectiles)
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

Weapon* ModuleBehaviour::addWeapon(GameObject* parentGameObject)
{
	for (Weapon& behaviour : weapons)
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
