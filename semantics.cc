/*
 *==================================================
 *File  :   semantics.cc
 *Author:   Christopher MacDonald
 *==================================================
 */

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include <cmath>



using namespace std;

//Multivalue provided by Professor Irfan with added bool and char values
union Multivalue {
    int iValue;
    float fValue;
    bool bValue;
    char cValue;
} value;

//heterogeneous object holds a type and a value (provided by Professor Irfan)
class Heterogeneous {
public:
    string type;
    Multivalue value;
    
    //constructor
    Heterogeneous(string type, Multivalue value) {
        this->type = type;
        this->value = value;
    }
    
    //default constructor
    Heterogeneous() {}
};

//global variables used are vectors for holding tokens and lexemes,
//a counter for keeping track of current index in the token vector,
//an index to keep track of the most recent type lexeme in a string of declarations,
//and a map to hold symbols with their types and values
vector<string> *tokens;
vector<string> *lexemes;
int currToken;
int lastTypeIndex;
map<string, Heterogeneous> symTable;

//function prototypes to allow for forward referencing
void program ();
void declarations ();
void declaration ();
void statements();
void statement(bool assign);
void assignment (bool assign);
Heterogeneous expression ();
Heterogeneous conjunction ();
Heterogeneous equality ();
Heterogeneous relation ();
Heterogeneous addition ();
Heterogeneous term ();
Heterogeneous factor();
void printStmt();
void ifStmt();
void whileStmt();
void returnStmt();
void addSymbol();



/*
 *===========================================
 *  MAIN -- read in tokens and begin parsing
 *===========================================
 */
int main ( int argc, char *argv[] ) {
    

    //use ifstream to read in file
    ifstream input;
    
    //check for correct number of arguments
    if ( argc != 2 ) return 0;
    
    //read in input file
    string argFile = argv[1];
    input.open(argFile.c_str());
    
    //check for non-existent input file
    if ( !input.is_open() ) {
        cout << "Error: could not open input file " << argFile << endl;
        return 0;
    }
    
    //check to make sure input file is not empty
    else if ( input.peek() == ifstream::traits_type::eof() ) {
        input.close();
        input.clear();
        cout << "Error: empty input file " << argFile << endl;
        return 0;
    }
    
    //used to read in and store input file data
    int counter = 0;
    string word;
    lexemes = new vector<string>;
    tokens = new vector<string>;

    //read in tokens and lexemes from input file and store them in respective vectors
    while ( input >> word ) {

        if ( counter % 2 == 0 ) {
            tokens->push_back( word );
        }
        else {
            lexemes->push_back( word );
        }
        counter++;
    }
    
    //initialize index value and begin parsing by calling program method
    currToken = -1;
    program();
      
    //free memory
    delete lexemes;
    delete tokens;
    lexemes = 0;
    tokens = 0;
      
    //close the input file
    input.close();
}

/*
 *===================================================================
 *  PROGRAM FCN -- consume tokens at start and end of a program, 
 *                 call fcns to consume other tokens in declarations
 *                 and statements
 *===================================================================
 */

void program () {
    

    //advance index to first element in token vector and begin by consuming a type
    string programToken = tokens->at(++currToken);
    if ( programToken != "type" ) {
        cout << "Error: 'type' token missing for main function return value" << endl;
        exit(0);
    }
    //consume 'main' token
    programToken = tokens->at(++currToken);
    if ( programToken != "main" ) {
        cout << "Error: 'main' token missing" <<endl;
        exit(0);
    }
    //consume '(' token
    programToken = tokens->at(++currToken);
    if ( programToken != "(" ) {
        cout << "Error: '(' token missing in main function" <<endl;
        exit(0);
    }
    //consume ')' token
    programToken = tokens->at(++currToken);
    if ( programToken != ")" ) {
        cout << "Error: ')' token missing in main function" <<endl;
        exit(0);
    }
    //consume '{' token
    programToken = tokens->at(++currToken);
    if ( programToken != "{" ) {
        cout << "Error: '{' token missing at beginning of main function" <<endl;
        exit(0);
    }
    
    //parse for all declarations, then parse for all statements
    declarations();
    statements();
    
    //consume '}' token for end of main function
    programToken = tokens->at(++currToken);
    if ( programToken != "}" ) {
        cout << "Error: '}' token missing at end of main function" <<endl;
        exit(0);
    }
}

