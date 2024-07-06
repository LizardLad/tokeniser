#ifndef TOKENISER_H
#define TOKENISER_H

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <DocumentList.hpp>


template <typename T>
struct __VocabEntry {
    T pair[2];
    T replacement;
};

using VocabEntry = struct __VocabEntry<uint16_t>;

class Tokeniser
{
private:
    std::vector<VocabEntry> vocab;
public:
    Tokeniser(/* args */);
    ~Tokeniser();
    std::unordered_map<std::pair<uint16_t, uint16_t>, uint64_t> getAllBytePairs(std::vector<uint16_t> &data);
    bool train(DocumentList &documents, uint16_t vocab_size);
    void transform(std::vector<uint16_t> &data);
};


#endif
