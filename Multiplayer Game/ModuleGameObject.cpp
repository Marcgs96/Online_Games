#include "Networks.h"
#include "ModuleGameObject.h"

bool ModuleGameObject::init()
{
	return true;
}

bool ModuleGameObject::preUpdate()
{
	BEGIN_TIMED_BLOCK(GOPreUpdate);

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::STARTING,     // After INSTANTIATE
		GameObject::UPDATING,     // After STARTING
		GameObject::UPDATING,     // After UPDATING
		GameObject::DESTROYING,   // After DESTROY
		GameObject::NON_EXISTING  // After DESTROYING
	};

	for (GameObject &gameObject : gameObjects)
	{
		gameObject.state = gNextState[gameObject.state];
	}

	END_TIMED_BLOCK(GOPreUpdate);

	return true;
}

bool ModuleGameObject::update()
{
	// Delayed destructions
	for (DelayedDestroyEntry &destroyEntry : gameObjectsWithDelayedDestruction)
	{
		if (destroyEntry.object != nullptr)
		{
			destroyEntry.delaySeconds -= Time.deltaTime;
			if (destroyEntry.delaySeconds <= 0.0f)
			{
				Destroy(destroyEntry.object);
				destroyEntry.object = nullptr;
			}
		}
	}

	return true;
}

bool ModuleGameObject::postUpdate()
{
	return true;
}

bool ModuleGameObject::cleanUp()
{
	return true;
}

GameObject * ModuleGameObject::Instantiate()
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		GameObject &gameObject = App->modGameObject->gameObjects[i];

		if (gameObject.state == GameObject::NON_EXISTING)
		{
			gameObject = GameObject();
			gameObject.id = i;
			gameObject.state = GameObject::INSTANTIATE;
			return &gameObject;
		}
	}

	ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
	return nullptr;
}

void ModuleGameObject::Destroy(GameObject * gameObject)
{
	ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::DESTROY,      // After INSTANTIATE
		GameObject::DESTROY,      // After STARTING
		GameObject::DESTROY,      // After UPDATING
		GameObject::DESTROY,      // After DESTROY
		GameObject::DESTROYING    // After DESTROYING
	};

	ASSERT(gameObject->state < GameObject::STATE_COUNT);
	gameObject->state = gNextState[gameObject->state];
}

void ModuleGameObject::Destroy(GameObject * gameObject, float delaySeconds)
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (App->modGameObject->gameObjectsWithDelayedDestruction[i].object == nullptr)
		{
			App->modGameObject->gameObjectsWithDelayedDestruction[i].object = gameObject;
			App->modGameObject->gameObjectsWithDelayedDestruction[i].delaySeconds = delaySeconds;
			break;
		}
	}
}

GameObject * Instantiate()
{
	GameObject *result = ModuleGameObject::Instantiate();
	return result;
}

void Destroy(GameObject * gameObject)
{
	ModuleGameObject::Destroy(gameObject);
}

void Destroy(GameObject * gameObject, float delaySeconds)
{
	ModuleGameObject::Destroy(gameObject, delaySeconds);
}

void GameObject::write(OutputMemoryStream& packet)
{
	// Write object properties
	packet.Write(this->position.x);
	packet.Write(this->position.y);

	packet.Write(this->size.x);
	packet.Write(this->size.y);

	packet.Write(this->angle);

	//If it has a sprite, write it
	if (this->sprite)
	{
		packet.Write(true);
		packet.Write(std::string(this->sprite->texture->filename));
		packet.Write(this->sprite->order);
	}
	else
	{
		packet.Write(false);
	}

	if (this->collider)
	{
		packet.Write(true);
		packet.Write(this->collider->type);
		packet.Write(this->collider->isTrigger);
	}
	else
	{
		packet.Write(false);
	}

	if (this->behaviour)
	{
		packet.Write(true);
		packet.Write(this->behaviour->type());
	}
	else
	{
		packet.Write(false);
	}
}

void GameObject::read(const InputMemoryStream& packet)
{
	packet.Read(this->position.x);
	packet.Read(this->position.y);

	packet.Read(this->size.x);
	packet.Read(this->size.y);

	packet.Read(this->angle);

	bool ret = false;
	packet.Read(ret);
	//If it has a sprite, read it
	if (ret)
	{
		std::string filename;
		packet.Read(filename);

		if (sprite == nullptr)
		{
			sprite = App->modRender->addSprite(this);
			sprite->texture = App->modResources->GetTextureByFile(filename);
		}
		packet.Read(this->sprite->order);
	}

	packet.Read(ret);
	if (ret)
	{
		ColliderType type = ColliderType::None;
		packet.Read(type);
		if (collider == nullptr)
		{
			collider = App->modCollision->addCollider(type, this);
		}
		packet.Read(this->collider->isTrigger);
	}

	packet.Read(ret);
	if (ret)
	{
		BehaviourType type = BehaviourType::None;
		packet.Read(type);
		if (behaviour == nullptr)
		{
			behaviour = App->modBehaviour->addBehaviour(type, this);
		}
	}
}
