#include <iostream>
#include <numeric>
#include "server.hpp"
#include "dict.hpp"

int main(int argc, char** argv)
{
    // Parse command-line arguments
    if (argc < 2) {
        std::cout << "Usage: sch <word1> (<word2> ...)" << std::endl;
        std::cout << "Usage: sch sounds" << std::endl;
        std::cout << "Usage: sch serve" << std::endl;
        return 0;
    }
    
    Dict dict = Dict();
    
    if (argc == 2 && strcmp(argv[1], "sounds") == 0) {
        dict.buildSoundIndex();
    } else if (argc == 2 && strcmp(argv[1], "serve") == 0) {
        auto server = HTTPServer();
        server.start();
    } else {
        for (int i = 1; i < argc; i++) {
            std::string query = argv[i];
            
            SearchResult result = dict.search(query);
            std::visit([&](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, CharResult>) {
                        std::cout << query << ": " << accumulate(arg.prs.begin(), arg.prs.end(), std::string(), [&](auto a, auto b) {
                            return a.empty() ? b : a + ", " + b;
                        }) << std::endl;
                        dict.say(query);
                    } else if constexpr (std::is_same_v<T, EntriesResult>) {
                        for (auto entry : arg.entries) {
                            Dict::printEntry(entry, query);
                            Dict::say(entry.word);
                        }
                    } else if constexpr (std::is_same_v<T, NotFoundResult>) {
                        std::cout << query << " is not found." << std::endl;
                    }
                }, result);
        }
    }
    
    return 0;
}
