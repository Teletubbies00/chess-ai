#include <iostream>
using namespace std;

// 棋子代號，和之前一樣
enum Piece {
    EMPTY = 0,
    WP = 1,
    WN = 2,
    WB = 3,
    WR = 4,
    WQ = 5,
    WK = 6,
    BP = -1,
    BN = -2,
    BB = -3,
    BR = -4,
    BQ = -5,
    BK = -6
};

// Board 類別：專門負責「棋盤狀態」相關的事情
class Board {
   public:
    int board[8][8];  // 8x8 棋盤

    // 建構子：建立物件時自動呼叫
    Board() {
        clear();
        initStartPosition();  // 先做一個起始局面
    }

    // 把整個棋盤清成 EMPTY
    void clear() {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                board[r][c] = EMPTY;
            }
        }
    }

    // 設定起始局面（就是你剛剛手動塞的那一段）
    void initStartPosition() {
        // 先清空，避免髒資料
        clear();

        // 黑棋
        int black_back_rank[8] = {BR, BN, BB, BQ, BK, BB, BN, BR};
        int black_pawns[8] = {BP, BP, BP, BP, BP, BP, BP, BP};

        for (int c = 0; c < 8; c++) {
            board[0][c] = black_back_rank[c];
            board[1][c] = black_pawns[c];
        }

        // 白棋
        int white_back_rank[8] = {WR, WN, WB, WQ, WK, WB, WN, WR};
        int white_pawns[8] = {WP, WP, WP, WP, WP, WP, WP, WP};

        for (int c = 0; c < 8; c++) {
            board[6][c] = white_pawns[c];
            board[7][c] = white_back_rank[c];
        }
    }

    // 印出棋盤（就是原本的 printBoard）
    void print() const {
        for (int r = 0; r < 8; r++) {
            cout << 8 - r << " ";
            for (int c = 0; c < 8; c++) {
                int p = board[r][c];
                char symbol;

                switch (p) {
                    case WP:
                        symbol = 'P';
                        break;
                    case WN:
                        symbol = 'N';
                        break;
                    case WB:
                        symbol = 'B';
                        break;
                    case WR:
                        symbol = 'R';
                        break;
                    case WQ:
                        symbol = 'Q';
                        break;
                    case WK:
                        symbol = 'K';
                        break;
                    case BP:
                        symbol = 'p';
                        break;
                    case BN:
                        symbol = 'n';
                        break;
                    case BB:
                        symbol = 'b';
                        break;
                    case BR:
                        symbol = 'r';
                        break;
                    case BQ:
                        symbol = 'q';
                        break;
                    case BK:
                        symbol = 'k';
                        break;
                    default:
                        symbol = '.';
                }

                cout << symbol << " ";
            }
            cout << endl;
        }
        cout << "  a b c d e f g h" << endl;
    }
};

int main() {
    Board b;    // 建立一個棋盤物件
    b.print();  // 把棋盤印出來
    return 0;
}
