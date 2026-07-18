#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include <cstdio>
using namespace std;

const int ROWS = 6, COLS = 6;

struct Car {
    string id;
    int r, c, len;
    bool horizontal;
    bool target;
    char symbol;
};

struct Level {
    vector<Car> cars;
    int exitRow;
    int moveLimit;
};

Car makeCar(string id, int r, int c, int len, bool horizontal, bool isTarget, char symbol) {
    Car car;
    car.id = id;
    car.r = r;
    car.c = c;
    car.len = len;
    car.horizontal = horizontal;
    car.target = isTarget;
    car.symbol = symbol;
    return car;
}

Level makeLevel(vector<Car> cars, int exitRow, int moveLimit) {
    Level lv;
    lv.cars = cars;
    lv.exitRow = exitRow;
    lv.moveLimit = moveLimit;
    return lv;
}

vector<Level> buildLevels() {
    vector<Level> L;
    vector<Car> cs;

    cs.clear();
    cs.push_back(makeCar("T",2,0,2,true ,true ,'T'));
    cs.push_back(makeCar("A",0,2,2,false,false,'A'));
    cs.push_back(makeCar("B",0,4,3,false,false,'B'));
    cs.push_back(makeCar("C",3,1,2,true ,false,'C'));
    cs.push_back(makeCar("D",4,3,2,false,false,'D'));
    cs.push_back(makeCar("E",1,0,2,true ,false,'E'));
    L.push_back(makeLevel(cs, 2, 14));

    cs.clear();
    cs.push_back(makeCar("T",2,1,2,true ,true ,'T'));
    cs.push_back(makeCar("A",0,3,2,false,false,'A'));
    cs.push_back(makeCar("B",2,3,2,false,false,'B'));
    cs.push_back(makeCar("C",4,0,3,true ,false,'C'));
    cs.push_back(makeCar("D",0,0,2,false,false,'D'));
    cs.push_back(makeCar("E",3,4,2,true ,false,'E'));
    cs.push_back(makeCar("F",1,2,2,false,false,'F'));
    L.push_back(makeLevel(cs, 2, 12));

    cs.clear();
    cs.push_back(makeCar("T",2,0,2,true ,true ,'T'));
    cs.push_back(makeCar("A",0,2,2,false,false,'A'));
    cs.push_back(makeCar("B",1,3,3,true ,false,'B'));
    cs.push_back(makeCar("C",2,2,2,false,false,'C'));
    cs.push_back(makeCar("D",4,2,2,true ,false,'D'));
    cs.push_back(makeCar("E",0,0,2,false,false,'E'));
    cs.push_back(makeCar("F",0,4,2,false,false,'F'));
    cs.push_back(makeCar("G",3,4,2,false,false,'G'));
    L.push_back(makeLevel(cs, 2, 15));

    cs.clear();
    cs.push_back(makeCar("T",2,0,2,true ,true ,'T'));
    cs.push_back(makeCar("G",2,3,2,false,false,'G'));
    cs.push_back(makeCar("A",0,1,2,false,false,'A'));
    cs.push_back(makeCar("B",0,3,2,false,false,'B'));
    cs.push_back(makeCar("C",0,5,3,false,false,'C'));
    cs.push_back(makeCar("D",3,0,2,true ,false,'D'));
    cs.push_back(makeCar("E",4,2,3,true ,false,'E'));
    cs.push_back(makeCar("F",5,0,2,true ,false,'F'));
    L.push_back(makeLevel(cs, 2, 20));

    return L;
}

vector<Car> cars;
int exitRow;
int moveLimit;
int moveCount;
vector<vector<int> > grid;

struct UndoEntry { int carIndex, r, c; };
vector<UndoEntry> undoStack;

void rebuildGrid() {
    grid.assign(ROWS, vector<int>(COLS, -1));
    for (int i = 0; i < (int)cars.size(); i++) {
        Car &car = cars[i];
        for (int k = 0; k < car.len; k++) {
            int r = car.horizontal ? car.r : car.r + k;
            int c = car.horizontal ? car.c + k : car.c;
            grid[r][c] = i;
        }
    }
}

