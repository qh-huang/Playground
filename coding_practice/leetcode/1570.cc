// 1570. Dot Product of Two Sparse Vectors
// https://leetcode.com/problems/dot-product-of-two-sparse-vectors/

class SparseVector {
public:
    
    SparseVector(vector<int> &nums) {
    	numbers = nums;
    	for (int i=0; i<nums.size(); i++) {
    		if (nums[i] != 0) non_zero_index.push_back(i);
    	}
    }
    
    // Return the dotProduct of two sparse vectors
    int dotProduct(SparseVector& vec) {
    	int sum = 0;
	if (non_zero_index.size() < vec.non_zero_index.size()) {
	    for (int i=0; i<non_zero_index.size(); i++) {
	    	if (vec.numbers[non_zero_index[i]] != 0) {
	    		sum += (numbers[non_zero_index[i]] * vec.numbers[non_zero_index[i]]);
	    	}
	    }
	} else {
	    for (int i=0; i<vec.non_zero_index.size(); i++) {
	    	if (numbers[vec.non_zero_index[i]] != 0) {
	    		sum += (numbers[vec.non_zero_index[i]] * vec.numbers[vec.non_zero_index[i]]);
	    	}
	    }
	}
	return sum;
    }
    
    vector<int> non_zero_index;
    vector<int> numbers;
};

// Your SparseVector object will be instantiated and called as such:
// SparseVector v1(nums1);
// SparseVector v2(nums2);
// int ans = v1.dotProduct(v2);
