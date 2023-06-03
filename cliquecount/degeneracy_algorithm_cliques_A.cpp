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
    // �ڵ㼯�ϱ��������½ṹ������
    // |--X--|--P--|
    int* vertexSets = (int *)calloc(size, sizeof(int));

    // �ڵ�i������vertexSets[vertexLookup[i]]
    int* vertexLookup = (int *)calloc(size, sizeof(int));
    //��¼�ھӣ���P���ϣ�������
    int** neighborsInP = (int **)calloc(size, sizeof(int*));
    int* numNeighbors = (int *)calloc(size, sizeof(int));
    int i = 0;
    //��ʼ��
    while(i<size)
    {
        vertexLookup[i] = i;
        vertexSets[i] = i;
        neighborsInP[i] = (int *)calloc(1, sizeof(int));
        numNeighbors[i] = 1;
        i++;
    }
    //Bron-Kerbosch�㷨��3�����ϵ���ʼ����
    int beginX = 0;
    int beginP = 0;
    int beginR = size;
    // ��ÿ���ڵ�
    for(i=0;i<size;i++)
    {
        int vertex = (int)orderingArray[i]->vertex;
        
        int newBeginX, newBeginP, newBeginR;

        //�����㷨��3�����ϣ��Լ�P�����е��ڽ���Ϣ
        fillInPandXForRecursiveCallDegeneracyCliques( i, vertex, 
                                               vertexSets, vertexLookup, 
                                               orderingArray,
                                               neighborsInP, numNeighbors,
                                               &beginX, &beginP, &beginR, 
                                               &newBeginX, &newBeginP, &newBeginR);

        //rsize����ڵ������drop����ȡ�ڵ�Ĳ����������Ǳ����ظ�������ͨ��ȡ��ͼȻ���ۼӼ�ӵõ�����������
        int drop = 0;
        int rsize = 1;

        listAllCliquesDegeneracyRecursive_A(cliqueCounts, 
                                                  vertexSets, vertexLookup,
                                                  neighborsInP, numNeighbors,
                                                  newBeginX, newBeginP, newBeginR, max_k, rsize, drop); 


        beginR = beginR + 1;
    }

    cliqueCounts[0] = 1;

    //�����ڴ�
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



//�ҳ�������ȫ��ͼ��������׾���
void listAllCliquesDegeneracyRecursive_A(vector<BigNumber> &cliqueCounts,
                                               int* vertexSets, int* vertexLookup,
                                               int** neighborsInP, int* numNeighbors,
                                               int beginX, int beginP, int beginR, int max_k, 
                                               int rsize, int drop)
{
    //�ݹ���ֹ��������׾��������,�����ԭ��Ҳ�����������������,��dp
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

    //�õ�pivot�Ͳ���pivot���ڵĺ�ѡ�ڵ�
    int pivot = findBestPivotNonNeighborsDegeneracyCliques( &myCandidatesToIterateThrough,
                                         &numCandidatesToIterateThrough,
                                         vertexSets, vertexLookup,
                                         neighborsInP, numNeighbors,
                                         beginX, beginP, beginR);

    //�������ȫ��ͼ
    if(numCandidatesToIterateThrough != 0)
    {
        int iterator = 0;
        while(iterator < numCandidatesToIterateThrough)
        {
            //Ҫ����Ľڵ�
            int vertex = myCandidatesToIterateThrough[iterator];

            int newBeginX, newBeginP, newBeginR;

            //��ӽڵ㵽R����������Ӧ�ļ��ϼ�����
            moveToRDegeneracyCliques( vertex, 
                               vertexSets, vertexLookup, 
                               neighborsInP, numNeighbors,
                               &beginX, &beginP, &beginR, 
                               &newBeginX, &newBeginP, &newBeginR);


            
            //�ݹ�������ľ��࣬Ϊ�˼����ظ���������pivot�����֣�drop+1
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

            //���ڵ��ƶ���X��
            moveFromRToXDegeneracyCliques( vertex, 
                                    vertexSets, vertexLookup,
                                    &beginX, &beginP, &beginR );

            iterator++;
        }

        //��X�еĽڵ�Ż�P�����У���Ѱ�Ҹ���ľ���
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

    //�����ڴ�
    free(myCandidatesToIterateThrough);
    
    return;
}
