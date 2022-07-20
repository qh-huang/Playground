/*
// Definition for a Node.
class Node {
public:
    int val;
    Node* left;
    Node* right;
    Node* parent;
};
*/

class Solution {
public:
    Node* lowestCommonAncestor(Node* p, Node * q) {
        vector<Node*> ancestors;
        // push all p's ancestors
        Node* cur = p;
        while (cur != NULL) {
            ancestors.push_back(cur);
            cur = cur->parent;
        }
        cur = q;
        while (cur != NULL) {
            if (find(ancestors.begin(), ancestors.end(), cur) != ancestors.end()) {
                return cur;
            }
            cur = cur->parent;
        }
        return NULL;
    }
};
