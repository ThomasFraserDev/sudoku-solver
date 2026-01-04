#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
using namespace std;

/** 
 * Reads a Sudoku puzzle from a file and populates a 9x9 board, where:
 *  - Digits 1-9 represent filled cells
 *  - 0s or spaces represent blank cells
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
        if (value == board[i][col] or value == board[row][i]) { // If the value is already within the position's row or column
            return false;
        }
    }

    int boxRow = (row / 3) * 3; // Calculates the row of the top left cell of the sub-square containing (row, col)
    int boxCol = (col / 3) * 3; // Calculates the column of the top left cell of the sub-square containing (row, col)

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
 * Iterates through values 1-9, and checks if each value is valid at the given row and column, updating the list of valid values as it goes
 * @param board The 9x9 puzzle board
 * @param row The row of the cell being checked
 * @param col The column of the cell being checked
 * @param validNums Pointer to the list of valid values
*/
void findValid(int board[9][9], int row, int col, vector<int> &validNums) {
    for (int i = 1; i < 10; i++) { // Get a list of all possible valid values at the current empty cell
        if (isValid(board, row, col, i)) {
            validNums.push_back(i);
        }
    }
}

/** 
 * Iterates through values 1-9 and counts the number of constraints removed for each cell in the given cell's row, column and subsqaure for each value
 * The list of valid values is then updated with each valid value, ordered from the least constraining to the highest constraining
 * @param board The 9x9 puzzle board
 * @param row The row of the cell being checked
 * @param col The column of the cell being checked
 * @param validNums Pointer to the list of valid values
*/
void findValidLCV(int board[9][9], int row, int col, vector<int> &validNums) {
    vector<pair<int, int>> valueConstraints; // pairs of values and constraint counts
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
        int boxRow = (row / 3) * 3; // Calculates the row of the top left cell of the sub-square containing (row, col)
        int boxCol = (col / 3) * 3; // Calculates the column of the top left cell of the sub-square containing (row, col)
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
 * Iterates through the board, checking for an empty square (0) and returning its location if found
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
 * Iterates through the board, and uses the Minimum Remaining Value heuristic to determine the next empty cell to be filled
 * The function iterates through empty cells on the board, and returns the position of the one with the least remaining valid values
 * @param board The 9x9 puzzle board
 */
pair<int, int> findEmptyMRV(int board[9][9]) {
    int highest = 10; // Default best number of possible values +1 for comparisons
    pair<int, int> cell = {-1, -1};
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] != 0) { // Skip filled cells
                continue;
            }
            vector<int> validNums;
            findValid(board, i, j, validNums);
            if (validNums.size() < highest) {
                highest = validNums.size();
                cell = {i, j};
                if (highest == 0 or highest == 1) { // Exit early if a dead end or the lowest possible amount of valid values is found
                    return cell;
                }
            }
        }
    }
    return cell;
}

/**
 * Checks if any empty cells on the board have no possible remaining values
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
 * Recursively solves the sudoku using backtracking with pruning, by recursively checking each valid value within each position and backtracking if none exist.
 * Returns true once the board is solved, and returns false if the board is unsolvable.
 * @param board The 9x9 puzzle board
 * @param steps The running total of steps used to solve the puzzle
 * @param backtracks The running total of backtracks used when solving the puzzle
*/
bool pruning(int board[9][9], int &steps, int &backtracks, function<pair<int, int>(int[9][9])> nextEmpty, function<void(int[9][9], int, int, vector<int>&)> validNumFinder) {
    pair<int, int> emptyCell = nextEmpty(board);
    if (emptyCell == make_pair(-1, -1)) {
        return true; // If no empty cells remain, assume the board to be solved
    }
    int row = emptyCell.first;
    int col = emptyCell.second;
    steps += 1;

    vector<int> validNums;
    validNumFinder(board, row, col, validNums);

    for (int i=0; i < validNums.size(); i++) { // Recursively place valid numbers into empty positions until the board is solved
        board[row][col] = validNums[i];
        if (pruning(board, steps, backtracks, nextEmpty, validNumFinder)) {
            return true;
        }
        else {
            backtracks += 1;
            board[row][col] = 0;
        }
    }
    return false;
}

