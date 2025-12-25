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
 * Recursively solves the sudoku using backtracking with pruning, by recursively checking each valid value within each position and backtracking if none exist.
 * Returns true once the board is solved, and returns false if the board is unsolvable.
 * @param board The 9x9 puzzle board
*/
bool solve(int board[9][9], int &steps, int &backtracks) {
    pair<int, int> emptyCell = findEmpty(board);
    if (emptyCell == make_pair(-1, -1)) {
        return true; // If no empty cells remain, assume the board to be solved
    }
    int row = emptyCell.first;
    int col = emptyCell.second;
    steps += 1;

    vector<int> validNums;
    for (int i = 1; i < 10; i++) { // Get a list of all possible valid values at the current empty cell
        if (isValid(board, row, col, i)) {
            validNums.push_back(i);
        }
    }

    for (int i=0;i < validNums.size(); i++) { // Recursively place valid numbers into empty positions until the board is solved
        board[row][col] = validNums[i];
        if (solve(board, steps, backtracks)) {
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

int main() {
    int board[9][9] = {};
    string fileName;
    cout << "Enter file name: ";
    cin >> fileName;
    readPuzzle("puzzles/" + fileName, board);
    int steps = 0;
    int backtracks = 0;
    auto start = chrono::steady_clock::now();
    bool solved = solve(board, steps, backtracks);
    auto end = chrono::steady_clock::now();
    auto elapsed_ms = chrono::duration_cast<chrono::milliseconds>(end - start).count();
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