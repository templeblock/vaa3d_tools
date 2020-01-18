//------------------------------------------------------------------------------
// Copyright (c) 2019 Hsienchi Kuo (Allen Institute, Hanchuan Peng's team)
// All rights reserved.
//------------------------------------------------------------------------------

/*******************************************************************************
*
*  [integratedDataTypes] is part of the NeuronStructNavigator library.
*  The namespace manages all integrated data structures used by all other NeuronStructNavigator classes.
*  All data structures in this namespace are integrated with standard Vaa3D data types with additional features, aiming to make developing neuron structure operations and algorithms more convenient.
*  Any new development on the datatypes should be put in this namespace to keep them organized and avoid the confusion of header inclusion.
*
*  [profiledTree] is the core data type in throughout the whole NeuronStructNavigator library. It profiles the NeuronTree and carries crucial information of it.
*  Particularly profiledTree provides node-location, child-location, and detailed segment information of a NeuronTree.
*  Each segment of a NeuronTree is represented as a segUnit struct. A segUnit struct carries within-segment node-location, child-location, head, and tails information.
*  All segments are stored and sorted in profiledTree's map<int, segUnit> data member.

********************************************************************************/

#include <iostream>

#include "integratedDataTypes.h"
#include "NeuronGeoGrapher.h"
#include "NeuronStructExplorer.h"
#include "NeuronStructUtilities.h"

using namespace std;

integratedDataTypes::segUnit::segUnit(const V_NeuronSWC& inputV_NeuronSWC) : to_be_deleted(false)
{
	this->segID = inputV_NeuronSWC.row.begin()->seg_id;
	this->head = (inputV_NeuronSWC.row.end() - 1)->n;
	this->tails.push_back(inputV_NeuronSWC.row.begin()->n);
	for (vector<V_NeuronSWC_unit>::const_iterator unitIt = inputV_NeuronSWC.row.begin(); unitIt != inputV_NeuronSWC.row.end() - 1; ++unitIt)
	{
		NeuronSWC node;
		node.x = unitIt->data[2];
		node.y = unitIt->data[3];
		node.z = unitIt->data[4];
		node.type = unitIt->data[1];
		node.n = unitIt->data[0];
		node.parent = unitIt->data[6];

		this->nodes.push_front(node);
	}

	NeuronStructUtil::node2loc_node2childLocMap(this->nodes, this->seg_nodeLocMap, this->seg_childLocMap);
}

integratedDataTypes::segPairProfile::segPairProfile(const segUnit& inputSeg1, const segUnit& inputSeg2, connectOrientation connOrt) : seg1Ptr(&inputSeg1), seg2Ptr(&inputSeg2), currConnOrt(connOrt)
{
	this->getSegDistance(connOrt);

	if (inputSeg1.tails.size() > 1 || inputSeg2.tails.size() > 1)
	{
		cerr << " -- Branched segments are currently not supported. The turnning angle of 2 segments won't be computed." << endl;
		return;
	}
	
	this->turning12(connOrt);
	this->segsAngleDiff12(connOrt);
}

