#include"utils.h"
#include "../DataStructure.h"
#include<QMessageBox>



namespace DataInit {

	//开始
	const int MAX_HASH = 4096;//哈希的模，也是文件分成部分的数量
	const int MAX_BUFSZ = 8192 * 4;//临时buffer的大小
	const int MIN_YEAR = 1900;//最低年份
	//用c++实现的一个线程barrier
	class Barrier {
		mutex m_mutex;
		condition_variable cv;
		int m_cnt;
		int m_thread;
		bool m_release;
	public:
		Barrier(int num) :m_cnt(0), m_thread(num), m_release(0) {}
		void waiting() {
			unique_lock<mutex> lk(m_mutex);
			if (m_cnt == 0) {
				m_release = 0;
			}
			m_cnt++;
			if (m_cnt == m_thread) {
				m_cnt = 0;
				m_release = 1;
				cv.notify_all();
			}
			else {
				cv.wait(lk, [&] {return m_release == 1; });
				//cv.wait(lk);
			}
		}
	};
	Barrier barrier(5);//对barrier初始化，5是因为有5个同时处理的线程
	string* buffer[2];//两个临时buffer
	int curr_idx = 0;//当前在写入的buffer编号
	bool read_end = 0;//是否读取xml文件结束
	//记录文章名字以及所处xml文件的位置
	class ArticlePos {
	public:
		string m_title;
		size_t m_pos;
		ArticlePos(string title, size_t pos) :m_title(title), m_pos(pos) {}
	};
	//记录作者们以及文章标题
	class AuthorsTitle {
	public:
		vector<string> m_name;
		string m_title;
		AuthorsTitle(vector<string> name, string title) :m_name(name), m_title(title) {}
	};

	vector<ArticlePos*>* all_article;//记录所有文章及其位置
	map<string, int>* author_rank;//记录所有作者及其文章数
	vector<string>* year_title;//记录所在年的所有文章
	vector<AuthorsTitle>* authors_title;//记录所有（作者们，文章）
	map<string, vector<string>>* author_titles;//记录（作者，其所写文章）
	map<string, set<string>>* author_authors;//记录（作者，合作者）

