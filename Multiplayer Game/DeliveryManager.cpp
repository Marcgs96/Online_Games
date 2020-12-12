#include "Networks.h"
#include "DeliveryManager.h"

// TODO(you): Reliability on top of UDP lab session

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet.Write(nextSequenceNumber);

	Delivery* newDelivery = new Delivery();
	newDelivery->sequenceNumber = nextSequenceNumber++;
	newDelivery->dispatchTime = Time.time;
	pendingDeliveries.push_back(newDelivery);

	return newDelivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 sequenceNumber = 0;
	packet.Read(sequenceNumber);

	bool order = sequenceNumber >= nextExpectedSequenceNumber;

	if (order)
	{
		pendingAckSequenceNumbers.push_back(sequenceNumber);
		nextExpectedSequenceNumber = sequenceNumber + 1;
	}

	return order;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	for (auto sequenceNumber : pendingAckSequenceNumbers)
	{
		packet.Write(sequenceNumber);
	}

	pendingAckSequenceNumbers.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
	while ((int)packet.RemainingByteCount() > 0)
	{
		uint32 sequenceNumber = 0;
		packet.Read(sequenceNumber);

		for (auto it = pendingDeliveries.begin(); it != pendingDeliveries.end();)
		{
			if ((*it)->sequenceNumber == sequenceNumber)
			{
				(*it)->delegate->onDeliverySuccess(this);
				delete (*it)->delegate;
				delete* it;
				it = pendingDeliveries.erase(it);
			}
			else
				++it;
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
	for (auto it = pendingDeliveries.begin(); it != pendingDeliveries.end();)
	{
		if (Time.time - (*it)->dispatchTime >= PACKET_DELIVERY_TIMEOUT_SECONDS)
		{
			(*it)->delegate->onDeliveryFailure(this);
			delete (*it)->delegate;
			delete* it;
			it = pendingDeliveries.erase(it);
		}
		else
			++it;
	}
}

ReplicationDeliveryDelegate::ReplicationDeliveryDelegate(ReplicationManagerServer* repManager) :replicationManager(repManager)
{
	for (std::unordered_map<uint32, ReplicationCommand>::iterator it = replicationManager->commands.begin(); it != replicationManager->commands.end(); ++it)
	{
		commands.push_back((*it).second);
	}
}

void ReplicationDeliveryDelegate::repeatReplication()
{
	for (std::vector<ReplicationCommand>::iterator it = commands.begin(); it != commands.end(); ++it)
	{
		switch ((*it).action)
		{
		case ReplicationAction::Create:
			replicationManager->create((*it).networkId);
			break;
		case ReplicationAction::Update:
			replicationManager->update((*it).networkId);
			break;
		case ReplicationAction::Destroy:
			replicationManager->destroy((*it).networkId);
			break;
		default:
			break;
		}
	}
}
