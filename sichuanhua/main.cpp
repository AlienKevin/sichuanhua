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

int main()
{
    // Open the CSV file
    ifstream file("shupin.dict.yaml");
//    cout << "Reading shupin.dict" << endl;

    // Declare variables to hold the values
    unordered_map<string, vector<string>> prs;

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
    
    cout << prs.size() << endl;

    // Output the values
    for (auto const& pair : prs)
    {
        string pr = accumulate(begin(pair.second), end(pair.second), string(),
                               [](const string& a, const string& b) -> string {
                                   return a.empty() ? b : a + ", " + b;
        });
        std::cout << pair.first << ": " << pr << std::endl;
    }

    return 0;
}
