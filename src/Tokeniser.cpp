#include <iostream>
#include <fstream>
#include <cstdint>
#include <climits>
#include <utility>
#include <vector>
#include <unordered_map>

template<>
struct std::hash<std::pair<uint32_t, uint32_t>>
{
    std::size_t operator()(const std::pair<uint32_t, uint32_t>& s) const noexcept
    {
        std::size_t h1 = std::hash<uint32_t>{}(std::get<0>(s));
        std::size_t h2 = std::hash<uint32_t>{}(std::get<1>(s));
        return h1 ^ (h2 << 1);
    }
};

struct token_substitution_t {
    char first;
    char second;
    char replacement;
};

class BasicTokeniser
{
private:
    unsigned int vocabulary_size;
    std::vector<struct token_substitution_t> vocabulary;

    void u32_replace_all(std::u32string &s, std::u32string &to_replace, std::u32string &replace_with);


    std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> get_token_pair_counts(std::u32string &document);
    std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> merge_token_pair_counts(std::vector<std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t>> token_pair_map_vec);
    std::pair<uint32_t, uint32_t> get_highest_pair(std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> byte_pairs);
    void train_step(std::vector<std::u32string> documents, int i);
public:
    BasicTokeniser(unsigned int vocabulary_size);
    ~BasicTokeniser();
    
    void train(std::vector<std::string> documents_str);
    
};

BasicTokeniser::BasicTokeniser(unsigned int vocabulary_size)
{
    if(vocabulary_size < 256) {
        std::clog << "Invalid vocabulary size, it can't be less than 256\n";
    }
    this->vocabulary_size = vocabulary_size;
    this->vocabulary = {};
}

BasicTokeniser::~BasicTokeniser()
{
}

void BasicTokeniser::u32_replace_all(std::u32string &s, std::u32string &to_replace, std::u32string &replace_with) 
{
    std::u32string buf;
    std::size_t pos = 0;
    std::size_t prev_pos;
    buf.reserve(s.size());
    while(true) {
        prev_pos = pos;
        pos = s.find(to_replace, pos);
        if(pos == std::u32string::npos) {
            break;
        }

        buf.append(s, prev_pos, pos-prev_pos);
        buf += replace_with;
        pos += to_replace.size();
    }
    buf.append(s, prev_pos, s.size() - prev_pos);
    s.swap(buf);
}

void BasicTokeniser::train_step(std::vector<std::u32string> documents, int i)
{
    //Step one, identify the most common pair across documents
    std::vector<std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t>> token_pair_vec = {};
    for(std::u32string &document: documents) {
        std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> document_token_pairs = this->get_token_pair_counts(document);
        token_pair_vec.push_back(document_token_pairs);
    }
    std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> token_pairs = this->merge_token_pair_counts(token_pair_vec);

    //Now can get the byte pairs
    std::pair<uint32_t, uint32_t> highest_pair = get_highest_pair(token_pairs);
    std::cout << "Highest Byte pair: [" << std::get<0>(highest_pair) << ", " << std::get<1>(highest_pair) << "] | Count: HIGHEST\n";

    //Step two, replace the pair across documents with a new token
    uint32_t new_token = i +  (UCHAR_MAX+1);
    std::u32string replace_with;replace_with.push_back(new_token);
    for(std::u32string &document: documents) {
        std::u32string to_replace;to_replace.push_back(std::get<0>(highest_pair));to_replace.push_back(std::get<1>(highest_pair));
        std::cout << "Trying to replace stuff" <<std::endl;
        this->u32_replace_all(document, to_replace, replace_with); //BUG the replace isn't getting up to ::train
        std::cout << "[";
        for(const auto &n: document) {
            std::cout << n << ", ";
        }
        std::cout << "]" << std::endl;
    }
    
}

void BasicTokeniser::train(std::vector<std::string> documents_str) 
{
    std::vector<std::u32string> documents;
    for(auto &document: documents_str) {
        std::u32string document_str;
        for(auto &c: document) {
            document_str.push_back((char32_t)c);
        }
        documents.push_back(document_str);
    }

    for(int i = 0; i < (vocabulary_size - (UCHAR_MAX+1)); i++) { //Only loop for added vocabulary past the existing one byte already represented
        //Get most common pair of bytes in each document but not including any bytes on document boundries
        this->train_step(documents, i);
    }
}


std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> BasicTokeniser::get_token_pair_counts(std::u32string &document)
{
    std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> byte_pairs = {};
    for(std::u32string::iterator it = document.begin(); it != document.end() - 1; it++) {
        std::pair<uint32_t, uint32_t> pair = {(*it), *(it+1)};
        if(auto search = byte_pairs.find(pair); search != byte_pairs.end()) {
            byte_pairs[pair]++;
        } else {
            byte_pairs[pair] = 1;
        }
    }
    return byte_pairs;
}

std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> BasicTokeniser::merge_token_pair_counts(std::vector<std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t>> token_pair_map_vec) 
{
    if(token_pair_map_vec.size() < 1) {
        std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> map = {};
        return map;
    } else if(token_pair_map_vec.size() == 1) {
        return token_pair_map_vec[0];
    }

    std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> map = token_pair_map_vec[0];
    for(std::vector<std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t>>::size_type i = 0; i < token_pair_map_vec.size(); i++) {
        for(const std::pair<std::pair<uint32_t, uint32_t>, uint64_t> &n: token_pair_map_vec[i]) {
            std::pair<uint32_t, uint32_t> pair = n.first;
            uint64_t count = n.second;
            if(auto search = map.find(pair); search != map.end()) {
                map[pair] += count;
            } else {
                map[pair] = count;
            }
        }
    }
    return map;
}

std::pair<uint32_t, uint32_t> BasicTokeniser::get_highest_pair(std::unordered_map<std::pair<uint32_t, uint32_t>, uint64_t> byte_pairs) {
    uint64_t highest_count = 0;
    std::pair<uint32_t, uint32_t> pair = {0,0};
    for(const std::pair<std::pair<uint32_t, uint32_t>, uint64_t> &n: byte_pairs) {
        if(highest_count < n.second) {
            pair = n.first;
            highest_count = n.second;
        }
    }
    return pair;
}

int main(int argc, char **argv) {
    std::ifstream fs;
    fs.open("test.txt");
    std::string input((std::istreambuf_iterator<char>(fs)), (std::istreambuf_iterator<char>()));
    fs.close();

    BasicTokeniser tokeniser(300);
    tokeniser.train(std::vector<std::string>{input});

    return 0;
}

