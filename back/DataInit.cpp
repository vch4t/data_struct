#include"utils.h"
#include "../DataStructure.h"
#include<QMessageBox>



namespace DataInit {

	//��ʼ
	const int MAX_HASH = 4096;//��ϣ��ģ��Ҳ���ļ��ֳɲ��ֵ�����
	const int MAX_BUFSZ = 8192 * 4;//��ʱbuffer�Ĵ�С
	const int MIN_YEAR = 1900;//������
	//��c++ʵ�ֵ�һ���߳�barrier
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
	Barrier barrier(5);//��barrier��ʼ����5����Ϊ��5��ͬʱ������߳�
	string* buffer[2];//������ʱbuffer
	int curr_idx = 0;//��ǰ��д���buffer���
	bool read_end = 0;//�Ƿ��ȡxml�ļ�����
	//��¼���������Լ�����xml�ļ���λ��
	class ArticlePos {
	public:
		string m_title;
		size_t m_pos;
		ArticlePos(string title, size_t pos) :m_title(title), m_pos(pos) {}
	};
	//��¼�������Լ����±���
	class AuthorsTitle {
	public:
		vector<string> m_name;
		string m_title;
		AuthorsTitle(vector<string> name, string title) :m_name(name), m_title(title) {}
	};

	vector<ArticlePos*>* all_article;//��¼�������¼���λ��
	map<string, int>* author_rank;//��¼�������߼���������
	vector<string>* year_title;//��¼���������������
	vector<AuthorsTitle>* authors_title;//��¼���У������ǣ����£�
	map<string, vector<string>>* author_titles;//��¼�����ߣ�����д���£�
	map<string, set<string>>* author_authors;//��¼�����ߣ������ߣ�

