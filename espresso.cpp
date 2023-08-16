#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <map>
#include <bitset>
#include <algorithm>
#include <string.h>
#include <utility>
using namespace std;

struct term {
    string input;
    string output;
    vector<bitset<2>> positional_representation;
};
vector <term> minTerms, dontcareTerms, maxTerms, onsetTerms, expandedTerms, irredundantTerms, redundantTerms;
map <char, bitset<2>> positional_representation_map = {
    {'1',1},
    {'0',2},
    {'X',3}
};
unsigned int iteration = 1;
unsigned int num_input_bits = 0;

vector<bitset<2>> positionallyRepresent(string input) {
    vector<bitset<2>> positional_representation;
    for (auto &literal : input) {
        if (positional_representation_map.find(literal) == positional_representation_map.end()) {
            cout << "Invalid value of literal. Only 0/1/X allowed" << endl;
            cout << "Exiting..." << endl;
            exit(0);
        } else {
            positional_representation.push_back(positional_representation_map[literal]);
        }
    }
    return positional_representation;
}

string reverse_positional_representation(vector<bitset<2>> input) {
    string reverse_positional_representation;
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i] == positional_representation_map['0']) {
            reverse_positional_representation += '0';
        }
        else if (input[i] == positional_representation_map['1']) {
            reverse_positional_representation += '1';
        }
        else if (input[i] == positional_representation_map['X']) {
            reverse_positional_representation += 'X';
        }
        else {
            cout << "Illegal value present, please check code" << endl;
        }
    }
    return reverse_positional_representation;
}

void extractTermsandRepresent(const string &s) {
    stringstream ss(s);
    string token;
    term row;
    while (getline(ss, token)) {
        size_t pos = token.find(' ');
        row.input = token.substr(0, pos);
        //row.output = token.substr(pos + 1);
        row.output = token[pos + 1];
        row.positional_representation = positionallyRepresent(row.input);
        if (row.output.compare("1") == 0) { // ONSET term
            minTerms.push_back(row);
        }
        else if (row.output.compare("-") == 0) { // DC term
            dontcareTerms.push_back(row);
        }
        else if (row.output.compare("0") == 0) { // OFFSET term
            maxTerms.push_back(row);
        }
        else {
            cout << "Invalid value of output. Only 0/1/- allowed" << endl;
            cout << "Exiting..." << endl;
            exit(0);
        }
    }
}

void preprocess(string truth_table_file) {
    ifstream file(truth_table_file, std::ios::in);
    string line;
    unsigned int countRow = 0;
    if (file) {
        while(getline(file, line)) {
            if(countRow == 0 ){ // First row indicating number of bits in input
                num_input_bits = atoi(line.c_str());
                countRow++;
            }
            else {
                extractTermsandRepresent(line);
                countRow++;
            }
        }
        if (countRow != pow(2,num_input_bits)+1) { //+1 due to num_input header line
            cout << "Please specify all input-output combinations with header as number of input bits" << endl;
            cout << "Exiting..." << endl;
            exit(0);
        }
        // Add minTerms and dontcareTerms to ONSET
        onsetTerms = minTerms;
        onsetTerms.insert(onsetTerms.end(), dontcareTerms.begin(), dontcareTerms.end());
    }
    else {
        cout << "Truth table file with name " << truth_table_file << " does not exist" << endl;
        cout << "Exiting..." << endl;
        exit(0);
    }
}

