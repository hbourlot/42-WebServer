
/// @brief Get all the locations
/// @param path To split the locations
/// @return 
std::vector<std::string> getLocationsInPath(const std::string& path)
{
    std::vector<std::string> locations;
    std::string current;
    size_t i = 0;

    while (i < path.length())
    {
        if (path[i] == '/')
        {
            size_t next = path.find('/', i + 1);
            if (next == std::string::npos)
                break;

            current = path.substr(0, next);
            locations.push_back(current);
            i = next;
        }
        else
            i++;
    }

    return locations;
}

std::string getFileInPath(const std::string& path){
    size_t pos = path.rfind('.');
    if (pos == std::string::npos)
        return "";
    return path.substr(pos);
}

std::vector