bool inBounds(int r, int c) { return r >= 0 && r < ROWS && c >= 0 && c < COLS; }

bool canMove(int idx, int dr, int dc) {
    Car &car = cars[idx];
    int nr = car.r + dr, nc = car.c + dc;
    for (int k = 0; k < car.len; k++) {
        int r = car.horizontal ? nr : nr + k;
        int c = car.horizontal ? nc + k : nc;
        if (!inBounds(r, c)) return false;
        if (grid[r][c] != -1 && grid[r][c] != idx) return false;
    }
    return true;
}

void clearCarFromGrid(int idx) {
    Car &car = cars[idx];
    for (int k = 0; k < car.len; k++) {
        int r = car.horizontal ? car.r : car.r + k;
        int c = car.horizontal ? car.c + k : car.c;
        grid[r][c] = -1;
    }
}
void placeCarOnGrid(int idx) {
    Car &car = cars[idx];
    for (int k = 0; k < car.len; k++) {
        int r = car.horizontal ? car.r : car.r + k;
        int c = car.horizontal ? car.c + k : car.c;
        grid[r][c] = idx;
    }
}

bool tryStep(int idx, char dirChar) {
    int dr = 0, dc = 0;
    if (dirChar == 'w') dr = -1;
    else if (dirChar == 's') dr = 1;
    else if (dirChar == 'a') dc = -1;
    else if (dirChar == 'd') dc = 1;

    Car &car = cars[idx];
    if (car.horizontal && dr != 0) return false;
    if (!car.horizontal && dc != 0) return false;
    if (!canMove(idx, dr, dc)) return false;

    UndoEntry e;
    e.carIndex = idx; e.r = car.r; e.c = car.c;
    undoStack.push_back(e);

    clearCarFromGrid(idx);
    car.r += dr; car.c += dc;
    placeCarOnGrid(idx);
    moveCount++;
    return true;
}

int findTargetIdx() {
    for (int i = 0; i < (int)cars.size(); i++) if (cars[i].target) return i;
    return -1;
}

bool checkWin() {
    int t = findTargetIdx();
    if (t == -1) return false;
    Car &car = cars[t];
    return (car.c + car.len - 1 >= COLS - 1) && (car.r == exitRow);
}

void undoMove() {
    if (undoStack.empty()) { cout << "Undo ke liye kuch nahi hai.\n"; return; }
    UndoEntry e = undoStack.back(); undoStack.pop_back();
    clearCarFromGrid(e.carIndex);
    cars[e.carIndex].r = e.r; cars[e.carIndex].c = e.c;
    placeCarOnGrid(e.carIndex);
    moveCount = max(0, moveCount - 1);
    cout << "Undo ho gaya.\n";
}

void showHint() {
    int t = findTargetIdx();
    if (t == -1) return;
    Car &car = cars[t];
    vector<string> blockers;
    for (int c = car.c + car.len; c < COLS; c++) {
        int o = grid[car.r][c];
        if (o != -1) blockers.push_back(cars[o].id);
    }
    if (blockers.empty()) {
        cout << "Hint: Target seedha EXIT tak ja sakti hai!\n";
    } else {
        cout << "Hint: Pehle inhe raaste se hatao -> ";
        for (size_t i = 0; i < blockers.size(); i++) cout << "Car_" << blockers[i] << " ";
        cout << "\n";
    }
}

void printBoard() {
    vector<vector<char> > disp(ROWS, vector<char>(COLS, '.'));
    for (int i = 0; i < (int)cars.size(); i++) {
        Car &car = cars[i];
        for (int k = 0; k < car.len; k++) {
            int r = car.horizontal ? car.r : car.r + k;
            int c = car.horizontal ? car.c + k : car.c;
            disp[r][c] = car.symbol;
        }
    }
    cout << "\n   ";
    for (int c = 0; c < COLS; c++) cout << c << " ";
    cout << "\n";
    for (int r = 0; r < ROWS; r++) {
        cout << " " << r << " ";
        for (int c = 0; c < COLS; c++) {
            char ch = disp[r][c];
            if (r == exitRow && c == COLS - 1 && ch == '.') ch = 'X';
            cout << ch << " ";
        }
        cout << "\n";
    }
    cout << "(T = Target car, X = EXIT cell, letters = other cars)\n";
    cout << "Moves: " << moveCount << "/" << moveLimit << "\n";
}

