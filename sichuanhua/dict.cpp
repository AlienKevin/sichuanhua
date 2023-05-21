//
//  dict.cpp
//  sichuanhua
//
//  Created by Kevin Li on 5/21/23.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>
#include <numeric>
#include <unicode/unistr.h>
#include <unicode/regex.h>
#include <regex>
#include <json.hpp>
#include <optional>
#include <cstdlib>
#include <future>
#include "dict.hpp"

using namespace std;
using json = nlohmann::json;

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

bool isAnnotation(const string& str) {
    icu::UnicodeString unicode = icu::UnicodeString::fromUTF8(str);
    array<icu::UnicodeString, 4> annPrefixes = {"又说", "又作", "又音", "同 "};
    return any_of(annPrefixes.begin(), annPrefixes.end(), [unicode](auto& prefix) {
        return unicode.startsWith(prefix);
    });
}

FYDict readFYDict() {
    // Read in the JSON file
    ifstream jsonFile("fangyan.json");
    json fyJson;
    jsonFile >> fyJson;

    // Create an unordered_map to store the key-value pairs
    FYDict dict;

    // Inserting the key-value pairs from the JSON dictionary into the unordered_map
    int id = 0;
    regex gnPattern("(^|\\d)gn");
    for (auto& [category, entries] : fyJson.items()) {
        for (auto& entry : entries) {
            auto fyEntry = make_shared<FYEntry>();
            fyEntry->id = id;
            fyEntry->category = category;
            string pinyin = entry["pinyin"];
            // Replace gn with n
            pinyin = regex_replace(pinyin, gnPattern, "$1n");
            // Filter out puncutations in pinyin
            icu::UnicodeString pinyin_ustr = icu::UnicodeString::fromUTF8(pinyin);
            UErrorCode status = U_ZERO_ERROR;
            icu::RegexMatcher matcher = icu::RegexMatcher("[^[:alnum:]]", pinyin_ustr, 0, status);
            pinyin_ustr = matcher.replaceAll("", status);

            // Remove original tone before →
            int32_t arrow_index = pinyin_ustr.indexOf(u"→");
            if (arrow_index >= 0) {
                pinyin_ustr.remove(0, arrow_index + 1);
            }
            pinyin = "";
            pinyin_ustr.toUTF8String(pinyin);
            fyEntry->pinyin = pinyin;
            auto definitions = entry["definitions"].items();
            auto startDefinition = definitions.begin();
            if (isAnnotation(definitions.begin().value()[0])) {
                fyEntry->annotation = definitions.begin().value()[0];
                startDefinition = next(startDefinition);
            }
            for (auto it = startDefinition; it != definitions.end(); ++it) {
                FYDefinition fyDefinition;
                fyDefinition.definition = it.value()[0];
                for (auto& example : it.value()[1]) {
                    fyDefinition.examples.push_back(example);
                }
                fyEntry->definitions.push_back(fyDefinition);
            }
            
            string word = entry["word"];
            if (word.find("*") != word.npos) {
                word.erase(remove(word.begin(), word.end(), '*'), word.end());
                fyEntry->phoneticCharacters = true;
            } else {
                fyEntry->phoneticCharacters = false;
            }
            fyEntry->word = word;
            dict.dictByWord[word] = fyEntry;
            dict.dictById[id] = fyEntry;
            id += 1;
        }
    }
    
    return dict;
}

optional<shared_ptr<FYEntry>> lookupFYDict(FYDict& fyDict, string word) {
    if (fyDict.dictByWord.count(word)) {
        return fyDict.dictByWord[word];
    } else {
        return std::nullopt;
    }
}

vector<string> splitPinyinSyllables(const string& pinyin) {
  vector<string> syllables;
    int prevDigitIndex = 0;
    for (int i = 0; i < pinyin.size(); i++) {
        if (isdigit(pinyin[i])) {
            syllables.push_back(pinyin.substr(prevDigitIndex, i -prevDigitIndex + 1));
            i += 1;
            prevDigitIndex = i;
        }
    }
    return syllables;
}

