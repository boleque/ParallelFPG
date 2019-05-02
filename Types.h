#pragma once

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>
#include <CL/cl2.hpp>

using namespace std;

using Item				   = int;
using Pattern			   = pair<set<Item>, unsigned int>;
using PrefixPath		   = pair<vector<Item>, unsigned int>;
using Transaction		   = vector<Item>;
using TransactionBase      = vector<Transaction>;
using Array				   = vector<int>;
using ArrayOfParentsIndx   = vector<cl_int4>;
using ItemsTableGPU		   = vector<cl_int2>;
using TimeValues           = vector<double>;

//using TransactionOfAliases = vector<int>;	/**< Псевдоним единичной транзакции псевдонимов. */

/** \struct FPInfo
 *  \brief Структура объединяет информацию для узла FP-дерева.
 */
struct FPInfo {

	FPInfo::FPInfo(const Item& _item, const shared_ptr<FPInfo>& _parent) :
		item(_item),
		frequency(1),
		node_link(nullptr),
		parent(_parent),
		children()
	{
	}

	const Item item;
	unsigned frequency;
	shared_ptr<FPInfo> node_link;
	shared_ptr<FPInfo> parent;
	vector<shared_ptr<FPInfo>> children;
};

/** \struct FPInfo
 *  \brief Структура объединяет данные для узла FP-дерева.
 */
struct FPNode
{
	FPNode(const Item& _item, const std::shared_ptr<FPInfo>& _info) :
		item(_item),
		info(_info)
	{
	}

	bool operator== (const FPNode& rL) {

		if (&rL == this)
			return true;

		return (this->info == rL.info) && (this->item == rL.item);
	}

	bool operator== (const Item& rL) {

		return (this->item == rL);
	}

	const Item item;
	std::shared_ptr<FPInfo> info;
};

using FPTree =  vector< FPNode >;

/** \struct SortCriteria
 *  \brief Структура содрежит функтор, используемый для сортировки элементов тразакций по убыванию частоты.
 */
struct SortCriteria
{
	bool operator() (const std::pair<unsigned int, Item>& lhs, const std::pair<unsigned int, Item>& rhs) const
	{
		return (lhs.first > rhs.first);
	}
};

using ItemsTableCPU = set<pair<unsigned int, Item>, SortCriteria >;