struct SnapCar { int r, c; };

string keyOf(const vector<SnapCar> &snap) {
    string k;
    char buf[32];
    for (size_t i = 0; i < snap.size(); i++) {
        sprintf(buf, "%d_%d|", snap[i].r, snap[i].c);
        k += buf;
    }
    return k;
}

bool isGoalSnap(const vector<SnapCar> &snap) {
    int t = findTargetIdx();
    Car &car = cars[t];
    return snap[t].c + car.len - 1 >= COLS - 1;
}

bool canMoveSnap(const vector<vector<int> > &occ, int idx, int dr, int dc, const vector<SnapCar> &snap) {
    Car &car = cars[idx];
    int nr = snap[idx].r + dr, nc = snap[idx].c + dc;
    for (int k = 0; k < car.len; k++) {
        int r = car.horizontal ? nr : nr + k;
        int c = car.horizontal ? nc + k : nc;
        if (!inBounds(r, c)) return false;
        if (occ[r][c] != -1 && occ[r][c] != idx) return false;
    }
    return true;
}

vector<vector<int> > buildOccupancy(const vector<SnapCar> &snap) {
    vector<vector<int> > occ(ROWS, vector<int>(COLS, -1));
    for (int i = 0; i < (int)cars.size(); i++) {
        Car &car = cars[i];
        for (int k = 0; k < car.len; k++) {
            int r = car.horizontal ? snap[i].r : snap[i].r + k;
            int c = car.horizontal ? snap[i].c + k : snap[i].c;
            occ[r][c] = i;
        }
    }
    return occ;
}

struct Move { int idx; char dir; };
struct ParentInfo { string prevKey; Move move; };

bool solvePuzzle(vector<Move> &outMoves) {
    vector<SnapCar> start(cars.size());
    for (int i = 0; i < (int)cars.size(); i++) {
        SnapCar s; s.r = cars[i].r; s.c = cars[i].c;
        start[i] = s;
    }

    if (isGoalSnap(start)) { outMoves.clear(); return true; }

    int dRow[4] = { -1, 1, 0, 0 };
    int dCol[4] = { 0, 0, -1, 1 };
    char dirChars[4] = { 'w','s','a','d' };

    queue<vector<SnapCar> > q;
    q.push(start);
    map<string, ParentInfo> parent;
    set<string> visited;
    string startKey = keyOf(start);
    visited.insert(startKey);

    while (!q.empty()) {
        vector<SnapCar> state = q.front(); q.pop();
        string key = keyOf(state);
        vector<vector<int> > occ = buildOccupancy(state);

        for (int idx = 0; idx < (int)cars.size(); idx++) {
            Car &car = cars[idx];
            for (int d = 0; d < 4; d++) {
                int dr = dRow[d], dc = dCol[d];
                if (car.horizontal && dr != 0) continue;
                if (!car.horizontal && dc != 0) continue;
                if (canMoveSnap(occ, idx, dr, dc, state)) {
                    vector<SnapCar> newState = state;
                    newState[idx].r += dr; newState[idx].c += dc;
                    string newKey = keyOf(newState);
                    if (visited.find(newKey) == visited.end()) {
                        visited.insert(newKey);
                        ParentInfo pi;
                        pi.prevKey = key;
                        pi.move.idx = idx;
                        pi.move.dir = dirChars[d];
                        parent[newKey] = pi;
                        if (isGoalSnap(newState)) {
                            vector<Move> moves;
                            string curKey = newKey;
                            map<string, ParentInfo>::iterator it = parent.find(curKey);
                            while (it != parent.end()) {
                                moves.push_back(it->second.move);
                                curKey = it->second.prevKey;
                                it = parent.find(curKey);
                            }
                            reverse(moves.begin(), moves.end());
                            outMoves = moves;
                            return true;
                        }
                        q.push(newState);
                    }
                }
            }
        }
    }
    return false;
}

