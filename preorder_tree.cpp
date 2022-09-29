#include <iostream>
#include <string>
#include <queue>
using namespace std;

struct node
{
	char ch;
	node* lc;
	node* rc;
};

class BST
{
	node* root;

public:
	BST() { root = NULL; }
	void set_left(node* head, node* temp)
	{
		head->lc = temp;
	}
	void set_right(node* head, node* temp)
	{
		head->rc = temp;
	}
	node* creathelp(string str,int &idx)
	{
		node* head = new node;
		head->ch = str[idx];
		head->lc = NULL;
		head->rc = NULL;
		if (str[idx] >= 'a' && str[idx] <= 'z')
		{
			++idx;
			set_left(head, creathelp(str, idx));
			++idx;
			set_right(head, creathelp(str, idx));
			return head;
		}
		else
		{
			return head;
		}
	}
	void creat(string str)
	{
		int idx = 0;
		root = creathelp(str, idx);
	}
	void print()
	{
		if (root == NULL) cout << "empty!" << endl;
		queue<node*> q;
		q.push(root);
		while (!q.empty())
		{
			node* temp = q.front();
			q.pop();
			cout << temp->ch;
			if (temp->lc)
			{
				q.push(temp->lc);
			}
			if (temp->rc)
			{
				q.push(temp->rc);
			}
		}
		cout << endl;
	}

};

int main()
{
	int times;
	cin >> times;
	while (times--)
	{
		BST bst;
		string str;
		cin >> str;
		bst.creat(str);
		bst.print();
	}
	
	return 0;
}
