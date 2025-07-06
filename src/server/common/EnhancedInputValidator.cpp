#include "server/common/EnhancedInputValidator.hpp"
#include "server/common/ErrorCodes.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iostream>

// SQL注入关键词列表
const vector<string> EnhancedInputValidator::SQL_INJECTION_KEYWORDS = {
    "union", "select", "insert", "update", "delete", "drop", "create", "alter",
    "exec", "execute", "sp_", "xp_", "script", "javascript", "vbscript",
    "onload", "onerror", "onclick", "onmouseover", "onfocus", "onblur",
    "eval", "expression", "applet", "object", "embed", "form", "iframe",
    "meta", "link", "style", "title", "base", "bgsound", "blink",
    "body", "frame", "frameset", "head", "html", "layer", "ilayer",
    "input", "button", "textarea", "select", "option", "optgroup",
    "fieldset", "legend", "label", "table", "tr", "td", "th",
    "--", "/*", "*/", "@@", "char", "nchar", "varchar", "nvarchar",
    "binary", "varbinary", "text", "ntext", "image", "decimal", "numeric",
    "float", "real", "datetime", "smalldatetime", "timestamp", "uniqueidentifier",
    "sql", "money", "smallmoney", "bit", "cursor", "table", "database",
    "schema", "procedure", "function", "trigger", "view", "index",
    "constraint", "default", "rule", "user", "role", "permission",
    "grant", "revoke", "deny", "backup", "restore", "checkpoint",
    "dbcc", "bulk", "openquery", "openrowset", "opendatasource"
};

// XSS攻击模式
const vector<string> EnhancedInputValidator::XSS_PATTERNS = {
    "<script", "</script>", "javascript:", "vbscript:", "onload=", "onerror=",
    "onclick=", "onmouseover=", "onfocus=", "onblur=", "onchange=", "onsubmit=",
    "<iframe", "<object", "<embed", "<applet", "<meta", "<link",
    "<style", "<img", "<svg", "<video", "<audio", "<source",
    "eval(", "setTimeout(", "setInterval(", "Function(", "alert(",
    "confirm(", "prompt(", "document.", "window.", "location.",
    "cookie", "localStorage", "sessionStorage", "innerHTML", "outerHTML",
    "expression(", "url(", "@import", "behavior:", "binding:",
    "data:text/html", "data:text/javascript", "data:application/javascript"
};

// 命令注入关键词
const vector<string> EnhancedInputValidator::COMMAND_INJECTION_KEYWORDS = {
    ";", "|", "&", "`", "$(", "${", "&&", "||", "<", ">", ">>",
    "cat", "ls", "pwd", "cd", "rm", "mv", "cp", "chmod", "chown",
    "ps", "kill", "killall", "top", "netstat", "ifconfig", "ping",
    "wget", "curl", "nc", "telnet", "ssh", "ftp", "scp", "rsync",
    "tar", "gzip", "gunzip", "zip", "unzip", "find", "grep", "awk",
    "sed", "sort", "uniq", "head", "tail", "wc", "tr", "cut",
    "echo", "printf", "env", "export", "set", "unset", "alias",
    "history", "which", "whereis", "locate", "file", "stat", "du",
    "df", "mount", "umount", "fdisk", "mkfs", "fsck", "crontab",
    "systemctl", "service", "chkconfig", "iptables", "ufw", "firewall"
};

// 危险文件扩展名
const unordered_set<string> EnhancedInputValidator::DANGEROUS_EXTENSIONS = {
    ".exe", ".bat", ".cmd", ".com", ".pif", ".scr", ".vbs", ".vbe",
    ".js", ".jse", ".wsf", ".wsh", ".msi", ".msp", ".hta", ".cpl",
    ".scf", ".lnk", ".inf", ".reg", ".dll", ".ocx", ".sys", ".drv",
    ".php", ".asp", ".aspx", ".jsp", ".py", ".pl", ".rb", ".sh",
    ".bash", ".zsh", ".fish", ".ps1", ".psm1", ".psd1", ".ps1xml"
};

