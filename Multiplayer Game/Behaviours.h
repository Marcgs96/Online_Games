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

	virtual void writeCreate(OutputMemoryStream &packet) { }

	virtual void readCreate(const InputMemoryStream &packet) { }

	virtual void writeUpdate(OutputMemoryStream& packet) { }

	virtual void readUpdate(const InputMemoryStream& packet) { }

	virtual void OnInterpolationDisable() { }
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
	BowProjectile,
	Spell,
	AxeSpell,
	StaffSpell,
	BowSpell,
	WhirlwindAxeProjectile
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

// Player State
enum class PlayerState : uint8 {
	Idle,
	Running,
	Charging,
	Dead
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

	void writeCreate(OutputMemoryStream& packet) override;
	void readCreate(const InputMemoryStream& packet) override;

	void onMouseInput(const MouseController& input) override;
	void HandleWeaponRotation(const MouseController& input);
};

struct Projectile : public Behaviour
{
	float secondsSinceCreation = 0.0f;
	float velocity = 1000.0f;
	bool perforates = false;
	vec2 direction = { 0,0 };
	uint8 damagePoints = 1;
	
	uint32 shooterID = 0;

	BehaviourType type() const override { return BehaviourType::Projectile; }

	virtual void start() override;

	virtual void update() override;

	void writeCreate(OutputMemoryStream& packet) override;
	void readCreate(const InputMemoryStream& packet) override;
};

struct AxeProjectile : public Projectile
{
	float angleIncrementRatio = 10;
	vec2 direction;

	BehaviourType type() const override { return BehaviourType::AxeProjectile; }

	void start() override;

	void update() override;
};

struct WhirlwindAxeProjectile : public Projectile
{
	float selfRotationIncrementRatio = 10;
	float orbitAngle = 0;
	float orbitSpeed = 2;
	float rotationRadius = 150;

	vec2 direction;
	uint8 index = 0;

	GameObject* player = nullptr;

	BehaviourType type() const override { return BehaviourType::WhirlwindAxeProjectile; }

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

struct Spell : public Behaviour
{
	float spellCooldown = 1.0f;
	float spellCooldownTimer = 1.0f;

	GameObject* player = nullptr;

	BehaviourType type() const override { return BehaviourType::Spell; }

	virtual void start() override;
	virtual void update() override;

	virtual void Use();
};

struct AxeSpell : public Spell
{
	static const uint8 NUM_AXES = 3;
	BehaviourType type() const override { return BehaviourType::AxeSpell; }
	GameObject* axes[NUM_AXES];

	virtual void start() override;
	virtual void update() override;

	virtual void Use();
};

struct StaffSpell : public Spell
{
	BehaviourType type() const override { return BehaviourType::StaffSpell; }

	virtual void start() override;
	virtual void update() override;

	virtual void Use() override;
};

struct BowSpell : public Spell
{
	//Constants
	static const uint8 MIN_CHARGE = 0;
	static const uint8 MAX_CHARGE = 3;
	static const uint8 MIN_DAMAGE = 2;
	static const uint8 MAX_DAMAGE = 15;
	static const uint8 MAX_INCREASE = 3;
	static const uint8 MIN_INCREASE = 1;

	float chargeTime = 0.0f;
	bool charging = false;

	GameObject* chargeEffect = nullptr;

	BehaviourType type() const override { return BehaviourType::BowSpell; }

	void start() override;
	void Use() override;
	void Hold();
	void Release();
	void onInput(const InputController& input) override;
};

struct Player : public Behaviour
{
	static const uint8 BASE_HP = 5;
	static const uint8 BASE_LEVEL = 1;
	static const uint8 HP_INCREMENT = 1;
	static const uint8 MAX_LEVEL = 10;
	static const uint8 BASE_SIZE = 65;
	static const uint8 BASE_SPEED = 200;

	inline int HitPoints(uint8 x) { return (BASE_HP + (x * HP_INCREMENT) - 1); }
	inline int LevelSize(uint8 x, uint8 y){ return y + (y * x * 0.2f); }
	inline int LevelSpeed(uint8 x) { return BASE_SPEED - (x * 10); }

	uint8 level = BASE_LEVEL;
	uint8 hitPoints = BASE_HP;
	uint8 maxHitPoints = BASE_HP;
	uint8 movementSpeed = BASE_SPEED;

	GameObject* lifebar = nullptr;
	GameObject* weapon = nullptr;
	std::string name = "";

	//////Spell///////
	Spell* spell = nullptr;
	/////////////////

	BehaviourType type() const override { return BehaviourType::Player; }

	void start() override;

	void onInput(const InputController &input) override;

	void onMouseInput(const MouseController& input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider &c1, Collider &c2) override;

	void writeCreate(OutputMemoryStream& packet) override;
	void readCreate(const InputMemoryStream& packet) override;
	void writeUpdate(OutputMemoryStream& packet) override;
	void readUpdate(const InputMemoryStream& packet) override;

	void GetChildrenNetworkObjects(std::list<GameObject*>& networkChildren) override;

	void OnInterpolationDisable() override;

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
