#include"frontFunc.h"

namespace mysearch {
	
	QStringList get_articleInfo(string title) {
		//排除特殊情况
		if (title.empty()) {
			QStringList mylist;
			return mylist;
		}
		//寻找相关信息
		int idx = ELFhash(title);
		string path = "DataBase\\Article_pos\\" + to_string(idx) + ".txt";
		ifstream infile(path);
		string line;
		size_t pos=0;
		QStringList mylist;
		//打开xml文件
		ifstream fileInfile("path.ini");
		getline(fileInfile, line);
		fileInfile.close();
		fileInfile.open(line);
		while (!infile.eof()) {
			getline(infile, line);
			while (!infile.eof()) {
				if (line.substr(0,line.find("$"))==title) {
					pos = stoll(line.substr(line.find("$") + 1));
					break;
				}
				getline(infile, line);
			}
			//找不到，结束
			if (infile.eof()) {
				return mylist;
			}
			//跳转到相应位置
			fileInfile.seekg(pos);
			getline(fileInfile, line);
			mylist.push_back(QString::fromStdString(line));
			getline(fileInfile, line);
			//读取文章信息并返回
			while (!isTail(line)) {
				mylist.push_back(QString::fromStdString(line));
				getline(fileInfile, line);
			}
		}
		return mylist;
	}
	QStringList get_author_titles(string author) {
		if (author.empty()) {
			return QStringList();
		}
		//找作者信息
		int idx = ELFhash(author);
		string path = "DataBase\\Author_titles\\" + to_string(idx) + ".txt";
		ifstream infile(path);
		string buf;

		while (!infile.eof()) {
			getline(infile, buf);
			if (buf.substr(0, buf.find("$")) == author) {
				break;
			}
		}
		QStringList mylist;
		if (infile.eof()) {
			return mylist;
		}
		string name;
		stringstream ss(buf);
		//读取相应信息
		getline(ss, name, '$');
		while (getline(ss, name, '$')) {
			mylist.push_back(QString::fromStdString(name));
		}
		return mylist;
	}
	QStringList get_author_authors(string author) {
		//类似其他
		int idx = ELFhash(author);
		string path = "DataBase\\Author_authors\\" + to_string(idx) + ".txt";
		ifstream infile(path);
		string buf;
		while (!infile.eof()) {
			getline(infile, buf);
			if (buf.substr(0, buf.find("$")) == author) {
				break;
			}
		}
		string name;
		stringstream ss(buf);
		getline(ss, name, '$');
		QStringList mylist;
		while (getline(ss, name, '$')) {
			mylist.push_back(QString::fromStdString(name));
		}
		return mylist;
	}
	QStringList get_author_rank() {
		//类似其他
		QStringList mylist;
		string path = "DataBase\\Author_rank\\rank.txt";
		ifstream infile(path);
		string buf;
		getline(infile, buf);
		while (!infile.eof()) {
			stringstream ss(buf);
			getline(ss, buf, '$');
			mylist.push_back(QString::fromStdString(buf));
			getline(ss, buf, '$');
			mylist.push_back(QString::fromStdString(buf));
			getline(infile, buf);
		}
		return mylist;
	}
	bool isCliqueInit() {
		//能打开文件就是存在
		ifstream ifs("DataBase/clique.txt");
		if (ifs.good()) {
			return 1;
		}
		return 0;
	}
	QStringList get_clique() {
		//读取文件信息
		QStringList mylist;
		ifstream infile("DataBase/clique.txt");
		string buf;
		getline(infile, buf);
		int num = stoi(buf);
		for (int i = 0; i < num; ++i) {
			getline(infile, buf);
			stringstream ss(buf);
			ss >> buf;
			mylist.push_back(QString::fromStdString(buf));
			ss >> buf;
			mylist.push_back(QString::fromStdString(buf));
		}
		return mylist;
	}
	//可视化中边的结构体
	struct edge {
		string source;//边的起点
		string target;//边的终点
		//重载==,在map中进行判断要用到
		bool operator==(edge input) {
			bool flag = 0;
			if (source == input.source && target == input.target) {
				flag = 1;
			}
			if (source == input.target && target == input.source) {
				flag = 1;
			}
			return flag;
		}
		friend bool operator==(const edge &in,const edge &input) {
			bool flag = 0;
			if (in.source == input.source && in.target == input.target) {
				flag = 1;
			}
			if (in.source == input.target && in.target == input.source) {
				flag = 1;
			}
			return flag;
		}
	};
	//重载哈希map中的哈希函数
	struct hash_edge {
		size_t operator()(const edge& e) const {
			return hash<string>()(e.source) ^ hash<string>()(e.target);
		}
	};

