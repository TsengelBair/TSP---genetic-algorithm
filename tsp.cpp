
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_set>

using namespace std;

const int INF = 10000; // для инициализации

// Граф задан матрицей смежности
vector<vector<int>> graf = {
    {0, 44, 46, 32, 37, INF},
    {44, 0, 45, INF, INF, 30},
    {46, 45, 0, INF, 15, INF},
    {32, INF, INF, 0, 22, 17},
    {37, INF, 15, 22, 0, 35},
    {INF, 30, INF, 17, 35, 0}
};

const double mutationProbability = 0.1; // Вероятность мутации
const double crossoverProbability = 0.8; // Вероятность скрещивания

// Фитнес ф-ция для расчета стоимости маршрута
int calculateRouteWeight(const vector<int>& route) {
    int weight = 0;
    for (int i = 0; i < route.size() - 1; i++) {
        int from = route[i];
        int to = route[i + 1];
        weight += graf[from][to];
    }
    return weight;
}

// Cоздание случайной популяции
vector<vector<int>> createRandomPopulation(int populationSize, int numVertices) {
    vector<vector<int>> population;
    for (int i = 0; i < populationSize; i++) {
        vector<int> route(numVertices);
        for (int j = 0; j < numVertices; j++) {
            route[j] = j;
        }
        // Случайная перестановка элементов массива
        shuffle(route.begin() + 1, route.end(), default_random_engine(random_device()()));
        population.push_back(route);
    }
    return population;
}

// Турнирный отбор
vector<vector<int>> tournamentSelection(const vector<vector<int>>& population, int tournamentSize) {
    vector<vector<int>> selectedIndividuals;
    random_device rd;
    mt19937 gen(rd());

    for (int i = 0; i < population.size(); i++) {
        vector<vector<int>> tournament;
        for (int j = 0; j < tournamentSize; j++) {
            int randomIndex = uniform_int_distribution<int>(0, population.size() - 1)(gen);
            tournament.push_back(population[randomIndex]);
        }
        // Находим индивида с наименьшим весом маршрута в турнире
        auto minIndividual = min_element(tournament.begin(), tournament.end(), [](const vector<int>& a, const vector<int>& b) {
            return calculateRouteWeight(a) < calculateRouteWeight(b);
            });
        selectedIndividuals.push_back(*minIndividual);
    }
    return selectedIndividuals;
}

// Скрещивание двух индивидов с учетом уникальности генов
vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(1, parent1.size() - 2); // Выбираем точку для разделения, исключая начальную и конечную вершины
    int crossoverPoint1 = distribution(gen);
    int crossoverPoint2 = distribution(gen);

    // Убеждаемся, что crossoverPoint2 больше crossoverPoint1
    if (crossoverPoint1 > crossoverPoint2) {
        swap(crossoverPoint1, crossoverPoint2);
    }

    vector<int> child(parent1.size(), -1); // Инициализируем потомка -1
    unordered_set<int> usedGenes; // Храним уже использованные гены

    // Копируем гены от родителя 1
    for (int i = crossoverPoint1; i <= crossoverPoint2; i++) {
        child[i] = parent1[i];
        usedGenes.insert(parent1[i]); // Добавляем гены родителя 1 в множество использованных генов
    }

    // Заполняем оставшиеся вершины из родителя 2
    int currentPos = 0;
    for (int i = 0; i < parent2.size(); i++) {
        if (currentPos == crossoverPoint1) {
            currentPos = crossoverPoint2 + 1;
        }
        if (usedGenes.find(parent2[i]) == usedGenes.end()) { // Если ген родителя 2 еще не использовался
            while (child[currentPos] != -1) {
                ++currentPos;
            }
            child[currentPos] = parent2[i];
            usedGenes.insert(parent2[i]); // Добавляем ген родителя 2 в множество использованных генов
        }
    }

    // Заполняем оставшиеся пустые места в потомке из родителя 2
    for (int i = 0; i < child.size(); i++) {
        if (child[i] == -1) {
            for (int j = 0; j < parent2.size(); j++) {
                if (usedGenes.find(parent2[j]) == usedGenes.end()) { // Если ген родителя 2 еще не использовался
                    child[i] = parent2[j];
                    usedGenes.insert(parent2[j]); // Добавляем ген родителя 2 в множество использованных генов
                    break;
                }
            }
        }
    }

    return child;
}
// Функция для мутации одного индивида
void mutate(vector<int>& individual) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distribution(1, individual.size() - 2); // Выбираем случайную вершину, исключая начальную и конечную
    int mutationPoint1 = distribution(gen);
    int mutationPoint2 = distribution(gen);

    // Ищем одинаковые элементы для обмена
    int valueToSwap = individual[mutationPoint1];
    auto it = find(individual.begin(), individual.end(), valueToSwap);
    int swapIndex = distance(individual.begin(), it);

    // Меняем местами две вершины
    swap(individual[mutationPoint1], individual[swapIndex]);
}

