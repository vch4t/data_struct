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

#include<assert.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>

#include <vector>
#include "bignumber.h"

#include"misc.h"
#include"LinkedList.h"
#include"degeneracy_helper.h"

vector<vector<BigNumber>> nCr;

//�����������dp�������ҵ����������������ó�������ͼ
void populate_nCr()
{
    const int maxrow = 1500;
    const int maxcol = 800;
    nCr.resize(maxrow + 1);
    for(int row = 0; row < maxrow + 1; ++row)
    {
        nCr[row].resize(maxcol + 1);
        for (int col = 0; col < maxcol + 1; ++col)
        {
            if (row == 0 || col == 0 || row == col) nCr[row][col] = 1;
            else nCr[row][col] = nCr[row-1][col] + nCr[row-1][col-1];
        }
    }
}

//ɾ��������������
void destroyCliqueResults(LinkedList* cliques)
{
    Link* curr = cliques->head->next;
    while(!isTail(curr))
    {
        free(&(curr->data));
        curr = curr->next;
    }
    destroyLinkedList(cliques); 
}
//��Ҫ���ܽӿ�
void runAndPrintStatsCliques(LinkedList** adjListLinked, int n)
{
    populate_nCr();
    //��ʼ��
    fflush(stderr);
    int max_k = 0;
    int deg = 0, m = 0;
    FILE *fp;
    //vs�²�������fopen,ֻ��fopen_s�����������ļ�·��
#ifdef _MSC_VER
    fopen_s(&fp, "DataBase/clique.txt", "w");
#else
    fp = fopen("DataBase/clique.txt", "w");
#endif
    fflush(stdout);
    //�õ�NeighborListArray
    NeighborListArray** orderingArray = computeDegeneracyOrderArray(adjListLinked, n);
    fflush(stdout);
    //�õ��׵����ֵ����ʼ��
    for (int i=0; i<n; i++)
    {
        if (deg < orderingArray[i]->laterDegree) deg = orderingArray[i]->laterDegree;
        m += orderingArray[i]->laterDegree;
    }
    max_k = deg + 1;
    vector<BigNumber> cliqueCounts(max_k + 1);
    //���о������
    listAllCliquesDegeneracy_A(cliqueCounts, orderingArray, n, max_k);
    while (cliqueCounts[max_k] == 0) max_k--;
    //�����д���ļ�
    fprintf(fp, "%d\n", max_k);
    BigNumber totalCliques = 0;
    for (int i=1; i<=max_k; i++)
    {
        fprintf(fp, "%d %s\n", i, cliqueCounts[i].getString().c_str()); 
        totalCliques += cliqueCounts[i];
    }

    fprintf(fp, "%s\n", totalCliques.getString().c_str());
    fclose(fp);
    free(orderingArray);
    nCr.clear();
}

