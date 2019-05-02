
void fillNull( int start, int end, __global int* array)
{
    for (int i = start; i < end; i++)
        array[i] = 0;
}

void retrieveCondPrefixFromArrayOfParentsIndx( int threadIndx, 
											   int condItemTransactionArraySize, 
											   int itemIndex, 
											   __global int4* mainArrayOfParentsIndx, 
											   __global int* condItemTransaction)
{
	const int root = -1;
	int parent = mainArrayOfParentsIndx[itemIndex].s2;
	int item = mainArrayOfParentsIndx[itemIndex].s0;
	bool isRetrieveEnds = false;
	int searchIndx = itemIndex;
	int insertIndx = 0;

	// если предок корень, то префикс будет состоять только из корня
	if (parent == root)
	{
		condItemTransaction[threadIndx * condItemTransactionArraySize + insertIndx] = 0;
		return;
	}

	while (!isRetrieveEnds)
	{
		for (int i = searchIndx; i >= 0; i--)
		{
			if (parent == mainArrayOfParentsIndx[i].s1)
			{
				parent = mainArrayOfParentsIndx[i].s2;
				int test = mainArrayOfParentsIndx[i].s0;
				condItemTransaction[threadIndx * condItemTransactionArraySize + insertIndx] = mainArrayOfParentsIndx[i].s0;
				insertIndx++;
			}
		}

		// если дошли до корня, выходим
		if (parent == root)
			isRetrieveEnds = true;
	}

	// инвертируем 
	int indx = condItemTransactionArraySize;
	int count = 0;

    for (int indx = (threadIndx * condItemTransactionArraySize + condItemTransactionArraySize - 1); indx > threadIndx * condItemTransactionArraySize; indx--)
    {
        if (condItemTransaction[indx] == 0)
            count++;
        else
            break;
    }

	for (int i = 0; i < (condItemTransactionArraySize - count) / 2; i++)
	{
		int tmp = condItemTransaction[threadIndx * condItemTransactionArraySize + i];
		condItemTransaction[threadIndx * condItemTransactionArraySize + i] = condItemTransaction[threadIndx * condItemTransactionArraySize + (condItemTransactionArraySize - count) - i - 1];
		condItemTransaction[threadIndx * condItemTransactionArraySize + (condItemTransactionArraySize - count) - i - 1] = tmp;
	}
}

void createConditionalBasises( int threadIndx,
							   int commonItemCount,
							   int condItemTransactionArraySize,
							   int arrayOfParentsIndxSize,
							   __global int* condItemTransaction,
							   __global int2* itemtable,
							   __global int4* mainArrayOfParentsIndx,
							   __global int* condNotParsedTransactionBaseArrays )
{
	int item = itemtable[threadIndx].s0; // элемент, для кот. строятся условные базисы
	int itemParent = -1;
	int itemFreq = 0;

	int insertRowIndx = threadIndx * condItemTransactionArraySize * commonItemCount;
	int insertColumnIndx = 0;

	const int root = -1;
	
	for (int itemIndx = 0; itemIndx < arrayOfParentsIndxSize; itemIndx++)
	{
		if (item == mainArrayOfParentsIndx[itemIndx].s0)
		{
			// запоминаем ближайщего родителя элемента 
			itemParent = mainArrayOfParentsIndx[itemIndx].s2;
			itemFreq = mainArrayOfParentsIndx[itemIndx].s3;

			// если предок корень, то условный базис будет состоять только из частоты элемента, сразу записываем в массив
			if (itemParent == root)
			{
				condNotParsedTransactionBaseArrays[insertRowIndx + condItemTransactionArraySize - 1] = itemFreq;
				// увеличиваем индекс строки массива условных базисов
				insertRowIndx += condItemTransactionArraySize;
				continue;
			}
			
			// иначе формируем префиксный путь
			retrieveCondPrefixFromArrayOfParentsIndx(threadIndx, condItemTransactionArraySize, itemIndx, mainArrayOfParentsIndx, condItemTransaction);
			
			// добавляем сформированный путь в массив условных наборов
			int itemCount = 0;
			for (int k = 0; k < condItemTransactionArraySize; k++)
			{
				if (condItemTransaction[threadIndx * condItemTransactionArraySize + k] != 0)
				{
					condNotParsedTransactionBaseArrays[insertRowIndx + k] = condItemTransaction[threadIndx * condItemTransactionArraySize + k];
					itemCount++;
				}

				// добавляем в конец единичного базисного набора информацию о частоте элемента
				if (k == condItemTransactionArraySize - 1)
				{
					condNotParsedTransactionBaseArrays[insertRowIndx + k] = itemFreq;
				}
			}

			// увеличиваем индекс строки массива условных базисов
			insertRowIndx += condItemTransactionArraySize;
			//обнуляем единичный условный базис
			const int nullBeginRange = threadIndx * condItemTransactionArraySize;
			const int nullEndRange = threadIndx * condItemTransactionArraySize + condItemTransactionArraySize;
			
			fillNull(nullBeginRange, nullEndRange, condItemTransaction);
		}
	}
}

