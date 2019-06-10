
#include "reader.h"
#include <unordered_set>
#include <sstream>
#include <climits>

Reader::Reader()
{
    init();
}

bool Reader::processLine(std::string const &doc)
{
    docStream_.setDocument(doc);
    errors_.clear();
    return parse(doc);
}

bool Reader::parse(std::string const &doc)
{
    bool successful = getValue(true);
    return successful && (firstType_ == ReadType::BeginArray || firstType_ == ReadType::BeginObj);
}

bool Reader::getChars(StringToken &token)
{
    auto c = '\0';
    char sep = token.type_ == ReadType::String ? '"' : '\'';
    if (sep == '"' and (docStream_.end_ - docStream_.it_ > 1) and *docStream_.it_ == '"')
    {
        Log("Unescaped double quote in a string", token, 1);
        ++docStream_.it_;
    }
    while (docStream_.it_ != docStream_.end_)
    {
        c = docStream_.getC();
        if (c == '\\')
            docStream_.getC();
        else if (c == sep)
            break;
    }
    if (sep == '"' and (docStream_.end_ - docStream_.it_ > 1) and c == '"' and *docStream_.it_ == '"')
    {
        Log("Unescaped double quote in a string", token, 1);
        ++docStream_.it_;
        return true;
    }

    if(sep == '\'')
         Log("String enclosed by single quote instead of double quote", token,2);

    return c == sep;
}

bool Reader::getSubStr(const char *pattern, int patternLength)
{
    if (docStream_.end_ - docStream_.it_ < patternLength)
        return false;
    int index = patternLength;
    while (index--)
        if (docStream_.it_[index] != pattern[index])
            return false;
    docStream_.it_ += patternLength;
    return true;
}
bool Reader::getMissingFieldVal(StringToken &token)
{
    bool ok{true};
    if ((*docStream_.it_ >= '0' && *docStream_.it_ <= '9'))
    {
        Log("Missing field value", token, 3);
        readNumber(token);
    }
    else
    {
        ok = false;
    }
    return ok;
}
bool Reader::getToken(StringToken &token)
{
    docStream_.removeSpaces();
    token.begin_ = docStream_.it_;
    auto c = docStream_.getC();
    bool ok = true;
    auto it = tokenLookup_.find(c);
    if (it == tokenLookup_.end())
    {
        ok = false;
    }
    if(ok) {
        token.type_ = it->second;
    }

    if (ok)
    {
        auto it = tokenHandlers_.find(token.type_);
        if (it != tokenHandlers_.end())
        {
            ok = it->second(token);
        }
    }
    if (!ok)
        token.type_ = ReadType::ErrorEncounter;
    token.end_ = docStream_.it_;
    return true;
}
bool Reader::decodeStringWrap(StringToken &token)
{
    char sep = token.type_ == ReadType::String ? '"' : '\'';
    std::string decoded_string;
    if (!decodeString(token, decoded_string, sep))
        return false;
    return true;
}

bool Reader::decodeString(StringToken &token, std::string &decoded, char sep)
{
    if ((docStream_.end_ != docStream_.it_) and *docStream_.it_ == '"')
    {
        Log("Unescaped double quote in string", token, 1);
        ++docStream_.it_;
    }
    decoded.reserve(static_cast<size_t>(token.end_ - token.begin_ - 2));
    auto current = token.begin_ + 1; // skip '"'
    auto end = token.end_ - 1;       // do not include '"'
    auto c = '\0';
    while (current != end)
    {
        c = *current++;
        if (c == sep)
            break;
        else if (c == '\\')
        {
            if (current == end)
                return Log("Can't have empty escape", token);
            auto escape = *current++;
            auto it = escapeLookup_.find(escape);
            if (it == escapeLookup_.end())
            {
                return Log("error escape sequence ", token);
            }
            decoded += it->second;
        }
        else
        {
            decoded += c;
        }
    }
    if ((docStream_.end_ != docStream_.it_) and c == '"' and *docStream_.it_ == '"')
    {
        Log("Unescaped double quote in string", token, 1);
        ++docStream_.it_;
        return true;
    }
    return true;
}
bool Reader::decodeDouble(StringToken &token)
{
    double value = 0;
    std::string buffer(token.begin_, token.end_);
    std::istringstream is(buffer);
    if (!(is >> value))
        return Log("'" + std::string(token.begin_, token.end_) + "' is not a number.", token);
    return true;
}

