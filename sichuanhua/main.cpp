#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <numeric>
#include <unicode/unistr.h>
#include <regex>

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

vector<string> lookupPrDict(const PrDict& prs, const string& word) {
    auto pr = prs.find(word);
    if (pr != prs.end()) {
        return pr->second;
    } else {
        return vector<string> {};
    }
}

string applyRegexRules(const string& pinyin, const vector<pair<regex, string>>& rules) {
    string result = pinyin;
    for (auto rule : rules) {
        result = regex_replace(result, rule.first, rule.second);
    }
    return result;
}

PrDict convertToCenduPrDict(PrDict prs) {
    vector<pair<regex, string>> rules = {
        // 特字部分
        {regex("Ddi"), "le"},    // 的字讀音
        {regex("Dqu4"), "qie4"}, // 去字讀音
        {regex("Dlu5"), "liu2"},  // 六字讀音
        // 通入部分
        {regex("^su5$"), "sv5"}, // 成都特殊设定・速俗v/io两可
        // 入聲派向部分
        {regex("5$"), "2"},
        // 戈歌部分
        {regex("eo([1234])$"), "o$1"}, // 不分舒聲戈歌
        {regex("eo5"), "o5"}, // 不分入聲戈歌
        // 尖團部分
        {regex("^zyi"), "ji"}, // zyi>ji (挤)
        {regex("^cyi"), "qi"}, // cyi>qi (七)
        {regex("^syi"), "xi"}, // syi>xi (西)
        {regex("^zi([a-z]+)"), "ji$1"}, // zi->ji- (尖)
        {regex("^ci([a-z]+)"), "qi$1"}, // ci->qi- (雀)
        {regex("^si([a-z]+)"), "xi$1"}, // si->xi- (仙)
        {regex("^zv"), "ju"},  // zü>ju, zü->ju- (聚)
        {regex("^cv"), "qu"},  // cü>qu, cü->qu- (趣)
        {regex("^sv"), "xu"},  // sü>xu, sü->xu- (须)
        // 平翹部分
        {regex("^([zcs])h"), "$1"}, // 無翹舌
        // h、f部分
        {regex("^hu([12345])$"), "fu$1"}, // hu>fu
        // 成都特字
        {regex("C"), ""},
    };
    for (auto& entry : prs) {
        for (auto& pr : entry.second) {
//            cout << "before: " << pr << endl;
            pr = applyRegexRules(pr, rules);
//            cout << "after: " << pr << endl;
        }
    }
    return prs;
}

int main(int argc, char** argv)
{
    // Parse command-line arguments
    if (argc < 2) {
        cout << "Usage: sch <word1> (<word2> ...)" << endl;
        return 0;
    }
    PrDict prs = convertToCenduPrDict(readPrDict());
    
    for (int i = 1; i < argc; i++) {
        string word = argv[i];
        
        vector<string> pr_results = lookupPrDict(prs, word);
        
        if (pr_results.empty()) {
            cout << word << " is not found." << endl;
        } else {
            cout << word << ": " << accumulate(pr_results.begin(), pr_results.end(), std::string(), [&](auto a, auto b) {
                return a.empty() ? b : a + ", " + b;
            }) << endl;
        }
    }
    
    return 0;
}
