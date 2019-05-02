#include "fptreeGPU.h"
#include "HelpTools.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>
#include <iomanip>

//---------------------------------------------------------------------------------------------------


void fptreeGPU::parseTransactionArray(int minsup, int rowscount, int columnscount, cl_int2* itemtable, TransactionBase& notparsedtransacarray, int& itemscount, TransactionBase& parsedtransacarray)
{
	bool isItemExist = false; // признак, что элемент уже содержится в таблице
	int currentItem = -1;     // текущий рассматриваемый элемент
	int itemCounter = 0;      // частота встречаемости элемента
	int indx = 0;			  // индекс таблицы элементов

	for (int i = 0; i < rowscount; i++)
	{
		for (int j = 0; j < columnscount; j++)
		{
			int elem = notparsedtransacarray[i][j];

			if (notparsedtransacarray[i][j] == 0)
				continue;

			// если такого элемента еще нет
			for (int k = 0; k < indx; ++k)
			{
				if (itemtable[k].s0 == notparsedtransacarray[i][j])
					isItemExist = true;
			}

			if (isItemExist)
			{
				isItemExist = false;
				continue;
			}
				
				
			currentItem = notparsedtransacarray[i][j];

			for (int k = 0; k < rowscount; k++)
			{
				for (int z = 0; z < columnscount; z++)
				{
					if (currentItem == notparsedtransacarray[k][z])
						itemCounter++;
				}
			}

			if (itemCounter >= minsup)
			{
				itemtable[indx].s0 = currentItem;
				itemtable[indx].s1 = itemCounter;
				++itemscount;
				++indx;
			}

            itemCounter = 0;
            isItemExist = false;
		}
	}

	// сортируем по убыванию частоты 
	for (int i = 0; i < indx; ++i)
	{
		for (int j = i + 1; j < indx; ++j)
		{
			if (itemtable[i].s1 < itemtable[j].s1)
			{
				cl_int2 buf = itemtable[i];
				itemtable[i] = itemtable[j];
				itemtable[j] = buf;
			}
		}
	}


	// повторно заполняем базы транзакций в соответствии с таблицей 
	for (int i = 0; i < rowscount; ++i)
	{
		int count = 0;
		for (int tableIndx = 0; tableIndx < indx; ++tableIndx)
		{		
			for (int j = 0; j < columnscount; ++j)
			{
				int tableitem = itemtable[tableIndx].s0;
				int baseitem = notparsedtransacarray[i][j];

				if (itemtable[tableIndx].s0 == notparsedtransacarray[i][j])
				{					
					parsedtransacarray[i][count] = notparsedtransacarray[i][j];
					++count;
					break;
				}
			}
		}
	}
}

//---------------------------------------------------------------------------------------------------
void fptreeGPU::transfomTranscactionToArray(const std::vector<Transaction>& in, int& commonItemCount, TransactionBase& out)
{
    // подсчет элементов
    std::set<int> allItemsSet;
    for (auto innerVec : in)
    {
        for (auto item : innerVec)
            allItemsSet.insert(item);
    }

    if (allItemsSet.size() != 0)
        commonItemCount = allItemsSet.size();

	// заполняем массив
	for (int i = 0; i < in.size(); ++i)
		for (int j = 0; j < in[i].size(); ++j)
			out[i][j] = in[i][j];
}

//---------------------------------------------------------------------------------------------------

bool fptreeGPU::findItemInArrayOfParentsIndx(int rowsBegin, int rowsEnd, int columnsRange, TransactionBase& transactionBase, int* out)
{
	// первая транзакция, значит сравнивать пока нечего
	if (rowsEnd == rowsBegin)
		return false;

	int matchesCount = 0; // счетчик совпадений
	int matchesRow = 0;   // строка макс совпадения
	for (int i = rowsBegin; i <= rowsEnd - 1; i++)
	{
		for (int j = 0; j <= columnsRange; j++)
		{
			if (transactionBase[i][j] == transactionBase[rowsEnd][j])
			{
				// запоминаем строку
				matchesRow = i;
				// увеличиваем счетчик совпадений
				matchesCount++;
			}
		}

		// если число совпадений равно длине диапазона столбцов
		if (matchesCount == columnsRange + 1)
		{
			// восстанавливаем транзакцию
			for (int i = 0; i < columnsRange; i++)
				out[i] = transactionBase[matchesRow][i];

			return true;
		}
		//обнуляем число совпадений
		else
			matchesCount = 0;
	}

	return false;
}

