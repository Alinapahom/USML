#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <cmath>

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
    char dimension[8];
    float discreteness;
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

uint16_t readHeader(char *, size_t);
std::string GetParametrName(char *buffer);
USMLPasportTable GetParametrPasport(char *buffer);
size_t GetParametrData(std::fstream &parametrFile, USMLPasportTable &parametrPropeties, char *buffer);
void GetPasportData(std::fstream &parametrFile, USMLPasportTable &parametrPropeties);

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

    if (fileSize > INT32_MAX)
    {
        std::cerr << "Invalid file size " << fileSize
                  << "Maxsumum size is 2gb" << std::endl;
        return -1;
    }

    // Создание буфера
    char *fileBuffer = new char[fileSize];

    file.read(fileBuffer, fileSize);
    file.close();

    uint16_t parametrCount = readHeader(fileBuffer, fileSize);

    char *startTableParametrs = &fileBuffer[32];                      // Начало таблицы паспортов
    char *startParametrData = &fileBuffer[32 + (58 * parametrCount)]; // Начало информационного пакета

    // Извлечение каждого параметра
    while (parametrCount--)
    {
        std::stringstream fileParametrName;

        auto parametrName = GetParametrName(startTableParametrs);

        fileParametrName << parametrCount << ".txt";

        std::fstream fileParametr;

        fileParametr.open(fileParametrName.str(), std::ios::out | std::ios::trunc);

        if (fileParametr.is_open() == false)
        {
            std::cerr << "Can't open file " << fileParametrName.str() << std::endl;
            return -1;
        }

        std::fstream pasportFile;

        std::stringstream filePasportName;

        filePasportName << "Паспорт" << parametrCount << ".txt";

        pasportFile.open(filePasportName.str(), std::ios::out | std::ios::trunc);

        if (pasportFile.is_open() == false)
        {
            std::cerr << "Can't open file " << filePasportName.str() << std::endl;
            return -1;
        }

        auto Pasport = GetParametrPasport(startTableParametrs); // Извлечение таблицы

        GetPasportData(pasportFile, Pasport);

        pasportFile.close();

        auto dataSize = GetParametrData(fileParametr, Pasport, startParametrData); // Получение информации и размер, запись файла

        if (dataSize == 0)
            continue;

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
    const uint32_t fileMarker = 2341245331;

    USMLHeader *usmlheader = (USMLHeader *)buffer; // Преобразование типа
    if (usmlheader->marker != fileMarker)
    {
        std::cerr << "Marker not found" << std::endl;
        return -1;
    }
    return usmlheader->countParamrters;
}

// Получение имя параметра
std::string GetParametrName(char *buffer)
{
    USMLPasportTable *usmlpasportable = (USMLPasportTable *)buffer;
    std::string outputName;
    outputName.append(usmlpasportable->nameParametrs, 12);
    return outputName;
}

// Получение паспорта параметров
USMLPasportTable GetParametrPasport(char *buffer)
{
    USMLPasportTable *usmlpasportable = (USMLPasportTable *)buffer;

    // usmlpasportable->dimension = _OSSwapInt64(usmlpasportable->dimension);
    // usmlpasportable->discreteness = _OSSwapInt32(usmlpasportable->discreteness);
    // usmlpasportable->lengthArray = _OSSwapInt32(usmlpasportable->lengthArray);

    return *usmlpasportable;
}

// Извлечение, конвертация и запись данных в файл
size_t GetParametrData(std::fstream &parametrFile, USMLPasportTable &parametrPropeties, char *buffer)
{
    if (parametrFile.is_open() == false)
    {
        std::cerr << "Error create file" << std::endl;
        return 0;
    }

    size_t dataParametrSize = 0;

    switch (parametrPropeties.format)
    {
    case dataChar:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            auto dataX = parametrPropeties.K1 * (buffer[i] - parametrPropeties.K0);
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0)
                continue;
            parametrFile << dataX << " " << dataY << std::endl;
            ++dataParametrSize;
        }
        break;

    case dataShort:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            auto dataX = parametrPropeties.K1 * ((((short *)buffer)[i]) - parametrPropeties.K0);
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0)
                continue;
            parametrFile << dataX << " " << dataY << std::endl;
            dataParametrSize += 2;
        }
        break;
    case dataLong:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            auto dataX = parametrPropeties.K1 * ((((long *)buffer)[i]) - parametrPropeties.K0);
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0)
                continue;
            parametrFile << dataX << " " << dataY << std::endl;
            dataParametrSize += 4;
        }
        break;
    case dataFloat:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            parametrFile.precision(8);
            auto dataX = parametrPropeties.K1 * ((((float *)buffer)[i]) - parametrPropeties.K0);
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0 || std::isnan(dataX))
                continue;
            parametrFile << dataX << " " << dataY << std::endl;
            dataParametrSize += 4;
        }
        break;
    case dataDouble:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            parametrFile.precision(8);
            auto dataX = parametrPropeties.K1 * ((((double *)buffer)[i]) - parametrPropeties.K0);
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0 || std::isnan(dataX))
                continue;
            parametrFile << dataX << " " << dataY << std::endl;
            dataParametrSize += 8;
        }
        break;
    default:
        break;
    };
    return dataParametrSize + 2;
}

void GetPasportData(std::fstream &parametrFile, USMLPasportTable &parametrPropeties)
{
    parametrFile << "Имя параметра: " << std:: string (parametrPropeties.nameParametrs, 12) << std::endl;
    parametrFile << "Имя характеристики: " << std:: string (parametrPropeties.nameProperties, 12) << std::endl;
    parametrFile << "Размерность: " << std:: string (parametrPropeties.dimension, 8) << std::endl;
    parametrFile << "Дискретность: " << parametrPropeties.discreteness << std::endl;
    parametrFile << "K0: " << parametrPropeties.K0 << std::endl;
    parametrFile << "K1: " << parametrPropeties.K1 << std::endl;
    parametrFile << "Длина массива: " << parametrPropeties.lengthArray << std::endl;
    parametrFile << "Формат: " << int (parametrPropeties.format) << std::endl;
    parametrFile << "Тн: " << parametrPropeties.Tn << std::endl;
    parametrFile << "Тк: " << parametrPropeties.Tk << std::endl;
    parametrFile << "Резерв: " << int (parametrPropeties.reserve) << std::endl;
}
