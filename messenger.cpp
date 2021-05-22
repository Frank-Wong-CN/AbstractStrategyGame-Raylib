#include "messenger.hpp"

using namespace Messenger;

Message::Message() : Message("undefined", nullptr) {}

Message::Message(string id, void *sender, RECEIVE_MODE mode, bool sequential)
{
	SetMessageID(id);
	SetSender(sender);
	SetReceiveMode(mode);
	SetSequential(sequential);
	SetDispatcher(nullptr);
	receivers = new vector<string>();
	refCount = 0;
}

Message::~Message()
{
	//printf("Message %s recvSize = %d\n", messageID.c_str(), GetReceiverCount());
	//printf("Message %s refCount depleted!\n", messageID.c_str());
	delete receivers;
}

void Message::incRef()
{
	refCount += 1;
	//printf("Message %s refCount = %d\n", messageID.c_str(), refCount);
}

void Message::decRef()
{
	refCount -= 1;
	//printf("Message %s refCount = %d\n", messageID.c_str(), refCount);

	if (refCount <= 0)
		delete this;
}

string Message::GetMessageID()
{
	return messageID;
}

void *Message::GetSender()
{
	return sender;
}

RECEIVE_MODE Message::GetReceiveMode()
{
	return receiveMode;
}

bool Message::IsSequential()
{
	return sequential;
}

Message *Message::SetMessageID(string id)
{
	messageID = id;
	return this;
}

Message *Message::SetSender(void *sender)
{
	this->sender = sender;
	return this;
}

Message *Message::SetReceiveMode(RECEIVE_MODE mode)
{
	receiveMode = mode;
	return this;
}

Message *Message::SetSequential(bool sequential)
{
	this->sequential = sequential;
	return this;
}

Message *Message::AddReceiver(string recv)
{
	if (!HasReceiver(recv))
		receivers->push_back(recv);
	return this;
}

Message *Message::AddReceivers(string recvs[], int length)
{
	if (recvs == nullptr || recvs == NULL || length <= 0)
		return this;

	for (int i = 0; i < length; i++)
		if (!HasReceiver(recvs[i]))
			receivers->push_back(recvs[i]);
	return this;
}

Message *Message::AddReceivers(vector<string> *recvs)
{
	if (recvs == nullptr || recvs == NULL)
		return this;
	
	for (auto it : *recvs)
		receivers->push_back(it);
	return this;
}

Message *Message::RemoveReceiver(string recv)
{
	receivers->erase(remove(receivers->begin(), receivers->end(), recv), receivers->end());
	return this;
}

Message *Message::RemoveReceivers(vector<string> *recvs)
{
	if (recvs == nullptr || recvs == NULL)
		return this;
	
	receivers->erase(remove_if(receivers->begin(), receivers->end(), [recvs](string recv) {
		vector<string>::iterator r = find(recvs->begin(), recvs->end(), recv);
		return r != recvs->end() && !recv.compare(*(r));
	}), receivers->end());
	return this;
}

Message *Message::RemoveAllReceivers()
{
	receivers->clear();
	return this;
}

bool Message::HasReceiver(string recv)
{
	return find(receivers->begin(), receivers->end(), recv) != receivers->end();
}

vector<string>::iterator Message::GetReceiverIterator(bool end)
{
	if (end)
		return receivers->end();
	else return receivers->begin();
}

int Message::GetReceiverCount()
{
	return receivers->size();
}

Message *Message::SetDispatcher(MessageDispatcher *disp)
{
	sequentialDispatcher = disp;
	return this;
}

Message *Message::PassOn()
{
	if (sequential && sequentialDispatcher != nullptr)
		sequentialDispatcher->Acquire(this);
	return this;
}

MessageQueue::MessageQueue()
{
	queue = new vector<Message *>();
}

MessageQueue::~MessageQueue()
{
	while (!queue->empty())
	{
		auto tmp = queue->back();
		queue->pop_back();
		tmp->decRef();
	}
	delete queue;
}

void MessageQueue::Enqueue(Message *msg)
{
	if (msg != nullptr && msg != NULL)
	{
		queue->push_back(msg);
		msg->incRef();
	}
}

Message *MessageQueue::Dequeue()
{
	Message *msg = nullptr;
	if (queue->size() > 0)
	{
		msg = *(queue->begin());
		queue->erase(queue->begin());
		// msg->decRef();
	}
	// msg->incRef();
	return msg;
}