void expand(unsigned int bit_position) {
    vector <bitset<2>> offsetIntersectionTerm;

    cout << endl << "Iteration " << iteration << " of expansion :" << endl;
    for( auto onsetiter = onsetTerms.begin(), expandediter = expandedTerms.begin(); onsetiter != onsetTerms.end() && expandediter != expandedTerms.end() ; onsetiter++, expandediter++ ) {
        // Change literal starting from MSB bit by bit to don't care
        expandediter->input[expandediter->input.size()-bit_position] = 'X';
        expandediter->positional_representation[expandediter->positional_representation.size()-bit_position] = positional_representation_map['X'];

        // Check intersection of expanded terms with each OFFSET term
        for( auto maxiter = maxTerms.begin(); maxiter != maxTerms.end() ; maxiter++ ) {
            bool offset_intersection_flag = true; // Assuming intersection with OFFSET to be true at start
            cout << "Intersection of Term " << expandediter->input << " with OFFSET term "<< maxiter->input << endl;
            // Intersection with OFFSET term
            for( size_t i=0; i!=expandediter->positional_representation.size(); i++ ) {
                offsetIntersectionTerm.push_back(expandediter->positional_representation[i] & maxiter->positional_representation[i]);
            }
            for( auto iter = offsetIntersectionTerm.begin(); iter != offsetIntersectionTerm.end() ; iter++ ) {
                cout << *iter;
            }
            cout << endl;
            for( auto iter = offsetIntersectionTerm.begin(); iter != offsetIntersectionTerm.end() ; iter++ ) {
                if (*iter == 0) { // No intersection with OFFSET term
                    offset_intersection_flag = false;
                    break;
                }
            }
            // Intersection with any one OFFSET term true hence cannot be expanded further
            if (offset_intersection_flag) {
                for( auto it = onsetiter->positional_representation.begin(); it != onsetiter->positional_representation.end() ; it++ ) {
                    cout << *it;
                }
                cout << " (" << onsetiter->input << ") cannot be expanded to ";
                for( auto it = expandediter->positional_representation.begin(); it != expandediter->positional_representation.end() ; it++ ) {
                    cout << *it;
                }
                cout << " (" << expandediter->input << ") due to intersection with OFFSET term " << maxiter->input << endl;
                // Revert the expansion by replacing the expanded literal with corresponding literal in minterm
                expandediter->input[expandediter->input.size()-bit_position] = onsetiter->input[onsetiter->input.size()-bit_position];
                expandediter->positional_representation[expandediter->positional_representation.size()-bit_position] = onsetiter->positional_representation[onsetiter->positional_representation.size()-bit_position];
                offsetIntersectionTerm.clear();
                break;
            }
            offsetIntersectionTerm.clear();  
        }
    }
    iteration++;
}

