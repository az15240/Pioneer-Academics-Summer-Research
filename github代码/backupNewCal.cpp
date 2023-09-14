//這份代碼用原來的單一 K 值

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#define DEBUG printf("passing [%s] in line %d\n",__FUNCTION__,__LINE__);
using namespace std;

struct user {
	double rank;
	string handle;
	int old_rating;
	double new_rating = 0.0;
	int official_new_rating;
	double exp_ranking = 0.5;
	double delta = 0.0;
	string validation;
} u;
vector<user> users;
string contest_ids[21] = {"1416","1444","1442","1446","1439","1456","1458","1464","1470","1477","1479","1495","1500","1483","1503","1508","1528","1540","1545","1548"};
double aveMSE;

string to_string(user &u) {
	char s[200];
	sprintf(s, "%5.0f  %-24s seed: %12.6f  rating: %4d -> %4f vs %4d%s\n",
		round(u.rank),
		u.handle.c_str(),
		u.exp_ranking,
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

//第一个人输掉的概率，也是第二个人赢的概率
double Prob(double rating1, double rating2) {
	return 1.0 / (1.0 + pow(10, (rating1 - rating2) / 400.0));
}

//Function to calculate Elo rating 
//K is a constant. 
//Player A wins over Player B.  
//tie = true if tie, false otherwise
pair<double,double> EloRating(double Ra, double Rb, double k, bool tie) {
	double Pa,Pb,deltA,deltB;
	Pb=Prob(Ra,Rb);//prob of b winning
	Pa=Prob(Rb,Ra);//prob of a winning
	if(tie) {
		deltA=k*(0.5-Pa);
		deltB=k*(0.5-Pb);
	}
	else {
		deltA=k*(1-Pa);
		deltB=k*(0-Pb);
	}
	return make_pair(deltA,deltB);
}

void calExpRanking(double k) {
	for(int i=0;i<users.size();i++) {
		for(int j=0;j<users.size();j++) {
			users[i].exp_ranking+=Prob(users[i].old_rating,users[j].old_rating);
		}
	}
}

void newWork(double k) {
	for(int i=0;i<users.size();i++) {
		for(int j=i+1;j<users.size();j++) {
			pair<double,double> pr;
			if(users[i].rank<users[j].rank) {
				pr=EloRating(users[i].old_rating,users[j].old_rating,k,false);
				users[i].delta+=pr.first;
				users[j].delta+=pr.second;
			}
			else if(users[i].rank>users[j].rank) {
				pr=EloRating(users[j].old_rating,users[i].old_rating,k,false);
				users[j].delta+=pr.first;
				users[i].delta+=pr.second;
			}
			else {
				pr=EloRating(users[i].old_rating,users[j].old_rating,k,true);
				users[i].delta+=pr.first;
				users[j].delta+=pr.second;
			}
		}
	}
}

void update() {
	for(int i=0;i<users.size();i++) {
		users[i].new_rating=users[i].old_rating+users[i].delta;
	}
}

double validate() {
	double tot=0.0;
	for(int i=0;i<users.size();i++) {
		for(int j=0;j<users.size();j++) {
			if(users[i].rank<users[j].rank) {
				tot+=pow(Prob(users[j].new_rating,users[i].new_rating)-1,2);
			}
			else if(users[i].rank>users[j].rank) {
				tot+=pow(Prob(users[j].new_rating,users[i].new_rating)-0,2);
			}
			else {
				tot+=pow(Prob(users[j].new_rating,users[i].new_rating)-0.5,2);
			}
		}
	}
	char s[20];
	for(int i = 0; i < users.size(); i++) {
		if(users[i].new_rating != users[i].official_new_rating) {
			sprintf(s, " [diff: %5f]", users[i].new_rating - users[i].official_new_rating);
			users[i].validation = string(s);
		}
	}
	int amount=users.size()*(users.size()-1)/2;
	return tot/amount;
}

int main() {
	ofstream outAMSE("tests/AMSE.csv", ios::out | ios::trunc);
	outAMSE << "K" << "," << "AMSE" << endl;
	ofstream outMSE("tests/MSE.csv", ios::out | ios::trunc);
	outMSE << "Index" << "," << "MSE" << endl;

	for(double K=1.3;K<=1.5001;K+=0.01) {
		aveMSE=0;
		for(int i=0;i<20;i++) {
			users.clear();
			string contest_id = contest_ids[i];
			string in_file = "tests/cf_rating_official_" + contest_id + ".txt";
			ifstream in(in_file, ios::in);
			while(in >> u.rank >> u.handle >> u.old_rating >> u.official_new_rating) 
				users.push_back(u);
			in.close();
			reassign_rank();
			calExpRanking(K);
			newWork(K);
			update();
			double MinSqErr=validate();
			
			//printf("In contest id %s, MinSqErr for simulation: %.8f\n",contest_id.c_str(),MinSqErr);			
			outMSE << i+1 << "," << MinSqErr << endl;
			
			aveMSE += MinSqErr;
			string out_file = "tests/cf_rating_result_" + contest_id + ".txt";
			ofstream outFile(out_file, ios::out);
			outFile << to_string(users);
			outFile.close();
		}
		outAMSE << K << "," << aveMSE/20 << endl;
		printf("K=%.2f, AveMinSqErr value: %.8f\n",K,aveMSE/20);
	}
	outAMSE.close();
	return 0;
}