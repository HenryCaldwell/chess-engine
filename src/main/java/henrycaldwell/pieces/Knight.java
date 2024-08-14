package henrycaldwell.pieces;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import com.google.common.collect.ImmutableList;

import henrycaldwell.Alliance;
import henrycaldwell.board.Board;
import henrycaldwell.board.BoardUtils;
import henrycaldwell.board.Move;
import henrycaldwell.board.Move.AttackMove;
import henrycaldwell.board.Move.MajorMove;
import henrycaldwell.board.Tile;

public class Knight extends Piece {

    private final static int[] CANDIDATE_MOVE_COORDINATES = {-17, -15, -10, -6, 6, 10, 15, 17};

    public Knight(int piecePosition, Alliance pieceAlliance) {
        super(piecePosition, pieceAlliance);
    }

    @Override
    public Collection<Move> calculateLegalMoves(Board board) {
        List<Move> legalMoves = new ArrayList<>();

        for (int currentCandidateOffset : CANDIDATE_MOVE_COORDINATES) {
            int candidateDestinationCoordinate = this.piecePosition + currentCandidateOffset;

            if (BoardUtils.isValidTileCoordinate(candidateDestinationCoordinate)) {
                if (isFirstColumnExclusion(this.piecePosition, currentCandidateOffset) || 
                        isSecondColumnExclusion(this.piecePosition, currentCandidateOffset) || 
                        isSeventhColumnExclusion(this.piecePosition, currentCandidateOffset) || 
                        isEighthColumnExclusion(this.piecePosition, currentCandidateOffset)) {
                    continue;
                }

                Tile candidateDestinationTile = board.getTile(candidateDestinationCoordinate);

                if (!candidateDestinationTile.isTileOccupied()) {
                    legalMoves.add(new MajorMove(board, this, candidateDestinationCoordinate));
                } else {
                    Piece pieceAtDestination = candidateDestinationTile.getPiece();
                    Alliance pieceAlliance = pieceAtDestination.getPieceAlliance();

                    if (this.pieceAlliance != pieceAlliance) {
                        legalMoves.add(new AttackMove(board, this, candidateDestinationCoordinate, pieceAtDestination));
                    }
                }
            }
        }

        return ImmutableList.copyOf(legalMoves);
    }

    private static boolean isFirstColumnExclusion(int currentPosition, int candidateOffset) {
        return BoardUtils.FIRST_COLUMN[currentPosition] && 
                (candidateOffset == -17 || 
                candidateOffset == -10 || 
                candidateOffset == 6 || 
                candidateOffset == 15);
    }

    private static boolean isSecondColumnExclusion(int currentPosition, int candidateOffset) {
        return BoardUtils.SECOND_COLUMN[currentPosition] && 
                (candidateOffset == -10 || 
                candidateOffset == 6);
    }

    private static boolean isSeventhColumnExclusion(int currentPosition, int candidateOffset) {
        return BoardUtils.SEVENTH_COLUMN[currentPosition] && 
                (candidateOffset == -6 ||
                candidateOffset == 10);
    }

    private static boolean isEighthColumnExclusion(int currentPosition, int candidateOffset) {
        return BoardUtils.EIGHTH_COLUMN[currentPosition] &&
                (candidateOffset == -15 || 
                candidateOffset == -6 || 
                candidateOffset == 10 ||
                candidateOffset == 17);
    }
}