void parseConditionalbasisArray( int threadIndx, 
                                 int commonItemCount, 
                                 int minsup, 
                                 int condTIDCount,
                                 int condItemTransactionArraySize,
                                 __global int2* condItemTables,
                                 __global int* condNotParsedTransactionBaseArrays,
                                 __global int* condParsedTransactionBaseArrays )
{
	bool isItemExist = false; // признак, что элемент уже содержится в таблице
	int currentItem = -1;     // текущий рассматриваемый элемент
	int itemCounter = 0;      // частота встречаемости элемента
	int indx = 0;			  // индекс таблицы элементов

    int rowsBegin = threadIndx * condItemTransactionArraySize * commonItemCount;
    int rowsEnd = (threadIndx + 1) * condItemTransactionArraySize * commonItemCount;

    for (int i = rowsBegin; i < rowsEnd; i += condItemTransactionArraySize)
	{
		for (int j = 0; j < condItemTransactionArraySize - 1; j++)
		{
			if (condNotParsedTransactionBaseArrays[i + j] == 0)
				continue;

			// если такого элемента еще нет
			for (int k = 0; k < indx; k++)
			{
				if (condItemTables[threadIndx * commonItemCount + k].s0 == condNotParsedTransactionBaseArrays[i + j])
					isItemExist = true;
			}

			if (isItemExist)
			{
				isItemExist = false;
				continue;
			}

			currentItem = condNotParsedTransactionBaseArrays[i + j];

            //----------------------------------------------------------
			for (int k = rowsBegin; k < rowsEnd; k += condItemTransactionArraySize)
			{
				for (int z = 0; z < condItemTransactionArraySize - 1; z++)
				{
                    if (currentItem == condNotParsedTransactionBaseArrays[k + z])
						itemCounter += condNotParsedTransactionBaseArrays[k + condItemTransactionArraySize - 1];
				}
			}
            //----------------------------------------------------------

			if (itemCounter >= minsup)
			{
				condItemTables[threadIndx * commonItemCount + indx].s0 = currentItem;
				condItemTables[threadIndx * commonItemCount + indx].s1 = itemCounter;

				itemCounter = 0;
				indx++;
				isItemExist = false;
			}
		}
	}

    // сортируем по убыванию частоты (метод пузырька, позже посмотрю наиб.оптимальные способы)
	for (int i = 0; i < indx; i++)
	{
		for (int j = i + 1; j < indx; j++)
		{
			if (condItemTables[threadIndx * commonItemCount + i].s1 < condItemTables[threadIndx * commonItemCount + j].s1)
			{
				int2 buf = condItemTables[threadIndx * commonItemCount + i];
				condItemTables[threadIndx * commonItemCount + i] = condItemTables[threadIndx * commonItemCount + j];
				condItemTables[threadIndx * commonItemCount + j] = buf;
			}
		}
	}

    // повторно заполняем базы транзакций в соответствии с таблицей 
	for (int i = rowsBegin; i < rowsEnd; i += condItemTransactionArraySize)
	{
		int count = 0;

		for (int tableIndx = 0; tableIndx < indx; tableIndx++)
		{
			for (int j = 0; j < condItemTransactionArraySize - 2; j++)
			{
				if (condItemTables[threadIndx*commonItemCount + tableIndx].s0 == condNotParsedTransactionBaseArrays[i + j])
				{                
					condParsedTransactionBaseArrays[i + count] = condNotParsedTransactionBaseArrays[i + j];
					count++;
					break;
				}
			}
		}
	}

    for (int i = rowsBegin; i < rowsEnd; i += condItemTransactionArraySize)
    {
		condParsedTransactionBaseArrays[i + condItemTransactionArraySize - 1] = condNotParsedTransactionBaseArrays[i + condItemTransactionArraySize - 1];
    }

}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------



