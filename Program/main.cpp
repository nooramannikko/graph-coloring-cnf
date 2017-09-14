#include <iostream>
#include <stdlib.h>

#include <vector>
#include <string>
#include <set>
#include <fstream>
#include <sstream>

//using namespace std;
using std::string;
using std::vector;
using std::set;
using std::cin;
using std::cout;
using std::endl;
using std::stringstream;
using std::ifstream;
using std::ofstream;

struct GraphConnections {
    GraphConnections(): connectionMatrix() {}
    vector<vector<bool>> connectionMatrix;
};

struct Literal {
    Literal(): node(0), color(0), bvalue(true), degree(0){}
    Literal(int n, int c): node(n), color(c), bvalue(true), degree(0) {}
    Literal(int n, int c, bool b): node(n), color(c), bvalue(b), degree(0) {}
    int node;
    int color;
    bool bvalue;
    int degree; // number of edges
};

struct Clause {
    Clause(): literals() {}
    vector<Literal*> literals;
};

GraphConnections * readGraphFromFile(string file, int& nodes, int& edges);
void createPhaseOneClauses(int nodes, vector<Clause*>& rawFormatFormula, int colors);
void createSecondPhaseClauses(int nodes, vector<Clause*>& rawFormatFormula, int colors);
void createThirdPhaseClauses(GraphConnections* graph, vector<Clause*>& rawFormatFormula, int colors);
void parseFormula(vector<Clause*>& rawFormatFormula, vector<string>& parsedFormula, int nodes);
void clearMemory(vector<Clause*>& rawFormatFormula);
void writeToFile(string file, vector<string>& formula, int colors);

int main() {
    cout << "To get formula, please give filepath." << endl;
    std::string file;
    cin >> file;

    // Create Graph
    int nodes = 0;
    int edges = 0;
    GraphConnections* graph = readGraphFromFile(file, nodes, edges);
    cout << "Debug: graph size " << graph->connectionMatrix.size() << " x " << graph->connectionMatrix.at(0).size() << endl;

    // Raw format and parsed formulas
    vector<Clause*> rawFormatFormula;
    vector<string> parsedFormula;

    cout << "Give number of colors" << endl;
    char colors[256];
    cin >> colors;
    int k = atoi(colors);
    //cout << "Debug: given number of colors " << k << ", minColors " << minColors << endl;
    if (k <= 0){
        // If k < minColors of Graph, return false
        cout << "Cannot form formula for " << k << " colors. k must be at least 1." << endl;
    }
    else {
        // Count formula with given color number k
        // Phases are something like http://ceur-ws.org/Vol-533/09_LANMR09_06.pdf
        createPhaseOneClauses(nodes, rawFormatFormula, k);
        createSecondPhaseClauses(nodes, rawFormatFormula, k);
        createThirdPhaseClauses(graph, rawFormatFormula, k);

        // Parse formula
        parsedFormula.push_back("p cnf variables clauses");
        parseFormula(rawFormatFormula, parsedFormula, nodes);

        // Get output file and write
        cout << "Give output file for result" << endl;
        string outputFile;
        cin >> outputFile;
        writeToFile(outputFile, parsedFormula, k);
    }

    // Delete created objects
    clearMemory(rawFormatFormula);

    return 0;
}

GraphConnections *readGraphFromFile(string file, int& nodes, int& edges) {
    GraphConnections* result = new GraphConnections();

    ifstream filestream(file);
    string line;
    if (filestream.is_open()){
        // Read file line by line
        //cout << "Debug: Reading file" << endl;
        while (getline(filestream, line)) {
            // Parse line
            stringstream sstream(line);
            /* Not prepared for cases where:
             * lines are of improper syntax;
             * number of given nodes and/or edges does not match the given problem
             */
            //cout << "Debug: read line " << line << endl;
            if (line.size() > 0) {
                string item;
                vector<string> parts;
                while(getline(sstream, item, ' ')){
                    parts.push_back(item);
                }
                if (parts.at(0) == "p"){
                    // Problem line
                    nodes = atoi(parts.at(2).c_str());
                    edges = atoi(parts.at(3).c_str());
                    result->connectionMatrix.resize(nodes, vector<bool>(nodes,false));
                }
                else if (parts.at(0) == "e") {
                    // Edge line
                    int node1 = atoi(parts.at(1).c_str());
                    int node2 = atoi(parts.at(2).c_str());
                    result->connectionMatrix.at(node1-1).at(node2-1) = true;
                    result->connectionMatrix.at(node2-1).at(node1-1) = true; // Mirrored connection
                }
            }
        }
        filestream.close();
        /*cout << "Debug: created graph with dimensions " << result->connectionMatrix.size() << " x "
            << result->connectionMatrix.at(0).size() << endl;*/
    }
    return result;
}

