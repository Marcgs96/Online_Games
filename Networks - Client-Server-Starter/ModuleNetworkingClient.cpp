#include "ModuleNetworkingClient.h"
#include <iostream>
#include <sstream>

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	//Todo: read this from file
	commands.emplace("/help", CommandType::Help);
	commands.emplace("/list", CommandType::List);
	commands.emplace("/kick", CommandType::Kick);
	commands.emplace("/whisper", CommandType::Whisper);
	commands.emplace("/change_name", CommandType::ChangeName);
	commands.emplace("/ban", CommandType::Ban);
	commands.emplace("/unban", CommandType::Unban);
	helpMessage = "All available commands are: \n/list to list all users. \n/kick [username] to kick the player from the chat. \n/whisper [username] [message] to send a private message. \n/change_name [newname] to change your username.";



	connectSocket = socket(AF_INET, SOCK_STREAM, 0);

	//Server address creation
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	inet_pton(AF_INET, serverAddressStr, &serverAddress.sin_addr);

	int result = connect(connectSocket, (const struct sockaddr*)&serverAddress, sizeof(serverAddress));

	if (result != SOCKET_ERROR) {
		addSocket(connectSocket);
		state = ClientState::Start;
	}
	else {
		reportError("connecting the client socket");
	}

	return true;
}

bool ModuleNetworkingClient::isRunning() const
{
	return state != ClientState::Stopped;
}

bool ModuleNetworkingClient::update()
{
	if (state == ClientState::Start)
	{
		OutputMemoryStream packet;
		packet.Write(ClientMessage::Hello);
		packet.Write(playerName);

		if (sendPacket(packet, connectSocket))
		{
			state = ClientState::Logging;
		}
		else {
			disconnect();
			state = ClientState::Stopped;
			reportError("sending hello");
		}
	}
	return true;
}

bool ModuleNetworkingClient::gui()
{
	if (state != ClientState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Client Window");

		Texture *tex = App->modResources->client;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		if (state == ClientState::Logging)
		{
			ImGui::Text("Connecting to the server...");
		}
		else if (state == ClientState::LoggedIn)
		{
			ImGui::Text("Welcome to The Barrens Chat %s", playerName.c_str());
			ImGui::SameLine();
			if (ImGui::Button("Logout"))
			{
				disconnect();
				state = ClientState::Stopped;
			}
			ImGui::BeginChild("Chat", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 32), true);
			for (std::string message : receivedMessages)
			{
				ImGui::Text(message.c_str());
			}
			if (scrollDown)
			{
				ImGui::SetScrollHere(1.0f);
				scrollDown = false;
			}


			ImGui::EndChild();

			static char textInput[1024];
			if (ImGui::InputText("", textInput, IM_ARRAYSIZE(textInput), ImGuiInputTextFlags_EnterReturnsTrue) /*&& !timedOut*/)
			{
				std::string textString = textInput;
				if (textString[0] == '/') { // check if the text is a command

					std::string delimiter = " ";
					size_t pos = 0;
					std::vector<std::string> splitString;

					//Split the command and the first parameter of it from the text input
					for (int i = 0; i < 2; ++i) {
						pos = textString.find(delimiter);
						pos = pos != std::string::npos? pos:textString.size();

						splitString.push_back(textString.substr(0, pos));
						textString.erase(0, pos + delimiter.length());
					}

					splitString.push_back(textString);

					HandleCommands(splitString);
				}
				else {
					//If enter was hit, send chat message packet
					OutputMemoryStream packet;
					packet.Write(ClientMessage::ChatText);
					std::string chatMessage = playerName + ": " + textString;
					packet.Write(chatMessage);

					if (!sendPacket(packet, connectSocket))
					{
						reportError("sending client chat message");
					}
				}

				memset(textInput, 0, IM_ARRAYSIZE(textInput));
			}
			if (ImGui::IsItemHovered() || (ImGui::IsRootWindowOrAnyChildFocused() && !ImGui::IsAnyItemActive() && !ImGui::IsMouseClicked(0)))
				ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}
		ImGui::End();
	}

	return true;
}

