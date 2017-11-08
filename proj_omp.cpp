#include <bits/stdc++.h>
#include <omp.h>
using namespace std;

string pad(string s,int sz){
	reverse(s.begin(),s.end());
	while(s.size() <sz)
		s+='0';
	reverse(s.begin(),s.end());
	return s;
}

void tokenadd(string a, string b,vector<string> &v,int carry,int i){
	const int blockSize = 9;

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
	v[i]=sumString;
	if(carry==1 && i-blockSize>=0){
		tokenadd(a,b,v,1,i-blockSize);
	}
}

string add(string a,string b){
	b=pad(b,max(a.size(),b.size()));
	a=pad(a,max(a.size(),b.size()));
	const int blockSize = 9;
	cout<<a<<endl;
	cout<<b<<endl;
	vector <string> v;
	for(int i=a.size()-1; i>=0 ;i--)
		v.push_back("");
	int carry =0;
	#pragma omp parallel shared(a,b,v) firstprivate(carry)
	#pragma omp for 
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
			//cout<<sumString <<" sumString "<<i<<endl;
			if(v[i]=="")
				v[i]=sumString;
			if(carry==1 && i-blockSize>=0){
				tokenadd(a,b,v,1,i-blockSize);
			}
		}
	string ret;
	//reverse(v.begin(),v.end());
	for(auto it : v){
		//cout<<it<<" it  "<<endl;
		ret+=it;
	}
	return ret;
}

int main(){
	string a,b;
	cin>>a>>b;
	// #shared(a,b,v,blockSize) firstprivate(carry)a omp single
	cout<<add(a,b);
	return 0;
}