bool EnhancedInputValidator::containsSQLInjection(const string& input) {
    string lowerInput = toLowerCase(input);
    string cleanInput = removeWhitespace(lowerInput);
    
    // 检查SQL注释模式
    if (cleanInput.find("--") != string::npos || 
        cleanInput.find("/*") != string::npos ||
        cleanInput.find("*/") != string::npos) {
        return true;
    }
    
    // 检查多个单引号或双引号
    size_t singleQuoteCount = 0;
    size_t doubleQuoteCount = 0;
    for (char c : input) {
        if (c == '\'') singleQuoteCount++;
        if (c == '"') doubleQuoteCount++;
    }
    if (singleQuoteCount > 2 || doubleQuoteCount > 2) {
        return true;
    }
    
    // 检查SQL函数调用模式
    vector<string> sqlFunctions = {
        "concat(", "substring(", "ascii(", "char(", "len(", "length(",
        "upper(", "lower(", "ltrim(", "rtrim(", "replace(", "reverse(",
        "count(", "sum(", "avg(", "min(", "max(", "group_concat(",
        "version(", "user(", "database(", "schema(", "table_name(",
        "column_name(", "information_schema", "sys.", "mysql.", "pg_"
    };
    
    for (const string& func : sqlFunctions) {
        if (cleanInput.find(func) != string::npos) {
            return true;
        }
    }
    
    // 特殊检查：排除邮箱地址中的user关键词
    if (input.find('@') != string::npos && input.find('.') != string::npos) {
        // 可能是邮箱地址，跳过user关键词检查
        vector<string> nonUserKeywords;
        for (const string& keyword : SQL_INJECTION_KEYWORDS) {
            if (keyword != "user") {
                nonUserKeywords.push_back(keyword);
            }
        }
        if (containsKeywords(lowerInput, nonUserKeywords)) {  // 使用lowerInput而不是cleanInput
            return true;
        }
    } else {
        // 检查SQL注入关键词
        if (containsKeywords(lowerInput, SQL_INJECTION_KEYWORDS)) {  // 使用lowerInput而不是cleanInput
            return true;
        }
    }
    
    // 特殊检查OR语句模式
    if (cleanInput.find("or") != string::npos) {
        // 检查是否是SQL注入中的OR语句
        if (cleanInput.find("'or") != string::npos || 
            cleanInput.find("or'") != string::npos ||
            cleanInput.find("or1=1") != string::npos ||
            cleanInput.find("or'1'='1") != string::npos) {
            return true;
        }
    }
    
    return false;
}

bool EnhancedInputValidator::containsXSS(const string& input) {
    string lowerInput = toLowerCase(input);
    return matchesPatterns(lowerInput, XSS_PATTERNS);
}

bool EnhancedInputValidator::containsCommandInjection(const string& input) {
    string lowerInput = toLowerCase(input);
    
    // 特殊字符列表 - 使用简单的子字符串匹配
    vector<string> specialChars = {
        ";", "|", "&", "`", "$(", "${", "&&", "||", "<", ">", ">>"
    };
    
    // 命令列表 - 使用单词边界检查
    vector<string> commands = {
        "cat", "ls", "pwd", "cd", "rm", "mv", "cp", "chmod", "chown",
        "ps", "kill", "killall", "top", "netstat", "ifconfig", "ping",
        "wget", "curl", "nc", "telnet", "ssh", "ftp", "scp", "rsync",
        "tar", "gzip", "gunzip", "zip", "unzip", "find", "grep", "awk",
        "sed", "sort", "uniq", "head", "tail", "wc", "tr", "cut",
        "echo", "printf", "env", "export", "set", "unset", "alias",
        "history", "which", "whereis", "locate", "file", "stat", "du",
        "df", "mount", "umount", "fdisk", "mkfs", "fsck", "crontab",
        "systemctl", "service", "chkconfig", "iptables", "ufw", "firewall"
    };
    
    // 检查特殊字符
    for (const string& specialChar : specialChars) {
        if (lowerInput.find(specialChar) != string::npos) {
            return true;
        }
    }
    
    // 检查命令（使用单词边界）
    if (containsKeywords(lowerInput, commands)) {
        return true;
    }
    
    return false;
}

string EnhancedInputValidator::advancedSanitize(const string& input, bool allowHtml) {
    string result = input;
    
    if (!allowHtml) {
        // HTML实体编码
        size_t pos = 0;
        while ((pos = result.find("<", pos)) != string::npos) {
            result.replace(pos, 1, "&lt;");
            pos += 4;
        }
        pos = 0;
        while ((pos = result.find(">", pos)) != string::npos) {
            result.replace(pos, 1, "&gt;");
            pos += 4;
        }
        pos = 0;
        while ((pos = result.find("&", pos)) != string::npos) {
            if (result.substr(pos, 4) != "&lt;" && result.substr(pos, 4) != "&gt;") {
                result.replace(pos, 1, "&amp;");
                pos += 5;
            } else {
                pos++;
            }
        }
    }
    
    // 移除或转义危险字符
    size_t pos = 0;
    while ((pos = result.find("\"", pos)) != string::npos) {
        result.replace(pos, 1, "&quot;");
        pos += 6;
    }
    pos = 0;
    while ((pos = result.find("'", pos)) != string::npos) {
        result.replace(pos, 1, "&#39;");
        pos += 5;
    }
    
    // 移除控制字符
    result.erase(remove_if(result.begin(), result.end(), 
        [](char c) { return c < 32 && c != '\t' && c != '\n' && c != '\r'; }), 
        result.end());
    
    return result;
}

