#pragma once


enum class BehaviourType : uint8;
struct Spell;
struct AxeSpell;

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

struct Player : public Behaviour
{
	static const uint8 BASE_HP = 5;
	static const uint8 BASE_LEVEL = 1;
	static const uint8 HP_INCREMENT = 1;
	static const uint8 MAX_LEVEL = 10;
	static const uint8 BASE_SIZE = 65;
	static const uint8 BASE_SPEED = 200;

	inline int HitPoints(uint8 x) { return (BASE_HP + (x * HP_INCREMENT) - 1); }
	inline int LevelSize(uint8 x, uint8 y) { return y + (y * x * 0.2f); }
	inline int LevelSpeed(uint8 x) { return BASE_SPEED - (x * 10); }

	uint8 level = BASE_LEVEL;
	uint8 hitPoints = BASE_HP;
	uint8 maxHitPoints = BASE_HP;
	uint8 movementSpeed = BASE_SPEED;

	vec2 movement_vector;

	GameObject* lifebar = nullptr;
	GameObject* weapon = nullptr;
	std::string name = "";

	//////Spell///////
	Spell* spell = nullptr;
	/////////////////

	BehaviourType type() const override { return BehaviourType::Player; }

	void start() override;

	void onInput(const InputController& input) override;

	void onMouseInput(const MouseController& input) override;

	void update() override;

	void destroy() override;

	void onCollisionTriggered(Collider& c1, Collider& c2) override;

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

struct Weapon : public Behaviour 
{
	BehaviourType type() const override { return BehaviourType::Weapon; }

	WeaponType weaponType = WeaponType::None;
	GameObject* player = nullptr;
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
	bool isFake = false;

	float secondsSinceCreation = 0.0f;
	float velocity = 1000.0f;
	bool perforates = false;
	vec2 direction = { 0,0 };
	uint8 damagePoints = 1;
	float lifetimeSeconds = 5.0f;
	Player* player = nullptr;
	
	uint32 shooterID = 0;

	BehaviourType type() const override { return BehaviourType::Projectile; }

	virtual void start() override;

	virtual void update() override;

	virtual bool CanDamagePlayer(GameObject* player) { return true; }

	void writeCreate(OutputMemoryStream& packet) override;
	void readCreate(const InputMemoryStream& packet) override;
};

struct AxeProjectile : public Projectile
{
	float angleIncrementRatio = 10;

	BehaviourType type() const override { return BehaviourType::AxeProjectile; }

	void start() override;

	void update() override;
};

struct WhirlwindAxeProjectile : public Projectile
{
	float selfRotationIncrementRatio = 15;
	float orbitAngle = 0;
	float orbitSpeed = 1;
	std::unordered_map<uint32, float> playersDamaged;
	float secondsToDamageAgain = 2.0f;
	float rotationRadius = 150;

	uint8 index = 0;

	BehaviourType type() const override { return BehaviourType::WhirlwindAxeProjectile; }

	void start() override;

	void update() override;

	bool CanDamagePlayer(GameObject* player) override;

	void HandleDamageTimers();
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
	float spellCooldown = 10.0f;
	float spellCooldownTimer = 10.0f;

	Player* player = nullptr;

	BehaviourType type() const override { return BehaviourType::Spell; }

	virtual void start() override;
	virtual void update() override;
	virtual void OnLevelUp() {}
	virtual void OnDeath() {}

	virtual void Use();
};

struct AxeSpell : public Spell
{
	static const uint8 NUM_AXES = 3;
	BehaviourType type() const override { return BehaviourType::AxeSpell; }
	GameObject* axes[NUM_AXES] = { nullptr };
	float rotationRadius = 150;
	float orbitSpeed = 1;

	float activeTime = 8;
	float activeTimeTimer = 0;

	bool isActive = false;

	void start() override;
	void update() override;
	void GetChildrenNetworkObjects(std::list<GameObject*>& networkChildren) override;

	virtual void Use();

	void onInput(const InputController& input) override;
	void OnLevelUp() override;
	void OnDeath() override;
};

struct StaffSpell : public Spell
{
	static const uint8 NUM_ORBS = 8;
	BehaviourType type() const override { return BehaviourType::StaffSpell; }
	GameObject* orbs[NUM_ORBS];

	void start() override;
	void update() override;

	virtual void Use();
	void OnDeath() override;

	void onInput(const InputController& input) override;
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

	GameObject* chargeEffect = nullptr;

	BehaviourType type() const override { return BehaviourType::BowSpell; }

	void start() override;
	void Use() override;
	void Hold();
	void Release();
	void onInput(const InputController& input) override;
	void OnDeath() override;
};
