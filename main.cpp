#include <iostream>
#include <string>
#include <fstream>
#include <chrono>

using namespace std;

struct Token
{
    int line;
    string type;
    string value;
};

struct IR
{
    int lineNum;
    string opcode;

    int opc1;
    int opc2;
    int opc3;

    IR* next;
    IR* prev;

    IR(int lineNum, string opcode, int opc1, int opc2, int opc3){
        this->lineNum = lineNum;
        this->opcode = opcode;

        this->opc1 = opc1;
        this->opc2 = opc2;
        this->opc3 = opc3;

        this->next = nullptr;
        this->prev = nullptr;
    }
};

void addIR(IR*& head, IR*& tail, IR* newIR) {
    if (!head) {
        head = newIR;
        tail = newIR;
    } else {
        tail->next = newIR;
        newIR->prev = tail;
        tail = newIR;
    }
}


pair<string, string> getNextToken(char currentChar, ifstream& inputFile);

int main(int argc, char* args[]) {
    //auto start = chrono::high_resolution_clock::now();

    bool hFlag = false;
    bool rFlag = false;
    bool pFlag = false;
    bool sFlag = false;

    bool memopFlag = false;
    bool loadIFlag = false;
    bool arithopFlag = false;
    bool outputFlag = false;
    bool nopFlag = false;
    bool grammarErrorFlag = false;

    int count = 0;

    IR* head = nullptr;
    IR* tail = nullptr;

    int lineNum = 1;
    string currentOpCode = "";

    int opc1 = -42;
    int opc2 = -42;
    int opc3 = -42;


    string filename = "";

    int totalFlags = 0;
    for(int i = 1; i < argc; i++){
        string arg = args[i];
        if(arg == "-h"){
            hFlag = true;
            totalFlags++;
        } else if(arg == "-r"){
            rFlag = true;
            totalFlags++;
        } else if(arg == "-p"){
            pFlag = true;
            totalFlags++;
        } else if(arg == "-s"){
            sFlag = true;
            totalFlags++;
        } else if (filename.empty()){
            filename = arg;
        }
    }



    if(totalFlags > 1){
        cout << "Multiple flags have been set in the command. We will only be using the highest priority flag." << endl;
    }

    string whatFlag = "";

    if(hFlag){
        whatFlag = "-h";
    } else if(rFlag){
        whatFlag = "-r";
    } else if(pFlag){
        whatFlag = "-p";
    } else if(sFlag){
        whatFlag = "-s";
    }

    if(whatFlag == ""){
        whatFlag = "-p";
    }

    if (hFlag) {
        cout << "Flags and their meanings:" << endl;
        cout << "-h (current flag): Show flags and their meanings." << endl;
        cout << "-r: 434fe -r <name>: Read ILOC code from name and scan, parse, build IR and print out the IR." << endl;
        cout << "-p: 434fe -p <name>: Read ILOC code from name and scan, parse, build IR and report either success or report all errors in file <name>." << endl;
        cout << "-s: 434fe -s <name>: Read ILOC code and print to standard output stream a list of tokens the scanner found." << endl;
        return 0;
    }

    if(filename.empty()){
        cerr << "No input file specified." << endl;
        return 1;
    }

    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename   << endl;
        return 1;
    }

    char currentChar;
    string currentToken;
    char nextChar;
    int line = 1;
    Token token;

    while (inputFile.get(currentChar)) {

        bool errorFlag = false;
        bool madeToken = false;

        if (currentChar == '\n') {
            token = {line, "EOL", "\\n"};
            if(whatFlag == "-s"){
                cout << line << " <" << token.type << ", " << token.value << ">" << endl;
            }
            line++;
            madeToken = true;
            //continue;
        }

        else if(currentChar == ' ' || currentChar == '\t' || currentChar == '\r'){
            continue;
        }

        else if(currentChar == '/'){
            int peakChar = inputFile.peek();

            if(peakChar != EOF  && (char)peakChar == '/'){
                inputFile.get(nextChar);

                while(currentChar != '\n' && inputFile.get(currentChar)){
                    ; //heeeeeyyyyy we skipping through here again. fun fact: the bigget mammal in the world is a blue whale.
                }
                token = {line, "EOL", "\\n"};

                if(whatFlag == "-s"){
                    cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                }

                line++;
                madeToken = true;
                //continue;
            } else {
                cerr << "ERROR" << line << ": Invalid token (potential comment), skipping to the end of the current line" << endl;
                while(currentChar != '\n' && inputFile.get(currentChar)){
                    ; //second fun fact: honey is the only food that never goes bad (i learned this recently)
                }
                token = {line, "EOL", "\\n"};

                if(whatFlag == "-s"){
                    cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                }

                line++;
                madeToken = true;
                //continue;
            }
        }

        if(!madeToken){
            int peekAheadChar = inputFile.peek();
            
            if(peekAheadChar == EOF){
                break;
            } else {
                nextChar = (char)peekAheadChar;
            }

            currentToken.clear();

            if(currentChar == 's' || currentChar == 'l' || currentChar == 'r' || currentChar == 'm' || currentChar == 'a' || currentChar == 'n' || currentChar == 'o' || currentChar == '=' || currentChar == ',' || currentChar == '\n' || (currentChar >= '0' && currentChar <= '9') || currentChar == '/'){
                currentToken += currentChar;
            } else {
                cerr << "ERROR" << line << ": Start of token is invalid, skipping to end" << endl;

                while(currentChar != '\n' && inputFile.get(currentChar)){
                    //hello. this is a useless comment. hope you read it! Have a good day whoever is grading this haha
                    ;
                }
                line++;
                continue;
            }

            bool leadingZeroFlag = true;

            if (currentToken[0] == 's'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "store"){
                    token = {line, "MEMOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else if (currentToken == "sub"){
                    token = {line, "ARITHOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == 'l'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "load"){
                    token = {line, "MEMOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else if (currentToken == "loadI"){
                    token = {line, "LOADI", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else if (currentToken == "lshift"){
                    token = {line, "ARITHOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == 'r'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    if(nextChar == '0' && currentToken == "r"){
                        inputFile.get(nextChar);
                        continue;
                    }

                    leadingZeroFlag = false;
                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "rshift"){
                    token = {line, "ARITHOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else if (currentToken == "r"){
                    currentToken = "r0";
                    token = {line, "REGISTER", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    madeToken = true;
                } else if (currentToken.size() > 1 && isdigit(currentToken[1])){
                    token = {line, "REGISTER", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == 'm'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "mult"){
                    token = {line, "ARITHOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == 'a'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "add"){
                    token = {line, "ARITHOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == 'n'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "nop"){
                    token = {line, "NOP", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == 'o'){
                while(true){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if(nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                if (currentToken == "output"){
                    token = {line, "OUTPUT", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }
            } else if (currentToken[0] == '='){
                if (nextChar == '>'){
                    inputFile.get(nextChar);
                    currentToken += nextChar;
                    token = {line, "INTO", currentToken};
                    if(whatFlag == "-s"){
                        cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                    }
                    currentToken.clear();
                    madeToken = true;
                } else {
                    errorFlag = true;
                }   
            } else if (currentToken[0] == ','){
                token = {line, "COMMA", currentToken};
                if(whatFlag == "-s"){
                    cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                }
                currentToken.clear();
                madeToken = true;
            } else if (currentToken[0] >= '0' && currentToken[0] <= '9'){
                bool skipLoop = false;

                int peakChar = inputFile.peek();
                if(peakChar == EOF){
                    skipLoop = true;
                } else {
                    nextChar = (char)peakChar;
                }

                if (nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                    skipLoop = true;
                }

                while(true && !skipLoop){
                    int peakChar = inputFile.peek();
                    if(peakChar == EOF){
                        break;
                    } else {
                        nextChar = (char)peakChar;
                    }

                    if (nextChar == ' ' || nextChar == '\n' || nextChar == '\t' || nextChar == '\r' || nextChar == ',' || nextChar == '=' || nextChar == '/'){
                        break;
                    }

                    if(nextChar < '0' || nextChar > '9'){
                        errorFlag = true;
                        break;
                    }

                    if(nextChar == '0' && currentToken == "0"){
                        inputFile.get(nextChar);
                        continue;
                    }

                    leadingZeroFlag = false;
                    inputFile.get(nextChar);
                    currentToken += nextChar;
                }

                token = {line, "CONSTANT", currentToken};
                if(whatFlag == "-s"){
                    cout << line << " <" << token.type << ", " << token.value << ">" << endl;
                }
                madeToken = true;
            } else {
                errorFlag = true;
            }
        }
        //END OF THE WHILE FOR SCANNER

        //PARSER LOGIC

        if(!madeToken){
            continue;
        }

        if(token.type == "MEMOP"){
            memopFlag = true;
            loadIFlag = false;
            arithopFlag = false;
            outputFlag = false;
            nopFlag = false;

            currentOpCode = token.value;
            lineNum = token.line;

            opc1 = -42;
            opc2 = -42;
            opc3 = -42;

            count = 1;
            continue;
        } else if(token.type == "LOADI"){
            loadIFlag = true;
            memopFlag = false;
            arithopFlag = false;
            outputFlag = false;
            nopFlag = false;

            currentOpCode = token.value;
            lineNum = token.line;

            opc1 = -42;
            opc2 = -42;
            opc3 = -42;

            count = 1;
            continue;
        } else if(token.type == "ARITHOP"){
            arithopFlag = true;
            memopFlag = false;
            loadIFlag = false;
            outputFlag = false;
            nopFlag = false;

            currentOpCode = token.value;
            lineNum = token.line;

            opc1 = -42;
            opc2 = -42;
            opc3 = -42;

            count = 1;
            continue;
        } else if(token.type == "OUTPUT"){
            outputFlag = true;
            memopFlag = false;
            loadIFlag = false;
            arithopFlag = false;
            nopFlag = false;

            currentOpCode = token.value;
            lineNum = token.line;

            opc1 = -42;
            opc2 = -42;
            opc3 = -42;

            count = 1;
            continue;
        } else if(token.type == "NOP"){
            nopFlag = true;
            memopFlag = false;
            loadIFlag = false;
            arithopFlag = false;
            outputFlag = false;

            currentOpCode = token.value;
            lineNum = token.line;

            opc1 = -42;
            opc2 = -42;
            opc3 = -42;

            count = 1;
            continue;
        }

        if(errorFlag){
            cerr << "ERROR" << line << ": Invalid token attempted to create with improper starting character on line: " << line << " with " << currentToken << endl;
            currentToken.clear();
            continue;
        }

        // cout << "current count: " << count << endl;
        // cout << "current token: " << token.type << " with value " << token.value << endl;

        if(memopFlag == true){
            if(count == 1 || count == 3){
                if(token.type == "REGISTER"){
                    if (count == 1){
                        opc1 = stoi(token.value.substr(1));
                        count = 2;
                    } else {
                        opc3 = stoi(token.value.substr(1));
                        count = 4;
                    }
                } else {
                    cerr << "ERROR" << line << ": Expected a REGISTER token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 2){
                if(token.type == "INTO"){
                    count = 3;
                } else {
                    cerr << "ERROR" << line << ": Expected an INTO token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 4){
                if(token.type == "EOL"){
                    IR* newIR = new IR(lineNum, currentOpCode, opc1, opc2, opc3);
                    addIR(head, tail, newIR);

                    opc1 = -42;
                    opc2 = -42;
                    opc3 = -42;

                    currentOpCode = "";

                    count = 0;
                    memopFlag = false;
                    loadIFlag = false;
                    arithopFlag = false;
                    outputFlag = false;
                    nopFlag = false;
                } else {
                    cerr << "ERROR" << line << ": Expected an EOL token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            }
        } else if (loadIFlag == true){
            if (count == 1){
                //cout << "TOKEN TYPE: " << token.type << endl;
                if (token.type == "CONSTANT"){
                    opc1 = stoi(token.value);
                    count = 2;
                } else {
                    cerr << "ERROR" << line << ": Expected a CONSTANT token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 2){
                if (token.type == "INTO"){
                    count = 3;
                } else {
                    cerr << "ERROR" << line << ": Expected an INTO token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 3){
                if (token.type == "REGISTER"){
                    opc3 = stoi(token.value.substr(1));
                    count = 4;
                } else {
                    cerr << "ERROR" << line << ": Expected a REGISTER token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 4){
                if(token.type == "EOL"){
                    IR* newIR = new IR(lineNum, currentOpCode, opc1, opc2, opc3);
                    addIR(head, tail, newIR);

                    opc1 = -42;
                    opc2 = -42;
                    opc3 = -42;

                    currentOpCode = "";

                    count = 0;
                    loadIFlag = false;
                    memopFlag = false;
                    arithopFlag = false;
                    outputFlag = false;
                    nopFlag = false;
                } else {
                    cerr << "ERROR" << line << ": Expected an EOL token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            }
        } else if (arithopFlag == true){
            if(count == 1 || count == 3 || count == 5){
                if(token.type == "REGISTER"){
                    if(count == 1){
                        opc1 = stoi(token.value.substr(1));
                        count = 2;
                    } else if (count == 3){
                        opc2 = stoi(token.value.substr(1));
                        count = 4;
                    } else {
                        opc3 = stoi(token.value.substr(1)); 
                        count = 6;
                    }
                } else {
                    cerr << "ERROR" << line << ": Expected a REGISTER token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 2){
                if(token.type == "COMMA"){
                    count = 3;
                } else {
                    cerr << "ERROR" << line << ": Expected a COMMA token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 4){
                if (token.type == "INTO"){
                    count = 5;
                } else {
                    cerr << "ERROR" << line << ": Expected an INTO token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 6){
                if(token.type == "EOL"){
                    IR* newIR = new IR(lineNum, currentOpCode, opc1, opc2, opc3);
                    addIR(head, tail, newIR);

                    opc1 = -42;
                    opc2 = -42;
                    opc3 = -42;

                    currentOpCode = "";

                    count = 0;
                    arithopFlag = false;
                    memopFlag = false;
                    loadIFlag = false;
                    outputFlag = false;
                    nopFlag = false;
                } else {
                    cerr << "ERROR" << line << ": Expected an EOL token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            }
        } else if (outputFlag == true){
            if (count == 1){
                if (token.type == "CONSTANT"){
                    opc1 = stoi(token.value);
                    count = 2;
                } else {
                    cerr << "ERROR" << line << ": Expected a CONSTANT token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            } else if (count == 2){
                if(token.type == "EOL"){
                    IR* newIR = new IR(lineNum, currentOpCode, opc1, opc2, opc3);
                    addIR(head, tail, newIR);

                    opc1 = -42;
                    opc2 = -42;
                    opc3 = -42;

                    currentOpCode = "";

                    count = 0;
                    outputFlag = false;
                    memopFlag = false;
                    loadIFlag = false;
                    arithopFlag = false;
                    nopFlag = false;
                } else {
                    cerr << "ERROR" << line << ": Expected an EOL token but recieved a " << token.type << " token type on line " << line << endl;
                    count+=1;
                    grammarErrorFlag = true;
                }
            }
        } else if (nopFlag == true){
            if(token.type == "EOL"){
                IR* newIR = new IR(lineNum, currentOpCode, opc1, opc2, opc3);
                addIR(head, tail, newIR);

                opc1 = -42;
                opc2 = -42;
                opc3 = -42;

                currentOpCode = "";

                count = 0;
                nopFlag = false;
            } else {
                cerr << "ERROR" << line << ": Expected an EOL token but recieved a " << token.type << " token type on line " << line << endl;
                count+=1;
                grammarErrorFlag = true;
            }
        } else if (!memopFlag && !loadIFlag && !arithopFlag && !outputFlag && !nopFlag){
            if(token.type == "EOL"){
                continue;
            }
        } else if (grammarErrorFlag == false){
            cerr << "ERROR" << line << ": Improper syntax" << endl;
            grammarErrorFlag = true;
        }
        
        if(grammarErrorFlag){
            currentToken.clear();
            continue;
        }

    }

    if(whatFlag == "-p"){
        if(grammarErrorFlag){
            cout << "Bad parsing. Errors detected. >:|" << endl;
        } else {
            cout << "Good parsing! No errors! :)" << endl;
        }
    }

    if(whatFlag == "-r" && !grammarErrorFlag){
        IR* current = head;

        while(current){
            cout << current->lineNum << ": " << current->opcode << " " << current->opc1 << ", " << current->opc2 << ", " << current->opc3 << endl;
            current = current->next;
        }
    }

    // auto end = chrono::high_resolution_clock::now();
    // chrono::duration<double> elapsed = end - start;

    // cout << "Scanning completed in " << elapsed.count() << " seconds." << endl;

    return 0;
}