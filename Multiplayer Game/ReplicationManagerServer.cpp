#include "Networks.h"
#include "ReplicationManagerServer.h"

// TODO(you): World state replication lab session

void ReplicationManagerServer::create(uint32 networkId)
{
	if (networkId == 0)
		return;
	/*ReplicationCommand command;
	command.action = ReplicationAction::Create;
	command.networkId = networkId;

	commands.emplace(networkId, command);*/
	commands[networkId].action = ReplicationAction::Create;
	commands[networkId].networkId = networkId;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	if (networkId == 0)
		return;

	if (commands[networkId].action == ReplicationAction::Create || commands[networkId].action == ReplicationAction::Destroy)
		return;

	commands[networkId].action = ReplicationAction::Update;
	commands[networkId].networkId = networkId;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	if (networkId == 0)
		return;

	if (commands[networkId].action == ReplicationAction::Create) //If client hasn't created the object but it has to be destroyed already ignore this object
		commands[networkId].action == ReplicationAction::None;

	commands[networkId].action = ReplicationAction::Destroy;
	commands[networkId].networkId = networkId;
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
			else //This is an old packet with create for a object that has already been deleted on the server so we will create for a dummy
			{
				GameObject* dummy = Instantiate();
				dummy->writeCreate(packet);
				Destroy(dummy);
			}
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(it->second.networkId);
			if(gameObject)
				gameObject->writeUpdate(packet);
			else //This is an old packet with update for a object that has already been deleted on the server so we will write update for a dummy
			{
				GameObject* dummy = Instantiate();
				dummy->writeUpdate(packet);
				Destroy(dummy);
			}
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