ErrorCode EnhancedInputValidator::validateSQLParameter(const string& param, const string& paramType) {
    if (param.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    if (containsSQLInjection(param)) {
        return ErrorCode::INPUT_VALIDATION_SQL_INJECTION;
    }
    
    if (paramType == "int") {
        for (char c : param) {
            if (!isdigit(c) && c != '-' && c != '+') {
                return ErrorCode::INPUT_VALIDATION_INVALID_FORMAT;
            }
        }
    } else if (paramType == "email") {
        return validateEmail(param);
    } else if (paramType == "phone") {
        return validatePhone(param);
    } else if (paramType == "url") {
        return validateURL(param);
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validateEmail(const string& email) {
    if (email.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    // 先检查恶意内容，优先级高于格式检查
    if (containsXSS(email) || containsSQLInjection(email)) {
        return ErrorCode::INPUT_VALIDATION_MALICIOUS_INPUT;
    }
    
    regex emailPattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!regex_match(email, emailPattern)) {
        return ErrorCode::INPUT_VALIDATION_INVALID_FORMAT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validatePhone(const string& phone) {
    if (phone.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    // 允许数字、空格、连字符、括号和加号
    regex phonePattern(R"(^[\+]?[0-9\s\-\(\)]{10,20}$)");
    if (!regex_match(phone, phonePattern)) {
        return ErrorCode::INPUT_VALIDATION_INVALID_FORMAT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validateIPAddress(const string& ip) {
    if (ip.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    // IPv4格式验证
    regex ipv4Pattern(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    if (regex_match(ip, ipv4Pattern)) {
        return ErrorCode::SUCCESS;
    }
    
    // IPv6格式验证（简化版）
    regex ipv6Pattern(R"(^(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$)");
    if (regex_match(ip, ipv6Pattern)) {
        return ErrorCode::SUCCESS;
    }
    
    return ErrorCode::INPUT_VALIDATION_INVALID_FORMAT;
}

ErrorCode EnhancedInputValidator::validateURL(const string& url) {
    if (url.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    regex urlPattern(R"(^https?://[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}(?:/[^\s]*)?$)");
    if (!regex_match(url, urlPattern)) {
        return ErrorCode::INPUT_VALIDATION_INVALID_FORMAT;
    }
    
    if (containsXSS(url) || containsSQLInjection(url)) {
        return ErrorCode::INPUT_VALIDATION_MALICIOUS_INPUT;
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validateLength(const string& input, size_t maxLength, const string& fieldName) {
    (void)fieldName; // 消除未使用参数警告
    if (input.length() > maxLength) {
        return ErrorCode::INPUT_VALIDATION_TOO_LONG;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validateFilename(const string& filename) {
    if (filename.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    // 检查危险字符
    string dangerousChars = "<>:\"/|?*";
    for (char c : dangerousChars) {
        if (filename.find(c) != string::npos) {
            return ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS;
        }
    }
    
    // 检查危险扩展名
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != string::npos) {
        string extension = toLowerCase(filename.substr(dotPos));
        if (DANGEROUS_EXTENSIONS.find(extension) != DANGEROUS_EXTENSIONS.end()) {
            return ErrorCode::INPUT_VALIDATION_DANGEROUS_FILE_TYPE;
        }
    }
    
    // 检查保留文件名（Windows）
    vector<string> reservedNames = {
        "con", "prn", "aux", "nul", "com1", "com2", "com3", "com4",
        "com5", "com6", "com7", "com8", "com9", "lpt1", "lpt2",
        "lpt3", "lpt4", "lpt5", "lpt6", "lpt7", "lpt8", "lpt9"
    };
    
    string baseName = filename;
    if (dotPos != string::npos) {
        baseName = filename.substr(0, dotPos);
    }
    baseName = toLowerCase(baseName);
    
    for (const string& reserved : reservedNames) {
        if (baseName == reserved) {
            return ErrorCode::INPUT_VALIDATION_RESERVED_NAME;
        }
    }
    
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validatePath(const string& path) {
    if (path.empty()) {
        return ErrorCode::INPUT_VALIDATION_EMPTY_FIELD;
    }
    
    // 检查目录遍历攻击
    if (path.find("..") != string::npos) {
        return ErrorCode::INPUT_VALIDATION_PATH_TRAVERSAL;
    }
    
    // 检查绝对路径（可能不安全）
    if (path[0] == '/' || (path.length() > 1 && path[1] == ':')) {
        return ErrorCode::INPUT_VALIDATION_ABSOLUTE_PATH;
    }
    
    return ErrorCode::SUCCESS;
}

bool EnhancedInputValidator::isInWhitelist(const string& input, const unordered_set<string>& whitelist) {
    return whitelist.find(input) != whitelist.end();
}

bool EnhancedInputValidator::isInBlacklist(const string& input, const unordered_set<string>& blacklist) {
    return blacklist.find(input) != blacklist.end();
}

ErrorCode EnhancedInputValidator::validatePasswordStrength(const string& password) {
    if (password.length() < 8) {
        return ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_SHORT;
    }
    
    if (password.length() > 128) {
        return ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_LONG;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    
    int complexity = hasUpper + hasLower + hasDigit + hasSpecial;
    if (complexity < 3) {
        return ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_WEAK;
    }
    
    // 检查常见弱密码
    vector<string> commonPasswords = {
        "password", "123456", "123456789", "qwerty", "abc123",
        "password123", "admin", "letmein", "welcome", "monkey",
        "1234567890", "password1", "123123", "111111", "000000"
    };
    
    string lowerPassword = toLowerCase(password);
    for (const string& common : commonPasswords) {
        if (lowerPassword == common) {
            return ErrorCode::INPUT_VALIDATION_PASSWORD_TOO_COMMON;
        }
    }
    
    return ErrorCode::SUCCESS;
}

bool EnhancedInputValidator::containsExcessiveRepeats(const string& input, int maxRepeats) {
    if (input.length() < static_cast<size_t>(maxRepeats)) return false;
    
    int count = 1;
    char lastChar = input[0];
    
    for (size_t i = 1; i < input.length(); i++) {
        if (input[i] == lastChar) {
            count++;
            if (count > maxRepeats) {
                return true;
            }
        } else {
            count = 1;
            lastChar = input[i];
        }
    }
    
    return false;
}

ErrorCode EnhancedInputValidator::validateNumberRange(int value, int min, int max) {
    if (value < min || value > max) {
        return ErrorCode::INPUT_VALIDATION_OUT_OF_RANGE;
    }
    return ErrorCode::SUCCESS;
}

ErrorCode EnhancedInputValidator::validateCharacterSet(const string& input, const string& allowedChars) {
    for (char c : input) {
        if (allowedChars.find(c) == string::npos) {
            return ErrorCode::INPUT_VALIDATION_INVALID_CHARACTERS;
        }
    }
    return ErrorCode::SUCCESS;
}

bool EnhancedInputValidator::containsKeywords(const string& input, const vector<string>& keywords) {
    string lowerInput = toLowerCase(input);
    for (const string& keyword : keywords) {
        string lowerKeyword = toLowerCase(keyword);
        size_t pos = lowerInput.find(lowerKeyword);
        while (pos != string::npos) {
            // 检查是否为完整单词（前后都是非字母数字字符或字符串边界）
            bool isWordStart = (pos == 0 || !isalnum(lowerInput[pos - 1]));
            bool isWordEnd = (pos + lowerKeyword.length() >= lowerInput.length() || 
                             !isalnum(lowerInput[pos + lowerKeyword.length()]));
            
            if (isWordStart && isWordEnd) {
                return true;
            }
            
            pos = lowerInput.find(lowerKeyword, pos + 1);
        }
    }
    return false;
}

bool EnhancedInputValidator::matchesPatterns(const string& input, const vector<string>& patterns) {
    for (const string& pattern : patterns) {
        if (input.find(pattern) != string::npos) {
            return true;
        }
    }
    return false;
}

string EnhancedInputValidator::toLowerCase(const string& str) {
    string result = str;
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

string EnhancedInputValidator::removeWhitespace(const string& str) {
    string result;
    for (char c : str) {
        if (!isspace(c)) {
            result += c;
        }
    }
    return result;
}