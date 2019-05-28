
#pragma once
#include <unordered_map>
#include <vector>
#include <functional>
#include "types.h"
class Reader
{
public:
    Reader();

    const std::vector<ErrorType> &getErrors() const { return errors_; }

    bool processLine(std::string const &doc);

private:
    // propagate cache, callbacks, 
    void init();

    //! Parse one token from JSON text
    bool getToken(StringToken &token);

    // Parse object: { string : value, ... }
    bool getObject(StringToken &token);

    // Parse array: [ value, ... ]
    bool getArray(StringToken &token);

    // Parse [ value, ... ] or {"tag",value,...}
    bool getValue(bool firstType = false);

   // Parse string and generate error event, if encoured.
    bool parse(std::string const &doc);
    
    ReadType getFirstType() const { return firstType_; }

    bool getChars(StringToken &token);
    bool readNumber(StringToken &token);

    // Substring wrapper
    bool getSubStr(const char *pattern, int patternLength);
    bool getTrue(StringToken &token) { return getSubStr("rue", 3); }
    bool getFalse(StringToken &token) { return getSubStr("alse", 4); }
    bool getNull(StringToken &token) { return getSubStr("ull", 3); }

    bool getMissingFieldVal(StringToken &token);

    //Utility functions to get some types 
    bool decodeStringWrap(StringToken &token);
    bool decodeString(StringToken &token, std::string &decoded, char sep);
    bool decodeDouble(StringToken &token);
    bool decodeNumber(StringToken &token);

    bool Log(const std::string &message, StringToken &token, int type = 5);


    //Read value error
    bool readError(StringToken &token);

    using functrType = std::function<bool(StringToken &)>;

    std::unordered_map<ReadType, functrType> valueHandlers_;
    std::unordered_map<ReadType, functrType> tokenHandlers_;

    std::unordered_map<char, char> escapeLookup_;
    std::unordered_map<char, ReadType> tokenLookup_;

    DocumentStream docStream_;
    ReadType firstType_;
    std::vector<ErrorType> errors_;
};
