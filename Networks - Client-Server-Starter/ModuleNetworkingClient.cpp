#include "ModuleNetworkingClient.h"

bool  ModuleNetworkingClient::start(const char * serverAddressStr, int serverPort, const char *pplayerName)
{
	playerName = pplayerName;

	// TODO(jesus): TCP connection stuff
	// - Create the socket
	// - Create the remote address object
	// - Connect to the remote address
	// - Add the created socket to the managed list of sockets using addSocket()

	// If everything was ok... change the state

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

		//int result = send(connectSocket, playerName.c_str(), playerName.size(), 0);
		//if (result != SOCKET_ERROR) {
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
			ImGui::EndChild();

			static char textInput[1024];
			if (ImGui::InputText("", textInput, IM_ARRAYSIZE(textInput), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				//If enter was hit, send chat message packet
				OutputMemoryStream packet;
				packet.Write(ClientMessage::ChatText);
				std::string chatMessage = playerName + ": " + textInput;
				packet.Write(chatMessage);

				if (!sendPacket(packet, connectSocket))
				{
					reportError("sending client chat message");
				}

				memset(textInput, 0, IM_ARRAYSIZE(textInput));
			}
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
		} break;
	}	
}

void ModuleNetworkingClient::onSocketDisconnected(SOCKET socket)
{
	state = ClientState::Stopped;
}

