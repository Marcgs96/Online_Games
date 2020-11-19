#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	//commands.emplace(networkId, ReplicationAction::Create);
	commands[networkId].action = ReplicationAction::Create;
	commands[networkId].networkId = networkId;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	commands[networkId].action = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	commands[networkId].action = ReplicationAction::Destroy;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	packet.Write(ServerMessage::Replication);
	for (auto it = commands.begin(); it != commands.end(); ++it)
	{
		packet.Write(it->second.networkId);
		packet.Write(it->second.action);

		switch (it->second.action)
		{
		case ReplicationAction::None:
		{

		}
		break;
		case ReplicationAction::Create:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(it->second.networkId);
			gameObject->behaviour->write(packet);
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(it->second.networkId);
			gameObject->behaviour->write(packet);
		}
		break;
		case ReplicationAction::Destroy:
		{

		}
		break;
		default:
			break;
		}

		//This is to clear the action
		it->second.action = ReplicationAction::None;
	}
}