//�ҵ�ӵ�������ܽڵ��pivot���Լ���pivot�����ڵ�P�нڵ�����
int findBestPivotNonNeighborsDegeneracyCliques( int** pivotNonNeighbors, int* numNonNeighbors,
                                                int* vertexSets, int* vertexLookup,
                                                int** neighborsInP, int* numNeighbors,
                                                int , int beginP, int beginR)
{
    int pivot = -1;
    int maxIntersectionSize = -1;

    int j = beginP;
    //�ҵ�P������ӵ�������ܽڵ�Ľڵ�
    while(j<beginR)
    {
        int vertex = vertexSets[j];
        int numPotentialNeighbors = std::min(beginR - beginP, numNeighbors[vertex]); 

        int numNeighborsInP = 0;

        int k = 0;
        while(k<numPotentialNeighbors)
        {
            int neighbor = neighborsInP[vertex][k];
            int neighborLocation = vertexLookup[neighbor];

            if(neighborLocation >= beginP && neighborLocation < beginR)
            {
                numNeighborsInP++;
            }
            else
            {
                break;
            }

            k++;
        }

        if(numNeighborsInP > maxIntersectionSize)
        {
            pivot = vertex;
            maxIntersectionSize = numNeighborsInP;
        }

        j++;
    }

    //��ʼ������pivot���ڵ�����
    *pivotNonNeighbors = (int *)calloc(beginR-beginP, sizeof(int));
    memcpy(*pivotNonNeighbors, &vertexSets[beginP], (beginR-beginP)*sizeof(int));


    *numNonNeighbors = beginR-beginP;

    int numPivotNeighbors = std::min(beginR - beginP, numNeighbors[pivot]); 
  
    //���pivot��P�е��ھ�
    j = 0;
    while(j<numPivotNeighbors)
    {
        int neighbor = neighborsInP[pivot][j];
        int neighborLocation = vertexLookup[neighbor];

        if(neighborLocation >= beginP && neighborLocation < beginR)
        {
            (*pivotNonNeighbors)[neighborLocation-beginP] = -1;
        }
        else
        {
            break;
        }

        j++;
    }

    j = 0;
    //�õ�����pivot���ڵģ��������ڣ��б�ǵģ���ɾ��
    while(j<*numNonNeighbors)
    {
        int vertex = (*pivotNonNeighbors)[j];

        if(vertex == -1)
        {
            (*numNonNeighbors)--;
            (*pivotNonNeighbors)[j] = (*pivotNonNeighbors)[*numNonNeighbors];
            continue;
        }

        j++;
    }

    return pivot; 
}

//�������ü��ϣ�Ϊ�ݹ���׼��
void fillInPandXForRecursiveCallDegeneracyCliques( int vertex, int orderNumber,
                                                   int* vertexSets, int* vertexLookup, 
                                                   NeighborListArray** orderingArray,
                                                   int** neighborsInP, int* numNeighbors,
                                                   int* , int *, int *pBeginR, 
                                                   int* pNewBeginX, int* pNewBeginP, int *pNewBeginR)
{   
    //���ڵ����R����
    int vertexLocation = vertexLookup[vertex];

    (*pBeginR)--;
    vertexSets[vertexLocation] = vertexSets[*pBeginR];
    vertexLookup[vertexSets[*pBeginR]] = vertexLocation;
    vertexSets[*pBeginR] = vertex;
    vertexLookup[vertex] = *pBeginR;
    
    *pNewBeginR = *pBeginR;
    *pNewBeginP = *pBeginR;

    //���P����
    int j = 0;
    while(j<orderingArray[orderNumber]->laterDegree)
    {
        int neighbor = orderingArray[orderNumber]->later[j];
        int neighborLocation = vertexLookup[neighbor];

        (*pNewBeginP)--;

        vertexSets[neighborLocation] = vertexSets[*pNewBeginP];
        vertexLookup[vertexSets[*pNewBeginP]] = neighborLocation;
        vertexSets[*pNewBeginP] = neighbor;
        vertexLookup[neighbor] = *pNewBeginP;

        j++; 
    }

    *pNewBeginX = *pNewBeginP;


    j = *pNewBeginP;
    //��ʼ��P�����нڵ����Ϣ
    while(j<*pNewBeginR)
    {
        int vertexInP = vertexSets[j];
        numNeighbors[vertexInP] = 0;
        free(neighborsInP[vertexInP]);
        neighborsInP[vertexInP]= (int *)calloc( std::min( *pNewBeginR-*pNewBeginP, 
                                             orderingArray[vertexInP]->laterDegree 
                                           + orderingArray[vertexInP]->earlierDegree), sizeof(int));

        j++;
    }

    j = *pNewBeginP;
    //����P�����нڵ���ھ�
    while(j<*pNewBeginR)
    {
        int vertexInP = vertexSets[j];

        int k = 0;
        while(k<orderingArray[vertexInP]->laterDegree)
        {
            int laterNeighbor = orderingArray[vertexInP]->later[k];
            int laterNeighborLocation = vertexLookup[laterNeighbor];

            if(laterNeighborLocation >= *pNewBeginP && laterNeighborLocation < *pNewBeginR)
            {
                neighborsInP[vertexInP][numNeighbors[vertexInP]] = laterNeighbor;
                numNeighbors[vertexInP]++;
                neighborsInP[laterNeighbor][numNeighbors[laterNeighbor]] = vertexInP;
                numNeighbors[laterNeighbor]++;
            }

            k++;
        }

        j++;
    }
}


