#pragma once

Information::Information() : player(nullptr), opponent(nullptr) {}
Information::Information(const DecisionMaker *pl, const DecisionMaker *opp) : player(pl), opponent(opp) {}

const Information *Information::operator()(const DecisionMaker *pl, const DecisionMaker *opp)
{
	player = pl;
	opponent = opp;
	oldPlayerKnowledge = 0;
	oldOpponentDeception = 0;
	oldOpponentHP = 0;
	oldOpponentMT = 0;
	fakeOpponentHP = 0.0f;
	fakeOpponentMT = 0.0f;
	updateTime = 0.0f;
	updateTimer = 0.0f;
	ArrayInit(fakeHPNum, sizeof(float) * 5);
	ArrayInit(fakeMTNum, sizeof(float) * 5);
	return this;
}

void Information::CalculateFakeValue(const float oldValue, const float oldValueMax, const int diff, float &fakeValue, float &fakeValueMax, string &descValue, float cache[5])
{
	ArrayInit(cache, sizeof(float) * 5);
	int fig = (int)std::ceil(std::log10(std::max(oldValueMax, 1.0f)));
	
	if (diff < -10)
	{
		// Just don't know
		fakeValueMax = fakeValue = std::numeric_limits<float>::max();
		descValue = "?";
	}
	else if (diff < -8) // 9, 10
	{
		// Can take a few guesses
		if (fig == 0) fig = 1;
		fig += GetRandomValue(0, 4);
		fakeValueMax = GetRandomValue(10, (int)std::pow(10, fig));
		fakeValue = GetRandomValue(10, fakeValueMax);
		if (fakeValue > fakeValueMax) fakeValue = fakeValueMax;
		cache[0] = std::pow(10, (int)std::floor(std::log10(std::max(fakeValue, 1.0f))));
		descValue = TextFormat("I Guess It's %d!", (int)cache[0]);
	}
	else if (diff < -5) // 6, 7, 8
	{
		// You can guess its digits
		if (fig == 0) fig = 1;
		fig += -3; // diff - 3
		int pow = std::max((int)std::pow(10, (int)std::abs(diff) + fig), 10);
		cache[0] = GetRandomValue(1, pow) / 100.0f;
		if (!GetRandomValue(0, 4))
			cache[0] = -cache[0];
		fakeValue = std::max(oldValue + cache[0], 0.0f);
		fakeValueMax = std::max(fakeValue, 1.0f);
		cache[1] = std::ceil(std::log10(std::max(fakeValue, 1.0f)));
		descValue = TextFormat("Must Be %d Digits!", (int)cache[1]);
	}
	else if (diff < -1) // 2, 3, 4, 5
	{
		// Somewhat accurate?
		if (fig == 0) fig = 1;
		fig += -1; // diff - 1
		int pow = std::max((int)std::pow(10, (int)std::abs(diff) + fig), 1);
		cache[1] = GetRandomValue(1, pow) / 100.0f;
		cache[2] = GetRandomValue(1, pow) / 100.0f;
		if (!GetRandomValue(0, 4))
			cache[1] = -cache[1];
		if (!GetRandomValue(0, 4))
			cache[2] = -cache[2];
		fakeValue = std::max(oldValue + cache[1], 0.0f);
		fakeValueMax = std::max(oldValueMax + cache[2], 1.0f);
		if (fakeValue > fakeValueMax) fakeValue = fakeValueMax;
		descValue = TextFormat("Around %d?", (int)std::ceil(fakeValue));
	}
	else if (diff < 0) // 1
	{
		// You know how hard it can be, but not how hard it is now
		cache[0] = GetRandomValue(-1000, 1000) / 100.0f;
		fakeValue = oldValue + cache[0];
		fakeValueMax = std::max(oldValueMax, fakeValue);
		if (fakeValue < 0.0f)
			fakeValue = 0.0f;
		descValue = TextFormat("Around %d / %d", (int)std::ceil(fakeValue), (int)std::ceil(fakeValueMax));
	}
	else
	{
		fakeValue = oldValue;
		fakeValueMax = oldValueMax;
		descValue = TextFormat("%d / %d", (int)std::ceil(fakeValue), (int)std::ceil(fakeValueMax));
	}
}

void Information::Update(bool force)
{
	bool update = force;
	
	if (oldPlayerKnowledge != player->levelKnowledge)
		oldPlayerKnowledge = player->levelKnowledge, update = true;
	if (oldOpponentDeception != opponent->levelDeception)
		oldOpponentDeception = opponent->levelDeception, update = true;

	int diff = oldPlayerKnowledge - oldOpponentDeception;
	
	if (diff < 0)
		updateTime = std::abs(diff);
	else updateTime = 0.0f;
	
	updateTimer -= GetFrameTime();
	if (updateTimer < 0.0f)
		updateTimer = updateTime, update = true;

	bool updateHP = update;
	if (oldOpponentHP != opponent->hpCur)
	{
		int fig = (int)std::ceil(std::log10(std::max(opponent->hpMax, 1.0f)));
		float thres = std::pow(10, fig - 3) * 5; // 0.5f on start
		if ((oldOpponentHP - opponent->hpCur) > thres)
			updateHP = true;
		oldOpponentHP = opponent->hpCur;
	}

	bool updateMT = update;
	if (oldOpponentMT != opponent->matterCur)
	{
		int fig = (int)std::ceil(std::log10(std::max(opponent->matterMax, 1.0f)));
		float thres = std::pow(10, fig - 3) * 5; // 0.5f on start
		if ((oldOpponentMT - opponent->matterCur) > thres)
			updateMT = true;
		oldOpponentMT = opponent->matterCur;
	}
	
	if (updateHP)
		CalculateFakeValue(opponent->hpCur, opponent->hpMax, diff, fakeOpponentHP, fakeOpponentHPMax, descOppHP, fakeHPNum);
	
	if (updateMT)
		CalculateFakeValue(opponent->matterCur, opponent->matterMax, diff, fakeOpponentMT, fakeOpponentMTMax, descOppMT, fakeMTNum);
}

float Information::GetOpponentHPPortion() const
{
	return std::min(fakeOpponentHP / std::max(fakeOpponentHPMax, 1.0f), 1.0f);
}

float Information::GetOpponentMTPortion() const
{
	return std::min(fakeOpponentMT / std::max(fakeOpponentMTMax, 1.0f), 1.0f);
}

string Information::GetOpponentHealth() const
{
	return descOppHP;
}

string Information::GetOpponentMatter() const
{
	return descOppMT;
}