void removeDuplicateExpandedTerms() {
    cout << endl << "---Removing repeated expandedTerms---" << endl;
    cout << endl << "Remaining expandedTerms :" << endl;
    for(size_t i=0;i<expandedTerms.size();i++) {
        for(size_t j=0;j<expandedTerms.size();j++) {
            if(expandedTerms[i].input == expandedTerms[j].input && i != j) {
                expandedTerms.erase(expandedTerms.begin()+j);
                j--;
            }
        }   
    }
    for( auto iter = expandedTerms.begin(); iter != expandedTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
}

void checkRedundancy() {
    vector <bitset<2>> irredundancycheckANDTerm, irredundancycheckORTerm, tautologyORTerm;
    // Check for containment of each term in another
    for( auto expandediteri = expandedTerms.begin(); expandediteri != expandedTerms.end(); expandediteri++) {
        bool tautology_check_flag = false;
        bool tautology_variable_flag = false;
        for( auto expandediterj = expandedTerms.begin(); expandediterj != expandedTerms.end(); expandediterj++) {
            bool void_flag = false;
            bool tautology = false;
            if (expandediteri->input.compare(expandediterj->input) == 0) { // Skip redundancy check for terms with same implicants
                continue;
            }
            else {
                cout << expandediteri->input << " ";
                for( auto it = expandediteri->positional_representation.begin(); it != expandediteri->positional_representation.end() ; it++ ) {
                    cout << *it;
                }
                cout << " Bitwise AND ";
                cout << expandediterj->input << " ";
                for( auto it = expandediterj->positional_representation.begin(); it != expandediterj->positional_representation.end() ; it++ ) {
                    cout << *it;
                }
                cout << " : ";
                // AND Operation
                for( size_t i=0; i!=expandediterj->positional_representation.size(); i++ ) {
                    irredundancycheckANDTerm.push_back(expandediteri->positional_representation[i] & expandediterj->positional_representation[i]);
                }
                for( auto iter = irredundancycheckANDTerm.begin(); iter != irredundancycheckANDTerm.end() ; iter++ ) {
                    cout << *iter;
                }
                cout << endl;
                for( auto iter = irredundancycheckANDTerm.begin(); iter != irredundancycheckANDTerm.end() ; iter++ ) {
                    if (*iter == 0) { // Void term present after AND operation
                        void_flag = true;
                        cout << "Void! hence skipping Bitwise OR operation" << endl;
                        break;
                    }
                }
                if (!void_flag) { // OR operation with complement only if AND term doesn't contain any invalid(00) operation
                    for( size_t i=0; i!=expandediteri->positional_representation.size(); i++ ) {
                        cout << ~expandediteri->positional_representation[i];
                    }
                    cout << " Bitwise OR ";
                    for( auto iter = irredundancycheckANDTerm.begin(); iter != irredundancycheckANDTerm.end() ; iter++ ) {
                        cout << *iter;
                    }
                    cout << " : ";
                    // OR Operation
                    for( size_t i=0; i!=irredundancycheckANDTerm.size(); i++ ) {
                        irredundancycheckORTerm.push_back(irredundancycheckANDTerm[i] | ~expandediteri->positional_representation[i]);
                    }
                    for( auto iter = irredundancycheckORTerm.begin(); iter != irredundancycheckORTerm.end() ; iter++ ) {
                        cout << *iter;
                    }
                    cout << endl;
                    // Tautology check
                    for (auto iteri = irredundancycheckORTerm.begin(); iteri != irredundancycheckORTerm.end(); iteri++) {
                        // Check if a term contains all 1's
                        if (*iteri == positional_representation_map['X'] && adjacent_find(irredundancycheckORTerm.begin(), irredundancycheckORTerm.end(), std::not_equal_to<>()) == irredundancycheckORTerm.end()) {
                            tautology = true;
                            break;
                        }
                        else {
                            cout << "Not a Tautology (no row of 1's)" << endl;
                            tautology = false;
                            // Check if tautology is obtained by considering single variable by ORing all bitwise sum terms
                            if (!tautology_check_flag) {
                                tautologyORTerm = irredundancycheckORTerm;
                                tautology_check_flag = true;
                            }
                            else {
                                for( size_t i=0; i!=irredundancycheckORTerm.size(); i++ ) {
                                    tautologyORTerm[i] = tautologyORTerm[i] | irredundancycheckORTerm[i];
                                }
                            }
                            break;
                        }
                    }
                    if (tautology){
                        cout << "Tautology (row of 1's)" << endl; // All 1's
                        redundantTerms.push_back(*expandediteri);
                        expandedTerms.erase(expandediterj--); // remove redundant term
                        irredundancycheckANDTerm.clear();
                        irredundancycheckORTerm.clear();
                        break;
                    }
                }
            }
            irredundancycheckANDTerm.clear();
            irredundancycheckORTerm.clear();
        }
        cout << "ORing all bitwise sum terms : ";
        for( auto iter = tautologyORTerm.begin(); iter != tautologyORTerm.end() ; iter++ ) {
            cout << *iter;
            // Check if a term contains all 1's
            if (*iter == positional_representation_map['X'] && adjacent_find(tautologyORTerm.begin(), tautologyORTerm.end(), std::not_equal_to<>()) == tautologyORTerm.end()) {
                tautology_variable_flag=true;
                //break;
            }
            else {
                tautology_variable_flag=false;
            }
        }
        cout << endl;
        if (tautology_variable_flag){
            cout << "Tautology due to single/multiple variable(s)" << endl;
            redundantTerms.push_back(*expandediteri);
            expandedTerms.erase(expandediteri--); // remove redundant term
        }
        else{
            irredundantTerms.push_back(*expandediteri);
        }
        tautologyORTerm.clear();
        cout << endl;
    }
    cout << "Irredundant terms : " << endl;
    for( auto iter = irredundantTerms.begin(); iter != irredundantTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
    cout << "Redundant terms : " << endl;
    for( auto iter = redundantTerms.begin(); iter != redundantTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
    redundantTerms.clear();
}

vector <vector<bitset<2>>> generatePCRofTermstoComplement(vector<term> TermstoComplement) {
    //PCR --- Positional Cube Representation
    vector<bitset<2>> complementedTerm;
    vector <vector<bitset<2>>> generatedPCRlist;
    for( auto iter = irredundantTerms.begin(); iter != irredundantTerms.end() ; iter++ ) {
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            complementedTerm.push_back(*it);
        }
        generatedPCRlist.push_back(complementedTerm);
        complementedTerm.clear();
    }
    return generatedPCRlist;
}

// Unate Recursive Paradigm Code starts
vector <vector<bitset<2>>> DeMorganLaws(vector<bitset<2>> singleLiteral) {
    vector<bitset<2>> DeMorganResult;
    vector <vector<bitset<2>>> DeMorganResultList;

    cout << "Complement of ";
    for( auto iter = singleLiteral.begin(); iter != singleLiteral.end() ; iter++ ) {
        cout << *iter;
    }
    cout << " : ";
    for( size_t index=0; index!=singleLiteral.size(); index++ ) {
        if (singleLiteral[index] == positional_representation_map['X']) {
            continue;
        }
        else if (singleLiteral[index] == positional_representation_map['0']) {
            for (size_t literal_count = 0; literal_count < num_input_bits; literal_count++) {
                if (literal_count == index) {
                    DeMorganResult.push_back(positional_representation_map['1']);
                }
                else {
                    DeMorganResult.push_back(positional_representation_map['X']);
                }
            }
        }
        else if (singleLiteral[index] == positional_representation_map['1']) {
            for (size_t literal_count = 0; literal_count < num_input_bits; literal_count++) {
                if (literal_count == index) {
                    DeMorganResult.push_back(positional_representation_map['0']);
                }
                else {
                    DeMorganResult.push_back(positional_representation_map['X']);
                }
            }
        }
        else {
            cout << "Error, 00 should not be present, please check code again";
            exit(0);
        }
        DeMorganResultList.push_back(DeMorganResult);
        DeMorganResult.clear();
    }
    for (auto iter = DeMorganResultList.begin(); iter != DeMorganResultList.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << (*it);
        }
        cout << " ";
    }
    cout << endl;
    return DeMorganResultList;
}

int selectMostBinateLiteral(vector <vector<bitset<2>>> Terms) {
    vector <pair<int,int>> true_form; // Pair of Index of literal and n.o of occurrences in true form
    vector <pair<int,int>> complemented_form; // Pair of Index of literal and n.o of occurrences in complemented form
    vector <pair<int,int>> unate_list, binate_list;

    //cout << "True and complemented form initialization : " << endl;
    for (int literal_count = 0; literal_count < num_input_bits; literal_count++) {
        true_form.push_back(make_pair(literal_count, 0));
        complemented_form.push_back(make_pair(literal_count, 0));
    }
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        size_t literal_count = 0;
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            // literal present in true form
            if (*it == positional_representation_map['1']) {
                true_form[literal_count].second += 1;
            }
            // literal present in complemented form
            else if (*it == positional_representation_map['0']) {
                complemented_form[literal_count].second += 1;
            }
            else {
                ; // do nothing
            }
            literal_count++;
        }
    }
    for (size_t literal_count = 0; literal_count < num_input_bits; literal_count++) {
        cout << "True form (variable_index : occurrence) " << true_form[literal_count].first << " : " << true_form[literal_count].second << endl;
        cout << "Complemented form (variable_index : occurrence) " << complemented_form[literal_count].first << " : " << complemented_form[literal_count].second << endl;
    }
    for (size_t literal_counti = 0; literal_counti < num_input_bits; literal_counti++) {
        // Push in binate list if present in both true and complemented form
        if (true_form[literal_counti].second && complemented_form[literal_counti].second) {
            binate_list.push_back(make_pair(literal_counti,true_form[literal_counti].second+complemented_form[literal_counti].second));
        }
        // Push in unate list if present in either true / complemented form
        else if (true_form[literal_counti].second) {
            unate_list.push_back(make_pair(literal_counti,true_form[literal_counti].second));
        }
        else if (complemented_form[literal_counti].second) {
            unate_list.push_back(make_pair(literal_counti,complemented_form[literal_counti].second));
        }
        else {
            ;
        }
    }
    cout << endl << "Binate list : ";
    for (size_t i = 0; i != binate_list.size(); i++) {
        cout << binate_list[i].first << " " << binate_list[i].second << "  ";
    }
    cout << endl;
    cout << "Unate list : ";
    for (size_t i = 0; i != unate_list.size(); i++) {
        cout << unate_list[i].first << " " << unate_list[i].second << "  ";
    }
    cout << endl;
    if (!binate_list.empty()) {
        cout << "Most binate literal index : ";
        cout << max_element(binate_list.begin(), 
            binate_list.end(), 
            [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; })->first;
        cout << endl << "Most binate literal occurence value : ";
        cout << max_element(binate_list.begin(), 
            binate_list.end(), 
            [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; })->second;
        cout << endl;
        return max_element(binate_list.begin(), 
            binate_list.end(), 
            [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; })->first;
    }
    //if (!unate_list.empty()) {
    else {
        cout << "Most unate literal index : ";
        cout << max_element(unate_list.begin(), 
            unate_list.end(), 
            [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; })->first;
        cout << endl << "Most unate literal occurence value : ";
        cout << max_element(unate_list.begin(), 
            unate_list.end(), 
            [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; })->second;
        cout << endl;
        return max_element(unate_list.begin(), 
            unate_list.end(), 
            [](const auto& lhs, const auto& rhs) { return lhs.second < rhs.second; })->first;
    }
}

