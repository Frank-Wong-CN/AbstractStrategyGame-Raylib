#pragma once

networkOpClient = new ThreadFunction([this](){
	while (conn != ConnectionTerminated)
	{
		switch (conn)
		{
		case NoConnection:
			peer = SLNet::RakPeerInterface::GetInstance();
			if (peer->Startup(1, new SLNet::SocketDescriptor(0, 0), 1) == SLNet::RAKNET_STARTED)
			{
				accessLock.lock();
				conn = ClientStartup;
				gameResult = 23;
			}
			else
			{
				accessLock.lock();
				conn = ConnectionTerminated;
				gameResult = 21;
			}
			break;
		case ClientStartup:
			if (peer->Connect(networkIP, port, NET_PASSWD, strlen(NET_PASSWD) + 1) == SLNet::CONNECTION_ATTEMPT_STARTED)
			{
				accessLock.lock();
				conn = ClientConnecting;
				gameResult = 23;
			}
			else
			{
				accessLock.lock();
				conn = ConnectionTerminated;
				gameResult = 21;
			}
			break;
		case ClientConnecting:
			packet = peer->Receive();
			accessLock.lock();
			if (packet)
			{
				switch (packet->data[0])
				{
				case ID_CONNECTION_REQUEST_ACCEPTED:
					conn = ClientConnected;
					nGameState = InitGame;
					gameResult = 22;
					break;
				case ID_CONNECTION_ATTEMPT_FAILED:
				case ID_NO_FREE_INCOMING_CONNECTIONS:
				case ID_CONNECTION_LOST:
				case ID_DISCONNECTION_NOTIFICATION:
					conn = ConnectionTerminated;
					gameResult = 21;
					break;
				default:
					DEBUGS("Unknown packet received during client connecting...\n");
					break;
				}
			}
			break;
		case ClientConnected:
			if (!packet) packet = peer->Receive();
			if (peer->GetConnectionState(peer->GetGUIDFromIndex(0)) == SLNet::IS_DISCONNECTED)
			{
				if (packet)
					peer->DeallocatePacket(packet);
				packet = nullptr;
				accessLock.lock();
				conn = StopConnection;
				gameResult = 3;
				break;
			}
			accessLock.lock();
			ASync(false);
			break;
		case StopConnection:
			packet = peer->Receive();
			if (packet)
				peer->DeallocatePacket(packet);
			packet = nullptr;
			peer->Shutdown(1000);
			accessLock.lock();
			nGameState = NoNetwork;
			conn = ConnectionTerminated;
			break;
		case ConnectionTerminated:
		default:
			accessLock.lock();
			gameResult = 24;
		}
		accessLock.unlock();
		std::this_thread::sleep_for(std::chrono::milliseconds{30});
	}
	DEBUGS("Client thread terminated.\n");
});