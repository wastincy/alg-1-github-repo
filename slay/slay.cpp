#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>
#include <windows.h> // Библиотека для работы с Windows API (для кодировки UTF-8 в CMD)

using namespace std;

const int MAX_NODES = 1000; // Максимальное количество узлов

// Класс для хранения вектора
class Vector {
private:
    vector<double> data;
public:
    Vector(int size) : data(size) {}

    double& operator[](int index) {
        return data[index];
    }

    const double& operator[](int index) const {
        return data[index];
    }

    int size() const {
        return data.size();
    }
};

// Функция для ввода параметров
void readParameters(int &N, double &t_end, double &L, double &lamda, double &ro, double &c, double &T0, double &Tl, double &Tr) {
    cout << "Введите количество пространственных узлов, N: ";
    cin >> N;
    cout << "Введите окончание по времени, t_end: ";
    cin >> t_end;
    cout << "Введите толщину пластины, L: ";
    cin >> L;
    cout << "Введите коэффициент теплопроводности материала пластины, lamda: ";
    cin >> lamda;
    cout << "Введите плотность материала пластины, ro: ";
    cin >> ro;
    cout << "Введите теплоемкость материала пластины, c: ";
    cin >> c;
    cout << "Введите начальную температуру, T0: ";
    cin >> T0;
    cout << "Введите температуру на границе x=0, Tl: ";
    cin >> Tl;
    cout << "Введите температуру на границе x=L, Tr: ";
    cin >> Tr;
}

// Функция для инициализации температуры
void initializeTemperature(vector<double> &T, int N, double T0) {
    for (int i = 0; i < N; i++) {
        T[i] = T0;
    }
}

// Функция для интегрирования нестационарного уравнения теплопроводности
void solveHeatEquation(vector<double> &T, int N, double lamda, double ro, double c, double h, double tau, double Tl, double Tr, double t_end) {
    vector<double> alfa(MAX_NODES), beta(MAX_NODES);
    double time = 0;

    while (time < t_end) {
        time += tau;

        // Начальные прогоночные коэффициенты
        alfa[0] = 0.0;
        beta[0] = Tl;

        // Определение прогоночных коэффициентов
        for (int i = 1; i < N - 1; i++) {
            double ai = lamda / (h * h);
            double bi = 2.0 * lamda / (h * h) + ro * c / tau;
            double ci = lamda / (h * h);
            double fi = -ro * c * T[i] / tau;

            alfa[i] = ai / (bi - ci * alfa[i - 1]);
            beta[i] = (ci * beta[i - 1] - fi) / (bi - ci * alfa[i - 1]);
        }

        // Температура на правой границе
        T[N - 1] = Tr;

        // Определение поля температуры
        for (int i = N - 2; i >= 0; i--) {
            T[i] = alfa[i] * T[i + 1] + beta[i];
        }
    }
}

// Функция для вывода результатов в файл
void writeResults(const vector<double> &T, int N, double L, double lamda, double ro, double c, double T0, double Tl, double Tr, double h, double tau, double t_end) {
    ofstream f("res.txt");
    f << fixed << setprecision(4);
    f << "Толщина пластины L = " << L << endl;
    f << "Число узлов по координате N = " << N << endl;
    f << "Коэффициент теплопроводности материала пластины lamda = " << lamda << endl;
    f << "Плотность материала пластины ro = " << ro << endl;
    f << "Теплоемкость материала пластины c = " << c << endl;
    f << "Начальная температура T0 = " << T0 << endl;
    f << "Температура на границе x = 0, Tl = " << Tl << endl;
    f << "Температура на границе x = L, Tr = " << Tr << endl;
    f << "Результат получен с шагом по координате h = " << h << endl;
    f << "Результат получен с шагом по времени tau = " << tau << endl;
    f << "Температурное поле в момент времени t = " << t_end << endl;
    f.close();
}

// Функция для вывода температурного поля в файл
void writeTemperatureField(const vector<double> &T, int N, double h) {
    ofstream g("temp.txt");
    g << fixed << setprecision(3);
    for (int i = 0; i < N; i++) {
        g << setw(6) << h * i << "   " << setw(8) << T[i] << endl;
    }
    g.close();
}

int main() {
    // Устанавливаем кодовую страницу консоли на UTF-8
    SetConsoleOutputCP(CP_UTF8);

    int N;
    double t_end, L, lamda, ro, c, T0, Tl, Tr;

    // Ввод параметров
    readParameters(N, t_end, L, lamda, ro, c, T0, Tl, Tr);

    // Расчетный шаг по пространственной координате и времени
    double h = L / (N - 1);
    double tau = t_end / 100.0;

    // Вектор для температуры
    vector<double> T(N);

    // Инициализация температуры
    initializeTemperature(T, N, T0);

    // Интегрирование уравнения теплопроводности
    solveHeatEquation(T, N, lamda, ro, c, h, tau, Tl, Tr, t_end);

    // Вывод результатов
    writeResults(T, N, L, lamda, ro, c, T0, Tl, Tr, h, tau, t_end);
    writeTemperatureField(T, N, h);

    return 0;
}