//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
bool findItemInCondArrayOfParentsIndx( int threadIndx, 
									   int condItemTransactionArraySize, 
									   int rowsBegin,
									   int rowsEnd, 
									   int columnsRange,
									   __global int* condParsedTransactionBaseArrays,
									   __global int* condItemTransaction )
{
    // первая транзакция, значит сравнивать пока нечего
    if (rowsEnd == rowsBegin)
        return false;

    int matchesCount = 0; // счетчик совпадений
    int matchesRow = 0;   // строка макс совпадения
    for (int i = rowsBegin; i <= rowsEnd - condItemTransactionArraySize; i += condItemTransactionArraySize)
    {
        for (int j = 0; j <= columnsRange; j++)
        {
            if (condParsedTransactionBaseArrays[i + j] == condParsedTransactionBaseArrays[rowsEnd + j])
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
                condItemTransaction[threadIndx * condItemTransactionArraySize + i] = condParsedTransactionBaseArrays[matchesRow + i];

            return true;
        }
        //обнуляем число совпадений
        else
            matchesCount = 0;
    }

    return false;
}

void retrievePrefixFromCondArrayOfParentsIndx( int threadIndx, 
											   int commonItemCount, 
											   int condArrayOfParentsIndxsSize, 
											   int condItemTransactionArraySize, 
											   int itemIndex, 
											   __global int4* condArrayOfParentsIndx, 
											   __global int* condItemPrefixPath )
{
	int root = -1;
	int parent = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + itemIndex].s2;
	int item = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + itemIndex].s0;
	bool isRetrieveEnds = false;
	int searchIndx = itemIndex;
	int insertIndx = 0;

	// если предок корень, то префикс будет состоять только из самого элемента 
	if (parent == root)
	{
		condItemPrefixPath[threadIndx * commonItemCount + insertIndx] = item;
		return;
	}
	
	while (!isRetrieveEnds)
	{
		for (int i = searchIndx; i >= 0; i--)
		{
			if (parent == condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s1 )
			{
				parent = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s2;
				int test = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s0;
				condItemPrefixPath[threadIndx * (condItemTransactionArraySize + 2) + insertIndx] = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s0;
				insertIndx++;
			}
		}

		// если дошли до корня, выходим
		if (parent == root)
			isRetrieveEnds = true;
	}

    //добавляем сам элемент
    condItemPrefixPath[threadIndx * (condItemTransactionArraySize + 2) + insertIndx] = item;

	// инвертируем 
	int indx = condItemTransactionArraySize;
	int count = 0;

	while (condItemPrefixPath[threadIndx * commonItemCount + indx - 1] == 0)
	{
		count++;
		indx--;
	}

	for (int i = threadIndx * commonItemCount; i < (condItemTransactionArraySize - count) / 2; i++)
	{
		int tmp = condItemPrefixPath[i];
		condItemPrefixPath[i] = condItemPrefixPath[(condItemTransactionArraySize - count) - i - 1];
		condItemPrefixPath[(condItemTransactionArraySize - count) - i - 1] = tmp;
	}
}

int getItemIndexInCondArrayOfParentsIndx( int threadIndx, 
										  int commonItemCount, 
										  int condArrayOfParentsIndxsSize,
										  int item,
										  int rangeEnd, 
										  int condItemTransactionArraySize, 
										  __global int4* condArrayOfParentsIndx, 
										  __global int* condItemTransaction, 
										  __global int* condItemPrefixPath )
{
    int matchesCount = 0;	// счетчик совпадений

    for (int i = 0; i < rangeEnd; i++)
    {
        if (condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s0 == item)
        {
            // извлекаем путь от корня до элемента
			retrievePrefixFromCondArrayOfParentsIndx(threadIndx, commonItemCount, condArrayOfParentsIndxsSize, condItemTransactionArraySize, i, condArrayOfParentsIndx, condItemPrefixPath);

            //сравниваем транзакцию с префиксным путем
            for (int j = 0; j < condItemTransactionArraySize + 2 ; j++)
            {
                if (condItemTransaction[threadIndx * (condItemTransactionArraySize + 2) + i] == condItemPrefixPath[threadIndx * (condItemTransactionArraySize + 2) + i])
                    matchesCount++;
            }

            // если совпадение полное - выходим
            if (matchesCount == (condItemTransactionArraySize + 2))
                return i;
            // обнуляем счетчик совпадений и массив префиксов
            else
                matchesCount = 0;     
        }
    }
}

