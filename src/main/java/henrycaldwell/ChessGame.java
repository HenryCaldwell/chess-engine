package henrycaldwell;

import henrycaldwell.board.Board;

public class ChessGame {

    public static void main(String[] args) {
        Board board = Board.createStandardBoard();

        System.out.println(board);
    }
}