template<typename MapType>
std::vector<int>& getVector(MapType& myMap, const typename MapType::key_type& key) {
    // If the key does not exist, insert an empty vector
    if (myMap.find(key) == myMap.end()) {
        myMap[key] = std::vector<int>();
    }
    return myMap[key];
}

std::string removeCharacterAtIndex(const icu::UnicodeString& str, int32_t index) {
    std::string plainStr;
    if (index >= str.length() || index < 0) {
        str.toUTF8String(plainStr);
        return plainStr;  // Index out of bounds, return the original string
    }
    (str.tempSubString(0, index) + str.tempSubString(index + 1)).toUTF8String(plainStr);
    return plainStr;
}

void serializeIndex(const Index& index, const string& filename) {
    json jsonObject;
    
    for (const auto& pair : index) {
        jsonObject[pair.first] = pair.second;
    }
    
    ofstream outputFile(filename + ".json", ios::out | ios::trunc);
    if (outputFile.is_open()) {
        outputFile << jsonObject.dump();
        outputFile.close();
        std::cout << "Index dumped to " << filesystem::current_path() / filesystem::path(filename + ".json") << std::endl;
    } else {
        std::cout << "Failed to create or open " << filename << ".json" << std::endl;
    }
}

Index deserializeIndex(const string& filename) {
    ifstream inputFile(filename + ".json");
    if (inputFile.is_open()) {
        // Read the JSON content from the file
        json jsonObject = json::parse(inputFile);
        
        // Deserialize JSON into std::unordered_map<std::string, std::vector<int>>
        unordered_map<string, vector<int>> index = jsonObject;
        return index;
    } else {
        return Index();
    }
}

