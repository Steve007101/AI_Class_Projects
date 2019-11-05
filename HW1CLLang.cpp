/*
Steven Perry
CS 438-001
HW1 Calculator Language
1/24/19

Summary: opens 191.in to read variable assignment statements and outputs to console
	 if any variables changed.

*/

#include<iostream>
#include<fstream>
#include<string>
#include<locale>
#include<iomanip>
#include<algorithm>
using namespace std;

string* getInput(int& numberOfInputLines);
void removeBlanks(int& numberOfInputLines, string* input);
void processInput(int& numberOfInputLines, string* input, double* currentVariables, double* previousVariables);
double recursiveProcessLine(string& line, int currIndex, int endIndex, double* currentVariables);

int main()
{
	int numberOfInputLines = 0;
	string* input;
	double currentVariables[26];
	double previousVariables[26];
	// set all variables initially to 0
	for (int i = 0; i < 26; i++)
	{
		currentVariables[i] = 0;
		previousVariables[i] = 0;
	}
	// cout << "Welcome to the Calculator Language Program!\n\n";

	input = getInput(numberOfInputLines);
	removeBlanks(numberOfInputLines, input);
	processInput(numberOfInputLines, input, currentVariables, previousVariables);

	// cout << "\nThank you for using the Calculator Language Program!\n";
	system("pause");
	return 0;
}

string* getInput(int& numberOfInputLines)
{
	string* input;
	string dummyStr;
	ifstream inFile;
	inFile.open("191.in");

	if (inFile.fail())
	{
		cout << "Opening of input file 191.in failed\n";
		system("pause");
		exit(1);
	}
	// cout << "Opening of input file 191.in succeeded\n";

	while (getline(inFile, dummyStr) && dummyStr != "#")
		numberOfInputLines++;

	// cout << numberOfInputLines << " lines found in input file...\n";
	input = new string[numberOfInputLines];

	inFile.clear();
	inFile.seekg(0, ios::beg);

	for (int i = 0; i < numberOfInputLines; i++)
	{
		getline(inFile, dummyStr);
		input[i] = dummyStr;
	}

	/*
	cout << "Loaded input as follows:\n";
	for (int i = 0; i < numberOfInputLines; i++)
	{
		cout << "line " << i + 1 << ": " << input[i] << endl;
	}
	*/
	return input;
	
}

void removeBlanks(int& numberOfInputLines, string* input)
{
	for (int i = 0; i < numberOfInputLines; i++)
	{
		input[i].erase(remove(input[i].begin(), input[i].end(), ' '), input[i].end());
	}
	/*
	cout << "Input with no spaces as follows:\n";
	for (int i = 0; i < numberOfInputLines; i++)
	{
		cout << "line " << i + 1 << ": " << input[i] << endl;
	}
	*/
}

void processInput(int& numberOfInputLines, string* input, double* currentVariables, double* previousVariables)
{
	// cout << "\n\nOutput:\n";
	for (int i = 0; i < numberOfInputLines; i++)
	{
		// send line to be recursively processed to set new variable values
		recursiveProcessLine(input[i], 0, input[i].length()-1, currentVariables);
		// compare old and new variable values, output to screen if they change
		bool variableChanged = false;
		char letter;
		for (int i = 0; i < 26; i++)
		{
			if (currentVariables[i] != previousVariables[i])
			{
				// first add ", " if not first variable to be displayed
				if (variableChanged)
				{
					cout << ", ";
				}
				// set changed variable label (ASCII value is index + 65)
				letter = i + 65;
				// output changed variable
				// **** Note: since I used doubles and the sample output
				// **** shows integer truncation I truncate the numbers here
				cout << letter << " = " << (int)currentVariables[i];
				// update previousVariables
				previousVariables[i] = currentVariables[i];
				// set changed variable flag
				variableChanged = true;
			}
		}
		// if no variables were changed, output "No Change"
		if (variableChanged == false)
		{
			cout << "No Change";
		}
		// finally end line of output
		cout << endl;
	}
}

