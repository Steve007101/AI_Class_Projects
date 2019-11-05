/*
Steven Perry
CS 438-001 Intro to AI
2/19/2019
HW3: Pegboard Solver 2

Summary: reads in pegboard files (names below as .in files) and outputs solutions

Release Build Heuristic Search Nodes and Times (C=1)

uparrow: 2021 nodes, 254 milliseconds
diamond: 101595 nodes, 546 milliseconds
solitaire: 464 nodes, 304 milliseconds
bigPlus: 46 nodes, 105 milliseconds
bigFireplace: 42753 nodes, 237 milliseconds
bigUpArrow: 406046 nodes, 1060 milliseconds
bigDiamond: 101595 nodes, 553 milliseconds
bigSolitaire: 19067 nodes, 369 milliseconds

*/


#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <math.h>
#include <stack>
#include <queue>
#include <set>
#include <chrono>
#include <bitset>

using namespace std;

// Peg board size 
const int SIZE = 9; // valid values: 5, 7, 9, 11
// Note: the max size is 11 due to my implementaion of using a long long int
// converted from a bitset to store the board states. This limits the size
// of the boards vs using a string to store them but in return boards take up 
// a lot less memory (8 bytes total vs 1 per position or 45 total on a 9x9)
// allowing for a smaller memory size need.
// We only had to account for a board of size 9 for the assignment anyway.
// I wasn't able to figure out how to make arbitrary bitset sizes work 
// with the standard library commands or that would have been a better solution.
const int lowCut = (SIZE - 3) / 2;
const int highCut = SIZE - lowCut;
const int bitSetSize = (SIZE*SIZE) - (4 * (lowCut*lowCut));
float C = 10.0;

struct node
{
	int m[SIZE][SIZE];
	float hv, gv, fv; // values of h, g, f functions
	node *parent;
	node *next; // the next node in the solution path
	node(int sm[][SIZE], node* p = NULL, node* n = NULL)
	{
		for (int r = 0; r < SIZE; r++)
			for (int c = 0; c < SIZE; c++)
				m[r][c] = sm[r][c];
		parent = p;
		next = n;
	}
};

typedef node* nodeP;

void setSentinelValues(int m[][SIZE])
{
	for (int r = 0; r < lowCut; r++)
		for (int c = 0; c < lowCut; c++)
		{
			m[r][c] = -1;
		}
	for (int r = 0; r < lowCut; r++)
		for (int c = highCut; c < SIZE; c++)
		{
			m[r][c] = -1;
		}
	for (int r = highCut; r < SIZE; r++)
		for (int c = 0; c < lowCut; c++)
		{
			m[r][c] = -1;
		}
	for (int r = highCut; r < SIZE; r++)
		for (int c = highCut; c < SIZE; c++)
		{
			m[r][c] = -1;
		}
}

//int le(node *n1, node *n2)
//{
//	return ((n1->fv) >(n2->fv));
//}

void copy(int m[][SIZE], int n[][SIZE])
{
	for (int ro = 0; ro < lowCut; ro++)
		for (int co = lowCut; co < highCut; co++)
			n[ro][co] = m[ro][co];
	for (int ro = lowCut; ro < highCut; ro++)
		for (int co = 0; co < SIZE; co++)
			n[ro][co] = m[ro][co];
	for (int ro = highCut; ro < SIZE; ro++)
		for (int co = lowCut; co < highCut; co++)
			n[ro][co] = m[ro][co];
}

void getnumber(int m[][SIZE], unsigned long long &b)
{
	bitset<bitSetSize> board;
	int count = 0;
	for (int ro = 0; ro < lowCut; ro++)
		for (int co = lowCut; co < highCut; co++)
		{
			if (m[ro][co] == 1)
				board[count] = 1;
			else
				board[count] = 0;
			count++;
		}
	for (int ro = lowCut; ro < highCut; ro++)
		for (int co = 0; co < SIZE; co++)
		{
			if (m[ro][co] == 1)
				board[count] = 1;
			else
				board[count] = 0;
			count++;
		}
	for (int ro = highCut; ro < SIZE; ro++)
		for (int co = lowCut; co < highCut; co++)
		{
			if (m[ro][co] == 1)
				board[count] = 1;
			else
				board[count] = 0;
			count++;
		}
	b = board.to_ullong();
	// Note: as explained at the top this limits
	// the total board positions that can be recorded
	// to 64 or a board size of 11
}

