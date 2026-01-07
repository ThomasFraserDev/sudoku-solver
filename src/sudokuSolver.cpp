#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <chrono>
using namespace std;

/** 
 * Reads a Sudoku puzzle from a file and populates a 9x9 board, where:
 * Digits 1-9 represent filled squares
 * 0s or spaces represent blank squares
 * @param fname The name/path of the file containing the puzzle
 * @param board The 9x9 puzzle board
*/
void readPuzzle(string fname, int board[9][9]) {
    ifstream f(fname); // Read from the puzzle file
    try {
        if (!f.is_open()) {
            throw runtime_error("Could not open file: " + fname); // Throw error if the file couldn't be opened
        }

        string line;
        int row = 0;
        while (getline(f, line) && row < 9) { // While there's still lines in the file, and the max amount of rows hasn't been exceeded
            int col = 0;
            for (int i = 0; i < line.length() && col < 9; i++) { // For each value in the line
                char c = line[i];
                if (c == ',') {
                    continue;
                }
                if (isdigit(c)) {
                    board[row][col] = c - '0'; // Append the board with the ASCII value converted to int
                    col++;
                }
                else if (c == ' ') { // Allow spaces to represent blank spaces
                    board[row][col] = 0;
                    col++;
                }
            }
            row++;
        }
        f.close();
        cout << "Puzzle read successfully.\n";
    }
    catch (const std::exception &e) {
        cout << "Something went wrong when reading the file, please try again. ";
    }
};

/**
 * Checks if a specified value is valid at a given position, by checking the position's row, column and 3x3 subsqaure to see if the value is already there
 * @param board The 9x9 puzzle board
 * @param row The row of the position being checked
 * @param col The column of the position being checked
 * @param value The value being checked
*/
bool isValid(int board[9][9], int row, int col, int value) {
    for (int i=0; i < 9; i++) {
        if (value == board[i][col] || value == board[row][i]) { // If the value is already within the position's row or column
            return false;
        }
    }

    int boxRow = (row / 3) * 3; // Calculates the row of the top left square of the subsquare containing (row, col)
    int boxCol = (col / 3) * 3; // Calculates the column of the top left square of the subsquare containing (row, col)

    for (int r = boxRow; r < boxRow + 3; r++) {
        for (int c = boxCol; c < boxCol + 3; c++) {
            if (value == board[r][c]) { // If the value is already within the subsquare
                return false;
            }
        }
    }
    return true;
}

/**
 * Gets all the related squares (within the same row, column or 3x3 subsquare) for a given square at row, col and updates the related list with their positions
 * Filters out duplicates potentially added when searching for related squares within a row/column while searching for related squares within a 3x3 subsquare
 * @param row The row of the square
 * @param col The column of the square
 * @param related Pointer to the list of related squares
 */
void getRelated(int row, int col, vector<pair<int, int>> &related) {
    for (int i = 0; i < 9; i++) {
        if (i != col) {
            related.push_back({row, i}); // Get all squares within the same row
        }
        if (i != row) {
            related.push_back({i, col}); // Get all squares within the same column
        }
    }
    int boxRow = (row / 3) * 3; // Calculates the row of the top left square of the subsquare containing (row, col)
    int boxCol = (col / 3) * 3; // Calculates the column of the top left square of the subsquare containing (row, col)
    for (int i = boxRow; i < boxRow + 3; i++) {
        for (int j = boxCol; j < boxCol + 3; j++) {
            if (i == row && j == col) {
                continue;
            }
            bool duplicate = false;
            for (auto &square : related) {
                if (square.first == i && square.second == j) {
                    duplicate = true; // Avoid adding any duplicates previously added when searching row and column
                    break;
                }
            }
            if (!duplicate) {
                related.push_back({i, j}); // Get all squares within the same subsquare
            }
        }
    }
}

