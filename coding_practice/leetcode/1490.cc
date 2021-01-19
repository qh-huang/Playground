// 1490. Clone N-ary Tree
// https://leetcode.com/problems/clone-n-ary-tree/

/*
// Definition for a Node.
class Node {
public:
    int val;
    vector<Node*> children;

    Node() {}

    Node(int _val) {
        val = _val;
    }

    Node(int _val, vector<Node*> _children) {
        val = _val;
        children = _children;
    }
};
*/

#if 0

class Solution {
public:
    Node* cloneTree(Node* root) {
    	if (!root) return NULL;
	Node* ret = new Node(root->val);
	ret->children.resize(root->children.size());
	for (int i=0; i<ret->children.size(); i++) {
	    ret->children[i] = cloneTree(root->children[i]);
	}
	return ret;
    }
};

#else

class Solution {
public:
    queue< pair<Node*, Node*> > stack;
    Node* cloneTree(Node* root) {
    	if (!root) return NULL;
    	Node* ret = new Node(root->val);
    	for (int i=0; i<root->children.size(); i++) {
    	    stack.push({ret, root->children[i]});
    	}
    	while (!stack.empty()) {
	    Node* parent = stack.front().first;
	    Node* child = stack.front().second;
	    stack.pop();
    	    Node* node = new Node(child->val);
    	    parent->children.push_back(node);
    	    for (int i=0; i<child->children.size(); i++) {
    	    	stack.push({node, child->children[i]});
    	    }
    	}
    	return ret;
    }
};

#endif
