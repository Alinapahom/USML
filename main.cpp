#include <fstream>
#include <iostream>
#include <sstream>

int main(int argc, const char **argv)
{

    std::string fileName = "ACTPA-06.П48_28-10-21.usm";

    std::ifstream file;

    file.open(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (file.is_open() == false)
    {
        std::cerr << "Can't open file " << fileName << std::endl;
        return -1;
    }

    size_t fileSize = file.tellg();
    file.seekg(0);

    if (fileSize < UINT32_MAX)
    {
        std::cerr << "Invalid file size " << fileSize << " B"
                  << "Maxsumum size is 4gb" << std::endl;
        return -1;
    }

    char *fileBuffer = new char[fileSize];













    delete[] fileBuffer;
    file.close();
    return 0;
}