/** 
 * Iterates through values 1-9, and checks if each value is valid at the given row and column, updating the list of valid values as it goes
 * @param board The 9x9 puzzle board
 * @param row The row of the square being checked
 * @param col The column of the square being checked
 * @param validNums Pointer to the list of valid values
*/
void findValid(int board[9][9], int row, int col, vector<int> &validNums) {
    for (int i = 1; i < 10; i++) { // Get a list of all possible valid values at the current empty square
        if (isValid(board, row, col, i)) {
            validNums.push_back(i);
        }
    }
}
/**
 * Updates the list of valid numbers to the current domain, used for MAC solving
 * @param domains The 9x9 list of domains for each square
 * @param row The row of the square being checked
 * @param col The column of the square being checked
 * @param validNums Pointer to the list of valid values
 */
void findValidMAC(vector<int> domains[9][9], int row, int col, vector<int> &validNums) {
    validNums = domains[row][col];
}

/** 
 * Iterates through values 1-9 and counts the number of constraints removed for each square in the given square's row, column and subsqaure for each value
 * The list of valid values is then updated with each valid value, ordered from the least constraining to the highest constraining
 * @param board The 9x9 puzzle board
 * @param row The row of the square being checked
 * @param col The column of the square being checked
 * @param validNums Pointer to the list of valid values
*/
void findValidLCV(int board[9][9], int row, int col, vector<int> &validNums) {
    vector<pair<int, int>> valueConstraints; // Pairs of values and constraint counts
    for (int i = 1; i < 10; i++) {
        if (!isValid(board, row, col, i)) {
            continue; // Skip invalid values
        }
        int constraints = 0;
        board[row][col] = i;
        
        for (int j = 0; j < 9; j++) {
            if (board[row][j] == 0 && j != col) { // Count constraints within each row
                for (int k = 1; k < 10; k++) {
                    if (isValid(board, row, j, k)) {
                        constraints++;
                    }
                }
            }
            if (board[j][col] == 0 && j != row) { // Count constraints within each column
                for (int k = 1; k < 10; k++) {
                    if (isValid(board, j, col, k)) {
                        constraints++;
                    }
                }
            }
        }
        int boxRow = (row / 3) * 3; // Calculates the row of the top left square of the subsquare containing (row, col)
        int boxCol = (col / 3) * 3; // Calculates the column of the top left square of the subsquare containing (row, col)
        for (int r = boxRow; r < boxRow + 3; r++) { // Count constraints within each subsquare
            for (int c = boxCol; c < boxCol + 3; c++) {
                if (board[r][c] == 0 && (r != row || c != col)) {
                    for (int v = 1; v < 10; v++) {
                        if (isValid(board, r, c, v)) {
                            constraints++;
                        }
                    }
                }
            }
        }
        board[row][col] = 0;
        int pos = 0;
        while (pos < valueConstraints.size() && valueConstraints[pos].second <= constraints) { 
             pos++; // Find the position to insert the value and constraints pair, so that the vector is sorted ascendingly 
        }
        valueConstraints.insert(valueConstraints.begin() + pos, {i, constraints}); // Insert the pair at the correct position
    }

    for (auto &p : valueConstraints) {
        validNums.push_back(p.first); // Update validNums with the LCV sorted valid values
    }
}

/** 
 * Iterates through values in a square's domain and counts the number of constraints each value would impose on related squares
 * The list of valid values is then updated with each value, ordered from the least constraining to the most constraining
 * Uses pre-computed domains from AC-3, so calculations are faster than recalculating constraints from scratch
 * @param domains The 9x9 list of domains for each square
 * @param row The row of the square being checked
 * @param col The column of the square being checked
 * @param validNums Pointer to the list of valid values
*/
void findValidLCVMAC(vector<int> domains[9][9], int row, int col, vector<int> &validNums) {
    validNums.clear();
    vector<pair<int, int>> valueConstraints; // Pairs of values and constraint counts
    
    for (int val : domains[row][col]) {
        int constraints = 0; // Count of related squares that would have only 1 choice left
        vector<pair<int, int>> related;
        getRelated(row, col, related);

        for (auto &square : related) {
            int squareRow = square.first;
            int squareCol = square.second;
            if (domains[squareRow][squareCol].empty()) continue;
            
            int supportedCount = 0;
            for (int v : domains[squareRow][squareCol]) {
                if (v != val) supportedCount++;
            }
            
            if (supportedCount == 1) constraints += 100; // Heavy penalty
            else constraints += supportedCount; // Light penalty for remaining options
        }
        
        size_t pos = 0;
        while (pos < valueConstraints.size() && valueConstraints[pos].second <= constraints) pos++;
        valueConstraints.insert(valueConstraints.begin() + pos, {val, constraints});
    }
    
    for (auto &p : valueConstraints) validNums.push_back(p.first);
}

