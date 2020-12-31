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

	for (Spell* behaviour : spells)
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
		return addProjectile(BehaviourType::Projectile, parentGameObject);
	case BehaviourType::AxeProjectile:
		return addProjectile(BehaviourType::AxeProjectile, parentGameObject);
	case BehaviourType::StaffProjectile:
		return addProjectile(BehaviourType::StaffProjectile, parentGameObject);
	case BehaviourType::BowProjectile:
		return addProjectile(BehaviourType::BowProjectile, parentGameObject);
	case BehaviourType::WhirlwindAxeProjectile:
		return addProjectile(BehaviourType::WhirlwindAxeProjectile, parentGameObject);
	case BehaviourType::AxeSpell:
		return addSpell(BehaviourType::AxeSpell, parentGameObject);
	case BehaviourType::StaffSpell:
		return addSpell(BehaviourType::StaffSpell, parentGameObject);
	case BehaviourType::BowSpell:
		return addSpell(BehaviourType::BowSpell, parentGameObject);
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

Projectile* ModuleBehaviour::addProjectile(BehaviourType type, GameObject* parentGameObject)
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
			case BehaviourType::WhirlwindAxeProjectile:
				behaviour = new WhirlwindAxeProjectile;
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
				case BehaviourType::WhirlwindAxeProjectile:
					behaviour = new WhirlwindAxeProjectile;
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

Spell* ModuleBehaviour::addSpell(BehaviourType behaviourType, GameObject* parentGameObject)
{
	for (Spell*& behaviour : spells)
	{
		if (behaviour == nullptr)
		{
			switch (behaviourType)
			{
			case BehaviourType::AxeSpell:
				behaviour = new AxeSpell;
				break;
			case BehaviourType::StaffSpell:
				behaviour = new StaffSpell;
				break;
			case BehaviourType::BowSpell:
				behaviour = new BowSpell;
				break;
			default:
				break;
			}
			behaviour->gameObject = parentGameObject;
			return behaviour;
		}
		else
		{
			if (behaviour->gameObject == nullptr)
			{
				delete behaviour;
				switch (behaviourType)
				{
				case BehaviourType::AxeSpell:
					behaviour = new AxeSpell;
					break;
				case BehaviourType::StaffSpell:
					behaviour = new StaffSpell;
					break;
				case BehaviourType::BowSpell:
					behaviour = new BowSpell;
					break;
				default:
					break;
				}
				behaviour->gameObject = parentGameObject;
				return behaviour;
			}
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
