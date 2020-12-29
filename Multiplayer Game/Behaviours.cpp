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
	if (isServer)
	{
		weapon = NetworkInstantiate();
		Weapon* wBehaviour = App->modBehaviour->addWeapon(weapon);
		weapon->sprite = App->modRender->addSprite(weapon);

		switch (playerType)
		{
		case PlayerType::Berserker:
			weapon->sprite->texture = App->modResources->axe;
			wBehaviour->weaponType = WeaponType::Axe;
			weapon->sprite->pivot = vec2{ 0.5f, 0.0f };
			weapon->size = vec2{ 50, 75 };
			break;
		case PlayerType::Wizard:
			weapon->sprite->texture = App->modResources->staff;
			wBehaviour->weaponType = WeaponType::Staff;
			weapon->sprite->pivot = vec2{ 0.5f, 0.3f };
			weapon->size = vec2{ 50, 100 };
			break;
		case PlayerType::Hunter:
			weapon->sprite->texture = App->modResources->bow;
			wBehaviour->weaponType = WeaponType::Bow;
			weapon->sprite->pivot = vec2{ 0.5f, 0.2f };
			weapon->size = vec2{ 100, 50 };
			break;
		case PlayerType::None:
			break;
		default:
			break;
		}

		wBehaviour->player = this->gameObject;
		wBehaviour->isServer = isServer;
		weapon->behaviour = wBehaviour;

		weapon->sprite->order = 6;
		weapon->tag = gameObject->tag;
	}
}

void Player::onInput(const InputController &input)
{
	if (currentState == PlayerState::Dead)
		return;

	HandleMovementInput(input);
}

void Player::onMouseInput(const MouseController& input)
{
	if (input.mouse1 == ButtonState::Press && isServer)
	{
		UseWeapon();
	}
	if (weapon) 
		weapon->behaviour->onMouseInput(input);
}


void Player::update()
{
	//LifeBar update
	static const vec4 colorAlive = vec4{ 0.2f, 1.0f, 0.1f, 0.5f };
	static const vec4 colorDead = vec4{ 1.0f, 0.2f, 0.1f, 0.5f };

	const float lifeRatio = max(0.01f, (float)(hitPoints) / (maxHitPoints));
	lifebar->position = gameObject->position + vec2{ -50.0f, -50.0f };
	lifebar->size = vec2{ lifeRatio * 80.0f, 5.0f };
	lifebar->sprite->color = lerp(colorDead, colorAlive, lifeRatio);

	if (currentState == PlayerState::Dead && isServer)
	{
		static float time_dead = 0.0f;
		time_dead += Time.deltaTime;

		if (time_dead >= 2.0f)
		{
			Respawn();
			NetworkUpdate(gameObject);
			time_dead = 0.0f;
		}		
	}
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
			if (hitPoints > 0)
			{
				hitPoints--;
				NetworkUpdate(gameObject);
			}

			if (hitPoints <= 0)
			{
				Die();

				Projectile* projectile = (Projectile*)c2.gameObject->behaviour;
				if (projectile)
				{
					GameObject* shooter = App->modLinkingContext->getNetworkGameObject(projectile->shooterID);
					if (shooter)
					{
						Player* player = (Player*)shooter->behaviour;
						player->LevelUp(this->level);
					}
				}
					

				NetworkUpdate(gameObject);
			}

			NetworkDestroy(c2.gameObject); // Destroy the Projectile
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
		gameObject->position += movement_vector * movementSpeed * Time.deltaTime;

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
		if (weaponBehaviour->cooldownTimer >= weaponBehaviour->cooldown) {
			weaponBehaviour->Use();
			weaponBehaviour->cooldownTimer = 0;
		}
	}
}

void Player::UseSpell()
{
	/*if (spell)
	{
		spell.Use();
	}*/
}

void Player::Die()
{
	// Centered death effect
	float size = gameObject->size.y;
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

	//Kill player
	level = BASE_LEVEL;
	gameObject->collider->isTrigger = false;
	gameObject->sprite->enabled = false;
	if (weapon)
	{
		weapon->sprite->enabled = false;
		NetworkUpdate(weapon);
	}
	ChangeState(PlayerState::Dead);
}

void Player::Respawn()
{
	gameObject->position = 500.0f * vec2{ Random.next() - 0.5f, Random.next() - 0.5f };
	gameObject->hasTeleported = true;
	gameObject->collider->isTrigger = true;
	gameObject->sprite->enabled = true;
	lifebar->sprite->enabled = true;

	hitPoints = BASE_HP;
	maxHitPoints = BASE_HP;
	gameObject->size = { BASE_SIZE, BASE_SIZE };
	movementSpeed = BASE_SPEED;

	if (weapon)
	{
		Weapon* weaponBehaviour = (Weapon*)weapon->behaviour;
		weapon->size = vec2{ weaponBehaviour->initial_size.x, weaponBehaviour->initial_size.y };
		weapon->sprite->enabled = true;

		NetworkUpdate(weapon);
	}

	ChangeState(PlayerState::Idle);
}

void Player::LevelUp(uint8 killedLevel)
{
	uint8 newLevel = min(level + 1, MAX_LEVEL);
	level = max(killedLevel, newLevel);

	uint8 newMaxHP = HitPoints(level);
	float currentHPPercentage = (float)hitPoints / (float)maxHitPoints;
	hitPoints = newMaxHP * currentHPPercentage;
	maxHitPoints = newMaxHP;

	float newSize = LevelSize(level, BASE_SIZE);
	gameObject->size = vec2{ gameObject->size.x > 0 ? newSize : -newSize, newSize };

	movementSpeed = LevelSpeed(level);

	if (weapon)
	{
		Weapon* weaponBehaviour = (Weapon*)weapon->behaviour;
		float size_x = LevelSize(level, weaponBehaviour->initial_size.x);
		float size_y = LevelSize(level, weaponBehaviour->initial_size.y);
		weapon->size = vec2{ size_x, size_y };

		NetworkUpdate(weapon);
	}

	NetworkUpdate(gameObject);
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
		lifebar->sprite->enabled = true;
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
		lifebar->sprite->enabled = true;
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
	case PlayerState::Dead:
		lifebar->sprite->enabled = false;
		//set death animation
		break;
	}

	return true;
}

