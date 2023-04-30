#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <numeric>

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
//        size_t pos = pr.find('◎');
//        // If the '◎' character is found, remove all characters after it
//        if (pos != std::string::npos) {
//            pr = pr.substr(0, pos);
//        }
        
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
