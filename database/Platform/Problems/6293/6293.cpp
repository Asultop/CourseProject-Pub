#include <iostream>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n; 
    cin >> n;
    int cnt = 0;
    for (int i = 0; i < n; i++) {
        int x; 
        cin >> x;
        if (x != 1) cnt++;
    }
    cout << cnt << '\n';
    return 0;
}