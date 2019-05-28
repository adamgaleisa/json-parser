#include "types.h"
#include "reader.h"

#include <iostream>
#include <iterator>
#include <fstream>

using std::cout;
using std::endl;

void replaceAll(std::string& doc, std::string const& tag, std::string const& value) {

    std::string::size_type pos = 0;
    for( ; (pos = doc.find(tag, pos)) != std::string::npos; ) {

        doc.erase(pos, tag.length());
        doc.insert(pos, value);

        pos += value.length(); //start find from last replace value
    }
};

bool fixErrors(std::string &doc, std::vector<ErrorType> const& errors) 
{
     for(auto it : errors) {
         
         if(it.type_ == 1) {
             cout<<it.message_<<", at: "<<(it.strTok_.begin_ - doc.begin())<<endl;
             replaceAll(doc,"\"\"","\"");
             cout<<doc<<endl;

         } else if(it.type_ == 2) {
             cout<<it.message_<<", at: "<<(it.strTok_.begin_ - doc.begin())<<endl;
             replaceAll(doc,"'","\"");
             cout<<doc<<endl;

         } else if(it.type_ == 3) {
             auto pos = it.strTok_.begin_ - doc.begin();
             cout<<it.message_<<", at: "<<pos<<endl;
             doc.insert(pos, 1, '0');
             cout<<doc<<endl;
         }
     }
     return false; 
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cerr << "Usage: "<<argv[0]<<" <infile>\n";
        return EXIT_FAILURE;
    }
    Reader reader;
    std::ifstream infile(argv[1]);
    std::string line;
    size_t counter{0};
    
    while(std::getline(infile, line)) {
         counter++;
         bool ok = reader.processLine(line);
         const auto &errors = reader.getErrors();
         if(ok && errors.empty() ) {
             cout<<"line #" <<counter<<", contains valid json"<<endl;
         } else if(ok)  {
             cout<<"line #" <<counter<<", contains, #"<<errors.size()<< " errors, atempting to fix them."<<endl;
             fixErrors(line,errors);
         }else
             cout<<"line #" <<counter<<", contains invalid json"<<endl;
    }
}