void createPhaseOneClauses(int nodes, vector<Clause*>& rawFormatFormula, int colors) {
    // Go through each node
    for (int i = 0; i < nodes; ++i){
        // Apply each color
        Clause* clause = new Clause();
        for (int k = 1; k <= colors; ++k){
            Literal* lit = new Literal(i+1,k);
            clause->literals.push_back(lit);
        }
        rawFormatFormula.push_back(clause);
    }
}

void createSecondPhaseClauses(int nodes, vector<Clause*>& rawFormatFormula, int colors) {
    // Go through each node
    for (int i = 0; i < nodes; ++i){
        // Go through each color
        for (int k = 1; k <= colors; ++k){
            Literal* litCurrentColor = new Literal(i+1,k,false);
            // Go through other colors
            for (int kk = 1; kk <= colors; ++kk) {
                if (kk != k) {
                    Literal *litOtherColor = new Literal(i + 1, kk, false);
                    Clause *clause = new Clause();
                    clause->literals.push_back(litCurrentColor);
                    clause->literals.push_back(litOtherColor);
                    rawFormatFormula.push_back(clause);
                    //cout << "Debug: added clause x_" << i + 1 << " k_" << k << " false OR x_" << i + 1 << " k_" << kk;
                    //cout << " false" << endl;
                }
            }
        }
    }
}

void createThirdPhaseClauses(GraphConnections* graph, vector<Clause*>& rawFormatFormula, int colors){
    // Go through each node
    for (int i = 0; i < graph->connectionMatrix.size(); ++i){
        // Go through each connection and color
        for (int j = 0; j < graph->connectionMatrix.at(i).size(); ++j){
            if (i != j && graph->connectionMatrix.at(i).at(j)){
                for (int k = 1; k <= colors; ++k){
                    Clause* clause = new Clause();
                    Literal* litNode = new Literal(i+1,k, false);
                    Literal* litEdge = new Literal(j+1,k, false);
                    clause->literals.push_back(litNode);
                    clause->literals.push_back(litEdge);
                    rawFormatFormula.push_back(clause);
                    //cout << "Debug: added clause x_" << i + 1 << " k_" << k << " false OR x_" << j + 1 << " k_" << k;
                    //cout << " false" << endl;
                    //cout << "Debug: litNodes " << litNode->node << " " << litEdge->node << endl;
                }
            }
        }
    }
}

void parseFormula(vector<Clause*>& rawFormatFormula, vector<string>& parsedFormula, int nodes){
    // Transform each clause into string
    // Variable number = label + (color-1)*nodes
    // With negated variables number*-1
    set<int> usedVariables;
    for (int i = 0; i < rawFormatFormula.size(); ++i){
        string clause;
        for (int j = 0; j < rawFormatFormula.at(i)->literals.size(); ++j){
            Literal* lit = rawFormatFormula.at(i)->literals.at(j);
            int varNumber = lit->node + (lit->color - 1) * nodes;
            if (!lit->bvalue) { varNumber *= -1; }
            usedVariables.insert(varNumber);
            stringstream ss;
            ss << varNumber;
            string varLabel = ss.str();
            clause += varLabel + " ";
        }
        clause += "0";
        parsedFormula.push_back(clause);
    }
    stringstream ss1;
    ss1 << usedVariables.size();
    string numOfVariables = ss1.str();
    cout << "Debug: number of variables: " << numOfVariables << " vs. " << usedVariables.size() << endl;
    stringstream ss2;
    ss2 << parsedFormula.size() - 1;
    string numOfClauses = ss2.str();
    cout << "Debug: number of clauses: " << numOfClauses << " vs. " << parsedFormula.size() - 1 << endl;
    parsedFormula.at(0) = "p cnf " + numOfVariables + " " + numOfClauses;
}

void clearMemory(vector<Clause*>& rawFormatFormula){
    for (int i = 0; i < rawFormatFormula.size(); ++i){
        for (int j = 0; j < rawFormatFormula.at(i)->literals.size(); j++){
            delete rawFormatFormula.at(i)->literals.at(j);
        }
        rawFormatFormula.at(i)->literals.clear();
        delete rawFormatFormula.at(i);
    }
    rawFormatFormula.clear();
}

void writeToFile(string file, vector<string>& formula, int colors){
    ofstream fstream(file);
    if (fstream.is_open()){
        fstream << "c CNF formula with " << colors << " colors to follow." << '\n';
        for (int i = 0; i < formula.size(); ++i){
            fstream << formula.at(i) << '\n';
        }
        fstream.close();
    }
}