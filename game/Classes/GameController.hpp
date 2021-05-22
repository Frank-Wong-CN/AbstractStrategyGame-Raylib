#pragma once

void GameController::SetupPlayer(DecisionMaker &player)
{
	player.credits = 100;
	player.creditRestoreTimeBase = STARTING_CTIMER;
	player.creditRestoreTimeModifier = 1.0f;

	player.hpCur = STARTING_HP / 5;
	player.hpMax = STARTING_HP;
	player.matterCur = 0.0f;
	player.matterMax = 0.0f;
	player.energyCur = 0.0f;
	player.energyMax = std::numeric_limits<float>::max();

	player.damageModifier = 1.0f; // Damage means damage taken from foe, not player's attack damage
	player.attackModifier = 1.0f;
	player.defenseModifier = 1.0f;

	player.recov[0] = 100.0f; // Feature: reprogram recovery into a "potion queue"
	player.recov[1] = 0.0f;
	player.recov[2] = 0.0f;
	player.recovSpeed[0] = 0.15f;
	player.recovSpeed[1] = 0.15f;
	player.recovSpeed[2] = 0.15f;
	player.recovRetainSurplus[0] = false;
	player.recovRetainSurplus[1] = false;
	player.recovRetainSurplus[2] = false;

	player.autoHeal = 0.0025f;
	
	player.levelKnowledge = 1;
	player.levelDeception = 1;
}

void GameController::RecoverValue(float &val, float valMax, float &remaining, float speed, bool retainSurplus)
{
	if (remaining > 0.0001f)
	{
		float toRecov = std::min(remaining, speed);
		float reqRecov = std::min(valMax - val, toRecov);
		val += reqRecov;
		if (retainSurplus)
			remaining -= reqRecov;
		else remaining -= toRecov;
	}
	else remaining = 0.0f;
}

void GameController::DoRecover(DecisionMaker &player)
{
	if (player.hpCur < player.hpMax)
		player.hpCur += std::min(player.hpMax - player.hpCur, player.hpMax * player.autoHeal * GetFrameTime());
	
	RecoverValue(player.hpCur, player.hpMax, player.recov[0], player.recovSpeed[0], player.recovRetainSurplus[0]);
	RecoverValue(player.matterCur, player.matterMax, player.recov[1], player.recovSpeed[1], player.recovRetainSurplus[1]);
	RecoverValue(player.energyCur, player.energyMax, player.recov[2], player.recovSpeed[2], player.recovRetainSurplus[2]);
}

void GameController::AI()
{
	/* TODO: AI opponent */
}

void GameController::Sync()
{
	/* TODO: network opponent */
}

void GameController::CopyStaging(DecisionMakerStaging &staging, const DecisionMaker &copy, bool force)
{
	memcpy((void *) &stagingSend + (int)sizeof(stagingSend) - VERIFY_PADDING_SIZE, VERIFY_PADDING, VERIFY_PADDING_SIZE);
	if (!staging.relative && !force)
	{
		staging.hpCur = copy.hpCur;
		staging.hpMax = copy.hpMax;
		staging.mtCur = copy.matterCur;
		staging.mtMax = copy.matterMax;
		staging.lvK = copy.levelKnowledge;
		staging.lvD = copy.levelDeception;
		staging.relative = false;
	}
}

void GameController::PasteStaging(DecisionMakerStaging &staging, DecisionMaker &paste, bool force)
{
	if (!staging.relative && !force)
	{
		paste.hpCur = staging.hpCur;
		paste.hpMax = staging.hpMax;
		paste.matterCur = staging.mtCur;
		paste.matterMax = staging.mtMax;
		paste.levelKnowledge = staging.lvK;
		paste.levelDeception = staging.lvD;
	}
	else
	{
		paste.hpCur = std::max(-1.0f, paste.hpCur + staging.hpCur);
		paste.hpMax = std::max(0.0f, paste.hpMax + staging.hpMax);
		paste.matterCur = std::max(0.0f, paste.matterCur + staging.mtCur);
		paste.matterMax = std::max(0.0f, paste.matterMax + staging.mtMax);
		paste.levelKnowledge = std::max(1.0f, (float) paste.levelKnowledge + staging.lvK);
		paste.levelDeception = std::max(1.0f, (float) paste.levelDeception + staging.lvD);
		staging.relative = false;
	}
}