bool fptreeGPU::findItemInCondArrayOfParentsIndx(int threadIndx, int transacsize, int rowsBegin, int rowsEnd, int columnsRange, int* transactionBase, int* itemtransaction)
{
    // первая транзакция, значит сравнивать пока нечего
    if (rowsEnd == rowsBegin)
        return false;

    int matchesCount = 0; // счетчик совпадений
    int matchesRow = 0;   // строка макс совпадения
    for (int i = rowsBegin; i <= rowsEnd - transacsize; i += transacsize)
    {
        for (int j = 0; j <= columnsRange; j++)
        {
            if (transactionBase[i + j] == transactionBase[rowsEnd + j])
            {
                // запоминаем строку
                matchesRow = i;
                // увеличиваем счетчик совпадений
                matchesCount++;
            }
        }

        // если число совпадений равно длине диапазона столбцов
        if (matchesCount == columnsRange + 1)
        {
            // восстанавливаем транзакцию
            for (int i = 0; i <= columnsRange; i++)
            {
                int test = transactionBase[matchesRow + i];
                itemtransaction[threadIndx * transacsize + i] = transactionBase[matchesRow + i];
            }


            return true;
        }
        //обнуляем число совпадений
        else
            matchesCount = 0;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------

int fptreeGPU::getItemIndexInArrayOfParentsIndx(int item, int rangeEnd, int transactionsize, cl_int4* arrayOfParentsIndx, int* itemTransaction, int* itemPrefixPath)
{
	int matchesCount = 0;	// счетчик совпадений
	for (int i = 0; i < rangeEnd; i++)
	{
		if (arrayOfParentsIndx[i].s0 == item)
		{
			// извлекаем путь от корня до элемента
			retrievePrefixFromArrayOfParentsIndx(transactionsize, i, arrayOfParentsIndx, itemPrefixPath);

			//сравниваем транзакцию с префиксным путем
			for (int j = 0; j <= transactionsize - 1; j++)
			{				
				if (itemTransaction[j] == itemPrefixPath[j])
					matchesCount++;
			}

			// если совпадение полное - выходим
			if (matchesCount == transactionsize)
				return i;
			// обнуляем счетчик совпадений
			else
				matchesCount = 0;
		}
	}

}

int fptreeGPU::getItemIndexInCondArrayOfParentsIndx(int threadIndx, int shift, int condarraysize, int item, int rangeEnd, int transactionsize, cl_int4* arrayOfParentsIndx, int* itemTransaction, int* itemPrefixPath)
{
    int matchesCount = 0;	// счетчик совпадений

    for (int i = 0; i < rangeEnd; i++)
    {
        if (arrayOfParentsIndx[threadIndx * condarraysize + i].s0 == item)
        {
            // извлекаем путь от корня до элемента
			retrievePrefixFromCondArrayOfParentsIndx(threadIndx, shift, condarraysize, transactionsize, i, arrayOfParentsIndx, itemPrefixPath);

            //сравниваем транзакцию с префиксным путем
            for (int j = 0; j < transactionsize + 2 ; j++)
            {
                int test1 = itemTransaction[threadIndx * (transactionsize + 2) + j];
                int test2 = itemPrefixPath[threadIndx * (transactionsize + 2) + j];

                if (itemTransaction[threadIndx * (transactionsize + 2) + i] == itemPrefixPath[threadIndx * (transactionsize + 2) + i])
                    matchesCount++;
            }

            // если совпадение полное - выходим
            if (matchesCount == (transactionsize + 2))
                return i;
            // обнуляем счетчик совпадений и массив префиксов
            else
            {
                matchesCount = 0;
            }
                
        }
    }
}

//---------------------------------------------------------------------------------------------------
void fptreeGPU::retrievePrefixFromArrayOfParentsIndx(int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath)
{
	int root = -1;
	int parent = arrayOfParentsIndx[itemIndex].s2;
	int item = arrayOfParentsIndx[itemIndex].s0;
	bool isRetrieveEnds = false;
	int searchIndx = itemIndex;
	int insertIndx = 0;

	// если предок корень, то префикс будет состоять только из корня
	if (parent == root)
	{
		itemPrefixPath[insertIndx] = 0;
		return;
	}

	while (!isRetrieveEnds)
	{
		for (int i = searchIndx; i >= 0; i--)
		{
			if (parent == arrayOfParentsIndx[i].s1)
			{
				parent = arrayOfParentsIndx[i].s2;
				int test = arrayOfParentsIndx[i].s0;
				itemPrefixPath[insertIndx] = arrayOfParentsIndx[i].s0;
				insertIndx++;
			}
		}

		// если дошли до корня, выходим
		if (parent == root)
			isRetrieveEnds = true;
	}

	// инвертируем 
	int indx = transactionsize;
	int count = 0;

	while (itemPrefixPath[indx - 1] == 0)
	{
		count++;
		indx--;
	}

	for (int i = 0; i < (transactionsize - count) / 2; i++)
	{
		int tmp = itemPrefixPath[i];
		itemPrefixPath[i] = itemPrefixPath[(transactionsize - count) - i - 1];
		itemPrefixPath[(transactionsize - count) - i - 1] = tmp;
	}
}

void fptreeGPU::retrievePrefixFromCondArrayOfParentsIndx(int threadIndx, int shift, int condarraysize, int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath)
{
	int root = -1;
	int parent = arrayOfParentsIndx[threadIndx * condarraysize + itemIndex].s2;
	int item = arrayOfParentsIndx[threadIndx * condarraysize + itemIndex].s0;
	bool isRetrieveEnds = false;
	int searchIndx = itemIndex;
	int insertIndx = 0;

	// если предок корень, то префикс будет состоять только из самого элемента 
	if (parent == root)
	{
		itemPrefixPath[threadIndx * shift + insertIndx] = item;
		return;
	}
	
	while (!isRetrieveEnds)
	{
		for (int i = searchIndx; i >= 0; i--)
		{
			if (parent == arrayOfParentsIndx[threadIndx * condarraysize + i].s1 )
			{
				parent = arrayOfParentsIndx[threadIndx * condarraysize + i].s2;
				int test = arrayOfParentsIndx[threadIndx * condarraysize + i].s0;
				itemPrefixPath[threadIndx * (transactionsize + 2) + insertIndx] = arrayOfParentsIndx[threadIndx * condarraysize + i].s0;
				insertIndx++;
			}
		}

		// если дошли до корня, выходим
		if (parent == root)
			isRetrieveEnds = true;
	}

    //добавляем сам элемент
    itemPrefixPath[threadIndx * (transactionsize + 2) + insertIndx] = item;

	// инвертируем 
	int indx = transactionsize;
	int count = 0;

	while (itemPrefixPath[threadIndx * shift + indx - 1] == 0)
	{
		count++;
		indx--;
	}

	for (int i = threadIndx * shift; i < (transactionsize - count) / 2; i++)
	{
		int tmp = itemPrefixPath[i];
		itemPrefixPath[i] = itemPrefixPath[(transactionsize - count) - i - 1];
		itemPrefixPath[(transactionsize - count) - i - 1] = tmp;
	}
}

//---------------------------------------------------------------------------------------------------
void fptreeGPU::retrieveCondPrefixFromArrayOfParentsIndx(int threadIndx, int transactionsize, int itemIndex, cl_int4* arrayOfParentsIndx, int* itemPrefixPath)
{
	int root = -1;
	int parent = arrayOfParentsIndx[itemIndex].s2;
	int item = arrayOfParentsIndx[itemIndex].s0;
	bool isRetrieveEnds = false;
	int searchIndx = itemIndex;
	int insertIndx = 0;

	// если предок корень, то префикс будет состоять только из корня
	if (parent == root)
	{
		itemPrefixPath[threadIndx * transactionsize + insertIndx] = 0;
		return;
	}

	while (!isRetrieveEnds)
	{
		for (int i = searchIndx; i >= 0; i--)
		{
			if (parent == arrayOfParentsIndx[i].s1)
			{
				parent = arrayOfParentsIndx[i].s2;
				int test = arrayOfParentsIndx[i].s0;
				itemPrefixPath[threadIndx * transactionsize + insertIndx] = arrayOfParentsIndx[i].s0;
				insertIndx++;
			}
		}

		// если дошли до корня, выходим
		if (parent == root)
			isRetrieveEnds = true;
	}

	// инвертируем 
	int indx = transactionsize;
	int count = 0;

    for (int indx = (threadIndx * transactionsize + transactionsize - 1); indx > threadIndx * transactionsize; indx--)
    {
        if (itemPrefixPath[indx] == 0)
            count++;
        else
            break;
    }

	for (int i = 0; i < (transactionsize - count) / 2; i++)
	{
		int tmp = itemPrefixPath[threadIndx * transactionsize + i];
		itemPrefixPath[threadIndx * transactionsize + i] = itemPrefixPath[threadIndx * transactionsize + (transactionsize - count) - i - 1];
		itemPrefixPath[threadIndx * transactionsize + (transactionsize - count) - i - 1] = tmp;
	}
}

//---------------------------------------------------------------------------------------------------
int fptreeGPU::createArrayOfParentsIndx(TransactionBase& transactionbase,
						            	 int*     itemTransaction,
						            	 int*     itemPrefixPath,
						                 int      rowsCount,
						            	 int      columnsCount,
						            	 cl_int4* out
                                        )
{
	const int root = -1;
    const int rowsBegin = 0;
    int nextFreeIndx = 0; //следующий свободный индекс в массива
	int parentIndex = 0;  // индекс родителя
	int id = 1;           // индентификатор элемента

	bool isItemAdded = false;

	for (int rowsIndx = 0; rowsIndx < rowsCount; rowsIndx++)
	{
		for (int columnsIndx = 0; columnsIndx < columnsCount; columnsIndx++)
		{
			if (transactionbase[rowsIndx][columnsIndx] != 0)
			{
				// элемент следующий после корня
				if (columnsIndx == 0)
				{
					// такой элемент уже есть
					if (findItemInArrayOfParentsIndx(rowsBegin, rowsIndx, columnsIndx, transactionbase, itemTransaction))
					{
						// ищем его индекс
						int range = nextFreeIndx - 1;
						int itemIndx = getItemIndexInArrayOfParentsIndx(transactionbase[rowsIndx][columnsIndx], range, columnsCount, out, itemTransaction, itemPrefixPath);
						// элемент становится родителем
						parentIndex = itemIndx;
						// увеличиваем частоту элемента
						out[parentIndex].s3++;
					}
					// такого элемента нет
					else
					{
						// добавляемый на след. свободный индекс
						out[nextFreeIndx].s0 = transactionbase[rowsIndx][columnsIndx];
						// присваиваем уникальный индентификатор
						out[nextFreeIndx].s1 = id;
						// устанавливаем родителем корень
						out[nextFreeIndx].s2 = root;
						// увеличиваем частоту
						int freq = out[nextFreeIndx].s3;
						out[nextFreeIndx].s3++;
						// сам элемент становится родителем
						parentIndex = nextFreeIndx;

						// смещаем индекс на след.свободный
						nextFreeIndx++;
						// изменяем индентификатор
						id++;
					}

					// обнуляем массивы транзакций и префиксов
                    HelpTools::fillNull(columnsCount, itemTransaction);
                    HelpTools::fillNull(columnsCount, itemPrefixPath);
				}
				// элемент уже имеет предков, отличных от корня
				else
				{
					// такой элемент уже есть
					if (findItemInArrayOfParentsIndx(rowsBegin, rowsIndx, columnsIndx, transactionbase, itemTransaction))
					{
						// ищем его индекс
						int range = nextFreeIndx - 1;
						int itemIndx = getItemIndexInArrayOfParentsIndx(transactionbase[rowsIndx][columnsIndx], range, columnsCount, out, itemTransaction, itemPrefixPath);
						// элемент становится родителем
						parentIndex = itemIndx;
						// увеличиваем частоту элемента
						out[parentIndex].s3++;
					}
					// такого элемента нет
					else
					{
						// добавляемый на след. свободный индекс
						out[nextFreeIndx].s0 = transactionbase[rowsIndx][columnsIndx];
						// присваиваем уникальный индентификатор
						out[nextFreeIndx].s1 = id;
						// устанавливаем родителя

						int parent = out[parentIndex].s1;

						out[nextFreeIndx].s2 = out[parentIndex].s1;
						// увеличиваем частоту

						int freq = out[nextFreeIndx].s3;

						out[nextFreeIndx].s3++;
						// сам элемент становится родителем
						parentIndex = nextFreeIndx;

						// смещаем индекс на след.свободный
						nextFreeIndx++;
						// изменяем индентификатор
						id++;
					}

					// обнуляем массивы транзакций и префиксов
                    HelpTools::fillNull(columnsCount, itemTransaction);
                    HelpTools::fillNull(columnsCount, itemPrefixPath);
				}
			}
		}
	}

	return nextFreeIndx;
}

