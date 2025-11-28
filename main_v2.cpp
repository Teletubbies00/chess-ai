#include <iostream>
#include <string>
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

// 把 'a'~'h' 和 '1'~'8' 轉成 row, col
// e.g. file='a', rank='8' -> row=0, col=0  (a8)
//      file='e', rank='4' -> row=4, col=4  (e4)
bool fileRankToRC(char file, char rank, int& row, int& col) {
    // 檢查是否在合法範圍
    if (file < 'a' || file > 'h')
        return false;
    if (rank < '1' || rank > '8')
        return false;

    col = file - 'a';          // 'a' -> 0, 'b' -> 1, ..., 'h' -> 7
    int rankNum = rank - '0';  // '1' -> 1, ..., '8' -> 8

    // 我們的 row=0 是最上面那排，也就是 8 排
    row = 8 - rankNum;  // rank=8 -> row=0, rank=1 -> row=7
    return true;
}

// 把 row, col 轉回像 "e4" 這種字串
string rcToSquare(int row, int col) {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return "??";  // 非法就回傳 "??" 當作 debug 用
    }
    char file = 'a' + col;  // 0 -> 'a', 4 -> 'e'
    char rank = '8' - row;  // row=0 -> '8', row=7 -> '1'
    string s;
    s.push_back(file);
    s.push_back(rank);
    return s;
}

struct Move {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    Piece promotion;  // 升變用，沒有升變時用 EMPTY

    Move() : fromRow(0), fromCol(0), toRow(0), toCol(0), promotion(EMPTY) {}

    Move(int fr, int fc, int tr, int tc, Piece promo = EMPTY)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), promotion(promo) {}
};

// 解析像 "e2e4" 這樣的字串為 Move
// 不檢查棋規，只把座標轉成 row,col
// 解析失敗時回傳 false
bool parseSimpleMove(const string& s, Move& move) {
    if (s.size() != 4) {
        return false;
    }

    char fromFile = s[0];  // 'e'
    char fromRank = s[1];  // '2'
    char toFile = s[2];    // 'e'
    char toRank = s[3];    // '4'

    int fr, fc, tr, tc;
    if (!fileRankToRC(fromFile, fromRank, fr, fc)) {
        return false;
    }
    if (!fileRankToRC(toFile, toRank, tr, tc)) {
        return false;
    }

    move = Move(fr, fc, tr, tc);
    return true;
}

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

    Piece charToPiece(char ch) const {
        switch (ch) {
            case 'P':
                return WP;
            case 'N':
                return WN;
            case 'B':
                return WB;
            case 'R':
                return WR;
            case 'Q':
                return WQ;
            case 'K':
                return WK;
            case 'p':
                return BP;
            case 'n':
                return BN;
            case 'b':
                return BB;
            case 'r':
                return BR;
            case 'q':
                return BQ;
            case 'k':
                return BK;
            default:
                return EMPTY;
        }
    }
    bool loadFEN(const string& fen) {
        // 找到第一個空白，把棋盤那段切出來
        size_t spacePos = fen.find(' ');
        string boardPart;
        if (spacePos == string::npos) {
            boardPart = fen;  // 整串都是棋盤
        } else {
            boardPart = fen.substr(0, spacePos);
        }

        // 先用 temp 暫存
        int temp[8][8];
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                temp[r][c] = EMPTY;
            }
        }

        int row = 0;
        int col = 0;

        for (char ch : boardPart) {
            if (ch == '/') {
                if (col != 8) {
                    cout << "[loadFEN] Invalid FEN: row " << row << " has "
                         << col << " squares (must be 8)." << endl;
                    return false;  // ⭐ 錯 → false
                }
                row++;
                col = 0;
                if (row >= 8) {
                    cout << "[loadFEN] Invalid FEN: too many rows." << endl;
                    return false;  // ⭐ 錯 → false
                }
            } else if (ch >= '1' && ch <= '8') {
                int emptyCount = ch - '0';
                col += emptyCount;
                if (col > 8) {
                    cout << "[loadFEN] Invalid FEN: too many squares in row "
                         << row << " (col=" << col << ")." << endl;
                    return false;  // ⭐ 錯 → false
                }
            } else {  // 棋子字元
                Piece p = charToPiece(ch);
                if (p == EMPTY) {
                    cout << "[loadFEN] Invalid FEN: unknown piece char '" << ch
                         << "'." << endl;
                    return false;  // ⭐ 錯 → false
                }
                if (col >= 8) {
                    cout << "[loadFEN] Invalid FEN: too many squares in row "
                         << row << " when placing piece '" << ch << "'."
                         << endl;
                    return false;  // ⭐ 錯 → false
                }
                temp[row][col] = p;
                col++;
            }
        }

        // 檢查最後一排是不是剛好走完
        if (row != 7 || col != 8) {
            cout << "[loadFEN] Invalid FEN: total rows/cols not 8x8." << endl;
            return false;  // ⭐ 錯 → false
        }

        // 到這裡代表 FEN 合法，才真的覆蓋 board
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                board[r][c] = temp[r][c];
            }
        }
        return true;  // ⭐ 成功 → true
    }
};

int main() {
    Board b;

    // 1. 起始局面
    string fenStart =
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    if (b.loadFEN(fenStart)) {
        cout << "[載入 fenStart 成功]" << endl;
        b.print();
    } else {
        cout << "[載入 fenStart 失敗]" << endl;
    }

    cout << endl;

    // 2. 兩顆兵局面
    string fen2 = "8/8/8/3p4/4P3/8/8/8 w - - 0 1";
    if (b.loadFEN(fen2)) {
        cout << "[載入 fen2 成功]" << endl;
        b.print();
    } else {
        cout << "[載入 fen2 失敗]" << endl;
    }

    cout << endl;
    cout << "=== 測試壞掉的 FEN 防呆 ===" << endl;

    // 3. bad1（記得改成你想要的壞 FEN，例如 "6k/..."）
    string bad1 = "6k/8/8/8/8/8/8/8 w - - 0 1";
    cout << "\n[測試 bad1] " << bad1 << endl;
    if (!b.loadFEN(bad1)) {
        cout << "bad1 載入失敗（預期）" << endl;
    }

    // 4. bad2
    string bad2 = "8/8/8/8/8/8/8/8/8 w - - 0 1";
    cout << "\n[測試 bad2] " << bad2 << endl;
    if (!b.loadFEN(bad2)) {
        cout << "bad2 載入失敗（預期）" << endl;
    }

    // 5. bad3
    string bad3 = "8/8/8/8/8/8/8/7X w - - 0 1";
    cout << "\n[測試 bad3] " << bad3 << endl;
    if (!b.loadFEN(bad3)) {
        cout << "bad3 載入失敗（預期）" << endl;
    }

    return 0;
}