/**
 * Initialising a list of initial domains for each square, by adding the square's set value or, if empty, all its potential values to the list
 * @param board The 9x9 puzzle board
 * @param domains The 9x9 list of domains
 */
void initDomains(int board[9][9], vector<int> domains[9][9]) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            domains[i][j].clear(); // Clear any previously set domains
            if (board[i][j] != 0) {
                domains[i][j].push_back(board[i][j]); // Limit preset squares' domains to their preset value
                continue;
            }
            for (int k = 1; k <= 9; k++) {
                if (isValid(board, i, j, k)) {
                    domains[i][j].push_back(k); // Iteratively add all valid values to the domain
                }
            }
        }
    }
}

/**
 * Updates the domain of squarei by removing any values of the domain where that value is the only value within the domain of squarej
 * Returns true if the domain was updated and false otherwise
 * @param domains The 9x9 list of domains
 * @param squarei The position of the square's domain that's being updated
 * @param squarej the position of the square that squarei is being checked against
 */
bool update(vector<int> domains[9][9], pair<int, int> squarei, pair<int, int> squarej) {
    bool updated = false;
    vector<int> newDomain;
    for (int i : domains[squarei.first][squarei.second]) { // For each value in squarei's domain
        bool valid = false;
        for (int j : domains[squarej.first][squarej.second]) { // For each value in squarej's domain
            if (i != j) {
                valid = true; // The value within squarei's domain is valid if it isn't the only value within squarej's domain
                break;
            }
        }
        if (valid) {
            newDomain.push_back(i);
        }
        else {
            updated = true;
        }
    }
    domains[squarei.first][squarei.second] = newDomain; // Update squarei's domain
    return updated;
}

/**
 * Applies the AC-3 algorithm to enforce arc consistency on all squares, by generating arcs and updating domains
 * Returns false if an inconsistency is detected, and true otherwise
 * @param domains The 9x9 list of domains
 */
bool ac3(vector<int> domains[9][9]) {
    queue<pair<pair<int, int>, pair<int, int>>> arcs;
    vector<pair<int, int>> related;
    for (int i = 0; i < 9; i ++) {
        for (int j = 0; j < 9; j++) {
            getRelated(i, j, related); // Get related squares for each square
            for(auto &square : related) {
                arcs.push({{i, j}, square}); // Add arcs for each square x related square
            }
            related.clear();
        }
    }

    while (!arcs.empty()) {
        auto arc = arcs.front();
        arcs.pop();
        auto squarei = arc.first;
        auto squarej = arc.second;
        if (update(domains, squarei, squarej)) {
            if (domains[squarei.first][squarei.second].empty()) {
                return false; // If the domain is empty, there is an inconsistency
            }
            related.clear();
            getRelated(squarei.first, squarei.second, related);
            for (auto &square : related) {
                if (square.first == squarei.first && square.second == squarei.second) {
                    continue;
                }
                arcs.push({square, squarei}); // Readd arcs with updated domains
            }
        }
    }
    return true;
}

/**
 * Fills squares with single value domains with their value
 * @param board The 9x9 puzzle board
 * @param domains The 9x9 list of domains
 */
void fillSingles(int board[9][9], vector<int> domains[9][9]) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == 0 && domains[i][j].size() == 1) {
                board[i][j] = domains[i][j][0];
            }
        }
    }
}

/** 
 * Iterates through the board, checking for an empty square (represented by 0) and returning its location if found
 * @param board The 9x9 puzzle board
*/
pair<int, int> findEmpty(int board[9][9]) {
    for (int i = 0; i < 9; i++) { // for each row
        for (int j = 0; j < 9; j++) { // for each column
            if (board[i][j] == 0) {
                return {i, j};
            }
        }
    }
    return {-1, -1}; // return impossible location if none found
}

