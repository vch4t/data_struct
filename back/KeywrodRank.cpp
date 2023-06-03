#include"KeywordRank.h"
#include"../lemmagen/sl_lemmatizer.h"
#include<qobject.h>
namespace keyword {
	string* buffer[2];//buffer
	unordered_map<string, int>* word_rank;//关键词哈希表
	unordered_map<string, bool>* common;//日常用词数组
	const int MAX_BUFSZ = 4096;//buffer大小
	int curr_idx = 0;//当前正在写入的数组
	bool reader_end = 0;//文章标题读取完成标记
	bool deal_end = 0;//名词处理完成标记

	//环形队列，学习linux中的设计,对取模的优化
	class RingBuffer {
		string* mbuffer;
		uint32_t size;
		uint32_t read_ptr;
		uint32_t write_ptr;
	public:
		RingBuffer(uint32_t sz) {
			mbuffer = new string[sz];
			size = sz;
			read_ptr = write_ptr = 0;
		}
		inline bool empty() {
			return read_ptr == write_ptr;
		}
		inline bool full() {
			return size == (write_ptr - read_ptr);
		}
		bool write(string* str) {
			atomic_thread_fence(memory_order_seq_cst);//多核防止优化，造成重复读写错误
			if (size == (write_ptr - read_ptr)) {
				return 0;
			}
			uint32_t idx = write_ptr & 0xfff;
			mbuffer[idx] = *str;
			write_ptr++;
			return 1;
		}
		bool read(string* c) {
			atomic_thread_fence(memory_order_seq_cst);//多核防止优化，造成重复读写错误
			if (read_ptr == write_ptr) {
				return 0;
			}
			uint32_t idx = read_ptr & 0xfff;
			*c = mbuffer[idx];
			read_ptr++;
			return 1;
		}
		~RingBuffer() {
			if (mbuffer != NULL) {
				delete[] mbuffer;
			}
		}
	};
	RingBuffer *ringbuffer;

