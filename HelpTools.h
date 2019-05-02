#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <numeric>

struct HelpTools
{

    /** \brief Метод предназначен для вывода статистики времени на экран
     *  \param[in] size - размер массива.
     *  \param[out] array - массив.
     */
    static void printTimeStatistic(TimeValues times, double hostPerformanceTimeMS)
    {
        std::sort(times.begin(), times.end());
        double totalTime = std::accumulate(times.begin(), times.end(), 0.0);
        double averageTime = totalTime / times.size();
        double minTime = times[0];
        double maxTime = times[times.size() - 1];
        double medianTime = times[times.size() / 2];
        cout << "Calculation time statistic: (" << times.size() << " runs)" << endl;
        cout << "Med: " << medianTime << " ms (" << hostPerformanceTimeMS / medianTime << "X faster then host)" << endl;
        cout << "Avg: " << averageTime << " ms" << endl;
        cout << "Min: " << minTime << " ms" << endl;
        cout << "Max: " << maxTime << " ms" << endl << endl;
    }

    /** \brief Вспомогательный метод заполнения одномерного массива нулями.
     *  \param[in] size - размер массива.
     *  \param[out] array - массив.
     */
    static void fillNull(int size, int* array)
    {
        for (int i = 0; i < size; i++)
            array[i] = 0;
    }

    /** \brief Вспомогательный метод заполнения двумерного массива нулями.
     *  \param[in] rowsCount    - число строк.
     *  \param[in] columnsCount - число столбцов.
     *  \param[out] array       - массив.
     */
    static void fillNull(int rowsCount, int columnsCount, TransactionBase array)
    {
        for (int i = 0; i < rowsCount; i++)
            for (int j = 0; j < columnsCount; j++)
                array[i][j] = 0;
    }
    
    /** \brief Вспомогательный метод заполнения одномерного массива нулями.
     *  \param[in] rowsCount    - число строк.
     *  \param[in] columnsCount - число столбцов.
     *  \param[out] array       - массив.
     */
    static void fillNull(int start, int end, int* array)
    {
        for (int i = start; i < end; i++)
            array[i] = 0;
    }

    /** \brief Метод предназначен для чтения базы транзакций из текстового файла и заполнения контейнера транзакций
     *  \param[in]  fileName - имя файла, содержащий базу транзакций
     *  \param[out] transactions - прочитанная база транзакций
     *  \return true/false - успех/неудача
     */
    static bool readTransactionBase(const std::string& fileName, TransactionBase& transactions)
    {
        std::ifstream file;
        file.open(fileName.c_str());
        if (!file)
        {
            assert("Can not open file!");
            return false;
        }

        std::string line;
        std::stringstream ss;
        Transaction singleTransaction;

        while (getline(file, line))
        {
            if (line.empty())
                break;

            ss << line;
            int item;

            while (ss >> item)
                singleTransaction.push_back(item);

            transactions.push_back(singleTransaction);
            ss.clear();
            singleTransaction.clear();
        }

        file.close();
    }
};