bool Reader::decodeNumber(StringToken &token)
{
    auto current = token.begin_;
    bool isNegative = *current == '-';
    if (isNegative)
        ++current;

    int maxIntegerValue = isNegative ? INT_MIN : INT_MAX;

    int threshold = maxIntegerValue / 10;
    int value = 0;
    while (current < token.end_)
    {
        char c = *current++;
        if (c < '0' || c > '9')
            return decodeDouble(token);
        auto digit(static_cast<int>(c - '0'));
        if (value >= threshold)
        {
            if (value > threshold || current != token.end_ ||
                digit > maxIntegerValue % 10)
            {
                return decodeDouble(token);
            }
        }
        value = value * 10 + digit;
    }
    return true;
}

bool Reader::Log(const std::string &message, StringToken &token, int type)
{
    ErrorType info;
    info.strTok_ = token;
    info.message_ = message;
    info.type_ = type;
    errors_.push_back(info);
    return false;
}
bool Reader::getObject(StringToken &token)
{
    StringToken tokenName;
    std::string tag_name;
    std::unordered_set<std::string> seen; //hash for dupliate keys
    while (getToken(tokenName))
    {
        bool initialTokOk = true;
        if (!initialTokOk)
            break;
        if (tokenName.type_ == ReadType::EndObj && tag_name.empty()) // empty object
            return true;
        tag_name.clear();
        if (tokenName.type_ == ReadType::String or tokenName.type_ == ReadType::StringSingleQ)
        {
            char sep = tokenName.type_ == ReadType::String ? '"' : '\'';
            if (!decodeString(tokenName, tag_name, sep))
                return Log("Decode String error", tokenName);

            if (seen.count(tag_name))
            { //duplicate not allowed
                std::string msg = "Duplicate key: '" + tag_name + "'";
                return Log(msg, tokenName);
            }
        }
        else
        {
            break;
        }
        StringToken colon;
        if (!getToken(colon) || colon.type_ != ReadType::MemberSeparator)
        {
            return Log("Missing ':' after object member name", colon);
        }

        if (!getValue())
            return Log("error getValue", colon);

        StringToken comma;
        if (!getToken(comma) ||
            (comma.type_ != ReadType::EndObj && comma.type_ != ReadType::ArraySeperator))
        {
            return Log("Missing ',' or '}' in object declaration", comma);
        }
        if (comma.type_ == ReadType::EndObj)
            return true;
    }
    return Log("Missing '}' or object member name", tokenName);
}
bool Reader::getArray(StringToken &token)
{
    docStream_.removeSpaces();
    if (docStream_.it_ != docStream_.end_ && *docStream_.it_ == ']') // empty array
    {
        StringToken endArray;
        getToken(endArray);
        return true;
    }
    for (;;)
    {
        bool ok = getValue();
        if (!ok) // error already set
            return Log("error getValue", token);

        StringToken currentTok;
        ok = getToken(currentTok);

        bool badReadType = (currentTok.type_ != ReadType::ArraySeperator &&
                            currentTok.type_ != ReadType::EndArray);
        if (!ok || badReadType)
        {
            return Log("Missing ',' or ']' in array declaration", currentTok);
        }
        if (currentTok.type_ == ReadType::EndArray)
            break;
    }
    return true;
}
bool Reader::getValue(bool firstType)
{
    StringToken token;
    getToken(token);
    bool ok{true};
    if (firstType)
        firstType_ = token.type_;
    auto it = valueHandlers_.find(token.type_);
    if (it != valueHandlers_.end())
    {
        ok = it->second(token);
    }
    return ok;
}
bool Reader::readError(StringToken &token)
{
    /*
        StringToken currentTok;
        auto ok = getToken(currentTok);
        bool badReadType = (currentTok.type_ == ReadType::ArraySeperator &&
                            currentTok.type_ != ReadType::EndArray);
*/

    return Log("missing [value, array, object]", token, 4);
}