void autoSolve() {
    vector<Move> moves;
    bool found = solvePuzzle(moves);
    if (!found) { cout << "Koi solution nahi mila.\n"; return; }
    if (moves.empty()) { cout << "Puzzle already solved hai!\n"; return; }
    cout << "Auto-Solving (" << moves.size() << " moves)...\n";
    for (size_t i = 0; i < moves.size(); i++) {
        Move m = moves[i];
        tryStep(m.idx, m.dir);
        string dirName = (m.dir=='w') ? "UP" : (m.dir=='s') ? "DOWN" : (m.dir=='a') ? "LEFT" : "RIGHT";
        cout << "  Car_" << cars[m.idx].id << " -> " << dirName << "\n";
    }
    printBoard();
    if (checkWin()) cout << "\n *** PUZZLE COMPLETE! *** Total moves: " << moveCount << "\n";
}

int findCarIndexById(const string &id) {
    for (int i = 0; i < (int)cars.size(); i++)
        if (cars[i].id == id) return i;
    return -1;
}

void loadLevel(Level &lv) {
    cars = lv.cars;
    exitRow = lv.exitRow;
    moveLimit = lv.moveLimit;
    moveCount = 0;
    undoStack.clear();
    rebuildGrid();
}

void playLevel(int levelNumber, Level &lv) {
    loadLevel(lv);
    cout << "\n================ LEVEL " << levelNumber << " ================\n";
    cout << "Target car 'T' ko row " << exitRow << " ke EXIT (right side) tak nikalo.\n";
    cout << "Commands: <CarID> <w/a/s/d>  (w=up s=down a=left d=right)\n";
    cout << "          undo | hint | solve | board | quit\n";

    printBoard();
    while (true) {
        cout << "\n> ";
        string cmd;
        if (!(cin >> cmd)) return;

        if (cmd == "quit") { cout << "Level chhod diya.\n"; return; }
        else if (cmd == "undo") { undoMove(); printBoard(); }
        else if (cmd == "hint") { showHint(); }
        else if (cmd == "board") { printBoard(); }
        else if (cmd == "solve") { autoSolve(); if (checkWin()) return; }
        else {
            string id = cmd;
            char dir;
            if (!(cin >> dir)) { cout << "Direction bhi do (w/a/s/d).\n"; continue; }
            int idx = findCarIndexById(id);
            if (idx == -1) { cout << "Aisi koi car nahi hai: " << id << "\n"; continue; }
            Car &car = cars[idx];
            if (car.horizontal && (dir == 'w' || dir == 's')) {
                cout << "Car_" << id << " Horizontal hai - sirf a/d use karo.\n"; continue;
            }
            if (!car.horizontal && (dir == 'a' || dir == 'd')) {
                cout << "Car_" << id << " Vertical hai - sirf w/s use karo.\n"; continue;
            }
            if (tryStep(idx, dir)) {
                printBoard();
                if (checkWin()) {
                    cout << "\n *** PUZZLE COMPLETE! *** Total moves: " << moveCount << "\n";
                    return;
                }
                if (moveLimit && moveCount >= moveLimit) {
                    cout << "\n Move limit khatam ho gayi! Level FAIL.\n";
                    return;
                }
            } else {
                cout << "Blocked! Us taraf nahi ja sakti.\n";
            }
        }
    }
}

int main() {
    vector<Level> levels = buildLevels();
    cout << "=====================================\n";
    cout << "   CAR PARKING PUZZLE (C++ Console)\n";
    cout << "=====================================\n";

    while (true) {
        cout << "\nLevels available: 1 to " << levels.size() << "\n";
        cout << "Level number choose karo (0 = exit): ";
        int choice;
        if (!(cin >> choice)) break;
        if (choice == 0) break;
        if (choice < 1 || choice > (int)levels.size()) {
            cout << "Invalid level number.\n";
            continue;
        }
        playLevel(choice, levels[choice - 1]);
    }

    cout << "Game khatam. Shukriya!\n";
    return 0;
}
