#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cctype>
#include <unordered_map>
#include <sstream>

class Scanner {
private:
    std::istream* input;
    std::ifstream inputFile;
    char currentChar;
    bool ownStream;
    
    // Keywords map
    std::unordered_map<std::string, std::string> keywords = {
        {"int", "INT"},
        {"float", "FLOAT"},
        {"if", "IF"},
        {"else", "ELSE"},
        {"while", "WHILE"},
        {"return", "RETURN"},
        {"input", "INPUT"},
        {"print", "PRINT"},
        {"void", "VOID"}
    };
    
    // Read next character from input
    void readChar() {
        input->get(currentChar);
    }
    
    // Check if we're at the end of the input
    bool isEOF() {
        return input->eof();
    }
    
    // Skip whitespace characters
    void skipWhitespace() {
        while (!isEOF() && isspace(currentChar)) {
            readChar();
        }
    }
    
    // Check if character is valid for identifier (a-z, A-Z, 0-9)
    bool isIdChar(char c) {
        return isalnum(c);
    }
    
    // Check if character is digit (0-9)
    bool isDigit(char c) {
        return isdigit(c);
    }
    
    // Process identifier or keyword
    std::pair<std::string, std::string> scanIdentifier() {
        std::string value;
        
        // Collect the identifier characters
        while (!isEOF() && isIdChar(currentChar)) {
            value += currentChar;
            readChar();
        }
        
        // Check if it's a keyword
        if (keywords.find(value) != keywords.end()) {
            return {keywords[value], value};
        }
        
        // It's an identifier
        return {"ID", value};
    }
    
    // Process number (integer or float)
    std::pair<std::string, std::string> scanNumber() {
        std::string value;
        bool isFloat = false;
        
        // Collect digits
        while (!isEOF() && isDigit(currentChar)) {
            value += currentChar;
            readChar();
        }
        
        // Check for decimal point
        if (currentChar == '.') {
            isFloat = true;
            value += currentChar;
            readChar();
            
            // Collect digits after decimal point
            while (!isEOF() && isDigit(currentChar)) {
                value += currentChar;
                readChar();
            }
        }
        
        if (isFloat) {
            return {"FLO", value};
        } else {
            return {"NUM", value};
        }
    }

public:
    // Constructor for file input
    Scanner(const std::string& filename) : ownStream(true) {
        inputFile.open(filename);
        if (!inputFile.is_open()) {
            throw std::runtime_error("Could not open input file: " + filename);
        }
        input = &inputFile;
        readChar(); // Read the first character
    }
    
    // Constructor for stdin
    Scanner() : ownStream(false) {
        input = &std::cin;
        readChar(); // Read the first character
    }
    
    ~Scanner() {
        if (ownStream && inputFile.is_open()) {
            inputFile.close();
        }
    }
    
    // Get the next token from input
    std::pair<std::string, std::string> getNextToken() {
        skipWhitespace();
        
        if (isEOF()) {
            return {"EOF", ""};
        }
        
        // Identifier
        if (isalpha(currentChar)) {
            return scanIdentifier();
        }
        
        // Number
        if (isDigit(currentChar)) {
            return scanNumber();
        }
        
        // Handle special characters and operators
        char c = currentChar;
        readChar();
        
        switch (c) {
            case '+': return {"ADD", "+"};
            case '*': return {"MUL", "*"};
            case '-': return {"SUB", "-"};
            case '/': return {"DIV", "/"};
            case '%': return {"MOD", "%"};
            case '<': 
                if (currentChar == '=') {
                    readChar();
                    return {"LE", "<="};
                }
                return {"LT", "<"};
            case '=':
                if (currentChar == '=') {
                    readChar();
                    return {"EQ", "=="};
                }
                return {"ASG", "="};
            case '!':
                if (currentChar == '=') {
                    readChar();
                    return {"NE", "!="};
                }
                return {"NOT", "!"};
            case '>':
                if (currentChar == '=') {
                    readChar();
                    return {"GE", ">="};
                }
                return {"GT", ">"};
            case '&':
                if (currentChar == '&') {
                    readChar();
                    return {"ANDAND", "&&"};
                }
                return {"AND", "&"};
            case '|':
                if (currentChar == '|') {
                    readChar();
                    return {"OROR", "||"};
                }
                return {"OR", "|"};
            case '(': return {"LPA", "("};
            case ')': return {"RPA", ")"};
            case '[': return {"LBK", "["};
            case ']': return {"RBK", "]"};
            case '{': return {"LBR", "{"};
            case '}': return {"RBR", "}"};
            case ',': return {"CMA", ","};
            case ';': return {"SCO", ";"};
            default:
                return {"ERROR", std::string(1, c)};
        }
        
        return {"ERROR", std::string(1, c)};
    }
    
    // Scan the entire input and return all tokens
    std::vector<std::pair<std::string, std::string>> scanAllTokens() {
        std::vector<std::pair<std::string, std::string>> tokens;
        std::pair<std::string, std::string> token;
        
        do {
            token = getNextToken();
            if (token.first != "EOF") {
                tokens.push_back(token);
            }
        } while (token.first != "EOF");
        
        return tokens;
    }
};

int main(int argc, char* argv[]) {
    try {
        Scanner* scanner;
        
        if (argc >= 2) {
            scanner = new Scanner(argv[1]);
        } else {
            std::cout << "Enter input :" << std::endl;
            scanner = new Scanner();
        }
        
        std::vector<std::pair<std::string, std::string>> tokens = scanner->scanAllTokens();
        
        bool result = true;
        for (const auto& token : tokens) {
            if (token.first == "ERROR") {
                result = false;
                std::cout << "Parse Error near: " << token.second << std::endl;
            }
        }

        if (result)
            for (const auto& token : tokens) {
                std::cout << "(" << token.first << ", " << token.second << ")" << std::endl;
            }
        
        delete scanner;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}