void createCondArrayOfParentsIndx( int threadIndx,
                                    int commonItemCount,
                                    int condArrayOfParentsIndxsSize,
								    int condItemTransactionArraySize,
                                    __global int* condParsedTransactionBaseArrays,
                                    __global int* condItemTransaction,
                                    __global int* condItemPrefixPath,                          
                                    __global int2* itemTable,
                                    __global int4* condArrayOfParentsIndx )
{
	const int root   = -1;  // корень
    int insertIndx   = threadIndx * condArrayOfParentsIndxsSize;
    int rowsBegin    = threadIndx * condItemTransactionArraySize * commonItemCount;
    int rowsEnd      = (threadIndx + 1) * condItemTransactionArraySize * commonItemCount;
    int nextFreeIndx = 0; // следующий свободный индекс в массива
	int parentIndex  = 0; // индекс родителя
	int id           = 1; // индентификатор элемента

	for (int rowsIndx = rowsBegin; rowsIndx < rowsEnd; rowsIndx += condItemTransactionArraySize)
	{
		for (int columnsIndx = 0; columnsIndx < condItemTransactionArraySize - 1; columnsIndx++)
		{
            // если дошли до последнего элемента базы транзакций, добавляем целевой элемент (для которого строится условный массив индексов предков)
            if (columnsIndx == condItemTransactionArraySize - 2)
            {
                if (condParsedTransactionBaseArrays[rowsIndx] == 0)
                   continue;

                // добавляемый на след. свободный индекс
                int item = itemTable[threadIndx].s0;
                condArrayOfParentsIndx[insertIndx + nextFreeIndx].s0 = itemTable[threadIndx].s0;
                // присваиваем уникальный индентификатор
                condArrayOfParentsIndx[insertIndx + nextFreeIndx].s1 = id;
                // устанавливаем родителя
                int parent = condArrayOfParentsIndx[insertIndx + parentIndex].s1;
                condArrayOfParentsIndx[insertIndx + nextFreeIndx].s2 = condArrayOfParentsIndx[insertIndx + parentIndex].s1;
                // устанавливаем частоту
                condArrayOfParentsIndx[insertIndx + nextFreeIndx].s3 = 1;
                // смещаем индекс на след.свободный
                nextFreeIndx++;
                // изменяем индентификатор
                id++;
            }

			if (condParsedTransactionBaseArrays[rowsIndx + columnsIndx] != 0)
			{
				// элемент следующий после корня
				if (columnsIndx == 0)
				{
					if (findItemInCondArrayOfParentsIndx(threadIndx, condItemTransactionArraySize, rowsBegin, rowsIndx, columnsIndx, condParsedTransactionBaseArrays, condItemTransaction))
					{
						// ищем его индекс
						int range = nextFreeIndx - 1;
                        int item = condParsedTransactionBaseArrays[rowsIndx + columnsIndx];
						int itemIndx = getItemIndexInCondArrayOfParentsIndx(threadIndx, commonItemCount, condArrayOfParentsIndxsSize, condParsedTransactionBaseArrays[rowsIndx + columnsIndx], range, condItemTransactionArraySize - 2, condArrayOfParentsIndx, condItemTransaction, condItemPrefixPath);
						// элемент становится родителем
						parentIndex = itemIndx;
						// увеличиваем частоту элемента
						condArrayOfParentsIndx[insertIndx + parentIndex].s3 += condParsedTransactionBaseArrays[rowsIndx + condItemTransactionArraySize - 1];
					}
					// такого элемента нет
					else
					{
						// добавляемый на след. свободный индекс
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s0 = condParsedTransactionBaseArrays[rowsIndx + columnsIndx];
						// присваиваем уникальный индентификатор
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s1 = id;
						// устанавливаем родителем корень
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s2 = root;
						// увеличиваем частоту
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s3 += condParsedTransactionBaseArrays[rowsIndx + condItemTransactionArraySize - 1];
						// сам элемент становится родителем
						parentIndex = nextFreeIndx;

						// смещаем индекс на след.свободный
						nextFreeIndx++;
						// изменяем индентификатор
						id++;
					}

					// обнуляем массивы транзакций и префиксов
					fillNull(threadIndx * condItemTransactionArraySize, (threadIndx + 1) * condItemTransactionArraySize, condItemTransaction);
					fillNull(threadIndx * condItemTransactionArraySize, (threadIndx + 1) * condItemTransactionArraySize, condItemPrefixPath);
				}
				// элемент уже имеет предков, отличных от корня
				else
				{
					// такой элемент уже есть
					if (findItemInCondArrayOfParentsIndx(threadIndx, commonItemCount, rowsBegin, rowsIndx, columnsIndx, condParsedTransactionBaseArrays, condItemTransaction))
					{
						// ищем его индекс
						int range = nextFreeIndx - 1;
                        int item = condParsedTransactionBaseArrays[rowsIndx + columnsIndx];
						int itemIndx = getItemIndexInCondArrayOfParentsIndx(threadIndx, commonItemCount, condArrayOfParentsIndxsSize, condParsedTransactionBaseArrays[rowsIndx + columnsIndx], range, condItemTransactionArraySize - 2, condArrayOfParentsIndx, condItemTransaction, condItemPrefixPath);
						// элемент становится родителем
						parentIndex = itemIndx;
						// увеличиваем частоту элемента
						condArrayOfParentsIndx[insertIndx + parentIndex].s3 += condParsedTransactionBaseArrays[rowsIndx + condItemTransactionArraySize - 1];
					}
					// такого элемента нет
					else
					{
						// добавляемый на след. свободный индекс
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s0 = condParsedTransactionBaseArrays[rowsIndx + columnsIndx];
						// присваиваем уникальный индентификатор
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s1 = id;
						// устанавливаем родителя
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s2 = condArrayOfParentsIndx[insertIndx + parentIndex].s1;
						// увеличиваем частоту
						condArrayOfParentsIndx[insertIndx + nextFreeIndx].s3 += condParsedTransactionBaseArrays[rowsIndx + condItemTransactionArraySize - 1];
						// сам элемент становится родителем
						parentIndex = nextFreeIndx;
						// смещаем индекс на след.свободный
						nextFreeIndx++;
						// изменяем индентификатор
						id++;
					}

					// обнуляем массивы транзакций и префиксов
					fillNull(threadIndx * condItemTransactionArraySize, (threadIndx + 1) * condItemTransactionArraySize, condItemTransaction);
					fillNull(threadIndx * condItemTransactionArraySize, (threadIndx + 1) * condItemTransactionArraySize, condItemPrefixPath);
				}
			}
		}
	}
}