void integratedDataTypes::segPairProfile::getSegDistance(connectOrientation connOrt)
{
	if (connOrt == head_head)
	{
		this->currConnOrt = head_head;
		NeuronSWC seg1Head = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(this->seg1Ptr->head));
		NeuronSWC seg2Head = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(this->seg2Ptr->head));
		this->connDistMap.insert({ head_head, sqrt((seg1Head.x - seg2Head.x) * (seg1Head.x - seg2Head.x) + (seg1Head.y - seg2Head.y) * (seg1Head.y - seg2Head.y) + (seg1Head.z - seg2Head.z) * (seg1Head.z - seg2Head.z)) });
	}
	else if (connOrt == head_tail)
	{
		this->currConnOrt = head_tail;
		NeuronSWC seg1Head = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(this->seg1Ptr->head));
		NeuronSWC seg2Tail = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(*this->seg2Ptr->tails.begin()));
		this->connDistMap.insert({ head_tail, sqrt((seg1Head.x - seg2Tail.x) * (seg1Head.x - seg2Tail.x) + (seg1Head.y - seg2Tail.y) * (seg1Head.y - seg2Tail.y) + (seg1Head.z - seg2Tail.z) * (seg1Head.z - seg2Tail.z)) });
	}
	else if (connOrt == tail_head)
	{
		this->currConnOrt = tail_head;
		NeuronSWC seg1Tail = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(*this->seg1Ptr->tails.begin()));
		NeuronSWC seg2Head = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(this->seg2Ptr->head));
		this->connDistMap.insert({ tail_head, sqrt((seg1Tail.x - seg2Head.x) * (seg1Tail.x - seg2Head.x) + (seg1Tail.y - seg2Head.y) * (seg1Tail.y - seg2Head.y) + (seg1Tail.z - seg2Head.z) * (seg1Tail.z - seg2Head.z)) });
	}
	else if (connOrt == tail_tail)
	{
		this->currConnOrt = tail_tail;
		NeuronSWC seg1Tail = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(*this->seg1Ptr->tails.begin()));
		NeuronSWC seg2Tail = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(*this->seg2Ptr->tails.begin()));
		this->connDistMap.insert({ tail_tail, sqrt((seg1Tail.x - seg2Tail.x) * (seg1Tail.x - seg2Tail.x) + (seg1Tail.y - seg2Tail.y) * (seg1Tail.y - seg2Tail.y) + (seg1Tail.z - seg2Tail.z) * (seg1Tail.z - seg2Tail.z)) });
	}
	else if (connOrt == all_ort)
	{
		this->currConnOrt = all_ort;
		NeuronSWC seg1Head = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(this->seg1Ptr->head));
		NeuronSWC seg2Head = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(this->seg2Ptr->head));
		NeuronSWC seg1Tail = this->seg1Ptr->nodes.at(this->seg1Ptr->seg_nodeLocMap.at(*this->seg1Ptr->tails.begin()));
		NeuronSWC seg2Tail = this->seg2Ptr->nodes.at(this->seg2Ptr->seg_nodeLocMap.at(*this->seg2Ptr->tails.begin()));
		this->connDistMap.insert({ head_head, sqrt((seg1Head.x - seg2Head.x) * (seg1Head.x - seg2Head.x) + (seg1Head.y - seg2Head.y) * (seg1Head.y - seg2Head.y) + (seg1Head.z - seg2Head.z) * (seg1Head.z - seg2Head.z)) });
		this->connDistMap.insert({ head_tail, sqrt((seg1Head.x - seg2Tail.x) * (seg1Head.x - seg2Tail.x) + (seg1Head.y - seg2Tail.y) * (seg1Head.y - seg2Tail.y) + (seg1Head.z - seg2Tail.z) * (seg1Head.z - seg2Tail.z)) });
		this->connDistMap.insert({ tail_head, sqrt((seg1Tail.x - seg2Head.x) * (seg1Tail.x - seg2Head.x) + (seg1Tail.y - seg2Head.y) * (seg1Tail.y - seg2Head.y) + (seg1Tail.z - seg2Head.z) * (seg1Tail.z - seg2Head.z)) });
		this->connDistMap.insert({ tail_tail, sqrt((seg1Tail.x - seg2Tail.x) * (seg1Tail.x - seg2Tail.x) + (seg1Tail.y - seg2Tail.y) * (seg1Tail.y - seg2Tail.y) + (seg1Tail.z - seg2Tail.z) * (seg1Tail.z - seg2Tail.z)) });
	}
}

void integratedDataTypes::segPairProfile::turning12(connectOrientation connOrt)
{
	if (connOrt == head_head) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
	else if (connOrt == head_tail) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
	else if (connOrt == tail_head) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
	else if (connOrt == tail_tail) this->turningAngle = NeuronGeoGrapher::segTurningAngle(*this->seg1Ptr, *this->seg2Ptr, connOrt);
}