	//词形还原
	inline string deal_word(string& str) {
		string buf = lem_lemmatize_word_alloc(const_cast<char*>(str.c_str()));
		return buf;
	}
	//判断是否为日常用词
	bool isCommon(string& str) {
		if ((*common)[str] == 1)return 1;
		return 0;
	}
	//读取文件
	void reader(fstream& infile) {

		for (int i = 0; i < MAX_BUFSZ; ++i) {
			buffer[curr_idx][i].clear();
		}
		for (int i = 0; i < MAX_BUFSZ && !infile.eof(); ++i) {
			getline(infile, buffer[curr_idx][i]);
		}
	}
	//处理名词排行
	void get_rank() {
		while (1) {
			//ringbuffer读取完成
			if (deal_end && ringbuffer->empty()) {
				return;
			}
			if (!ringbuffer->empty()) {
				string ans; ringbuffer->read(&ans);
				ans = deal_word(ans);//词形还原
				transform(ans.begin(), ans.end(), ans.begin(), ::tolower);//转成小写
				if (!isCommon(ans)) {
					(*word_rank)[ans]++;
				}
			}

		}
	}
	//从文章中分词
	void get_token(fstream& infile) {
		reader(infile);
		curr_idx = 1 - curr_idx;
		while (!infile.eof()) {
			thread readerT(reader, ref(infile));
			string* m_head = buffer[1 - curr_idx];
			for (int i = 0; i < MAX_BUFSZ; ++i) {
				stringstream ss(m_head[i]);
				string s;
				while (ss >> s) {
					if (s[s.size() - 1] == '.') {
						s.pop_back();
					}//除去尾部符号
					if (!s.empty()) {
						while (!ringbuffer->write(&s)) {}//等待可写
					}
				}
			}
			readerT.join();
			curr_idx = 1 - curr_idx;
		}
		//最后一个buffer的处理
		string* m_head = buffer[1 - curr_idx];
		for (int i = 0; i < MAX_BUFSZ; ++i) {
			stringstream ss(m_head[i]);
			string s;
			while (ss >> s) {
				if (!isalnum(s[s.size() - 1])) {
					s.pop_back();
				}
				if (!s.empty()) {

					while (!ringbuffer->write(&s)) {}
				}
			}
		}
		//处理完成
		deal_end = 1;
	}
	//初始化常用词表
	void fullCommon() {
		//常用词组
		string temp[] = { "a","no","at","as","on",
	 "all", "any", "be", "both","by","so","to",
	"each", "either", "one", "two", "three",
	"four", "five", "six", "seven", "eight",
	"nine", "ten", "none", "little", "few",
	"many", "much", "other", "another", "some",
	"every", "nobody", "anybody", "somebody",
	"everybody", "when", "under","it","me"
	"first", "second", "third", "forth", "fifth",
	"sixth", "seventh", "above", "over", "below",
	"under", "beside", "behind", "the", "after",
	"from", "since", "for", "which", "next",
	"where", "how", "who", "there", "before"
	 "do", "this", "that","tomorrow", "yesterday",
	"last", "brfore", "because", "against",
	"except", "beyond", "along", "among", "but",
	"towards", "you","i","he","my","or","an",
	"your", "his", "her", "she", "its", "they",
	"them", "and", "have", "us","if","in",
	"would", "then", "too", "our", "off","base",
	"into", "can", "of","have", "even", "these", "those",
	"ours", "with", "use","the","their", "between" };
		//记入哈希表
		common = new unordered_map<string, bool>;
		for (int i = 0; i < 113; ++i) {
			(*common)[temp[i]] = 1;
		}
	}
	//主要功能函数
	QStringList rank_keyword(string& year_path) {

		QStringList mylist;
		//如果有记录，直接读取输出
		ifstream qinfile("DataBase\\Keyword_rank\\" + year_path + ".txt");
		if (qinfile.good()) {
			string buf;
			getline(qinfile, buf);
			while (!qinfile.eof()) {
				istringstream ss(buf);
				getline(ss, buf, '$');
				mylist.push_back(QString::fromStdString(buf));
				getline(ss, buf, '$');
				mylist.push_back(QString::fromStdString(buf));
				getline(qinfile, buf);
			}
			qinfile.close();
			return mylist;
		}
		//没有记录，进行查找
		//数据初始化
		ringbuffer = new RingBuffer(0x1000);
		string* buffer1=buffer[0] = new string[MAX_BUFSZ];
		string* buffer2=buffer[1] = new string[MAX_BUFSZ];
		word_rank = new unordered_map<string, int>;
		lem_load_language_library((char*)"english.bin");//载入词形还原模块

		fstream infile("DataBase\\Year_title\\"+ year_path + ".txt", ios::in);
		fullCommon();//初始化常用词表
		thread getTKT(get_token, ref(infile));
		thread getRKT(get_rank);

		getTKT.join();
		getRKT.join();

		infile.close();
		//堆排
		auto cmp = [](pair<string, int>& a, pair<string, int>& b) {
			return a.second > b.second;
		};
		priority_queue<pair<string, int>, vector<pair<string, int>>, decltype(cmp)> q(cmp);

		for (auto& c : *word_rank) {
			if (q.size() < 10) {
				q.push(c);
			}
			else {
				if (c.second > q.top().second) {
					q.pop();
					q.push(c);
				}
			}
		}
		delete word_rank;
		delete[] buffer1;
		delete[] buffer2;
		delete common;
		delete ringbuffer;

		fstream outfile("DataBase\\Keyword_rank\\" + year_path + ".txt", ios::out);
		for (int i = 0; i < 10; ++i) {
			mylist.push_back(QString::fromStdString(q.top().first));
			mylist.push_back(QString::number(q.top().second));//返回结果
			outfile << q.top().first << "$" << q.top().second << endl;//保存结果
			q.pop();
		}
		outfile.close();
		deal_end = 0;


		return mylist;
	}
}