// this implementation only cares if there's a single 1 remaining
// for deciding if the goal is reached as opposed to it having
// to be in a specific place
bool isGoal(int m[][SIZE])
{
	int numOfOnes = 0;
	for (int ro = 0; ro < lowCut; ro++)
		for (int co = lowCut; co < highCut; co++)
			if (m[ro][co] == 1)
				numOfOnes++;
	for (int ro = lowCut; ro < highCut; ro++)
		for (int co = 0; co < SIZE; co++)
			if (m[ro][co] == 1)
				numOfOnes++;
	for (int ro = highCut; ro < SIZE; ro++)
		for (int co = lowCut; co < highCut; co++)
			if (m[ro][co] == 1)
				numOfOnes++;
	if (numOfOnes == 1)
		return true;
	else return false;
}

// unlike the 15puzzle.cpp example, the board values
// are either 0 or 1 so no need to have fixed precision
// for outputting positions
void printsolution(node* n)
{
	// set up next for printing
	while (n->parent)
	{
		n->parent->next = n;
		n = n->parent;
	}

	int count = -1;
	while (n)
	{
		count++;
		cout << endl;
		for (int r = 0; r < SIZE; r++)
		{
			for (int c = 0; c < SIZE; c++)
				// check if valid position
				if (n->m[r][c] != -1)
					cout << n->m[r][c] << " ";
				else
					cout << "  ";
			cout << endl;
		}
		n = n->next;
	}
	cout << "Solution found has " << count << " steps.\n";
}


// Movement operations that takes an input of a string for the movement operation
// and the row and column of the moving peg
bool makeMove(int m[][SIZE], int n[][SIZE], string position, int r, int c) {
	if (position == "up") {
		// first we check if move would be off board
		if (r < 2 || (r < (lowCut + 2) && (c < lowCut || c >= highCut)))
			return false;
		// next we check if the pegs above the designated peg
		// are the correct ones for a jump (1 then 0)
		if (m[r - 1][c] == 1 && m[r - 2][c] == 0)
		{
			// now we set the second/temp matrix to the first
			copy(m, n);
			// finally we actually do the jump for second/temp matrix
			n[r][c] = 0;
			n[r - 1][c] = 0;
			n[r - 2][c] = 1;
			return true;
		}
		// otherwise not a correct move
		return false;
	}
	else if (position == "down") {
		// first we check if move would be off board
		if (r > (SIZE-3) || (r > (highCut-3) && (c < lowCut || c >= highCut)))
			return false;
		// next we check if the pegs below the designated peg
		// are the correct ones for a jump (1 then 0)
		if (m[r + 1][c] == 1 && m[r + 2][c] == 0)
		{
			// now we set the second/temp matrix to the first
			copy(m, n);
			// finally we actually do the jump
			n[r][c] = 0;
			n[r + 1][c] = 0;
			n[r + 2][c] = 1;
			return true;
		}
		// otherwise not a correct move
		return false;
	}
	else if (position == "left") {
		// first we check if move would be off board
		if (c < 2 || (c < (lowCut+2) && (r < lowCut || r >= highCut)))
			return false;
		// next we check if the pegs left of the designated peg
		// are the correct ones for a jump (1 then 0)
		if (m[r][c - 1] == 1 && m[r][c - 2] == 0)
		{
			// now we set the second/temp matrix to the first
			copy(m, n);
			// finally we actually do the jump
			n[r][c] = 0;
			n[r][c - 1] = 0;
			n[r][c - 2] = 1;
			return true;
		}
		// otherwise not a correct move
		return false;
	}
	else if (position == "right") {
		// first we check if move would be off board
		if (c > (SIZE-3) || (c > (highCut-3) && (r < lowCut || r >= highCut)))
			return false;
		// next we check if the pegs right of the designated peg
		// are the correct ones for a jump (1 then 0)
		if (m[r][c + 1] == 1 && m[r][c + 2] == 0)
		{
			// now we set the second/temp matrix to the first
			copy(m, n);
			// finally we actually do the jump
			n[r][c] = 0;
			n[r][c + 1] = 0;
			n[r][c + 2] = 1;
			return true;
		}
		// otherwise not a correct move
		return false;
	}
}