	QString get_js(string authorName) {
		//找到输入作者的信息
		string name = authorName;
		if (name.empty()) {
			return QString();
		}
		int idx = ELFhash(name);
		string path = "DataBase\\Author_authors\\" + to_string(idx) + ".txt";
		ifstream infile(path);
		string buf;

		while (!infile.eof()) {
			getline(infile, buf);
			istringstream ss(buf);
			string temp;
			getline(ss, temp, '$');
			if (temp==name) {
				break;
			}
		}
		QString js;
		if (infile.eof()) return js;

		infile.close();
		//作者的信息
		stringstream ss(buf);
		unordered_set<string> authors;
		QJsonArray data, links;
		unordered_map<edge, unordered_set<string>, hash_edge> links_map;

		//设置输入节点为红色
		getline(ss, name, '$');
		QJsonObject dataobj;
		authors.emplace(name);
		QString str = QString::fromStdString(name);
		dataobj.insert("name", str);
		QJsonObject itemobj;
		itemobj.insert("color", "#ff0000");
		dataobj.insert("itemStyle", itemobj);
		data.append(dataobj);
		string mainName = name;
		//其他节点
		while (getline(ss, name, '$')) {
			QJsonObject().swap(dataobj);
			authors.emplace(name);
			QString str = QString::fromStdString(name);
			dataobj.insert("name", str);
			data.append(dataobj);//记录结点信息
			edge info;
			info.source = mainName;
			info.target = name;
			links_map[info].insert((string)"ERROR:Multi Results");
		}
		//找作者写过的文章
		infile.clear();
		path = "DataBase\\Author_titles\\" + to_string(idx) + ".txt";
		infile.open(path);
		while (!infile.eof()) {
			getline(infile, buf);
			if (buf.find(authorName) != string::npos) {
				break;
			}
		}
		infile.close();
		istringstream ss2(buf);
		getline(ss2, name, '$');
		//处理每个文章的作者信息
		while (getline(ss2, name, '$')) {
			if (isHomePage(name))continue;
			int idx_title = ELFhash(name);
			infile.clear();
			path = "DataBase\\Title_authors\\" + to_string(idx_title) + ".txt";
			infile.open(path);
			while (!infile.eof()) {
				getline(infile, buf);
				if (buf.find(name) != string::npos) {
					break;
				}
			}
			infile.close();
			istringstream iss(buf);
			string title;
			getline(iss, title, '$');
			vector<string> v;
			//处理一篇文章中作者间的合作
			while (getline(iss, name, '$')) {
				if (name != authorName) {
					edge info;
					info.source = authorName;
					info.target = name;
					if (links_map[info].count((string)"ERROR:Multi Results")) {

						links_map[info].clear();
					}
					links_map[info].insert(title);//合作的文章
					v.emplace_back(name);//文章的所有作者
				}
			}
			//作者两两为合作
			for (int i = 0; i < v.size(); ++i) {
				for (int j = i+1; j < v.size(); ++j) {
					edge info;
					info.source = v[i];
					info.target = v[j];
					if (links_map[info].count((string)"ERROR:Multi Results")) {
						links_map[info].clear();
					}
					links_map[info].insert(title);
				}
			}
		}
		//将边处理成json格式
		for (auto& link : links_map) {
			QJsonObject dataobj;
			dataobj.insert("source", QString::fromStdString(link.first.source));
			dataobj.insert("target", QString::fromStdString(link.first.target));
			QJsonArray js_array;

			for (auto& m_title : link.second) {
				js_array.append(QString::fromStdString(m_title));
			}
			dataobj.insert("relation", js_array);
			links.append(dataobj);
		}
		QString datastr = QJsonDocument(data).toJson();
		QString linkstr = QJsonDocument(links).toJson();
		js = QString("update(%1,%2)").arg(datastr).arg(linkstr);//用来运行javascript
		return js;

	}
	//QString test_js() {
	//	QJsonArray data, links;
	//	
	//	QJsonObject dataobj;
	//	dataobj.insert("name", "aaa");
	//	data.append(dataobj);
	//	QJsonObject().swap(dataobj);

	//	dataobj.insert("name", "bbb");
	//	data.append(dataobj);
	//	QJsonObject().swap(dataobj);

	//	dataobj.insert("name", "ccc");
	//	data.append(dataobj);
	//	QJsonObject().swap(dataobj);

	//	dataobj.insert("name", "ddd");
	//	data.append(dataobj);
	//	QJsonObject().swap(dataobj);

	//	dataobj.insert("name", "eee");
	//	data.append(dataobj);

	//	QJsonObject linkobj;
	//	linkobj.insert("source", "aaa");
	//	linkobj.insert("target", "bbb");
	//	linkobj.insert("relation", "alphas_test");
	//	links.append(linkobj);
	//	QString datastr = QJsonDocument(data).toJson();
	//	QString linkstr = QJsonDocument(links).toJson();
	//	QString js = QString("update(%1,%2)").arg(datastr).arg(linkstr);
	//	return js;
	//}
}