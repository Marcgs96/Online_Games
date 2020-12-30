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

			spell = App->modBehaviour->addSpell(BehaviourType::AxeSpell, this->gameObject);
			spell->player = gameObject;
			spell->isServer = isServer;

			break;
		case PlayerType::Wizard:
			weapon->sprite->texture = App->modResources->staff;
			wBehaviour->weaponType = WeaponType::Staff;
			weapon->sprite->pivot = vec2{ 0.5f, 0.3f };
			weapon->size = vec2{ 50, 100 };

			spell = App->modBehaviour->addSpell(BehaviourType::StaffSpell, this->gameObject);
			break;
		case PlayerType::Hunter:
			weapon->sprite->texture = App->modResources->bow;
			wBehaviour->weaponType = WeaponType::Bow;
			weapon->sprite->pivot = vec2{ 0.5f, 0.2f };
			weapon->size = vec2{ 100, 50 };
			
			spell = App->modBehaviour->addSpell(BehaviourType::BowSpell, this->gameObject);
			spell->isServer = isServer;
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
	HandleCombatInput(input);
}

void Player::onMouseInput(const MouseController& input)
{
	if (currentState == PlayerState::Dead)
		return;

	if (input.mouse1 == ButtonState::Press && isServer &&
		currentState != PlayerState::Charging)
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
			Projectile* projectile = (Projectile*)c2.gameObject->behaviour;

			if (hitPoints > 0)
			{
				hitPoints = projectile->damagePoints <= hitPoints? hitPoints - projectile->damagePoints : 0;
			}

			if (hitPoints <= 0)
			{
				GameObject* shooter = App->modLinkingContext->getNetworkGameObject(projectile->shooterID);
				if (shooter)
				{
					Player* player = (Player*)shooter->behaviour;
					player->LevelUp(this->level);
				}

				Die();					
			}

			NetworkUpdate(gameObject);
			if(!projectile->perforates)
				NetworkDestroy(c2.gameObject); // Destroy the Projectile
		}
	}
}

void Player::HandleMovementInput(const InputController& input)
{
	if (currentState == PlayerState::Charging)
		return;

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
	if (!isServer)
		return;

	if (spell)
		spell->onInput(input);
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
	if (spell && spell->spellCooldownTimer >= spell->spellCooldown)
	{
		spell->Use();
	}
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

	case PlayerState::Charging:
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
	}

	return true;
}

void Player::writeCreate(OutputMemoryStream& packet)
{
	packet << playerType;
	packet << currentState;
	packet << hitPoints;
	packet << maxHitPoints;
	packet << movementSpeed;
	packet << level;
	packet << name;
	packet << (weapon? weapon->networkId:0);
}

void Player::readCreate(const InputMemoryStream& packet)
{
	PlayerState new_state;

	packet >> playerType;
	packet >> new_state;
	packet >> hitPoints;
	packet >> maxHitPoints;
	packet >> movementSpeed;
	packet >> level;
	packet >> name;

	uint32 weaponNetworkID;
	packet >> weaponNetworkID;
	if (!weapon)
	{
		weapon = App->modLinkingContext->getNetworkGameObject(weaponNetworkID);
		if (weapon)
		{
			((Weapon*)weapon->behaviour)->player = gameObject;
			weapon->networkInterpolationEnabled = gameObject->networkInterpolationEnabled;
		}
			
;	}

	ChangeState(new_state);
}

void Player::writeUpdate(OutputMemoryStream& packet)
{
	packet << currentState;
	packet << hitPoints;
	packet << maxHitPoints;
	packet << movementSpeed;
	packet << level;
}

void Player::readUpdate(const InputMemoryStream& packet)
{
	PlayerState new_state;

	packet >> new_state;
	packet >> hitPoints;
	packet >> maxHitPoints;
	packet >> movementSpeed;
	packet >> level;

	ChangeState(new_state);
}

void Player::GetChildrenNetworkObjects(std::list<GameObject*>& networkChildren)
{
	networkChildren.push_back(weapon);
	weapon->behaviour->GetChildrenNetworkObjects(networkChildren);
}

