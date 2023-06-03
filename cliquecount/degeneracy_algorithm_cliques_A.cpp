/* 
    This file contains the algorithm for listing all cliques
    according to the algorithm of Jain et al. specified in 
    "The power of pivoting for exact clique counting." (WSDM 2020).

    This code is a modified version of the code of quick-cliques-1.0 library for counting 
    maximal cliques by Darren Strash (first name DOT last name AT gmail DOT com).

    Original author: Darren Strash (first name DOT last name AT gmail DOT com)

    Copyright (c) 2011 Darren Strash. This code is released under the GNU Public License (GPL) 3.0.

    Modifications Copyright (c) 2020 Shweta Jain
    
    This program is free software: you can redistribute it and/or modify 
    it under the terms of the GNU General Public License as published by 
    the Free Software Foundation, either version 3 of the License, or 
    (at your option) any later version. 
 
    This program is distributed in the hope that it will be useful, 
    but WITHOUT ANY WARRANTY; without even the implied warranty of 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
    GNU General Public License for more details. 
 
    You should have received a copy of the GNU General Public License 
    along with this program.  If not, see <http://www.gnu.org/licenses/> 
*/

#include<limits.h>
#include<assert.h>
#include<stdio.h>
#include<stdlib.h>



#include"misc.h"
#include"LinkedList.h"
#include"degeneracy_helper.h"
#include"degeneracy_algorithm_cliques_A.h"
using std::vector;

extern vector<vector<BigNumber>> nCr;

void listAllCliquesDegeneracy_A(vector<BigNumber> &cliqueCounts, NeighborListArray** orderingArray, 
                                      int size, int max_k)
{
    // 节点集合保存在如下结构的数组
    // |--X--|--P--|
    int* vertexSets = (int *)calloc(size, sizeof(int));

    // 节点i保存在vertexSets[vertexLookup[i]]
    int* vertexLookup = (int *)calloc(size, sizeof(int));
    //记录邻居（或P集合）的数组
    int** neighborsInP = (int **)calloc(size, sizeof(int*));
    int* numNeighbors = (int *)calloc(size, sizeof(int));
    int i = 0;
    //初始化
    while(i<size)
    {
        vertexLookup[i] = i;
        vertexSets[i] = i;
        neighborsInP[i] = (int *)calloc(1, sizeof(int));
        numNeighbors[i] = 1;
        i++;
    }
    //Bron-Kerbosch算法的3个集合的起始索引
    int beginX = 0;
    int beginP = 0;
    int beginR = size;
    // 对每个节点
    for(i=0;i<size;i++)
    {
        int vertex = (int)orderingArray[i]->vertex;
        
        int newBeginX, newBeginP, newBeginR;

        //设置算法的3个集合，以及P集合中的邻接信息
        fillInPandXForRecursiveCallDegeneracyCliques( i, vertex, 
                                               vertexSets, vertexLookup, 
                                               orderingArray,
                                               neighborsInP, numNeighbors,
                                               &beginX, &beginP, &beginR, 
                                               &newBeginX, &newBeginP, &newBeginR);

        //rsize代表节点个数，drop代表取节点的层数（作用是避免重复计数，通过取子图然后累加间接得到各阶数量）
        int drop = 0;
        int rsize = 1;

        listAllCliquesDegeneracyRecursive_A(cliqueCounts, 
                                                  vertexSets, vertexLookup,
                                                  neighborsInP, numNeighbors,
                                                  newBeginX, newBeginP, newBeginR, max_k, rsize, drop); 


        beginR = beginR + 1;
    }

    cliqueCounts[0] = 1;

    //回收内存
    free(vertexSets);
    free(vertexLookup);

    for(i = 0; i<size; i++)
    {
        free(neighborsInP[i]);
        free(orderingArray[i]->later);
        free(orderingArray[i]->earlier);
        free(orderingArray[i]);
    }

    free(neighborsInP);
    free(numNeighbors);

    return;
}



//找出最大的完全子图并计算各阶聚类
void listAllCliquesDegeneracyRecursive_A(vector<BigNumber> &cliqueCounts,
                                               int* vertexSets, int* vertexLookup,
                                               int** neighborsInP, int* numNeighbors,
                                               int beginX, int beginP, int beginR, int max_k, 
                                               int rsize, int drop)
{
    //递归终止，计算各阶聚类的数量,计算的原理也是利用组合数的性质,即dp
    if ((beginP >= beginR) || (rsize-drop > max_k))
    {
        for (int i=drop; (i>=0) && (rsize-i <= max_k); i--) 
        {
            int k = rsize - i;
            cliqueCounts[k] += nCr[drop][i];
        }
       
        return;
    }
    
    int* myCandidatesToIterateThrough;
    int numCandidatesToIterateThrough = 0;

    //得到pivot和不与pivot相邻的候选节点
    int pivot = findBestPivotNonNeighborsDegeneracyCliques( &myCandidatesToIterateThrough,
                                         &numCandidatesToIterateThrough,
                                         vertexSets, vertexLookup,
                                         neighborsInP, numNeighbors,
                                         beginX, beginP, beginR);

    //找最大完全子图
    if(numCandidatesToIterateThrough != 0)
    {
        int iterator = 0;
        while(iterator < numCandidatesToIterateThrough)
        {
            //要加入的节点
            int vertex = myCandidatesToIterateThrough[iterator];

            int newBeginX, newBeginP, newBeginR;

            //添加节点到R，并更新相应的集合及内容
            moveToRDegeneracyCliques( vertex, 
                               vertexSets, vertexLookup, 
                               neighborsInP, numNeighbors,
                               &beginX, &beginP, &beginR, 
                               &newBeginX, &newBeginP, &newBeginR);


            
            //递归计算最大的聚类，为了减少重复计数，用pivot作区分，drop+1
            if (vertex == pivot)
                listAllCliquesDegeneracyRecursive_A(cliqueCounts,
                                                      vertexSets, vertexLookup,
                                                      neighborsInP, numNeighbors,
                                                      newBeginX, newBeginP, newBeginR, max_k, rsize+1, drop+1);
            else
                listAllCliquesDegeneracyRecursive_A(cliqueCounts,
                                                      vertexSets, vertexLookup,
                                                      neighborsInP, numNeighbors,
                                                      newBeginX, newBeginP, newBeginR, max_k, rsize+1, drop);

            //将节点移动到X中
            moveFromRToXDegeneracyCliques( vertex, 
                                    vertexSets, vertexLookup,
                                    &beginX, &beginP, &beginR );

            iterator++;
        }

        //将X中的节点放回P集合中，以寻找更大的聚类
        iterator = 0;
        while(iterator < numCandidatesToIterateThrough)
        {
            int vertex = myCandidatesToIterateThrough[iterator];
            int vertexLocation = vertexLookup[vertex];

            beginP--;
            vertexSets[vertexLocation] = vertexSets[beginP];
            vertexSets[beginP] = vertex;
            vertexLookup[vertex] = beginP;
            vertexLookup[vertexSets[vertexLocation]] = vertexLocation;

            iterator++;
        }
    }

    //回收内存
    free(myCandidatesToIterateThrough);
    
    return;
}
