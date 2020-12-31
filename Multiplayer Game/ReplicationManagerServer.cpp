#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	/*ReplicationCommand command;
	command.action = ReplicationAction::Create;
	command.networkId = networkId;

	commands.emplace(networkId, command);*/
	commands[networkId].action = ReplicationAction::Create;
	commands[networkId].networkId = networkId;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	if (commands[networkId].action == ReplicationAction::Create || commands[networkId].action == ReplicationAction::Destroy)
		return;

	commands[networkId].action = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	if (commands[networkId].action == ReplicationAction::Create) //If client hasn't created the object but it has to be destroyed already ignore this object
		commands[networkId].action == ReplicationAction::None;

	commands[networkId].action = ReplicationAction::Destroy;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet)
{
	std::vector<decltype(commands)::key_type> vec;

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
			if(gameObject)
				gameObject->writeCreate(packet);
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(it->second.networkId);
			if(gameObject)
				gameObject->writeUpdate(packet);
		}
		break;
		case ReplicationAction::Destroy:
		{
			vec.emplace_back(it->first);
		}
		break;
		default:
			break;
		}

		//This is to clear the action
		it->second.action = ReplicationAction::None;
	}
	for (auto&& key : vec)
		commands.erase(key);
}