/* 
 *=======================================
 *  FCNS FOR DECLARATIONS AND STATEMENTS
 *=======================================
 */ 


void declarations () {
    //parse all declarations first
    while ( currToken < tokens->size() - 1 ) {
        //if the next token is not 'type', it must not be the start of a new declaration
        string typeToken = tokens->at(++currToken);
        if ( typeToken != "type" ) {
            currToken--;
            return;
        }
        declaration();
    }
}


void declaration () {
    
    //save the index of the last type so we can refer back to that type if there is a
    //string of comma-separated declarations
    lastTypeIndex = currToken;
    
    //if last token was 'type', next token consumed must be 'id'
    string decToken = tokens->at(++currToken);
    if ( decToken == "id" ) {
        //put new id in the symbol table
        addSymbol();
    }
    else {
        cout << "Error: missing 'id' token at start of declaration" << endl;
        exit(0);
    }
    //check for series of declarations separated by commas
    while ( currToken < tokens->size() - 1 ) {
        decToken = tokens->at(++currToken);
        if ( decToken != "," ) {
            currToken--;
            break;
        }
        decToken = tokens->at(++currToken);
        if ( decToken != "id" ) {
            cout << "Error: missing 'id' token in series of declarations" << endl;
            exit(0);
        }
        //if another variable is declared, must add it to symbol table
        addSymbol();
    }
    //consume ';' at end of line of declaration(s)
    decToken = tokens->at(++currToken);
    if ( decToken != ";" ) {
        cout << "Error: ';' token missing from end of declaration" << endl;
        exit(0);
    }
}


void statements () {

    //parse all statements after all declarations have been parsed
    string stateToken;
    while ( currToken < tokens->size() - 1 ) {
        //check for a token indicating a new statement
        stateToken = tokens->at(++currToken);
        if ( stateToken != "id" && stateToken != "print" && stateToken != "if" &&
            stateToken != "while" && stateToken != "return" )  {
            currToken--;
            return;
        }
        else {
            currToken--;
            //argument is true because we will allow a variable's value to be updated in the
            //symbol table while declarations and statements are being made
            statement(true);
        }
    }
}


void statement (bool assign) {
    
    int statementToken = currToken;
    //check for each kind of statement   
    assignment(true);
    //don't check multiple types of statements
    if ( currToken != statementToken ) {
        return;
    }
    printStmt();
    if ( currToken != statementToken ) {
        return;
    }
    ifStmt();
    if ( currToken != statementToken ) {
        return;
    }
    whileStmt();
    if ( currToken != statementToken ) {
        return;
    }
    returnStmt();
}

/*
 *=================================
 *  FCNS FOR ASSIGNMENT STATEMENTS
 *=================================
 */

void assignment (bool assign) {
    
    //if next token is not 'id', must not be an assignment statement
    string assignToken = tokens->at(++currToken);
    if ( assignToken != "id" ) {
        currToken--;
        return;
    }
    
    //save id
    string id = lexemes->at(currToken);

    //consume 'assignOp' token
    assignToken = tokens->at(++currToken);
    if ( assignToken != "assignOp" ) {
        cout << "Error: 'assignOp' token missing from assignment" << endl;
        exit(0);
    }
    
    //get the expression following the assignment
    Heterogeneous assignVal = expression();
    
    //check if variable is of same type as its assignment (and that 'assign' is true)
    if ( symTable[id].type == assignVal.type && assign ) {
        symTable[id] = assignVal;
    }
    //widening conversion for floats (Type Rule 3)
    else if ( symTable[id].type == "float" && assignVal.type == "int" && assign ) {
        symTable[id].value.fValue = assignVal.value.iValue;
    }

    //consume ';' token at end of assignment
    assignToken = tokens->at(++currToken);
    if ( assignToken != ";" ) {
        cout << "Error: ';' token missing from end of assignment" << endl;
        exit(0);
    }
}