void integratedDataTypes::segPairProfile::segsAngleDiff12(connectOrientation connOrt)
{
	vector<double> dispVec1, dispVec2;
	if (connOrt == head_head)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, head);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, tail);
	}
	else if (connOrt == head_tail)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, head);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, head);
	}
	else if (connOrt == tail_head)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, tail);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, tail);
	}
	else if (connOrt == tail_tail)
	{
		dispVec1 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg1Ptr, tail);
		dispVec2 = NeuronGeoGrapher::getSegDispVector<double>(*this->seg2Ptr, head);
	}

	this->segsAngleDiff = NeuronGeoGrapher::getPiAngle(dispVec1, dispVec2);
}

integratedDataTypes::profiledTree::profiledTree(const NeuronTree& inputTree, float nodeTileLength, float segTileLength)
{
	this->tree = inputTree;
	this->segTileSize = segTileLength;
	this->nodeTileSize = nodeTileLength;

	NeuronStructUtil::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileSize);
	NeuronStructUtil::node2loc_node2childLocMap(this->tree.listNeuron, this->node2LocMap, this->node2childLocMap);

	this->segs = NeuronStructExplorer::findSegs(this->tree.listNeuron, this->node2childLocMap);
	//cout << "segs num: " << this->segs.size() << endl;
	
	vector<segUnit> allSegs;
	for (map<int, segUnit>::iterator it = this->segs.begin(); it != this->segs.end(); ++it)
	{
		//if (it->second.tails.size() > 1) cout << " branching seg: " << it->first << endl;
		allSegs.push_back(it->second);
	}
	this->segHeadMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength);
	this->segTailMap = NeuronStructExplorer::segTileMap(allSegs, segTileLength, false);
}

void integratedDataTypes::profiledTree::nodeTileResize(float nodeTileLength)
{
	if (nodeTileLength == NODE_TILE_LENGTH) return;
	else
	{
		if (!this->nodeTileMap.empty())
		{
			this->nodeTileMap.clear();
			NeuronStructUtil::nodeTileMapGen(this->tree, this->nodeTileMap, nodeTileLength);
		}
		else
		{
			this->nodeTileSize = nodeTileLength;
			return;
		}
	}
}

void integratedDataTypes::profiledTree::addTopoUnit(int nodeID)
{
	if (this->node2LocMap.empty() || this->node2childLocMap.empty())
	{
		cerr << "The tree profile hasn't been initialized yet. Do 'profiledTree(NeuronTree)' before calling to this method.\n Do nothing and return." << endl;
		return;
	}

	topoCharacter topoUnit(this->tree.listNeuron.at(this->node2LocMap.at(nodeID)));
	topoUnit.upstream.clear();
	topoUnit.downstreams.clear();

	int parentID = topoUnit.topoCenter.parent;
	while (topoUnit.upstream.size() <= 10)
	{
		if (this->node2LocMap.find(parentID) == this->node2LocMap.end()) break;
		topoUnit.upstream.push_back(this->tree.listNeuron.at(this->node2LocMap.at(parentID)));
		parentID = topoUnit.upstream.back().parent;
		if (parentID == -1) break;
	}

	for (vector<size_t>::iterator it = this->node2childLocMap.at(nodeID).begin(); it != this->node2childLocMap.at(nodeID).end(); ++it)
	{
		deque<NeuronSWC> downstream;
		int childID = this->tree.listNeuron.at(*it).n;
		while (downstream.size() <= 10)
		{
			if (this->node2LocMap.find(childID) == this->node2LocMap.end()) break;
			downstream.push_back(this->tree.listNeuron.at(this->node2LocMap.at(childID)));
			cout << downstream.back().n << " ";
			childID = this->tree.listNeuron.at(*node2childLocMap.at(childID).begin()).n;
		}
		topoUnit.downstreams.insert(pair<int, deque<NeuronSWC>>(downstream.begin()->n, downstream));
	}

	this->topoList.insert(pair<int, topoCharacter>(nodeID, topoUnit));
}

void integratedDataTypes::profiledTreeReInit(profiledTree& inputProfiledTree)
{
	profiledTree tempTree(inputProfiledTree.tree, inputProfiledTree.segTileSize);
	inputProfiledTree = tempTree;
}