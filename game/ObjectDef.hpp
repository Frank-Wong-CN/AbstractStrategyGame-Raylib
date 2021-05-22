#pragma once

enum NodeType;
class DecisionMaker;
class DecisionTree;
class Information;
class PossibleNodes;
class GameController;
class ThreadFunction;

CLASS(Decision)
{
friend class DecisionTree;
private:
	GameController *ctrl;
	Decision *parentNode;
	DecisionTree *tree;
	PossibleNodes *pbln;
	vector<Decision *> subNodes;
	int layer;
	int tid;
	bool inactive;
	bool enabled;
	bool used;
	bool failed;
	bool loading;
	float radius;
	float loadTime;
	Color nodeColor;
	Color textColor;
	
public:
	NodeType type;
	int cost;
	float probability;
	int *descIcon;
	string desc;
	std::function<void(DecisionMaker *, DecisionMaker *)> otherAbility;

	Decision();
	void InitNode(int layer, NodeType type, GameController *ctrl = nullptr);
	void SetParent(Decision *p);
	void SetFunction();
	void SetType(NodeType type);
	void DeriveNodes();
};

CLASS(GameController)
{
private:
	Font font;
	DecisionMaker player;
	DecisionMaker opponent;
	Information infor;
	
	float creditTimer;
	Vector2 panStart;
	Color creditColor;
	float creditColorLerp;
	int gameResult;
	
	int argc;
	char **argv;
	const char *networkIP;
	int port;
	ConnectionState conn;
	SLNet::Packet *packet;
	SLNet::RakPeerInterface *peer;
	NetworkedGameState nGameState;
	DecisionMakerStaging stagingSend;
	DecisionMakerStaging stagingRecv;
	
	std::mutex accessLock;
	std::thread *networkThr;
	ThreadFunction *networkOpServer;
	ThreadFunction *networkOpClient;
	
	bool IsGameEnded();
	bool IsNetworkGame();
	void SetupPlayer(DecisionMaker &player);
	void RecoverValue(float &val, float valMax, float &remaining, float speed, bool retainSurplus);
	void DoRecover(DecisionMaker &player);
	void AI();
	void Sync();
	void ASync(bool isServer);
	void InitSession();
	void InitNetworkedSession();
	void InitDummy();
	void CopyStaging(DecisionMakerStaging &staging, const DecisionMaker &copy, bool force = false);
	void PasteStaging(DecisionMakerStaging &staging, DecisionMaker &paste, bool force = false);
	bool VerifyStaging(const DecisionMakerStaging &staging);
	void SwitchTree(NodeType type);
	void Unleash(DecisionMaker &player, DecisionMaker &opponent);

public:
	GameController();
	void DecisionEffect(Decision *n);
	bool ReducePlayerCredit(int cost);
	void SetCommandlineArguments(int argc, char *argv[]);
};
