#include "Networks.h"
#include "ModuleResources.h"


#if defined(USE_TASK_MANAGER)

void ModuleResources::TaskLoadTexture::execute()
{
	*texture = App->modTextures->loadTexture(filename);
}

#endif


Texture* ModuleResources::GetTextureByFile(std::string fileName)
{
	if (fileName == "background.jpg") return background;
	else if (fileName == "arena.png") return grass;
	else if (fileName == "death_animation.png") return death;
	else if (fileName == "berserker_idle.png") return berserkerIdle;
	else if (fileName == "berserker_run.png") return berserkerRun;
	else if (fileName == "wizard_idle.png") return wizardIdle;
	else if (fileName == "wizard_run.png") return wizardRun;
	else if (fileName == "hunter_idle.png") return hunterIdle;
	else if (fileName == "hunter_run.png") return hunterRun;
	else if (fileName == "Waraxe.png") return axe;
	else if (fileName == "Staff.png") return staff;
	else if (fileName == "Bow.png") return bow;
	else if (fileName == "Waraxe_p.png") return axeProjectile;
	else if (fileName == "Staff_p.png") return staffProjectile;
	else if (fileName == "Bow_p.png") return bowProjectile;
	else if (fileName == "chargeEffect.png") return chargeEffect;
	else if (fileName == "iceSpike.png") return iceSpike;
	
}
bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");

#if !defined(USE_TASK_MANAGER)
	grass = App->modTextures->loadTexture("arena.jpg");
	death = App->modTextures->loadTexture("death_animation.png");
	berserkerIdle = App->modTextures->loadTexture("berserker_idle.png");
	berserkerRun = App->modTextures->loadTexture("berserker_run.png");
	wizardIdle = App->modTextures->loadTexture("wizard_idle.png");
	wizardRun = App->modTextures->loadTexture("wizard_run.png");
	hunterIdle = App->modTextures->loadTexture("hunter_idle.png");
	hunterRun = App->modTextures->loadTexture("hunter_run.png");
	axe = App->modTextures->loadTexture("Waraxe.png");
	staff = App->modTextures->loadTexture("Staff.png");
	bow = App->modTextures->loadTexture("Bow.png");
	axeProjectile = App->modTextures->loadTexture("Waraxe_p.png");
	staffProjectile = App->modTextures->loadTexture("Staff_p.png");
	bowProjectile = App->modTextures->loadTexture("Bow_p.png");
	chargeEffect = App->modTextures->loadTexture("chargeEffect.png");
	iceSpike = App->modTextures->loadTexture("iceSpike.png");

	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("arena.png",			 &grass);
	loadTextureAsync("death_animation.png",  &death);
	loadTextureAsync("berserker_idle.png",	 &berserkerIdle);
	loadTextureAsync("berserker_run.png",	 &berserkerRun);
	loadTextureAsync("wizard_idle.png",		 &wizardIdle);
	loadTextureAsync("wizard_run.png",		 &wizardRun);
	loadTextureAsync("hunter_idle.png",		 &hunterIdle);
	loadTextureAsync("hunter_run.png",		 &hunterRun);
	loadTextureAsync("Waraxe.png",			 &axe);
	loadTextureAsync("Staff.png",			 &staff);
	loadTextureAsync("Bow.png",				 &bow);
	loadTextureAsync("Waraxe_p.png",		 &axeProjectile);
	loadTextureAsync("Staff_p.png",			 &staffProjectile);
	loadTextureAsync("Bow_p.png",			 &bowProjectile);
	loadTextureAsync("chargeEffect.png",	 &chargeEffect);
	loadTextureAsync("iceSpike.png",		 &iceSpike);
#endif

	audioClipDeath = App->modSound->loadAudioClip("death.wav");

	return true;
}

#if defined(USE_TASK_MANAGER)

void ModuleResources::loadTextureAsync(const char * filename, Texture **texturePtrAddress)
{
	ASSERT(taskCount < MAX_RESOURCES);
	
	TaskLoadTexture *task = &tasks[taskCount++];
	task->owner = this;
	task->filename = filename;
	task->texture = texturePtrAddress;

	App->modTaskManager->scheduleTask(task, this);
}

void ModuleResources::onTaskFinished(Task * task)
{
	ASSERT(task != nullptr);

	TaskLoadTexture *taskLoadTexture = dynamic_cast<TaskLoadTexture*>(task);

	for (uint32 i = 0; i < taskCount; ++i)
	{
		if (task == &tasks[i])
		{
			finishedTaskCount++;
			task = nullptr;
			break;
		}
	}

	ASSERT(task == nullptr);

	if (finishedTaskCount == taskCount)
	{
		finishedLoading = true;

		// Create the explosion animation clip
		deathClip = App->modRender->addAnimationClip();
		deathClip->frameTime = 0.1f;
		deathClip->loop = true;
		for (int i = 0; i < 3; ++i)
		{
			float x = (i % 3) / 3.0f;
			float y = 0.f;
			float w = 1.0f / 3.0f;
			float h = 1.0f;
			deathClip->addFrameRect(vec4{ x, y, w, h });
		}

		//Create player idle animation clip
		playerIdleClip = App->modRender->addAnimationClip();
		playerIdleClip->frameTime = 0.2f;
		playerIdleClip->loop = true;
		for (int i = 0; i < 4; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = 0;
			float w = 1.0f / 4.0f;
			float h = 1.0f;
			playerIdleClip->addFrameRect(vec4{ x, y, w, h });
		}

		//Create player run animation clip
		playerRunClip = App->modRender->addAnimationClip();
		playerRunClip->frameTime = 0.1f;
		playerRunClip->loop = true;
		for (int i = 0; i < 7; ++i)
		{
			float x = (i % 7) / 7.0f;
			float y = 0;
			float w = 1.0f / 7.0f;
			float h = 1.0f;
			playerRunClip->addFrameRect(vec4{ x, y, w, h });
		}

		//Create charge effect animation clip
		chargeEffectClip = App->modRender->addAnimationClip();
		chargeEffectClip->frameTime = 0.075f;
		chargeEffectClip->loop = true;
		for (int i = 0; i < 4; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = 0;
			float w = 1.0f / 4.0f;
			float h = 1.0f;
			chargeEffectClip->addFrameRect(vec4{ x, y, w, h });
		}
	}
}

#endif