void retrieveFrequentItemSets( int threadIndx, 
                               int commonItemCount,  
                               int condItemTransactionArraySize,
                               int condArrayOfParentsIndxsSize, 
                               __global int4* condArrayOfParentsIndx, 
                               __global int2* itemTable, 
                               __global int2* condItemTables,
                               __global int* freqItemSet,
                               __global int* arrayOfFreqItemSets )
{

    const int root = -1;
    int parent     = 0;
    int searchIndx = 0;
    int itemFreq   = 0;
    int item       = itemTable[threadIndx].s0;
	int beginIndx  = threadIndx * (condItemTransactionArraySize + 1); // + 1 - для разделения частых наборов в массив (через 0)
	int endIndx    = (threadIndx + 1) * (condItemTransactionArraySize + 1); // + 1 - для разделения частых наборов в массив (через 0)
    int insertIndx = 0;
    
    int insertOutIndx = threadIndx * commonItemCount * (condItemTransactionArraySize + 1);
    bool isRetrieveEnds = false;

	    // проходим по всему условному массиву индексов предков
    for (int itemIndex = 0; itemIndex < condArrayOfParentsIndxsSize; itemIndex++)
    {
        if (condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + itemIndex].s0 == itemTable[threadIndx].s0)
        {
            insertIndx = 0;

            // добавляем сам элемент в набор
            freqItemSet[beginIndx + insertIndx] = item;
            insertIndx++;

            item = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + itemIndex].s0;
            parent = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + itemIndex].s2;

            searchIndx = itemIndex;
    
            // нахоим ближайщего родителя и берем его частоту (по свойству монотонности)
            for (int i = searchIndx; i >= 0; i--)
            {
                if (parent == condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s1)
                {
                    int finditem = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s0;
                    for (int i = 0; i < commonItemCount; i++)
                    {
                        if (finditem == condItemTables[threadIndx * commonItemCount + i].s0)
                        {
                            itemFreq = condItemTables[threadIndx * commonItemCount + i].s1;
                            break;
                        }
                    }                    
                }
            }

            // извлекаем предков
            while (!isRetrieveEnds)
            {
                for (int i = searchIndx; i >= 0; i--)
                {
                    if (parent == condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s1)
                    {
                        parent = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s2;
                        freqItemSet[beginIndx + insertIndx] = condArrayOfParentsIndx[threadIndx * condArrayOfParentsIndxsSize + i].s0;
                        insertIndx++;
                    }
                }

                // если дошли до корня, выходим
                if (parent == root)
                    isRetrieveEnds = true;
            }

            // инвертируем 
            int indx = condItemTransactionArraySize;
            int count = 0;

            while (freqItemSet[beginIndx + indx - 1] == 0)
            {
                count++;
                indx--;
            }

            for (int i = 0; i < (condItemTransactionArraySize - count) / 2; i++)
            {
                int tmp = freqItemSet[beginIndx + i];
                freqItemSet[beginIndx + i] = freqItemSet[beginIndx + (condItemTransactionArraySize - count) - i - 1];
                freqItemSet[beginIndx + (condItemTransactionArraySize - count) - i - 1] = tmp;
            }

            // добавляем в конец значение частоты набора
            freqItemSet[beginIndx + condItemTransactionArraySize - 1] = itemFreq;

            // записываем в выходной массив частых наборов
            for (int i = 0; i < condItemTransactionArraySize; i++)
            {
                arrayOfFreqItemSets[insertOutIndx + i] = freqItemSet[beginIndx + i];
            }
            // переходим на следующую строчку выходного массива частых наборов
            insertOutIndx += (condItemTransactionArraySize + 1);

            // очищаем частый набор
			fillNull(beginIndx, endIndx, freqItemSet);
            isRetrieveEnds = false;
        }
    }
}

