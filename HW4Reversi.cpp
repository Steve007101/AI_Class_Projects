/*
Name: Steven Perry
Class: CS438-001
Assignment: Homework 4 Reversi Alpha-Beta Search w/Heuristic
Date: 3/7/2019

Summary: Uses board.txt to read board and writes to move.txt to determine next move.
	 I have attached a folder with Instrutor provided programs to use this with.
	 7 layers seems to be the best trade-off for speed and success
	 Plays to the end, doesn't make illegal moves.
	 Beats ReversiFL 10/10 times playing as White or Black.
	 Beats greedy 6/10 times playing as Black, 4/10 as White.



*/

#include <iostream>
#include <fstream>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#define ROWS 8
#define COLS 8

using namespace std;

const int maxturn = 1;
const int minturn = -1;
const int blank = 0;
const int VS = -1000000;
const int VL = 1000000;
const int maxDepth = 7;

int nextGlobalDepth = 1;

bool getGameBoard(int iGameBoard[ROWS][COLS])
{
	int iCur;
	char cInput;
	ifstream in("board.txt");

	for (iCur = 0; iCur<ROWS*COLS; iCur++)
	{
		cInput = in.get();
		switch (cInput)
		{
		case '\n':
		case '\t':
			iCur--;
			break;
		case 'X':
		case 'x':
		case '-':
			iGameBoard[iCur / COLS][iCur%COLS] = -1;
			break;
		case 'O':
		case 'o':
		case '0':
		case '+':
			iGameBoard[iCur / COLS][iCur%COLS] = 1;
			break;
		case ' ':
		case '.':
		case '_':
			iGameBoard[iCur / COLS][iCur%COLS] = 0;
			break;
		default:
			in.close();
			return false;
		}
	}
	in.close();
	return true;
}

bool putMove(int iMoveRow, int iMoveCol)
{
	ofstream out("move.txt");

	if (iMoveCol >= COLS || iMoveCol<0)
		return false;
	if (iMoveRow >= ROWS || iMoveRow<0)
		return false;
	out << iMoveRow << " " << iMoveCol << endl;
	out.close();
	return true;
}

struct board
{
	int m[8][8];	// 1, 0, -1
	//int r, c, turn, h;	// the move that gets to this board
	int h; // heuristic value of this board state
	int nextR, nextC; // calculated next move
	board(int n[][8], int he = 0, int nR = -1, int nC = -1)
	{
		for (int k = 0; k<8; k++)
			for (int l = 0; l<8; l++)
				m[k][l] = n[k][l];
		h = he; nextR = nR; nextC = nC;
	}
};

struct movePos
{
public:
	int r, c;
	movePos(int row, int column)
	{
		r = row; c = column;
	}

};

typedef board *state_t;


int max(int a, int b)
{
	return a>b ? a : b;
}

int min(int a, int b)
{
	return a>b ? b : a;
}

void swap(int &a, int &b)
{
	int tmp = a;
	a = b;
	b = tmp;
}

int other(int k)
{
	return k == 1 ? -1 : 1;
}

bool legal(state_t s, int row, int column, int turn)
{

	// check up if it's open
	if (s->m[row][column] != 0)
		return false;


	int r, c;

	// check up the column - is there a capture from above?
	for (r = row - 1; r >= 0 && s->m[r][column] == other(turn); r--);
	if (r<row - 1 && r >= 0 && s->m[r][column] == turn)
	{
		return true;
	}
	// check down the column
	for (r = row + 1; r<8 && s->m[r][column] == other(turn); r++);
	if (r>row + 1 && r<8 && s->m[r][column] == turn)
	{
		return true;
	}
	// check to the left in the row
	for (c = column - 1; c >= 0 && s->m[row][c] == other(turn); c--);
	if (c<column - 1 && c >= 0 && s->m[row][c] == turn)
	{
		return true;
	}
	// check to the right in the row
	for (c = column + 1; c<8 && s->m[row][c] == other(turn); c++);
	if (c>column + 1 && c<8 && s->m[row][c] == turn)
	{
		return true;
	}
	// check NW
	for (c = column - 1, r = row - 1; c >= 0 && r >= 0 && s->m[r][c] == other(turn); c--, r--);
	if (c<column - 1 && c >= 0 && r >= 0 && s->m[r][c] == turn)
	{
		return true;
	}
	// check NE
	for (c = column + 1, r = row - 1; c<8 && r>0 && s->m[r][c] == other(turn); c++, r--);
	if (c>column + 1 && c<8 && r >= 0 && s->m[r][c] == turn)
	{
		return true;
	}
	// check SE
	for (c = column + 1, r = row + 1; c<8 && r<8 && s->m[r][c] == other(turn); c++, r++);
	if (c>column + 1 && c<8 && r<8 && s->m[r][c] == turn)
	{
		return true;
	}

	// check SW
	for (c = column - 1, r = row + 1; c >= 0 && r<8 && s->m[r][c] == other(turn); c--, r++);
	if (c<column - 1 && c >= 0 && r<8 && s->m[r][c] == turn)
	{
		return true;
	}

	return false;
}