/**
 * Iterates through the board, checking for an empty square (represented by 0) and returning its location if found. Used for MAC solving
 * @param board The 9x9 puzzle board
 * @param domains The 9x9 list of domains for each square
 */
pair<int, int> findEmptyMAC(int board[9][9], vector<int> domains[9][9]) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == 0) {
                return {i, j};
            }
        }
    }
    return {-1, -1};
}

/**
 * Iterates through the board, and uses the Minimum Remaining Value heuristic to determine the next empty square to be filled
 * The function iterates through empty squares on the board, and returns the position of the one with the least remaining valid values
 * @param board The 9x9 puzzle board
 */
pair<int, int> findEmptyMRV(int board[9][9]) {
    int smallest = 10; // Default best number of possible values +1 for comparisons
    pair<int, int> square = {-1, -1};
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] != 0) { // Skip filled squares
                continue;
            }
            vector<int> validNums;
            findValid(board, i, j, validNums);
            if (validNums.size() < smallest) {
                smallest = validNums.size();
                square = {i, j};
                if (smallest == 0 || smallest == 1) { // Exit early if a dead end or the lowest possible amount of valid values is found
                    return square;
                }
            }
        }
    }
    return square;
}
/**
 * Iterates through the board, and uses the Minimum Remaining Value heuristic to determine the next empty square to be filled
 * The function iterates through empty squares on the board, and returns the position of the one with the smallest domain size
 * @param board The 9x9 ppuzzle board
 * @param domains The 9x9 list of domains for each square
 */
pair<int, int> findEmptyMRVMAC(int board[9][9], vector<int> domains[9][9]) {
    int smallest = 10; // Default best number of possible values +1 for comparisons
    pair<int, int> square = {-1, -1};
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] != 0) { // Skip filled squares
                continue;
            }
            int domainSize = domains[i][j].size();
            if (domainSize < smallest) {
                smallest = domainSize;
                square = {i, j};
                if (smallest == 0 || smallest == 1) { // Exit early if a dead end or the lowest possible amount of valid values is found
                    return square;
                }
            }
        }
    }
    return square;
}

/**
 * Checks if any empty squares on the board have no possible remaining values
 * @param board The 9x9 puzzle board
 */
bool hasFuture(int board[9][9]) {
    for(int i = 0; i < 9; i++) {
        for(int j = 0; j < 9; j++) {
            if (board[i][j] != 0) {
                continue;
            }
            bool anyVal = false;
            for (int v = 1; v < 10 ; v++) {
                if (isValid(board, i, j, v)) {
                    anyVal = true;
                }
            }
            if (anyVal == false) {
                return false;
            }
        }
    }
    return true;
}

/**
 * Recursively solves the sudoku using backtracking with pruning, by recursively checking each valid value within each square and backtracking if none exist.
 * Returns true once the board is solved, and returns false if the board is unsolvable.
 * @param board The 9x9 puzzle board
 * @param steps The running total of steps used to solve the puzzle
 * @param backtracks The running total of backtracks used when solving the puzzle
 * @param nextEmpty The function used to find the next empty square, decided by user input
 * @param validNumFinder The function used to find and order the valid numbers that make up a square's domain
*/
bool pruning(int board[9][9], int &steps, int &backtracks, function<pair<int, int>(int[9][9])> nextEmpty, function<void(int[9][9], int, int, vector<int>&)> validNumFinder) {
    pair<int, int> emptysquare = nextEmpty(board);
    if (emptysquare == make_pair(-1, -1)) {
        return true; // If no empty squares remain, assume the board to be solved
    }
    int row = emptysquare.first;
    int col = emptysquare.second;
    steps++;

    vector<int> validNums;
    validNumFinder(board, row, col, validNums);

    for (int i=0; i < validNums.size(); i++) { // Recursively place valid numbers into empty positions until the board is solved
        board[row][col] = validNums[i];
        if (pruning(board, steps, backtracks, nextEmpty, validNumFinder)) {
            return true;
        }
        else {
            backtracks++;
            board[row][col] = 0;
        }
    }
    return false;
}