	//һ���ж�ȡ�ļ��ĺ���
	void reader(ifstream& infile) {
		while (!infile.eof()) {
			//���buffer
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				buffer[curr_idx][i].clear();
			}
			//д��buffer
			for (int i = 0; i < MAX_BUFSZ && !infile.eof(); ++i) {
				getline(infile, buffer[curr_idx][i]);
			}
			curr_idx = 1 - curr_idx;//�л�Ҫд���buffer
			barrier.waiting();
		}
		read_end = 1;//�����ļ��ˣ����б��
	}
	//�õ����¼���λ�õĺ���
	void get_article_pos() {
		size_t curr_pos = 0, pos = 0;//������Ӧ��λ�ã�curr_pos�ǵ�ǰ�е�λ�ã�pos��һƪ���µĳ�ʼλ�ã������δ�õ�����һ������ȴ�л�buffer
		while (1) {
			int idx = 1 - curr_idx;;//Ҫʹ�õ�buffer�ı�ţ������ڶ�ȡ�ı�Ų�ͬ
			string* m_head = buffer[idx];//����һ��ָ�뷽���Ķ�
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isStart(m_head[i])) {
					pos = curr_pos;//��һƪ���µĿ�ͷ������λ��
				}
				else if (isTitle(m_head[i])) {//�����±���������
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					string title = m_head[i].substr(l + 1, r - l - 1);//�õ����±���
					all_article[ELFhash(title)].emplace_back(new ArticlePos(title, pos));//�����м�ֵ������
				}
				curr_pos += m_head[i].length() + 1;//��ǰ��λ�ø���
			}
			if (read_end)return;//������Ϣ�������ˣ�������
			barrier.waiting();//barrier���ȴ������߳�
		}
	}
	//�õ��������еĺ���
	void get_author_rank() {
		while (1) {
			int idx = 1 - curr_idx;
			string* m_head = buffer[idx];
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isAuthor(m_head[i])) {
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					string name = m_head[i].substr(l + 1, r - l - 1);
					(*author_rank)[name]++;//����һ����������ͬ����һƪ���£�������������+1
				}
			}
			if (read_end)return;
			barrier.waiting();
		}
		//δ����������get_article_pos����ʾ
	}
	//�õ�ÿһ�����������
	void get_year_title() {
		string title;//�������±��⣬�����δ�õ�����һ������ȴ�л�buffer
		while (1) {
			int idx = 1 - curr_idx;
			string* m_head = buffer[idx];
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				if (isTitle(m_head[i])) {//�ҵ����Ⲣ��¼
					size_t l = m_head[i].find(">");
					size_t r = m_head[i].rfind("<");
					title = m_head[i].substr(l + 1, r - l - 1);
				}
				if (m_head[i].find("<year>") != string::npos) {
					size_t ly = m_head[i].find("<year>");
					size_t ry = m_head[i].find("</year>");
					string year = m_head[i].substr(ly + 6, ry - ly - 5 - 1);
					year_title[stoi(year) - MIN_YEAR].emplace_back(title);//��¼��ǰ���м�ֵ������
				}

			}
			if (read_end)return;
			barrier.waiting();
		}
		//δ����������get_article_pos����ʾ
	}
	//�õ�һƪ���¼�����������
	void get_authors_title() {
		vector<string> author;//���������ǣ������δ�õ�����һ������ȴ�л�buffer
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
					author.clear();//�ǵ����
				}
			}
			if (read_end)return;
			barrier.waiting();
			//δ����������get_article_pos����ʾ
		}
	}
	//�õ����߼�������ߵĺ��� 
	void get_author_authors() {
		for (int i = 0; i < MAX_HASH; ++i) {
			for (auto& title : authors_title[i]) {
				int size = title.m_name.size();
				vector<string>& temp = title.m_name;
				if (size == 1) {
					size_t idx = ELFhash(temp[0]);
					author_authors[idx][temp[0]];
					//��û�к�����ʱ��¼
				}
				for (int j = 0; j < size; ++j) {
					for (int k = 0; k < size; ++k) {
						if (j == k) {
							continue;
						}
						size_t idx = ELFhash(temp[j]);
						author_authors[idx][temp[j]].insert(temp[k]);
						//һƪ����������Ϊ�����ߣ���¼
					}
				}
			}

		}

	}
	//���棨���ߣ����£��ĺ���
	void write_authorTitles() {
		//�õ������ߣ����£���Ϣ
		for (int i = 0; i < MAX_HASH; ++i) {
			for (int j = 0; j < authors_title[i].size(); ++j) {
				vector<string>& c = authors_title[i][j].m_name;
				string title = authors_title[i][j].m_title;
				for (auto& name : c) {
					author_titles[ELFhash(name)][name].emplace_back(title);
				}
			}
		}
		//���ļ�˳�򱣴�
		for (int i = 0; i < MAX_HASH; ++i) {
			//���ļ�
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
				temp.shrink_to_fit();//�ڴ����
				for (auto& d : temp) {
					outfile << '$' << d;//�����ָ���
				}
				outfile << endl;
				vector<string>().swap(temp);//���temp��ָ���ڴ�
			}
			outfile.close();
		}
		delete[] author_titles;
	}
	//���棨���£����ߣ��ĺ���
	void writeTA() {
		thread aT(get_author_authors);//ͬʱִ�У�����һ������ڴ�
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
	//���棨���ߣ������ߣ��ĺ���
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
	//�����������еĺ���
	void writeRK() {
		//����С����
		auto cmp = [](pair<string, int>& a, pair<string, int>& b) {
			return a.second > b.second;
		};
		priority_queue<pair<string, int>, vector<pair<string, int>>, decltype(cmp)> q(cmp);

		//����ֻ����ǰ100������
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
			outfile << temp.first << "$" << temp.second << endl;//����д��
		}
		outfile.close();

	}
	//���棨���ߣ�λ�ã��ĺ���
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
	//���������ݵ����º���
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
	//�����������ݵ��ļ���
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
	
	//���ݶ�ȡ
	void data_creat(string& xml_path) {
		//�����ڴ�
		all_article = new vector<ArticlePos*>[MAX_HASH];
		author_rank = new map<string, int>;
		year_title = new vector<string>[124];
		authors_title = new vector<AuthorsTitle>[MAX_HASH];
		author_titles = new map<string, vector<string>>[MAX_HASH];
		author_authors = new map<string, set<string>>[MAX_HASH];
		buffer[0] = new string[MAX_BUFSZ];
		buffer[1] = new string[MAX_BUFSZ];
		
		//��ȡ��һ��buffer
		ifstream infile(xml_path,ios::binary);
		for (int i = 0; i < MAX_BUFSZ; ++i) {
			buffer[curr_idx][i].clear();
		}
		for (int i = 0; i < MAX_BUFSZ && !infile.eof(); ++i) {
			getline(infile, buffer[curr_idx][i]);
		}
		curr_idx = 1 - curr_idx;
		//���߳�����
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
	//��Ҫ����
	void data_init(string xml_path) {
		time_t start = time(NULL);//��¼ʱ��

		data_creat(xml_path);

		//cout << "������ȡ" << endl;
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
		outfile << "1" << endl;//��¼�ļ��Ƿ��ȡ��
		DataStructure::window->set_choosefile_en(start, end);//�ظ�maindow

	}
	void data_init_low(string path) {
		time_t start = time(NULL);

		data_creat(path);
		//һ��һ�����У���Լ�ڴ�
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