void Player::write(OutputMemoryStream& packet)
{
	packet << playerType;
	packet << currentState;
	packet << hitPoints;
	packet << maxHitPoints;
	packet << movementSpeed;
	packet << level;
	packet << name;
}

void Player::read(const InputMemoryStream& packet)
{
	PlayerState new_state;

	packet >> playerType;
	packet >> new_state;
	packet >> hitPoints;
	packet >> maxHitPoints;
	packet >> movementSpeed;
	packet >> level;
	packet >> name;

	ChangeState(new_state);
}

void Player::GetChildrenNetworkObjects(std::list<GameObject*>& networkChildren)
{
	networkChildren.push_back(weapon);
	weapon->behaviour->GetChildrenNetworkObjects(networkChildren);
}

void Projectile::start()
{
	gameObject->networkInterpolationEnabled = true;
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

void Projectile::write(OutputMemoryStream& packet)
{
	packet << shooterID;
}

void Projectile::read(const InputMemoryStream& packet)
{
	packet >> shooterID;
}

void AxeProjectile::start()
{
	Projectile::start();
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip

	if (isServer) {
		direction = vec2FromDegrees(gameObject->angle);
		direction = { -direction.x, -direction.y };
	}
}

void AxeProjectile::update()
{
	if (isServer) {
		Projectile::update();

		gameObject->angle += angleIncrementRatio;
		gameObject->position += direction * velocity * Time.deltaTime;

		NetworkUpdate(gameObject);
	}
}

void StaffProjectile::start()
{
	Projectile::start();
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip

	if (isServer) {
		direction = vec2FromDegrees(gameObject->angle);
		direction = { -direction.x, -direction.y };
	}
}

void StaffProjectile::update()
{
	if (isServer) {
		Projectile::update();

		gameObject->position += direction * velocity * Time.deltaTime;

		NetworkUpdate(gameObject);
	}
}


void BowProjectile::start()
{
	Projectile::start();
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip

	if (isServer) {
		direction = vec2FromDegrees(gameObject->angle);
		direction = { -direction.x, -direction.y };
	}
}

void BowProjectile::update()
{
	if (isServer) {
		Projectile::update();

		gameObject->position += direction * velocity * Time.deltaTime;

		NetworkUpdate(gameObject);
	}
}


void Weapon::start()
{
	initial_size = gameObject->size;
}

void Weapon::update()
{
	vec2 offset = { 0, 8 };
	gameObject->position = player->position + offset;

	cooldownTimer += Time.deltaTime;
}

void Weapon::Use()
{
	if (isServer)
	{
		GameObject* projectile = NetworkInstantiate();
		projectile->sprite = App->modRender->addSprite(projectile);
		projectile->sprite->order = 3;

		switch (weaponType)
		{
			case WeaponType::Axe: {

				projectile->sprite->texture = App->modResources->axeProjectile;
				AxeProjectile* projectileBehaviour = App->modBehaviour->addAxeProjectile(projectile);
				projectileBehaviour->isServer = isServer;
				projectileBehaviour->shooterID = player->networkId;

			} break;
			case WeaponType::Staff: {

				projectile->sprite->texture = App->modResources->staffProjectile;
				StaffProjectile* projectileBehaviour = App->modBehaviour->addStaffProjectile(projectile);
				projectileBehaviour->isServer = isServer;
				projectileBehaviour->shooterID = player->networkId;

			} break;
			case WeaponType::Bow: {

				projectile->sprite->texture = App->modResources->bowProjectile;
				BowProjectile* projectileBehaviour = App->modBehaviour->addBowProjectile(projectile);
				projectileBehaviour->isServer = isServer;
				projectileBehaviour->shooterID = player->networkId;

			} break;
			default: {

			} break;
		}

		projectile->position = gameObject->position;
		projectile->angle = gameObject->angle;
		projectile->size = { 75, 75 };

		projectile->tag = gameObject->tag;
	}
}

void Weapon::write(OutputMemoryStream& packet)
{
	packet << weaponType;
	packet << player->networkId;
}

void Weapon::read(const InputMemoryStream& packet)
{
	packet >> weaponType;
	uint32 playerID;
	packet >> playerID;
	player = App->modLinkingContext->getNetworkGameObject(playerID);	
	if(player)
		((Player*)player->behaviour)->weapon = gameObject;
}

void Weapon::onMouseInput(const MouseController& input)
{
	HandleWeaponRotation(input);
}

void Weapon::HandleWeaponRotation(const MouseController& input)
{
	vec2 mousePosition = { input.x, input.y };

	float angle = atan2(mousePosition.y - gameObject->position.y, mousePosition.x - gameObject->position.x) * (180 / PI) - 90;

	gameObject->angle = angle;

	if (angle <= 90 && angle > -90) {
		gameObject->sprite->order = 6;
	}
	else {
		gameObject->sprite->order = 1;
	}
}

void DeathGhost::start()
{
	App->modSound->playAudioClip(App->modResources->audioClipDeath);
}

void DeathGhost::update()
{
	const float advanceSpeed = 50.0f;
	gameObject->position.y -= advanceSpeed * Time.deltaTime;
	gameObject->sprite->color.a -= 0.5f * Time.deltaTime;
}