// 1506. Find Root of N-Ary Tree
// https://leetcode.com/problems/find-root-of-n-ary-tree/

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

class Solution {
public:
    Node* findRoot(vector<Node*> tree) {
    	Node* top = NULL;
    	for (auto node : tree) {
    	    if (top == NULL) top = node;
    	    for (auto child : node->children) {
    	        parent_of[child] = node;
    	    }
    	    top = FindTop(top);
    	}    
 	return top;
    }
    
    Node* FindTop(Node* node) {
    	Node* top = node;
    	while (parent_of.find(top) != parent_of.end()) {
    	    top = parent_of[top];
    	}
    	return top;
    }
    unordered_map<Node*, Node*> parent_of;
};