/**
 * Recursively solves the sudoku using forward checking, by placing a valid value within in a cell then checking if doing so elimates all valid values for any other cells
 * Returns true once the board is solved, and returns false if the board is unsolvable.
 * @param board The 9x9 puzzle board
 * @param steps The running total of steps used to solve the puzzle
 * @param backtracks The running total of backtracks used when solving the puzzle
*/
bool forwardChecking(int board[9][9], int &steps, int &backtracks, function<pair<int, int>(int[9][9])> nextEmpty, function<void(int[9][9], int, int, vector<int>&)> validNumFinder) {
    pair<int, int> emptyCell = nextEmpty(board);
    if (emptyCell == make_pair(-1, -1)) {
        return true; // If no empty cells remain, assume the board to be solved
    }
    int row = emptyCell.first;
    int col = emptyCell.second;
    steps += 1;

    vector<int> validNums;
    validNumFinder(board, row, col, validNums);

    for(int i = 0; i < validNums.size(); i++) { // Recursively place valid numbers into empty positions until the board is solved
        board[row][col] = validNums[i];
        if (!hasFuture(board)) { // If placing a value into this cell eliminates all possible values for any other cell, backtrack
            board[row][col] = 0;
            backtracks += 1;
            continue;
        }
        if (forwardChecking(board, steps, backtracks, nextEmpty, validNumFinder)) {
            return true;
        }
        else {
            backtracks += 1;
            board[row][col] = 0;
        }
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
 * Main function that handles the CLI and other function calls
 */
int main() {
    int board[9][9] = {};
    bool solved = false;
    string fileName;
    cout << "Enter sudoku puzzle file name: ";
    cin >> fileName;
    readPuzzle("puzzles/" + fileName, board);
    int steps = 0;
    int backtracks = 0;
    int method;
    int heuristic;
    int valueOrder;
    cout << "Select an approach: \n [1] Backtracking with pruning \n [2] Backtracking with forward checking \n";
    cin >> method;
    cout << "Select a heuristic: \n [1] None (first empty) \n [2] MRV (minimum remaining values) \n [3] LCV (least constraining value) \n [4] MRV + LCV";
    cin >> heuristic;
    cout << "Select value ordering heuristic: \n [1] Basic (no ordering) \n [2] LCV (least constraining value) \n";
    cin >> valueOrder;
    auto start = chrono::steady_clock::now(); // Begin tracking runtime
    if (method == 1 and heuristic == 1 and valueOrder == 1) {
        solved = pruning(board, steps, backtracks, findEmpty, findValid);
    }
    else if (method == 1 and heuristic == 1 and valueOrder == 2) {
        solved = pruning(board, steps, backtracks, findEmpty, findValidLCV);
    }
    else if (method == 1 and heuristic == 2 and valueOrder == 1) {
        solved = pruning(board, steps, backtracks, findEmptyMRV, findValid);
    }
    else if (method == 1 and heuristic == 2 and valueOrder == 2) {
        solved = pruning(board, steps, backtracks, findEmptyMRV, findValidLCV);
    }
    else if (method == 2 and heuristic == 1 and valueOrder == 1) {
        solved = forwardChecking(board, steps, backtracks, findEmpty, findValid);
    }
    else if (method == 2 and heuristic == 1 and valueOrder == 2) {
        solved = forwardChecking(board, steps, backtracks, findEmpty, findValidLCV);
    }
    else if (method == 2 and heuristic == 2 and valueOrder == 1) {
        solved = forwardChecking(board, steps, backtracks, findEmptyMRV, findValid);
    }
    else if (method == 2 and heuristic == 2 and valueOrder == 2) {
        solved = forwardChecking(board, steps, backtracks, findEmptyMRV, findValidLCV);
    }
    auto end = chrono::steady_clock::now(); // Finish tracking runtime
    auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count(); // Calculate runtime
    if (solved) {
        cout << "Solved Board:\n";
        printBoard(board);
        cout << "Steps: " << steps << "\n";
        cout << "Backtracks: " << backtracks << "\n";
        cout << "Runtime: " << elapsed_ms << " ms\n";
    }
    else {
        cout << "No solution exists for the entered sudoku.\n";
        cout << "Runtime: " << elapsed_ms << "ms\n";
    }
    return 0;
}