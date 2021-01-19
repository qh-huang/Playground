// 1602. Find Nearest Right Node in Binary Tree
// https://leetcode.com/problems/find-nearest-right-node-in-binary-tree/

/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode() : val(0), left(nullptr), right(nullptr) {}
 *     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
 *     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
 * };
 */
class Solution {
public:
    TreeNode* findNearestRightNode(TreeNode* root, TreeNode* u) {
    	if (!root) return NULL;
    	vector<TreeNode*> nodes;
    	nodes.push_back(root);
    	return FindNearestRightNodeHelper(nodes, u);
    }
    
    TreeNode* FindNearestRightNodeHelper(vector<TreeNode*> nodes, TreeNode* u) {
    	if (nodes.empty()) return NULL;
    	vector<TreeNode*> next_level_nodes;
    	for (int i=0; i<nodes.size(); i++) {
    	    if (nodes[i] == u) {
    	    	return (i == nodes.size()-1)? NULL : nodes[i+1];
    	    }
    	    if (nodes[i]->left) next_level_nodes.push_back(nodes[i]->left);
    	    if (nodes[i]->right) next_level_nodes.push_back(nodes[i]->right);
    	}
    	return FindNearestRightNodeHelper(next_level_nodes, u);
    }
};
