// 1485. Clone Binary Tree With Random Pointer
// https://leetcode.com/problems/clone-binary-tree-with-random-pointer/

/**
 * Definition for a Node.
 * struct Node {
 *     int val;
 *     Node *left;
 *     Node *right;
 *     Node *random;
 *     Node() : val(0), left(nullptr), right(nullptr), random(nullptr) {}
 *     Node(int x) : val(x), left(nullptr), right(nullptr), random(nullptr) {}
 *     Node(int x, Node *left, Node *right, Node *random) : val(x), left(left), right(right), random(random) {}
 * };
 */

class Solution {
public:
    NodeCopy* copyRandomBinaryTree(Node* root) {
        if (!root) return NULL;
        NodeCopy* ret = new NodeCopy(root->val);
        to_new_addr[root] = ret;
        if (root->random) {
            if (to_new_addr.find(root->random) == to_new_addr.end()) {
                ret->random = copyRandomBinaryTree(root->random);
                to_new_addr[root->random] = ret->random;
            } else {
                ret->random = to_new_addr[root->random];
            }
        }
        if (to_new_addr.find(root->left) != to_new_addr.end()) {
            ret->left = to_new_addr[root->left];
        } else {
            ret->left = copyRandomBinaryTree(root->left);
        }
        if (to_new_addr.find(root->right) != to_new_addr.end()) {
            ret->right = to_new_addr[root->right];
        } else {
            ret->right = copyRandomBinaryTree(root->right);
        }
        return ret;
    }
    unordered_map<Node*, NodeCopy*> to_new_addr;
};