void printboard(int m[][8])
{
	for (int r = 0; r < 8; r++)
	{
		for (int c = 0; c < 8; c++)
		{
			if (m[r][c] != -1)
				cout << " ";
			cout << m[r][c] << " ";

		}
		cout << endl;
	}
}

void heuristic(state_t b)
{
	// the general idea of my heuristic is to grade based on weighted values
	// depending on the area the piece is in. Adds the bias/weighted value if
	// the maxturn player's piece and subtracts if it's the minturn player's piece
	int val = 0;
	// best area
	int cornerBias = 10; // 4 corners only
	// better area
	int edgeBias = 6; // all other spots at the edge of the board
	// neutral area
	int middleBias = 3; // the center area/what isn't covered otherwise
	// worst area
	int riskyAreaBias = -10; // all areas next to corners or edge pieces
	//	corner area check

	// top left
	if (b->m[0][0] != blank)
	{
		if (b->m[0][0] == maxturn)
			val += cornerBias;
		else
			val -= cornerBias;
	}
	// top right
	if (b->m[0][7] != blank)
	{
		if (b->m[0][7] == maxturn)
			val += cornerBias;
		else
			val -= cornerBias;
	}
	// bottom left
	if (b->m[7][0] != blank)
	{
		if (b->m[7][0] == maxturn)
			val += cornerBias;
		else
			val -= cornerBias;
	}
	// bottom right
	if (b->m[7][7] != blank)
	{
		if (b->m[7][7] == maxturn)
			val += cornerBias;
		else
			val -= cornerBias;
	}

	//	edge area check

	// left edge
	for (int r = 2; r < 6; r++)
	{
		if (b->m[r][0] != blank)
		{
			if (b->m[r][0] == maxturn)
				val += edgeBias;
			else
				val -= edgeBias;
		}
	}
	// right edge
	for (int r = 2; r < 6; r++)
	{
		if (b->m[r][7] != blank)
		{
			if (b->m[r][7] == maxturn)
				val += edgeBias;
			else
				val -= edgeBias;
		}
	}
	// top edge
	for (int c = 2; c < 6; c++)
	{
		if (b->m[0][c] != blank)
		{
			if (b->m[0][c] == maxturn)
				val += edgeBias;
			else
				val -= edgeBias;
		}
	}
	// bottom edge
	for (int c = 2; c < 6; c++)
	{
		if (b->m[7][c] != blank)
		{
			if (b->m[7][c] == maxturn)
				val += edgeBias;
			else
				val -= edgeBias;
		}
	}

	// middle area check

	for (int r = 2; r < 6; r++)
	{
		for (int c = 2; c < 6; c++)
		{
			if (b->m[r][c] != blank)
			{
				if (b->m[r][c] == maxturn)
					val += middleBias;
				else
					val -= middleBias;
			}
		}
	}

	// risky area check

	// left risky column
	for (int r = 0; r < 8; r++)
	{ 
		if (b->m[r][1] != blank)
		{
			if (b->m[r][1] == maxturn)
				val += riskyAreaBias;
			else
				val -= riskyAreaBias;
		}
	}
	// right risky column
	for (int r = 0; r < 8; r++)
	{
		if (b->m[r][6] != blank)
		{
			if (b->m[r][6] == maxturn)
				val += riskyAreaBias;
			else
				val -= riskyAreaBias;
		}
	}
	// middle of top risky row
	for (int c = 2; c < 6; c++)
	{
		if (b->m[1][c] != blank)
		{
			if (b->m[1][c] == maxturn)
				val += riskyAreaBias;
			else
				val -= riskyAreaBias;
		}
	}
	// middle of bottom risky row
	for (int c = 2; c < 6; c++)
	{
		if (b->m[6][c] != blank)
		{
			if (b->m[6][c] == maxturn)
				val += riskyAreaBias;
			else
				val -= riskyAreaBias;
		}
	}
	// top left lone risky spot
	if (b->m[1][0] != blank)
	{
		if (b->m[1][0] == maxturn)
			val += riskyAreaBias;
		else
			val -= riskyAreaBias;
	}
	// bottom left lone risky spot
	if (b->m[6][0] != blank)
	{
		if (b->m[6][0] == maxturn)
			val += riskyAreaBias;
		else
			val -= riskyAreaBias;
	}

	// top right lone risky spot
	if (b->m[1][7] != blank)
	{
		if (b->m[1][7] == maxturn)
			val += riskyAreaBias;
		else
			val -= riskyAreaBias;
	}
	// bottom right risky spot
	if (b->m[6][7] != blank)
	{
		if (b->m[6][7] == maxturn)
			val += riskyAreaBias;
		else
			val -= riskyAreaBias;
	}

	b->h = val;
}