//--------------------------------------------------------------------------------------------------------------------

__kernel void retrieveFreqPatternsKernel ( int minsupport,
                                           int condTIDCount,
                                           int commonItemCount, 
                                           int condItemTransactionArraySize,
                                           int arrayOfParentsIndxSize,
					                	   int condArrayOfParentsIndxsSize,
                                           __global int* condItemTransaction,
					                	   __global int* condItemPrefixPath,
					                	   __global int* condNotParsedTransactionBaseArrays,
                                           __global int* condParsedTransactionBaseArrays,
					                	   __global int* freqItemSet,
					                	   __global int* arrayOfFreqItemSets,
                                           __global int2* itemTable,
                                           __global int2* condItemTables,
                                           __global int4* mainArrayOfParentsIndx,
					                	   __global int4* condArrayOfParentsIndx 
						                  )

{
    int threadIndx = get_global_id(0);

    if (threadIndx >= commonItemCount) return; 

    createConditionalBasises(threadIndx, commonItemCount, condItemTransactionArraySize, arrayOfParentsIndxSize, condItemTransaction, itemTable, mainArrayOfParentsIndx, condNotParsedTransactionBaseArrays);
    barrier(CLK_GLOBAL_MEM_FENCE);

    parseConditionalbasisArray(threadIndx, commonItemCount, minsupport, condTIDCount, condItemTransactionArraySize, condItemTables, condNotParsedTransactionBaseArrays, condParsedTransactionBaseArrays);
	barrier(CLK_GLOBAL_MEM_FENCE);

	createCondArrayOfParentsIndx( threadIndx, commonItemCount, condArrayOfParentsIndxsSize, condItemTransactionArraySize, condParsedTransactionBaseArrays, condItemTransaction, condItemPrefixPath, itemTable, condArrayOfParentsIndx );
	barrier(CLK_GLOBAL_MEM_FENCE);
	
	retrieveFrequentItemSets( threadIndx, commonItemCount, condItemTransactionArraySize, condArrayOfParentsIndxsSize, condArrayOfParentsIndx, itemTable, condItemTables, freqItemSet, arrayOfFreqItemSets );
 }

