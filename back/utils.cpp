#include"utils.h"


bool isStart(const string& buf)
{
	if (buf.find("<booktitle") != string::npos) return 0;
	string target[] = { "<article" ,"<inproceedings" ,"<proceedings" ,"<book" ,"<phdthesis" ,"<incollection" ,"<mastersthesis","<www" };
	for (int i = 0; i < 8; ++i) {
		auto it = search(buf.begin(), buf.end(), boyer_moore_searcher(target[i].begin(), target[i].end()));//可以理解为string::find
		if (it != buf.end()) return 1;
		/*if (it != buf.end()&&i==3) {
			if (buf.find("<booktitle") != string::npos) return 0;
			return 1;
		}*/
	}
	return 0;
};

bool isTitle(const string& str) {
	if (str.find("<title>") != string::npos)return 1;
	return 0;
}
bool isAuthor(const string& str) {
	if (str.find("<author>") != string::npos || str.find("<editor>") != string::npos) return 1;
	return 0;
}
bool isTail(const string& tempstr)
{
	if ((tempstr.find("</article>") != string::npos) || (tempstr.find("</inproceedings>") != string::npos) ||
		(tempstr.find("</proceedings>") != string::npos) || (tempstr.find("</book>") != string::npos) ||
		(tempstr.find("</incollection>") != string::npos) || (tempstr.find("</phdthesis>") != string::npos) ||
		(tempstr.find("</mastersthesis>") != string::npos) || (tempstr.find("</www>") != string::npos))
		return 1;
	return 0;
};
bool isHomePage(const string& buf) {
	string target[] = {
		"Preface.",
		"Introduction.",
		"Editorial.",
		"Home Page"

	};
	for (int i = 0; i < 4; ++i) {
		auto it = search(buf.begin(), buf.end(), boyer_moore_searcher(target[i].begin(), target[i].end()));//可以理解为string::find
		if (it != buf.end()) return 1;
	}
	return 0;
}