bool GameController::VerifyStaging(const DecisionMakerStaging &staging)
{
	return !memcmp((void *) &staging + (int)sizeof(staging) - VERIFY_PADDING_SIZE, VERIFY_PADDING, VERIFY_PADDING_SIZE);
}

void GameController::ASync(bool isServer)
{
	SLNet::BitStream bsOut;
	SLNet::SystemAddress addr;
	unsigned short sizeOfAddr = 1;
	peer->GetConnectionList(&addr, &sizeOfAddr);
	if (sizeOfAddr != 1)
	{
		conn = StopConnection;
		return;
	}
	
	switch (nGameState)
	{
	case NoNetwork:
	default:
		break;
	case InitGame:
		InitSession();
		stagingSend = {};
		CopyStaging(stagingSend, player, true);
		stagingSend.mID = StagingInitSend;
		bsOut.Write<DecisionMakerStaging>(stagingSend);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
		nGameState = StagingSent;
		stagingSend = {};
	case StagingSent:
		if (packet)
		{
			SLNet::BitStream bsIn(packet->data, packet->length, false);
			if (bsIn.DoEndianSwap()) SLNet::BitStream::ReverseBytesInPlace(packet->data, packet->length);
			memcpy(&stagingRecv, packet->data, sizeof(DecisionMakerStaging));
			peer->DeallocatePacket(packet);
			packet = nullptr;
			if (!VerifyStaging(stagingRecv))
				break;
		}
		else break;
		if (stagingRecv.mID == StagingInitSend)
		{
			PasteStaging(stagingRecv, opponent, true);
			CopyStaging(stagingSend, player);
			stagingSend.mID = StagingInitReceiveACK;
			bsOut.Write<DecisionMakerStaging>(stagingSend);
			peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
			stagingSend = {};
		}
		else if (stagingRecv.mID == StagingInitReceiveACK)
			nGameState = GamingUpdate;
		stagingRecv = {};
		break;
	case GamingUpdate:
		stagingSend.mID = StagingUpdate;
		CopyStaging(stagingSend, player);
		bsOut.Write<DecisionMakerStaging>(stagingSend);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
		stagingSend = {};
		if (packet)
		{
			SLNet::BitStream bsIn(packet->data, packet->length, false);
			if (bsIn.DoEndianSwap()) SLNet::BitStream::ReverseBytesInPlace(packet->data, packet->length);
			memcpy(&stagingRecv, packet->data, sizeof(DecisionMakerStaging));
			peer->DeallocatePacket(packet);
			packet = nullptr;
			if (!VerifyStaging(stagingRecv))
				break;
		}
		else break;
		switch (stagingRecv.mID)
		{
		case StagingEnd:
			nGameState = EndGame;
		case StagingUpdate:
			if (!stagingRecv.relative)
				PasteStaging(stagingRecv, opponent);
			else
				PasteStaging(stagingRecv, player);
			break;
		case StagingRestart:
			nGameState = InitGame;
			break;
		}
		stagingRecv = {};
		break;
	case EndGame:
		stagingSend.mID = StagingEnd;
		CopyStaging(stagingSend, player);
		bsOut.Write<DecisionMakerStaging>(stagingSend);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
		stagingSend = {};
		if (packet)
		{
			SLNet::BitStream bsIn(packet->data, packet->length, false);
			if (bsIn.DoEndianSwap()) SLNet::BitStream::ReverseBytesInPlace(packet->data, packet->length);
			memcpy(&stagingRecv, packet->data, sizeof(DecisionMakerStaging));
			peer->DeallocatePacket(packet);
			packet = nullptr;
			if (!VerifyStaging(stagingRecv))
				break;
		}
		else break;
		switch (stagingRecv.mID)
		{
		case StagingUpdate:
			if (!stagingRecv.relative)
				PasteStaging(stagingRecv, opponent);
			else
				PasteStaging(stagingRecv, player);
			break;
		case StagingRestart:
			nGameState = InitGame;
			break;
		}
		stagingRecv = {};
		break;
	case RestartGame:
		if (packet)
			peer->DeallocatePacket(packet);
		packet = nullptr;
		stagingSend.mID = StagingRestart;
		CopyStaging(stagingSend, player);
		bsOut.Write<DecisionMakerStaging>(stagingSend);
		peer->Send(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, addr, false);
		stagingSend = {};
		nGameState = InitGame;
		break;
	}
}

