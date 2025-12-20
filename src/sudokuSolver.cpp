#include <iostream>
#include <fstream>
#include <string>
using namespace std;

/** 
    Reads a Sudoku puzzle from a file and populates a 9x9 board, where:
        - Digits 1-9 represent filled cells
        - 0s or spaces represent blank cells
    @param fname The name/path of the file containing the puzzle
    @param board The 9x9 puzzle board
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
        cout << "Puzzle read successfully.";
    }
    catch (const std::exception &e) {
        cout << "Something went wrong when reading the file, please try again.";
    }
};

int main() {
    return 0;
}