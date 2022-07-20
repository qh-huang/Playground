class Solution {
public:
    bool isSubseq(string s, string word) {
        int w=0;
        for (int i=0; i<word.size(); i++) {
            if (w == s.size()) return false;
            while (w < s.size() && s[w] != word[i]) {
                w++;
            }
            if (s[w] == word[i]) {
                if (i == word.size()-1) return true;
                w++;
                continue;
            }
        }
        return false;
    }

    int numMatchingSubseq(string s, vector<string>& words) {
        int res = 0;
        unordered_map<string, bool> cache;
        for (const auto word : words) {
            if (cache.find(word) != cache.end()) {
                res += cache.at(word)? 1 : 0;
                continue;
            }
            const bool is_subseq = isSubseq(s, word);
            cache[word] = is_subseq;
            res += (is_subseq)? 1 : 0;
        }
        return res;
    }
};
