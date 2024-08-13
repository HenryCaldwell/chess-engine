package henrycaldwell.board;

import java.util.HashMap;
import java.util.Map;

import com.google.common.collect.ImmutableMap;

import henrycaldwell.pieces.Piece;

public abstract class Tile {

    protected final int tileCoordinate;

    private static final Map<Integer, EmptyTile> EMPTY_TILES_CACHE = createAllPossibleEmptyTiles();

    public static Tile createTile(int tileCoordinate, Piece piece) {
        return piece != null ? new OccupiedTile(tileCoordinate, piece) : EMPTY_TILES_CACHE.get(tileCoordinate);
    }

    private Tile(int tileCoordinate) {
        this.tileCoordinate = tileCoordinate;
    }

    private static Map<Integer, EmptyTile> createAllPossibleEmptyTiles() {
        Map<Integer, EmptyTile> emptyTileMap = new HashMap<>();

        for (int i = 0; i < 64; i++) {
            emptyTileMap.put(i, new EmptyTile(i));
        }

        return ImmutableMap.copyOf(emptyTileMap);
    }

    public abstract boolean isTileOccupied();

    public abstract Piece getPiece();

    public static final class EmptyTile extends Tile {

        private EmptyTile(int tileCoordinate) {
            super(tileCoordinate);
        }

        @Override
        public boolean isTileOccupied() {
            return false;
        }

        @Override
        public Piece getPiece() {
            return null;
        }
    }

    public static final class OccupiedTile extends Tile {
        
        private final Piece pieceOnTile;

        private OccupiedTile(int tileCoordinate, Piece pieceOnTile) {
            super(tileCoordinate);
            this.pieceOnTile = pieceOnTile;
        }

        @Override
        public boolean isTileOccupied() {
            return true;
        }

        @Override
        public Piece getPiece() {
            return pieceOnTile;
        }
    }
}
