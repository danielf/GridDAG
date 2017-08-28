#include <bits/stdc++.h>
using namespace std;
class all_trees {
  // input /////////////////////////////////////////////////////////////////////////////
  // l[i][j] is the length of the diagonal arc with head at (i, j).
  // If no such arc exists use l[i][j] = -infinity (aka l[i][j] < -L)
  const vector<vector<int>>& l;
  int L;
  int N, M;

  // output ////////////////////////////////////////////////////////////////////////////
  //   timeH and timeD encode the longest path trees
  vector<vector<int>> timeH, timeD; 

  // internal variables ////////////////////////////////////////////////////////////////
  // valD and valV compare the diagonal and vertical options with the horizontal option.
  //   valD[i][j] = S_a^k[i-1][j-1] + l_a^k[i][j] - S_a^k[i][j-1]
  //   valV[i][j] = S_a^k[i-1][j] - S_a^k[i][j-1]
  vector<vector<int>> valD, valV;
  // Border points. Maintain it to avoid memory allocations.
  vector<int> B;
  
  // Helper functions //////////////////////////////////////////////////////////////////
  // Set the parent pointer at a certain time
  void setVertical(int i, int j, int a) {}
  void setDiagonal(int i, int j, int a) {
    timeD[i][j] = min(timeD[i][j], a);
  }
  void setHorizontal(int i, int j, int a) {
    timeD[i][j] = min(timeD[i][j], a);
    timeH[i][j] = min(timeH[i][j], a);
  }

	// Update "P" (ie, timeD and timeH) based on valV and valD
	void updateP(int i, int j, int a) {
		int op = max({0, valV[i][j], valD[i][j]});
		if (op == 0) setHorizontal(i, j, a);
		else if (op == valD[i][j]) setDiagonal(i, j, a);
		else setVertical(i, j, a);
	}
  // return parent family:
  //   0 - horizontal
  //   1 - diagonal
  //   2 - vertical
  public:
  int getParentFamilyAt(int i, int j, int a) const {
    if (a < timeD[i][j]) return 2;
    if (a < timeH[i][j]) return 1;
    return 0;
  }

  private:
  // find the next border value
  int findFirstNotHorizontal(int i, int j, int a) {
    for (; j <= M; j++) {
      if (getParentFamilyAt(i, j, a) != 0) return j;
    }
    return M + 1; // Didn't find
  }

  // main part, actually use the theorems from the paper :)
  void advance(int a) {
    B[a] = 0;
    B[a + 1] = findFirstNotHorizontal(a + 1, 1, a + 1);
    for (int i = a + 2; i <= N; i++) {
      if (getParentFamilyAt(i, B[i - 1], a + 1) == 2) B[i] = B[i - 1];
      else B[i] = findFirstNotHorizontal(i, B[i - 1] + 1, a + 1);
      if (B[i] == M + 1) break; // no need to continue
    }
    // Now that we have all B, advance the internal data structures
    for (int i = a + 1; i <= N && B[i] != M + 1; i++) {
      valV[i][B[i]]--;
      if (B[i - 1] <= B[i] - 1) valD[i][B[i]]--;
      if (i < N && B[i + 1] > B[i]) valV[i + 1][B[i]]--;
			updateP(i, B[i], a + 1);
    }
  }

  // Traditional DP, not very interesting.
  void computeT0() {
    vector<vector<int>> S(N + 1, vector<int>(M + 1)); // needed only for the initial representation
    S[0][0] = 0; // The parent is not defined here
    // base case: first row and column
    for (int j = 1; j <= M; j++) {
      S[0][j] = 0;
      setHorizontal(0, j, 0);
    }
    for (int i = 1; i <= N; i++) {
      S[i][0] = 0;
      setVertical(i, 0, 0);
    }
    // main part
    for (int i = 1; i <= N; i++) for (int j = 1; j <= M; j++) {
      // compute the three options
      int opH = S[i][j - 1];
      int opD = S[i - 1][j - 1] + l[i][j];
      int opV = S[i - 1][j];
			S[i][j] = max({opH, opD, opV});
      valD[i][j] = opD - opH;
      valV[i][j] = opV - opH;
			updateP(i, j, 0);
    }
  }
  public:
  // constructor, main algorithm
  all_trees(const vector<vector<int>>& l) : l(l) {
    N = l.size() - 1;
    M = l[0].size() - 1;
    // Compute L
    L = 0;
    for (auto & x : l) for (int y : x) L = max(L, y);
    // initialize the internal variables:
    valD.assign(N + 1, vector<int>(M + 1));
    valV = valD;
    timeD.assign(N + 1, vector<int>(M + 1, N + 5));
    timeH = timeD;
    B.assign(N + 1, 0);
    // Compute T0
    computeT0();
    // Now compute all other values
    for (int a = 0; a < N; a++) {
      for (int times = 0; times < L; times++) advance(a);
    }
  }
};

void check(int a, const vector<vector<int>> &l, const all_trees& t) {
  // Compute the ``correct answer''
  int N = l.size() - 1, M = l[0].size() - 1;
  vector<vector<int>> S(N + 1, vector<int>(M + 1, 0));
  auto family = S;
  // do first row and column
  for (int i = a + 1; i <= N; i++) family[i][0] = 2;
  for (int j = 1; j <= M; j++) family[a][j] = 0;
  // do the rest
  for (int i = a + 1; i <= N; i++) for (int j = 1; j <= M; j++) {
    int opH = S[i][j - 1];
    int opD = S[i - 1][j - 1] + l[i][j];
    int opV = S[i - 1][j];
    S[i][j] = max({opH, opD, opV});
    if (S[i][j] == opH) family[i][j] = 0;
    else if (S[i][j] == opD) family[i][j] = 1;
    else family[i][j] = 2;
  }
  array<int, 3> c{0, 0, 0};
  int correct = 0;
  int incorrect = 0;
  for (int i = a; i <= N; i++) for (int j = 0; j <= M; j++) if (i > a || j > 0) {
    int expect = family[i][j];
    int real = t.getParentFamilyAt(i, j, a);
    c[expect]++;
    if (real == expect) correct++;
    else incorrect++;
  }
  printf("%d: [%d %d %d], %d correct, %d incorrect\n", a, c[0], c[1], c[2], correct, incorrect);
  if (incorrect > 0) printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*&^*&!^*&!^*&!^*&!^!&!!!!!\n");
}

// Main function to check. Generate a random NxM with maximum size L.
int main(int argc, char * argv[]) {
	if (argc != 4) {
		printf("Usage: %s N M L\n", argv[0]);
		return 1;
	}
  int N = atoi(argv[1]), M = atoi(argv[2]);
  int L = atoi(argv[3]);
  // initialize l with - infinity
  vector<vector<int>> l(N + 1, vector<int>(M + 1, -N*L*L));
  mt19937 engine;
  uniform_int_distribution<int> generateL(1, L), decide(0, 1);

  for (int i = 1; i <= N; i++) for (int j = 1; j <= M; j++)
    if (decide(engine) == 1) l[i][j] = generateL(engine);

  clock_t before = clock();
  all_trees T(l);
  clock_t after = clock();

  clock_t before2 = clock();
  for (int a = 0; a <= N; a++) check(a, l, T);
  clock_t after2 = clock();

  fprintf(stderr, "Time: paper = %.3f s, normal = %.3f s\n", (double)(after - before) / CLOCKS_PER_SEC, (double)(after2 - before2) / CLOCKS_PER_SEC);

  return 0;
}