int MessageQueue::Size()
{
	return queue->size();
}

void MessageQueue::Clear()
{
	while (!queue->empty())
	{
		auto tmp = queue->back();
		queue->pop_back();
		tmp->decRef();
	}
}

void MessageDispatcher::DispatchExecution(IMessageReceiver *recv, Message *msg)
{
	msg->incRef();
	recv->onReceive(msg);
	msg->decRef();
}

MessageDispatcher::MessageDispatcher()
{
	msgQueue = new MessageQueue();
	registeredReceivers = new DispatcherRegistration();
}

MessageDispatcher::~MessageDispatcher()
{
	delete msgQueue;
	delete registeredReceivers;
}

void MessageDispatcher::Acquire(Message *msg)
{
	if (msg == nullptr)
		return;

	msgQueue->Enqueue(msg);
}

void MessageDispatcher::DispatchNext()
{
	if (GetQueueSize() <= 0)
		return;

	Message *msg = msgQueue->Dequeue();
	if (msg->IsSequential())
		msg->SetDispatcher(this);
	vector<string> *toRemove = new vector<string>();

	switch (msg->GetReceiveMode())
	{
	case AT_LEAST_ONE:
	{
		bool received = false;
		for (auto it = msg->GetReceiverIterator(); it != msg->GetReceiverIterator(true); it++)
		{
			string receiver = *it;
			if (IsRegistered(receiver))
			{
				IMessageReceiver *target = registeredReceivers->at(receiver);
				DispatchExecution(target, msg);
				received = true;
				toRemove->push_back(receiver);
				if (msg->IsSequential())
				{
					msg->SetReceiveMode(NO_NEED_AT_ALL);
					break; // This break breaks out of the for loop, not the switch case!
				}
			}
		}
		msg->RemoveReceivers(toRemove);
		toRemove->clear();
		if (!received)
			Acquire(msg);
		msg->decRef();
		break;
	}
	case ALL_REQUIRED:
	{
		for (auto it = msg->GetReceiverIterator(); it != msg->GetReceiverIterator(true); it++)
		{
			string receiver = *it;
			if (IsRegistered(receiver))
			{
				IMessageReceiver *target = registeredReceivers->at(receiver);
				DispatchExecution(target, msg);
				toRemove->push_back(receiver);
				if (msg->IsSequential())
					break;
			}
		}
		msg->RemoveReceivers(toRemove);
		toRemove->clear();
		if (msg->GetReceiverCount() > 0 && !msg->IsSequential())
			Acquire(msg);
		msg->decRef();
		break;
	}
	case NO_NEED_AT_ALL:
	{
		for (auto it = msg->GetReceiverIterator(); it != msg->GetReceiverIterator(true); it++)
		{
			string receiver = *it;
			if (IsRegistered(receiver))
			{
				IMessageReceiver *target = registeredReceivers->at(receiver);
				DispatchExecution(target, msg);
				toRemove->push_back(receiver);
				if (msg->IsSequential())
					break;
			}
			else
				toRemove->push_back(receiver);
		}
		msg->RemoveReceivers(toRemove);
		toRemove->clear();
		msg->decRef();
		break;
	}
	case ALL_PRESENCE:
	{
		bool isEverybodyHere = true;
		for (auto it = msg->GetReceiverIterator(); it != msg->GetReceiverIterator(true); it++)
		{
			string receiver = *it;
			if (!IsRegistered(receiver))
			{
				isEverybodyHere = false;
				break;
			}
		}
		if (!isEverybodyHere)
			Acquire(msg);
		else
		{
			if (msg->IsSequential())
			{
				auto it = msg->GetReceiverIterator();
				// while (!IsRegistered(*it) && ++it != msg->GetReceiverIterator(true)); // Just in case
				if (it != msg->GetReceiverIterator(true))
					DispatchExecution(registeredReceivers->at(*it), msg);
				else break;
				msg->RemoveReceiver(*it);
			}
			else
			{
				for (auto it = msg->GetReceiverIterator(); it != msg->GetReceiverIterator(true); it++)
					if (IsRegistered(*it))
					{
						DispatchExecution(registeredReceivers->at(*it), msg);
						toRemove->push_back(*it);
					}
				msg->RemoveReceivers(toRemove);
				toRemove->clear();
			}
		}
		msg->decRef();
		break;
	}
	case ALL_PRESENCE_SPAM:
	{
		if (!msg->IsSequential())
		{
			bool reacquire = false;
			for (auto it = msg->GetReceiverIterator(); it != msg->GetReceiverIterator(true); it++)
			{
				string receiver = *it;
				if (IsRegistered(receiver))
					DispatchExecution(registeredReceivers->at(receiver), msg);
				else
					reacquire = true;
			}
			if (reacquire)
				Acquire(msg);
		}
		else
		{
			if (msg->GetReceiverCount() > 0)
			{
				auto it = msg->GetReceiverIterator();
				if (it == msg->GetReceiverIterator(true) || !IsRegistered(*it))
					Acquire(msg);
				else
				{
					IMessageReceiver *target = registeredReceivers->at(*it);
					DispatchExecution(target, msg);
					msg->RemoveReceiver(*it);
				}
			}
		}
		msg->decRef();
		break;
	}
	default:
		break;
	}

	delete toRemove;
}

