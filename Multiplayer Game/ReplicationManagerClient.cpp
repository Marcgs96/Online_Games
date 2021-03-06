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

			if (App->modLinkingContext->getNetworkGameObject(networkId) != nullptr)
			{
				gameObject->readCreate(packet);
				App->modGameObject->Destroy(gameObject);
			}
			else
			{
				App->modLinkingContext->registerNetworkGameObjectWithNetworkId(gameObject, networkId);
				gameObject->readCreate(packet);

				if (networkId == App->modNetClient->GetNetworkId())
				{
					gameObject->networkInterpolationEnabled = false;
					gameObject->behaviour->OnInterpolationDisable();
				}				
			}
		}
		break;
		case ReplicationAction::Update:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if(gameObject)
				gameObject->readUpdate(packet);
			else
			{
				gameObject = App->modGameObject->Instantiate();
				gameObject->readUpdate(packet);
				Destroy(gameObject);
			}
		}
		break;
		case ReplicationAction::Destroy:
		{
			GameObject* gameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (gameObject)
			{
				App->modLinkingContext->unregisterNetworkGameObject(gameObject);
				App->modGameObject->Destroy(gameObject);
			}				
		}
		break;
		default:
			break;
		}
	}
}
