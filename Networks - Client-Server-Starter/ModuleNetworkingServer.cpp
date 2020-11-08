#include "ModuleNetworkingServer.h"

#define MAX_TIMEOUT_SCORE 4.0f
#define TIMEOUT_TIME 10.0f
#define TIMEOUT_DAMPENING 0.5f

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	//Remote Address creation
	sockaddr_in bindAddr;
	bindAddr.sin_family = AF_INET; // IPv4
	bindAddr.sin_port = htons(port); // Port
	bindAddr.sin_addr.S_un.S_addr = INADDR_ANY; // Any local IP address

	//Bind remote address
	int result = bind(listenSocket, (const struct sockaddr*)&bindAddr, sizeof(bindAddr));
	if (result == SOCKET_ERROR) {
		reportError("binding listener socket!");
	}

	listen(listenSocket, 1);

	addSocket(listenSocket);

	state = ServerState::Listening;

	return true;
}

bool ModuleNetworkingServer::isRunning() const
{
	return state != ServerState::Stopped;
}

//////////////////////////////////////////////////////////////////////
// Module virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::update()
{
	for (std::map<SOCKET, float>::iterator it = timedOutClients.begin(); it != timedOutClients.end();)
	{
		it->second += 1.0f * Time.deltaTime;
		if (it->second >= TIMEOUT_TIME)
		{
			OutputMemoryStream packet;
			WritePacket(packet, ServerMessage::ReleaseTimeout, "Your timeout has been lifted, welcome back.");

			if (sendPacket(packet, it->first))
			{
				it = timedOutClients.erase(it);
				LOG("Successfully sent disconnection chat packet");
			}
			else
			{
				reportError("sending disconnection chat packet");
			}		
		}
		else
			++it;
	}

	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{		
		it->timeOutScore -= TIMEOUT_DAMPENING * Time.deltaTime;
		if (it->timeOutScore < 0.0f)
			it->timeOutScore = 0.0f;
	}

	return true;
}

