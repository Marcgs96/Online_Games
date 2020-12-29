#include "Networks.h"
#include "ModuleBehaviour.h"

bool ModuleBehaviour::update()
{
	for (Player &behaviour : players)
	{
		handleBehaviourLifeCycle(&behaviour);
	}
	
	for (Projectile* behaviour : projectiles)
	{
		handleBehaviourLifeCycle(behaviour);
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
		return addProjectile(parentGameObject, BehaviourType::Projectile);
	case BehaviourType::AxeProjectile:
		return addProjectile(parentGameObject, BehaviourType::AxeProjectile);
	case BehaviourType::StaffProjectile:
		return addProjectile(parentGameObject, BehaviourType::StaffProjectile);
	case BehaviourType::BowProjectile:
		return addProjectile(parentGameObject, BehaviourType::BowProjectile);
	case BehaviourType::DeathGhost:
		return addDeathGhost(parentGameObject);	
	case BehaviourType::Weapon:
		return addWeapon(parentGameObject);
	case BehaviourType::AxeSpell:
		return addAxeSpell(parentGameObject);
	case BehaviourType::StaffSpell:
		return addStaffSpell(parentGameObject);
	case BehaviourType::BowSpell:
		return addBowSpell(parentGameObject);
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

Projectile* ModuleBehaviour::addProjectile(GameObject* parentGameObject, BehaviourType type)
{
	for (Projectile*& behaviour : projectiles)
	{
		if (behaviour == nullptr)
		{
			switch (type)
			{
			case BehaviourType::StaffProjectile:
				behaviour = new StaffProjectile;
				break;
			case BehaviourType::AxeProjectile:
				behaviour = new AxeProjectile;
				break;
			case BehaviourType::BowProjectile:
				behaviour = new BowProjectile;
				break;
			default:
				break;
			}
			behaviour->gameObject = parentGameObject;
			parentGameObject->behaviour = behaviour;
			return behaviour;
		}
		else
		{
			if (behaviour->gameObject == nullptr)
			{
				delete behaviour;
				switch (type)
				{
				case BehaviourType::StaffProjectile:
					behaviour = new StaffProjectile;
					break;
				case BehaviourType::AxeProjectile:
					behaviour = new AxeProjectile;
					break;
				case BehaviourType::BowProjectile:
					behaviour = new BowProjectile;
					break;
				default:
					break;
				}
				behaviour->gameObject = parentGameObject;
				parentGameObject->behaviour = behaviour;
				return behaviour;
			}
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

AxeSpell* ModuleBehaviour::addAxeSpell(GameObject* parentGameObject)
{
	for (AxeSpell& behaviour : axeSpells)
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

StaffSpell* ModuleBehaviour::addStaffSpell(GameObject* parentGameObject)
{
	for (StaffSpell& behaviour : staffSpells)
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

BowSpell* ModuleBehaviour::addBowSpell(GameObject* parentGameObject)
{
	for (BowSpell& behaviour : bowSpells)
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

std::list<Player> ModuleBehaviour::GetPlayersList()
{
	std::list<Player> playersList;
	for (Player player : players)
	{
		if (player.gameObject != nullptr)
		{
			playersList.push_back(player);
		}
	}

	return playersList;
}

void ModuleBehaviour::handleBehaviourLifeCycle(Behaviour *behaviour)
{
	if (!behaviour)
		return;

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
