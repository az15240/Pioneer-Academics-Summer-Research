 #include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

using namespace std;

struct user {
	double rank;
	string handle;
	int old_rating;
	double new_rating = 0;
	int official_new_rating;
	double seed = 0.5;
	double delta = 0.0;
} u;

vector<user> users;

string contest_ids[21] = {"1416","1444","1442","1446","1439","1456","1458","1464","1470","1477","1479","1495","1500","1483","1503","1508","1528","1540","1545","1548"};

double aveRsq;

// Functions for user struct

bool cmp_old_rating_desc(user a, user b) {
	return a.old_rating > b.old_rating;
}

bool cmp_rank_asc(user a, user b) {
	return a.rank < b.rank;
}

string to_string(user &u) {
	char s[200];
	sprintf(s, "%5.0f  %-24s seed: %12.6f  rating: %4d -> %4d vs %4d%s\n",
		round(u.rank),
		u.handle.c_str(),
		u.seed,
		u.old_rating,
		u.new_rating,
		u.official_new_rating,
		u.validation.c_str()
	);
	return string(s);
}

string to_string(vector<user> &users) {
	string s;
	for(int i = 0; i < users.size(); i++) {
		s += to_string(users[i]);
	}
	return s;
}

// Functions for calculation

double cal_p(user &a, user &b) {
	return 1.0 / (1.0 + pow(10, (b.old_rating - a.old_rating) / 400.0));
}

double cal_seed(int idx, int rating) {
	user ex_user;
	ex_user.old_rating = rating;
	double res = 1.0;
	for(int i = 0; i < users.size(); i++) {
		if(i != idx) {
			res += cal_p(users[i], ex_user);
		}
	}
	return res;
}

int cal_rating_to_rank(int idx, double rank) {
	int l = 1, r = 8000;
	while(r-l > 1) {
		int mid = (l + r) / 2;
		if(cal_seed(idx, mid) < rank) {
			r = mid;
		}
		else {
			l = mid;
		}
	}
	return l;
}

void work(double K) {
	// Calculate seed
	for(int i = 0; i < users.size(); i++) {
		for(int j = 0; j < users.size(); ++j) {
			//if(i != j) {
				users[i].seed += cal_p(users[j], users[i]);
			//}
		}
	}
	for(int i = 0; i < users.size(); i++) {
		users[i].delta = round(K * (users[i].rank - users[i].seed));
	}

	/*
	// Calculate initial delta and sum_delta
	int sum_delta = 0;
	for(int i = 0; i < users.size(); i++) {
		double m = sqrt(users[i].rank * users[i].seed);
		int R = cal_rating_to_rank(i, m);
		users[i].delta = (R - users[i].old_rating) / 2;
		sum_delta += users[i].delta;
	}
	// Calculate the first inc
	int inc = -(sum_delta / users.size()) - 1;
	for(int i = 0; i < users.size(); i++) {
		users[i].delta += inc;
	}
	// Calculate the second inc
	sort(users.begin(), users.end(), cmp_old_rating_desc);
	int s = min((int)(users.size()), (int)(4*round(sqrt(users.size()))));
	int sum_s = 0;
	for(int i = 0; i < s; i++) {
		sum_s += users[i].delta;
	}
	inc = min(max(-(sum_s / s), -10), 0);
	for(int i = 0; i < users.size(); i++) {
		users[i].delta += inc;
	}
	*/
	// Calculate new rating
	for(int i = 0; i < users.size(); i++) {
		users[i].new_rating = users[i].old_rating + users[i].delta;
	}
	sort(users.begin(), users.end(), cmp_rank_asc);//感觉不需要！！
	///////////////////////////！！2i3个 i 嗯不给你各位 i 虐问你 额 v
}

// Compare calculated new rating with official new rating

double validate() {
	char s[20];
	double diff_count = 0, totDiff=0.0;

	for(int i = 0; i < users.size(); i++) {
		if(users[i].new_rating != users[i].official_new_rating) {
			sprintf(s, " [diff: %5d]", users[i].new_rating - users[i].official_new_rating);
			users[i].validation = string(s);
			diff_count++;
			totDiff += (users[i].new_rating - users[i].official_new_rating) * (users[i].new_rating - users[i].official_new_rating);
		}
	}
	//return diff_count;
	return (double)(totDiff)/users.size();
}

void reassign_rank() {
	int last_idx = 0, last_rank = 1;
	for(int i = 0; i < users.size(); i++) {
		if(users[i].rank > last_rank) {
			for(int j = last_idx; j < i; ++j) {
				users[j].rank = i;
			}
			last_idx = i;
			last_rank = users[i].rank;
		}
	}
	for(int i = last_idx; i < users.size(); i++) {
		users[i].rank = users.size();
	}
}

int main(int argc, char const *argv[]) {
	/*if(argc < 2) {
		printf("Usage: ./file_name [codeforces_contest_id]\n");
		return 1;
	}*/
	for(double K=-1;K<=1;K+=0.1) {
		aveRsq=0;
		for(int i=0;i<20;i++) {
			users.clear();
		 
			//string contest_id = string(argv[1]);
			string contest_id = contest_ids[i];
			string in_file = "tests/cf_rating_official_" + contest_id + ".txt";
			string out_file = "tests/cf_my_rating_result_" + contest_id + ".txt";
			ifstream in(in_file, ios::in);
		
			while(in >> u.rank >> u.handle >> u.old_rating >> u.official_new_rating) {
				users.push_back(u);
			}
			in.close();
			if(users.empty()) {
				printf("Invalid contest\n");
				return 1;
			}
	
			reassign_rank();
			work(K);
			double Rsquare = validate();

			ofstream out(out_file, ios::out);
			out << to_string(users);
			out.close();
			//printf("[Validation] diff count: %d/%d (%.2f%%)\n",diff_count,users.size(),100.0*diff_count/users.size());
			//printf("In contest id %s, R-square for simulation: %.2f\n",contest_id.c_str(),Rsquare);
			aveRsq += Rsquare;
		}
		printf("K=%.2f, average Rsquare value: %.2f\n",K,aveRsq/20);
	}
	return 0;
}