//dfs 
void dfs(int sm[][SIZE])
{
	string move[4] = { "up", "down", "left", "right" };

	stack<node*> open;
	nodeP *np;
	np = new nodeP[20000000];
	int npCount = 0;
	set<unsigned long long> close;
	node *start, *current, *succ;
	unsigned long long sucnum;
	start = new node(sm); // 
	int temp[SIZE][SIZE], success = 0;
	setSentinelValues(temp);
	open.push(start);
	np[npCount++] = start;
	getnumber(start->m, sucnum);
	close.insert(sucnum);
	long gencount = 1;
	cout << "Starting dfs\n";
	while (!open.empty() && !success)
	{
		current = open.top();
		open.pop();
		if (isGoal(current->m))
		{
			cout << "Goal found!\n";
			printsolution(current);
			cout << "Total of " << gencount
				<< " nodes examined.\n\n";
			success = 1;
		}
		else
		{
			// need to find all 1s in current node's board
			// top block of positions
			for (int ro = 0; ro < lowCut; ro++)
				for (int co = lowCut; co < highCut; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								getnumber(temp, sucnum);
								if (close.find(sucnum) == close.end()) // not already in CLOSE
								{
									succ = new node(temp, current);
									close.insert(sucnum);	// add the newly generated successor to CLOSE
									open.push(succ);
									np[npCount++] = succ;
									gencount++;
								}
							}
			// middle block of positions
			for (int ro = lowCut; ro < highCut; ro++)
				for (int co = 0; co < SIZE; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								getnumber(temp, sucnum);
								if (close.find(sucnum) == close.end()) // not already in CLOSE
								{
									succ = new node(temp, current);
									close.insert(sucnum);	// add the newly generated successor to CLOSE
									open.push(succ);
									np[npCount++] = succ;
									gencount++;
								}
							}
			// bottom block of positions
			for (int ro = highCut; ro < SIZE; ro++)
				for (int co = lowCut; co < highCut; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								getnumber(temp, sucnum);
								if (close.find(sucnum) == close.end()) // not already in CLOSE
								{
									succ = new node(temp, current);
									close.insert(sucnum);	// add the newly generated successor to CLOSE
									open.push(succ);
									np[npCount++] = succ;
									gencount++;
								}
							}
		} // end of else
	} // end of while

	if (!success)
	{
		cout << "No solution.\n";
		cout << "Total of " << gencount
			<< " nodes examined.\n\n";
	}

	for (int j = 0; j < npCount; j++)
		delete np[j];

	delete[] np;
}
//bfs = Breadth First Search
void bfs(int sm[][SIZE])
{
	string move[4] = { "up", "down", "left", "right" };

	queue<node*> open;
	nodeP *np;
	np = new nodeP[20000000];
	int npCount = 0;
	set<unsigned long long> close;
	node *start, *current, *succ;
	unsigned long long sucnum;
	start = new node(sm); // 
	int temp[SIZE][SIZE], success = 0;
	setSentinelValues(temp);
	open.push(start);
	np[npCount++] = start;
	getnumber(start->m, sucnum);
	close.insert(sucnum);
	long gencount = 1;
	cout << "Starting bfs\n";
	while (!open.empty() && !success)
	{
		current = open.front();
		open.pop();
		if (isGoal(current->m))
		{
			cout << "Goal found!\n";
			printsolution(current);
			cout << "Total of " << gencount
				<< " nodes examined.\n\n";
			success = 1;
		}
		else
		{
			// need to find all 1s in current node's board
			// top block of positions
			for (int ro = 0; ro < lowCut; ro++)
				for (int co = lowCut; co < highCut; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								getnumber(temp, sucnum);
								if (close.find(sucnum) == close.end()) // not already in CLOSE
								{
									succ = new node(temp, current);
									close.insert(sucnum);	// add the newly generated successor to CLOSE
									open.push(succ);
									np[npCount++] = succ;
									gencount++;
								}
							}
			// middle block of positions
			for (int ro = lowCut; ro < highCut; ro++)
				for (int co = 0; co < SIZE; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								getnumber(temp, sucnum);
								if (close.find(sucnum) == close.end()) // not already in CLOSE
								{
									succ = new node(temp, current);
									close.insert(sucnum);	// add the newly generated successor to CLOSE
									open.push(succ);
									np[npCount++] = succ;
									gencount++;
								}
							}
			// bottom block of positions
			for (int ro = highCut; ro < SIZE; ro++)
				for (int co = lowCut; co < highCut; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								getnumber(temp, sucnum);
								if (close.find(sucnum) == close.end()) // not already in CLOSE
								{
									succ = new node(temp, current);
									close.insert(sucnum);	// add the newly generated successor to CLOSE
									open.push(succ);
									np[npCount++] = succ;
									gencount++;
								}
							}
		} // end of else
	} // end of while

	if (!success)
	{
		cout << "No solution.\n";
		cout << "Total of " << gencount
			<< " nodes examined.\n\n";
	}

	for (int j = 0; j < npCount; j++)
		delete np[j];

	delete[] np;
}

