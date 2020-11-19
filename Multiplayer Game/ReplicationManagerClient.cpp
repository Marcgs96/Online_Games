#include "Networks.h"
#include "ReplicationManagerClient.h"

// TODO(you): World state replication lab session

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while ((int)packet.RemainingByteCount() > 0)
	{
		uint32 networkId = 0;
		packet.Read(networkId);

		ReplicationAction action = ReplicationAction::None;
		packet.Read(action);

		switch (action)
		{
		case ReplicationAction::None:
		{

		}
		break;
		case ReplicationAction::Create:
		{
			GameObject* gameObject = App->modGameObject->Instantiate();
			BehaviourType bType = BehaviourType::None;
			packet.Read(bType);
			gameObject->behaviour = 

			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
			gameObject->behaviour->read(packet);
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			gameObject->behaviour->read(packet);
		}
		break;
		case ReplicationAction::Destroy:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			App->modLinkingContext->unregisterNetworkGameObject(gameObject);
			App->modGameObject->Destroy(gameObject);
		}
		break;
		default:
			break;
		}
	}
}
