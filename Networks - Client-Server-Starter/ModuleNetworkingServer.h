#pragma once

#include "ModuleNetworking.h"

class ModuleNetworkingServer : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer public methods
	//////////////////////////////////////////////////////////////////////

	bool start(int port);

	bool isRunning() const;



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool update() override;

	bool gui() override;


	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking virtual methods
	//////////////////////////////////////////////////////////////////////

	bool isListenSocket(SOCKET socket) const override;

	void onSocketConnected(SOCKET socket, const sockaddr_in &socketAddress) override;

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingServer private methods
	//////////////////////////////////////////////////////////////////////

	bool IsUniquePlayer(std::string playerName);

	void HandleHelloMessage(SOCKET socket, const InputMemoryStream& packet);

	void HandleChatMessage(SOCKET socket, const InputMemoryStream& packet);
	
	void HandleListMessage(SOCKET socket, const InputMemoryStream& packet);

	void HandleKickMessage(SOCKET socket, const InputMemoryStream& packet);

	void HandleWhisperMessage(SOCKET socket, const InputMemoryStream& packet);

	void HandleNameChangeMessage(SOCKET socket, const InputMemoryStream& packet);

	void HandleNameBanMessage(SOCKET socket, const InputMemoryStream& packet);

	void Kick(SOCKET socket);

	//////////////////////////////////////////////////////////////////////
	// State
	//////////////////////////////////////////////////////////////////////

	enum class ServerState
	{
		Stopped,
		Listening
	};

	ServerState state = ServerState::Stopped;

	SOCKET listenSocket;

	struct ConnectedSocket
	{
		sockaddr_in address;
		SOCKET socket;
		std::string playerName;
	};

	std::vector<ConnectedSocket> connectedSockets;
	std::map<std::string, std::string> blackList;
};

