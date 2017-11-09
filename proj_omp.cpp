#include <bits/stdc++.h>
#include <omp.h>
#include <sys/time.h>
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
	// cout<<a<<endl;
	// cout<<b<<endl;
	vector <string> v;
	for(int i=a.size()-1; i>=0 ;i--)
		v.push_back("");
	int carry =0;
	struct timeval TimeValue_Start;
	struct timezone TimeZone_Start;
	struct timeval TimeValue_Final;
	struct timezone TimeZone_Final;
	long time_start, time_end;
	double time_overhead;
	int chunk = a.size()/4;
	gettimeofday(&TimeValue_Start, &TimeZone_Start);
	#pragma omp parallel shared(a,b,v) firstprivate(carry)
	#pragma omp for schedule(dynamic) nowait
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
	gettimeofday(&TimeValue_Final, &TimeZone_Final);
	time_start = TimeValue_Start.tv_sec * 1000000 + TimeValue_Start.tv_usec;
	time_end = TimeValue_Final.tv_sec * 1000000 + TimeValue_Final.tv_usec;
	time_overhead = (time_end - time_start)/1000.0;
	printf("\n Time in mili Seconds (T) : %lf\n",time_overhead);
	string ret;
	//reverse(v.begin(),v.end());
	for(auto it : v){
		//cout<<it<<" it  "<<endl;
		ret+=it;
	}
	return ret;
}
void randomise(int n,string &s){
    s.resize(n);
	for(int i=0;i<n;i++){
	    s[i]=rand()%10+'0';
	}
}
int main(){
	string a,b;
	int n;
	cin>>n;
	srand(time(NULL));
	randomise(n,a);
	randomise(n,b);
	/*cout<<*/add(a,b);
	return 0;
	// string a,b;
	// cin>>a>>b;
	// // #shared(a,b,v,blockSize) firstprivate(carry)a omp single
	// cout<<add(a,b);
	// return 0;
}
