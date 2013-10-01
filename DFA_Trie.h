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

template <class D, class A>
class DFA_handler
{
public:
    virtual bool operator()(A & arg, D & data, std::string & str) = 0;
};

// D - data type of each node, A - Argument type of the handler
template <class D, class A>
class DFA_Trie
{
public:
	std::list<DFA_Trie *> parents;
	DFA_Trie * branch[LANGUAGE_SIZE];
	D * data;
    DFA_handler<D,A> * handler;     //bool (*handler)(A & arg, D & data, std::string & str);
	int visited;

public:
	DFA_Trie()					{ memset(branch, 0, sizeof(branch)); data = 0; visited = 0; handler = 0;}
	DFA_Trie(DFA_Trie * parent) { memset(branch, 0, sizeof(branch)); data = 0; visited = 0; handler = 0; parents.push_back(parent); }

	bool add(std::string & expression, D * data, DFA_handler<D,A> * handler, unsigned int index = 0) {
		if(expression.size() < index)
			return false;

		if(expression.size() == index)
		{
			if(this->data == 0)	// nondestructive add
			{
				this->data = data;
				this->handler = handler;
				return true;
			}
			return false;
		}
		if(branch[(unsigned char)expression[index]] == 0)
			branch[(unsigned char)expression[index]] = new DFA_Trie(this);
		return branch[(unsigned char)expression[index]]->add(expression, data, handler, index+1);
	}
	bool addReplace(std::string & expression, D * data, DFA_handler<D,A> * handler, unsigned int index = 0) {
		if(expression.size() < index)
			return false;

		if(expression.size() == index)
		{
			this->data = data;
			this->handler = handler;
			return true;
		}
		if(branch[(unsigned char)expression[index]] == 0)
			branch[(unsigned char)expression[index]] = new DFA_Trie(this);
		return branch[(unsigned char)expression[index]]->addReplace(expression, data, handler, index+1);
	}
	bool replace();

	bool add(DFA_Trie * expression) {
		// Should check for inconsistencies!
		std::list<DFA_Trie*> goals_dummy;
		//merge(this, expression, goals_dummy);
        DFA_Trie * copy = new DFA_Trie();
        deepCopy(this, copy, goals_dummy);
        clear();
        merge(copy, expression, this, goals_dummy);

		return true;
	}
    bool addReplace(DFA_Trie * expression) {
		// Should check for inconsistencies!
		std::list<DFA_Trie*> goals_dummy;
		//merge(this, expression, goals_dummy);
        DFA_Trie * copy = new DFA_Trie();
        deepCopy(this, copy, goals_dummy);
        clear();
        merge(expression, copy, this, goals_dummy);

		return true;
	}
		