int td(int r1, int c1, int r2, int c2)
{
	return (abs(r1 - r2) + abs(c1 - c2));
}


bool isolatedPeg(const int m[][SIZE], const int &r, const int &c)
{
	bool isIsolated = true;
	// check if the space above is valid
	if (r != 0 && !(r == lowCut && (c < lowCut || c >= highCut)))
		// if peg is in that valid space above, peg isn't isolated
		if (m[r - 1][c] == 1)
			isIsolated = false;
	// check if space below is valid
	if (r != SIZE-1 && !(r == highCut-1 && (c < lowCut || c >= highCut)))
		// if peg is in that valid space above, peg isn't isolated
		if (m[r + 1][c] == 1)
			isIsolated = false;
	// check if space left is valid
	if (c != 0 && !(c == lowCut && (r < lowCut|| r >= highCut)))
		if (m[r][c-1] == 1)
			isIsolated = false;
	// check if space right is valid
	if (c != SIZE-1 && !(c == highCut-1 && (r < lowCut || r >= highCut)))
		if (m[r][c + 1] == 1)
			isIsolated = false;
	return isIsolated;

}

float h(int m[][SIZE])
{
	// After trying several options, using the total city block distance
	// between each consequtively found peg on the board  
	// yielded a heuristic that works for all boards within a couple seconds.

	//int totDisToCenter = 0;
	int totDisToNext = 0;
	//int numPegsRemaining = 0; 
	//int numPegsIsolated = 0;
	int center = SIZE / 2;
	int lastR = center;
	int lastC = center;

	for (int ro = 0; ro < lowCut; ro++)
		for (int co = lowCut; co < highCut; co++)
			if (m[ro][co] == 1)
			{
				//totDisToCenter += td(ro, co, center, center);
				totDisToNext += td(lastR, lastC, ro, co);
				lastR = ro;
				lastC = co;
				//numPegsRemaining++;
				//if (isolatedPeg(m, ro, co))
				//	numPegsIsolated++;
			}
	for (int ro = lowCut; ro < highCut; ro++)
		for (int co = 0; co < SIZE; co++)
			if (m[ro][co] == 1)
			{
				//totDisToCenter += td(ro, co, center, center);
				totDisToNext += td(lastR, lastC, ro, co);
				lastR = ro;
				lastC = co;
				//numPegsRemaining++;
				//if (isolatedPeg(m, ro, co))
				//	numPegsIsolated++;
			}
	for (int ro = highCut; ro < SIZE; ro++)
		for (int co = lowCut; co < highCut; co++)
			if (m[ro][co] == 1)
			{
				//totDisToCenter += td(ro, co, center, center);
				totDisToNext += td(lastR, lastC, ro, co);
				lastR = ro;
				lastC = co;
				//numPegsRemaining++;
				//if (isolatedPeg(m, ro, co))
				//	numPegsIsolated++;
			}
	return C * (totDisToNext);

}

struct LE
{
	bool operator()(node*  lhs, node* rhs) const
	{
		return lhs->fv > rhs->fv; // use > for less than since priority queue is a max heap
	}
};

