#include "Networks.h"
#include "Behaviours.h"



void Laser::start()
{
	gameObject->networkInterpolationEnabled = false;

	App->modSound->playAudioClip(App->modResources->audioClipLaser);
}

void Laser::update()
{
	secondsSinceCreation += Time.deltaTime;

	const float pixelsPerSecond = 1000.0f;
	gameObject->position += vec2FromDegrees(gameObject->angle) * pixelsPerSecond * Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		}

		const float lifetimeSeconds = 2.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			NetworkDestroy(gameObject);
		}
	}
}





void Player::start()
{
	gameObject->tag = (uint32)(Random.next() * UINT_MAX);

	lifebar = Instantiate();
	lifebar->sprite = App->modRender->addSprite(lifebar);
	lifebar->sprite->pivot = vec2{ 0.0f, 0.5f };
	lifebar->sprite->order = 5;

	//Todo: Create weapon and spell
}

void Player::onInput(const InputController &input)
{
	HandleMovementInput(input);
	HandleCombatInput(input);

	
	if (input.actionDown == ButtonState::Pressed)
	{
		/*const float advanceSpeed = 200.0f;
		gameObject->position += vec2FromDegrees(gameObject->angle) * advanceSpeed * Time.deltaTime;

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}*/
	}

	if (input.horizontalAxis != 0.0f)
	{
		/*const float rotateSpeed = 180.0f;
		gameObject->angle += input.horizontalAxis * rotateSpeed * Time.deltaTime;
		*/
	}

}

void Player::update()
{
	//LifeBar update
	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };

	const float lifeRatio = max(0.01f, (float)(hitPoints) / (MAX_HIT_POINTS));
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);
}

void Player::destroy()
{
	Destroy(lifebar);
}

void Player::onCollisionTriggered(Collider &c1, Collider &c2)
{
	if (c2.type == ColliderType::Laser && c2.gameObject->tag != gameObject->tag)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the laser
		
			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject);
			}

			if (hitPoints <= 0)
			{
				// Centered death effect
				float size = 35.0f;
				vec2 position = gameObject->position;

				GameObject* deathEffect = NetworkInstantiate();
				deathEffect->position = position;
				deathEffect->size = vec2{ size, size };

				deathEffect->sprite = App->modRender->addSprite(deathEffect);
				deathEffect->sprite->texture = App->modResources->death;
				deathEffect->sprite->order = 100;

				deathEffect->animation = App->modRender->addAnimation(deathEffect);
				deathEffect->animation->clip = App->modResources->deathClip;

				NetworkDestroy(deathEffect, 2.0f);
				// NOTE(jesus): Only played in the server right now...
				// You need to somehow make this happen in clients
				App->modSound->playAudioClip(App->modResources->audioClipDeath);

				//Destroy player
				NetworkDestroy(gameObject);
			}
		}
	}
}

void Player::write(OutputMemoryStream & packet)
{
	packet << playerType;
	packet << currentState;
	packet << gameObject->size.x;
	packet << hitPoints;
}

void Player::read(const InputMemoryStream & packet)
{
	packet >> playerType;

	PlayerState new_state;
	packet >> new_state;
	ChangeState(new_state);

	packet >> gameObject->size.x;

	packet >> hitPoints;
}

void Player::HandleMovementInput(const InputController& input)
{
	vec2 movement_vector;

	movement_vector.x = input.horizontalAxis;
	movement_vector.y = -input.verticalAxis;

	if (!isZero(movement_vector))
	{
		const float advanceSpeed = 200.0f;
		gameObject->position += movement_vector * advanceSpeed * Time.deltaTime;

		ChangeState(PlayerState::Running);
		if (movement_vector.x != 0) //Flip character according to direction
			gameObject->size.x = movement_vector.x > 0 ? abs(gameObject->size.x): -abs(gameObject->size.x);

		if (isServer)
		{
			NetworkUpdate(gameObject);
		}
	}
	else
	{
		bool changed = ChangeState(PlayerState::Idle);
		if (isServer && changed)
			NetworkUpdate(gameObject);
	}
}

void Player::HandleCombatInput(const InputController& input)
{
	if (input.actionLeft == ButtonState::Press)
	{
		if (isServer)
		{
			UseWeapon();
		}
	}
	if (input.actionRight == ButtonState::Press)
	{
		if (isServer)
		{
			UseSpell();
		}
	}
}

void Player::UseWeapon()
{
	/*if (weapon)
	{
		Weapon* weapon_behav = (Weapon*)weapon->behaviour;
		weapon_behav.Use();
	}*/

	GameObject* laser = NetworkInstantiate();

	laser->position = gameObject->position;
	laser->angle = gameObject->angle;
	laser->size = { 20, 60 };

	laser->sprite = App->modRender->addSprite(laser);
	laser->sprite->order = 3;
	laser->sprite->texture = App->modResources->laser;

	Laser* laserBehaviour = App->modBehaviour->addLaser(laser);
	laserBehaviour->isServer = isServer;

	laser->tag = gameObject->tag;
}

void Player::UseSpell()
{
	/*if (spell)
	{
		spell.Use();
	}*/
}

bool Player::ChangeState(PlayerState newState)
{
	if (newState == currentState)
		return false;

	currentState = newState;
	switch (currentState)
	{
	case PlayerState::Idle:
	{
		switch (playerType)
		{
		case PlayerType::Berserker:
			gameObject->sprite->texture = App->modResources->berserkerIdle;
			gameObject->animation->clip = App->modResources->playerIdleClip;
			break;
		case PlayerType::Wizard:
			gameObject->sprite->texture = App->modResources->wizardIdle;
			gameObject->animation->clip = App->modResources->playerIdleClip;
			break;
		case PlayerType::Hunter:
			gameObject->sprite->texture = App->modResources->hunterIdle;
			gameObject->animation->clip = App->modResources->playerIdleClip;
			break;
		}
	}	
		break;

	case PlayerState::Running:
	{
		switch (playerType)
		{
		case PlayerType::Berserker:
			gameObject->sprite->texture = App->modResources->berserkerRun;
			gameObject->animation->clip = App->modResources->playerRunClip;
			break;
		case PlayerType::Wizard:
			gameObject->sprite->texture = App->modResources->wizardRun;
			gameObject->animation->clip = App->modResources->playerRunClip;
			break;
		case PlayerType::Hunter:
			gameObject->sprite->texture = App->modResources->hunterRun;
			gameObject->animation->clip = App->modResources->playerRunClip;
			break;
		}
	}
		break;
	}

	return true;
}
