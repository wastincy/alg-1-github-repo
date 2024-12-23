#include <iostream>
#include <vector>   // Библиотека для работы с динамическими массивами (векторами)
#include <string>   // Библиотека для работы со строками
#include <algorithm> // Библиотека для алгоритмов find
#include <windows.h> // Библиотека для работы с Windows API (для кодировки UTF-8 вCMD)
#include <limits>       // Для использования numeric_limits
#include <queue>        // Для использования приоритетной очереди
#include <unordered_map> // Для использования хеш-таблицы (unordered_map)
#include <fstream>  // Подключает библиотеку для работы с файлами, которая предоставляет классы для чтения и записи в файлы
#include <map>

using namespace std;

// Класс для представления вершины графа
template <class V, class E>
class Vertex; // Предварительное объявление класса Vertex для использования в классе Edge

// Класс для представления ребер графа
template <class V, class E>
class Edge {
public:
    // Конструктор класса Edge
    // properties - свойства ребра, vertex1 и vertex2 - указатели на вершины, соединенные этим ребром
    Edge(const E& properties, Vertex<V, E>* vertex1, Vertex<V, E>* vertex2)
        : properties_(properties), vertex1_(vertex1), vertex2_(vertex2) {}

    // Метод для получения первой вершины, связанной с ребром
    const Vertex<V, E>* getVertex1() const { return vertex1_; }

    // Метод для получения второй вершины, связанной с ребром
    const Vertex<V, E>* getVertex2() const { return vertex2_; }

    // Метод для получения свойств ребра
    const E* getProperties() const { return &properties_; }

private:
    E properties_; // Свойства ребра
    Vertex<V, E>* vertex1_; // Указатель на первую вершину
    Vertex<V, E>* vertex2_; // Указатель на вторую вершину
};

// Класс для представления вершины графа
template <class V, class E>
class Vertex {
public:
    // Деструктор
    ~Vertex() {
        // Освобождаем память, выделенную для рёбер
        for (Edge<V, E>* edge : edges_) {
            delete edge; // Удаляем каждое ребро
        }
    }

    // Метод для вывода информации о вершине и её рёбрах
    void print() const {
        cout << "Вершина: " << properties_ << endl;
        for (const Edge<V, E>* edge : edges_) {
            cout << "  Ребро к " << *(edge->getVertex2()->getProperties())
                 << " (стоимость: " << *(edge->getProperties()) << ")" << endl;
        }
    }

    // Конструктор класса Vertex
    // properties - свойства вершины
    Vertex(const V& properties) : properties_(properties) {}

    // Метод для получения свойств вершины
    const V* getProperties() const { return &properties_; }

    // Метод для получения списка ребер, связанных с вершиной
    const vector<Edge<V, E>*>* getEdges() const { return &edges_; }

    // Метод для добавления ребра, соединяющего эту вершину с целевой вершиной
    void addEdge(const E& properties, Vertex<V, E>* target) {
        // Создание нового ребра
        Edge<V, E>* edge = new Edge<V, E>(properties, this, target);
        edges_.push_back(edge); // Добавление ребра в список этой вершины
        target->edges_.push_back(edge); // Добавление ребра в список целевой вершины
    }

private:
    V properties_; // Свойства вершины
    vector<Edge<V, E>*> edges_; // Список указателей на ребра, связанные с вершиной
};

// Посетитель для обхода графа, который предотвращает зацикливание
template <class V, class E>
class OneTimeVisitor {
public:
    // Метод для посещения вершины
    // Возвращает true, если вершина была посещена впервые, иначе false
    bool visitVertex(const Vertex<V, E>* vertex) {
        if (find(visited_.begin(), visited_.end(), vertex) != visited_.end())
            return false; // Если вершина уже посещена, возвращаем false
        visited_.push_back(vertex); // Добавляем вершину в список посещенных
        return true; // Возвращаем true, так как вершина посещена
    }

    // Метод для посещения ребра
    // Возвращает true, так как мы не ограничиваем посещение ребер
    bool visitEdge(const Edge<V, E>*) { return true; }

    // Метод для выхода из вершины
    void leaveVertex(const Vertex<V, E>*) { visited_.pop_back(); } // Удаляем последнюю посещенную вершину

    // Метод для выхода из ребра
    void leaveEdge(const Edge<V, E>*) {} // Ничего не делаем при выходе из ребра

private:
    vector<const Vertex<V, E>*> visited_; // Список посещенных вершин
};