/**
 * Recursively solves the sudoku using backtracking with forward checking, by placing a valid value within a square then checking if doing so elimates all valid values for any other squares
 * Returns true once the board is solved, and returns false if the board is unsolvable.
 * @param board The 9x9 puzzle board
 * @param steps The running total of steps used to solve the puzzle
 * @param backtracks The running total of backtracks used when solving the puzzle
 * @param nextEmpty The function used to find the next empty square, decided by user input
 * @param validNumFinder The function used to find and order the valid numbers that make up a square's domain
*/
bool forwardChecking(int board[9][9], int &steps, int &backtracks, function<pair<int, int>(int[9][9])> nextEmpty, function<void(int[9][9], int, int, vector<int>&)> validNumFinder) {
    pair<int, int> emptysquare = nextEmpty(board);
    if (emptysquare == make_pair(-1, -1)) {
        return true; // If no empty squares remain, assume the board to be solved
    }
    int row = emptysquare.first;
    int col = emptysquare.second;
    steps++;

    vector<int> validNums;
    validNumFinder(board, row, col, validNums);

    for(int i = 0; i < validNums.size(); i++) { // Recursively place valid numbers into empty positions until the board is solved
        board[row][col] = validNums[i];
        if (!hasFuture(board)) { // If placing a value into this square eliminates all possible values for any other square, backtrack
            board[row][col] = 0;
            backtracks++;
            continue;
        }
        if (forwardChecking(board, steps, backtracks, nextEmpty, validNumFinder)) {
            return true;
        }
        else {
            backtracks++;
            board[row][col] = 0;
        }
    }
    return false;
}

/**
 * Recursively solves the sudoku using backtracking with pruning and MAC, by placing a valid value within a square
 * The algorithm then checks all related squares domains to detect if all values have been eliminated, and prunes the path if so
 * Returns true once the board is solved, and returns false if the board is unsolvable.
 * @param board The 9x9 puzzle board
 * @param domains The 9x9 list of domains for each square
 * @param steps The running total of steps used to solve the puzzle
 * @param backtracks The running total of backtracks used when solving the puzzle
 * @param nextEmpty The function used to find the next empty square, decided by user input
 * @param validNumFinder The function used to find and order the valid numbers that make up a square's domain
*/
bool pruningMAC(int board[9][9], vector<int> domains[9][9], int &steps, int &backtracks, function<pair<int, int>(int[9][9], vector<int>[9][9])> nextEmpty, function<void(vector<int>[9][9], int, int, vector<int>&)> validNumFinder) {
    pair<int, int> emptysquare = nextEmpty(board, domains);
    if (emptysquare == make_pair(-1, -1)) {
        return true; // If no empty squares remain, assume the board to be solved
    }
    int row = emptysquare.first;
    int col = emptysquare.second;
    steps++;

    vector<int> validNums;
    validNumFinder(domains, row, col, validNums);

    for(int i = 0; i < validNums.size(); i++) {
        vector<int> domainsCopy[9][9];
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                domainsCopy[i][j] = domains[i][j];
            }
        }
        board[row][col] = validNums[i];
        domainsCopy[row][col] = {validNums[i]};

        if (ac3(domainsCopy)) {
            if (pruningMAC(board, domainsCopy, steps, backtracks, nextEmpty, validNumFinder)) {
                for (int i = 0; i < 9; i++)
                    for (int j = 0; j < 9; j++)
                        domains[i][j] = domainsCopy[i][j];
                return true;
            }
        }
        backtracks++;
        board[row][col] = 0;
    }
    return false;
}