Heterogeneous expression () {

    //need to save value of an expression, return it so that:
    //  1.  we can print it
    //  2.  we can update a value in the symbol table
    //      --> can do this with a heterogeneous object
    //      --> only can mix floats and ints (not bools and chars)
    
    //parse the conjunction, save the result for semantic analysis
    Heterogeneous result = conjunction();
    Heterogeneous temp;
    
    //as long as next token is '||', parse all ensuing conjunctions
    string expToken; 
    while ( currToken < tokens->size() - 1 ) {
        expToken = tokens->at(++currToken);
        if ( expToken == "||" ) {

            //check to make sure result has type bool; only bools
            //can use logical operators
            if ( result.type != "bool" ) {
                cout << "Error: cannot use logical operator '||' on non-boolean types" << endl;
                exit(0); 
            }

            //get next conjunction production
            temp = conjunction();

            //make sure temp has type 'bool'
            if ( temp.type != "bool" ) {
                cout << "Error: cannot use logical operator '||' on non-boolean types" << endl;
                exit(0); 
            }

            //update the value of the result
            if ( result.value.bValue == true || temp.value.bValue == true ) {
                result.value.bValue = true;
            }
            else {
                result.value.bValue = false;
            }
        }
        else {
            currToken--;
            break;
        }
    }
    return result;
}


Heterogeneous conjunction () {

    //parse the equality, save the result for semantic analysis
    Heterogeneous result = equality();
    Heterogeneous temp;

    //as long as next token is '&&', parse all ensuing equalities
    string conjToken;
    while ( currToken < tokens->size() - 1 ) {
        conjToken = tokens->at(++currToken);
        if ( conjToken == "&&" ) {

            //check to make sure result has type bool; if not, error 
            //(cannot use logical operators on non-boolean types)
            if ( result.type != "bool" ) {
                cout << "Error: cannot use logical operator '&&' on non-boolean types" << endl;
                exit(0); 
            }

            //get next equality production
            temp = equality();

            //check if temp has type 'bool'
            if ( temp.type != "bool" ) {
                cout << "Error: cannot use logical operator '&&' on non-boolean types" << endl;
                exit(0); 
            }

            //update the value of the result
            if ( result.value.bValue == true && temp.value.bValue == true ) {
                result.value.bValue = true;
            }
            //NOTE: once the value is false, it will always be false (one false bool in a 
            //string of bools connected by &&'s makes the entire string false).  However,
            //we can't break the loop here because we might have to keep consuming more tokens,
            //i.e. there might be another equality production next
            else {
                result.value.bValue = false;
            }
        }
        else {
            currToken--;
            break;
        }
    }
    return result;
}


Heterogeneous equality () {
    
    //parse the relation, save the result for semantic analysis
    Heterogeneous result = relation();
    Heterogeneous temp;

    //keep track of which equality symbol
    bool equal = false;

    //if next token is 'equOp', parse another relation
    string equToken = tokens->at(++currToken);
    if ( equToken == "equOp" ) {
     
        //find which equality symbol
        if ( lexemes->at(currToken) == "==" ) {
            equal = true;
        }

        //get next relation production
        temp = relation();

        //cannot do comparison of chars or bools in this program
        if ( temp.type == "charLiteral" || temp.type == "boolLiteral" || result.type
            == "charLiteral" || result.type == "boolLiteral" ) {
            cout << "Error: cannot perform comparison on chars or bools" << endl;
            exit(0);
        }

        //keep track of whether either temp or result has type 'float'
        bool tempFloat = false;
        bool resultFloat = false;
        if ( temp.type == "float" ) {
            tempFloat = true;
        }
        if ( result.type == "float" ) {
            resultFloat = true;
        }

        //update result type to boolean 
        result.type = "bool";

        //update value of result if temp and result have type 'float'
        if ( tempFloat && resultFloat ) {
            if ( equal ) 
                result.value.bValue = result.value.fValue == temp.value.fValue;
            else
                result.value.bValue = result.value.fValue != temp.value.fValue;
        }
        //update value of result if temp has type 'int' and result has type 'float'
        else if ( !tempFloat && resultFloat ) {
            if ( equal ) 
                result.value.bValue = result.value.fValue == temp.value.iValue;
            else
                result.value.bValue = result.value.fValue != temp.value.iValue;
        }
        //update value of result if temp has type 'float' and result has type 'int'
        else if ( tempFloat && !resultFloat ) {
            if ( equal ) 
                result.value.bValue = result.value.iValue == temp.value.fValue;
            else
                result.value.bValue = result.value.iValue != temp.value.fValue;
        }
        //update value of result if temp and result have type 'int'
        else {
            if ( equal ) 
                result.value.bValue = result.value.iValue == temp.value.iValue;
            else
                result.value.bValue = result.value.iValue != temp.value.iValue;
        }
        equal = false;
    }
    else {
        currToken--;
    }
    return result;
}