FYIndex buildFYIndex(const FYDict& fyDict) {
    Index fullIndexWithSpaces = deserializeIndex("fullIndexWithSpaces");
    bool buildFullIndexWithSpaces = fullIndexWithSpaces.empty();
    
    Index fullIndexWithoutSpaces = deserializeIndex("fullIndexWithoutSpaces");
    bool buildFullIndexWithoutSpaces = fullIndexWithoutSpaces.empty();
    
    Index tonelessIndexWithSpaces = deserializeIndex("tonelessIndexWithSpaces");
    bool buildTonelessIndexWithSpaces = tonelessIndexWithSpaces.empty();
    
    Index tonelessIndexWithoutSpaces = deserializeIndex("tonelessIndexWithoutSpaces");
    bool buildTonelessIndexWithoutSpaces = tonelessIndexWithoutSpaces.empty();
    
    Index fullIndexWithSpacesDelete1 = deserializeIndex("fullIndexWithSpacesDelete1");
    if (buildFullIndexWithSpaces) {
        fullIndexWithSpacesDelete1.clear();
    }
    Index fullIndexWithoutSpacesDelete1 = deserializeIndex("fullIndexWithoutSpacesDelete1");
    if (buildFullIndexWithoutSpaces) {
        fullIndexWithoutSpacesDelete1.clear();
    }
    Index tonelessIndexWithSpacesDelete1 = deserializeIndex("tonelessIndexWithSpacesDelete1");
    if (buildTonelessIndexWithSpaces) {
        tonelessIndexWithSpacesDelete1.clear();
    }
    Index tonelessIndexWithoutSpacesDelete1 = deserializeIndex("tonelessIndexWithoutSpacesDelete1");
    if (buildTonelessIndexWithoutSpaces) {
        tonelessIndexWithoutSpacesDelete1.clear();
    }
    
    if (buildFullIndexWithSpaces || buildFullIndexWithoutSpaces || buildTonelessIndexWithSpaces || buildTonelessIndexWithoutSpaces) {
        for (auto& pair : fyDict.dictById) {
            // Replace all ü with v for easy typing by user
            icu::UnicodeString pinyinUstr = icu::UnicodeString::fromUTF8(pair.second->pinyin.c_str());
            pinyinUstr.findAndReplace(icu::UnicodeString(u"ü"), icu::UnicodeString(u"v"));
            std::string pinyin;
            pinyinUstr.toUTF8String(pinyin);
            vector<string> s = splitPinyinSyllables(pinyin);
            
            if (buildFullIndexWithSpaces) {
                string sWithSpaces = std::accumulate(std::next(s.begin()), s.end(), s[0], [](std::string a, std::string b) {return a + " " + b;});
                getVector(fullIndexWithSpaces, sWithSpaces).push_back(pair.second->id);
                icu::UnicodeString sWithSpacesUstr = icu::UnicodeString::fromUTF8(sWithSpaces);
                if (sWithSpacesUstr.length() > 1) {
                    for (int i = 0; i < sWithSpacesUstr.length(); i++) {
                        getVector(fullIndexWithSpacesDelete1, removeCharacterAtIndex(sWithSpacesUstr, i)).push_back(pair.second->id);
                    }
                }
            }
            
            if (buildFullIndexWithoutSpaces) {
                string sWithoutSpaces = std::accumulate(std::next(s.begin()), s.end(), s[0], [](std::string a, std::string b) {return a + b;});
                getVector(fullIndexWithoutSpaces, sWithoutSpaces).push_back(pair.second->id);
                icu::UnicodeString sWithoutSpacesUstr = icu::UnicodeString::fromUTF8(sWithoutSpaces);
                if (sWithoutSpacesUstr.length() > 1) {
                    for (int i = 0; i < sWithoutSpacesUstr.length(); i++) {
                        getVector(fullIndexWithoutSpacesDelete1, removeCharacterAtIndex(sWithoutSpacesUstr, i)).push_back(pair.second->id);
                    }
                }
            }
            
            if (buildTonelessIndexWithSpaces) {
                string sTonelessWithSpaces = std::accumulate(s.begin(), s.end(), string(), [](std::string a, std::string b) {
                    icu::UnicodeString bUstr = icu::UnicodeString::fromUTF8(b);
                    bUstr.truncate(bUstr.length() - 1);
                    b = "";
                    bUstr.toUTF8String(b);
                    if (a.empty()) {
                        return b;
                    } else {
                        return a + " " + b;
                    }
                });
                getVector(tonelessIndexWithSpaces, sTonelessWithSpaces).push_back(pair.second->id);
                icu::UnicodeString sTonelessWithSpacesUstr = icu::UnicodeString::fromUTF8(sTonelessWithSpaces);
                if (sTonelessWithSpacesUstr.length() > 1) {
                    for (int i = 0; i < sTonelessWithSpacesUstr.length(); i++) {
                        getVector(tonelessIndexWithSpacesDelete1, removeCharacterAtIndex(sTonelessWithSpacesUstr, i)).push_back(pair.second->id);
                    }
                }
            }
            
            if (buildTonelessIndexWithoutSpaces) {
                string sTonelessWithoutSpaces = std::accumulate(s.begin(), s.end(), string(), [](std::string a, std::string b) {
                    icu::UnicodeString bUstr = icu::UnicodeString::fromUTF8(b);
                    bUstr.truncate(bUstr.length() - 1);
                    b = "";
                    bUstr.toUTF8String(b);
                    if (a.empty()) {
                        return b;
                    } else {
                        return a + b;
                    }
                });
                getVector(tonelessIndexWithoutSpaces, sTonelessWithoutSpaces).push_back(pair.second->id);
                icu::UnicodeString sTonelessWithoutSpacesUstr = icu::UnicodeString::fromUTF8(sTonelessWithoutSpaces);
                if (sTonelessWithoutSpacesUstr.length() > 1) {
                    for (int i = 0; i < sTonelessWithoutSpacesUstr.length(); i++) {
                        getVector(tonelessIndexWithoutSpacesDelete1, removeCharacterAtIndex(sTonelessWithoutSpacesUstr, i)).push_back(pair.second->id);
                    }
                }
            }
        }
    }
    
    if (buildFullIndexWithSpaces) {
        serializeIndex(fullIndexWithSpaces, "fullIndexWithSpaces");
        serializeIndex(fullIndexWithSpacesDelete1, "fullIndexWithSpacesDelete1");
    }
    if (buildFullIndexWithoutSpaces) {
        serializeIndex(fullIndexWithoutSpaces, "fullIndexWithoutSpaces");
        serializeIndex(fullIndexWithoutSpacesDelete1, "fullIndexWithoutSpacesDelete1");
    }
    if (buildTonelessIndexWithSpaces) {
        serializeIndex(tonelessIndexWithSpaces, "tonelessIndexWithSpaces");
        serializeIndex(tonelessIndexWithSpacesDelete1, "tonelessIndexWithSpacesDelete1");
    }
    if (buildTonelessIndexWithoutSpaces) {
        serializeIndex(tonelessIndexWithoutSpaces, "tonelessIndexWithoutSpaces");
        serializeIndex(tonelessIndexWithoutSpacesDelete1, "tonelessIndexWithoutSpacesDelete1");
    }
    
    FYIndex result = {
        fullIndexWithSpaces,
        fullIndexWithoutSpaces,
        tonelessIndexWithSpaces,
        tonelessIndexWithoutSpaces,
        
        fullIndexWithSpacesDelete1,
        fullIndexWithoutSpacesDelete1,
        tonelessIndexWithSpacesDelete1,
        tonelessIndexWithoutSpacesDelete1
    };
    return result;
}

