#include <iostream>
#include <string>
#include "../include/server/common/EnhancedInputValidator.hpp"

using namespace std;

int main() {
    string testInput = "UNION select * from users";
    cout << "Testing input: \"" << testInput << "\"" << endl;
    
    bool result = EnhancedInputValidator::containsSQLInjection(testInput);
    cout << "Result: " << (result ? "SQL Injection Detected" : "No SQL Injection") << endl;
    
    // 测试各个组件
    cout << "\n=== Debug Information ===" << endl;
    
    // 测试toLowerCase
    string lowerInput = testInput;
    transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
    cout << "Lower case: \"" << lowerInput << "\"" << endl;
    
    // 测试removeWhitespace
    string cleanInput = lowerInput;
    cleanInput.erase(remove_if(cleanInput.begin(), cleanInput.end(), ::isspace), cleanInput.end());
    cout << "Clean input: \"" << cleanInput << "\"" << endl;
    
    // 手动测试关键词检查
    vector<string> testKeywords = {"union", "select", "from"};
    for (const string& keyword : testKeywords) {
        size_t pos = lowerInput.find(keyword);
        cout << "Keyword \"" << keyword << "\": ";
        if (pos != string::npos) {
            cout << "Found at position " << pos;
            // 检查单词边界
            bool isWordStart = (pos == 0 || !isalnum(lowerInput[pos - 1]));
            bool isWordEnd = (pos + keyword.length() >= lowerInput.length() || 
                             !isalnum(lowerInput[pos + keyword.length()]));
            cout << " (word boundaries: start=" << isWordStart << ", end=" << isWordEnd << ")";
        } else {
            cout << "Not found";
        }
        cout << endl;
    }
    
    return 0;
}