//�ƶ��ڵ㵽R���ó��¼���
void moveToRDegeneracyCliques( int vertex, 
                               int* vertexSets, int* vertexLookup, 
                               int** neighborsInP, int* numNeighbors,
                               int* , int *pBeginP, int *pBeginR, 
                               int* pNewBeginX, int* pNewBeginP, int *pNewBeginR)
{
    //�ƶ��ڵ㵽R
    int vertexLocation = vertexLookup[vertex];
    
    (*pBeginR)--;
    vertexSets[vertexLocation] = vertexSets[*pBeginR];
    vertexLookup[vertexSets[*pBeginR]] = vertexLocation;
    vertexSets[*pBeginR] = vertex;
    vertexLookup[vertex] = *pBeginR;

    //��ʼ����������λ��
    *pNewBeginX = *pBeginP;
    *pNewBeginP = *pBeginP;
    *pNewBeginR = *pBeginP;

    int sizeOfP = *pBeginR - *pBeginP;

    int j = (*pBeginP);
    //�����µ�R���ϣ��뵱ǰ�ڵ㲻���ڵ�
    while(j<(*pBeginR))
    {
        int neighbor = vertexSets[j];
        int neighborLocation = j;

        int numPotentialNeighbors = std::min(sizeOfP, numNeighbors[neighbor]); 
        int k = 0;
        while(k<numPotentialNeighbors)
        {
            if(neighborsInP[neighbor][k] == vertex)
            {
                vertexSets[neighborLocation] = vertexSets[(*pNewBeginR)];
                vertexLookup[vertexSets[(*pNewBeginR)]] = neighborLocation;
                vertexSets[(*pNewBeginR)] = neighbor;
                vertexLookup[neighbor] = (*pNewBeginR);
                (*pNewBeginR)++;
            }

            k++;
        }

        j++;
    }

    j = (*pNewBeginP);
    //����P�нڵ�Ŀ����ڽӽڵ�
    while(j < *pNewBeginR)
    {
        int thisVertex = vertexSets[j];

        int numPotentialNeighbors = std::min(sizeOfP, numNeighbors[thisVertex]); 

        int numNeighborsInP = 0;

        int k = 0;
        while(k < numPotentialNeighbors)
        {
            int neighbor = neighborsInP[thisVertex][k];
            int neighborLocation = vertexLookup[neighbor];
            if(neighborLocation >= *pNewBeginP && neighborLocation < *pNewBeginR)
            {
                neighborsInP[thisVertex][k] = neighborsInP[thisVertex][numNeighborsInP];
                neighborsInP[thisVertex][numNeighborsInP] = neighbor;
                numNeighborsInP++;
            }
            k++;
        }

        j++;
    }
}


//���ڵ��R�ƶ���X
void moveFromRToXDegeneracyCliques( int vertex, 
                                    int* vertexSets, int* vertexLookup, 
                                    int* , int* pBeginP, int* pBeginR )
{
    int vertexLocation = vertexLookup[vertex];

    //����beginP and beginR������
    vertexSets[vertexLocation] = vertexSets[*pBeginP];
    vertexLookup[vertexSets[*pBeginP]] = vertexLocation;
    vertexSets[*pBeginP] = vertex;
    vertexLookup[vertex] = *pBeginP;
    //X������ߣ���Ҫ����һλ
    *pBeginP = *pBeginP + 1;
    *pBeginR = *pBeginR + 1;
}
