#include"CountCompleteSubgraph.h"
#include "../cliquecount/LinkedList.h"
#include "../cliquecount/misc.h"
#include"../DataStructure.h"
namespace clique {
	unordered_map<string, unordered_set<string>>* author_authors;//���ߺ�����ϵ
	unordered_map<string, int>* name_id;//���ֱ��
	vector<vector<int>>* G;//ͼ���ڽӱ�


	void load_authors() {
		//��ʼ������
		author_authors = new unordered_map<string, unordered_set<string>>;
		name_id = new unordered_map<string, int>;
		int num = 0;
		//���ߺ�����ϵ��Ľ���
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
	//�õ��ڽӱ�����
	void calc_adjlist() {
		//���ݳ�ʼ��
		int n = author_authors->size();
		G = new vector<vector<int>>(n);
		LinkedList** adjList = (LinkedList**)calloc(n, sizeof(LinkedList*));
		for (int i = 0; i < n; i++)
			adjList[i] = createLinkedList();

		//������ȡ��ţ��������ڽ�����
		for (auto& i : *author_authors) {
			int u = (*name_id)[i.first];
			for (auto& j : i.second) {
				(*G)[u].emplace_back((*name_id)[j]);
			}
			sort((*G)[u].begin(), (*G)[u].end());
		}
		//�����ڽ�����
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
		//�õ����ŷ������
		runAndPrintStatsCliques(adjList, n);
		for (int i = 0; i < n; i++)
			destroyLinkedList(adjList[i]);
		free(adjList);
	}

	//��Ҫ���ܺ���
	void clique_analysis() {
		time_t start = time(NULL);

		load_authors();
		calc_adjlist();
		time_t end = time(NULL);

		DataStructure::window->set_cliqueButton(start, end);//mainwindow����
	}

}