#pragma once

#define USE_TASK_MANAGER

struct Texture;

class ModuleResources : public Module
{
public:
	Texture *background = nullptr;
	Texture *space = nullptr;
	Texture *asteroid1 = nullptr;
	Texture *asteroid2 = nullptr;

	Texture *laser = nullptr;
	Texture *death = nullptr;

	Texture* berserkerIdle = nullptr;
	Texture* berserkerRun = nullptr;

	Texture* wizardIdle = nullptr;
	Texture* wizardRun = nullptr;

	Texture* hunterIdle = nullptr;
	Texture* hunterRun = nullptr;

	AnimationClip *deathClip = nullptr;

	AnimationClip* playerIdleClip = nullptr;
	AnimationClip* playerRunClip = nullptr;

	AudioClip *audioClipLaser = nullptr;
	AudioClip *audioClipDeath = nullptr;

	bool finishedLoading = false;

	Texture* GetTextureByFile(std::string fileName);

private:

	bool init() override;

#if defined(USE_TASK_MANAGER)
	
	class TaskLoadTexture : public Task
	{
	public:

		const char *filename = nullptr;
		Texture **texture = nullptr;

		void execute() override;
	};

	static const int MAX_RESOURCES = 16;
	TaskLoadTexture tasks[MAX_RESOURCES] = {};
	uint32 taskCount = 0;
	uint32 finishedTaskCount = 0;

	void onTaskFinished(Task *task) override;

	void loadTextureAsync(const char *filename, Texture **texturePtrAddress);

#endif

};

