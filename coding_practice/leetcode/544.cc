// 544. Output Contest Matches
// https://leetcode.com/problems/output-contest-matches/

class Solution {
public:
    string findContestMatch(int n) {
    	vector<string> teams;
    	for (int i=1; i<=n; i++) {
    	    teams.push_back(to_string(i));
    	}
    	while (teams.size() > 1) {
    	    teams = FindContestMatchHelper(teams);
    	}
    	return teams[0];
    }
    vector<string> FindContestMatchHelper(vector<string> teams) {
	vector<string> matches;
	for (int i=0; i<teams.size()/2; i++) {
	    string match;
	    match.append("(");
	    match.append(teams[i]);
	    match.append(",");
	    match.append(teams[teams.size()-1-i]);
	    match.append(")");
	    matches.push_back(match);
	}
	return matches;    
    }
};