bool ModuleNetworkingServer::gui()
{
	if (state != ServerState::Stopped)
	{
		// NOTE(jesus): You can put ImGui code here for debugging purposes
		ImGui::Begin("Server Window");

		Texture *tex = App->modResources->server;
		ImVec2 texSize(400.0f, 400.0f * tex->height / tex->width);
		ImGui::Image(tex->shaderResource, texSize);

		ImGui::Text("List of connected sockets:");

		for (auto &connectedSocket : connectedSockets)
		{
			ImGui::Separator();
			ImGui::Text("Socket ID: %d", connectedSocket.socket);
			ImGui::Text("Address: %d.%d.%d.%d:%d",
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b1,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b2,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b3,
				connectedSocket.address.sin_addr.S_un.S_un_b.s_b4,
				ntohs(connectedSocket.address.sin_port));
			ImGui::Text("Player name: %s", connectedSocket.playerName.c_str());
		}

		ImGui::End();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::isListenSocket(SOCKET socket) const
{
	return socket == listenSocket;
}

void ModuleNetworkingServer::onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress)
{
	// Add a new connected socket to the list
	ConnectedSocket connectedSocket;
	connectedSocket.socket = socket;
	connectedSocket.address = socketAddress;
	connectedSockets.push_back(connectedSocket);
}

void ModuleNetworkingServer::onSocketDisconnected(SOCKET socket)
{
	std::string disconnectedPlayer;
	// Remove the connected socket from the list
	for (auto it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		auto& connectedSocket = *it;
		if (connectedSocket.socket == socket)
		{
			disconnectedPlayer = connectedSocket.playerName;
			connectedSockets.erase(it);
			break;
		}
	}

	if (disconnectedPlayer == "")
		return;

	//Message about player disconnection to all other users
	OutputMemoryStream packet;
	std::string chatMessage = disconnectedPlayer + " has left the Barrens chat";
	WritePacket(packet, ServerMessage::ChatText, chatMessage, Color::Gray());

	for (auto &connectedSocket : connectedSockets) {
		if (sendPacket(packet, connectedSocket.socket))
		{
			LOG("Successfully sent disconnection chat packet");
		}
		else
		{
			reportError("sending disconnection chat packet");
		}
	}
}

void ModuleNetworkingServer::onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > 0)
	{
		ClientMessage clientMessage;
		packet.Read(clientMessage);

		switch (clientMessage)
		{
		case ClientMessage::Hello: HandleHelloMessage(socket, packet); break;
		case ClientMessage::ChatText: HandleChatMessage(socket, packet); break;
		case ClientMessage::UserList: HandleListMessage(socket, packet); break;
		case ClientMessage::Kick: HandleKickMessage(socket, packet); break;
		case ClientMessage::Whisper: HandleWhisperMessage(socket, packet); break;
		case ClientMessage::ChangeName: HandleNameChangeMessage(socket, packet); break;
		case ClientMessage::Ban: HandleBanMessage(socket, packet); break;
		case ClientMessage::Unban: HandleUnbanMessage(socket, packet); break;
		}
	}
}

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer private methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingServer::WritePacket(OutputMemoryStream& packet, ServerMessage type, std::string message, Color textColor)
{
	packet.Write(type);
	packet.Write(message);

	switch (type)
	{
	case ServerMessage::Welcome:
		if (textColor.IsEmpty())
			Color::Green().Write(packet);
		else
			textColor.Write(packet);
		break;
	case ServerMessage::NonWelcome:
		if (textColor.IsEmpty())
			Color::Red().Write(packet);
		break;
	case ServerMessage::ChatText:
		if (textColor.IsEmpty())
			Color::White().Write(packet);
		else
			textColor.Write(packet);
		break;
	case ServerMessage::UserList:
		if (textColor.IsEmpty())
			Color::Cyan().Write(packet);
		else
			textColor.Write(packet);
		break;
	case ServerMessage::ChangeName:
		if (textColor.IsEmpty())
			Color::Cyan().Write(packet);
		else
			textColor.Write(packet);
		break;
	case ServerMessage::Timeout:
		if (textColor.IsEmpty())
			Color::Red().Write(packet);
		else
			textColor.Write(packet);
		break;
	case ServerMessage::ReleaseTimeout:
		if (textColor.IsEmpty())
			Color::Cyan().Write(packet);
		else
			textColor.Write(packet);
		break;
	default:
		if (textColor.IsEmpty())
			Color::White().Write(packet);
		else
			textColor.Write(packet);
		break;

	}
}

//Return whether the player name is found in the current connected clients or not
bool ModuleNetworkingServer::IsUniquePlayer(std::string playerName)
{
	// Look for the playerName on all connected clients
	for (auto& connectedSocket : connectedSockets)
	{
		if (connectedSocket.playerName == playerName)
		{
			return false;
		}
	}

	return true;
}

// Handles the reception of a packet with Hello type
void ModuleNetworkingServer::HandleHelloMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string playerName;
	packet.Read(playerName);

	bool loggedIn = false;
	if (IsUniquePlayer(playerName))
	{
		// Set the player name of the corresponding connected socket proxy
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				//Check if this scoket ip has been banned from the server, if so kick the fucker out
				for (auto it = blackList.begin(); it != blackList.end(); ++it) {
					char ip[INET6_ADDRSTRLEN];
					inet_ntop(connectedSocket.address.sin_family, &connectedSocket.address.sin_addr, (PSTR)ip, sizeof ip);
					if (it->second == std::string(ip)) {
						Kick(socket);
						return;
					}
				}
				connectedSocket.playerName = playerName;

				//Send welcome message to the new player
				OutputMemoryStream packet;
				WritePacket(packet, ServerMessage::Welcome, "You have joined the Barrens chat.");
				loggedIn = true;

				if (sendPacket(packet, connectedSocket.socket))
				{
					LOG("Successfully sent welcome packet");
				}
				else
				{
					//TODO: kick the fucker out
					reportError("sending welcome packet");
				}
			}
		}
	}
	else
	{
		//Send non-welcome packet
		OutputMemoryStream packet;
		WritePacket(packet, ServerMessage::NonWelcome, "Client name is already in use. Please change your name.");

		if (sendPacket(packet, socket))
		{
			Kick(socket);
			LOG("Successfully sent non-welcome packet");
		}
		else
		{
			reportError("sending non-welcome packet");
		}
	}

	if (loggedIn)
	{
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket != socket)
			{
				//Message about player connection to all other users.
				OutputMemoryStream packet;
				std::string chatMessage = playerName + " has joined the Barrens chat!";
				WritePacket(packet, ServerMessage::ChatText, chatMessage, Color::Green());
				if (sendPacket(packet, connectedSocket.socket))
				{
					LOG("Successfully sent new connection packet");
				}
				else
				{
					reportError("sending new connection packet");
				}
			}
		}
	}
}

