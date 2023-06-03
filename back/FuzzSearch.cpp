#include"FuzzSearch.h"


namespace fuzz {

	const int MAX_THREAD = 256;//最大线程数
	vector<string>* titles;//全部标题
	vector<string>* answers;//匹配的标题
	string target = "";//要找的内容
	bool inited = 0;//记录是否开启模糊搜索
	//内存中保存所有的文章标题信息
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
	//寻找目标的函数，3个参数分别为开始索引，结束索引，线程id
	void finder(int low,int up,int id) {

		for (int i = low; i < up; ++i) {
			string& buf = (*titles)[i];
			auto it = search(buf.begin(), buf.end(), boyer_moore_searcher(target.begin(), target.end()));
			if (it != buf.end()) {
				answers[id].emplace_back(buf);
			}//匹配则加入数组
		}
	}

	QStringList fuzz_search(string &input) {

		if (!inited) {
			load_data();
		}
		target = input;
		int offset = titles->size() / MAX_THREAD;//线程访问数组的偏移
		thread finderT[MAX_THREAD];
		//cout << "开始寻找" << target << endl;
		for (int i = 0; i < MAX_THREAD-1; ++i) {
			finderT[i] = thread(finder, i * offset, (i + 1) * offset, i);
		}
		//最后一个不一定等于偏移特殊处理，放在前一个线程中
		finderT[MAX_THREAD - 1]= thread(finder, (MAX_THREAD - 1) * offset, titles->size(), MAX_THREAD - 1);
		for (int i = 0; i < MAX_THREAD; ++i) {
			finderT[i].join();
		}
		//将内容传回mainwindow
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