	//一行行读取文件的函数
	void reader(ifstream& infile) {
		while (!infile.eof()) {
			//清空buffer
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				buffer[curr_idx][i].clear();
			}
			//写入buffer
			for (int i = 0; i < MAX_BUFSZ && !infile.eof(); ++i) {
				getline(infile, buffer[curr_idx][i]);
			}
			curr_idx = 1 - curr_idx;//切换要写入的buffer
			barrier.waiting();
		}
		read_end = 1;//读完文件了，进行标记
	}
	//得到文章及其位置的函数
	void get_article_pos() {
		size_t curr_pos = 0, pos = 0;//保存相应的位置，curr_pos是当前行的位置，pos是一篇文章的初始位置，解决在未得到完整一组数据却切换buffer
		while (1) {
			int idx = 1 - curr_idx;;//要使用的buffer的编号，与正在读取的编号不同
			string* m_head = buffer[idx];//保存一个指针方便阅读
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isStart(m_head[i])) {
					pos = curr_pos;//是一篇文章的开头，保存位置
				}
				else if (isTitle(m_head[i])) {//是文章标题所在行
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					string title = m_head[i].substr(l + 1, r - l - 1);//得到文章标题
					all_article[ELFhash(title)].emplace_back(new ArticlePos(title, pos));//保存有价值的文章
				}
				curr_pos += m_head[i].length() + 1;//当前行位置更新
			}
			if (read_end)return;//所有信息处理完了，结束了
			barrier.waiting();//barrier，等待其他线程
		}
	}
	//得到作者排行的函数
	void get_author_rank() {
		while (1) {
			int idx = 1 - curr_idx;
			string* m_head = buffer[idx];
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isAuthor(m_head[i])) {
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					string name = m_head[i].substr(l + 1, r - l - 1);
					(*author_rank)[name]++;//发现一次作者名等同于有一篇文章，所以文章数量+1
				}
			}
			if (read_end)return;
			barrier.waiting();
		}
		//未解释内容如get_article_pos内所示
	}
	//得到每一年的所有文章
	void get_year_title() {
		string title;//保存文章标题，解决在未得到完整一组数据却切换buffer
		while (1) {
			int idx = 1 - curr_idx;
			string* m_head = buffer[idx];
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isTitle(m_head[i])) {//找到标题并记录
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					title = m_head[i].substr(l + 1, r - l - 1);
				}
				if (m_head[i].find("<year>") != string::npos) {
					size_t ly = m_head[i].find("<year>");
					size_t ry = m_head[i].find("</year>");
					string year = m_head[i].substr(ly + 6, ry - ly - 5 - 1);
					year_title[stoi(year) - MIN_YEAR].emplace_back(title);//记录当前年有价值的文章
				}

			}
			if (read_end)return;
			barrier.waiting();
		}
		//未解释内容如get_article_pos内所示
	}
	//得到一篇文章及其所有作者
	void get_authors_title() {
		vector<string> author;//保存作者们，解决在未得到完整一组数据却切换buffer
		while (1) {
			int idx = 1 - curr_idx;
			string* m_head = buffer[idx];
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isAuthor(m_head[i])) {
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					string name = m_head[i].substr(l + 1, r - l - 1);
					author.emplace_back(name);
				}
				else if (isTitle(m_head[i])) {
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					string title = m_head[i].substr(l + 1, r - l - 1);
					authors_title[ELFhash(title)].emplace_back(AuthorsTitle(author, title));
					author.clear();//记得清空
				}
			}
			if (read_end)return;
			barrier.waiting();
			//未解释内容如get_article_pos内所示
		}
	}
	//得到作者及其合作者的函数 
	void get_author_authors() {
		for (int i = 0; i < MAX_HASH; ++i) {
			for (auto& title : authors_title[i]) {
				int size = title.m_name.size();
				vector<string>& temp = title.m_name;
				if (size == 1) {
					size_t idx = ELFhash(temp[0]);
					author_authors[idx][temp[0]];
					//当没有合作者时记录
				}
				for (int j = 0; j < size; ++j) {
					for (int k = 0; k < size; ++k) {
						if (j == k) {
							continue;
						}
						size_t idx = ELFhash(temp[j]);
						author_authors[idx][temp[j]].insert(temp[k]);
						//一篇文章两两互为合作者，记录
					}
				}
			}

		}

	}
	//保存（作者，文章）的函数
	void write_authorTitles() {
		//得到（作者，文章）信息
		for (int i = 0; i < MAX_HASH; ++i) {
			for (int j = 0; j < authors_title[i].size(); ++j) {
				vector<string>& c = authors_title[i][j].m_name;
				string title = authors_title[i][j].m_title;
				for (auto& name : c) {
					author_titles[ELFhash(name)][name].emplace_back(title);
				}
			}
		}
		//按文件顺序保存
		for (int i = 0; i < MAX_HASH; ++i) {
			//打开文件
			stringstream ss;
			string num;
			string path = "DataBase\\Author_titles\\";
			ss << i;
			ss >> num;
			path += num;
			path += ".txt";
			fstream outfile(path.c_str(), ios::out);
			for (auto& c : author_titles[i]) {
				outfile << c.first;
				vector<string>& temp = c.second;
				temp.shrink_to_fit();//内存控制
				for (auto& d : temp) {
					outfile << '$' << d;//当作分隔符
				}
				outfile << endl;
				vector<string>().swap(temp);//清空temp所指向内存
			}
			outfile.close();
		}
		delete[] author_titles;
	}
	//保存（文章，作者）的函数
	void writeTA() {
		thread aT(get_author_authors);//同时执行，方便一起回收内存
		for (int i = 0; i < MAX_HASH; ++i) {
			stringstream ss;
			string num;
			string path = "DataBase\\Title_authors\\";
			ss << i;
			ss >> num;
			path += num;
			path += ".txt";
			fstream outfile(path.c_str(), ios::out);
			for (auto& c : authors_title[i]) {
				outfile << c.m_title;
				const vector<string>& temp = c.m_name;
				for (auto& d : temp) {
					outfile << '$' << d;
				}
				outfile << endl;
			}
			outfile.close();
		}
		aT.join();
		delete[] authors_title;
	}
	//保存（作者，合作者）的函数
	void writeAA() {

		for (int i = 0; i < MAX_HASH; ++i) {
			stringstream ss;
			string num;
			ss << i;
			ss >> num;
			string path = "DataBase\\Author_authors\\";
			path += num;
			path += ".txt";
			fstream outfile(path.c_str(), ios::out);
			for (auto& c : author_authors[i]) {
				outfile << c.first;
				for (auto& d : c.second) {
					outfile << '$' << d;
				}
				outfile << endl;
			}
			outfile.close();
		}
		delete[] author_authors;
	}
	//保存作者排行的函数
	void writeRK() {
		//创建小顶堆
		auto cmp = [](pair<string, int>& a, pair<string, int>& b) {
			return a.second > b.second;
		};
		priority_queue<pair<string, int>, vector<pair<string, int>>, decltype(cmp)> q(cmp);

		//排序并只留下前100的作者
		for (auto& c : *author_rank) {
			if (q.size() < 100) {
				q.push(c);
			}
			else {
				if (c.second > q.top().second) {
					q.pop();
					q.push(c);
				}
			}
		}
		delete author_rank;
		fstream outfile("DataBase\\Author_rank\\rank.txt", ios::out);
		for (int i = 0; i < 100; ++i) {
			pair<string, int> temp = q.top(); q.pop();
			outfile << temp.first << "$" << temp.second << endl;//排序并写入
		}
		outfile.close();

	}
	//保存（作者，位置）的函数
	void writeAP() {
		for (int i = 0; i < MAX_HASH; ++i) {
			stringstream ss;
			string num;
			ss << i;
			ss >> num;
			string path = "DataBase\\Article_pos\\";
			path += num;
			path += ".txt";
			fstream outfile(path.c_str(), ios::out);
			for (int j = 0; j < all_article[i].size(); ++j) {
				outfile << all_article[i][j]->m_title << "$" << all_article[i][j]->m_pos << endl;
				delete all_article[i][j];
			}
			outfile.close();
			vector<ArticlePos*>().swap(all_article[i]);
		}
		delete[] all_article;

	}
	//保存各个年份的文章函数
	void writeYT() {
		for (int i = 0; i < 124; ++i) {
			stringstream ss;
			string num;
			ss << (i + MIN_YEAR);
			ss >> num;
			string path = "DataBase\\Year_title\\";
			path += num;
			path += ".txt";
			fstream outfile(path.c_str(), ios::out);
			for (auto& c : year_title[i]) {
				outfile << c << endl;
			}
			outfile.close();
			vector<string>().swap(year_title[i]);
		}
		delete[] year_title;
	}
	//创建保存数据的文件夹
	void creat_directory() {
		char* buf;
		buf = _getcwd(0, 0);
		string  path("mkdir ");
		path += buf;
		system((path + "\\DataBase\\Year_title").c_str());
		system((path + "\\DataBase\\Article_pos").c_str());
		system((path + "\\DataBase\\Author_rank").c_str());
		system((path + "\\DataBase\\Author_authors").c_str());
		system((path + "\\DataBase\\Author_titles").c_str());
		system((path + "\\DataBase\\Title_authors").c_str());
		system((path + "\\DataBase\\Keyword_rank").c_str());

	}
	
	//数据读取
	void data_creat(string& xml_path) {
		//申请内存
		all_article = new vector<ArticlePos*>[MAX_HASH];
		author_rank = new map<string, int>;
		year_title = new vector<string>[124];
		authors_title = new vector<AuthorsTitle>[MAX_HASH];
		author_titles = new map<string, vector<string>>[MAX_HASH];
		author_authors = new map<string, set<string>>[MAX_HASH];
		buffer[0] = new string[MAX_BUFSZ];
		buffer[1] = new string[MAX_BUFSZ];
		
		//读取第一块buffer
		ifstream infile(xml_path,ios::binary);
		for (int i = 0; i < MAX_BUFSZ; ++i) {
			buffer[curr_idx][i].clear();
		}
		for (int i = 0; i < MAX_BUFSZ && !infile.eof(); ++i) {
			getline(infile, buffer[curr_idx][i]);
		}
		curr_idx = 1 - curr_idx;
		//多线程运行
		thread readerT(reader, ref(infile));
		thread getAPT(get_article_pos);
		thread getART(get_author_rank);
		thread getYTT(get_year_title);
		thread getATT(get_authors_title);
		creat_directory();
		readerT.join();
		getAPT.join();
		getART.join();
		getYTT.join();
		getATT.join();

		//cout << author_rank->size() << endl;

		delete[] buffer[0];
		delete[] buffer[1];
	}
	//主要函数
	void data_init(string xml_path) {
		time_t start = time(NULL);//记录时间

		data_creat(xml_path);

		//cout << "结束读取" << endl;
		thread writeATT(write_authorTitles);

		thread writeRKT(writeRK);
		thread writeYTT(writeYT);
		thread writeAPT(writeAP);
		thread writeTAT(writeTA);
		writeATT.join();
		writeAA();
		writeAPT.join();
		writeYTT.join();
		writeRKT.join();
		writeTAT.join();
		time_t end = time(NULL);

		fstream outfile("init.ini", ios::out);
		outfile << "1" << endl;//记录文件是否读取过
		DataStructure::window->set_choosefile_en(start, end);//回复maindow

	}
	void data_init_low(string path) {
		time_t start = time(NULL);

		data_creat(path);
		//一个一个进行，节约内存
		thread writeRKT(writeRK);
		thread writeYTT(writeYT);
		thread writeAPT(writeAP);
		writeAPT.join();
		writeYTT.join();
		writeRKT.join();
		write_authorTitles();
		writeTA();
		writeAA();
		time_t end = time(NULL);

		fstream outfile("init.ini", ios::out);
		outfile << "1" << endl;
		DataStructure::window->set_choosefile_en(start, end);
	}
	//void test() {
	//	time_t start = time(NULL);
	//	Sleep(2000);
	//	time_t end = time(NULL);


	//	DataStructure::window->set_choosefile_en(start, end);

	//}

}