vector <vector<bitset<2>>> positiveCoFactor(vector <vector<bitset<2>>> Terms, int index) {
    vector<bitset<2>> positiveCofactorTerm;
    vector <vector<bitset<2>>> positiveCofactorTerms;
    bool positiveCoFactorexists;

    cout << endl << "Terms input to positiveCoFactor : " ;
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << *it;
        }
        cout << "---";
    }
    cout << endl;
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        size_t literal_count = 0;
        positiveCoFactorexists = true;
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            if (literal_count == index) {
                if (*it == positional_representation_map['X']) {
                    positiveCofactorTerm.push_back(*it);
                }
                else if (*it == positional_representation_map['0']) {
                    positiveCoFactorexists = false;
                    positiveCofactorTerm.clear();
                    break;
                }
                else if (*it == positional_representation_map['1']) {
                    positiveCofactorTerm.push_back(positional_representation_map['X']);
                }
            } else {
                positiveCofactorTerm.push_back(*it);
            }
            literal_count++;
        }
        if (positiveCoFactorexists){       
            positiveCofactorTerms.push_back(positiveCofactorTerm);
            positiveCofactorTerm.clear();
        }
    }
    cout << "Positive Cofactor terms : " ;
    for (auto iter = positiveCofactorTerms.begin(); iter != positiveCofactorTerms.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << *it;
        }
        cout << "---";
    }
    cout << endl;
    return positiveCofactorTerms;
}

