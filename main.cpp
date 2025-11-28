#include <iostream>
#include <string>
using namespace std;

// =======================
//  棋子代號（Piece Enum）
// -----------------------
// 用一個 enum 來代表棋子種類：
//   正數 = 白棋、負數 = 黑棋、0 = 空格
// 之後程式裡只要看到 WP / BK 等等，就知道是什麼棋子
// =======================
enum Piece {
    EMPTY = 0,  // 空格（沒有棋子）

    WP = 1,  // White Pawn  白兵
    WN = 2,  // White Knight白馬
    WB = 3,  // White Bishop白象
    WR = 4,  // White Rook  白車
    WQ = 5,  // White Queen 白后
    WK = 6,  // White King  白王

    BP = -1,  // Black Pawn  黑兵
    BN = -2,  // Black Knight黑馬
    BB = -3,  // Black Bishop黑象
    BR = -4,  // Black Rook  黑車
    BQ = -5,  // Black Queen 黑后
    BK = -6   // Black King  黑王
};

// ===================================================
//  座標轉換工具：file / rank <-> row / col
// ---------------------------------------------------
//  - 外部世界（下棋的人、FEN、棋譜）習慣用 "e4" 這種表示法
//    * file：a ~ h   （欄）
//    * rank：1 ~ 8   （行）
//  - 內部程式用陣列 board[8][8]：
//    * row：0 在最上面（8排）到 7（1排）
//    * col：0 在最左邊（a線）到 7（h線）
// ===================================================

// 把 'a'~'h' 和 '1'~'8' 轉成 row, col
// 例：file='a', rank='8' -> row=0, col=0  (a8)
//     file='e', rank='4' -> row=4, col=4  (e4)
bool fileRankToRC(char file, char rank, int& row, int& col) {
    // 1) 基本範圍檢查：不是 a~h / 1~8 就直接失敗
    if (file < 'a' || file > 'h')
        return false;
    if (rank < '1' || rank > '8')
        return false;

    // 2) 檔（file）轉成欄（col）
    col = file - 'a';  // 'a' -> 0, 'b' -> 1, ..., 'h' -> 7

    // 3) 橫列（rank）先變成數字 1~8
    int rankNum = rank - '0';  // '1' -> 1, ..., '8' -> 8

    // 4) 我們的 row=0 代表棋盤最上面那排，也就是 rank=8
    row = 8 - rankNum;  // rank=8 -> row=0, rank=1 -> row=7
    return true;
}

// 把 row, col 轉回像 "e4" 這種字串
//   row=4, col=4 -> "e4"
string rcToSquare(int row, int col) {
    // 如果 row / col 不在 0~7 範圍，代表不合理，回傳 "??" 當 debug 用
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return "??";
    }

    // col 反推回 file（欄）字母：0 -> 'a', 4 -> 'e'
    char file = 'a' + col;

    // row 反推回 rank（行）數字字元：row=0 -> '8', row=7 -> '1'
    char rank = '8' - row;

    string s;
    s.push_back(file);
    s.push_back(rank);
    return s;
}

// =======================
//  Move 結構：一手棋
// -----------------------
// 用來描述「從哪裡走到哪裡」：
//   fromRow, fromCol : 起點
//   toRow,   toCol   : 終點
//   promotion        : 升變成什麼子（沒升變就是 EMPTY）
// =======================
struct Move {
    int fromRow;
    int fromCol;
    int toRow;
    int toCol;
    Piece promotion;  // 升變用，沒有升變時用 EMPTY

    // 預設建構子：全部給一個安全的初始值
    Move() : fromRow(0), fromCol(0), toRow(0), toCol(0), promotion(EMPTY) {}

    // 一般建構子：直接指定起終點與升變
    Move(int fr, int fc, int tr, int tc, Piece promo = EMPTY)
        : fromRow(fr), fromCol(fc), toRow(tr), toCol(tc), promotion(promo) {}
};