void Reader::init()
{
    escapeLookup_['"'] = '"';
    escapeLookup_['/'] = '/';
    escapeLookup_['\\'] = '\\';
    escapeLookup_['b'] = '\b';
    escapeLookup_['f'] = '\f';
    escapeLookup_['n'] = '\n';
    escapeLookup_['r'] = '\r';
    escapeLookup_['t'] = '\t';

    valueHandlers_[ReadType::BeginObj] = std::bind(&Reader::getObject, this, std::placeholders::_1);
    valueHandlers_[ReadType::BeginArray] = std::bind(&Reader::getArray, this, std::placeholders::_1);
    valueHandlers_[ReadType::Digit] = std::bind(&Reader::decodeNumber, this, std::placeholders::_1);
    valueHandlers_[ReadType::String] = std::bind(&Reader::decodeStringWrap, this, std::placeholders::_1);
    valueHandlers_[ReadType::StringSingleQ] = std::bind(&Reader::decodeStringWrap, this, std::placeholders::_1);
    valueHandlers_[ReadType::ArraySeperator] = std::bind(&Reader::readError, this, std::placeholders::_1);
    valueHandlers_[ReadType::EndObj] = std::bind(&Reader::readError, this, std::placeholders::_1);
    valueHandlers_[ReadType::EndArray] = std::bind(&Reader::readError, this, std::placeholders::_1);

    tokenLookup_['{'] = ReadType::BeginObj;
    tokenLookup_['}'] = ReadType::EndObj;
    tokenLookup_['['] = ReadType::BeginArray;
    tokenLookup_[']'] = ReadType::EndArray;
    tokenLookup_[','] = ReadType::ArraySeperator;
    tokenLookup_[':'] = ReadType::MemberSeparator;
    tokenLookup_['\''] = ReadType::StringSingleQ;
    tokenLookup_['"'] = ReadType::String;
    tokenLookup_['t'] = ReadType::True;
    tokenLookup_['f'] = ReadType::False;
    tokenLookup_['n'] = ReadType::JsonNull;
    tokenLookup_['.'] = ReadType::MissingFieldValue;
    tokenLookup_['0'] = ReadType::Digit;
    tokenLookup_['1'] = ReadType::Digit;
    tokenLookup_['2'] = ReadType::Digit;
    tokenLookup_['3'] = ReadType::Digit;
    tokenLookup_['4'] = ReadType::Digit;
    tokenLookup_['5'] = ReadType::Digit;
    tokenLookup_['6'] = ReadType::Digit;
    tokenLookup_['7'] = ReadType::Digit;
    tokenLookup_['8'] = ReadType::Digit;
    tokenLookup_['9'] = ReadType::Digit;
    tokenLookup_[0] = ReadType::EndRead;

    tokenHandlers_[ReadType::Digit] = std::bind(&Reader::readNumber, this, std::placeholders::_1);
    tokenHandlers_[ReadType::StringSingleQ] = std::bind(&Reader::getChars, this, std::placeholders::_1);
    tokenHandlers_[ReadType::String] = std::bind(&Reader::getChars, this, std::placeholders::_1);
    tokenHandlers_[ReadType::False] = std::bind(&Reader::getFalse, this, std::placeholders::_1);
    tokenHandlers_[ReadType::True] = std::bind(&Reader::getTrue, this, std::placeholders::_1);
    tokenHandlers_[ReadType::JsonNull] = std::bind(&Reader::getNull, this, std::placeholders::_1);
    tokenHandlers_[ReadType::MissingFieldValue] = std::bind(&Reader::getMissingFieldVal, this, std::placeholders::_1);
}
bool Reader::readNumber(StringToken &token)
{
    auto p = docStream_.it_;
    char c = '0';
    // integral part
    while (c >= '0' && c <= '9')
        c = (docStream_.it_ = p) < docStream_.end_ ? *p++ : '\0';
    // fractional part
    if (c == '.')
    {
        c = (docStream_.it_ = p) < docStream_.end_ ? *p++ : '\0';
        while (c >= '0' && c <= '9')
            c = (docStream_.it_ = p) < docStream_.end_ ? *p++ : '\0';
    }
    // exponential part
    if (c == 'e' || c == 'E')
    {
        c = (docStream_.it_ = p) < docStream_.end_ ? *p++ : '\0';
        if (c == '+' || c == '-')
            c = (docStream_.it_ = p) < docStream_.end_ ? *p++ : '\0';
        while (c >= '0' && c <= '9')
            c = (docStream_.it_ = p) < docStream_.end_ ? *p++ : '\0';
    }
    return true;
}
