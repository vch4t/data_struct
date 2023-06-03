#include"KeywordRank.h"
#include"../lemmagen/sl_lemmatizer.h"
#include<qobject.h>
namespace keyword {
	string* buffer[2];//buffer
	unordered_map<string, int>* word_rank;//�ؼ��ʹ�ϣ��
	unordered_map<string, bool>* common;//�ճ��ô�����
	const int MAX_BUFSZ = 4096;//buffer��С
	int curr_idx = 0;//��ǰ����д�������
	bool reader_end = 0;//���±����ȡ��ɱ��
	bool deal_end = 0;//���ʴ�����ɱ��

	//���ζ��У�ѧϰlinux�е����,��ȡģ���Ż�
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
			atomic_thread_fence(memory_order_seq_cst);//��˷�ֹ�Ż�������ظ���д����
			if (size == (write_ptr - read_ptr)) {
				return 0;
			}
			uint32_t idx = write_ptr & 0xfff;
			mbuffer[idx] = *str;
			write_ptr++;
			return 1;
		}
		bool read(string* c) {
			atomic_thread_fence(memory_order_seq_cst);//��˷�ֹ�Ż�������ظ���д����
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

	//���λ�ԭ
	inline string deal_word(string& str) {
		string buf = lem_lemmatize_word_alloc(const_cast<char*>(str.c_str()));
		return buf;
	}
	//�ж��Ƿ�Ϊ�ճ��ô�
	bool isCommon(string& str) {
		if ((*common)[str] == 1)return 1;
		return 0;
	}
	//��ȡ�ļ�
	void reader(fstream& infile) {

		for (int i = 0; i < MAX_BUFSZ; ++i) {
			buffer[curr_idx][i].clear();
		}
		for (int i = 0; i < MAX_BUFSZ && !infile.eof(); ++i) {
			getline(infile, buffer[curr_idx][i]);
		}
	}
	//������������
	void get_rank() {
		while (1) {
			//ringbuffer��ȡ���
			if (deal_end && ringbuffer->empty()) {
				return;
			}
			if (!ringbuffer->empty()) {
				string ans; ringbuffer->read(&ans);
				ans = deal_word(ans);//���λ�ԭ
				transform(ans.begin(), ans.end(), ans.begin(), ::tolower);//ת��Сд
				if (!isCommon(ans)) {
					(*word_rank)[ans]++;
				}
			}

		}
	}
	//�������зִ�
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
					}//��ȥβ������
					if (!s.empty()) {
						while (!ringbuffer->write(&s)) {}//�ȴ���д
					}
				}
			}
			readerT.join();
			curr_idx = 1 - curr_idx;
		}
		//���һ��buffer�Ĵ���
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
		//�������
		deal_end = 1;
	}
	//��ʼ�����ôʱ�
	void fullCommon() {
		//���ô���
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
		//�����ϣ��
		common = new unordered_map<string, bool>;
		for (int i = 0; i < 113; ++i) {
			(*common)[temp[i]] = 1;
		}
	}
	//��Ҫ���ܺ���
	QStringList rank_keyword(string& year_path) {

		QStringList mylist;
		//����м�¼��ֱ�Ӷ�ȡ���
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
		//û�м�¼�����в���
		//���ݳ�ʼ��
		ringbuffer = new RingBuffer(0x1000);
		string* buffer1=buffer[0] = new string[MAX_BUFSZ];
		string* buffer2=buffer[1] = new string[MAX_BUFSZ];
		word_rank = new unordered_map<string, int>;
		lem_load_language_library((char*)"english.bin");//������λ�ԭģ��

		fstream infile("DataBase\\Year_title\\"+ year_path + ".txt", ios::in);
		fullCommon();//��ʼ�����ôʱ�
		thread getTKT(get_token, ref(infile));
		thread getRKT(get_rank);

		getTKT.join();
		getRKT.join();

		infile.close();
		//����
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
			mylist.push_back(QString::number(q.top().second));//���ؽ��
			outfile << q.top().first << "$" << q.top().second << endl;//������
			q.pop();
		}
		outfile.close();
		deal_end = 0;


		return mylist;
	}
}