void GameController::SetCommandlineArguments(int argc, char *argv[])
{
	this->argc = argc;
	this->argv = argv;
}

void GameController::InitNetworkedSession()
{
	InitSession();
	
	conn = NoConnection;
	nGameState = NoNetwork;
	networkIP = nullptr;
	peer = nullptr;
	packet = nullptr;
	port = NET_PORT;
	if (argc == 1)
		return;
	
	/* TODO: Start new thread to do this */
	if (!strcmp(argv[1], "listen"))
	{
		// Server mode
		#include "NetworkServer.hpp"
		networkThr = new std::thread(*networkOpServer);
		accessLock.lock();
	}
	else
	{
		// Client mode
		networkIP = argv[1];
		#include "NetworkClient.hpp"
		networkThr = new std::thread(*networkOpClient);
		accessLock.lock();
	}
	
	accessLock.unlock();
}

void GameController::InitSession()
{
	SetupPlayer(player);
	
	if (!player.perception)
		player.perception = new DecisionTree();
	if (!player.regeneration)
		player.regeneration = new DecisionTree();
	if (!player.destruction)
		player.destruction = new DecisionTree();
	if (!player.protection)
		player.protection = new DecisionTree();
	
	player.perception->Destroy();
	player.regeneration->Destroy();
	player.destruction->Destroy();
	player.protection->Destroy();
	
	Decision *rootP = new Decision();
	rootP->InitNode(0, Perception, this);
	rootP->SetFunction();
	player.perception->AddNode(rootP);
	gm::AddInstance(rootP);
	
	Decision *rootR = new Decision();
	rootR->InitNode(0, Regeneration, this);
	rootR->SetFunction();
	player.regeneration->AddNode(rootR);
	gm::AddInstance(rootR);
	
	Decision *rootD = new Decision();
	rootD->InitNode(0, Destruction, this);
	rootD->SetFunction();
	player.destruction->AddNode(rootD);
	gm::AddInstance(rootD);
	
	Decision *rootS = new Decision();
	rootS->InitNode(0, Protection, this);
	rootS->SetFunction();
	player.protection->AddNode(rootS);
	gm::AddInstance(rootS);
	
	SetupPlayer(opponent);
	infor = Information(&player, &opponent);
	alarm[0] = player.creditRestoreTimeBase;

	Camera2D &cam = gm::GetCurrentCamera();
	cam.target = { 0 };
		
	panStart = { 0 };
	creditColor = RAYWHITE;
	creditColorLerp = 0.0f;
	player.perception->ToggleTree();
	
	gameResult = 0;
}

void GameController::InitDummy()
{
	opponent.hpCur = STARTING_HP;
	opponent.hpMax = STARTING_HP;
	opponent.matterCur = 20.0f;
	opponent.matterMax = 20.0f;
	opponent.energyCur = 0.0f;
	opponent.energyMax = std::numeric_limits<float>::max();

	opponent.damageModifier = 0.8f; // Damage means damage taken from foe, not player's attack damage
	opponent.attackModifier = 1.0f;
	opponent.defenseModifier = 1.2f;

	opponent.recov[0] = 0.0f; // Feature: reprogram recovery into a "potion queue"
	opponent.recov[1] = 0.0f;
	opponent.recov[2] = 0.0f;
	opponent.recovSpeed[0] = 0.15f;
	opponent.recovSpeed[1] = 0.15f;
	opponent.recovSpeed[2] = 0.15f;
	opponent.recovRetainSurplus[0] = false;
	opponent.recovRetainSurplus[1] = false;
	opponent.recovRetainSurplus[2] = false;

	opponent.autoHeal = 0.0025f;

	opponent.levelKnowledge = 10;
	opponent.levelDeception = 12;
}

