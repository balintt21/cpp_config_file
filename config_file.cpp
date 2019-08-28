#include "config_file.h"

#include <stdio.h>
#include <vector>
#include <string_view>
#include <filesystem>

static std::string_view& trim_view_left(std::string_view& str)
{
    auto it = str.begin();
    size_t cnt = 0;
    for(; it != str.end(); ++it)
    {
        if( !std::isspace(*it) ) { break; }
        ++cnt;
    }
    if( it != str.end() ) { str.remove_prefix(cnt); }
    return str;
}

static std::string_view& trim_view_right(std::string_view& str)
{
    auto it = str.rbegin();
    size_t cnt = 0;
    for(; it != str.rend(); ++it)
    {
        if( !std::isspace(*it) ) { break; }
        ++cnt;
    }
    if( it != str.rend() ) { str.remove_suffix(cnt); }
    return str;
}

static std::string_view& trim_view(std::string_view& str)
{ return trim_view_left(trim_view_right(str)); }

static std::vector<std::string_view> split_view(const std::string_view& sv, const std::string& delim)
{
    std::vector<std::string_view> result;
    std::string_view strv_cpy(sv);
    size_t pos = strv_cpy.find(delim);
    if(pos == 0) { result.emplace_back(std::string_view()); }
    while( pos != std::string_view::npos )
    {
        if( pos > 0)
        { result.emplace_back(strv_cpy.substr(0, pos)); }
        strv_cpy = strv_cpy.substr(pos + 1);
        pos = strv_cpy.find(delim);
    }

    if( !strv_cpy.empty() )
    { result.emplace_back(strv_cpy); }

    return result;
}

ConfigFile::ConfigFile(const std::string& path, const std::string& key_value_separator, const std::string& line_comment_symbol) : mConfig()
{
	if( !path.empty() && std::filesystem::exists(std::filesystem::path(path)) )
	{
		std::error_code ec;
		auto file_size = std::filesystem::file_size(std::filesystem::path(path), ec);
		if( (ec.value() == 0) && (file_size > 0LL) )
		{
			std::vector<char> buffer(file_size, 0);
			FILE* file = fopen(path.c_str(), "r");
			if(file)
			{
				if( fread(buffer.data(), buffer.size(), 1, file) != 1 ) { buffer.clear(); }
				fclose(file);
			}

			if(!buffer.empty())
			{
				std::string_view buffer_view(buffer.data(),buffer.size());
				auto lines = split_view(buffer_view, "\n");
				for(auto& line : lines)
				{
					std::string_view view_copy(line);
					if( line.find(line_comment_symbol) != std::string_view::npos)
					{
						auto comment = split_view(view_copy, line_comment_symbol);
						if( comment.empty() || comment[0].empty() )
						{ continue; }
						else
						{
							auto it = comment[0].begin();
							for(; it != comment[0].end(); ++it)
							{
								if( !std::isspace(*it) ) { break; }
							}

							if( it == comment[0].end() ) { continue; }
							else						 { view_copy = comment[0]; }
						}
					}
					auto key_value = split_view(view_copy, key_value_separator);
					trim_view(key_value[0]);
					trim_view(key_value[1]);
					mConfig.emplace(std::string(key_value[0]), std::string(key_value[1]));
				}
			}
		}
	}
}

const std::map<std::string, std::string>& ConfigFile::data() const
{
	return mConfig;
}

std::string ConfigFile::get(const std::string& key) const
{
	const auto it = mConfig.find(key);
	if(it != mConfig.cend())
	{
		return it->second;
	}
	return "";
}

std::optional<int64_t> ConfigFile::getInteger(const std::string& key) const
{
	auto str = get(key);
	char* end = nullptr;
	auto value = std::strtoll(str.c_str(), &end, 10);
	if(str.c_str() != end)
	{
		return value;
	}
	return std::nullopt;
}

std::optional<double> ConfigFile::getDouble(const std::string& key) const
{
	auto str = get(key);
	char* end = nullptr;
	auto value = std::strtod(str.c_str(), &end);
	if(str.c_str() != end)
	{
		return value;
	}
	return std::nullopt;
}