vector<movePos> generateMoves(state_t board, int turnPlayer)
{
	vector<movePos> moveList;

	for (int row = 0; row < 8; row++)
		for (int column = 0; column < 8; column++)
			if (legal(board, row, column, turnPlayer))
				moveList.push_back(movePos(row, column));
	// console verification of generated moves
	//for (short int i = 0; i < moveList.size(); i++)
	//	cout << "Discovering move at row " << moveList[i].r + 1 << " column " << moveList[i].c + 1 << "\n";
	return moveList;
}

void makeMove(state_t s, int row, int column, int turn)
{
	// similar to legal function but actually executes all captures for move

	int r, c;

	// check up the column - is there a capture from above?
	for (r = row - 1; r >= 0 && s->m[r][column] == other(turn); r--);
	if (r<row - 1 && r >= 0 && s->m[r][column] == turn)
	{
		// if yes, capture pieces between r and row, down from r
		while (r != row - 1)
		{
			r++;
			s->m[r][column] = turn;
		}
		r = -1; // safety value to terminate for loop
	}
	// check down the column
	for (r = row + 1; r<8 && s->m[r][column] == other(turn); r++);
	if (r>row + 1 && r<8 && s->m[r][column] == turn)
	{
		// if yes, capture pieces between r and row, up from r
		while (r != row + 1)
		{
			r--;
			s->m[r][column] = turn;
		}
		r = 8; // safety value to terminate for loop
	}
	// check to the left in the row
	for (c = column - 1; c >= 0 && s->m[row][c] == other(turn); c--);
	if (c<column - 1 && c >= 0 && s->m[row][c] == turn)
	{
		// if yes, capture pieces between c and column, right from c
		while (c != column - 1)
		{
			c++;
			s->m[row][c] = turn;
		}
		c = -1; // safety value to terminate for loop
	}
	// check to the right in the row
	for (c = column + 1; c<8 && s->m[row][c] == other(turn); c++);
	if (c>column + 1 && c<8 && s->m[row][c] == turn)
	{
		// if yes, capture pieces between c and column, left from c
		while (c != column + 1)
		{
			c--;
			s->m[row][c] = turn;
		}
		c = 8; // safety value to terminate for loop
	}
	// check NW
	for (c = column - 1, r = row - 1; c >= 0 && r >= 0 && s->m[r][c] == other(turn); c--, r--);
	if (c<column - 1 && c >= 0 && r >= 0 && s->m[r][c] == turn)
	{
		// if yes, capture pieces between r, c and row, column, moving SE
		while (c != column - 1 && r != row - 1)
		{
			c++; r++;
			s->m[r][c] = turn;
		}
		c = -1; // safety value to terminate for loop
	}
	// check NE
	for (c = column + 1, r = row - 1; c<8 && r>0 && s->m[r][c] == other(turn); c++, r--);
	if (c>column + 1 && c<8 && r >= 0 && s->m[r][c] == turn)
	{
		// if yes, capture pieces between r, c and row, column, moving SW
		while (c != column + 1 && r != row - 1)
		{
			c--; r++;
			s->m[r][c] = turn;
		}
		r = -1; // safety value to terminate for loop
	}
	// check SE
	for (c = column + 1, r = row + 1; c<8 && r<8 && s->m[r][c] == other(turn); c++, r++);
	if (c > column + 1 && c < 8 && r < 8 && s->m[r][c] == turn)
	{
		// if yes, capture pieces between r, c and row, column, moving NW
		while (c != column + 1 && r != row + 1)
		{
			c--; r--;
			s->m[r][c] = turn;
		}
		c = 8; // safety value to terminate for loop
	}


	// check SW
	for (c = column - 1, r = row + 1; c >= 0 && r<8 && s->m[r][c] == other(turn); c--, r++);
	if (c<column - 1 && c >= 0 && r<8 && s->m[r][c] == turn)
	{
		// if yes, capture pieces between r, c and row, column, moving NE
		while (c != column - 1 && r != row + 1)
		{
			c++; r--;
			s->m[r][c] = turn;
		}
		c = -1; // safety value to terminate for loop
	}

}