void ModuleNetworkingClient::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	ServerMessage serverMessage;
	packet.Read(serverMessage);

	switch (serverMessage)
	{
		case ServerMessage::Welcome:
		{
			std::string welcomeMessage;
			packet.Read(welcomeMessage);

			receivedMessages.push_back(welcomeMessage);
			scrollDown = true;

			state = ClientState::LoggedIn;
		} break;
		case ServerMessage::NonWelcome:
		{
			std::string nonWelcomeMessage;
			packet.Read(nonWelcomeMessage);

			ELOG(nonWelcomeMessage.c_str());

			disconnect();
			state = ClientState::Stopped;
		} break;
		case ServerMessage::ChatText:
		{
			std::string chatMessage;
			packet.Read(chatMessage);

			receivedMessages.push_back(chatMessage);
			scrollDown = true;
		} break;
		case ServerMessage::ChangeName:
		{
			std::string chatMessage;
			std::string newName;
			packet.Read(chatMessage);
			packet.Read(newName);

			receivedMessages.push_back(chatMessage);
			scrollDown = true;
			playerName = newName;

		} break;
		case ServerMessage::Timeout:
		{
			std::string chatMessage;
			packet.Read(chatMessage);

			receivedMessages.push_back(chatMessage);
			scrollDown = true;
			timedOut = true;
		} break;
		case ServerMessage::ReleaseTimeout:
		{
			std::string chatMessage;
			packet.Read(chatMessage);

			receivedMessages.push_back(chatMessage);
			scrollDown = true;
			timedOut = false;
		} break;
	}	
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

void ModuleNetworkingClient::HandleCommands(std::vector<std::string> splitString)
{
	if (splitString.size() < 0)
	{
		reportError("handling command since split failed");
		return;
	}

	CommandType type = commands[splitString[0]];

	switch (type)
	{
	case ModuleNetworkingClient::CommandType::Help:
		receivedMessages.push_back(helpMessage);
		scrollDown = true;
		break;
	case ModuleNetworkingClient::CommandType::List: 
	{
		OutputMemoryStream packet;
		packet.Write(ClientMessage::UserList);

		if (!sendPacket(packet, connectSocket))
		{
			reportError("sending client list command message");
		}
	}break;
	case ModuleNetworkingClient::CommandType::Kick:
	{
		if(splitString.size() > 1) {
			OutputMemoryStream packet;

			packet.Write(ClientMessage::Kick);
			packet.Write(splitString[1]);
			if (!sendPacket(packet, connectSocket))
			{
				reportError("sending kick command");
			}
		}
	} break;
	case ModuleNetworkingClient::CommandType::Whisper:
	{
		if (splitString.size() > 1) {
			OutputMemoryStream packet;

			packet.Write(ClientMessage::Whisper);
			packet.Write(splitString[1]);
			packet.Write(splitString[2]);
			packet.Write(playerName);
			if (!sendPacket(packet, connectSocket))
			{
				reportError("sending whisper command");
			}
		}
	} break;
	case ModuleNetworkingClient::CommandType::ChangeName:
	{
		if (splitString.size() > 1) {
			OutputMemoryStream packet;

			packet.Write(ClientMessage::ChangeName);
			packet.Write(playerName);
			packet.Write(splitString[1]);

			if (!sendPacket(packet, connectSocket))
			{
				reportError("sending change name command");
			}
		}
	} break;
	case ModuleNetworkingClient::CommandType::Ban:
	{
		if (splitString.size() > 1) {
			OutputMemoryStream packet;

			packet.Write(ClientMessage::Ban);
			packet.Write(playerName);
			packet.Write(splitString[1]);

			if (!sendPacket(packet, connectSocket))
			{
				reportError("sending ban command");
			}
		}
	} break;
	case ModuleNetworkingClient::CommandType::Unban:
	{
		if (splitString.size() > 1) {
			OutputMemoryStream packet;

			packet.Write(ClientMessage::Unban);
			packet.Write(playerName);
			packet.Write(splitString[1]);

			if (!sendPacket(packet, connectSocket))
			{
				reportError("sending unban command");
			}
		}
	} break;
	default:
		break;
	}
}