// Алгоритм обхода графа в глубину с использованием паттерна "Посетитель"
// vertex - указатель на текущую вершину графа, которую мы обходим
// visitor - указатель на объект посетителя, который реализует логику обработки вершин и ребер
template <class V, class E, class F>
void depthPass(const Vertex<V, E>* vertex, F* visitor) {
    // Если посетитель не разрешает посещение текущей вершины, выходим из функции
    if (!visitor->visitVertex(vertex))
        return;

    // Проходим по всем рёбрам текущей вершины
    for (Edge<V, E>* edge : *vertex->getEdges()) {
        // Если посетитель не разрешает обработку текущего ребра, переходим к следующему
        if (!visitor->visitEdge(edge))
            continue;

        // Определяем следующую вершину для обхода
        const Vertex<V, E>* next = edge->getVertex1() == vertex || edge->getVertex1() == nullptr
            ? edge->getVertex2() : edge->getVertex1();

        // Рекурсивно вызываем обход для следующей вершины
        depthPass(next, visitor);

        // Уведомляем посетителя о том, что покидаем текущее ребро
        visitor->leaveEdge(edge);
    }

    // Уведомляем посетителя о том, что покидаем текущую вершину
    visitor->leaveVertex(vertex);
}

// Рекурсивная функция поиска в глубину с подсчетом стоимости пути
// vertex - указатель на текущую вершину, которую мы исследуем
// targetName - имя целевой вершины, которую мы ищем
// visited - вектор, содержащий посещенные вершины, чтобы избежать циклов
// cost - ссылка на переменную, в которую будет записана стоимость найденного пути
template <class V, class E>
bool search(const Vertex<V, E>* vertex, const V& targetName, vector<const Vertex<V, E>*>& visited, int& cost) {
    // Проверяем, была ли текущая вершина уже посещена
    if (find(visited.begin(), visited.end(), vertex) != visited.end())
        return false;

    // Добавляем текущую вершину в список посещенных
    visited.push_back(vertex);

    // Проверяем, является ли текущая вершина целевой
    if (*vertex->getProperties() == targetName) {
        cout << "Целевая вершина " << targetName << " найдена!" << endl;
        return true;
    }

    // Проходим по всем рёбрам текущей вершины
    for (const Edge<V, E>* edge : *vertex->getEdges()) {
        // Определяем следующую вершину для обхода
        const Vertex<V, E>* next = (edge->getVertex1() == vertex || edge->getVertex1() == nullptr)
            ? edge->getVertex2() : edge->getVertex1();

        // Выводим информацию о текущем шаге
        cout << "Переход к вершине " << *(next->getProperties())
             << " через ребро со стоимостью " << *(edge->getProperties()) << endl;

        // Рекурсивно вызываем поиск для следующей вершины
        if (search(next, targetName, visited, cost)) {
            // Если путь найден, добавляем стоимость текущего ребра к общей стоимости
            cost += *edge->getProperties();
            cout << "Добавляем стоимость ребра: " << *(next->getProperties())
                 << " (стоимость: " << *(edge->getProperties())
                 << "), общая стоимость: " << cost << endl;
            return true;
        }
    }

    // Если путь не найден, удаляем текущую вершину из списка посещенных
    visited.pop_back();
    return false;
}

// Алгоритм Дейкстры для нахождения кратчайших путей
template <class V, class E>
void dijkstra(const Vertex<V, E>* start) {
    // Хранение расстояний до каждой вершины
    unordered_map<const Vertex<V, E>*, int> distances;
    // Хранение предшественников для восстановления пути
    unordered_map<const Vertex<V, E>*, const Vertex<V, E>*> previous;

    // Инициализация расстояний
    for (const Edge<V, E>* edge : *start->getEdges()) {
        const Vertex<V, E>* neighbor = edge->getVertex2();
        distances[neighbor] = numeric_limits<int>::max(); // Устанавливаем начальное расстояние как бесконечность
    }
    distances[start] = 0; // Расстояние до самой себя равно 0

    // Используем приоритетную очередь для хранения вершин по расстоянию
    auto cmp = [](pair<const Vertex<V, E>*, int>& a, pair<const Vertex<V, E>*, int>& b) {
        return a.second > b.second; // Сравниваем по расстоянию
    };
    priority_queue<pair<const Vertex<V, E>*, int>, vector<pair<const Vertex<V, E>*, int>>, decltype(cmp)> pq(cmp);
    pq.push({start, 0}); // Добавляем начальную вершину в очередь

    while (!pq.empty()) {
        const Vertex<V, E>* current = pq.top().first; // Текущая вершина
        pq.pop();

        // Проходим по всем рёбрам текущей вершины
        for (const Edge<V, E>* edge : *current->getEdges()) {
            const Vertex<V, E>* neighbor = edge->getVertex2();
            int newDist = distances[current] + *edge->getProperties(); // Рассчитываем новое расстояние

            // Если найдено более короткое расстояние
            if (newDist < distances[neighbor]) {
                distances[neighbor] = newDist; // Обновляем расстояние
                previous[neighbor] = current; // Обновляем предшественника
                pq.push({neighbor, newDist}); // Добавляем в очередь
            }
        }
    }

    // Выводим результаты
    for (const auto& pair : distances) {
        cout << "Расстояние до вершины " << *(pair.first->getProperties())
             << " равно " << pair.second << endl;
    }
}

