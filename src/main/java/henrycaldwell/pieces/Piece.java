package henrycaldwell.pieces;

import java.util.Collection;

import henrycaldwell.Alliance;
import henrycaldwell.board.Board;
import henrycaldwell.board.Move;

public abstract class Piece {

    protected final int piecePosition;
    protected final Alliance pieceAlliance;
    protected final boolean isFirstMove;

    public Piece (int piecePosition, Alliance pieceAlliance) {
        this.piecePosition = piecePosition;
        this.pieceAlliance = pieceAlliance;
        this.isFirstMove = false;
    }

    public abstract Collection<Move> calculateLegalMoves(Board board);

    public Alliance getPieceAlliance() {
        return this.pieceAlliance;
    }

    public boolean isFirstMove() {
        return this.isFirstMove;
    }
}
