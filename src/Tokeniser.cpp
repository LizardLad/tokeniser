#include <iostream>
#include <utility>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <Tokeniser.hpp>
#include <DocumentList.hpp>
#include <WikipediaReader.hpp>

#include <omp.h>

template<>
struct std::hash<std::pair<uint16_t, uint16_t>>
{
    std::size_t operator()(const std::pair<uint16_t, uint16_t>& s) const noexcept
    {
        std::size_t h1 = std::hash<uint16_t>{}(s.first);
        std::size_t h2 = std::hash<uint16_t>{}(s.second);
        return h1 ^ (h2 << 1);
    }
};

Tokeniser::Tokeniser(void)
{
}

Tokeniser::~Tokeniser()
{
}

std::unordered_map<std::pair<uint16_t, uint16_t>, uint64_t> Tokeniser::getAllBytePairs(std::vector<uint16_t> &data) {
    std::unordered_map<std::pair<uint16_t, uint16_t>,uint64_t> pairs = {};
    for(std::vector<uint16_t>::iterator it = data.begin(); it != data.end() - 1; it++) {
        pairs.emplace(std::make_pair((*it), *(it+1)), 0).first->second++;
    }
    return pairs;
}

bool Tokeniser::train(DocumentList &documents, uint16_t vocab_size) {
    if(vocab_size < 256) {return false;}
    for(uint16_t i = 256; i < vocab_size; i++) {
        std::unordered_map<std::pair<uint16_t, uint16_t>, uint64_t> pairs;
        pairs.reserve(UINT16_MAX);
        while(1) {
            std::vector<std::vector<uint16_t>> parallel_documents;
            int j = 0;
            int num_threads = omp_get_num_threads();
            for(; j < num_threads; j++) {
                std::vector<uint16_t> document;
                try {
                    std::vector<uint16_t> document = documents.next();
                } catch (std::runtime_error error) {
                    std::cerr << error.what() << std::endl;
                    break;
                }
                if(document.size() < 1) {
                    break;
                }

                parallel_documents.push_back(document);
            }

            if(j+1 != num_threads) {
                break;
            }

            #pragma omp parallel shared(pairs)
            for (std::vector<std::vector<uint16_t>>::iterator it = parallel_documents.begin(); it != parallel_documents.end(); it++){
                //Apply all transformations already known because they can't persist
                this->transform(*it);
                std::unordered_map<std::pair<uint16_t,uint16_t>, uint64_t> pairs_ = this->getAllBytePairs(*it);

                #pragma omp critical
                for(auto& [pair, count]: pairs_) {
                    if(auto search = pairs.find(pair); search != pairs.end()) {
                        pairs[pair] += count;
                    } else {
                        pairs[pair] = count;
                    }
                }
            }

        }

        std::pair<uint16_t, uint16_t> most_common = {0,0};
        uint64_t count = 0;
        for(auto& [pair, count_]: pairs) {
            if(count_ > count) {
                most_common = pair;
            }
        }

        VocabEntry entry = {.pair = {std::get<0>(most_common), std::get<1>(most_common)}, .replacement=i};
        this->vocab.push_back(entry);

        std::cout << "Most common pair: {" << entry.pair[0] << ", " << entry.pair[1] << "} Replacement: " << i << " Count: " << count << std::endl;
    }
    return true;
}

void Tokeniser::transform(std::vector<uint16_t> &data) {
    for(std::vector<VocabEntry>::iterator pit = this->vocab.begin(); pit != this->vocab.end(); ++pit) {
        VocabEntry pair_w_replacement = (*pit);
        for(std::vector<uint16_t>::iterator it = data.begin(); it < (data.end()-1); ++it) {
            if(*it == pair_w_replacement.pair[0] && *(it+1) == pair_w_replacement.pair[1]) {
                *it = pair_w_replacement.replacement;
                data.erase(it+1);
            }
        }
    }
}