// Handles the reception of a packet with Chat type
void ModuleNetworkingServer::HandleChatMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string chatMessage;
	packet.Read(chatMessage);

	//Create chat packet with the new chat message received
	OutputMemoryStream outputPacket;
	WritePacket(outputPacket, ServerMessage::ChatText, chatMessage);

	for (std::vector<ConnectedSocket>::iterator it = connectedSockets.begin(); it != connectedSockets.end(); ++it)
	{
		if (it->socket == socket)
		{
			it->timeOutScore += 1.0f;
			if (it->timeOutScore > MAX_TIMEOUT_SCORE)
			{
				timedOutClients.emplace(it->socket, 0.0f);
				it->timeOutScore = 0.0f;
				
				OutputMemoryStream timeoutPacket;
				WritePacket(timeoutPacket, ServerMessage::Timeout, "You have been timed out.");
				if (sendPacket(timeoutPacket, it->socket))
				{
					LOG("Successfully sent timeout packet");
				}
				else
				{
					reportError("sending timeout packet");
				}
				return;
			}
		}

		//Send new text message to client
		if (sendPacket(outputPacket, it->socket))
		{
			LOG("Successfully sent chat packet");
		}
		else
		{
			reportError("sending chat packet");
		}
	}
}

//Handles user listing message
void ModuleNetworkingServer::HandleListMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string userNames = "Current users connected to the server:\n";

	//Gather all current user names
	for (auto& connectedSocket : connectedSockets) {
		userNames.append("-" + connectedSocket.playerName + "\n");
	}

	//Create chat packet with the new chat message received
	OutputMemoryStream outputPacket;
	WritePacket(outputPacket, ServerMessage::ChatText, userNames, Color::Cyan());


	//Send new text message to client
	if (sendPacket(outputPacket, socket))
	{
		LOG("Successfully sent chat packet");
	}
	else
	{
		reportError("sending chat packet");
	}
}

//Handles kick message
void ModuleNetworkingServer::HandleKickMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string playerName;
	std::string playerKickName;

	packet.Read(playerName);
	packet.Read(playerKickName);

	bool exists = false;

	for (auto& connectedSocket : connectedSockets) {
		if (connectedSocket.playerName == playerKickName) {
			Kick(connectedSocket.socket);
			exists = true;
		}
	}
	if (exists)
	{
		OutputMemoryStream outputPacket;
		WritePacket(outputPacket, ServerMessage::ChatText, playerKickName + " has been kicked from the server by " + playerName + ".", Color::Red());

		for (auto& connectedSocket : connectedSockets) {
			if (sendPacket(outputPacket, connectedSocket.socket)) {
				LOG("Successfully sent kick chat packet");
			}
			else
			{
				reportError("sending kick chat packet");
			}
		}
	}
}

void ModuleNetworkingServer::HandleWhisperMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string fromName;
	std::string toName;
	std::string message;
	packet.Read(toName);
	packet.Read(message);
	packet.Read(fromName);

	bool exists = false;
	for (auto& connectedSocket : connectedSockets) {
		if (connectedSocket.playerName == toName)
		{
			//Create chat packet with the new chat message received
			OutputMemoryStream outputPacketFrom;
			WritePacket(outputPacketFrom, ServerMessage::ChatText, fromName + " whispers: " + message, Color::Purple());

			if (sendPacket(outputPacketFrom, connectedSocket.socket))
			{
				LOG("Successfully sent chat packet");
			}
			else
			{
				reportError("sending chat packet");
			}

			exists = true;
		}
	}

	if (exists)
	{
		OutputMemoryStream outputPacketTo;
		WritePacket(outputPacketTo, ServerMessage::ChatText, "To " + toName + ": " + message, Color::Purple());

		if (sendPacket(outputPacketTo, socket)) {
			LOG("Successfully sent chat packet");
		}
		else
		{
			reportError("sending chat packet");
		}
	}
}

