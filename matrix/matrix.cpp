#include <iostream>
#include <fstream> // Для работы с файлами
#include <sstream>  // Для работы с строковыми потоками
#include <string> // Для работы со строками
#include <vector> // Для работы с векторами
#include <chrono> // Для работы с временем
#include <ctime>  // Для работы с временем
#include <iomanip> // Для управления форматированием ввода и вывода
#include <stdexcept>  // Для обработки исключений
#include <cmath> // Для использования математических функций

using namespace std;

// Класс для работы с матрицами
template<typename T = double>
class MatrixDense {
    T* __data; // Указатель на массив данных
    unsigned _m, _n; // Размеры матрицы: количество строк (_m) и столбцов (_n)
    bool __is_valid; // Флаг валидности матрицы

public:
    // Конструктор, принимающий размеры матрицы
    MatrixDense(unsigned m = 0, unsigned n = 0) : _m(m), _n(n), __is_valid(false) {
        try {
            if (m > 0 && n > 0) {
                __data = new T[_m * _n](); // Инициализация нулями
                __is_valid = true;
            } else {
                __data = nullptr;
            }
        } catch (const bad_alloc& e) {
            __data = nullptr;
            __is_valid = false;
        }
    }

    // Конструктор копирования
    MatrixDense(const MatrixDense& other) : _m(other._m), _n(other._n), __is_valid(false) {
        try {
            if (other.__is_valid && _m > 0 && _n > 0) {
                __data = new T[_m * _n];
                copy(other.__data, other.__data + _m * _n, __data);
                __is_valid = true;
            } else {
                __data = nullptr;
            }
        } catch (const bad_alloc& e) {
            __data = nullptr;
            __is_valid = false;
        }
    }

    // Оператор присваивания
    MatrixDense& operator=(const MatrixDense& other) {
        if (this != &other) {
            if (__data) {
                delete[] __data;
            }
            _m = other._m;
            _n = other._n;
            __is_valid = false;
            try {
                if (other.__is_valid && _m > 0 && _n > 0) {
                    __data = new T[_m * _n];
                    copy(other.__data, other.__data + _m * _n, __data);
                    __is_valid = true;
                } else {
                    __data = nullptr;
                }
            } catch (const bad_alloc& e) {
                __data = nullptr;
                __is_valid = false;
            }
        }
        return *this;
    }

    // Деструктор
    ~MatrixDense() {
        if (__data != nullptr) {
            delete[] __data;
            __data = nullptr;
        }
    }

    // Метод для доступа к элементам матрицы с проверкой границ
    T& getElement(unsigned i, unsigned j) {
        if (!__is_valid) {
            throw runtime_error("Доступ к элементам недопустимой матрицы");
        }
        if (i >= _m || j >= _n) {
            throw out_of_range("Индекс матрицы выходит за рамки допустимого");
        }
        return __data[j + i * _n];
    }

    // Константная версия getElement
    const T& getElement(unsigned i, unsigned j) const {
        if (!__is_valid) {
            throw runtime_error("Доступ к элементам недопустимой матрицы");
        }
        if (i >= _m || j >= _n) {
            throw out_of_range("Индекс матрицы выходит за рамки допустимого");
        }
        return __data[j + i * _n];
    }

    // Проверка валидности матрицы
    bool isValid() const { return __is_valid; }

    unsigned rows() const { return _m; }
    unsigned cols() const { return _n; }
};

struct MatrixData {
    MatrixDense<double> matrix;
    unsigned rows;
    unsigned cols;

    MatrixData() : rows(0), cols(0), matrix(MatrixDense<double>(0, 0)) {}
};

struct CalcProblemParams {
    string filePath1;
    string filePath2;
    string exportPath;
    enum class operations { vv_sum, vv_sub, mm_sum };
    operations op;
};

struct VectorData {
    vector<double> values;
    int size = 0;
};

struct ExportConfig {
    string path;
};

struct CalcResults {
    VectorData result;
    MatrixData matrixResult;
};