void Player::OnInterpolationDisable()
{
	if (weapon)
		weapon->networkInterpolationEnabled = false;
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

void Projectile::writeCreate(OutputMemoryStream& packet)
{
	packet << shooterID;
}

void Projectile::readCreate(const InputMemoryStream& packet)
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
				Projectile* projectileBehaviour = App->modBehaviour->addProjectile(BehaviourType::AxeProjectile, projectile);
				projectileBehaviour->isServer = isServer;
				projectileBehaviour->shooterID = player->networkId;

			} break;
			case WeaponType::Staff: {

				projectile->sprite->texture = App->modResources->staffProjectile;
				Projectile* projectileBehaviour = App->modBehaviour->addProjectile(BehaviourType::StaffProjectile, projectile);
				projectileBehaviour->isServer = isServer;
				projectileBehaviour->shooterID = player->networkId;

			} break;
			case WeaponType::Bow: {

				projectile->sprite->texture = App->modResources->bowProjectile;
				Projectile* projectileBehaviour = App->modBehaviour->addProjectile(BehaviourType::BowProjectile, projectile);
				projectileBehaviour->isServer = isServer;
				projectileBehaviour->shooterID = player->networkId;

			} break;
			default: {

			} break;
		}

		projectile->position = gameObject->position;
		projectile->angle = gameObject->angle;
		vec2 standardSize = { 75, 75 };
		Player* playerBehaviour = (Player*)player->behaviour;
		float sizeX = playerBehaviour->LevelSize(playerBehaviour->level, standardSize.x);
		float sizeY = playerBehaviour->LevelSize(playerBehaviour->level, standardSize.y);
		projectile->size = { sizeX, sizeY };
		projectile->tag = gameObject->tag;
	}
}

void Weapon::writeCreate(OutputMemoryStream& packet)
{
	packet << weaponType;
	packet << (player? player->networkId:0);
}

