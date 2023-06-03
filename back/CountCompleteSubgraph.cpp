#include"CountCompleteSubgraph.h"
#include "../cliquecount/LinkedList.h"
#include "../cliquecount/misc.h"
#include"../DataStructure.h"
namespace clique {
	unordered_map<string, unordered_set<string>>* author_authors;//作者合作关系
	unordered_map<string, int>* name_id;//名字编号
	vector<vector<int>>* G;//图的邻接表


	void load_authors() {
		//初始化数据
		author_authors = new unordered_map<string, unordered_set<string>>;
		name_id = new unordered_map<string, int>;
		int num = 0;
		//作者合作关系表的建立
		for (int i = 0; i < 4096; ++i) {
			fstream infile("DataBase\\Author_authors\\" + to_string(i) + ".txt", ios::in);
			string buf;
			string name;
			string temp;

			getline(infile, buf);

			while (!infile.eof()) {
				stringstream ss(buf);
				getline(ss, name, '$');
				(*name_id)[name] = num++;
				(*author_authors)[name];
				while (getline(ss, temp, '$')) {
					(*author_authors)[name].emplace(temp);
				}
				getline(infile, buf);
			}
			infile.close();
		}
	}
	//得到邻接表并调用
	void calc_adjlist() {
		//数据初始化
		int n = author_authors->size();
		G = new vector<vector<int>>(n);
		LinkedList** adjList = (LinkedList**)calloc(n, sizeof(LinkedList*));
		for (int i = 0; i < n; i++)
			adjList[i] = createLinkedList();

		//给名字取编号，并建立邻接链表
		for (auto& i : *author_authors) {
			int u = (*name_id)[i.first];
			for (auto& j : i.second) {
				(*G)[u].emplace_back((*name_id)[j]);
			}
			sort((*G)[u].begin(), (*G)[u].end());
		}
		//建立邻接链表
		for (int u = 0; u < n; ++u) {
			for (int v : (*G)[u]) {
				if (v <= u) continue;
				addLast(adjList[u], v);
				addLast(adjList[v], u);
			}
		}
		delete author_authors;
		delete name_id;
		delete G;
		//得到聚团分析结果
		runAndPrintStatsCliques(adjList, n);
		for (int i = 0; i < n; i++)
			destroyLinkedList(adjList[i]);
		free(adjList);
	}

	//主要功能函数
	void clique_analysis() {
		time_t start = time(NULL);

		load_authors();
		calc_adjlist();
		time_t end = time(NULL);

		DataStructure::window->set_cliqueButton(start, end);//mainwindow处理
	}

}