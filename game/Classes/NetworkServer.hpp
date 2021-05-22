#pragma once

networkOpServer = new ThreadFunction([this](){
	while (conn != ConnectionTerminated)
	{
		switch (conn)
		{
		case NoConnection:
			peer = SLNet::RakPeerInterface::GetInstance();
			peer->SetIncomingPassword(NET_PASSWD, strlen(NET_PASSWD) + 1);
			peer->SetTimeoutTime(10000, SLNet::UNASSIGNED_SYSTEM_ADDRESS);
			if (peer->Startup(1, new SLNet::SocketDescriptor(port, 0), 1) == SLNet::RAKNET_STARTED)
			{
				peer->SetMaximumIncomingConnections(1);
				accessLock.lock();
				conn = ServerListening;
				gameResult = 20;
			}
			else
			{
				accessLock.lock();
				conn = ConnectionTerminated;
				gameResult = 21;
			}
			break;
		case ServerListening:
			accessLock.lock();
			if (peer->GetConnectionState(peer->GetGUIDFromIndex(0)) == SLNet::IS_CONNECTED)
			{
				conn = ServerConnected;
				nGameState = InitGame;
			}
			break;
		case ServerConnected:
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
			ASync(true); // ASync deallocates the packet
			break;
		case StopConnection:
			packet = peer->Receive();
			if (packet)
				peer->DeallocatePacket(packet);
			packet = nullptr;
			nGameState = NoNetwork;
			peer->Shutdown(1000);
			accessLock.lock();
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
	DEBUGS("Server thread terminated.\n");
});