// A* search
void aStar(int sm[][SIZE])
{
	string move[4] = { "up", "down", "left", "right" };
	node *start, *current, *succ;

	set<unsigned long long> close;
	unsigned long long s;

	start = new node(sm); // 
	start->gv = 0; // cost so far is 0
	priority_queue <node*, vector<node*>, LE> open;
	// priority is a MAX heap so LE is defined as > since we want lowest fv

	open.push(start);

	getnumber(sm, s);
	close.insert(s);

	int temp[SIZE][SIZE], success = 0;
	setSentinelValues(temp);
	long gencount = 1;

	cout << "Starting heuristic search\n";
	while (!open.empty() && !success)
	{
		current = open.top();
		open.pop();
		if (isGoal(current->m))
		{
			cout << "Goal found!\n";
			printsolution(current);
			cout << "Total of " << gencount
				<< " nodes examined.\n\n";
			success = 1;
		}
		else
		{
			// need to find all 1s in current node's board
			// top block of positions
			for (int ro = 0; ro < lowCut; ro++)
				for (int co = lowCut; co < highCut; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								unsigned long long s1;
								getnumber(temp, s1);
								if (close.find(s1) == close.end()) // not already in CLOSE
								{
									close.insert(s1); // add the newly generated successor to CLOSE
									succ = new node(temp, current);
									succ->hv = h(temp);
									succ->gv = (current->gv) + 1;
									succ->fv = succ->hv + succ->gv;
									open.push(succ);
									gencount++;
								}
							}
			// middle block of positions
			for (int ro = lowCut; ro < highCut; ro++)
				for (int co = 0; co < SIZE; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								unsigned long long s1;
								getnumber(temp, s1);
								if (close.find(s1) == close.end()) // not already in CLOSE
								{
									close.insert(s1); // add the newly generated successor to CLOSE
									succ = new node(temp, current);
									succ->hv = h(temp);
									succ->gv = (current->gv) + 1;
									succ->fv = succ->hv + succ->gv;
									open.push(succ);
									gencount++;
								}
							}
			// bottom block of positions
			for (int ro = highCut; ro < SIZE; ro++)
				for (int co = lowCut; co < highCut; co++)
					if (current->m[ro][co] == 1)
						// now we check if there's a valid move to be made at ro,co
						for (int m = 0; m<4; m++)
							if (makeMove(current->m, temp, move[m], ro, co)) // move m is ok
							{
								unsigned long long s1;
								getnumber(temp, s1);
								if (close.find(s1) == close.end()) // not already in CLOSE
								{
									close.insert(s1); // add the newly generated successor to CLOSE
									succ = new node(temp, current);
									succ->hv = h(temp);
									succ->gv = (current->gv) + 1;
									succ->fv = succ->hv + succ->gv;
									open.push(succ);
									gencount++;
								}
							}
		} // end of else
	} // end of while

	if (!success)
	{
		cout << "No solution.\n";
		cout << "Total of " << gencount
			<< " nodes examined.\n\n";
	}

}


void getInput(string file, int m[][SIZE])
{
	string filename = file + ".in";
	ifstream inFile;

	inFile.open(filename);

	if (inFile.fail())
	{
		cout << "Opening of input file " + filename + " failed\n";
		system("pause");
		exit(1);
	}
	cout << "Opening of input file " + filename + " succeeded\n";

	// only set non-sentinel entries
	for (int ro = 0; ro < lowCut; ro++)
		for (int co = lowCut; co < highCut; co++)
			inFile >> m[ro][co];
	for (int ro = lowCut; ro < highCut; ro++)
		for (int co = 0; co < SIZE; co++)
			inFile >> m[ro][co];
	for (int ro = highCut; ro < SIZE; ro++)
		for (int co = lowCut; co < highCut; co++)
			inFile >> m[ro][co];

	cout << "The following pegboard was read from file:\n";
	for (int r = 0; r < SIZE; r++)
	{
		for (int c = 0; c < SIZE; c++)
			// check if valid position
			if (m[r][c] != -1)
				cout << m[r][c] << " ";
			else
				cout << "  ";
		cout << endl;
	}
}

int main()
{
	int matrix[SIZE][SIZE];
	setSentinelValues(matrix);
	string file;

	cout << "Please enter the name of the shape to be solved.\nEnter Q or Quit to terminate the program.\n";
	cin >> file;
	while (file.substr(0, 1) != "Q" && file.substr(0, 4) != "Quit")
	{
		getInput(file, matrix);
		cout << "\tSelect a search method:\n"
			<< "\t1 Depth-first Search\n"
			<< "\t2 Breath-first Search\n"
			<< "\t3 Heuristic Search\n";
		int choice;
		cin >> choice;
		if (choice == 3)
		{
			cout << "Please enter a C value from 1 to 10, 1 being optimal, 10 being fastest\n";
			cin >> C;
		}
		auto start = chrono::steady_clock::now();
		if (choice == 1)
			dfs(matrix);
		else if (choice == 2)
			bfs(matrix);
		else
			aStar(matrix);

		auto elapsed1 = chrono::duration_cast<chrono::milliseconds>(chrono::steady_clock::now() - start);
		auto elapsed2 = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now() - start);
		cout << "The elpased run time is: " << elapsed1.count() << " milliseconds" << " or: " << elapsed2.count() << " seconds\n\n";

		cout << "Please enter the name of the shape to be solved.\nEnter Q to quit.\n";
		cin >> file;
	}
	system("pause");

	return 0;
}