int logAll(string data) {
    ofstream logFile("log.txt", ios::app);
    if (!logFile) {
        return -1;
    }
    time_t t = time(0);
    struct tm *now = localtime(&t);
    logFile << put_time(now, "%Y-%m-%d %H:%M:%S") << " - " << data << endl;
    logFile.close();
    return 0;
}

VectorData readDataFromFile(const string& filepath) {
    string line;
    VectorData vectorData;
    ifstream dataFile(filepath);
    if (!dataFile) {
        logAll("Ошибка открытия файла");
        return vectorData;
    }
    while (getline(dataFile, line)) {
        if (line == "vector") {
            logAll("В файле обнаружено значение vector");
            if (getline(dataFile, line)) {
                vectorData.size = stoi(line);
                vectorData.values.resize(vectorData.size);
                if (getline(dataFile, line)) {
                    istringstream iss(line);
                    for (int i = 0; i < vectorData.size; ++i) {
                        iss >> vectorData.values[i];
                    }
                }
            }
        }
    }
    dataFile.close();
    return vectorData;
}

MatrixData readMatrixFromFile(const string& filepath) {
    string line;
    MatrixData matrixData;
    ifstream dataFile(filepath);
    if (!dataFile) {
        logAll("Ошибка: Невозможно открыть файл: " + filepath);
        return matrixData;
    }
    dataFile.seekg(0, ios::end);
    streampos fileSize = dataFile.tellg();
    dataFile.seekg(0, ios::beg);
    if (fileSize == 0) {
        logAll("Ошибка: Файл пуст: " + filepath);
        return matrixData;
    }
    logAll("Попытка чтения матрицы из файла: " + filepath);
    try {
        while (getline(dataFile, line)) {
            if (line.empty() || line[0] == '#') continue;
            logAll("Чтение строки: " + line);
            if (line == "matrix") {
                logAll("Найдено значение 'matrix'");
                if (!getline(dataFile, line)) {
                    logAll("Ошибка: Неожиданный конец файла после 'matrix' маркера");
                    return matrixData;
                }
                logAll("Парсинг размерности: " + line);
                size_t xPos = line.find('x');
                if (xPos == string::npos) {
                    logAll("Ошибка: Неверный формат размерности матрицы. Ожидается 'MxN'.");
                    return matrixData;
                }
                try {
                    matrixData.rows = stoi(line.substr(0, xPos));
                    matrixData.cols = stoi(line.substr(xPos + 1));
                    const unsigned MAX_MATRIX_SIZE = 10000;
                    if (matrixData.rows == 0 || matrixData.cols == 0 ||
                        matrixData.rows > MAX_MATRIX_SIZE ||
                        matrixData.cols > MAX_MATRIX_SIZE) {
                        logAll("Ошибка: Неверные размеры матрицы. Строки: " +
                               to_string(matrixData.rows) +
                               ", Колонны: " + to_string(matrixData.cols));
                        return matrixData;
                    }
                    logAll("Размеры матрицы: " +
                           to_string(matrixData.rows) + "x" +
                           to_string(matrixData.cols));
                    matrixData.matrix = MatrixDense<double>(matrixData.rows, matrixData.cols);
                    for (unsigned i = 0; i < matrixData.rows; ++i) {
                        if (!getline(dataFile, line)) {
                            logAll("Ошибка: Неожиданный конец файла. Ожидается " +
                                   to_string(matrixData.rows) + " строк.");
                            return matrixData;
                        }
                        logAll("Чтение строки матрицы: " + line);
                        istringstream iss(line);
                        for (unsigned j = 0; j < matrixData.cols; ++j) {
                            double value;
                            if (!(iss >> value)) {
                                logAll("Ошибка: Не удалось прочитать элемент матрицы в позиции (" +
                                       to_string(i) + ", " + to_string(j) + ")");
                                return matrixData;
                            }
                            if (isnan(value) || isinf(value)) {
                                logAll("Внимание: Подозрительное значение matrix в позиции (" +
                                       to_string(i) + ", " + to_string(j) +
                                       "): " + to_string(value));
                            }
                            matrixData.matrix.getElement(i, j) = value;
                            logAll("Элемент matrix в позиции (" +
                                   to_string(i) + ", " + to_string(j) +
                                   ") is: " + to_string(value));
                        }
                        double extraValue;
                        if (iss >> extraValue) {
                            logAll("Внимание: В строке " +
                                   to_string(i) + " остались лишние элементы. Их игнорируется.");
                        }
                    }
                    logAll("Матрица успешно прочитана");
                    return matrixData;
                } catch (const exception& e) {
                    logAll("Ошибка парсинга матрицы: " + string(e.what()));
                    return matrixData;
                }
            }
        }
        logAll("Ошибка: Нет 'matrix' маркера в файле.");
        return matrixData;
    } catch (const exception& e) {
        logAll("Ошибка: обработка матрицы: " + string(e.what()));
        return matrixData;
    }
}

