#pragma once

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
#include<stdlib.h>

#include"misc.h"
#include"LinkedList.h"

//链表形式的邻接矩阵
struct NeighborList
{
    int vertex; //指向这个邻居链表的节点
    LinkedList* earlier; //前面已经加入序列的节点的链表
    LinkedList* later; //未加入的序列的节点的链表
    int orderNumber; //节点所在的位置
};

typedef struct NeighborList NeighborList;


//数组形式的邻接矩阵，内容如上
struct NeighborListArray
{
    int vertex; 
    int* earlier; 
    int earlierDegree; 
    int* later; 
    int laterDegree; 
    int orderNumber;
};

typedef struct NeighborListArray NeighborListArray;

NeighborListArray** computeDegeneracyOrderArray(LinkedList** list, int size);