Heterogeneous relation () {

    //parse the addition, save the result for semantic analysis
    Heterogeneous result = addition();
    Heterogeneous temp;

    //keep track of which relative operator
    bool less = false;
    bool leq = false;
    bool gre = false;
    bool greq = false;

    //if next token is 'relOp', parse for addition again
    string relToken = tokens->at(++currToken);
    if ( relToken == "relOp" ) {
        string relLex = lexemes->at(currToken);
    
        //find which relative operator
        if ( lexemes->at(currToken) == "<" ) {
            less = true;
        }
        else if ( lexemes->at(currToken) == "<=" ) {
            leq = true;
        }
        else if ( lexemes->at(currToken) == ">" ) {
            gre = true;
        }
        else {
            greq = true;
        }

        //get next addition production
        temp = addition();

        //cannot do relative comparison of chars or bools in this program
        if ( temp.type == "charLiteral" || temp.type == "boolLiteral" || result.type
            == "charLiteral" || result.type == "boolLiteral" ) {
            cout << "Error: cannot perform relative comparison on chars or bools" << endl;
            exit(0);
        }

        //keep track of whether either temp or result has type 'float'
        bool tempFloat = false;
        bool resultFloat = false;
        if ( temp.type == "float" ) {
            tempFloat = true;
        }
        if ( result.type == "float" ) {
            resultFloat = true;
        }

        //update result type to boolean 
        result.type = "bool";

        //update value of result if temp and result have type 'float'
        if ( tempFloat && resultFloat ) {
            if ( less )
                result.value.bValue = result.value.fValue < temp.value.fValue;
            else if ( leq )
                result.value.bValue = result.value.fValue <= temp.value.fValue;
            else if ( gre )
                result.value.bValue = result.value.fValue >= temp.value.fValue;
            else
                result.value.bValue = result.value.fValue > temp.value.fValue;
        }
        //update value of result if temp has type 'int' and result has type 'float'
        else if ( !tempFloat && resultFloat ) {
            if ( less )
                result.value.bValue = result.value.fValue < temp.value.iValue;
            else if ( leq )
                result.value.bValue = result.value.fValue <= temp.value.iValue;
            else if ( gre )
                result.value.bValue = result.value.fValue >= temp.value.iValue;
            else
                result.value.bValue = result.value.fValue > temp.value.iValue;
        } 
        //update value of result if temp has type 'float' and result has type 'int'
        else if ( tempFloat && !resultFloat ) {
            if ( less )
                result.value.bValue = result.value.iValue < temp.value.fValue;
            else if ( leq )
                result.value.bValue = result.value.iValue <= temp.value.fValue;
            else if ( gre )
                result.value.bValue = result.value.iValue >= temp.value.fValue;
            else
                result.value.bValue = result.value.iValue > temp.value.fValue;
        }  
        //update value of result if temp and result have type 'int'
        else {
            if ( less )
                result.value.bValue = result.value.iValue < temp.value.iValue;
            else if ( leq )
                result.value.bValue = result.value.iValue <= temp.value.iValue;
            else if ( gre )
                result.value.bValue = result.value.iValue >= temp.value.iValue;
            else
                result.value.bValue = result.value.iValue > temp.value.iValue;
        } 

        less = false;
        leq = false;
        gre = false;
        greq = false;
    }
    else {
        currToken--;
    }
    return result;
}


