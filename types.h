
#pragma once
#include <string>

enum class ReadType
{
    EndRead,
    BeginObj,
    EndObj,
    BeginArray,
    EndArray,
    String,
    StringSingleQ,
    Digit,
    True,
    False,
    JsonNull,
    ArraySeperator,
    MemberSeparator,
    MissingFieldValue,
    ErrorEncounter
};

using stringIter = std::string::const_iterator;

struct StringToken
{
    ReadType type_;
    stringIter begin_;
    stringIter end_;
};
struct ErrorType
{
    StringToken strTok_;
    std::string message_;
    int type_;
};

struct DocumentStream
{
    stringIter begin_;
    stringIter end_;
    stringIter it_;
    DocumentStream() = default;
    void setDocument(std::string const &doc)
    {
        begin_ = doc.begin();
        end_ = doc.end();
        it_ = begin_;
    }
    void removeSpaces()
    {
        while (it_ != end_)
        {
            auto c = *it_;
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
                ++it_;
            else
                break;
        }
    }
    char getC()
    {
        if (it_ == end_)
            return 0;
        return *it_++;
    }
};