void ModuleNetworkingServer::HandleNameChangeMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string currentName;
	std::string newName;
	packet.Read(currentName);
	packet.Read(newName);

	for (auto& connectedSocket : connectedSockets) {
		if (connectedSocket.playerName == newName) {

			OutputMemoryStream outputPacket;
			WritePacket(outputPacket, ServerMessage::ChatText, "The new name you want to use is not available", Color::Red());


			if (sendPacket(outputPacket, socket)) {
				LOG("Successfully sent chat packet");
			}
			else
			{
				reportError("sending chat packet");
			}

			return;
		}
	}

	for (auto& connectedSocket : connectedSockets) {
		if (connectedSocket.socket == socket) {
			connectedSocket.playerName = newName;

			OutputMemoryStream outputPacket;
			WritePacket(outputPacket, ServerMessage::ChangeName, "You have changed your name to " + newName + " successfully.", Color::Cyan());
			outputPacket.Write(newName);

			if (sendPacket(outputPacket, socket)) {
				LOG("Successfully sent chat packet");
			}
			else
			{
				reportError("sending chat packet");
			}
		}
		else {

			OutputMemoryStream outputPacket;
			WritePacket(outputPacket, ServerMessage::ChatText, currentName + " has changed his name to " + newName + ".", Color::Cyan());

			if (sendPacket(outputPacket, connectedSocket.socket)) {
				LOG("Successfully sent chat packet");
			}
			else
			{
				reportError("sending chat packet");
			}
		}
	}

}

void ModuleNetworkingServer::HandleBanMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string playerName;
	std::string playerBanName;

	packet.Read(playerName);
	packet.Read(playerBanName);

	bool exists = false;

	for (auto& connectedSocket : connectedSockets) {
		if (connectedSocket.playerName == playerBanName) {
			char ip[INET6_ADDRSTRLEN];
			inet_ntop(connectedSocket.address.sin_family, &connectedSocket.address.sin_addr, (PSTR)ip, sizeof ip);
			blackList.emplace(playerBanName, std::string(ip));
			Kick(connectedSocket.socket);
			exists = true;
		}
	}
	if (exists)
	{
		OutputMemoryStream outputPacket;
		WritePacket(outputPacket, ServerMessage::ChatText, playerBanName + " has been banned from the server by " + playerName + ".", Color::Red());

		for (auto& connectedSocket : connectedSockets) {
			if (sendPacket(outputPacket, connectedSocket.socket)) {
				LOG("Successfully sent ban chat packet");
			}
			else
			{
				reportError("sending ban chat packet");
			}
		}
	}
}

void ModuleNetworkingServer::HandleUnbanMessage(SOCKET socket, const InputMemoryStream& packet)
{
	std::string playerName;
	std::string playerUnbanName;

	packet.Read(playerName);
	packet.Read(playerUnbanName);

	bool exists = false;

	auto it = blackList.find(playerUnbanName);

	if (it != blackList.end())
	{
		exists = true;
		blackList.erase(it);
	}

	if (exists)
	{
		OutputMemoryStream outputPacket;
		WritePacket(outputPacket, ServerMessage::ChatText, playerUnbanName + " has been unbanned from the server by " + playerName + ".", Color::Cyan());

		for (auto& connectedSocket : connectedSockets) {
			if (sendPacket(outputPacket, connectedSocket.socket)) {
				LOG("Successfully sent unban chat packet");
			}
			else
			{
				reportError("sending unban chat packet");
			}
		}
	}
}

void ModuleNetworkingServer::Kick(SOCKET socket)
{
	shutdown(socket, 2);
	closesocket(socket);
	onSocketDisconnected(socket);
	sockets.erase(std::find(sockets.begin(), sockets.end(), socket));
}


