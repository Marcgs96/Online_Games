#pragma once

#include "ModuleNetworking.h"
#include <list>
#include <map>

class ModuleNetworkingClient : public ModuleNetworking
{
public:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient public methods
	//////////////////////////////////////////////////////////////////////

	bool start(const char *serverAddress, int serverPort, const char *playerName);

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

	void onSocketReceivedData(SOCKET socket, const InputMemoryStream& packet) override;

	void onSocketDisconnected(SOCKET socket) override;

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworkingClient private methods
	//////////////////////////////////////////////////////////////////////

	void HandleCommands(std::vector<std::string> splitString);

	//////////////////////////////////////////////////////////////////////
	// Client state
	//////////////////////////////////////////////////////////////////////

	enum class ClientState
	{
		Stopped,
		Start,
		Logging,
		LoggedIn,
	};

	enum class CommandType {
		Help,
		List,
		Kick,
		Whisper,
		ChangeName,
		Ban,
		Unban
	};

	ClientState state = ClientState::Stopped;

	sockaddr_in serverAddress = {};
	SOCKET connectSocket = INVALID_SOCKET;

	std::string playerName;
	std::list<std::string> receivedMessages;

	std::map<std::string, CommandType> commands;
	std::string helpMessage;
	bool timedOut = false;
	bool scrollDown = false;
};

