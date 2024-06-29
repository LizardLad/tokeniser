#include <fstream>
#include <iostream>
#include <cstring>

#include <libxml/xmlreader.h>
#include <vector>

std::ostream& operator<<(std::ostream& os, const std::vector<uint32_t> a) {
    for(auto &n: a) {
        if(n < 255) {
            os << (char)n;
        } else {
            os << n;
        }
    }
    return os;
}

class WikipediaReader
{
private:
    xmlChar *ignored_tag_start;
    xmlTextReaderPtr reader;
    int ret;

    bool nodeFilter(xmlChar *name, xmlChar *value);
    std::vector<uint32_t> processNode();
public:
    WikipediaReader(const char *filename);
    ~WikipediaReader();
    std::vector<uint32_t> next();
};

WikipediaReader::WikipediaReader(const char *filename)
{
    this->ignored_tag_start = xmlCharStrdup("#REDIRECT ");
    this->reader = xmlNewTextReaderFilename(filename);
    if(this->reader == NULL) {
        throw std::runtime_error("Failed to open file");
    }
}

WikipediaReader::~WikipediaReader()
{
    xmlFree(this->ignored_tag_start);
    xmlFreeTextReader(this->reader);
    xmlDictCleanup();
    xmlCleanupCharEncodingHandlers();
}


bool WikipediaReader::nodeFilter(xmlChar *name, xmlChar *value) 
{
    return (value != NULL && 
        xmlTextReaderDepth(this->reader) == 4 &&
        xmlTextReaderNodeType(this->reader) == 3 && //https://learn.microsoft.com/en-us/dotnet/api/system.xml.xmlnodetype?view=net-8.0
        xmlStrncmp(value, this->ignored_tag_start, 10) &&
        xmlTextReaderIsEmptyElement(this->reader) == 0 &&
    1);
}

std::vector<uint32_t> WikipediaReader::processNode() 
{
    xmlChar *name, *value;
    name = xmlTextReaderName(this->reader);
    if(name == NULL) {
        name = xmlStrdup(BAD_CAST "--");
    }
    value = xmlTextReaderValue(this->reader);

    if(!this->nodeFilter(name, value)) {
        std::vector<uint32_t> empty_vec({});
        xmlFree(name);
        if(value != NULL) {xmlFree(value);}
        return empty_vec;
    }

    size_t len = xmlStrlen(value);
    std::vector<uint32_t> text; text.reserve(len);
    for(size_t i = 0; i < len; i++) {
        text.push_back(value[i]); //Do it this way because of the underlying type difference (uint32_t vs uint8_t)
    }

    xmlFree(name);
    if(value != NULL) {xmlFree(value);}

    return text;
}

std::vector<uint32_t> WikipediaReader::next() {
    while(true) {
        if(!xmlTextReaderRead(this->reader)) {
            throw std::runtime_error("Failed to parse file");
        }
        xmlChar *name = xmlCharStrdup("text");
        xmlChar *text_node_name = BAD_CAST "text";
        while(xmlStrncmp(name, text_node_name, 4) || xmlTextReaderNodeType(this->reader) != 1 || xmlTextReaderDepth(this->reader) != 3) {
            xmlFree(name);
            if(!xmlTextReaderRead(this->reader)) {
                throw std::runtime_error("Failed to parse file");
            }
            name = xmlTextReaderName(this->reader);
        }

        xmlFree(name);
        //At the start node, inside this node is the page text (should be)
        if(!xmlTextReaderRead(this->reader)) {
            throw std::runtime_error("Failed to parse file");
        }

        std::vector<uint32_t> text = this->processNode();
        size_t len = text.size();
        if(len) {
            return text;
        }
    }
}

int main(int argc, char **argv) {
    if(argc != 2) {
        std::cerr << "Incorrect argument count" << std::endl;
        return 1;
    }
    
    char *filename = argv[1];
    std::unique_ptr<WikipediaReader> reader = std::make_unique<WikipediaReader>(filename);

    for(int i = 0; i < 5; i++) {
        std::cout << reader->next() << "\n\n" << std::endl;
    }

    return 0;
}