optional<vector<int>> lookupIndex(const Index& index, const string& query)
{
    auto it = index.find(query);
    if (it != index.end()) {
        return it->second;
    }
    else {
        return std::nullopt; // Key not found, return std::nullopt
    }
}

optional<vector<int>> lookupDelete1Index(const Index& index, const string& query) {
    auto result = lookupIndex(index, query);
    if (result.has_value()) {
        return result;
    }
    icu::UnicodeString queryUstr = icu::UnicodeString::fromUTF8(query);
    for (int i = 0; i < queryUstr.length(); i++) {
        icu::UnicodeString queryDelete1Ustr = queryUstr.tempSubString(0, i) + queryUstr.tempSubString(i + 1);
        string queryDelete1;
        queryDelete1Ustr.toUTF8String(queryDelete1);
        auto result = lookupIndex(index, queryDelete1);
        if (result.has_value()) {
            return result;
        }
    }
    return nullopt;
}

template<typename T, typename F>
std::optional<T> unwrapOrElse(const std::optional<T>& optValue, F&& fallbackFn) {
    return optValue.has_value() ? optValue : fallbackFn();
}

optional<vector<int>> lookupFYIndex(const FYIndex& index, const string& query) {
    return unwrapOrElse(lookupIndex(index.fullIndexWithSpaces, query),
        [&]() {return unwrapOrElse(lookupIndex(index.fullIndexWithoutSpaces, query),
        [&]() {return unwrapOrElse(lookupIndex(index.tonelessIndexWithSpaces, query),
        [&]() {return unwrapOrElse(lookupIndex(index.tonelessIndexWithoutSpaces, query),
        [&]() {return unwrapOrElse(lookupDelete1Index(index.fullIndexWithSpacesDelete1, query),
        [&]() {return unwrapOrElse(lookupDelete1Index(index.fullIndexWithoutSpacesDelete1, query),
        [&]() {return unwrapOrElse(lookupDelete1Index(index.tonelessIndexWithSpacesDelete1, query),
        [&]() {return lookupDelete1Index(index.tonelessIndexWithoutSpacesDelete1, query);});});});});});});});
}

void Dict::printEntry(const FYEntry& entry, const std::string& query) {
    cout << query << ": " << (entry.word == query ? "" : (entry.word + " ")) << entry.pinyin << " " << entry.annotation << endl;
    for (auto& definition : entry.definitions) {
        cout << "  " << definition.definition << endl;
        vector<string> examples = definition.examples;
        for (auto& example : examples) {
            cout << "    " << example << endl;
        }
    }
}