void GameController::SwitchTree(NodeType type)
{
	Vector2 p = player.perception->ToggleTree(true, type == Perception);
	Vector2 r = player.regeneration->ToggleTree(true, type == Regeneration);
	Vector2 d = player.destruction->ToggleTree(true, type == Destruction);
	Vector2 s = player.protection->ToggleTree(true, type == Protection);
	
	switch (type)
	{
	case Perception:
		gm::GetCurrentCamera().target = p;
		break;
	case Regeneration:
		gm::GetCurrentCamera().target = r;
		break;
	case Destruction:
		gm::GetCurrentCamera().target = d;
		break;
	case Protection:
		gm::GetCurrentCamera().target = s;
		break;
	}
}

bool GameController::ReducePlayerCredit(int cost)
{
	if (player.credits < cost)
		return creditColorLerp = 1.0f, false;
	else
		player.credits -= cost;
	return true;
}

void GameController::DecisionEffect(Decision *n)
{
	n->otherAbility(&player, &opponent);
}

void GameController::Unleash(DecisionMaker &player, DecisionMaker &opponent)
{
	int playerLevel = player.hpMax / 100;
	float baseAttackDamage = player.energyCur * player.attackModifier * playerLevel;
	float matterEffectiveHP = opponent.matterCur * MATTER_EFFECTIVE_HP * opponent.defenseModifier;
	float remainingMT = (matterEffectiveHP - baseAttackDamage) / MATTER_EFFECTIVE_HP / opponent.defenseModifier;
	float residualDamage = baseAttackDamage - matterEffectiveHP;
	
	if (!IsNetworkGame())
	{
		opponent.matterCur = std::max(0.0f, remainingMT);
		if (residualDamage > 0.0f)
			opponent.hpCur -= residualDamage * opponent.damageModifier;
	}
	else
	{
		accessLock.lock();
		stagingSend.relative = true;
		stagingSend.hpCur = -(residualDamage * opponent.damageModifier);
		stagingSend.hpMax = 0.0f;
		stagingSend.mtCur = remainingMT - opponent.matterCur;
		stagingSend.mtMax = 0.0f;
		stagingSend.lvK = 0;
		stagingSend.lvD = 0;
		accessLock.unlock();
	}
	player.energyCur = 0.0f;
}

bool GameController::IsGameEnded()
{
	// 0 - game is running
	// larger than 10 - reserved for game info
	return gameResult != 0 && gameResult < 10;
}

bool GameController::IsNetworkGame()
{
	return conn != NoConnection && conn != ConnectionTerminated;
}

