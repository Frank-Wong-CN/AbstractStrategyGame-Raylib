#pragma once

class Decision;
class DecisionMaker;
class DecisionTree;
class Information;
class GameController;

enum ConnectionState
{
	NoConnection,
	ServerListening,
	ServerConnected,
	ClientStartup,
	ClientConnecting,
	ClientConnected,
	ConnectionTerminated,
	StopConnection
};

enum NetworkedGameState
{
	NoNetwork,
	InitGame,
	StagingSent,
	GamingUpdate,
	EndGame,
	RestartGame
};

enum NodeType
{
	Perception,
	Regeneration,
	Destruction,
	Protection,
};

enum StagingMessageID
{
	NoStaging = ID_USER_PACKET_ENUM,
	StagingInitSend = 1,
	StagingInitReceiveACK = 2,
	StagingUpdate = 3,
	StagingEnd = 4,
	StagingSurrender = 5,
	StagingRestart = 6,
};

class NodePreset
{
public:
	int cost;
	float probability;
	int *descIcon; /* Reserved */
	string desc;
	std::function<void(DecisionMaker *, DecisionMaker *)> otherAbility;
};

class PossibleNodes
{
public:
	vector<NodePreset> nodes;
	
	PossibleNodes();
	void ReadFromFile(const char *fname);
	NodePreset GetRandomNode() const;
};

class DecisionTree
{
public:
	Decision *root;
	PossibleNodes *pbln;
	vector<Decision *> allNodes;
	vector<Decision *> activeNodes;
	vector<Decision *> curLeaves;
	map<Decision *, vector<Decision *> > nodesByRoot;
	vector<vector<Decision *> > nodesByLevel;
	vector<int> spans;
	bool enabled;
	bool disableSib;
	int curLayer;
	int activeLayerRange;
	const float nodeVertSpacing = NODE_VERTICAL_SPACING;
	const float nodeHoriSpacing = NODE_HORIZONTAL_SPACING;
	const float nodeRadius = NODE_RADIUS;
	float centerX;
	float bottomY;

	DecisionTree();
	int GetMaxSpan();
	int GetDepth();
	void InitTree();
	void AddNode(Decision *n);
	Vector2 ToggleTree(bool force = false, bool val = false);
	void CheckActive();
	void DisableSiblings(Decision *n, int layer);
	void ArrangeNode(int depth = 0);
	void Destroy();
};

class DecisionMakerStaging
{
public:
	unsigned mID : 8;
	float hpCur;
	float hpMax;
	float mtCur;
	float mtMax;
	int lvK;
	int lvD;
	unsigned relative : 1;
	unsigned padding : 23;
	unsigned verify : 32;
};

class DecisionMaker
{
public:
	int credits;
	float creditRestoreTimeBase;
	float creditRestoreTimeModifier;
	
	float hpCur;
	float hpMax;
	float matterCur;
	float matterMax;
	float energyCur;
	float energyMax; /* Reserved? */

	float damageModifier;
	float attackModifier;
	float defenseModifier;

	// Fill-up effect for three base values
	float recov[3]; // HP, MT, EG
	float recovSpeed[3];
	bool recovRetainSurplus[3];

	float autoHeal;
	
	int levelKnowledge;
	int levelDeception;

	DecisionTree *perception;
	DecisionTree *regeneration;
	DecisionTree *destruction;
	DecisionTree *protection;
};

class Information
{
private:
	const DecisionMaker *player;
	const DecisionMaker *opponent;

	int oldPlayerKnowledge;
	int oldOpponentDeception;
	float oldOpponentHP;
	float oldOpponentMT;
	float fakeOpponentHP;
	float fakeOpponentHPMax;
	float fakeOpponentMT;
	float fakeOpponentMTMax;
	float fakeHPNum[5];
	float fakeMTNum[5];
	string descOppHP;
	string descOppMT;
	float updateTime;
	float updateTimer;

public:
	Information();
	Information(const DecisionMaker *pl, const DecisionMaker *opp);
	
	const Information *operator()(const DecisionMaker *pl, const DecisionMaker *opp);
	void Update(bool force = false);
	void CalculateFakeValue(const float oldValue, const float oldValueMax, const int diff, float &fakeValue, float &fakeValueMax, string &descValue, float cache[5]);
	float GetOpponentHPPortion() const;
	float GetOpponentMTPortion() const;
	string GetOpponentHealth() const;
	string GetOpponentMatter() const;
};
