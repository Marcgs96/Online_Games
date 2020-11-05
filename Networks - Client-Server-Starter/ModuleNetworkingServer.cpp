#include "ModuleNetworkingServer.h"




//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer public methods
//////////////////////////////////////////////////////////////////////

bool ModuleNetworkingServer::start(int port)
{
	// TODO(jesus): TCP listen socket stuff
	// - Create the listenSocket
	// - Set address reuse
	// - Bind the socket to a local interface
	// - Enter in listen mode
	// - Add the listenSocket to the managed list of sockets using addSocket()
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


	//Message about player disconnection to all other users
	OutputMemoryStream packet;
	packet.Write(ServerMessage::ChatText);
	packet.Write(disconnectedPlayer + " has left the Barrens chat");

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
	ClientMessage clientMessage;
	packet.Read(clientMessage);

	switch (clientMessage)
	{
		case ClientMessage::Hello: HandleHelloMessage(socket, packet); break;
		case ClientMessage::ChatText: HandleChatMessage(socket, packet); break;
		case ClientMessage::UserList: HandleListMessage(socket, packet); break;
		case ClientMessage::Kick: HandleKickMessage(socket, packet); break;
	}
}

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingServer private methods
//////////////////////////////////////////////////////////////////////

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

	if (IsUniquePlayer(playerName))
	{
		// Set the player name of the corresponding connected socket proxy
		for (auto& connectedSocket : connectedSockets)
		{
			if (connectedSocket.socket == socket)
			{
				connectedSocket.playerName = playerName;

				//Send welcome message to the new player
				OutputMemoryStream packet;
				packet.Write(ServerMessage::Welcome);
				packet.Write("You have joined the Barrens chat.");
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
			else {

				//Message about player connection to all other users.
				OutputMemoryStream packet;
				packet.Write(ServerMessage::ChatText);
				packet.Write(playerName + " has joined the Barrens chat!");
				if (sendPacket(packet, connectedSocket.socket))
				{
					LOG("Successfully sent new connection packet");
				}
				else
				{
					//TODO: kick the fucker out
					reportError("sending new connection packet");
				}
			}
		}
	}
	else
	{
		//Send non-welcome packet
		OutputMemoryStream packet;
		packet.Write(ServerMessage::NonWelcome);
		packet.Write("Client name is already in use. Please change your name.");
		if (sendPacket(packet, socket))
		{
			LOG("Successfully sent non-welcome packet");
		}
		else
		{
			//TODO: kick the fucker out
			reportError("sending non-welcome packet");
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
	outputPacket.Write(ServerMessage::ChatText);
	outputPacket.Write(chatMessage);

	for (auto& connectedSocket : connectedSockets)
	{
		//Send new text message to client
		if (sendPacket(outputPacket, connectedSocket.socket))
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
	outputPacket.Write(ServerMessage::ChatText);
	outputPacket.Write(userNames);

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
	for (auto& connectedSocket : connectedSockets) {	
		if (connectedSocket.socket == socket)
		{
			Kick(connectedSocket.socket);
		}
	}
}

void ModuleNetworkingServer::Kick(SOCKET socket)
{
	shutdown(socket, 2);
	closesocket(socket);
	//disconnectedSockets.push_back(socket);
	//onSocketDisconnected(socket);
}


