
#include "Networks.h"

GameObject *spaceTopLeft = nullptr;
GameObject *spaceTopRight = nullptr;
GameObject *spaceBottomLeft = nullptr;
GameObject *spaceBottomRight = nullptr;

void ScreenGame::enable()
{
	if (isServer)
	{
		App->modNetServer->setListenPort(serverPort);
		App->modNetServer->setEnabled(true);
	}
	else
	{
		App->modNetClient->setServerAddress(serverAddress, serverPort);
		App->modNetClient->setPlayerInfo(playerName, classType);
		App->modNetClient->setEnabled(true);
	}

	spaceTopLeft = Instantiate();
	spaceTopLeft->sprite = App->modRender->addSprite(spaceTopLeft);
	spaceTopLeft->sprite->texture = App->modResources->grass;
	spaceTopLeft->sprite->order = -1;
	spaceTopRight = Instantiate();
	spaceTopRight->sprite = App->modRender->addSprite(spaceTopRight);
	spaceTopRight->sprite->texture = App->modResources->grass;
	spaceTopRight->sprite->order = -1;
	spaceBottomLeft = Instantiate();
	spaceBottomLeft->sprite = App->modRender->addSprite(spaceBottomLeft);
	spaceBottomLeft->sprite->texture = App->modResources->grass;
	spaceBottomLeft->sprite->order = -1;
	spaceBottomRight = Instantiate();
	spaceBottomRight->sprite = App->modRender->addSprite(spaceBottomRight);
	spaceBottomRight->sprite->texture = App->modResources->grass;
	spaceBottomRight->sprite->order = -1;
}

void ScreenGame::update()
{
	if (!(App->modNetServer->isConnected() || App->modNetClient->isConnected()))
	{
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenMainMenu);
	}
	else
	{
		if (!isServer)
		{
			vec2 camPos = App->modRender->cameraPosition;
			vec2 bgSize = spaceTopLeft->sprite->texture->size;
			spaceTopLeft->position = bgSize * floor(camPos / bgSize);
			spaceTopRight->position = bgSize * (floor(camPos / bgSize) + vec2{ 1.0f, 0.0f });
			spaceBottomLeft->position = bgSize * (floor(camPos / bgSize) + vec2{ 0.0f, 1.0f });
			spaceBottomRight->position = bgSize * (floor(camPos / bgSize) + vec2{ 1.0f, 1.0f });;
		}
	}
}

bool compareLevel(const Player& first, const Player& second)
{
	if (first.level == second.level)
		return (first.name < second.name);
	else
		return (first.level > second.level);
}

void ScreenGame::gui()
{
	std::list<Player> playersList = App->modBehaviour->GetPlayersList();
	playersList.sort(compareLevel);

	ImGuiWindowFlags window_flags = 0;
	window_flags |= ImGuiWindowFlags_NoBackground;
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	window_flags |= ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoResize;

	ImGui::SetNextWindowPos(ImVec2(750, 0));
	ImGui::SetNextWindowSize(ImVec2(225, 250));

	bool open = true;
	ImGui::Begin("LeaderBoard", &open, window_flags);

	uint8 count = 1;
	bool printedSelf = false;

	ImGui::Columns(3, "entries", false);
	ImGui::SetColumnWidth(0, 25);
	ImGui::SetColumnWidth(1, 125);
	ImGui::SetColumnWidth(2, 75);
	for (std::list<Player>::iterator it = playersList.begin(); it != playersList.end() && count < 6; ++it)
	{
		//ImGui::Text("#%i  %s  Level %i", count, (*it).name.c_str(), (*it).level);
		ImGui::Text("#%i",count);
		ImGui::NextColumn();
		ImGui::Text("%s", (*it).name.c_str());
		ImGui::NextColumn();
		ImGui::Text("Level %i", (*it).level);
		ImGui::NextColumn();

		if (!isServer && App->modNetClient->GetNetworkId() == (*it).gameObject->networkId)
		{
			printedSelf = true;
		}
		++count;
	}
	ImGui::Columns(1);
	if (!isServer && !printedSelf)
	{
		ImGui::Separator();
		ImGui::Columns(3, "playerEntry", false);
		ImGui::SetColumnWidth(0, 25);
		ImGui::SetColumnWidth(1, 125);
		ImGui::SetColumnWidth(2, 75);
		count = 1;
		for (std::list<Player>::iterator it = playersList.begin(); it != playersList.end(); ++it)
		{
			if (App->modNetClient->GetNetworkId() == (*it).gameObject->networkId)
			{
				//ImGui::Text("#%i  %s  Level %i", count, (*it).name.c_str(), (*it).level);
				ImGui::Text("#%i", count);
				ImGui::NextColumn();
				ImGui::Text("%s", (*it).name.c_str());
				ImGui::NextColumn();
				ImGui::Text("Level %i", (*it).level);
				ImGui::NextColumn();
				break;
			}
			++count;
		}
	}

	ImGui::End();
}

void ScreenGame::disable()
{
	Destroy(spaceTopLeft);
	Destroy(spaceTopRight);
	Destroy(spaceBottomLeft);
	Destroy(spaceBottomRight);
}
