#include <bits/stdc++.h>
using namespace std;

string pad(string s,int sz){
	reverse(s.begin(),s.end());
	while(s.size() <sz)
		s+='0';
	reverse(s.begin(),s.end());
	return s;
}

string add(string a,string b){
	b=pad(b,max(a.size(),b.size()));
	a=pad(a,max(a.size(),b.size()));
	const int blockSize = 9;
	cout<<a<<endl;
	cout<<b<<endl;
	vector <string> v;
	int carry =0;
	for(int i=a.size()-1; i>=0 ;i-=blockSize){
		int num1 =0 ,num2=0,sum;
		for(int j=max(i-8,0);j<=i;j++){
			num1=num1*10 + a[j]-'0';
			num2=num2*10 + b[j]-'0';
		}
		sum = num1+num2+carry;
		string sumString = to_string(sum);
		if(sumString.size()>9 && i>8){
			carry=sumString[0]-'0';
			sumString.erase(sumString.begin());
		}
		v.push_back(sumString);
	}
	string ret;
	reverse(v.begin(),v.end());
	for(auto it : v){
		ret+=it;
	}
	return ret;
}

int main(){
	string a,b;
	cin>>a>>b;
	cout<<add(a,b);
}