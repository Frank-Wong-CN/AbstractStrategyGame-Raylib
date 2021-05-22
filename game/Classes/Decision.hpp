#pragma once

void Decision::InitNode(int layer, NodeType type, GameController *ctrl)
{
	this->ctrl = ctrl;
	this->layer = layer;
	this->type = type;
	parentNode = nullptr;
	tree = nullptr;
	pbln = nullptr;
	inactive = false;
	enabled = false;
	used = false;
	failed = false;
	loading = false;
	tid = -1;
	cost = 5;
	loadTime = 1.0f;
	probability = 50.0f;
	desc = "";
	descIcon = nullptr;
	otherAbility = nullptr;
	
	switch (type)
	{
	case Perception:
		nodeColor = NODE_PERCEP_COLOR;
		textColor = NODE_PERCEP_TEXT_COLOR;
		break;
	case Regeneration:
		nodeColor = NODE_REGEN_COLOR;
		textColor = NODE_REGEN_TEXT_COLOR;
		break;
	case Destruction:
		nodeColor = NODE_DESTR_COLOR;
		textColor = NODE_DESTR_TEXT_COLOR;
		break;
	case Protection:
		nodeColor = NODE_PROTEC_COLOR;
		textColor = NODE_PROTEC_TEXT_COLOR;
		break;
	}
}

void Decision::SetParent(Decision *p)
{
	parentNode = p;
	if (p != nullptr)
		p->subNodes.push_back(this);
}

void Decision::SetFunction()
{
	NodePreset np;
	
	if (!pbln)
	{
		pbln = new PossibleNodes();
		switch (type)
		{
		case Perception:
			pbln->ReadFromFile(NODE_CONF_DIR NODE_PERCEP_FILE);
			break;
		case Regeneration:
			pbln->ReadFromFile(NODE_CONF_DIR NODE_REGEN_FILE);
			break;
		case Destruction:
			pbln->ReadFromFile(NODE_CONF_DIR NODE_DESTR_FILE);
			break;
		case Protection:
			pbln->ReadFromFile(NODE_CONF_DIR NODE_PROTEC_FILE);
			break;
		}
	}
	
	np = pbln->GetRandomNode();
	
	probability = np.probability;
	cost = np.cost;
	otherAbility = np.otherAbility;
	desc = np.desc;
}

void Decision::SetType(NodeType type)
{
	this->type = type;
}

void Decision::DeriveNodes()
{
	/* New generation rule here, if there is any */
	for (int i = 0; i < GetRandomValue(NODE_GEN_RANGE_MIN, NODE_GEN_RANGE_MAX); i++)
	{
		Decision *node = new Decision();
		node->InitNode(layer + 1, this->type, ctrl);
		node->pbln = this->pbln;
		tree->AddNode(node);
		node->SetParent(this);
		node->SetFunction();
		gm::AddInstance(node);
	}
	tree->ArrangeNode(layer);
}