double recursiveProcessLine(string& line, int currIndex, int endIndex, double* currentVariables)
{
	/*
	right associative, all operators same precidence, assume syntactically correct
	Resursive possibilities (and other steps):
	Variable
		end of line, return value
		=, resurively process rest, set new variable value and return value
		other operator, recurvively process rest and return value
	Negative sign
		assume next is number, set it to negative and treat the same
	Number
		check if Number has multiple digits
		end of line, return value
		operator, recursively process rest and return value
	Parenthesis
	resursively process them, then:
		if end of line, return value
		if operator, recursively process the rest and return value
	*/

	char currentChar = line[currIndex]; // the first character
	char nextChar = line[currIndex+1]; // the second character, assuming it is there

	// variable A-Z (65-90 in ascii values)
	// **** Note: I am assuming variables are only a single letter
	// **** for simplicity of coding as it was not specified 
	if (currentChar >= 65 && currentChar <= 90)
	{
		int varIndex = currentChar - 65; // the equivalent index for the variable in our variable array
		//  Variable then end of line
		if (currIndex == endIndex)
		{
			return currentVariables[varIndex];
		}
		// Variable = statement
		else if (nextChar == '=')
		{
			currentVariables[varIndex] = recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables);
			return currentVariables[varIndex];
		}
		// Variable + statement
		else if (nextChar == '+')
		{
			return (currentVariables[varIndex] + recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Variable - statement
		else if (nextChar == '-')
		{
			return (currentVariables[varIndex] - recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Variable * statement
		else if (nextChar == '*')
		{
			return (currentVariables[varIndex] * recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Variable / statement
		else if (nextChar == '/')
		{
			return (currentVariables[varIndex] / recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
	}
	// Number (48-57 in Ascii values) or Negative Number sign
	else if ((currentChar >= 48 && currentChar <= 57) || currentChar == '_')
	{
		int currentNum;
		bool isNegativeNum = false;
		// if negative number sign, move ahead to actual number and set negative flag
		if (currentChar == '_')
		{
			currIndex++;
			currentChar = line[currIndex];
			nextChar = line[currIndex + 1];
			isNegativeNum = true;
		}
		// check if single digit number (digit then non-digit)
		if ((currentChar >= 48 && currentChar <= 57) && !(nextChar >= 48 && nextChar <= 57))
		{
			currentNum = currentChar - 48;
			// if negative flag, change to negative number
			if (isNegativeNum) currentNum *= -1;
		}
		// check if multi digit number (digit then digit)
		if ((currentChar >= 48 && currentChar <= 57) && (nextChar >= 48 && nextChar <= 57))
		{
			// need to find last digit in sequence
			int lastDigitPlace = currIndex+1;
			while (isdigit(line[lastDigitPlace]))
			{
				lastDigitPlace++;
			}
			lastDigitPlace--;
			// now to convert the string number to an int
			currentNum = stoi(line.substr(currIndex, (lastDigitPlace - currIndex)+1));
			// scan ahead to end of number
			currIndex = lastDigitPlace;
			currentChar = line[currIndex];
			nextChar = line[currIndex + 1];
			// if negative flag, change to negative number
			if (isNegativeNum) currentNum *= -1;
		}
		

		// Number then end of line
		if (currIndex == endIndex)
		{
			return currentNum;
		}
		// Number + statement
		else if (nextChar == '+')
		{
			return (currentNum + recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Number - statement
		else if (nextChar == '-')
		{
			return (currentNum - recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Number * statement
		else if (nextChar == '*')
		{
			return (currentNum * recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Number / statement
		else if (nextChar == '/')
		{
			return (currentNum / recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
	}
	// Parenthesis "(", should be only thing left possible
	// **** Note: the way this is coded, parenthesis variable assignments
	// **** will happen before non-parenthesis variable assignments, as
	// **** this was not specified in the instructions I went with the simpler
	// **** implimentation
	else if (currentChar == '(')
	{
		int bracketCloseIndex = currIndex+1;
		double parenthesisValue;
		// first we need to find what index holds the closing bracket
		while(line[bracketCloseIndex] != ')')
		{
			bracketCloseIndex++;
		}
		// now we recursively find the value in brackets
		parenthesisValue = recursiveProcessLine(line, currIndex + 1, bracketCloseIndex - 1, currentVariables);
		// move indecies/Chars up and proceed as if the brackets were a number
		currIndex = bracketCloseIndex;
		currentChar = line[currIndex];
		nextChar = line[currIndex + 1];
		// Parenthesis Value then end of line
		if (currIndex == endIndex)
		{
			return parenthesisValue;
		}
		// Parenthesis Value + statement
		else if (nextChar == '+')
		{
			return (parenthesisValue + recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Parenthesis Value - statement
		else if (nextChar == '-')
		{
			return (parenthesisValue - recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Parenthesis Value * statement
		else if (nextChar == '*')
		{
			return (parenthesisValue * recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}
		// Parenthesis Value / statement
		else if (nextChar == '/')
		{
			return (parenthesisValue / recursiveProcessLine(line, currIndex + 2, endIndex, currentVariables));
		}

	}
	// if no valid input was found, display error and close
	else
	{
		cout << "Bad input, closing program...";
		system("pause");
		exit(1);
		return 0;
	}
}