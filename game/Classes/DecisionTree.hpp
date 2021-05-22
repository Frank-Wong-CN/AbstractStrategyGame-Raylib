#pragma once

DecisionTree::DecisionTree() : allNodes(), activeNodes(), curLeaves(), spans(), nodesByRoot(), nodesByLevel()
{
	InitTree();
}

void DecisionTree::InitTree()
{
	root = nullptr;
	pbln = nullptr;
	allNodes.clear();
	activeNodes.clear();
	curLeaves.clear();
	curLayer = 0;
	activeLayerRange = ACTIVE_LAYER_RANGE;
	spans.clear();
	nodesByRoot.clear();
	nodesByLevel.clear();
	enabled = false;
	disableSib = DISABLE_SIBLING;
	centerX = 0.0f;
	bottomY = 0.0f;
}

Vector2 DecisionTree::ToggleTree(bool force, bool val)
{
	if (force)
		enabled = val;
	else enabled = !enabled;
	for (auto &n : allNodes)
		n->enabled = enabled;
	
	return (Vector2){ centerX, bottomY };
}

void DecisionTree::CheckActive()
{
	for (auto i = activeNodes.begin(); i != activeNodes.end(); ++i)
		if ((*i)->layer < curLayer - activeLayerRange)
		{
			(*i)->inactive = true;
			activeNodes.erase(i--);
		}
}

void DecisionTree::DisableSiblings(Decision *n, int layer)
{
	for (auto &i : nodesByLevel[layer])
	{
		if (i == n)
			continue;
		i->inactive = true;
	
		auto node = std::find(activeNodes.begin(), activeNodes.end(), i);
		if (node != activeNodes.end())
			activeNodes.erase(node);
	}
	
	auto node = std::find(activeNodes.begin(), activeNodes.end(), n);
	if (node != activeNodes.end())
		activeNodes.erase(node);
}

void DecisionTree::AddNode(Decision *n)
{
	n->tid = allNodes.size();
	allNodes.push_back(n);
	n->enabled = enabled;
	n->radius = nodeRadius;
	n->tree = this;
	
	if (root == nullptr)
	{
		root = n;
		root->tid = 0;
		root->tree = this;
		pbln = root->pbln;
		activeNodes.push_back(n);
		curLeaves.push_back(n);
		spans.push_back(1);
		nodesByLevel.emplace_back(vector<Decision *>{ n });
		nodesByRoot.insert({ n, vector<Decision *>{} });
		return;
	}

	if (!n->inactive)
		activeNodes.push_back(n);
	
	if (n->layer > curLayer)
	{
		curLayer += 1;
		spans.push_back(0);
		nodesByLevel.emplace_back(vector<Decision *>{});
		
		CheckActive();
	}
	
	if (n->layer == curLayer)
		curLeaves.push_back(n);
	
	auto parentLeaf = std::find(curLeaves.begin(), curLeaves.end(), n->parentNode);
	if (parentLeaf != curLeaves.end())
		curLeaves.erase(parentLeaf);
	
	spans[n->layer] += 1;
	nodesByLevel[n->layer].push_back(n);
	
	if (!nodesByRoot.contains(n->parentNode))
		nodesByRoot.insert({ n->parentNode, vector<Decision *>{} });
	nodesByRoot[n->parentNode].push_back(n);
}

void DecisionTree::Destroy()
{
	while (allNodes.size() > 0)
	{
		auto tmp = allNodes.begin();
		gm::RemoveInstance((*tmp)->id);
		delete *tmp;
		allNodes.erase(tmp);
	}
	delete pbln;
	InitTree();
}

void DecisionTree::ArrangeNode(int depth)
{
	if (depth > curLayer)
		return;
	
	if (depth == 0)
	{
		root->x = 0;
		root->y = 0;
	}
	else
	{
		int span = spans[depth];
		float layerWidth = span * nodeRadius * 2 + (span - 1) * nodeHoriSpacing;
		float layerY = depth * (nodeRadius * 2 + nodeVertSpacing);
		
		centerX = 0.0f;
		bottomY = layerY;
		
		float startingX = -(layerWidth / 2.0f) + nodeRadius;
		float nodeSpacing = nodeRadius * 2 + nodeHoriSpacing;
		int arranged = 0;
		
		for (auto &parent : nodesByLevel[depth - 1])
		{
			for (auto &node : parent->subNodes)
			{
				node->x = startingX + nodeSpacing * arranged;
				node->y = layerY;
				arranged++;
			}
		}
	}
	
	ArrangeNode(depth + 1);
}

int DecisionTree::GetMaxSpan()
{
	int span = 0;
	for (auto &l : nodesByLevel)
		span = std::max(span, (int)l.size());
	return span;
}

int DecisionTree::GetDepth()
{ return curLayer; }
