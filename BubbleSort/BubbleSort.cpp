#include <iostream>
#include <vector>
using namespace std;

void function(vector<int>& v){
	int n = v.size();
	for(int i = 0; i < n-1; i++){
		for(int j = i; j < n; j++){
		       if (v[i] > v[j]){
		       swap(v[i] , v[j]);
		       }
		}
	}
}

int main (){
	vector <int> v = {3, 5, 11, 9, 6, 72, 12};
	for(int x : v){
		cout << x << ' ';
	}
	function(v);
	for (int x : v){
		cout << x << ' ';
	} 
} 
