#include "Networks.h"
#include "Behaviours.h"

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

void Player::onCollisionTriggered(Collider& c1, Collider& c2)
{
	if (c2.type == ColliderType::Projectile && c2.gameObject->tag != gameObject->tag)
	{
		if (isServer)
		{
			NetworkDestroy(c2.gameObject); // Destroy the Projectile

			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject);
			}

			if (hitPoints <= 0)
			{
				// Centered death effect
				float size = 40.0f;
				vec2 position = gameObject->position;

				GameObject* deathEffect = NetworkInstantiate();
				deathEffect->position = position;
				deathEffect->size = vec2{ size, size };

				deathEffect->sprite = App->modRender->addSprite(deathEffect);
				deathEffect->sprite->texture = App->modResources->death;
				deathEffect->sprite->order = 100;

				deathEffect->animation = App->modRender->addAnimation(deathEffect);
				deathEffect->animation->clip = App->modResources->deathClip;

				DeathGhost* ghost_behav = App->modBehaviour->addDeathGhost(deathEffect);
				deathEffect->behaviour = ghost_behav;
				deathEffect->behaviour->isServer = isServer;

				NetworkDestroy(deathEffect, 2.0f);
				// NOTE(jesus): Only played in the server right now...
				// You need to somehow make this happen in clients
				//App->modSound->playAudioClip(App->modResources->audioClipDeath);

				//Kill player
				//NetworkDestroy(gameObject);
				ChangeState(PlayerState::Dead);
				Respawn();
				NetworkUpdate(gameObject);
			}
		}
	}
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
	if (weapon)
	{
		Weapon* weaponBehaviour = (Weapon*)weapon->behaviour;
		weaponBehaviour->Use();
	}
}

void Player::UseSpell()
{
	/*if (spell)
	{
		spell.Use();
	}*/
}

void Player::Respawn()
{
	gameObject->position = 500.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f };
	hitPoints = MAX_HIT_POINTS;
	ChangeState(PlayerState::Idle);
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

void Player::write(OutputMemoryStream& packet)
{
	packet << playerType;
	packet << currentState;
	packet << gameObject->size.x;
	packet << hitPoints;
}

void Player::read(const InputMemoryStream& packet)
{
	PlayerState new_state;

	packet >> playerType;
	packet >> new_state;
	packet >> gameObject->size.x;
	packet >> hitPoints;

	ChangeState(new_state);
}

void DeathGhost::start()
{
	App->modSound->playAudioClip(App->modResources->audioClipDeath);
}

void DeathGhost::update()
{
	const float advanceSpeed = 50.0f;
	gameObject->position.y -= advanceSpeed * Time.deltaTime;
}

void Projectile::start()
{
	gameObject->networkInterpolationEnabled = false;
}

void Projectile::update()
{
	secondsSinceCreation += Time.deltaTime;

	if (isServer)
	{
		const float neutralTimeSeconds = 0.1f;
		if (secondsSinceCreation > neutralTimeSeconds && gameObject->collider == nullptr) {
			gameObject->collider = App->modCollision->addCollider(ColliderType::Projectile, gameObject);
		}

		const float lifetimeSeconds = 10.0f;
		if (secondsSinceCreation >= lifetimeSeconds) {
			NetworkDestroy(gameObject);
		}
	}
}

void AxeProjectile::start()
{
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip
}

void AxeProjectile::update()
{
	if (angleIncrementRatio >= PI)
		angleIncrementRatio = 0;

	gameObject->angle += angleIncrementRatio;
	gameObject->position += vec2FromDegrees(gameObject->angle) * velocity * Time.deltaTime;
}

void StaffProjectile::start()
{
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip
}

void StaffProjectile::update()
{
	gameObject->position += vec2FromDegrees(gameObject->angle) * velocity * Time.deltaTime;
}


void BowProjectile::start()
{
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip
}

void BowProjectile::update()
{
	gameObject->position += vec2FromDegrees(gameObject->angle) * velocity * Time.deltaTime;
}


void Weapon::start()
{

}

void Weapon::update()
{

}

void Weapon::Use()
{

	switch (weaponType)
	{
	case WeaponType::Axe: {

		} break;
	case WeaponType::Staff: {

		} break;
	case WeaponType::Bow: {

		} break;
	default: {

		} break;
	}

	//GameObject* Projectile = NetworkInstantiate();

	//Projectile->position = gameObject->position;
	//Projectile->angle = gameObject->angle;
	//Projectile->size = { 20, 60 };

	//Projectile->sprite = App->modRender->addSprite(Projectile);
	//Projectile->sprite->order = 3;
	//Projectile->sprite->texture = App->modResources->laser;

	//Projectile* ProjectileBehaviour = App->modBehaviour->addProjectile(Projectile);
	//ProjectileBehaviour->isServer = isServer;

	//Projectile->tag = gameObject->tag;
}

void Weapon::onMouseInput(const MouseController& input)
{
	HandleWeaponRotation(input);
}

void Weapon::HandleWeaponRotation(const MouseController& input)
{
	vec2 mousePosition = { input.x, input.y };
	float angle = atan2(mousePosition.y, mousePosition.x);

	gameObject->angle = angle;
}

