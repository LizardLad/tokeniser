#ifndef WIKIPEDIA_READER_H
#define WIKIPEDIA_READER_H

#include <vector>
#include <DocumentList.hpp>
#include <libxml/xmlreader.h>

std::ostream& operator<<(std::ostream& os, const std::vector<uint16_t> a);

class WikipediaReader : public DocumentList
{
private:
    xmlChar *ignored_tag_start;
    xmlTextReaderPtr reader;
    int ret;

    bool nodeFilter(xmlChar *name, xmlChar *value);
    std::vector<uint16_t> processNode();
public:
    WikipediaReader(const char *filename);
    ~WikipediaReader();
    std::vector<uint16_t> next(void) override;
};

#endif
