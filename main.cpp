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
static void cp2utf1(char *out, const char *in);
std::string cp2utf(std::string s);

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

        fileParametrName << parametrName << ".txt";

        std::fstream fileParametr;

        fileParametr.open(fileParametrName.str(), std::ios::out | std::ios::trunc);

        if (fileParametr.is_open() == false)
        {
            std::cerr << "Can't open file " << fileParametrName.str() << std::endl;
            return -1;
        }

        std::fstream pasportFile;

        std::stringstream filePasportName;

        filePasportName << "Паспорт " << parametrName << ".txt";

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
    std::string outputName, cpName;
    cpName.append(usmlpasportable->nameParametrs, 12);
    outputName = cp2utf(cpName); 
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

            auto dataX = buffer[i];
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0)
                parametrFile << dataY << std::endl;
            else
                parametrFile << dataY << " " << dataX << std::endl;
            ++dataParametrSize;
        }
        break;

    case dataShort:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            auto dataX = ((short *)buffer)[i];
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0)
                parametrFile << dataY << std::endl;
            else
                parametrFile << dataY << " " << dataX << std::endl;
            dataParametrSize += 2;
        }
        break;
    case dataLong:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            auto dataX = ((long *)buffer)[i];
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0 || std::isnan(dataX))
                parametrFile << dataY << std::endl;
            else
                parametrFile << dataY << " " << dataX << std::endl;
            dataParametrSize += 4;
        }
        break;
    case dataFloat:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            parametrFile.precision(8);
            auto dataX = ((float *)buffer)[i];
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0 || std::isnan(dataX))
                parametrFile << dataY << std::endl;
            else
                parametrFile << dataY << " " << dataX << std::endl;
            dataParametrSize += 4;
        }
        break;
    case dataDouble:
        for (size_t i = 0; i < parametrPropeties.lengthArray; ++i)
        {
            parametrFile.precision(8);
            auto dataX = ((double *)buffer)[i];
            auto dataY = parametrPropeties.Tn + (i * (parametrPropeties.discreteness));
            if (dataX == 0 || std::isnan(dataX))
                parametrFile << dataY << std::endl;
            else
                parametrFile << dataY << " " << dataX << std::endl;
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
    parametrFile << "\xc8\xec\xff \xef\xe0\xf0\xe0\xec\xe5\xf2\xf0\xe0: " << std::string(parametrPropeties.nameParametrs, 12) << std::endl;
    parametrFile << "\xc8\xec\xff \xf5\xe0\xf0\xe0\xea\xf2\xe5\xf0\xe8\xf1\xf2\xe8\xea\xe8: " << std::string(parametrPropeties.nameProperties, 12) << std::endl;
    parametrFile << "\xd0\xe0\xe7\xec\xe5\xf0\xed\xee\xf1\xf2\xfc: " << std::string(parametrPropeties.dimension, 8) << std::endl;
    parametrFile << "\xc4\xe8\xf1\xea\xf0\xe5\xf2\xed\xee\xf1\xf2\xfc: " << parametrPropeties.discreteness << std::endl;
    parametrFile << "\xca 0: " << parametrPropeties.K0 << std::endl;
    parametrFile << "\xca 1: " << parametrPropeties.K1 << std::endl;
    parametrFile << "\xc4\xeb\xe8\xed\xe0 \xec\xe0\xf1\xf1\xe8\xe2\xe0: " << parametrPropeties.lengthArray << std::endl;
    parametrFile << "\xd4\xee\xf0\xec\xe0\xf2: " << int(parametrPropeties.format) << std::endl;
    parametrFile << "\xd2\xed: " << parametrPropeties.Tn << std::endl;
    parametrFile << "\xd2\xea: " << parametrPropeties.Tk << std::endl;
    parametrFile << "\xd0\xe5\xe7\xe5\xf0\xe2: " << int(parametrPropeties.reserve) << std::endl;
}

// Кодировка из cp1251 в utf
static void cp2utf1(char *out, const char *in)
{
    static const int table[128] = {
        0x82D0, 0x83D0, 0x9A80E2, 0x93D1, 0x9E80E2, 0xA680E2, 0xA080E2, 0xA180E2,
        0xAC82E2, 0xB080E2, 0x89D0, 0xB980E2, 0x8AD0, 0x8CD0, 0x8BD0, 0x8FD0,
        0x92D1, 0x9880E2, 0x9980E2, 0x9C80E2, 0x9D80E2, 0xA280E2, 0x9380E2, 0x9480E2,
        0, 0xA284E2, 0x99D1, 0xBA80E2, 0x9AD1, 0x9CD1, 0x9BD1, 0x9FD1,
        0xA0C2, 0x8ED0, 0x9ED1, 0x88D0, 0xA4C2, 0x90D2, 0xA6C2, 0xA7C2,
        0x81D0, 0xA9C2, 0x84D0, 0xABC2, 0xACC2, 0xADC2, 0xAEC2, 0x87D0,
        0xB0C2, 0xB1C2, 0x86D0, 0x96D1, 0x91D2, 0xB5C2, 0xB6C2, 0xB7C2,
        0x91D1, 0x9684E2, 0x94D1, 0xBBC2, 0x98D1, 0x85D0, 0x95D1, 0x97D1,
        0x90D0, 0x91D0, 0x92D0, 0x93D0, 0x94D0, 0x95D0, 0x96D0, 0x97D0,
        0x98D0, 0x99D0, 0x9AD0, 0x9BD0, 0x9CD0, 0x9DD0, 0x9ED0, 0x9FD0,
        0xA0D0, 0xA1D0, 0xA2D0, 0xA3D0, 0xA4D0, 0xA5D0, 0xA6D0, 0xA7D0,
        0xA8D0, 0xA9D0, 0xAAD0, 0xABD0, 0xACD0, 0xADD0, 0xAED0, 0xAFD0,
        0xB0D0, 0xB1D0, 0xB2D0, 0xB3D0, 0xB4D0, 0xB5D0, 0xB6D0, 0xB7D0,
        0xB8D0, 0xB9D0, 0xBAD0, 0xBBD0, 0xBCD0, 0xBDD0, 0xBED0, 0xBFD0,
        0x80D1, 0x81D1, 0x82D1, 0x83D1, 0x84D1, 0x85D1, 0x86D1, 0x87D1,
        0x88D1, 0x89D1, 0x8AD1, 0x8BD1, 0x8CD1, 0x8DD1, 0x8ED1, 0x8FD1};
    while (*in)
    {
        if (*in & 0x80)
        {
            int v = table[(int)(0x7f & *in++)];
            if (!v)
                continue;
            *out++ = (char)v;
            *out++ = (char)(v >> 8);
            if (v >>= 16)
                *out++ = (char)v;
        }
        else
            *out++ = *in++;
    }
    *out = 0;
}
std::string cp2utf(std::string s)
{
    int c, i;
    int len = s.size();
    std::string ns;
    for (i = 0; i < len; i++)
    {
        c = s[i];
        char buf[4], in[2] = {0, 0};
        *in = c;
        cp2utf1(buf, in);
        ns += std::string(buf);
    }
    return ns;
}