Heterogeneous addition () {

    //parse a term, save the result for semantic analysis
    Heterogeneous result = term();
    Heterogeneous temp;

    //keep track of which addOp operand
    bool plus = false;

    //as long as next token is 'addOp', parse another term
    string addToken;
    while ( currToken < tokens->size() - 1 ) {
        addToken = tokens->at(++currToken);
        if ( addToken == "addOp" ) {
            
            //find out which operation to perform
            if ( lexemes->at(currToken) == "+" ) {
                plus = true;
            }

            //find next term production
            temp = term();

            //cannot add or subract chars or bools
            if ( temp.type == "charLiteral" || temp.type == "boolLiteral" || result.type
                == "charLiteral" || result.type == "boolLiteral" ) {
                cout << "Error: cannot perform +|- on chars or bools" << endl;
                exit(0);
            }

            //keep track of whether either temp or result has type 'float'
            bool tempFloat = false;
            bool resultFloat = false;

            //if one factor is a 'float', the type of the result must be 'float'; otherwise type is 'int'
            if ( temp.type == "floatLiteral" || result.type == "floatLiteral" ) {
                result.type = "float";

                if ( temp.type == "floatLiteral" ) {
                    tempFloat = true;
                }
                if ( result.type == "floatLiteral" ) {
                    resultFloat = true;
                }

                //update value in case where temp is 'int', result is 'float'
                if ( !tempFloat ) {
                    if ( plus ){
                        result.value.fValue = result.value.fValue + temp.value.iValue;
                    }
                    else {
                        result.value.fValue = result.value.fValue - temp.value.iValue;
                    }
                }
                //update value in case where temp is 'float', result is 'int'
                else if ( !resultFloat ) {
                    if ( plus ){
                        result.value.fValue = result.value.iValue + temp.value.fValue;
                    }
                    else {
                        result.value.fValue = result.value.iValue - temp.value.fValue;
                    }
                }
                //update value in case when both temp and result have type 'float'
                else {
                    if ( plus ){
                        result.value.fValue = result.value.fValue + temp.value.fValue;
                    }
                    else {
                        result.value.fValue = result.value.fValue - temp.value.fValue;
                    }
                }
            }
            //type of result is 'int', update value (both temp and result have type 'int')
            else {
                if ( plus ){
                        result.value.iValue = result.value.iValue + temp.value.iValue;
                }
                else {
                        result.value.iValue = result.value.iValue - temp.value.iValue;
                }
            }
            plus = false;
        }
        else {
            currToken--;
            break;
        }
    }
    return result;
}


Heterogeneous term () {

    //parse a factor, save the result for semantic analysis
    Heterogeneous result = factor();
    Heterogeneous temp;

    //keep track of which multOp operand
    bool mult = false;
    bool divide = false;
    bool mod = false;

    //as long as next token is 'multOp', parse another factor
    string termToken;
    while ( currToken < tokens->size() - 1 ) {
        termToken = tokens->at(++currToken);
        if ( termToken == "multOp" ) {

            //keep track of which multOp is being used 
            if ( lexemes->at(currToken) == "*" ) {
                mult = true;
            }
            else if ( lexemes->at(currToken) == "/" ) {
                divide = true;
            }
            else if ( lexemes->at(currToken) == "%" ) {
                mod = true;
            }

            //get next factor
            temp = factor();
            
            //cannot multiply or divide chars or bools
            if ( temp.type == "charLiteral" || temp.type == "boolLiteral" || result.type
                == "charLiteral" || result.type == "boolLiteral" ) {
                cout << "Error: cannot perform *|/ on chars or bools" << endl;
                exit(0);
            }

            //keep track of whether either temp or result has type 'float'
            bool tempFloat = false;
            bool resultFloat = false;

            //if one factor is a 'float', the type of the result must be 'float'; otherwise type is 'int'
            if ( temp.type == "floatLiteral" || result.type == "floatLiteral" ) {
                result.type = "float";

                if ( temp.type == "floatLiteral" ) {
                    tempFloat = true;
                }
                if ( result.type == "floatLiteral" ) {
                    resultFloat = true;
                }

                //if temp is an int and result is a float, calculate new value
                if ( !tempFloat ) {
                    if ( mult ) {
                        result.value.fValue = result.value.fValue * temp.value.iValue;
                    }
                    else if ( divide ) {
                        result.value.fValue = result.value.fValue / temp.value.iValue;
                    }
                    else if ( mod ) {
                        result.value.fValue = fmod(result.value.fValue, temp.value.iValue);
                    }
                }
                //if temp is a float and result is an int, calculate new value
                else if ( !resultFloat ) {
                    if ( mult ) {
                        result.value.fValue = result.value.iValue * temp.value.fValue;
                    }
                    else if ( divide ) {
                        result.value.fValue = result.value.iValue / temp.value.fValue;
                    }
                    else if ( mod ) {
                        result.value.fValue = fmod(result.value.iValue, temp.value.fValue);
                    }
                }
                //if both are floats, calculate new value
                else {
                    if ( mult ) {
                        result.value.fValue = result.value.fValue * temp.value.fValue;
                    }
                    else if ( divide ) {
                        result.value.fValue = result.value.fValue / temp.value.fValue;
                    }
                    else if ( mod ) {
                        result.value.fValue = fmod(result.value.fValue, temp.value.fValue);
                    }
                }
            }
            //else the result is an int, calculate new value
            else {
                if ( mult ) {
                    result.value.iValue = result.value.iValue * temp.value.iValue;
                }
                else if ( divide ) {
                    result.value.iValue = result.value.iValue / temp.value.iValue;
                }
                else if ( mod ) {
                    result.value.iValue = result.value.iValue % temp.value.iValue;
                }
            }
            mult = false;
            divide = false;
            mod = false;
        }
        else {
            currToken--;
            break;
        }
    }
    return result;
}


