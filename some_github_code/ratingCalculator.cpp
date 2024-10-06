#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>

#define DEBUG printf("passing [%s] in line %d\n",__FUNCTION__,__LINE__);
#define MAXN 1400
//#define PRIVILEGED 10

using namespace std;

struct user {
	double rank;
	string handle;
	int old_rating;
	int new_rating = 0;
	int official_new_rating;
	double seed = 0.5;
	int delta = 0;
	string validation;
} u;

vector<user> users;
double E[MAXN][MAXN],E2[MAXN][MAXN];


string contest_ids[21] = {"1416","1444","1442","1446","1439","1456","1458","1464","1470","1477","1479","1495","1500","1483","1503","1508","1528","1540","1545","1548"};

double aveRsq;

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

void reassign_rank() {
	int last_idx = 0, last_rank = 1;
	for(int i = 0; i < users.size(); i++) {
		if(users[i].rank > last_rank) {
			for(int j = last_idx; j < i; ++j)
				users[j].rank = i;
			last_idx = i;
			last_rank = users[i].rank;
		}
	}
	for(int i = last_idx; i < users.size(); i++) 
		users[i].rank = users.size();
}

double isWin(user &a, user &b) {
	if(a.rank < b.rank) return 1;
	if(a.rank > b.rank) return 0;
	return 0.5;
}

double cal_p(user &a, user &b) {
	return 1.0 / (1.0 + pow(10, (b.old_rating - a.old_rating) / 400.0));
}

double cal_p2(user &a, user &b) {
	return 1.0 / (1.0 + pow(10, (b.new_rating - a.new_rating) / 400.0));
}

void work(double k, double k2) {
	for(int i=0;i<users.size();i++) 
		for(int j=0;j<users.size();j++) {
			E[i][j]=cal_p(users[i], users[j]);
			users[i].seed += 1-E[i][j];
		}
	
	for(int i = 0; i < users.size(); i++) {
		for(int j = 0; j < users.size(); ++j) {
			users[i].seed += cal_p(users[j], users[i]);
		}
	}

	/*for(int i=0;i<PRIVILEGED;i++) 
		users[i].new_rating=users[i].old_rating+round(k2*(users[i].rank-users[i].seed));
	for(int i=PRIVILEGED;i<users.size();i++) 
		users[i].new_rating=users[i].old_rating+round(k*(users[i].rank-users[i].seed));
	*/
	/*
	for(int i = 0; i < users.size(); i ++) {
		users[i].delta = round(k * (users[i].rank - users[i].seed));
	}
	for(int i = 0; i < users.size(); i++) {
		users[i].new_rating = users[i].old_rating + users[i].delta;
	}*/
}

void update(double k, double k2) {
	for(int i=0;i<users.size();i++) 
		for(int j=0;j<users.size();j++) {
			E2[i][j]=cal_p2(users[i], users[j]);
		}
}

// Compare calculated new rating with official new rating
double validate() {
	//char s[20];
	double totDiff=0.0;
	for(int i = 0; i < users.size(); i++) {
		for(int j = i + 1; j < users.size(); j++) {
			if(users[i].rank<users[j].rank) 
				totDiff+=(E2[i][j]-1)*(E2[i][j]-1) + (E2[j][i]-0)*(E2[j][i]-0);
				//totDiff+=(E[i][j]-1)*(E[i][j]-1) + (E[j][i]-0)*(E[j][i]-0);
			else if(users[i].rank>users[j].rank) 
				totDiff+=(E2[i][j]-0)*(E2[i][j]-0) + (E2[j][i]-1)*(E2[j][i]-1);
				//totDiff+=(E[i][j]-0)*(E[i][j]-0) + (E[j][i]-2)*(E[j][i]-2);
			else 
				totDiff+=(E2[i][j]-0.5)*(E2[i][j]-0.5) + (E2[j][i]-0.5)*(E2[j][i]-0.5);
				//totDiff+=(E[i][j]-0.5)*(E[i][j]-0.5) + (E[j][i]-0.5)*(E[j][i]-0.5);
			//totDiff += (E[i][j]- isWin(users[i],users[j])) * (E[i][j]- isWin(users[i],users[j])); 
		}
		/*
		if(users[i].new_rating != users[i].official_new_rating) {
			sprintf(s, " [diff: %5d]", users[i].new_rating - users[i].official_new_rating);
			users[i].validation = string(s);
			diff_count++;
			//totDiff += (users[i].new_rating - users[i].official_new_rating) * (users[i].new_rating - users[i].official_new_rating);
		}
		*/
	}
	//return diff_count;
	int num=users.size();
	int denom=num*(num-1)/2;
	//printf("totDiff=%.8f, denom=%d\n", totDiff, denom);
	//DEBUG
	return (double)(totDiff)/denom;
}

int main(int argc, char const *argv[]) {
	/*if(argc < 2) {
		printf("Usage: ./file_name [codeforces_contest_id]\n");
		return 1;
	}*/

	string out_file = "tests/aveMinSqErr.csv";
	ofstream out(out_file, ios::out | ios::trunc);
	out << "K value" << "," << "Average MinSqErr" << endl;

	for(double K=-2;K<=2;K+=1) {
	for(double K2=0.1;K2<=0.2;K2+=0.1) {
		aveRsq=0;
		for(int i=0;i<20;i++) {
			users.clear();
			//memset(E,0,sizeof(E));
			for(int ii=0;ii<MAXN;ii++) 
				for(int jj=0;jj<MAXN;jj++) 
					E[ii][jj]=0;
 
			//string contest_id = string(argv[1]);
			string contest_id = contest_ids[i];
			string in_file = "tests/cf_rating_official_" + contest_id + ".txt";
			//string out_file = "tests/cf_my_rating_result_" + contest_id + ".txt";
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
			work(K,K2);
			update(K,K2);
			double Rsquare = validate();
			//DEBUG
			//ofstream out(out_file, ios::out);
			//out << to_string(users);
			//out.close();
			//printf("[Validation] diff count: %d/%d (%.2f%%)\n",diff_count,users.size(),100.0*diff_count/users.size());
			//printf("In contest id %s, R-square for simulation: %.2f\n",contest_id.c_str(),Rsquare);
			
			//out << i+1 << "," << Rsquare << endl;

			aveRsq += Rsquare;
		}
		out << K << "," << aveRsq/20 << endl;
		printf("K=%.2f, K2=%.2f, Ave Min Sq Err value: %.8f\n",K,K2,aveRsq/20);
	}
	}
	out.close();
	return 0;
}