MatrixData Calck_mm_sum(const MatrixData& mat1, const MatrixData& mat2) {
    if (!mat1.matrix.isValid() || !mat2.matrix.isValid()) {
        logAll("Ошибка: Одна или обе матрицы недействительны");
        return MatrixData();
    }
    if (mat1.rows != mat2.rows || mat1.cols != mat2.cols) {
        logAll("Ошибка: Размерности матриц не совпадают для сложения. " +
               string("Matrix 1: ") + to_string(mat1.rows) + "x" + to_string(mat1.cols) +
               ", Matrix 2: " + to_string(mat2.rows) + "x" + to_string(mat2.cols));
        return MatrixData();
    }
    const unsigned MAX_MATRIX_SIZE = 10000;
    if (mat1.rows == 0 || mat1.cols == 0 ||
        mat1.rows > MAX_MATRIX_SIZE || mat1.cols > MAX_MATRIX_SIZE) {
        logAll("Ошибка: Размеры матрицы превышают допустимые ограничения");
        return MatrixData();
    }
    MatrixData resultMatrix;
    resultMatrix.rows = mat1.rows;
    resultMatrix.cols = mat1.cols;
    resultMatrix.matrix = MatrixDense<double>(resultMatrix.rows, resultMatrix.cols);
    try {
        for (unsigned i = 0; i < mat1.rows; ++i) {
            for (unsigned j = 0; j < mat1.cols; ++j) {
                double val1 = mat1.matrix.getElement(i, j);
                double val2 = mat2.matrix.getElement(i, j);
                double sumVal = val1 + val2;
                if (isnan(sumVal) || isinf(sumVal)) {
                    logAll("Внимание: Переполнение или неверный результат в (" +
                           to_string(i) + ", " + to_string(j) +
                           "): " + to_string(val1) + " + " + to_string(val2));
                    sumVal = 0;
                }
                resultMatrix.matrix.getElement(i, j) = sumVal;
                logAll("Сумма матрицы в (" + to_string(i) + ", " + to_string(j) +
                       "): " + to_string(val1) + " + " + to_string(val2) +
                       " = " + to_string(sumVal));
            }
        }
        logAll("Суммирование матрицы завершено успешно");
        return resultMatrix;
    } catch (const exception& e) {
        logAll("Ошибка суммирования матриц: " + string(e.what()));
        return MatrixData();
    } catch (...) {
        logAll("Ошибка: Неизвестная ошибка суммирования матриц");
        return MatrixData();
    }
}

VectorData Calck_vv_sum(const VectorData& vec1, const VectorData& vec2) {
    if (vec1.size != vec2.size) {
        logAll("Ошибка! Размерность векторов не совпадает");
        return VectorData();
    }
    VectorData result;
    result.size = vec1.size;
    result.values.resize(result.size);
    for (int i = 0; i < result.size; ++i) {
        result.values[i] = vec1.values[i] + vec2.values[i];
    }
    return result;
}

VectorData Calck_vv_sub(const VectorData& vec1, const VectorData& vec2) {
    if (vec1.size != vec2.size) {
        logAll("Ошибка! Размерность векторов не совпадает");
        return VectorData();
    }
    VectorData result;
    result.size = vec1.size;
    result.values.resize(result.size);
    for (int i = 0; i < result.size; ++i) {
        result.values[i] = vec1.values[i] - vec2.values[i];
    }
    return result;
}

