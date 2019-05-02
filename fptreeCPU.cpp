#include "fptreeCPU.h"

#include <algorithm>
#include <cassert>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

fptreeCPU::fptreeCPU(const std::vector<Transaction>& transactions, unsigned int min_support) :
	m_root(std::make_shared<FPInfo>(Item(), nullptr)),
	m_fptree(),
	m_minSupp(min_support),
	m_transactions(transactions)
{
	createItemsTable();
	createFPTree();
}

void fptreeCPU::createItemsTable()
{
	unsigned int item_frequency = 0;
	std::map<Item, unsigned> frequencyByItemMap;

	// сканирование базы транзакций, подсчет элементов
	for (const Transaction& transaction : m_transactions)
	{
		for (const Item& item : transaction)
		{
			++frequencyByItemMap[item];
		}
	}

	// удаляем элементы не соотв. критерию minSup
	for (auto it = frequencyByItemMap.cbegin(); it != frequencyByItemMap.cend(); )
	{
		item_frequency = (*it).second;

		if (item_frequency < m_minSupp)
			frequencyByItemMap.erase(it++);
		else
			++it;
	}

	for (const auto& pair : frequencyByItemMap)
	{
		Item item = pair.first;
		item_frequency = pair.second;
		m_itemsTable.insert({ item_frequency, item });
	}
}

void fptreeCPU::createFPTree()
{
	for (const Transaction& transaction : m_transactions)
	{
		std::shared_ptr<FPInfo> currentNode = m_root;

		for (const auto& pair : m_itemsTable)
		{
			Item item = pair.second;

			// проверка что элемент содержится в текущей транзакции
			if (std::find(transaction.cbegin(), transaction.cend(), pair.second) != transaction.cend())
			{
				// проверка, что выбранный элемент является ребенком текущего узла
				const auto it = std::find_if(currentNode->children.cbegin(), currentNode->children.cend(), [item](const std::shared_ptr<FPInfo>& FPInfo)
				{
					return FPInfo->item == item;
				});

				// ребенка нет, создаем новый узел
				if (it == currentNode->children.cend())
				{
					++m_nodeCount;
					const std::shared_ptr<FPInfo> currNodeNewChild = std::make_shared<FPInfo>(item, currentNode); // новый ребенок текущего узла

					currentNode->children.push_back(currNodeNewChild);

					// обновляем fp-дерево
					auto it = find(m_fptree.begin(), m_fptree.end(), currNodeNewChild->item);
					if (it != m_fptree.end())
					{
						std::shared_ptr<FPInfo> prev_fpnode = it->info;
						while (prev_fpnode->node_link)
						{
							prev_fpnode = prev_fpnode->node_link;
						}

						prev_fpnode->node_link = currNodeNewChild;
					}
					else
					{
						FPNode newNode(currNodeNewChild->item, currNodeNewChild);
						m_fptree.push_back(newNode);
					}
					currentNode = currNodeNewChild;
				}
				else
				{
					std::shared_ptr<FPInfo> curr_fpnode_child = *it;
					++curr_fpnode_child->frequency;

					currentNode = curr_fpnode_child;
				}
			}
		}
	}
}

unsigned int fptreeCPU::getFPTreeSize()
{
	return m_nodeCount;
}

unsigned int fptreeCPU::getItemTableSize()
{
	return m_itemsTable.size();
}

void fptreeCPU::getFrequentItemSets(const std::shared_ptr<fptreeCPU> in, std::set<Pattern>& out)
{
	if (in->empty())
		return;

	std::vector<int> condSizesVec; // Число узлов в условных деревьях
	for (const auto& node : in->getFPTree())
	{
		//-------
		std::vector<PrefixPath> curItemPrefixPathsSets;  // Набор префиксов 
		std::shared_ptr<FPInfo> curItemNode = node.info; // Рассматриваемый узел
		createPrefixPathsSet(curItemNode, curItemPrefixPathsSets);
		//-------
		std::vector<Transaction> curItemConditionalTranscations; // Набор элементов, входящих в префиксы
		createConditionalTransactions(curItemPrefixPathsSets, curItemConditionalTranscations);
		//-------
		std::shared_ptr<fptreeCPU> curItemConditionalFPTree; // Условное fp-дерево элемента транзакции
		curItemConditionalFPTree = createConditionalFPTree(curItemConditionalTranscations, in->getMinSup());
		condSizesVec.push_back(curItemConditionalFPTree->getFPTreeSize());		
		//-------
		std::set<Pattern> curItemConditionalFrequentSet; // Условный популярный набор элемента
		getFrequentItemSets(curItemConditionalFPTree, curItemConditionalFrequentSet);
		//-------
		std::shared_ptr<FPInfo> fpnode = node.info;
		unsigned int curritemFrequency = 0; // Индекс элемента
		while (fpnode)
		{
			curritemFrequency += fpnode->frequency;
			fpnode = fpnode->node_link;
		}

		std::set<Pattern> curItemFrequentSets; // Популярные набор элемента
		Item curItem = node.item;             // Рассматриваемый элемент
		Pattern pattern{ { curItem }, curritemFrequency };
		curItemFrequentSets.insert(pattern);

		for (const Pattern& pattern : curItemConditionalFrequentSet)
		{
			Pattern newItemFrequentSet{ pattern };
			newItemFrequentSet.first.insert(curItem);
			assert(curritemFrequency >= pattern.second);
			newItemFrequentSet.second = pattern.second;

			curItemFrequentSets.insert({ newItemFrequentSet });
		}
		out.insert(curItemFrequentSets.cbegin(), curItemFrequentSets.cend());
	}

	m_condNodeCount = *std::max_element(condSizesVec.cbegin(), condSizesVec.cend());
}

void fptreeCPU::createPrefixPathsSet(const std::shared_ptr<FPInfo> in, std::vector<PrefixPath>& out)
{
	std::shared_ptr<FPInfo> startNode = in;

	while (startNode)
	{
		const unsigned startNodeFreq = startNode->frequency;
		std::shared_ptr<FPInfo> currNode = startNode->parent;

		if (currNode->parent)
		{
			PrefixPath PrefixPath{ {}, startNodeFreq };

			while (currNode->parent)
			{
				assert(currNode->frequency >= startNodeFreq);
				PrefixPath.first.push_back(currNode->item);

				currNode = currNode->parent;
			}
			out.push_back(PrefixPath);
		}
		startNode = startNode->node_link;
	}
}

std::shared_ptr<fptreeCPU> fptreeCPU::createConditionalFPTree(std::vector<Transaction>& conditionalTransac, unsigned int min_support)
{
	std::shared_ptr<fptreeCPU> conditionalTree(new fptreeCPU(conditionalTransac, min_support));
	return conditionalTree;
}

void fptreeCPU::createConditionalTransactions(const std::vector<PrefixPath>& in, std::vector<Transaction>& out)
{
	for (const PrefixPath& PrefixPath : in)
	{
		const std::vector<Item>& PrefixPathItems = PrefixPath.first;
		const unsigned PrefixPathItemsFreq = PrefixPath.second;

		Transaction transaction;
		for (const Item& item : PrefixPathItems)
			transaction.push_back(item);

		for (int i = 0; i < PrefixPathItemsFreq; ++i)
			out.push_back(transaction);
	}
}