/* neuron_completeness_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-2-25 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include "neuron_completeness_funcs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

#include "neuron_completeness_plugin.h"


map<size_t, set<size_t> > seg2SegsMap;
V_NeuronSWC_list segList;
set<vector<size_t> > detectedLoops;
set<set<size_t> > detectedLoopsSet;
set<set<size_t> > finalizedLoopsSet;
set<set<size_t> > nonLoopErrors;



QStringList importSWCFileList(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";


    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}


double minDist(QList<NeuronSWC> & neuron1, QList<NeuronSWC> & neuron2)
{
    double dis=MAX_DOUBLE;
    for(int i=0;i<neuron1.size();i++)
    {
        for(int j=0;j<neuron2.size();j++)
        {
            if(NTDIS(neuron1[i],neuron2[j])<dis)
                dis = NTDIS(neuron1[i],neuron2[j]);
        }
    }
    return dis;
}

void calComplete(NeuronTree nt, QList<NEURON_METRICS> & scores)
{
    QList<NeuronSWC> sorted_neuron;
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);

    LandmarkList markerlist;

    QHash<int,int> map_type;
    QMultiMap<int, QList<NeuronSWC> > multi_neurons;
    int first = 0;
    for(V3DLONG i=0; i<sorted_neuron.size();i++)
    {
        if(sorted_neuron.at(i).pn == -1)
        {
            if(i!=0)
            {
                multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
            }
            first = i;

        }
        if(!map_type.count(sorted_neuron.at(i).type))
        {
            map_type[sorted_neuron.at(i).type] = 1;
        }
    }
    multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
    QList<double> dist;
    QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
    int row =0;
    for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
    {
        QList<NeuronSWC> neuron1= it1.value();
        double minD = MAX_DOUBLE;
        int col=0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
        {
            if(it1 != it2)
            {
                if(matrix_dis[row][col] != MAX_DOUBLE)
                    minD = MIN(minD,matrix_dis[row][col]);
                else
                {
                    QList<NeuronSWC> neuron2 = it2.value();
                    double cur_dis = minDist(neuron1,neuron2);
                    matrix_dis[row][col] = cur_dis;
                    matrix_dis[col][row] = cur_dis;
                    minD = MIN(minD,cur_dis);
                }
            }
            col++;
        }
        row++;
        dist.push_back(minD);
        if(minD>2 && minD!=MAX_DOUBLE)
        {
            LocationSimple t;
            t.x = neuron1.at(0).x;
            t.y = neuron1.at(0).y;
            t.z = neuron1.at(0).z;
            t.color.r = 0;t.color.g = 0; t.color.b = 255;
            markerlist.push_back(t);
        }
    }

    NEURON_METRICS tmp;
    tmp.numTrees = multi_neurons.size();
    tmp.numTypes = map_type.size();
    tmp.numSegs = markerlist.size();
    scores.push_back(tmp);
    return;
}

void exportComplete(NeuronTree nt,QList<NeuronSWC>& sorted_neuron, LandmarkList& markerlist, QMultiMap<int, QList<NeuronSWC> >& multi_neurons,QHash<int,int>& map_type,QList<double>& dist)
{
    SortSWC(nt.listNeuron, sorted_neuron ,VOID, 0);

    int first = 0;
    int cur_type = -1;
    for(V3DLONG i=0; i<sorted_neuron.size();i++)
    {
        if(sorted_neuron.at(i).pn == -1)
        {
            if(i!=0)
            {
                multi_neurons.insert(i-first,sorted_neuron.mid(first,i-first));
            }
            first = i;

        }
        if(!map_type.count(sorted_neuron.at(i).type))
        {
            map_type[sorted_neuron.at(i).type] = 1;
//            if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3)
//            {
//                LocationSimple t;
//                t.x = sorted_neuron.at(i).x;
//                t.y = sorted_neuron.at(i).y;
//                t.z = sorted_neuron.at(i).z;
//                t.color.r = 255;t.color.g = 0; t.color.b = 0;
//                markerlist.push_back(t);
//            }
        }/*else
        {
            if(sorted_neuron.at(i).type!=1 && sorted_neuron.at(i).type!=2 && sorted_neuron.at(i).type!=3 && sorted_neuron.at(i).type !=cur_type)
            {
                LocationSimple t;
                t.x = sorted_neuron.at(i).x;
                t.y = sorted_neuron.at(i).y;
                t.z = sorted_neuron.at(i).z;
                t.color.r = 255;t.color.g = 0; t.color.b = 0;
                markerlist.push_back(t);
            }
        }*/
        cur_type = sorted_neuron.at(i).type;
    }
    multi_neurons.insert(sorted_neuron.size()-first,sorted_neuron.mid(first,sorted_neuron.size()-first));
    QVector<QVector<double> > matrix_dis(multi_neurons.size(),QVector<double>(multi_neurons.size(),MAX_DOUBLE));
    int row =0;
    QMultiMap<double, LocationSimple> greenSeg_list;
    for (QMultiMap<int, QList<NeuronSWC> >::iterator it1 = multi_neurons.end()-1; it1 != multi_neurons.begin()-1; --it1)
    {
        QList<NeuronSWC> neuron1= it1.value();
        double minD = MAX_DOUBLE;
        int col=0;
        for (QMultiMap<int, QList<NeuronSWC> >::iterator it2 = multi_neurons.end()-1; it2 != multi_neurons.begin()-1; --it2)
        {
            if(it1 != it2)
            {
                if(matrix_dis[row][col] != MAX_DOUBLE)
                    minD = MIN(minD,matrix_dis[row][col]);
                else
                {
                    QList<NeuronSWC> neuron2 = it2.value();
                    double cur_dis = minDist(neuron1,neuron2);
                    matrix_dis[row][col] = cur_dis;
                    matrix_dis[col][row] = cur_dis;
                    minD = MIN(minD,cur_dis);
                }
            }
            col++;
        }
        row++;
        dist.push_back(minD);
        LocationSimple t;
        t.x = neuron1.at(0).x;
        t.y = neuron1.at(0).y;
        t.z = neuron1.at(0).z;
        if (minD>2 && minD!=MAX_DOUBLE)
        {
            greenSeg_list.insert(minD,t);
        }
    }
    for(QMultiMap<double, LocationSimple>::iterator it = greenSeg_list.end()-1; it != greenSeg_list.begin()-1; --it)
    {
        LocationSimple t = it.value();
        t.color.r = 0;t.color.g = 255; t.color.b = 0;t.comments = QString("gap: %1").arg(QString::number(it.key(),'f',2)).toStdString();
        markerlist.push_back(t);
    }
    return;

}