Decision::BEGIN_FUNC(Decision) {
	FUNC(Create)
	{
		x = -500;
		y = -500;
	};
	
	FUNC(Step)
	{
		if (!enabled)
			return;
		
		if (!used && !inactive && !loading &&
			IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
			CheckCollisionPointCircle(gm::mouse, (Vector2){ x, y }, radius) &&
			ctrl->ReducePlayerCredit(cost))
		{
			alarm[0] = loadTime;
			loading = true;
			if (tree->disableSib)
				tree->DisableSiblings(this, layer);
		}
	};
	
	FUNC(Draw)
	{
		if (!enabled)
			return;
		
		if (inactive)
		{
			if (!used)
			{
				nodeColor.a = 128;
				float lineWid = 2.0f;
				float lineLen = radius * 0.8f;
				Vector2 no1 = { x + cosf(45 * DEG2RAD) * lineLen, y + sinf(45 * DEG2RAD) * lineLen };
				Vector2 no2 = { x + -cosf(45 * DEG2RAD) * lineLen, y + sinf(45 * DEG2RAD) * lineLen };
				Vector2 no3 = { x + -cosf(45 * DEG2RAD) * lineLen, y + -sinf(45 * DEG2RAD) * lineLen };
				Vector2 no4 = { x + cosf(45 * DEG2RAD) * lineLen, y + -sinf(45 * DEG2RAD) * lineLen };
				DrawLineEx(no2, no4, lineWid, GRAY);
				DrawLineEx(no1, no3, lineWid, GRAY);
			}
		}
		else
			nodeColor.a = 255;
		
		/*
		if (parentNode)
		{
			float lineWid = 2.0f;
			Vector2 fromParent = { x - parentNode->x, y - parentNode->y };
			Vector2 normal = { 1, 0 };
			fromParent = Vector2Normalize(fromParent);
			float deg = Vector2DotProduct(fromParent, normal);
			deg = acos(deg);
			DrawLineEx({ x - cosf(deg) * radius, y - sinf(deg) * radius }, { parentNode->x + cosf(deg) * radius, parentNode->y + sinf(deg) * radius }, lineWid, { 0x2B, 0x2B, 0x2B, 165 });
		}
		*/
		
		DrawCircle(x, y, radius, nodeColor);
		DrawCircleLines(x, y, radius, BLACK);
		
		if (!used && !inactive && !loading)
		{
			DrawTextCenter(GetFontDefault(), TextFormat("$%d\n%.2f%%\n%s", cost, probability, desc.c_str()), x, y, 20, 1.6f, textColor);
		}
		
		if (loading)
		{
			float lineWid = 2.0f;
			float lineLen = radius * 0.8f;
			Vector2 no1 = { x + cosf(60 * DEG2RAD) * lineLen, y + sinf(60 * DEG2RAD) * lineLen };
			Vector2 no2 = { x + -cosf(60 * DEG2RAD) * lineLen, y + sinf(60 * DEG2RAD) * lineLen };
			Vector2 no3 = { x + -cosf(60 * DEG2RAD) * lineLen, y + -sinf(60 * DEG2RAD) * lineLen };
			Vector2 no4 = { x + cosf(60 * DEG2RAD) * lineLen, y + -sinf(60 * DEG2RAD) * lineLen };
			DrawLineEx(no2, no4, lineWid, GRAY);
			DrawLineEx(no1, no3, lineWid, GRAY);
			DrawLineEx(no1, no2, lineWid, GRAY);
			DrawLineEx(no4, no3, lineWid, GRAY);
			
			float ringWid = 5.0f;
			int timerPortion = std::floor((loadTime - alarm[0]) / std::max(loadTime, 1.0f) * 360);
			DrawRing((Vector2){ x, y }, radius, radius + ringWid, 180, 180 + timerPortion, 180, BLUE);
			DrawCircleLines(x, y, radius + ringWid, BLACK);
		}
		
		if (used)
		{
			float lineWid = 5.0f;
			DrawCircle(x, y, radius, { 0, 0, 0, 128 });
			if (failed)
			{
				float lineLen = radius * 0.8f;
				Vector2 no1 = { x + cosf(45 * DEG2RAD) * lineLen, y + sinf(45 * DEG2RAD) * lineLen };
				Vector2 no2 = { x + -cosf(45 * DEG2RAD) * lineLen, y + sinf(45 * DEG2RAD) * lineLen };
				Vector2 no3 = { x + -cosf(45 * DEG2RAD) * lineLen, y + -sinf(45 * DEG2RAD) * lineLen };
				Vector2 no4 = { x + cosf(45 * DEG2RAD) * lineLen, y + -sinf(45 * DEG2RAD) * lineLen };
				DrawLineEx(no2, no4, lineWid, RED);
				DrawLineEx(no1, no3, lineWid, RED);
			}
			else
			{
				Vector2 no1 = { x - cosf(0 * DEG2RAD) * radius * 0.6f, y - sinf(0 * DEG2RAD) * radius * 0.6f };
				Vector2 no2 = { x - cosf(85 * DEG2RAD) * radius * 0.65f, y + sinf(85 * DEG2RAD) * radius * 0.65f };
				Vector2 no3 = { x + cosf(45 * DEG2RAD) * radius * 0.8f, y - sinf(45 * DEG2RAD) * radius * 0.8f };
				DrawLineEx(no1, no2, lineWid, GREEN);
				DrawLineEx(no2, no3, lineWid, GREEN);
			}
		}
	};
	
	BEGIN_ALARM(0.0f)
		used = true;
		loading = false;
		int hit = GetRandomValue(0, 10000) / 100.0f;
		if (hit > probability)
			failed = true;
		
		DeriveNodes();
		//gm::GetCurrentCamera().target.y = y + radius;
		
		if (!failed)
			ctrl->DecisionEffect(this);
	END_ALARM
}