GameController::BEGIN_FUNC(GameController) {
	FUNC(Create)
	{
		// Init session
		player.perception = nullptr;
		player.regeneration = nullptr;
		player.destruction = nullptr;
		player.protection = nullptr;
		
		InitNetworkedSession();
		//InitDummy();
		
		// Load resources
		fs::path cwd = GetWorkingDirectory();
		fs::path fontPath = cwd / "res/font.ttf";
		if (fs::exists(fontPath))
			font = LoadFontEx(fontPath.string().c_str(), 60, 0, 128);
		else font = GetFontDefault();
	};
	
	FUNC(Destroy)
	{
		accessLock.lock();
		conn = StopConnection;
		accessLock.unlock();
		
		if (networkThr)
		{
			if (networkThr->joinable())
				networkThr->join();
			delete networkThr;
		}
		
		if (networkOpServer) delete networkOpServer;
		if (networkOpClient) delete networkOpClient;
		
		player.perception->Destroy();
		player.regeneration->Destroy();
		player.destruction->Destroy();
		player.protection->Destroy();
		delete player.perception;
		delete player.regeneration;
		delete player.destruction;
		delete player.protection;
	};

	FUNC(Step)
	{
		Camera2D &cam = gm::GetCurrentCamera();
		
		accessLock.lock();
		
		if (IsKeyPressed(KEY_F10)) /* Restart game */
		{
			if (!IsNetworkGame())
				InitSession();
			else
				nGameState = RestartGame;
		}
		
		if (IsKeyPressed(KEY_F8)) /* Force close connection and restart with AI */
		{
			conn = StopConnection;
			InitSession();
		}
		
		accessLock.unlock();
		
		
		cam.zoom += GetMouseWheelMove() * 0.25f;
		if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON))
			panStart = gm::mouse;
		else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
			cam.target += (panStart - gm::mouse);
		
		if (IsKeyPressed(KEY_Q))
			SwitchTree(Perception);
		else if (IsKeyPressed(KEY_W))
			SwitchTree(Regeneration);
		else if (IsKeyPressed(KEY_E))
			SwitchTree(Destruction);
		else if (IsKeyPressed(KEY_R))
			SwitchTree(Protection);
		
		//if (IsKeyPressed(KEY_K))
		//	infor.Update(true);
		
		accessLock.lock();
		if (!IsGameEnded())
			// Game Results larger than 10 are reserved as game info.
		{
			accessLock.unlock();
			if (IsKeyPressed(KEY_SPACE))
				Unleash(player, opponent);
			
			creditTimer = player.creditRestoreTimeBase * player.creditRestoreTimeModifier;
			if (creditColorLerp > 0.0001f)
				creditColor = ColorLerp(RAYWHITE, RED, creditColorLerp);
			else
				creditColorLerp = 0.0f;
			creditColorLerp -= 1.25f * GetFrameTime();

			DoRecover(player);
			DoRecover(opponent);
			
			infor.Update();

			/*
			if (!IsNetworkGame())
				AI();
			else
				Sync();
			*/
			
			accessLock.lock();
			if (player.hpCur < 0.0001f)
				gameResult = 2;
			
			if (opponent.hpCur < 0.0001f)
				if (!gameResult)
					gameResult = 1;
				else gameResult = 3;
				
			if (IsGameEnded())
				nGameState = EndGame;
			accessLock.unlock();
		}
		else
		{
			accessLock.unlock();
			player.credits = 0;
			opponent.credits = 0;
			alarm[0] = 0.0f;
		}
	};

	FUNC(DrawGUI)
	{
		//DrawText(TextFormat("Mouse Position: %.2f, %.2f", gm::mouse.x, gm::mouse.y), 10, 770, 20, BLACK);
		
		float infoY = 770;
		
		accessLock.lock();
		switch(gameResult)
		{
		case 0:
		default:
			break;
		case 1:
			DrawText("You Win!! Press F10 to restart.", 10, infoY, 20, BLACK);
			break;
		case 2:
			DrawText("You Lose. Press F10 to restart.", 10, infoY, 20, BLACK);
			break;
		case 3:
			DrawText("Draw. Press F10 to restart.", 10, infoY, 20, BLACK);
			break;
		case 20:
			DrawText("Server listening on all interfaces...", 10, infoY, 20, BLACK);
			break;
		case 21:
			DrawText("Failed to start server. Returning to singleplayer mode.", 10, infoY, 20, BLACK);
			break;
		case 22:
			DrawText(TextFormat("Connected to %s!", argv[1]), 10, infoY, 20, BLACK);
			break;
		case 23:
			DrawText(TextFormat("Connecting to %s...", argv[1]), 10, infoY, 20, BLACK);
			break;
		case 24:
			DrawText(TextFormat("Disconnected.", argv[1]), 10, infoY, 20, BLACK);
			break;
		}
		accessLock.unlock();
		
		
		// Upper Banner
		float radiusDial = 70.0f;
		float bannerHeight = 150.0f;
		DrawRectangleGradientV(0, 0, conf::GetScreenWidth(), bannerHeight, { 0x09, 0x09, 0x09, 255 }, { 0x3F, 0x3F, 0x3F, 255 });
		float barHeight = bannerHeight / 3.0f;
		float barWidth = conf::GetScreenWidth() / 2;
		float barFontSize = 20;
		float barFontSpacing = barFontSize / 10.0f;
		
		// Player Side: Bars
		float hpPortion = player.hpCur / std::max(player.hpMax, 1.0f);
		DrawRectangle(0, 0, barWidth * hpPortion, barHeight, ColorLerp(RED, GREEN, hpPortion));
		float mtPortion = player.matterCur / std::max(player.matterMax, 1.0f);
		DrawRectangle(0, barHeight, barWidth * mtPortion, barHeight, GRAY);
		
		// Player Side: Texts
		float barCenterX = barWidth / 2 - radiusDial / 2;
		float barCenterY = barHeight / 2;
		DrawTextCenter(font, TextFormat("%d / %d", (int)std::ceil(player.hpCur), (int)std::ceil(player.hpMax)),
			barCenterX, barCenterY,
			barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("%.2f", player.matterCur),
			barCenterX, barCenterY + barHeight,
			barFontSize, barFontSpacing, RAYWHITE);
		
		// Opponent Side: Bars
		hpPortion = infor.GetOpponentHPPortion();
		DrawRectangle(barWidth + barWidth * (1 - hpPortion), 0, barWidth * hpPortion + 1, barHeight, ColorLerp(RED, GREEN, hpPortion));
		mtPortion = infor.GetOpponentMTPortion();
		DrawRectangle(barWidth + barWidth * (1 - mtPortion), barHeight, barWidth * mtPortion + 1, barHeight, GRAY);
		
		// Opponent Side: Texts
		barCenterX = barCenterX + barWidth + radiusDial;
		DrawTextCenter(font, TextFormat("%s", infor.GetOpponentHealth().c_str()),
			barCenterX, barCenterY,
			barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("%s", infor.GetOpponentMatter().c_str()),
			barCenterX, barCenterY + barHeight,
			barFontSize, barFontSpacing, RAYWHITE);

		// Player Side: Attrs
		barCenterY = barHeight * 3 - barHeight / 2;
		float attrCenterX = conf::GetScreenWidth() / 12.0f;
		DrawTextCenter(font, TextFormat("P: %d", player.levelKnowledge), attrCenterX * 1.0f, barCenterY,
				barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("D: %d", player.levelDeception), attrCenterX * 2.5f, barCenterY,
				barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("E: %.2f", player.energyCur), attrCenterX * 4.0f, barCenterY,
				barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("ATK: %.2f", player.attackModifier), attrCenterX * 8.0f, barCenterY,
				barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("DEF: %.2f", player.defenseModifier), attrCenterX * 9.5f, barCenterY,
				barFontSize, barFontSpacing, RAYWHITE);
		DrawTextCenter(font, TextFormat("PSY: %.2f", player.damageModifier), attrCenterX * 11.0f, barCenterY,
				barFontSize, barFontSpacing, RAYWHITE);

		// CC Countdown Dial
		float xCenter = (float)conf::GetScreenWidth() / 2;
		float yDial = bannerHeight / 2;
		float widthDial = 8.0f;
		int timerPortion = (int)std::max(std::floor(((creditTimer - alarm[0]) / creditTimer) * 360), 0.0f);
		DrawCircle(xCenter, yDial, radiusDial, { 0x2B, 0x2B, 0x2B, 156 });
		DrawRing((Vector2){ xCenter, yDial }, radiusDial - widthDial, radiusDial, 180, 180 + timerPortion, 180, BLUE);
		
		// Credit Display
		int fontSize = 50;
		float fontSpacing = fontSize / 10.0f;
		DrawTextCenter(font, TextFormat("%d", player.credits),
			xCenter, yDial,
			fontSize, fontSpacing, creditColor);
	};

	BEGIN_ALARM(0.0f)
		if (!IsGameEnded())
		{
			if (player.credits < 999)
				player.credits++;
			else
				player.credits = 999;
			alarm[0] = creditTimer;
		}
	END_ALARM
}