// ===================================================
//  解析簡單招法字串：例如 "e2e4"
// ---------------------------------------------------
//  這裡先不管「合不合法」，純粹做座標轉換：
//    "e2e4" -> from: (row,col) for e2, to: (row,col) for e4
//  失敗（字串長度不對或座標不合法）就回傳 false
// ===================================================
bool parseSimpleMove(const string& s, Move& move) {
    // 必須剛好 4 個字元：file rank file rank
    if (s.size() != 4) {
        return false;
    }

    char fromFile = s[0];  // 'e'
    char fromRank = s[1];  // '2'
    char toFile = s[2];    // 'e'
    char toRank = s[3];    // '4'

    int fr, fc, tr, tc;

    // 把起點的 file/rank 轉成 row/col，失敗就直接 false
    if (!fileRankToRC(fromFile, fromRank, fr, fc)) {
        return false;
    }

    // 把終點的 file/rank 轉成 row/col，失敗就直接 false
    if (!fileRankToRC(toFile, toRank, tr, tc)) {
        return false;
    }

    // 都成功就組成一個 Move
    move = Move(fr, fc, tr, tc);
    return true;
}

// ===================================================
//  Board 類別：負責「棋盤狀態」與相關操作
// ---------------------------------------------------
//  成員：
//    - board[8][8] : 目前棋盤上每一格的棋子（用 Piece enum）
//
//  方法（目前版本）：
//    - clear()            : 把整個棋盤清成 EMPTY
//    - initStartPosition(): 設定標準起始局面
//    - print()            : 把棋盤印到 console
//    - charToPiece()      : 單一字元轉成 Piece
//    - loadFEN()          : 從 FEN 字串載入局面（含防呆檢查）
// ===================================================
class Board {
   public:
    int board[8][8];  // 8x8 棋盤（row 0 在最上面，col 0 在最左）

    // ----------------------
    // 建構子：建立物件時自動呼叫
    // ----------------------
    Board() {
        clear();              // 先確保棋盤全是 EMPTY
        initStartPosition();  // 再設定成西洋棋標準起始局面
    }