void Dict::say(const string& word) {
    string command = "say --voice=\"Panpan (Premium)\" " + word + " > /dev/null 2>&1";
    system(command.c_str());
}

void sayAndSave(const string& word, int id) {
    string command = "say --voice=\"Panpan (Premium)\" -o " + to_string(id) + ".m4a " + " \"[[rate 170]] " + word + "\" > /dev/null 2>&1";
    system(command.c_str());
}

void buildSound(const std::pair<const int, std::shared_ptr<FYEntry>>& pair) {
    icu::UnicodeString wordUstr = icu::UnicodeString::fromUTF8(pair.second->word);
    wordUstr.findAndReplace("——", "，");
    string wordForPr;
    wordUstr.toUTF8String(wordForPr);
    sayAndSave(wordForPr, pair.first);
}

void Dict::buildSoundIndex() {
    cout << "Building sound index for each entry of Fangyan dict..." << endl;
    
    std::vector<std::future<void>> futures;

    for (const std::pair<const int, std::shared_ptr<FYEntry>> &pair : fyDict.dictById) {
        futures.emplace_back(std::async(std::launch::async, buildSound, pair));
    }

    // Wait for all tasks to complete
    for (auto& future : futures) {
        future.wait();
    }
}

Dict::Dict() {
    prs = convertToCenduPrDict(readPrDict());
    fyDict = readFYDict();
    fyIndex = buildFYIndex(fyDict);
}

SearchResult Dict::search(const std::string& query) {
    auto fyEntry = lookupFYDict(fyDict, query);
    if (fyEntry.has_value()) {
        FYEntry entry = *fyEntry.value();
        EntriesResult entries;
        entries.entries.push_back(entry);
        return entries;
    } else {
        auto fyResult = lookupFYIndex(fyIndex, query);
        if (fyResult.has_value()) {
            EntriesResult entries;
            for (int id : fyResult.value()) {
                FYEntry entry = *fyDict.dictById[id];
                entries.entries.push_back(entry);
            }
            return entries;
        } else {
            std::vector<std::string> prResults = lookupPrDict(prs, query);
            if (prResults.empty()) {
                return NotFoundResult();
            } else {
                CharResult result;
                result.prs = prResults;
                return result;
            }
        }
    }
}

nlohmann::json FYEntryToJson(const FYEntry& entry) {
    // Create a JSON object and assign the values
    nlohmann::json jsonObj;
    jsonObj["id"] = entry.id;
    jsonObj["word"] = entry.word;
    jsonObj["phoneticCharacters"] = entry.phoneticCharacters;
    jsonObj["category"] = entry.category;
    jsonObj["annotation"] = entry.annotation;
    jsonObj["pinyin"] = entry.pinyin;

    // Create a JSON array for definitions
    nlohmann::json definitionsArray;
    for (const auto& definition : entry.definitions) {
        nlohmann::json definitionObj;
        definitionObj["definition"] = definition.definition;
        definitionObj["examples"] = definition.examples;
        definitionsArray.push_back(definitionObj);
    }

    // Assign the definitions array to the JSON object
    jsonObj["definitions"] = definitionsArray;

    return jsonObj;
}

nlohmann::json FYEntriesToJson(const std::vector<FYEntry> entries) {
    nlohmann::json entriesArray;
    for (const auto& entry : entries) {
        entriesArray.push_back(FYEntryToJson(entry));
    }
    return entriesArray;
}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

nlohmann::json Dict::searchResultToJson(const SearchResult& result) {
    return std::visit(overloaded {
        [](const CharResult& result){
            return nlohmann::json{{"prs", result.prs}};
        },
        [](const EntriesResult& result) {
            return nlohmann::json{{"entries", FYEntriesToJson(result.entries)}};
        },
        [](const NotFoundResult& result) {
            return nlohmann::json{{"notFound", true}};
        }
        }, result);
}
