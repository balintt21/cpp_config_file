#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <stdint.h>
#include <string>
#include <map>
#include <optional>

class ConfigFile
{
private:
    std::map<std::string, std::string> mConfig;
public:
    ConfigFile(const std::string& path, const std::string& key_value_separator, const std::string& line_comment_symbol);

    const std::map<std::string, std::string>& data() const;

    std::string get(const std::string& key) const;

    std::optional<int64_t> getInteger(const std::string& key) const;

    std::optional<double> getDouble(const std::string& key) const;

    explicit operator bool () const { return !mConfig.empty(); }
};

#endif /* _CONFIG_FILE_H_ */

