/* DFA_Trie.h
 * 
 * Description:
 *	Deterministic Finite Automata implemented as a Trie
 * 
 * Author: Mattias Tiger, 2013
 */

#include <list>
#include <iostream>

#ifndef _TRIE_H_
#define _TRIE_H_

#define SUBPATTERN_SYMBOL	'$'		// The character that indicates a sub-pattern (to use it in a pattern \ must be prefix)
#define LANGUAGE_SIZE		256		// Extended ASCII

// D - data type of each node, A - Argument type of the handler
template <class D, class A>
class DFA_Trie
{
public:
	std::list<DFA_Trie *> parents;
	DFA_Trie * branch[LANGUAGE_SIZE];
	D * data;
	bool (*handler)(A & arg, D & data, std::string & str);
	bool visited;
	bool terminate;

public:
	DFA_Trie()					{ memset(branch, 0, sizeof(branch)); data = 0; visited = false; terminate = false; handler = 0;}
	DFA_Trie(DFA_Trie * parent) { memset(branch, 0, sizeof(branch)); data = 0; visited = false; terminate = false; handler = 0; parents.push_back(parent); }

	bool add(std::string & expression, D * data, bool (*handler)(A & arg, D & data, std::string & str), bool terminate = false, unsigned int index = 0) {
		if(expression.size() < index)
			return false;

		if(expression.size() == index)
		{
			//if(this->data == 0)
			//{
				this->data = data;
				this->handler = handler;
				this->terminate = terminate;
				return true;
			//}
			//return false;
		}
		if(branch[(unsigned char)expression[index]] == 0)
			branch[(unsigned char)expression[index]] = new DFA_Trie(this);
		return branch[(unsigned char)expression[index]]->add(expression, data, handler, terminate, index+1);
	}

	bool add(DFA_Trie * expression) {
		// Should check for inconsistencies!
		std::list<DFA_Trie*> goals_dummy;
		merge(this, expression, goals_dummy);
		return true;
	}
		
	void add_branch(int index, DFA_Trie * node) {
		if(branch[index] != 0)
		{
			branch[index]->parents.remove(this);
			/*
			int size = branch[index]->parents.size();
			branch[index]->parents.remove(branch[index]);
			if(branch[index]->parents.empty())
			{

			}
			else
			{
				if(size < branch[index]->parents.size())
					branch[index]->parents.push_back(branch[index]);
			}
			*/
		}
		branch[index] = node;
		node->parents.push_back(this);
	}

	bool isConsistent(char c)	{ return branch[c] != 0; }
	bool isFinished()			{ return data != 0; }
	bool terminates()			{ return terminate; }
	bool hasSubPattern()		{ return branch[SUBPATTERN_SYMBOL] != 0; }
	void executeHandler(A * arg, std::string & str)
								{ handler(*arg, *data, str); }
	
	DFA_Trie * getReturnPoint() { return branch[SUBPATTERN_SYMBOL]; }

	std::string toString() {
		std::string o, path;
		DFA_Trie * node = this;	
		for(int k = 0; k < LANGUAGE_SIZE; k++)
			toString_DFA_Trie<D,A>(o,path,node, k);
		return o;
	}
};

// Print out the trie
template <typename D, typename A>
void toString_DFA_Trie(std::string & o, std::string & path, DFA_Trie<D,A> * node, int n)
{
	if(node == 0)
		return;
	if(n == 0 && node->isFinished() && !node->visited)
		o += path + " -> " + std::to_string((int)node->data) + " (" + std::to_string((int)node) +")\n";
	if(node->branch[n] == 0)
		return;
	if(node->branch[n]->visited)
	{
		if(!path.empty())
		{
			if(node->branch[n] == node)
			{
				if(char(n) == path.back())
				{
					o += path + " -> <loop: " + std::to_string((int)node) + ">";
					o += " (to self)";
					o += "\n";
				}
			}
			else
			{
				o += path + char(n) + " -> <loop: " + std::to_string((int)node) + ">";
				o += "\n";
			}
		}
		return;
	}
	if(!node->visited)
	{
		path += char(n);	
		node->visited = true;
		for(int k = 0; k < 256; k++)
			toString_DFA_Trie(o,path,node->branch[n], k);
		node->visited = false;
		path.erase((++path.rbegin()).base());
	}
	
}

// merge n1 & n2 into n1 (leavs n2 untouched)
template <typename D, typename A>
void merge(DFA_Trie<D,A> * n1, DFA_Trie<D,A> * n2, std::list<DFA_Trie<D,A> *> & goals)
{
	if(n1->isFinished() && n2->isFinished())
		std::cout << "!!! Inconsistent patterns!\n";
	if(n2->isFinished())
	{
		n1->data = n2->data;
		goals.push_back(n1);
	}
	if(n1->isFinished())
		goals.push_back(n1);
	/*
	// Redirect parents of n2 to n1
	for(std::list<Trie *>::iterator it = n2->parents.begin(); it != n2->parents.end(); it++)
	{
		std::cout << n2->parents.size() << " | " << (it == n2->parents.end()) << " | " << (it == n2->parents.begin()) << "\n";
		for(int b = 0; b < 256; b++)
			if((*it)->branch[b] == n2)
				(*it)->add_branch(b, n1);
	}
	*/
	
	// Copy non-colliding branches to n
	for(int b = 0; b < 256; b++)
		if(n2->branch[b] != 0 && n1->branch[b] == 0)
			if(n2 == n2->branch[b])		// Circular (1-step)
				n1->add_branch(b, n1);
			else
				n1->add_branch(b, n2->branch[b]);
	
	// Solve collisions
	for(int b = 0; b < 256; b++)
		if(n1->branch[b] != 0 && n2->branch[b] != 0 && n1->branch[b] != n2->branch[b] && !(n1 == n1->branch[b] && n2 == n2->branch[b]))
		{
			DFA_Trie<D,A> * n_ = new DFA_Trie<D,A>();
			n_->data = n2->branch[b]->data;
			//goals.remove(n2->branch[b]); // Only in certain cases!
			for(int i = 0; i < 256; i++)
				if(n2->branch[b]->branch[i] != 0)
					n_->add_branch(i, n2->branch[b]->branch[i]);
			merge(n_, n1->branch[b] , goals);
			n1->add_branch(b, n_);
		}
}



	

#endif
