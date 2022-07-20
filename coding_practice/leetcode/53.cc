class Solution {
public:
    int maxSubArray(vector<int>& nums) {
        vector<int> subsum;
        subsum.resize(nums.size());
        int cur_max = 0;
        for (int i=0; i<nums.size(); i++) {
            if (i == 0) {
                subsum[i] = nums[i];
                cur_max = nums[i];
            } else {
                int add_res = subsum[i-1] + nums[i];
                subsum[i] = max(add_res, nums[i]);
                cur_max = max(cur_max, subsum[i]);
            }
        }
        return cur_max;
    }
};
