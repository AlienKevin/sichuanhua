//
//  dict.hpp
//  sichuanhua
//
//  Created by Kevin Li on 5/21/23.
//

#ifndef dict_hpp
#define dict_hpp

#include <nlohmann/json.hpp>
#include <variant>

using PrDict = std::unordered_map<std::string, std::vector<std::string>>;

using Index = std::unordered_map<std::string, std::vector<int>>;

struct FYIndex {
    Index fullIndexWithSpaces;
    Index fullIndexWithoutSpaces;
    Index tonelessIndexWithSpaces;
    Index tonelessIndexWithoutSpaces;
    
    Index fullIndexWithSpacesDelete1;
    Index fullIndexWithoutSpacesDelete1;
    Index tonelessIndexWithSpacesDelete1;
    Index tonelessIndexWithoutSpacesDelete1;
};

struct FYDefinition {
    std::string definition;
    std::vector<std::string> examples;
};

struct FYEntry {
    int id;
    std::string word;
    bool phoneticCharacters;
    std::string category;
    std::string annotation;
    std::string pinyin;
    std::vector<FYDefinition> definitions;
};

using FYDictByWord = std::unordered_map<std::string, std::shared_ptr<FYEntry>>;
using FYDictById = std::unordered_map<int, std::shared_ptr<FYEntry>>;
struct FYDict {
    FYDictByWord dictByWord;
    FYDictById dictById;
};

struct CharResult { std::vector<std::string> prs; };
struct EntriesResult { std::vector<FYEntry> entries; };
struct NotFoundResult {};
using SearchResult = std::variant<CharResult, EntriesResult, NotFoundResult>;

class Dict {
public:
    Dict();
    SearchResult search(const std::string& query);
    static void printEntry(const FYEntry& entry, const std::string& query);
    static void say(const std::string& word);
    static nlohmann::json searchResultToJson(const SearchResult& result);
    void buildSoundIndex();
private:
    PrDict prs;
    FYDict fyDict;
    FYIndex fyIndex;
};

#endif /* dict_hpp */