void minimaxSearch(int depth, state_t boardState, int turnPlayer, int alpha, int beta)
{
	// console output for debugging
	//if (depth == nextGlobalDepth)
	//{
	//	nextGlobalDepth++;
	//	cout << "Depth of " << depth << " reached\n";
	//}
	//cout << "Node at depth " << depth << endl;
	// if reached max depth, calculate value of board state
	if (depth == maxDepth)
		heuristic(boardState);
	else {
		if (turnPlayer == maxturn)
		{
			// alpha search section
			int bestVal = VS; // initial running value is effectively -infinity
			// generate list of possible moves
			vector<movePos> nextMoves = generateMoves(boardState, turnPlayer);
			// iterate through them
			for (int i = 0; i < nextMoves.size(); i++)
			{
				// calculate resulting board for move i out of nextMoves
				state_t newBoard = new board(boardState->m);
				makeMove(newBoard, nextMoves[i].r, nextMoves[i].c, turnPlayer);
				// recursively search its own possible moves
				// cout << "Queuing move at " << nextMoves[i].r << " " << nextMoves[i].c << endl;
				minimaxSearch(depth + 1, newBoard, other(turnPlayer), alpha, beta);
				// if this board is the best board so far
				if (newBoard->h > bestVal)
				{
					bestVal = newBoard->h;
					// save this move as the current best
					// for the current board and its h value
					boardState->h = newBoard->h;
					boardState->nextR = nextMoves[i].r;
					boardState->nextC = nextMoves[i].c;
				}
				// clear newBoard pointer
				delete newBoard;
				newBoard = NULL;
				// update alpha
				if (bestVal > alpha)
				{
					// cout << "New Alpha of " << bestVal << endl;
					alpha = bestVal;
				}
				// pruning
				if (beta <= alpha)
					// cout << "Prune occured\n";
					break;
			}
		}
		else // turnPlayer is minturn
		{
			// beta search section
			int bestVal = VL; // initial running value is effectively +infinity
							  // generate list of possible moves
			vector<movePos> nextMoves = generateMoves(boardState, turnPlayer);
			// iterate through them
			for (int i = 0; i < nextMoves.size(); i++)
			{
				// calculate resulting board for move i out of nextMoves
				state_t newBoard = new board(boardState->m);
				makeMove(newBoard, nextMoves[i].r, nextMoves[i].c, turnPlayer);
				// recursively search its own possible moves
				minimaxSearch(depth + 1, newBoard, other(turnPlayer), alpha, beta);
				// if this board is the best board so far
				if (newBoard->h < bestVal)
				{
					bestVal = newBoard->h;
					// save this move as the current best
					// for the current board and its h value
					boardState->h = newBoard->h;
					boardState->nextR = nextMoves[i].r;
					boardState->nextC = nextMoves[i].c;
				}
				// clear newBoard pointer
				delete newBoard;
				newBoard = NULL;
				// update beta
				if (bestVal < beta)
				{
					// cout << "New Beta of " << bestVal << endl;
					beta = bestVal;
				}
				// pruning
				if (beta <= alpha)
					// cout << "Prune occured\n";
					break;
			}
		}
	}
}

int main()
{
	int n[8][8];

	getGameBoard(n);
	printboard(n);

	state_t s = new board(n);

	minimaxSearch(1, s, maxturn, VS, VL);

	putMove(s->nextR, s->nextC);

	//cout << "best next move is at row " << s->nextR+1;
	//cout << " column " << s->nextC+1 << " (assuming row/column numbers start at 1)";

	delete s;
	s = NULL;

	//for testing
	// getchar();

}