vector<string> readVertices(const string& filename) {
    ifstream file(filename);

    // Проверяем, открыт ли файл
    if (!file.is_open()) {
        return {}; // Возвращаем пустой вектор, если файл не найден
    }

    vector<string> lines; // Вектор для хранения строк
    string line;

    // Читаем строки из файла
    while (getline(file, line) && lines.size() < 3) {
        lines.push_back(line); // Добавляем строку в вектор
    }

    return lines; // Возвращаем вектор строк
}

int main() {
    // Устанавливаем кодовую страницу консоли на UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // Создание графа
    Vertex<string, int> A1("A1");
    Vertex<string, int> B1("B1");
    Vertex<string, int> C1("C1");
    Vertex<string, int> B2("B2");
    Vertex<string, int> E1("E1");
    Vertex<string, int> F1("F1");
    Vertex<string, int> B3("B3");

    // Добавление рёбер с указанием стоимости
    A1.addEdge(15, &B1);
    B1.addEdge(22, &C1);
    A1.addEdge(15, &B2);
    C1.addEdge(35, &E1);
    E1.addEdge(41, &F1);
    B2.addEdge(20, &F1);
    A1.addEdge(17, &B3);
    B3.addEdge(10, &C1);

    // Вызов метода print для каждой вершины
    A1.print();
    B1.print();
    C1.print();
    B2.print();
    E1.print();
    F1.print();
    B3.print();

    // Пример поиска пути
    vector<const Vertex<string, int>*> visited; // Вектор для хранения посещенных вершин
    int cost = 0; // Переменная для хранения стоимости пути

    map<string, Vertex<string, int>*> vertexMap;
    vertexMap["A1"] = &A1;
    vertexMap["B1"] = &B1;
    vertexMap["C1"] = &C1;
    vertexMap["B2"] = &B2;
    vertexMap["E1"] = &E1;
    vertexMap["F1"] = &F1;
    vertexMap["B3"] = &B3;

    string filename = "vertices.txt"; // Имя файла
    vector<string> vertices = readVertices(filename); // Читаем вершины из файла

    // Проверяем, прочитали ли мы три строки
    if (vertices.size() < 3) {
        cout << "Файл не найден или содержит менее трёх строк." << endl;
        return 1; // Завершаем программу, если не удалось прочитать три строки
    }

    string firstVertex = vertices[0];
    string secondVertex = vertices[1];
    string dijkstraVertex = vertices[2];

    // Поиск первой вершины в отображении
    auto it = vertexMap.find(firstVertex);
    if (it != vertexMap.end()) {
        // Если вершина найдена, запускаем алгоритмы
        // Использование посетителя для обхода графа
        OneTimeVisitor<string, int> visitor;
        cout << "Обход графа с использованием OneTimeVisitor:" << endl;
        depthPass(it->second, &visitor);

        // Запускаем алгоритм Дейкстры от второй вершины
        auto itDijkstra = vertexMap.find(secondVertex);
        if (itDijkstra != vertexMap.end()) {
            // Передаем строку как string для поиска
            if (search(it->second, secondVertex, visited, cost)) {
                cout << "Путь найден, стоимость: " << cost << endl;
            } else {
                cout << "Путь не найден." << endl;
            }
            cout << "Запускаем алгоритм Дейкстры от вершины: " << dijkstraVertex << endl;
            dijkstra(itDijkstra->second); // Запускаем алгоритм Дейкстры от найденной вершины
        } else {
            cout << "Вершина для алгоритма Дейкстры не найдена в графе." << endl;
        }
    } else {
        cout << "Вершина не найдена в графе." << endl;
    }

    return 0;
}