	void add_branch(unsigned char index, DFA_Trie * node) {
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
	bool hasSubPattern()		{ return branch[SUBPATTERN_SYMBOL] != 0; }
	void executeHandler(A * arg, std::string & str)
								{ (*handler)(*arg, *data, str); }
	
	DFA_Trie * getReturnPoint() { return branch[SUBPATTERN_SYMBOL]; }

    void clear() { memset(branch, 0, sizeof(branch)); data = 0; visited = false; handler = 0; }
	std::string toString() {
		std::string o, path;
		DFA_Trie * node = this;	
		for(int k = 0; k < LANGUAGE_SIZE; k++)
			toString_DFA_Trie<D,A>(o,path,node, k);
		return o;
	}
};

// DFA
template <typename D, typename A>
class DFA
{
public:
    DFA() { start = 0; }
    DFA(DFA_Trie<D,A> * start) : start(start) {}
    DFA(DFA & dfa) { start = dfa.start; goals.assign(dfa.goals.begin(), dfa.goals.end()); }
public:
    DFA_Trie<D,A> * start;
    std::list<DFA_Trie<D,A>*> goals;
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


/*** Merge ***/
/*************/

template <typename D, typename A>
void copyNonCollidingBranches(DFA_Trie<D,A> * node1, DFA_Trie<D,A> * node2, DFA_Trie<D,A> * node)
{
    for(int b = 0; b < 256; b++)
        if(node2->branch[b] != 0 && node1->branch[b] == 0)
            if(node2 == node2->branch[b])		// Circular (1-step)
                node->add_branch(b, node);
            else
                node->add_branch(b, node2->branch[b]);
        else
        if(node1->branch[b] != 0 && node2->branch[b] == 0)
            if(node1 == node1->branch[b])		// Circular (1-step)
                node->add_branch(b, node);
            else
                node->add_branch(b, node1->branch[b]);
}

// Same as copyNonCollidingBranches, but recursions are to new nodes
template <typename D, typename A>
void copyNonCollidingBranchesForward(DFA_Trie<D,A> * node1, DFA_Trie<D,A> * node2, DFA_Trie<D,A> * node)
{
    for(int b = 0; b < 256; b++)
        if(node2->branch[b] != 0 && node1->branch[b] == 0)
            if(node2 == node2->branch[b]) {		// Circular (1-step)
                DFA_Trie<D,A> * n_ = new DFA_Trie<D,A>();
                n_->add_branch(b, n_);
                n_->data = node2->data;
                n_->handler = node2->handler;
                node->add_branch(b, n_);
                std::cout << "!!! Reduced coverage of pattern " << node2->data << " (node2) by pattern " << node1->data << " (node1)!\n";
            }
            else
            {
                std::cout << "!!! Inconsistent patterns!\n";
                node->add_branch(b, node2->branch[b]);
            }
        else
        if(node1->branch[b] != 0 && node2->branch[b] == 0)
            if(node1 == node1->branch[b]) {		// Circular (1-step)
                DFA_Trie<D,A> * n_ = new DFA_Trie<D,A>();
                n_->add_branch(b, n_);
                n_->data = node1->data;
                n_->handler = node1->handler;
                node->add_branch(b, n_);
                std::cout << "!!! Reduced coverage of pattern " << node1->data << " (node1) by pattern " << node2->data << " (node2)!\n";
            }
            else
            {
                std::cout << "!!! Inconsistent patterns!\n";
                node->add_branch(b, node1->branch[b]);
            }
}

template <typename D, typename A>
void deepCopy(DFA_Trie<D,A> * nodeSource, DFA_Trie<D,A> * nodeTarget, std::list<DFA_Trie<D,A> *> & goals)
{
    if(nodeSource->data != 0)
    {
        nodeTarget->data = nodeSource->data;
        nodeTarget->handler = nodeSource->handler;
        goals.push_back(nodeTarget);
    }
    for(int b = 0; b < 256; b++)
        if(nodeSource->branch[b] != 0)
        {
            if(nodeSource == nodeSource->branch[b])
                nodeTarget->add_branch(b, nodeTarget);
            else
            {
                DFA_Trie<D,A> * n_ = new DFA_Trie<D,A>();
                deepCopy(nodeSource->branch[b], n_, goals);
                nodeTarget->add_branch(b, n_);
            }
        }
}

// Merge node1 and node2 into node.
// node1 have a higher priority than node2 (if both collide, node1 goes..)
template <typename D, typename A>
void merge(DFA_Trie<D,A> * node1, DFA_Trie<D,A> * node2, DFA_Trie<D,A> * node, std::list<DFA_Trie<D,A> *> & goals)
{
    if(node1->isFinished()) {
        node->data = node1->data;
        node->handler = node1->handler;
        goals.push_back(node);
    }
    else
    if(node2->isFinished()) {
        node->data = node2->data;
        node->handler = node2->handler;
        goals.push_back(node);
    }
    
    if(node1->isFinished() && node2->isFinished())
    {
        copyNonCollidingBranchesForward(node1, node2, node);
    }
    else
    {
        copyNonCollidingBranches(node1, node2, node);
    }
    // Solve collisions
    for(int b = 0; b < 256; b++)
        if(node1->branch[b] != 0 && node2->branch[b] != 0)
        {
            if(node1->branch[b] == node2->branch[b])
            {
                node->branch[b] = new DFA_Trie<D,A>();
                deepCopy<D,A>(node1, node->branch[b], goals);
            }
            else
            {
                if(node1 == node1->branch[b] && node2 == node2->branch[b])
                    node->add_branch(b, node);
                else
                {
                    DFA_Trie<D,A> * n_ = new DFA_Trie<D,A>();
                    if(node1 == node1->branch[b])
                        merge(node1, node2->branch[b], n_, goals);
                    else
                    if(node2 == node2->branch[b])
                        merge(node1->branch[b], node2, n_, goals);
                    else
                        merge(node1->branch[b], node2->branch[b], n_, goals);
                    node->add_branch(b, n_);
                }
            }
        }
}

// merge n1 & n2 into n1 (leavs n2 untouched)
template <typename D, typename A>
void merge(DFA_Trie<D,A> * n1, DFA_Trie<D,A> * n2, std::list<DFA_Trie<D,A> *> & goals)
{
    DFA_Trie<D,A> * copy = new DFA_Trie<D,A>();
    //goals.clear();
    deepCopy(n1, copy, goals);
    n1->clear();
    merge(copy, n2, n1, goals);
}


	

#endif