vector <vector<bitset<2>>> negativeCoFactor(vector <vector<bitset<2>>> Terms, int index) {
    vector<bitset<2>> negativeCoFactorTerm;
    vector <vector<bitset<2>>> negativeCoFactorTerms;
    bool negativeCoFactorexists;

    cout << endl << "Terms input to negativeCoFactor : " ;
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << *it;
        }
        cout << "---";
    }
    cout << endl;
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        size_t literal_count = 0;
        negativeCoFactorexists = true;
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            if (literal_count == index) {
                if (*it == positional_representation_map['X']) {
                    negativeCoFactorTerm.push_back(*it);
                }
                else if (*it == positional_representation_map['1']) {
                    negativeCoFactorexists = false;
                    negativeCoFactorTerm.clear();
                    break ;
                }
                else if (*it == positional_representation_map['0']) {
                    negativeCoFactorTerm.push_back(positional_representation_map['X']);
                }
            } else {
                negativeCoFactorTerm.push_back(*it);
            }
            literal_count++;
        }
        if (negativeCoFactorexists){       
            negativeCoFactorTerms.push_back(negativeCoFactorTerm);
            negativeCoFactorTerm.clear();
        }
    }
    cout << "Negative Cofactor terms : " ;
    for (auto iter = negativeCoFactorTerms.begin(); iter != negativeCoFactorTerms.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << *it;
        }
        cout << "---";
    }
    cout << endl;
    return negativeCoFactorTerms;
}