vector<NeuronSWC> loopDetection(V_NeuronSWC_list inputSegList)
{
	vector<NeuronSWC> outputErroneousPoints;
	outputErroneousPoints.clear();

	segList = inputSegList;
	seg2SegsMap.clear();
	map<string, set<size_t> > wholeGrid2segIDmap;
	set<size_t> subtreeSegs;
	size_t segCount = 0;
	for (vector<V_NeuronSWC>::iterator segIt = segList.seg.begin(); segIt != segList.seg.end(); ++segIt)
	{
		for (vector<V_NeuronSWC_unit>::iterator nodeIt = segIt->row.begin(); nodeIt != segIt->row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / GRID_LENGTH;
			int yLabel = nodeIt->y / GRID_LENGTH;
			int zLabel = nodeIt->z / GRID_LENGTH;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();
			wholeGrid2segIDmap[gridKey].insert(size_t(segIt - segList.seg.begin()));
		}
		subtreeSegs.insert(segCount);
		++segCount;
	}

	for (set<size_t>::iterator it = subtreeSegs.begin(); it != subtreeSegs.end(); ++it)
	{
		//cout << *it << ":";
		set<size_t> connectedSegs;
		connectedSegs.clear();
		if (segList.seg[*it].row.size() <= 1)
		{
			segList.seg[*it].to_be_deleted = true;
			continue;
		}
		else if (segList.seg[*it].to_be_deleted) continue;

		for (vector<V_NeuronSWC_unit>::iterator nodeIt = segList.seg[*it].row.begin(); nodeIt != segList.seg[*it].row.end(); ++nodeIt)
		{
			int xLabel = nodeIt->x / GRID_LENGTH;
			int yLabel = nodeIt->y / GRID_LENGTH;
			int zLabel = nodeIt->z / GRID_LENGTH;
			QString gridKeyQ = QString::number(xLabel) + "_" + QString::number(yLabel) + "_" + QString::number(zLabel);
			string gridKey = gridKeyQ.toStdString();

			set<size_t> scannedSegs = wholeGrid2segIDmap[gridKey];
			if (!scannedSegs.empty())
			{
				for (set<size_t>::iterator scannedIt = scannedSegs.begin(); scannedIt != scannedSegs.end(); ++scannedIt)
				{
					int connectedSegsSize = connectedSegs.size();
					if (*scannedIt == *it || segList.seg[*scannedIt].to_be_deleted) continue;
					if (segList.seg[*scannedIt].row.size() == 1) continue;

					if (segList.seg[*scannedIt].row.begin()->x == nodeIt->x && segList.seg[*scannedIt].row.begin()->y == nodeIt->y && segList.seg[*scannedIt].row.begin()->z == nodeIt->z)
					{
						connectedSegs.insert(*scannedIt);
						set<size_t> reversed;
						reversed.insert(*it);
						if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) seg2SegsMap[*scannedIt].insert(*it);
					}
					else if ((segList.seg[*scannedIt].row.end() - 1)->x == nodeIt->x && (segList.seg[*scannedIt].row.end() - 1)->y == nodeIt->y && (segList.seg[*scannedIt].row.end() - 1)->z == nodeIt->z)
					{
						connectedSegs.insert(*scannedIt);
						set<size_t> reversed;
						reversed.insert(*it);
						if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*scannedIt, reversed)).second) seg2SegsMap[*scannedIt].insert(*it);
					}
				}
			}
		}
		if (!seg2SegsMap.insert(pair<size_t, set<size_t> >(*it, connectedSegs)).second)
		{
			for (set<size_t>::iterator otherSegIt = connectedSegs.begin(); otherSegIt != connectedSegs.end(); ++otherSegIt)
				seg2SegsMap[*it].insert(*otherSegIt);
		}
	}

	/*for (map<size_t, set<size_t> >::iterator seg2SegsIt = seg2SegsMap.begin(); seg2SegsIt != seg2SegsMap.end(); ++seg2SegsIt)
	{
	cout << seg2SegsIt->first << ":";
	for (set<size_t>::iterator it = seg2SegsIt->second.begin(); it != seg2SegsIt->second.end(); ++it)
	cout << *it << " ";

	cout << endl;
	}*/

	double segSize = segList.seg.size();

	cout << endl << "Starting loop detection.. " << endl;
	clock_t begin = clock();
	detectedLoopsSet.clear();
	finalizedLoopsSet.clear();
	nonLoopErrors.clear();
	for (map<size_t, set<size_t> >::iterator it = seg2SegsMap.begin(); it != seg2SegsMap.end(); ++it)
	{
		double progressBarValue = (double(it->first) / segSize) * 100;
		int progressBarValueInt = floor(progressBarValue);

		if (it->second.empty()) continue;
		else if (it->second.size() <= 2) continue;

		vector<size_t> loops2ThisSeg;
		loops2ThisSeg.clear();

		cout << "Starting segment: " << it->first << " ==> ";
		for (set<size_t>::iterator seg2SegsIt = seg2SegsMap[it->first].begin(); seg2SegsIt != seg2SegsMap[it->first].end(); ++seg2SegsIt)
			cout << *seg2SegsIt << " ";
		cout << endl;

		int loopCount = finalizedLoopsSet.size();

		rc_loopPathCheck(it->first, loops2ThisSeg);

		if (finalizedLoopsSet.size() - loopCount == 0) cout << " -- no loops detected with this starting seg." << endl;
		else cout << finalizedLoopsSet.size() - loopCount << " loops detected with seg " << it->first << endl << endl;
	}

	clock_t end = clock();
	float elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
	cout << "TIME ELAPSED: " << elapsed_secs << " SECS" << endl << endl;

	if (finalizedLoopsSet.empty()) return outputErroneousPoints;
	else
	{
		for (set<set<size_t> >::iterator loopIt = finalizedLoopsSet.begin(); loopIt != finalizedLoopsSet.end(); ++loopIt)
		{
			set<size_t> thisLoop = *loopIt;
			for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
			{
				//cout << *it << " ";
				for (vector<V_NeuronSWC_unit>::iterator unitIt = segList.seg[*it].row.begin(); unitIt != segList.seg[*it].row.end(); ++unitIt)
				{
					unitIt->type = 15;				
					
					if (unitIt->parent == -1)
					{
						NeuronSWC problematicNode;
						problematicNode.x = unitIt->x;
						problematicNode.y = unitIt->y;
						problematicNode.z = unitIt->z;
						problematicNode.type = 15;
						problematicNode.parent = unitIt->parent;
						problematicNode.n = unitIt->n;
						outputErroneousPoints.push_back(problematicNode);
					}
				}
			}
			//cout << endl << endl;
		}
		cout << "LOOPS NUMBER (set): " << finalizedLoopsSet.size() << endl << endl;

		if (nonLoopErrors.empty())
		{
			for (set<set<size_t> >::iterator loopIt = nonLoopErrors.begin(); loopIt != nonLoopErrors.end(); ++loopIt)
			{
				set<size_t> thisLoop = *loopIt;
				for (set<size_t>::iterator it = thisLoop.begin(); it != thisLoop.end(); ++it)
				{
					//cout << *it << " ";
					for (vector<V_NeuronSWC_unit>::iterator unitIt = segList.seg[*it].row.begin(); unitIt != segList.seg[*it].row.end(); ++unitIt)
					{
						unitIt->type = 20;

						if (unitIt->parent == -1)
						{
							NeuronSWC problematicNode;
							problematicNode.x = unitIt->x;
							problematicNode.y = unitIt->y;
							problematicNode.z = unitIt->z;
							problematicNode.type = 20;
							problematicNode.parent = unitIt->parent;
							problematicNode.n = unitIt->n;
							outputErroneousPoints.push_back(problematicNode);
						}
					}
				}
				//cout << endl << endl;
			}
		}
		//cout << "non LOOPS ERROR NUMBER (set): " << thisRenderer->nonLoopErrors.size() << endl << endl;
	}

	return outputErroneousPoints;
}

