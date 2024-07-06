#ifndef DOCUMENT_LIST_HPP
#define DOCUMENT_LIST_HPP

#include <vector>
#include <cstdint>

class DocumentList
{
    public:
        virtual std::vector<uint16_t> next(void) = 0;
};

#endif
