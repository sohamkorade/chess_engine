import chess
import chess.pgn

board=chess.Board()

with open("shubham","r") as f:
    moves=f.read().split()
    for i in moves:
        board.push_uci(i)
    
print(board)

pgn=open("shubham.pgn","w")
game=chess.pgn.Game.from_board(board)
print(game,file=pgn,end="\n\n")
