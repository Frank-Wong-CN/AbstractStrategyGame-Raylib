#pragma once

PossibleNodes::PossibleNodes() : nodes()
{}

void PossibleNodes::ReadFromFile(const char *fname)
{
	ifstream read(fname);
	if (!read)
		return;

	while (!read.eof())
	{
		NodePreset n = { 0 };
		int temp = 0;
		read >> n.cost;
		read >> n.probability;
		//read >> n.descIcon;
		
		int type = -1;
		float value = 0.0f;
		read >> n.desc;
		read >> type;
		read >> value;
		std::function<void(DecisionMaker *, DecisionMaker *)> tmp;
		
		float val2 = 0.0f;
		switch (type)
		{
		case 0: // HP Add
			read >> val2;
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->recov[0] += GetRandomValue(value, val2);
			};
			break;
		case 1: // MT Add
			read >> val2;
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->matterMax += value;
				sender->recov[1] += GetRandomValue(value, val2);
			};
			break;
		case 2: // EG Add
			read >> val2;
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->recov[2] += GetRandomValue(value, val2);
			};
			break;
		case 3: // Knowledge Add
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->levelKnowledge += value;
			};
			break;
		case 4: // Deception Add
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->levelDeception += value;
			};
			break;
		case 5: // Attack Mod Add
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->attackModifier += value;
			};
			break;
		case 6: // Defence Mod Add
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->defenseModifier += value;
			};
			break;
		case 7: // Damage Mod Dec
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->damageModifier *= value;
			};
			break;
		case 8: // Credit Time Dec
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->creditRestoreTimeModifier *= value;
			};
			break;
		case 9: // Max HP (Level per 100) Add
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->hpMax += value;
				sender->recov[0] += value;
			};
			break;
		case 10: // MT Restore
			tmp = [=](DecisionMaker *sender, DecisionMaker *target){
				sender->recov[1] += value;
			};
			break;
		}
		
		n.otherAbility = tmp;
		nodes.push_back(n);
	}
	
	read.close();
}

NodePreset PossibleNodes::GetRandomNode() const
{
	int random = GetRandomValue(0, nodes.size() - 1);
	return nodes.at(random);
}
