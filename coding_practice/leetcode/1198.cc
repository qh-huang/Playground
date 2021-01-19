// 1198. Find Smallest Common Element in All Rows
// https://leetcode.com/problems/find-smallest-common-element-in-all-rows/

class Solution {
public:
    int smallestCommonElement(vector<vector<int>>& mat) {
        if (mat.empty()) return -1;
        if (mat[0].empty()) return -1;
        
        int cols = mat[0].size();
        vector<int> nums = mat[0];
                
        /*
        [1,2,3,4,5]
        [2,4,5,8,10]
        [3,5,7,9,11]
        [1,3,5,7,9]
        */
        for (int i=1; i<mat.size(); i++) {
            for (int j=0; j<nums.size(); j++) {
                if (nums[j] < mat[i][0] || nums[j] > mat[i][cols-1]) {
                    nums.erase(nums.begin()+j);
                    continue;
                }  
            	if (find(mat[i].begin(), mat[i].end(), nums[j]) == mat[i].end()) {
            	    nums.erase(nums.begin()+j);
            	    j--;
            	}
            }
        }
        return (nums.empty())? -1 : nums[0];
    }
};