void Weapon::readCreate(const InputMemoryStream& packet)
{
	packet >> weaponType;
	uint32 playerID;
	packet >> playerID;

	if (!player)
	{
		player = App->modLinkingContext->getNetworkGameObject(playerID);
		if (player)
		{
			((Player*)player->behaviour)->weapon = gameObject;
			gameObject->networkInterpolationEnabled = player->networkInterpolationEnabled;
		}
			
	}
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

	if (isServer)
		NetworkUpdate(gameObject);
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

void Spell::start()
{
	spellCooldownTimer = 10.0f;
}

void Spell::update()
{
	if(spellCooldownTimer < spellCooldown)
		spellCooldownTimer += Time.deltaTime;
}

void Spell::Use()
{
	spellCooldownTimer = 0.0f;
}

void AxeSpell::start()
{
	Spell::start();
}

void AxeSpell::update()
{
	Spell::update();
}

void AxeSpell::Use()
{
	Spell::Use();
	if (isServer)
	{
		for (int i = 0; i < NUM_AXES; ++i) {

			axes[i] = NetworkInstantiate();
			axes[i]->sprite = App->modRender->addSprite(axes[i]);
			axes[i]->sprite->order = 3;

			axes[i]->sprite->texture = App->modResources->axeProjectile;
			WhirlwindAxeProjectile* whirlwindAxeBehaviour = (WhirlwindAxeProjectile*)App->modBehaviour->addProjectile(BehaviourType::WhirlwindAxeProjectile, axes[i]);
			whirlwindAxeBehaviour->isServer = isServer;
			whirlwindAxeBehaviour->shooterID = player->networkId;
			whirlwindAxeBehaviour->index = i;
			whirlwindAxeBehaviour->player = gameObject;

			vec2 standardSize = { 75, 75 };
			Player* playerBehaviour = (Player*)player->behaviour;
			float sizeX = playerBehaviour->LevelSize(playerBehaviour->level, standardSize.x);
			float sizeY = playerBehaviour->LevelSize(playerBehaviour->level, standardSize.y);
			axes[i]->size = { sizeX, sizeY };
			axes[i]->tag = gameObject->tag;
		}
	}
}

void StaffSpell::start()
{
}

void StaffSpell::update()
{
}

void StaffSpell::Use()
{
}

void BowSpell::start()
{
	spellCooldownTimer = spellCooldown = 10.0f;
}

void BowSpell::Use()
{
	charging = true;
	chargeTime = 0;
	Player* playerBehaviour = (Player*)gameObject->behaviour;
	playerBehaviour->ChangeState(PlayerState::Charging);

	// Charge effect
	vec2 offset = { 8, 17 };
	offset.x = playerBehaviour->LevelSize(playerBehaviour->level, offset.x);
	offset.y = playerBehaviour->LevelSize(playerBehaviour->level, offset.y);
	float newSize = 90;

	chargeEffect = NetworkInstantiate();
	chargeEffect->position = vec2{ gameObject->position.x - offset.x, gameObject->position.y - offset.y };
	newSize = playerBehaviour->LevelSize(playerBehaviour->level, newSize);
	chargeEffect->size = vec2{ newSize, newSize };

	chargeEffect->sprite = App->modRender->addSprite(chargeEffect);
	chargeEffect->sprite->texture = App->modResources->chargeEffect;
	chargeEffect->sprite->order = 100;

	chargeEffect->animation = App->modRender->addAnimation(chargeEffect);
	chargeEffect->animation->clip = App->modResources->chargeEffectClip;

	NetworkUpdate(gameObject);
}

void BowSpell::Hold()
{
	if(charging)
		chargeTime += Time.deltaTime;
}

void BowSpell::Release()
{
	if (!charging)
		return;

	GameObject* projectile = NetworkInstantiate();
	projectile->sprite = App->modRender->addSprite(projectile);
	projectile->sprite->order = 3;
	projectile->sprite->texture = App->modResources->iceSpike;

	vec2 standardSize = { 20, 75 };
	Player* playerBehaviour = (Player*)gameObject->behaviour;
	float sizeX = playerBehaviour->LevelSize(playerBehaviour->level, standardSize.x);
	float sizeY = playerBehaviour->LevelSize(playerBehaviour->level, standardSize.y);
	projectile->size = { sizeX, sizeY};
	projectile->tag = gameObject->tag;

	projectile->position = playerBehaviour->weapon->position;
	projectile->angle = playerBehaviour->weapon->angle;

	BowProjectile* projectileBehaviour = (BowProjectile*) App->modBehaviour->addProjectile(BehaviourType::BowProjectile, projectile);
	projectileBehaviour->isServer = isServer;
	projectileBehaviour->shooterID = gameObject->networkId;

	chargeTime = min(MAX_CHARGE, chargeTime);
	projectileBehaviour->damagePoints = MIN_DAMAGE + ((chargeTime - MIN_CHARGE) / (MAX_CHARGE - MIN_CHARGE)) * (MAX_DAMAGE - MIN_DAMAGE);
	float multiplier = MIN_INCREASE + ((chargeTime - MIN_CHARGE) / (MAX_CHARGE - MIN_CHARGE)) * (MAX_INCREASE - MIN_INCREASE);
	//projectile->size = { sizeX * multiplier, sizeY * multiplier };
	projectileBehaviour->velocity *= multiplier;

	NetworkDestroy(chargeEffect);

	playerBehaviour->ChangeState(PlayerState::Idle);
	charging = false;
	spellCooldownTimer = 0.0f;
}

void BowSpell::onInput(const InputController& input)
{
	if (input.space == ButtonState::Press && spellCooldownTimer >= spellCooldown) 
		Use();
	else if (input.space == ButtonState::Pressed)
		Hold();
	else if (input.space == ButtonState::Release)
		Release();
}

void WhirlwindAxeProjectile::start()
{
	Projectile::start();
	App->modSound->playAudioClip(App->modResources->audioClipLaser); //TODO Change to correct clip

	if (isServer) {
		if (index == 0)
			orbitAngle = PI/2;
		else if(index == 1)
			orbitAngle = 7 * PI / 6;
		else{
			orbitAngle = 11 * PI / 6;
		}
	}
}

void WhirlwindAxeProjectile::update()
{
	if (isServer) {
		Projectile::update();

		if (index == 0) {
			gameObject->angle += selfRotationIncrementRatio;
			orbitAngle += orbitSpeed * Time.deltaTime;
			gameObject->position = { player->position.x + rotationRadius * cos(orbitAngle) , player->position.y + rotationRadius * sin(orbitAngle) };
		}
		else if(index == 1){
			gameObject->angle += selfRotationIncrementRatio;
			orbitAngle += orbitSpeed * Time.deltaTime;
			gameObject->position = { player->position.x + rotationRadius * cos(orbitAngle) , player->position.y + rotationRadius * sin(orbitAngle) };
			LOG("orbit angle %f", orbitAngle);
		}else{
			gameObject->angle += selfRotationIncrementRatio;
			orbitAngle += orbitSpeed * Time.deltaTime;
			gameObject->position = { player->position.x + rotationRadius * cos(orbitAngle) , player->position.y + rotationRadius * sin(orbitAngle) };
		}

		NetworkUpdate(gameObject);
	}
}
