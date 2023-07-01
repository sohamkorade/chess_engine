import chess
import chess.engine
import chess.pgn
import argparse
import random
import time
import tqdm
import os

parser = argparse.ArgumentParser(description='Chess engine tester')
parser.add_argument('--engine1', type=str, default='stockfish', help='Path to engine')
parser.add_argument('--engine2', type=str, default='stockfish', help='Path to engine')
args = parser.parse_args()

# create dir 'games' if not exists
# (in current dir)
os.chdir(os.path.dirname(os.path.abspath(__file__)))
if not os.path.exists('games'):
	os.makedirs('games')

engine1_name=os.path.basename(args.engine1)
engine2_name=os.path.basename(args.engine2)

pgn=open(f"games/{engine1_name} vs {engine2_name} at {time.strftime('%Y-%m-%d %H:%M:%S')}.pgn", "a+")

def match(round):
	engine1=chess.engine.SimpleEngine.popen_uci(args.engine1)
	engine2=chess.engine.SimpleEngine.popen_uci(args.engine2)

	board = chess.Board()
	wtime=0.01
	btime=0.01
	turn=random.choice([engine1, engine2])

	# print("W: ", args.engine1)
	# print("B: ", args.engine2)

	for i in tqdm.trange(100):
		result = turn.play(board, chess.engine.Limit(time=wtime))
		if result.move == None:
			break
		board.push(result.move)
		if board.is_game_over():
			break
		turn = engine1 if turn == engine2 else engine2
		
	# else:
		# print("Game too long: aborted.")
	
	# print(board.result())
	# print(board)

	engine1.quit()
	engine2.quit()

	# save game
	game=chess.pgn.Game.from_board(board)
	game.headers["Event"]="Engine match"
	game.headers["Site"]="Local"
	game.headers["Round"]=round
	game.headers["Date"]=time.strftime('%Y/%m/%d %H:%M:%S')
	game.headers["White"]=args.engine1
	game.headers["Black"]=args.engine2
	game.headers["Result"]=board.result()

	print(game, file=pgn, end="\n\n")

	return board.result()


def runall():
	white = []
	black = []
	white_sum = 0
	black_sum = 0
	for i in range(10):
		result= match(i)
		if result == "1-0":
			white.append('1')
			black.append('0')
			white_sum += 1
		elif result == "0-1":
			white.append('0')
			black.append('1')
			black_sum += 1
		elif result == "1/2-1/2":
			white.append('½')
			black.append('½')
			white_sum += 0.5
			black_sum += 0.5
		else:
			white.append('*')
			black.append('*')
		
	print("White:"," ".join(white), "=", white_sum)
	print("Black:"," ".join(black), "=", black_sum)

runall()