void MessageDispatcher::Register(string id, IMessageReceiver *receiver)
{
	if (id.empty() || receiver == nullptr)
		return;
	
	if (registeredReceivers->find(id) == registeredReceivers->end())
		registeredReceivers->insert({id, receiver});
}

void MessageDispatcher::Deregister(string id)
{
	if (id.empty())
		return;
	
	registeredReceivers->erase(id);
}

bool MessageDispatcher::IsRegistered(string id)
{
	if (id.empty())
		return false;
	
	return registeredReceivers->find(id) != registeredReceivers->end();
}

void MessageDispatcher::ClearQueue()
{
	msgQueue->Clear();
}

void MessageDispatcher::Reset()
{
	msgQueue->Clear();
	registeredReceivers->clear();
}

int MessageDispatcher::GetQueueSize()
{
	return msgQueue->Size();
}

MessageBroadcaster::MessageBroadcaster()
{
	msgQueue = new MessageQueue();
	registeredReceivers = new BroadcasterRegistration();
}

MessageBroadcaster::~MessageBroadcaster()
{
	delete msgQueue;
	while (!registeredReceivers->empty())
	{
		auto it = registeredReceivers->begin();
		delete it->second;
		registeredReceivers->erase(it);
	}
	delete registeredReceivers;
}

void MessageBroadcaster::BroadcastExecution(IMessageReceiver *recv, Message *msg)
{
	msg->incRef();
	recv->onReceive(msg);
	msg->decRef();
}

void MessageBroadcaster::Acquire(Message *msg)
{
	if (msg == nullptr)
		return;

	msgQueue->Enqueue(msg);
}

void MessageBroadcaster::DispatchNext()
{
	if (msgQueue->Size() <= 0)
		return;
	
	Message *msg = msgQueue->Dequeue();
	if (IsRegistered(msg->GetMessageID()))
	{
		vector<IMessageReceiver *> *receivers = registeredReceivers->at(msg->GetMessageID());
		for (IMessageReceiver *recv : *receivers)
			BroadcastExecution(recv, msg);
	}
	else if (msg->GetReceiveMode() == AT_LEAST_ONE)
		Acquire(msg);
	msg->decRef();
}

void MessageBroadcaster::Register(string id, IMessageReceiver *recv)
{
	vector<IMessageReceiver *> *tmp;
	auto it = registeredReceivers->find(id);
	if (it != registeredReceivers->end())
		tmp = it->second;
	else
	{
		tmp = new vector<IMessageReceiver *>();
		registeredReceivers->insert({id, tmp});
	}

	if (find(tmp->begin(), tmp->end(), recv) == tmp->end())
		tmp->push_back(recv);
}

void MessageBroadcaster::Deregister(string id, IMessageReceiver *recv)
{
	if (IsRegistered(id))
	{
		auto tmp = registeredReceivers->at(id);
		auto it = find(tmp->begin(), tmp->end(), recv);
		if (it != tmp->end())
			tmp->erase(it);
	}
}

bool MessageBroadcaster::IsRegistered(string id)
{
	if (id.empty())
		return false;
	
	return registeredReceivers->find(id) != registeredReceivers->end();
}

void MessageBroadcaster::ClearQueue()
{
	msgQueue->Clear();
}

void MessageBroadcaster::Reset()
{
	msgQueue->Clear();
	while (!registeredReceivers->empty())
	{
		auto it = registeredReceivers->begin();
		delete it->second;
		registeredReceivers->erase(it);
	}
}
