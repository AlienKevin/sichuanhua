#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <numeric>
#include <unicode/unistr.h>

using namespace std;

string strip(string myString) {
    // Remove leading whitespace
    size_t startPos = myString.find_first_not_of(" \t\r\n");
    if (startPos != string::npos) {
        myString = myString.substr(startPos);
    }

    // Remove trailing whitespace
    size_t endPos = myString.find_last_not_of(" \t\r\n");
    if (endPos != string::npos) {
        myString = myString.substr(0, endPos + 1);
    }
    
    // Handle a string containing only whitespace
    if (startPos == string::npos && endPos == string::npos) {
        return "";
    } else {
        return myString;
    }
}

using PrDict = unordered_map<string, vector<string>>;

PrDict readPrDict() {
    // Open the CSV file
    ifstream file("shupin.simp.dict.yaml");
    
    // Declare variables to hold the values
    PrDict prs;
    
    // Read each line of the file
    string line;
    while (getline(file, line))
    {
        // Split the line into individual values
        stringstream ss(line);
        string word;
        getline(ss, word, '\t');
        word = strip(word);
        //        cout << word << ":";
        string pr;
        getline(ss, pr, '\t');
        pr = strip(pr);
        
        // Remove comment after pr
        icu::UnicodeString pr_ustr = icu::UnicodeString::fromUTF8(pr);
        int32_t index = pr_ustr.indexOf(u"◎");
        if (index >= 0) {
            std::string output;
            pr_ustr.truncate(index);
            pr_ustr.toUTF8String(output);
            pr = output;
            //            cout << "pr: " << pr << ", index: " << index << endl;
        }
        
        if (word.empty() || pr.empty()) {
            continue;
        }
        if (prs.contains(word)) {
            prs[word].push_back(pr);
        } else {
            prs[word] = vector<string>{pr};
        }
    }
    return prs;
}

vector<string> lookupPrDict(PrDict prs, string word) {
    auto pr = prs.find(word);
    if (pr != prs.end()) {
        return pr->second;
    } else {
        return vector<string> {};
    }
}

int main(int argc, char** argv)
{
    // Parse command-line arguments
    if (argc < 2) {
        cout << "Usage: sch <word>" << endl;
        return 0;
    }
    string word = argv[1];
    
    PrDict prs = readPrDict();

    vector<string> pr_results = lookupPrDict(prs, word);
    
    if (pr_results.empty()) {
        cout << word << " is not found." << endl;
    } else {
        cout << word << ": " << accumulate(pr_results.begin(), pr_results.end(), std::string(), [&](auto a, auto b) {
            return a.empty() ? b : a + ", " + b;
        }) << endl;
    }
    
    return 0;
}