vector <vector<bitset<2>>> x_and(vector <vector<bitset<2>>> Terms, int index) {
    vector<bitset<2>> x_andTerm;
    vector <vector<bitset<2>>> x_andTerms;
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        size_t literal_count = 0;
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            if (literal_count == index) {
                x_andTerm.push_back(positional_representation_map['1']);
            } else {
                x_andTerm.push_back(*it);
            }
            literal_count++;
        }
        x_andTerms.push_back(x_andTerm);
        x_andTerm.clear();
    }
    return x_andTerms;
}

vector <vector<bitset<2>>> x_and_bar(vector <vector<bitset<2>>> Terms, int index) {
    vector<bitset<2>> x_and_barTerm;
    vector <vector<bitset<2>>> x_and_barTerms;
    for (auto iter = Terms.begin(); iter != Terms.end() ; iter++) {
        size_t literal_count = 0;
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            if (literal_count == index) {
                x_and_barTerm.push_back(positional_representation_map['0']);
            } else {
                x_and_barTerm.push_back(*it);
            }
            literal_count++;
        }
        x_and_barTerms.push_back(x_and_barTerm);
        x_and_barTerm.clear();
    }
    return x_and_barTerms;
}

vector <vector<bitset<2>>> complementFunction(vector <vector<bitset<2>>> TermstoComplement) {
    vector<bitset<2>> complementedTerm;
    vector <vector<bitset<2>>> positiveCofactorTermsList, negativeCofactorTermsList, xandbarnegativeCofactorTermsList, complementedTermsList;
    bool alldontcares = false;

    cout << endl << "Terms to Complement : ";
    for (auto iter = TermstoComplement.begin(); iter != TermstoComplement.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << (*it);
        }
        cout << "---";
    }
    cout << endl;
    //Corner case of all zeroes
    if (TermstoComplement.size() == 0) {
        for (size_t i = 0; i < num_input_bits; i++){
            complementedTerm.push_back(positional_representation_map['X']);
        }
        complementedTermsList.push_back(complementedTerm);
        return complementedTermsList;
    }
    // Terms contain all dont-cares
    for (auto iter = TermstoComplement.begin(); iter != TermstoComplement.end() ; iter++) {
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            if (*it == positional_representation_map['X'] && adjacent_find(iter->begin(), iter->end(), std::not_equal_to<>()) == iter->end()) {
                complementedTerm.clear();
                alldontcares = true;
            }
        }
    }

    if (!alldontcares) {
        // Single term present, hence complement directly by using DeMorgan's law
        if (TermstoComplement.size() == 1) {
            cout << "Single cube, hence complemented directly by using DeMorgan's law" << endl;
            complementedTermsList = DeMorganLaws(TermstoComplement[0]);
            return complementedTermsList;
        }
        else {
            int index;
            index = selectMostBinateLiteral(TermstoComplement);
            positiveCofactorTermsList = complementFunction(positiveCoFactor(TermstoComplement, index));
            negativeCofactorTermsList = complementFunction(negativeCoFactor(TermstoComplement, index));

            complementedTermsList = x_and(positiveCofactorTermsList, index);
            xandbarnegativeCofactorTermsList = x_and_bar(negativeCofactorTermsList,index);
            complementedTermsList.insert(complementedTermsList.end(), xandbarnegativeCofactorTermsList.begin(), xandbarnegativeCofactorTermsList.end());
            return complementedTermsList;
        }
    } else {
        return complementedTermsList; // return empty complemented terms list
    }
}
// Unate Recursive Paradigm Code ends

vector<bitset<2>> formSuperCube(vector <vector<bitset<2>>> TermsToFormSuperCube) {
    vector<bitset<2>> superCubeTerm;

    superCubeTerm.assign(TermsToFormSuperCube[0].begin(),TermsToFormSuperCube[0].end());
    for (auto iter = TermsToFormSuperCube.begin()+1; iter != TermsToFormSuperCube.end() ; iter++) {
        size_t i = 0;
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            superCubeTerm[i] = superCubeTerm[i] | *it;
            i++;
        }
    }
    return superCubeTerm;
}

