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

#pragma pack(1)
struct USMLPasportTable
{
    char nameParametrs[12];
    char nameProperties[12];
    uint64_t dimension;
    uint32_t discreteness;
    float K0;
    float K1;
    uint32_t lengthArray;
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

    // Открытие файла
    std::string fileName = "ACTPA-06.П48_28-10-21.usm";

    std::ifstream file;

    file.open(fileName, std::ios::binary | std::ios::in | std::ios::ate);

    if (file.is_open() == false)
    {
        std::cerr << "Can't open file " << fileName << std::endl;
        return -1;
    }

    // Проверка размера
    size_t fileSize = file.tellg();
    file.seekg(0);

    if (fileSize < UINT32_MAX)
    {
        std::cerr << "Invalid file size " << fileSize << " B"
                  << "Maxsumum size is 4gb" << std::endl;
        return -1;
    }

    // Создание буфера
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

        if (fileParametrName.empty())
            continue;

        fileParametrName.append(".txt");

        std::fstream fileParametr;

        fileParametr.open(fileParametrName, std::ios::out);

        if (fileParametr.is_open() == false)
        {
            std::cerr << "Can't open file " << fileParametrName << std::endl;
            return -1;
        }

        auto Pasport = GetParametrPasport(startTableParametrs); // Извлечение таблицы

        auto dataSize = GetParametrData(fileParametr, Pasport, startParametrData); // Получение информации и размер, запись файла

        startTableParametrs += sizeof(USMLPasportTable);
        startParametrData += dataSize;

        fileParametr.close();
    }

    delete[] fileBuffer;

    return 0;
}

// Чтение заголовка
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

// Получение имя параметра
std::string &GetParametrName(char *buffer)
{
    USMLPasportTable *usmlpasportable = (USMLPasportTable *)buffer;
    std::string outputName = usmlpasportable->nameParametrs;
    return outputName;
}

// Получение паспорта параметров
USMLPasportTable GetParametrPasport(char *buffer)
{
    USMLPasportTable *usmlpasportable = (USMLPasportTable *)buffer;
    return *usmlpasportable;
}

// Извлечение, конвертация и запись данных в файл
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