void rc_loopPathCheck(size_t inputSegID, vector<size_t> curPathWalk)
{
	if (seg2SegsMap[inputSegID].size() < 2) return;

	//cout << "  input seg num: " << inputSegID << " ";
	curPathWalk.push_back(inputSegID);
	/*for (vector<size_t>::iterator curPathIt = curPathWalk.begin(); curPathIt != curPathWalk.end(); ++curPathIt)
	cout << *curPathIt << " ";
	cout << endl << endl;*/

	for (set<size_t>::iterator it = seg2SegsMap[inputSegID].begin(); it != seg2SegsMap[inputSegID].end(); ++it)
	{
		if (curPathWalk.size() >= 2 && *it == *(curPathWalk.end() - 2))
		{
			V_NeuronSWC_unit headUnit = *(segList.seg[*it].row.end() - 1);
			V_NeuronSWC_unit tailUnit = *segList.seg[*it].row.begin();

			bool headCheck = false, tailCheck = false;
			for (vector<V_NeuronSWC_unit>::iterator it = segList.seg[*(curPathWalk.end() - 1)].row.begin(); it != segList.seg[*(curPathWalk.end() - 1)].row.end(); ++it)
			{
				if (it->x == headUnit.x && it->y == headUnit.y && it->z == headUnit.z) headCheck = true;
				if (it->x == tailUnit.x && it->y == tailUnit.y && it->z == tailUnit.z) tailCheck = true;
			}

			if (headCheck == true && tailCheck == true)
			{
				set<size_t> detectedLoopPathSet;
				detectedLoopPathSet.clear();
				for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
					detectedLoopPathSet.insert(*loopIt);

				if (detectedLoopsSet.insert(detectedLoopPathSet).second)
				{
					nonLoopErrors.insert(detectedLoopPathSet);
					continue;
				}
				else return;
			}
			else continue;
		}

		if (find(curPathWalk.begin(), curPathWalk.end(), *it) == curPathWalk.end())
		{
			rc_loopPathCheck(*it, curPathWalk);
		}
		else
		{
			// a loop is found
			set<size_t> detectedLoopPathSet;
			detectedLoopPathSet.clear();
			for (vector<size_t>::iterator loopIt = find(curPathWalk.begin(), curPathWalk.end(), *it); loopIt != curPathWalk.end(); ++loopIt)
				detectedLoopPathSet.insert(*loopIt);

			if (detectedLoopsSet.insert(detectedLoopPathSet).second)
			{
				// pusedoloop by fork intersection check
				cout << "pusedoloop check.." << endl;

				if (*(curPathWalk.end() - 3) == *it)
				{
					if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 2)].end())
					{
						vector<V_NeuronSWC_unit> forkCheck;
						forkCheck.push_back(*(segList.seg[*(curPathWalk.end() - 1)].row.end() - 1));
						forkCheck.push_back(*segList.seg[*(curPathWalk.end() - 1)].row.begin());
						forkCheck.push_back(*(segList.seg[*(curPathWalk.end() - 2)].row.end() - 1));
						forkCheck.push_back(*segList.seg[*(curPathWalk.end() - 2)].row.begin());

						int headConnectedCount = 0;
						for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
						{
							if (checkIt->x == (segList.seg[*it].row.end() - 1)->x && checkIt->y == (segList.seg[*it].row.end() - 1)->y && checkIt->z == (segList.seg[*it].row.end() - 1)->z)
								++headConnectedCount;
						}

						int tailConnectedCount = 0;
						for (vector<V_NeuronSWC_unit>::iterator checkIt = forkCheck.begin(); checkIt != forkCheck.end(); ++checkIt)
						{
							if (checkIt->x == segList.seg[*it].row.begin()->x && checkIt->y == segList.seg[*it].row.begin()->y && checkIt->z == segList.seg[*it].row.begin()->z)
								++tailConnectedCount;
						}

						if (!(headConnectedCount == 1 && tailConnectedCount == 1))
						{
							cout << "  -> 3 seg intersection detected, exluded from loop candidates. (" << *it << ") ";
							for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
								cout << *thisLoopIt << " ";
							cout << endl;
							continue;
						}
						else
						{
							finalizedLoopsSet.insert(detectedLoopPathSet);
							cout << "  Loop from 3 way detected ----> (" << *it << ") ";
							for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
								cout << *thisLoopIt << " ";
							cout << endl;
							return;
						}
					}
				}
				else if (curPathWalk.size() == 4)
				{
					if ((*curPathWalk.end() - 4) == *it)
					{
						if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
							seg2SegsMap[*(curPathWalk.end() - 3)].find(*it) != seg2SegsMap[*(curPathWalk.end() - 3)].end())
						{
							if (seg2SegsMap[*(curPathWalk.end() - 2)].find(*(curPathWalk.end() - 1)) != seg2SegsMap[*(curPathWalk.end() - 2)].end() &&
								seg2SegsMap[*(curPathWalk.end() - 3)].find(*(curPathWalk.end() - 2)) != seg2SegsMap[*(curPathWalk.end() - 3)].end() &&
								seg2SegsMap[*(curPathWalk.end() - 4)].find(*(curPathWalk.end() - 3)) != seg2SegsMap[*(curPathWalk.end() - 4)].end())
							{
								cout << "  -> 4 seg intersection detected, exluded from loop candidates. (" << *it << ")" << endl;
								continue;
							}
						}
					}
				}
				else
				{
					finalizedLoopsSet.insert(detectedLoopPathSet);
					cout << "  Loop detected ----> (" << *it << ") ";
					for (set<size_t>::iterator thisLoopIt = detectedLoopPathSet.begin(); thisLoopIt != detectedLoopPathSet.end(); ++thisLoopIt)
						cout << *thisLoopIt << " ";
					cout << endl;
				}
			}
			else return;
		}
	}

	curPathWalk.pop_back();
	//cout << endl;
}