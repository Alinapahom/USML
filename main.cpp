#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>

uint16_t readHeader(char *, size_t);

struct USMLHeader
{
    uint32_t marker;
    char nameProduct[8];
    char nameTest[8];
    char date[8];
    uint16_t countParamrters;
    uint16_t reserve;
};

struct USMLPasportTable
{
    char nameParametrs[12];
    char nameProperties[12];
    uint64_t dimension;
    uint32_t discreteness;
    float K0;
    float K1;
    uint16_t lengthArray;
    uint8_t format;
    float Tn;
    float Tk;
    uint8_t reserve;
};

enum USMLFormatData
{
    dataChar = 1,
    dataShort = 2,
    dataLong = 3,
    dataFloat = 4,
    dataDouble = 8
};

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

    const int fileMarker = 2341245331;

    file.read(fileBuffer, fileSize);
    file.close();
    uint16_t parametrCount = readHeader(fileBuffer, fileSize);

    char *startTableParametrs = &fileBuffer[32];                      // Начало таблицы паспортов
    char *startParametrData = &fileBuffer[32 + (58 * parametrCount)]; // Начало информационного пакета

    // Извлечение каждого параметра
    while (parametrCount)
    {
        std::string fileParametrName = GetParametrName(startTableParametrs);
        std::ofstream fileParametr;

        fileParametr.open(fileParametrName);

        if (fileParametr.is_open() == false)
        {
            std::cerr << "Can't open file " << fileParametrName << std::endl;
            return -1;
        }
    }

    delete[] fileBuffer;

    return 0;
}

uint16_t readHeader(char *buffer, size_t size)
{
    const int fileMarker = 2341245331;

    USMLHeader *usmlheader = (USMLHeader *)buffer; // Преобразование типа
    if (usmlheader->marker != fileMarker)
    {
        std::cerr << "Marker not found" << std::endl;
        return -1;
    }
    return usmlheader->countParamrters;
}

std::string &GetParametrName(char *buffer)
{
    USMLPasportTable *usmlpasportable = (USMLPasportTable *)buffer;
    std::string outputName = usmlpasportable->nameParametrs;
    return outputName;
}

USMLPasportTable GetParametrPasport(char *buffer)
{
    USMLPasportTable *usmlpasportable = (USMLPasportTable *)buffer;
    return *usmlpasportable;
}

size_t GetParametrData(std::fstream &parametrFile, USMLPasportTable &parametrPropeties, char *buffer)
{
    if (parametrFile.is_open() == false)
    {
        std::cerr << "Error create file" << std::endl;
        return;
    }

    size_t dataParametrSize = 0;

    std::stringstream dataStream;

    switch (parametrPropeties.format)
    {
    case dataChar:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            dataStream << buffer[i];
            parametrFile << dataStream.str() << "\n\r";
            ++dataParametrSize;
        }
        break;

    case dataShort:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        { 
            dataStream << ((short *)buffer)[i]; // Конвертация байтового массива в 16-битный
            parametrFile << dataStream.str() << "\n\r";
            dataParametrSize += 2;
        }
        break;
    case dataLong:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            dataStream << ((long *)buffer)[i];
            parametrFile << dataStream.str() << "\n\r";
            dataParametrSize += 4;
        }
        break;
    case dataFloat:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            dataStream.precision(4);
            dataStream << ((float *)buffer)[i];
            parametrFile << dataStream.str() << "\n\r";
            dataParametrSize += 4;
        }
        break;
    case dataDouble:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            dataStream.precision(4);
            dataStream << ((double *)buffer)[i];
            parametrFile << dataStream.str() << "\n\r";
            dataParametrSize += 8;
        }
        break;
    default:
        break;
    };
    return dataParametrSize + 2;
}