Heterogeneous factor () {

    Heterogeneous result;
    
    //if next token is an id or literal, syntax for factor is correct
    string factor = tokens->at(++currToken);
    if ( factor == "id" || factor == "intLiteral" || factor == "boolLiteral" || 
        factor == "floatLiteral" || factor == "charLiteral" ) {
        
        //the type of the expression is the factor (unless factor is "id")
        string type = factor;
    
        Multivalue value;
        
        //if factor is an id, that heterogeneous object already exists in the symbol table
        if ( type == "id") {
            if ( symTable.count(lexemes->at(currToken)) == 0 ) {
                cout << "Error: use of undeclared identifier" << endl;
                exit(0);
            }
            result = symTable[lexemes->at(currToken)];
            
        }
        else if ( type == "intLiteral" ) {
            value.iValue = stoi(lexemes->at(currToken));
            result.value = value;
            result.type = "int";
        }
        else if ( type == "boolLiteral" ) {
            //NOTE: assumes syntactic correctness (doesn't check if bool is assigned something other
            //than true or false
            if (lexemes->at(currToken) == "true")
                value.bValue = true;
            else
                value.bValue = false;
            result.value = value;
            result.type = "bool";
        }
        else if ( type == "floatLiteral" ) {
            value.fValue = stof(lexemes->at(currToken));
            result.value = value;
            result.type = "float";
        }
        else {
            value.cValue = lexemes->at(currToken)[0];
            result.value = value;
            result.type = "char";
        }
    }

    //if next token is '(', factor must be an expression
    else if ( factor == "(" ){
        result = expression();
        factor = tokens->at(++currToken);

        //consume ')' at end of expression factor
        if ( factor != ")" ) {
            cout << "Error: missing ')' token in Factor -> (Expression)" << endl;
            exit(0);
        }
    }
    
    //if next token is not a factor, print error message and exit
    else {
        cout << "Error: missing factor" << endl;
        exit(0);
    }
    return result;
}

/*
 *=====================================
 *  FCNS FOR NON-ASSIGNMENT STATEMENTS 
 *=====================================
 */

void returnStmt () {

    //if next token consumed is not 'return', not a return statement
    string retToken = tokens->at(++currToken);
    if ( retToken != "return" ) {
        currToken--;
        return;
    }
    
    //parse an expression, "result" holds the object returned
    Heterogeneous result = expression();

    //consume ';' token at end of return statement
    retToken = tokens->at(++currToken);
    if ( retToken != ";" ) {
        cout << "Error: ';' token missing at end of returnStmt" << endl;
        exit(0);
    }
}