/** 
 * Prints the solved puzzle, with subsquares separated by -s and |s 
 * @param board The 9x9 puzzle board
*/
void printBoard(int board[9][9]) {
    for (int i = 0; i < 9 ; i++) {
        if (i % 3 == 0 && i != 0) {
            cout << "- - - - - - - - - - -\n";
        }
        for (int j = 0; j < 9; j++) {
            if (j % 3 == 0 && j != 0) {
                cout << "| ";
            }
            cout << board[i][j];
            if (j != 8) {
                cout << " ";
            }
        }
        cout << "\n";
    }
}
/** 
 * Stores the outcome of a solver run, including the final board state and metrics
 * @param board The 9x9 solved (or attempted) puzzle board
 * @param solved Whether the solver found a valid solution
 * @param steps The total number of recursion steps performed
 * @param backtracks The total number of backtracks performed
 * @param runtime The elapsed solving time in milliseconds
 */
struct SolveResult {
    int board[9][9];
    bool solved;
    int steps;
    int backtracks;
    int runtime;
};

/**
 * Solves a given sudoku board. Gets input from the user detailing which solve method, empty cell finding heuristic, value ordering heuristic and whether or not they want to use AC-3 preprocessing
 * The function returns a SolveResult, containing the details of the performed solve
 * @param board The 9x9 puzzle board
 */
SolveResult solve(int board[9][9]) {
    bool solved = false;
    int steps = 0;
    int backtracks = 0;
    int method;
    int emptyFinder;
    int valueOrder;
    int useAC3;
    vector<int> domains[9][9];
    cout << "Select an approach: \n [1] Backtracking with pruning \n [2] Backtracking with forward checking \n [3] Backtracking with pruning and MAC (Maintained Arc Consistency) \n";
    cin >> method;
    cout << "Select empty cell finding heuristic: \n [1] None (first empty) \n [2] MRV (Minimum Remaining Values) \n";
    cin >> emptyFinder;
    cout << "Select value ordering heuristic: \n [1] Basic (no ordering) \n [2] LCV (Least Constraining Value) \n";
    cin >> valueOrder;
    if (method < 3) { // If MAC isn't being used
        cout << "Apply AC-3 preprocessing? \n [1] Yes \n [2] No \n";
        cin >> useAC3;
    }
    if(useAC3 == 1 || method == 3) {
        initDomains(board, domains);
        if (!ac3(domains)) {
            cout << "No solution exists for the entered sudoku (AC-3 detected an inconsistency).";
            return {-1, -1, -1};
        }
        fillSingles(board, domains);
    }
    auto start = chrono::steady_clock::now(); // Begin tracking runtime
    if (method == 1 and emptyFinder == 1 and valueOrder == 1) {
        solved = pruning(board, steps, backtracks, findEmpty, findValid);
    }
    else if (method == 1 and emptyFinder == 1 and valueOrder == 2) {
        solved = pruning(board, steps, backtracks, findEmpty, findValidLCV);
    }
    else if (method == 1 and emptyFinder == 2 and valueOrder == 1) {
        solved = pruning(board, steps, backtracks, findEmptyMRV, findValid);
    }
    else if (method == 1 and emptyFinder == 2 and valueOrder == 2) {
        solved = pruning(board, steps, backtracks, findEmptyMRV, findValidLCV);
    }
    else if (method == 2 and emptyFinder == 1 and valueOrder == 1) {
        solved = forwardChecking(board, steps, backtracks, findEmpty, findValid);
    }
    else if (method == 2 and emptyFinder == 1 and valueOrder == 2) {
        solved = forwardChecking(board, steps, backtracks, findEmpty, findValidLCV);
    }
    else if (method == 2 and emptyFinder == 2 and valueOrder == 1) {
        solved = forwardChecking(board, steps, backtracks, findEmptyMRV, findValid);
    }
    else if (method == 2 and emptyFinder == 2 and valueOrder == 2) {
        solved = forwardChecking(board, steps, backtracks, findEmptyMRV, findValidLCV);
    }
    else if (method == 3 and emptyFinder == 1 and valueOrder == 1) {
        solved = pruningMAC(board, domains, steps, backtracks, findEmptyMAC, findValidMAC);
    }
    else if (method == 3 and emptyFinder == 1 and valueOrder == 2) {
        solved = pruningMAC(board, domains, steps, backtracks, findEmptyMAC, findValidLCVMAC);
    }
    else if (method == 3 and emptyFinder == 2 and valueOrder == 1) {
        solved = pruningMAC(board, domains, steps, backtracks, findEmptyMRVMAC, findValidMAC);
    }
    else if (method == 3 and emptyFinder == 2 and valueOrder == 2) {
        solved = pruningMAC(board, domains, steps, backtracks, findEmptyMRVMAC, findValidLCVMAC);
    }
    auto end = chrono::steady_clock::now(); // Finish tracking runtime
    auto runtime = chrono::duration_cast<chrono::milliseconds>(end - start).count(); // Calculate runtime
    SolveResult result{};
    for (int r = 0; r < 9; r++)
        for (int c = 0; c < 9; c++)
            result.board[r][c] = board[r][c];
    result.solved = solved;
    result.steps = steps;
    result.backtracks = backtracks;
    result.runtime = runtime;
    return result;
}
/**
 * Compares multiple solvers, determined by the user. Each solver's stats are then printed, along with the least steps, backtracks and fastest runtime
 * @param board The 9x9 puzzle board
 */