// Функция для мутации популяции
void mutatePopulation(vector<vector<int>>& population, double mutationRate) {
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> distribution(0.0, 1.0);

    for (auto& individual : population) {
        if (distribution(gen) < mutationRate) { // Применяем мутацию с вероятностью mutationRate
            mutate(individual);
        }
    }
}

// Функция для скрещивания популяции
vector<vector<int>> crossoverPopulation(const vector<vector<int>>& population) {
    vector<vector<int>> newPopulation;
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<double> crossoverDistribution(0.0, 1.0); // Распределение для определения, будет ли производиться скрещивание

    for (int i = 0; i < population.size(); i += 2) {
        if (crossoverDistribution(gen) < crossoverProbability) { // Проверяем вероятность скрещивания
            uniform_int_distribution<int> distribution(i, i + 1); // Выбираем двух родителей для скрещивания
            int parent1Index = distribution(gen);
            int parent2Index = parent1Index == i ? i + 1 : i;

            // Скрещиваем родителей и добавляем потомство в новую популяцию
            vector<int> child1 = crossover(population[parent1Index], population[parent2Index]);
            vector<int> child2 = crossover(population[parent2Index], population[parent1Index]);
            newPopulation.push_back(child1);
            newPopulation.push_back(child2);
        }
        else { // Если скрещивание не произошло, добавляем родителей в новую популяцию без изменений
            newPopulation.push_back(population[i]);
            newPopulation.push_back(population[i + 1]);
        }
    }

    return newPopulation;
}

int main() {
    setlocale(LC_ALL, "ru");

    int populationSize = 70; // Размер популяции
    int numVertices = graf.size(); // Количество вершин в графе
    int tournamentSize = 5; // Размер турнира
    int numEpochs = 10; // Количество эпох эволюции

    // Создание случайной популяции
    vector<vector<int>> population = createRandomPopulation(populationSize, numVertices);

    // Цикл по эпохам эволюции
    for (int epoch = 0; epoch < numEpochs; epoch++) {
        // Вывод текущей эпохи
        cout << "Эпоха " << epoch + 1 << ":" << endl;

        // Вывод текущей популяции
        cout << "Текущая популяция:" << endl;
        for (const auto& individual : population) {
            for (const auto& vertex : individual) {
                cout << vertex << " ";
            }
            cout << "- Вес: " << calculateRouteWeight(individual) << endl;
        }
        // Турнирный отбор
        vector<vector<int>> selectedIndividuals = tournamentSelection(population, tournamentSize);

        // Скрещивание популяции
        vector<vector<int>> newPopulation = crossoverPopulation(selectedIndividuals);

        // Мутация популяции
        mutatePopulation(newPopulation, mutationProbability); // Применяем мутацию с вероятностью mutationProbability

        // Замена старой популяции на новую
        population = move(newPopulation);
    }

    // Вывод лучшего индивида после всех эпох
    auto bestIndividual = min_element(population.begin(), population.end(), [](const vector<int>& a, const vector<int>& b) {
        return calculateRouteWeight(a) < calculateRouteWeight(b);
        });
    cout << "Лучший маршрут: ";
    for (const auto& vertex : *bestIndividual) {
        cout << vertex << " ";
    }
    cout << "- Вес: " << calculateRouteWeight(*bestIndividual) << endl;

    return 0;
}