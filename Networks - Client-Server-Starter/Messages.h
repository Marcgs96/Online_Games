#pragma once

// Add as many messages as you need depending on the
// functionalities that you decide to implement.

enum class ClientMessage
{
	Hello,
	ChatText,
	UserList,
	Kick,
	Whisper,
	ChangeName,
	Ban
};

enum class ServerMessage
{
	Welcome,
	NonWelcome,
	ChatText,
	UserList,
	ChangeName
};

