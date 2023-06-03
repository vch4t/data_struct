#include"FuzzSearch.h"


namespace fuzz {

	const int MAX_THREAD = 256;//����߳���
	vector<string>* titles;//ȫ������
	vector<string>* answers;//ƥ��ı���
	string target = "";//Ҫ�ҵ�����
	bool inited = 0;//��¼�Ƿ���ģ������
	//�ڴ��б������е����±�����Ϣ
	void load_data() {
		titles = new vector<string>;
		answers = new vector<string>[MAX_THREAD];
		for (int i = 1935; i < 2024; ++i) {
			fstream infile("DataBase\\Year_title\\" + to_string(i) + ".txt", ios::in);
			if (!infile) continue;
			string buf;
			getline(infile, buf);
			while (!infile.eof()) {
				titles->emplace_back(buf);
				getline(infile, buf);
			}
		}
		inited = 1;
	}
	//Ѱ��Ŀ��ĺ�����3�������ֱ�Ϊ��ʼ�����������������߳�id
	void finder(int low,int up,int id) {

		for (int i = low; i < up; ++i) {
			string& buf = (*titles)[i];
			auto it = search(buf.begin(), buf.end(), boyer_moore_searcher(target.begin(), target.end()));
			if (it != buf.end()) {
				answers[id].emplace_back(buf);
			}//ƥ�����������
		}
	}

	QStringList fuzz_search(string &input) {

		if (!inited) {
			load_data();
		}
		target = input;
		int offset = titles->size() / MAX_THREAD;//�̷߳��������ƫ��
		thread finderT[MAX_THREAD];
		//cout << "��ʼѰ��" << target << endl;
		for (int i = 0; i < MAX_THREAD-1; ++i) {
			finderT[i] = thread(finder, i * offset, (i + 1) * offset, i);
		}
		//���һ����һ������ƫ�����⴦������ǰһ���߳���
		finderT[MAX_THREAD - 1]= thread(finder, (MAX_THREAD - 1) * offset, titles->size(), MAX_THREAD - 1);
		for (int i = 0; i < MAX_THREAD; ++i) {
			finderT[i].join();
		}
		//�����ݴ���mainwindow
		QStringList mylist;
		for (int i = 0; i < MAX_THREAD; ++i) {
			for (auto& c : answers[i]) {
				mylist.push_back(QString::fromStdString(c));
			}
		}
		for (int i = 0; i < MAX_THREAD; ++i) {
			answers[i].clear();
		}
		return mylist;
	}
}