void reduce(vector <vector<bitset<2>>> irredundantTermsPCR) {
    vector<bitset<2>> reductionCheckTerm, complementTerm, maximallyReducedTerm;
    vector <vector<bitset<2>>> reductionCheckComplementTerms, complementedTerms, reducedTerms;
    if (irredundantTermsPCR.size() == 1) {
        cout << "Cannot be reduced further" << endl;
        return;
    }
    else if (irredundantTermsPCR.size() > 1) {
        for(size_t i=0;i<irredundantTermsPCR.size();i++) {
            //Check intersection of each term with terms in its complement set
            reductionCheckTerm.assign(irredundantTermsPCR[i].begin(),irredundantTermsPCR[i].end());
            cout << endl << "----------Checking reduction of " << reverse_positional_representation(reductionCheckTerm) << "----------" << endl;
            maximallyReducedTerm.assign(reductionCheckTerm.begin(),reductionCheckTerm.end());
            for(size_t j=0;j<irredundantTermsPCR.size();j++) {
                if (i!=j){
                    reductionCheckComplementTerms.push_back(irredundantTermsPCR[j]);
                }
            }
            complementedTerms = complementFunction(reductionCheckComplementTerms);
            cout << "complementedTerms : ";
            for (auto iter = complementedTerms.begin(); iter != complementedTerms.end() ; iter++) {
                for( auto it = iter->begin(); it != iter->end() ; it++ ) {
                    cout << (*it);
                }
                cout << " ";
            }
            cout << endl;
            complementTerm = formSuperCube(complementedTerms);
            cout << "SuperCube : ";
            for( auto it = complementTerm.begin(); it != complementTerm.end() ; it++ ) {
                cout << (*it);
            }
            cout << endl;
            for (size_t k = 0; k < reductionCheckTerm.size(); k++) {
                maximallyReducedTerm[k] = reductionCheckTerm[k] & complementTerm[k];
            }
            cout << "Cube " << reverse_positional_representation(reductionCheckTerm) << " can be maximally reduced to " << reverse_positional_representation(maximallyReducedTerm) << endl;
            reducedTerms.push_back(maximallyReducedTerm);
            reductionCheckComplementTerms.clear();
            reductionCheckTerm.clear();
        }
    }
    cout << endl << "Reduced Terms : " << endl;
    for (auto iter = reducedTerms.begin(); iter != reducedTerms.end() ; iter++) {
        cout << reverse_positional_representation(*iter) << " ";
        for( auto it = iter->begin(); it != iter->end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
}

void displayTerms() {
    cout << "minTerms : " << endl;
    for( auto iter = minTerms.begin(); iter != minTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
    cout << "dontcareTerms : " << endl;
    for( auto iter = dontcareTerms.begin(); iter != dontcareTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
    cout << "onsetTerms : " << endl;
    for( auto iter = onsetTerms.begin(); iter != onsetTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
    cout << "expandedTerms : " << endl;
    for( auto iter = expandedTerms.begin(); iter != expandedTerms.end() ; iter++ ) {
        cout << iter->input << " " << iter->output << " ";
        for( auto it = iter->positional_representation.begin(); it != iter->positional_representation.end() ; it++ ) {
            cout << *it;
        }
        cout << endl;
    }
}

int main() {
    string truth_table_file = "truthtable.txt";
    preprocess(truth_table_file);
    expandedTerms.assign(onsetTerms.begin(),onsetTerms.end());
    cout << endl << "----------Demonstrating single iteration of ESPRESSO----------" << endl;
    cout << "**********Expand Operation**********" << endl;
    for (size_t expand_bit_position = num_input_bits; expand_bit_position > 0; expand_bit_position--) {
        expand(expand_bit_position);
        displayTerms();
    }
    removeDuplicateExpandedTerms();

    cout << endl << "**********Irredundancy Operation**********" << endl << endl;
    checkRedundancy();

    cout << endl << "**********Reduce Operation**********" << endl;
    reduce(generatePCRofTermstoComplement(irredundantTerms));
}