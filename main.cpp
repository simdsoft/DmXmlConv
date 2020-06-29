#include "pugixml/pugixml.hpp"
#define TINYDIR_DISABLE_TCHAR_ON_WIN32 1
#include "tinydir/tinydir.h"
#include <string>
#include <algorithm>
#include <functional>

static bool is_directory_exists(const std::string& path) {

    WIN32_FILE_ATTRIBUTE_DATA attr = { 0 };
    GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attr);
    return (attr.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
}

static void list_files(const std::string& dirPath,
    const std::function<bool(tinydir_file&)>& callback, bool recursively = false)
{
    if (is_directory_exists(dirPath))
    {
        tinydir_dir dir;
        std::string fullpathstr = dirPath;

        if (tinydir_open(&dir, &fullpathstr[0]) != -1)
        {
            while (dir.has_next)
            {
                tinydir_file file;
                if (tinydir_readfile(&dir, &file) == -1)
                {
                    // Error getting file
                    break;
                }
                std::string fileName = file.name;

                if (fileName != "." && fileName != "..")
                {
                    if (callback(file)) break;
                    if (file.is_dir && recursively)
                        list_files(file.path, callback, recursively);
                }

                if (tinydir_next(&dir) == -1)
                {
                    // Error getting next file
                    break;
                }
            }
        }
        tinydir_close(&dir);
    }
}

// convert DmXml-1.0 to DmXml-2.0
static void convert(pugi::xml_node curNode) {
	if (curNode) {
		std::string elementName = curNode.name();
		std::transform(elementName.begin(), elementName.end(), elementName.begin(), &::tolower);
		if (elementName != "static")
			curNode.set_name(elementName.c_str());
		else
			curNode.set_name("label");
	}
	for (pugi::xml_node child : curNode)
		convert(child);
}

int main(int argc, char** argv) {

	if (argc < 2) { return ERROR_INVALID_PARAMETER; }

    list_files(argv[1], [](tinydir_file& f) {
        if (_stricmp(f.extension, "xml") == 0) {
            pugi::xml_document doc;
            pugi::xml_parse_result status = doc.load_file(f.path, pugi::parse_default | pugi::parse_comments, pugi::xml_encoding::encoding_utf8);
            if (status) {
                convert(doc.document_element());
                doc.save_file(f.path);
                printf("convert %s succeed.\n", f.path);
            }
            else {
                printf("convert %s failed, %s!\n", f.path, status.description());
            }
        }
        return false; 
        }, true);

	return 0;
}