void comparison(int board[9][9]) {
    int solvers;
    cout << "Enter how many solvers you would like to run: \n";
    cin >> solvers;
    vector<SolveResult> results;
    results.reserve(solvers);
    for (int i = 0; i < solvers; i++) {
        int boardCopy[9][9];
        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                boardCopy[r][c] = board[r][c];
            }
        }
        cout << "----- Solver " << (i +1) << " ----- \n";
        results[i] = solve(boardCopy);
    }

    pair<int, SolveResult> leastSteps = {0, results[0]};
    pair<int, SolveResult> leastBacktracks = {0, results[0]};
    pair<int, SolveResult> fastest = {0, results[0]};

    for (int i = 0; i < solvers; i++) {
        cout << "----- Solver " << (i +1) << " ----- \n";
        if (results[i].solved) {
            cout << "Solved Board:\n";
            printBoard(results[i].board);
            cout << "Steps: " << results[i].steps << "\n";
            if (results[i].steps < leastSteps.second.steps) {
                leastSteps.first = (i +1);
                leastSteps.second = results[i];
            }
            cout << "Backtracks: " << results[i].backtracks << "\n";
            if (results[i].backtracks < leastBacktracks.second.backtracks) {
                leastBacktracks.first = (i +1);
                leastBacktracks.second = results[i];
            }
            cout << "Runtime: " << results[i].runtime << "ms \n";
            if (results[i].runtime < fastest.second.runtime) {
                fastest.first = (i +1);
                fastest.second = results[i];
            }
        }
        else {
            cout << "No solution exists for the entered sudoku.\n";
            break;
        }
    }
    cout << "---------- \n";
    cout << "Solver that used the least amount of steps: " << leastSteps.first << " (" << leastSteps.second.steps << " steps)\n";
    cout << "Solver that backtracked the least: " << leastBacktracks.first << " (" << leastBacktracks.second.backtracks << " backtracks)\n";
    cout << "Solver that solved the puzzle the fastest: " << fastest.first << " (" << fastest.second.runtime << "ms)\n";
}

/**
 * Main function that takes input for the name of the sudoku puzzle file and whether single solve or comparison solve is to be used
 * If single solve is used, the function will also print the solver's metrics
 */
int main() {
    int board[9][9] = {};
    string fileName;
    int mode;
    cout << "Enter sudoku puzzle file name: \n";
    cin >> fileName;
    readPuzzle("puzzles/" + fileName, board);
    cout << "Choose a mode: \n [1] Solve a sudoku using a solver \n [2] Compare multiple solvers \n";
    cin >> mode;
    if (mode == 1) {
        SolveResult result{};
        result = solve(board);
        if (result.solved) {
            cout << "Solved Board:\n";
            printBoard(result.board);
            cout << "Steps: " << result.steps << "\n";
            cout << "Backtracks: " << result.backtracks << "\n";
            cout << "Runtime: " << result.runtime << "ms\n";
        }
        else {
            cout << "No solution exists for the entered sudoku.\n";
        }
    }
    else if (mode == 2) {
        comparison(board);
    }
    return 0;
}