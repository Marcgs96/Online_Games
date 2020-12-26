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
	else if (fileName == "space_background.jpg") return space;
	else if (fileName == "asteroid1.png") return asteroid1;
	else if (fileName == "asteroid2.png") return asteroid2;
	else if (fileName == "spacecraft1.png") return spacecraft1;
	else if (fileName == "spacecraft2.png") return spacecraft2;
	else if (fileName == "spacecraft3.png") return spacecraft3;
	else if (fileName == "laser.png") return laser;
	else if (fileName == "explosion1.png") return explosion1;
	else if (fileName == "player_idle.png") return player_idle;
	else if (fileName == "player_run.png") return player_run;
}
bool ModuleResources::init()
{
	background = App->modTextures->loadTexture("background.jpg");

#if !defined(USE_TASK_MANAGER)
	space = App->modTextures->loadTexture("space_background.jpg");
	asteroid1 = App->modTextures->loadTexture("asteroid1.png");
	asteroid2 = App->modTextures->loadTexture("asteroid2.png");
	spacecraft1 = App->modTextures->loadTexture("spacecraft1.png");
	spacecraft2 = App->modTextures->loadTexture("spacecraft2.png");
	spacecraft3 = App->modTextures->loadTexture("spacecraft3.png");
	player_idle = App->modTextures->loadTexture("player_idle.png");
	player_run = App->modTextures->loadTexture("player_run.png");
	loadingFinished = true;
	completionRatio = 1.0f;
#else
	loadTextureAsync("space_background.jpg", &space);
	loadTextureAsync("asteroid1.png",        &asteroid1);
	loadTextureAsync("asteroid2.png",        &asteroid2);
	loadTextureAsync("spacecraft1.png",      &spacecraft1);
	loadTextureAsync("spacecraft2.png",      &spacecraft2);
	loadTextureAsync("spacecraft3.png",      &spacecraft3);
	loadTextureAsync("laser.png",            &laser);
	loadTextureAsync("explosion1.png",       &explosion1);
	loadTextureAsync("player_idle.png",		 &player_idle);
	loadTextureAsync("player_run.png",		 &player_run);
#endif

	audioClipLaser = App->modSound->loadAudioClip("laser.wav");
	audioClipExplosion = App->modSound->loadAudioClip("explosion.wav");
	//App->modSound->playAudioClip(audioClipExplosion);

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
		explosionClip = App->modRender->addAnimationClip();
		explosionClip->frameTime = 0.1f;
		explosionClip->loop = false;
		for (int i = 0; i < 16; ++i)
		{
			float x = (i % 4) / 4.0f;
			float y = (i / 4) / 4.0f;
			float w = 1.0f / 4.0f;
			float h = 1.0f / 4.0f;
			explosionClip->addFrameRect(vec4{ x, y, w, h });
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
	}
}

#endif