void ifStmt () {

    //if next token consumed is not 'if', not an if statement
    string ifToken = tokens->at(++currToken);
    if ( ifToken != "if" ) {
        currToken--;
        return;
    }

    //consume '(' token at start of if statement
    ifToken = tokens->at(++currToken);
    if ( ifToken == "(" ) {
        //get expression
        Heterogeneous ifVal = expression();

        //check to make sure if statement condition is boolean expression
        if ( ifVal.type != "bool" ) {
            cout << "Error: must have boolean expression in if-statement condition" << endl;
            exit(0);
        }

        //consume ')' at end of if statement
        ifToken = tokens->at(++currToken);
        if ( ifToken == ")" ) {

            statement( ifVal.value.bValue );

            //check for else statement
            ifToken = tokens->at(++currToken);
            if ( ifToken == "else" ) {
                statement( !ifVal.value.bValue );
            }
            else {
                currToken--;
            }
        }
        else {
            cout << "Error: ')' token missing in ifStmt" << endl;
            exit(0);
        }
    }
    else {
            cout << "Error: '(' token missing in ifStmt" << endl;
            exit(0);
    }
}

void printStmt () {

    //if next token consumed is not 'print', not a print statement
    string pToken = tokens->at(++currToken);
    if ( pToken != "print" ) {
        currToken--;
        return;
    }

    //get an expression
    else {
        
        Heterogeneous printVal = expression();

        if ( printVal.type == "int" ) {
            cout << printVal.value.iValue << endl;
        }
        else if ( printVal.type == "float" ) {
            cout << printVal.value.fValue << endl;
        }
        else if ( printVal.type == "bool" ) {
            cout << printVal.value.bValue << endl;
        }
        else {
            cout << printVal.value.cValue << endl;
        }
    }

    //consume ';' token at end of print statement
    pToken = tokens->at(++currToken);
    if ( pToken != ";" ) {
        cout << "Error: missing ';' at end of printStmt" << endl;
        exit(0);
    }
}

void whileStmt () {

    //if next token consumed is not 'while', not a while statement
    string wToken = tokens->at(++currToken);
    if ( wToken != "while" ) {
        currToken--;
        return;
    }
    
    Heterogeneous whileVal;

    //consume '(' token at beginning of while statement
    wToken = tokens ->at(++currToken);
    if ( wToken == "(" ) {
        //keep track of starting while token, will reset this every time while loop runs
        //--> this is because we want to parse the same expression until it is no longer
        //    true
        int startWhileToken = currToken;
        int endWhileToken = 0;
         
        do {
            currToken = startWhileToken;

            //parse an expression
            whileVal = expression();

            //while statement condition expression must have type 'bool'
            if ( whileVal.type != "bool" ) {
                cout << "Error: must have boolean expression as while-statement condition" << endl;
                exit(0);
            }

            //consume ')' token at end of expression
            wToken = tokens ->at(++currToken);
            if ( wToken == ")" ) { 

                //if condition is no longer true, don't parse the statement
                if ( !whileVal.value.bValue && endWhileToken != 0 ) {
                    currToken = endWhileToken;
                    break;
                }
                //parse a statement
                statement(whileVal.value.bValue);
            }
            else {
                cout << "Error: missing ')' token in whileStmt" << endl;
                exit(0);
            }

            endWhileToken = currToken;

        } while (whileVal.value.bValue);

    }
    else {
        cout << "Error: missing '(' token in whileStmt" << endl;
        exit(0);
    }
}

/*
 *=====================================
 *         FCNS FOR TYPE SYSTEM
 *=====================================
 */

void addSymbol () {
    
    string varName = lexemes->at(currToken);
    string varType = lexemes->at(lastTypeIndex);
    
    //make sure no two variables have the same name
    if ( symTable.count(varName) != 0 ) {
        cout << "Error: " << varName << " is already being used as an identifier" << endl;
        exit(0);
    }
    
    //value has not yet been assigned
    Multivalue mv;
    
    //make a new entry with the type and value
    Heterogeneous entry (varType, mv);
    
    //add new entry in the symbol table using variable name as key
    symTable[varName] = entry;
}