    // ----------------------
    // clear()
    // 把整個棋盤清成 EMPTY
    // ----------------------
    void clear() {
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                board[r][c] = EMPTY;
            }
        }
    }

    // ----------------------
    // initStartPosition()
    // 設定成標準起始局面
    // ----------------------
    void initStartPosition() {
        clear();  // 先清空，避免髒資料

        // 黑棋後排（第 0 列）
        int black_back_rank[8] = {BR, BN, BB, BQ, BK, BB, BN, BR};
        // 黑兵（第 1 列）
        int black_pawns[8] = {BP, BP, BP, BP, BP, BP, BP, BP};

        for (int c = 0; c < 8; c++) {
            board[0][c] = black_back_rank[c];
            board[1][c] = black_pawns[c];
        }

        // 白棋後排（第 7 列）
        int white_back_rank[8] = {WR, WN, WB, WQ, WK, WB, WN, WR};
        // 白兵（第 6 列）
        int white_pawns[8] = {WP, WP, WP, WP, WP, WP, WP, WP};

        for (int c = 0; c < 8; c++) {
            board[6][c] = white_pawns[c];
            board[7][c] = white_back_rank[c];
        }
    }

    // ----------------------
    // print()
    // 把棋盤印出來（純文字版）
    // ----------------------
    void print() const {
        for (int r = 0; r < 8; r++) {
            cout << 8 - r << " ";  // 左邊印 rank（8 ~ 1）
            for (int c = 0; c < 8; c++) {
                int p = board[r][c];
                char symbol;

                // 根據 Piece 代號選擇要印的字元
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
                        symbol = '.';  // EMPTY -> 用 '.' 表示空格
                }

                cout << symbol << " ";
            }
            cout << endl;
        }
        cout << "  a b c d e f g h" << endl;  // 底下印 file 標記
    }

    // ----------------------
    // charToPiece()
    // 單一字元 -> Piece enum
    //   大寫 = 白棋、小寫 = 黑棋
    // ----------------------
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
                return EMPTY;  // 不認得的字元就先當作 EMPTY（上層會再檢查）
        }
    }

    // ----------------------
    // loadFEN()
    // 從 FEN 字串載入棋盤（只處理第一段「棋盤」部分）
    // 並包含基本防呆檢查：
    //   - 每列必須剛好 8 格
    //   - 必須剛好 8 列
    //   - 棋子字元必須合法
    //   - 不會直接破壞原本棋盤，只有在 FEN 合法時才覆蓋
    // 回傳：
    //   true  = 載入成功
    //   false = FEN 有問題，棋盤維持舊狀態
    // ----------------------
    bool loadFEN(const string& fen) {
        // 1) 先把第一段「棋盤描述」切出來（空白前面那一段）
        size_t spacePos = fen.find(' ');
        string boardPart;
        if (spacePos == string::npos) {
            boardPart = fen;  // 沒有空白 → 整串都當作棋盤
        } else {
            boardPart = fen.substr(0, spacePos);
        }

        // 2) 先用 temp 暫存，不直接寫進 board
        int temp[8][8];
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                temp[r][c] = EMPTY;
            }
        }

        int row = 0;
        int col = 0;

        // 3) 一個字元一個字元掃描 boardPart
        for (char ch : boardPart) {
            if (ch == '/') {
                // 換新的一排前，先檢查這排是不是剛好 8 格
                if (col != 8) {
                    cout << "[loadFEN] Invalid FEN: row " << row << " has "
                         << col << " squares (must be 8)." << endl;
                    return false;
                }
                row++;
                col = 0;

                // row 不能超過 7（總共只有 0~7 八列）
                if (row >= 8) {
                    cout << "[loadFEN] Invalid FEN: too many rows." << endl;
                    return false;
                }

            } else if (ch >= '1' && ch <= '8') {
                // 數字 1~8 代表連續的空格數
                int emptyCount = ch - '0';
                col += emptyCount;

                // 如果超過 8 格就錯
                if (col > 8) {
                    cout << "[loadFEN] Invalid FEN: too many squares in row "
                         << row << " (col=" << col << ")." << endl;
                    return false;
                }

            } else {  // 棋子字元
                Piece p = charToPiece(ch);

                // 如果 charToPiece 回傳 EMPTY，代表字元不認得
                if (p == EMPTY) {
                    cout << "[loadFEN] Invalid FEN: unknown piece char '" << ch
                         << "'." << endl;
                    return false;
                }

                // 試圖放棋子的時候 col 已經 >= 8 也不合法
                if (col >= 8) {
                    cout << "[loadFEN] Invalid FEN: too many squares in row "
                         << row << " when placing piece '" << ch << "'."
                         << endl;
                    return false;
                }

                temp[row][col] = p;
                col++;
            }
        }

        // 4) 結束時，必須剛好停在 row=7, col=8 才是完整 8x8
        if (row != 7 || col != 8) {
            cout << "[loadFEN] Invalid FEN: total rows/cols not 8x8." << endl;
            return false;
        }

        // 5) 到這裡代表 FEN 合法，才真的覆蓋 board
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                board[r][c] = temp[r][c];
            }
        }
        return true;
    }
};

// =======================
//  main(): 簡單測試區
// -----------------------
// 目前做的事：
//   1. 測試載入起始局面 FEN
//   2. 測試載入一個只有兩顆兵的局面
//   3. 測試幾個壞掉的 FEN，看防呆有沒有抓到
// =======================
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

    // 3. bad1：某一列少一格
    string bad1 = "6k/8/8/8/8/8/8/8 w - - 0 1";
    cout << "\n[測試 bad1] " << bad1 << endl;
    if (!b.loadFEN(bad1)) {
        cout << "bad1 載入失敗（預期）" << endl;
    }

    // 4. bad2：多一整列
    string bad2 = "8/8/8/8/8/8/8/8/8 w - - 0 1";
    cout << "\n[測試 bad2] " << bad2 << endl;
    if (!b.loadFEN(bad2)) {
        cout << "bad2 載入失敗（預期）" << endl;
    }

    // 5. bad3：有一個不認得的棋子字元 'X'
    string bad3 = "8/8/8/8/8/8/8/7X w - - 0 1";
    cout << "\n[測試 bad3] " << bad3 << endl;
    if (!b.loadFEN(bad3)) {
        cout << "bad3 載入失敗（預期）" << endl;
    }

    return 0;
}
