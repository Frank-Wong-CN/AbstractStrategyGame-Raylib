#pragma once

//#ifdef USE_WX
//#include <wx/string.h>
//#include <wx/vector.h>
//#include <wx/hashmap.h>
//typedef wxString string;
//typedef wxVector vector;
//#else
#include <string>
#include <vector>
#include <map>
using std::string;
using std::vector;
using std::map;
//#endif

#include <algorithm>
using std::remove;
using std::remove_if;
using std::find;

namespace Messenger
{
	enum RECEIVE_MODE
	{
		AT_LEAST_ONE,
		ALL_REQUIRED,
		ALL_PRESENCE,
		ALL_PRESENCE_SPAM,
		NO_NEED_AT_ALL
	};

	class MessageDispatcher;

	class Message
	{
		/**
		 * If the message is not sequential:
		 * AT_LEAST_ONE ensures the message will be received at least once.
		 * ALL_REQUIRED ensures every receiver will receive the message.
		 * ALL_PRESENCE ensures that when the message is received by any receiver, the other receivers will be present.
		 * ALL_PRESENCE_SPAM ensures that when the message is received by any receiver, the other receivers will be present, but until then, the message will be constantly received by the receivers that already present.
		 * NO_NEED_AT_ALL does not ensure anything. The message will be sent, but receiving is not guaranteed.
		 *
		 * If the message is sequential:
		 * AT_LEAST_ONE ensures the message will be received at least once.
		 * ALL_REQUIRED ensures every receiver will receive the message, but does not ensure the order of them receiving the message.
		 * ALL_PRESENCE ensures that when the message is received by any receiver, the other receivers will be present.
		 * ALL_PRESENCE_SPAM ensures every receiver will receive the message with given order.
		 * NO_NEED_AT_ALL does not ensure anything. The message will be sent, but receiving is not guaranteed.
		 */

	private:
		string messageID;
		void *sender;
		vector<string> *receivers;
		RECEIVE_MODE receiveMode;
		bool sequential;
		MessageDispatcher *sequentialDispatcher;
		int refCount;

	public:
		Message();
		Message(string id, void *sender = nullptr, RECEIVE_MODE mode = NO_NEED_AT_ALL, bool sequential = false);
		~Message();

		void incRef();
		void decRef();

		string GetMessageID();
		void *GetSender();
		RECEIVE_MODE GetReceiveMode();
		bool IsSequential();
		Message *SetMessageID(string messageID);
		Message *SetSender(void *sender);
		Message *SetReceiveMode(RECEIVE_MODE mode);
		Message *SetSequential(bool sequential);

		Message *AddReceiver(string recv);
		Message *AddReceivers(string recvs[], int length);
		Message *AddReceivers(vector<string> *recvs);
		Message *RemoveReceiver(string recv);
		Message *RemoveReceivers(vector<string> *recvs);
		Message *RemoveAllReceivers();
		bool HasReceiver(string recv);
		vector<string>::iterator GetReceiverIterator(bool end = false);
		int GetReceiverCount();
		Message *SetDispatcher(MessageDispatcher *disp);
		Message *PassOn();
	};

	class IMessageReceiver
	{
	public:
		virtual void onReceive(Message *message) = 0;
	};

	class MessageQueue
	{
	private:
		vector<Message *> *queue;
	public:
		MessageQueue();
		~MessageQueue();
		void Enqueue(Message *msg);
		Message *Dequeue();
		int Size();
		void Clear();
	};

	// WX_DECLARE_STRING_HASH_MAP(IMessageReceiver *, DispatcherRegistration);
	// WX_DECLARE_STRING_HASH_MAP(vector<IMessageReceiver *>, BroadcasterRegistration);

	typedef map<string, IMessageReceiver *> DispatcherRegistration;
	typedef map<string, vector<IMessageReceiver *>*> BroadcasterRegistration;

	// Message Dispatcher and Broadcaster will be responsible for the deletion of the processed Messages.
	class MessageDispatcher
	{
	private:
		MessageQueue *msgQueue;
		DispatcherRegistration *registeredReceivers;
		void DispatchExecution(IMessageReceiver *recv, Message *msg);
	public:
		MessageDispatcher();
		~MessageDispatcher();
		void Acquire(Message *msg);
		void DispatchNext();
		void Register(string id, IMessageReceiver *recv);
		void Deregister(string id);
		bool IsRegistered(string id);
		void ClearQueue();
		void Reset();
		int GetQueueSize();
	};

	class MessageBroadcaster
	{
	private:
		MessageQueue *msgQueue;
		BroadcasterRegistration *registeredReceivers;
		void BroadcastExecution(IMessageReceiver *recv, Message *msg);
	public:
		MessageBroadcaster();
		~MessageBroadcaster();
		void Acquire(Message *msg);
		void DispatchNext();
		void Register(string id, IMessageReceiver *recv);
		void Deregister(string id, IMessageReceiver *recv);
		bool IsRegistered(string id);
		void ClearQueue();
		void Reset();
	};
}