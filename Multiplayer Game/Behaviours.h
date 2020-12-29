#pragma once


enum class BehaviourType : uint8;

struct Behaviour
{
	GameObject *gameObject = nullptr;
	bool isServer = false;
	bool isLocalPlayer = false;

	virtual BehaviourType type() const = 0;

	virtual void start() { }

	virtual void onInput(const InputController &input) { }

	virtual void onMouseInput(const MouseController &input) { }

	virtual void update() { }

	virtual void destroy() { }

	virtual void onCollisionTriggered(Collider &c1, Collider &c2) { }

	virtual void GetChildrenNetworkObjects(std::list<GameObject*>&) { }

	virtual void write(OutputMemoryStream &packet) { }

	virtual void read(const InputMemoryStream &packet) { }
};


enum class BehaviourType : uint8
{
	None,
	Player,
	Projectile,
	DeathGhost,
	Weapon,
	StaffProjectile,
	AxeProjectile,
	BowProjectile
};

enum class PlayerType : uint8
{
	Berserker,
	Wizard,
	Hunter,
	None
};

enum class WeaponType : uint8
{
	Axe,
	Staff,
	Bow,
	None
};

struct Weapon : public Behaviour 
{
	BehaviourType type() const override { return BehaviourType::Weapon; }

	WeaponType weaponType = WeaponType::None;
	GameObject* player = nullptr;
	std::list<GameObject*> spawned_projectiles;
	vec2 initial_size = {};
	float cooldown = 0.5f;
	float cooldownTimer = 0.f;

	void start() override;
	void update() override;
	void Use();

	void write(OutputMemoryStream& packet) override;
	void read(const InputMemoryStream& packet) override;

	void onMouseInput(const MouseController& input) override;
	void HandleWeaponRotation(const MouseController& input);

};

struct Projectile : public Behaviour
{
	float secondsSinceCreation = 0.0f;
	float velocity = 1000.0f;
	vec2 direction = { 0,0 };
	uint8 damagePoints = 1;
	
	uint32 shooterID = 0;

	BehaviourType type() const override { return BehaviourType::Projectile; }

	virtual void start() override;

	virtual void update() override;

	void write(OutputMemoryStream& packet) override;

	void read(const InputMemoryStream& packet) override;
};

struct AxeProjectile : public Projectile
{
	float angleIncrementRatio = 10;
	vec2 direction;

	BehaviourType type() const override { return BehaviourType::AxeProjectile; }

	void start() override;

	void update() override;
};

struct StaffProjectile : public Projectile
{
	BehaviourType type() const override { return BehaviourType::StaffProjectile; }

	void start() override;

	void update() override;
};

struct BowProjectile : public Projectile
{

	BehaviourType type() const override { return BehaviourType::BowProjectile; }

	void start() override;

	void update() override;
};


struct Player : public Behaviour
{
	static const uint8 BASE_HP = 5;
	static const uint8 BASE_LEVEL = 1;
	static const uint8 HP_INCREMENT = 1;
	static const uint8 MAX_LEVEL = 10;
	static const uint8 BASE_SIZE = 65;
	static const uint8 BASE_SPEED = 200;

	#define HitPoints(x) BASE_HP + (x * HP_INCREMENT) - 1;
	#define LevelSize(x, y) y + (x * y * 0.3f);
	#define LevelSpeed(x) BASE_SPEED - (x * 10);

	uint8 level = BASE_LEVEL;
	uint8 hitPoints = BASE_HP;
	uint8 maxHitPoints = BASE_HP;
	uint8 movementSpeed = BASE_SPEED;

	GameObject* lifebar = nullptr;
	GameObject* weapon = nullptr;
	std::string name = "";
	//Spell* spell = nullptr;

	BehaviourType type() const override { return BehaviourType::Player; }

	void start() override;

	void onInput(const InputController &input) override;

	void onMouseInput(const MouseController& input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void write(OutputMemoryStream &packet) override;

	void read(const InputMemoryStream &packet) override;

	void GetChildrenNetworkObjects(std::list<GameObject*>& networkChildren) override;

	// Player State
	enum PlayerState{
		Idle,
		Running,
		Dead
	};
	PlayerState currentState = PlayerState::Idle;
	PlayerType playerType = PlayerType::None;

	void HandleMovementInput(const InputController& input);
	void HandleCombatInput(const InputController& input);

	void UseWeapon();
	void UseSpell();
	void Die();
	void Respawn();
	void LevelUp(uint8 killedLevel);
	bool ChangeState(PlayerState newState);
};

struct DeathGhost : public Behaviour
{
	BehaviourType type() const override { return BehaviourType::DeathGhost; }

	void start() override;
	void update() override;
};