int Export(const CalcResults& calcResults, const ExportConfig& config) {
    logAll("Открываю " + config.path);
    ofstream dataFile(config.path, ios::app);
    if (!dataFile) {
        logAll("Ошибка открытия файла: " + config.path);
        return -1;
    }
    if (calcResults.result.size > 0) {
        dataFile << "Результат вычисления векторов: " << endl;
        for (const auto& value : calcResults.result.values) {
            dataFile << value << " ";
        }
        dataFile << endl;
    } else {
        logAll("Нет данных для записи вектора, ничего не записывается в файл.");
    }
    unsigned countMatrixElements = calcResults.matrixResult.rows * calcResults.matrixResult.cols;
    if (countMatrixElements > 0) {
        dataFile << "Результат вычисления матриц:" << endl;
        for (unsigned i = 0; i < calcResults.matrixResult.rows; ++i) {
            for (unsigned j = 0; j < calcResults.matrixResult.cols; ++j) {
                dataFile << calcResults.matrixResult.matrix.getElement(i, j) << " ";
            }
            dataFile << endl;
        }
    } else {
        logAll("Нет данных для записи матрицы.");
    }
    dataFile.close();
    return 0;
}

int main(int argc, char* argv[]) {
    FILE* forcedLog = fopen("log.txt", "a");
    if (forcedLog) {
        fprintf(forcedLog, "\n--- НОВЫЙ ЗАПУСК ПРОГРАММЫ ---\n");
        fprintf(forcedLog, "Программа запущена с %d аргументами\n", argc);
        for (int k = 0; k < argc; ++k) {
            fprintf(forcedLog, "Аргументы %d: %s\n", k, argv[k] ? argv[k] : "NULL");
        }
        fclose(forcedLog);
    }
    for (int k = 0; k < argc; ++k) {
        if (argv[k] == nullptr) {
            fprintf(stderr, "Ошибка: Null аргумент в индексе %d\n", k);
            return 1;
        }
    }
    VectorData vector1, vector2;
    MatrixData matrix1, matrix2;
    CalcProblemParams calcParams;
    CalcResults calcResult;
    for (int k = 0; k < argc; ++k) {
        logAll("Полный Аргумент " + to_string(k) + ": " + string(argv[k]));
    }
    logAll("Запуск функции main с " + to_string(argc) + " arguments");
    bool fp1_processed = false;
    bool fp2_processed = false;
    bool op_processed = false;
    bool exp_processed = false;
    try {
        for (int i = 1; i < argc; i++) {
            logAll("Обработка аргумента " + to_string(i) + ": " + string(argv[i]));
            if (string(argv[i]) == "--fp1" || string(argv[i]) == "--matrix_fp1") {
                if (i + 1 < argc) {
                    calcParams.filePath1 = argv[++i];
                    logAll("Путь к первому файлу: " + calcParams.filePath1);
                    FILE* testFile = fopen(calcParams.filePath1.c_str(), "r");
                    if (!testFile) {
                        logAll("Ошибка: Не удалось открыть первый файл: " + calcParams.filePath1);
                        return 1;
                    }
                    fclose(testFile);
                    vector1 = readDataFromFile(calcParams.filePath1);
                    if (vector1.size == 0) {
                        matrix1 = readMatrixFromFile(calcParams.filePath1);
                        if (matrix1.rows == 0 || matrix1.cols == 0) {
                            logAll("Ошибка: Не удалось прочитать данные из первого файла");
                            return 1;
                        }
                    }
                    fp1_processed = true;
                } else {
                    logAll("Ошибка: нет аргумента после --fp1 или --matrix_fp1");
                    return 1;
                }
            } else if (string(argv[i]) == "--fp2" || string(argv[i]) == "--matrix_fp2") {
                if (i + 1 < argc) {
                    calcParams.filePath2 = argv[++i];
                    logAll("Путь ко второму файлу: " + calcParams.filePath2);
                    FILE* testFile = fopen(calcParams.filePath2.c_str(), "r");
                    if (!testFile) {
                        logAll("Ошибка: Не удалось открыть второй файл: " + calcParams.filePath2);
                        return 1;
                    }
                    fclose(testFile);
                    vector2 = readDataFromFile(calcParams.filePath2);
                    if (vector2.size == 0) {
                        matrix2 = readMatrixFromFile(calcParams.filePath2);
                        if (matrix2.rows == 0 || matrix2.cols == 0) {
                            logAll("Ошибка: Не удалось прочитать данные из второго файла");
                            return 1;
                        }
                    }
                    fp2_processed = true;
                } else {
                    logAll("Ошибка: нет аргумента после --fp2 или --matrix_fp2");
                    return 1;
                }
            } else if (string(argv[i]) == "--op") {
                if (i + 1 < argc) {
                    string opStr = argv[++i];
                    logAll("Операция: " + opStr);
                    if (opStr == "vv_sum") {
                        calcParams.op = CalcProblemParams::operations::vv_sum;
                    } else if (opStr == "vv_sub") {
                        calcParams.op = CalcProblemParams::operations::vv_sub;
                    } else if (opStr == "mm_sum") {
                        calcParams.op = CalcProblemParams::operations::mm_sum;
                    } else {
                        logAll("Неизвестная операция: " + opStr);
                        return 1;
                    }
                    op_processed = true;
                } else {
                    logAll("Ошибка: нет аргумента после --op");
                    return 1;
                }
            } else if (string(argv[i]) == "--exp") {
                if (i + 1 < argc) {
                    calcParams.exportPath = argv[++i];
                    logAll("Путь для экспорта: " + calcParams.exportPath);
                    exp_processed = true;
                } else {
                    logAll("Ошибка: нет аргумента после --exp");
                    return 1;
                }
            } else {
                logAll("Неожиданный аргумент: " + string(argv[i]));
                return 1;
            }
        }
        if (!fp1_processed || !fp2_processed || !op_processed || !exp_processed) {
            logAll("Ошибка: Не все обязательные аргументы обработаны");
            return 1;
        }
        switch (calcParams.op) {
            case CalcProblemParams::operations::vv_sum:
                if (vector1.size != vector2.size) {
                    logAll("Ошибка: Размеры векторов не совпадают");
                    return 1;
                }
                calcResult.result = Calck_vv_sum(vector1, vector2);
                break;
            case CalcProblemParams::operations::vv_sub:
                if (vector1.size != vector2.size) {
                    logAll("Ошибка: Размеры векторов не совпадают");
                    return 1;
                }
                calcResult.result = Calck_vv_sub(vector1, vector2);
                break;
            case CalcProblemParams::operations::mm_sum:
                if (matrix1.rows != matrix2.rows || matrix1.cols != matrix2.cols) {
                    logAll("Ошибка: Размеры матриц не совпадают");
                    return 1;
                }
                calcResult.matrixResult = Calck_mm_sum(matrix1, matrix2);
                break;
        }
        ExportConfig exportConfig;
        exportConfig.path = calcParams.exportPath;
        Export(calcResult, exportConfig);
        logAll("Операция успешно выполнена");
        return 0;
    } catch (const exception& e) {
        logAll("Ошибка: Ошибка обработки аргументов: " + string(e.what()));
        return 1;
    } catch (...) {
        logAll("Ошибка: Ошибка обработки аргументов");
        return 1;
    }
}

// Компиляция программы:
// g++ matrix.cpp -o app
// Сложение векторов
// ./app --fp1 vec1.txt --fp2 vec2.txt --op vv_sum --exp sum_data.txt
// Вычитание векторов
// ./app --fp1 vec1.txt --fp2 vec2.txt --op vv_sub --exp sub_data.txt
// Сложение матриц
// ./app --matrix_fp1 mat1.txt --matrix_fp2 mat2.txt --op mm_sum --exp matrix_sum.txt
// Описание аргументов в командной строке
// --fp1 или --matrix_fp1: путь к первому файлу с данными вектора или матрицы.
// --fp2 или --matrix_fp2: путь ко второму файлу с данными вектора или матрицы.
// --op: операция, которую нужно выполнить (vv_sum для сложения векторов, vv_sub для вычитания векторов, mm_sum для сложения матриц).
// --exp: путь к файлу, в который будут экспортированы результаты.