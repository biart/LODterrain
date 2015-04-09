#include "Common.h"
#include <vector>

#define QTREE_CHILD_NW 0
#define QTREE_CHILD_NE 1
#define QTREE_CHILD_SW 2
#define QTREE_CHILD_SE 3

#define QTREE_NORTH 0
#define QTREE_EAST 3
#define QTREE_SOUTH 2
#define QTREE_WEST 1

#define QTREE_CHILDREN_COUNT 4
#define QTREE_NEIGHBOURS_COUNT 4

#include "Common.h"
#include <vector>

template<class NodeData>
struct QuadTreeNode
{
	QuadTreeNode() : parent(nullptr)
	{
		for (int i = 0; i < QTREE_CHILDREN_COUNT; i++)
		{
			children[i] = nullptr;
		}
		for (int i = 0; i < QTREE_NEIGHBOURS_COUNT; i++)
		{
			neighbours[i] = nullptr;
		}
	}
	NodeData data;
	QuadTreeNode* children[QTREE_CHILDREN_COUNT];
	QuadTreeNode* neighbours[QTREE_NEIGHBOURS_COUNT];
	QuadTreeNode* parent;
};

template<typename NodeData>
class QuadTree
{
public:
	QuadTree() {}
	QuadTree(const NodeData& initdata) { heap.data = initdata; }
	~QuadTree() 
	{
		for (int i = 0; i < QTREE_CHILDREN_COUNT; i++)
			Heap().Remove(i);
	}
	struct Iterator
	{
	public:
		friend class QuadTree;

		bool operator==(const Iterator& op) const { return reference == op.reference; }
		NodeData* operator->() const { return &(reference->data); }
		operator bool() const { return reference; }

		Iterator Child(int index) const;
		Iterator Neighbour(int index) const;
		Iterator Add(int index, const NodeData& data = NodeData()) const;
		void Remove(int index, const NodeData& data = NodeData()) const;
		Iterator Parent() const;

		int Level()     const { return level;  }
		int LayerSize() const { return pow2(level - 1); }
		uvec2 Offset()  const { return offset; }
		vec2 OffsetFloat() const { return static_cast<vec3>(offset) / static_cast<float>(LayerSize()); }
	private:
		Iterator(QuadTreeNode<NodeData>* ref = nullptr, int level = 1, uvec2 offset = uvec2(0)) : 
			reference(ref), level(level), offset(offset) {}
		QuadTreeNode<NodeData>* reference;

		int level;
		uvec2 offset;
	};
	Iterator voidIterator = Iterator();
	Iterator Heap() { return QuadTree<NodeData>::Iterator(&heap); }
private:
	QuadTreeNode<NodeData> heap;
};

template<typename NodeData>
typename QuadTree<NodeData>::Iterator QuadTree<NodeData>::Iterator::Child(int index) const
{
	if (index < 0 || index >= QTREE_CHILDREN_COUNT)
		throw invalid_argument("Invalid index. It must be from 0 to 3.");
	uvec2 newOffset = uvec2(offset.x * 2, offset.y * 2);
	if (index == QTREE_CHILD_NW || index == QTREE_CHILD_SW)
		newOffset.x++;
	if (index == QTREE_CHILD_SW || index == QTREE_CHILD_SE)
		newOffset.y++;
	return QuadTree::Iterator(reference->children[index], level + 1, newOffset);
}

template<typename NodeData>
typename QuadTree<NodeData>::Iterator QuadTree<NodeData>::Iterator::Neighbour(int index) const
{
	if (index < 0 || index >= QTREE_NEIGHBOURS_COUNT)
		throw invalid_argument("Invalid index. It must be from 0 to 3.");
	uvec2 newOffset = offset;
	switch (index)
	{
	case QTREE_NORTH:
		newOffset.y--;
		break;
	case QTREE_SOUTH:
		newOffset.y++;
		break;
	case QTREE_WEST:
		newOffset.x++;
		break;
	case QTREE_EAST:
		newOffset.x--;
		break;
	}
	return QuadTree::Iterator(reference->neighbours[index], level, newOffset);
}

template<typename NodeData>
typename QuadTree<NodeData>::Iterator QuadTree<NodeData>::Iterator::Add(int index, const NodeData& data = NodeData()) const
{
	if (index < 0 || index >= QTREE_CHILDREN_COUNT)
		throw invalid_argument("Invalid index. It must be from 0 to 3.");

	//If child already exists, remove it
	if (reference->children[index]) Remove(index);
	reference->children[index] = new QuadTreeNode<NodeData>;
	reference->children[index]->parent = reference;
	reference->children[index]->data = data;

	//Fill neighbours pointers
	switch (index)
	{
	case QTREE_CHILD_NW:
		reference->children[index]->neighbours[QTREE_SOUTH] = reference->children[QTREE_CHILD_SW];
		reference->children[index]->neighbours[QTREE_EAST] = reference->children[QTREE_CHILD_NE];
		if (reference->neighbours[QTREE_NORTH])
			reference->children[index]->neighbours[QTREE_NORTH] = reference->neighbours[QTREE_NORTH]->children[QTREE_CHILD_SW];
		if (reference->neighbours[QTREE_WEST])
			reference->children[index]->neighbours[QTREE_WEST] = reference->neighbours[QTREE_WEST]->children[QTREE_CHILD_NE];
		break;
	case QTREE_CHILD_SW:
		reference->children[index]->neighbours[QTREE_NORTH] = reference->children[QTREE_CHILD_NW];
		reference->children[index]->neighbours[QTREE_EAST] = reference->children[QTREE_CHILD_SE];
		if (reference->neighbours[QTREE_SOUTH])
			reference->children[index]->neighbours[QTREE_SOUTH] = reference->neighbours[QTREE_SOUTH]->children[QTREE_CHILD_NW];
		if (reference->neighbours[QTREE_WEST])
			reference->children[index]->neighbours[QTREE_WEST] = reference->neighbours[QTREE_WEST]->children[QTREE_CHILD_SE];
		break;
	case QTREE_CHILD_NE:
		reference->children[index]->neighbours[QTREE_SOUTH] = reference->children[QTREE_CHILD_SE];
		reference->children[index]->neighbours[QTREE_WEST] = reference->children[QTREE_CHILD_NW];
		if (reference->neighbours[QTREE_NORTH])
			reference->children[index]->neighbours[QTREE_NORTH] = reference->neighbours[QTREE_NORTH]->children[QTREE_CHILD_SE];
		if (reference->neighbours[QTREE_EAST])
			reference->children[index]->neighbours[QTREE_EAST] = reference->neighbours[QTREE_EAST]->children[QTREE_CHILD_NW];
		break;
	case QTREE_CHILD_SE:
		reference->children[index]->neighbours[QTREE_NORTH] = reference->children[QTREE_CHILD_NE];
		reference->children[index]->neighbours[QTREE_WEST] = reference->children[QTREE_CHILD_SW];
		if (reference->neighbours[QTREE_SOUTH])
			reference->children[index]->neighbours[QTREE_SOUTH] = reference->neighbours[QTREE_SOUTH]->children[QTREE_CHILD_NE];
		if (reference->neighbours[QTREE_EAST])
			reference->children[index]->neighbours[QTREE_EAST] = reference->neighbours[QTREE_EAST]->children[QTREE_CHILD_SW];
		break;
	default:
		break;
	}
	if (reference->children[index]->neighbours[QTREE_SOUTH])
		reference->children[index]->neighbours[QTREE_SOUTH]->neighbours[QTREE_NORTH] = reference->children[index];
	if (reference->children[index]->neighbours[QTREE_NORTH])
		reference->children[index]->neighbours[QTREE_NORTH]->neighbours[QTREE_SOUTH] = reference->children[index];
	if (reference->children[index]->neighbours[QTREE_EAST])
		reference->children[index]->neighbours[QTREE_EAST]->neighbours[QTREE_WEST] = reference->children[index];
	if (reference->children[index]->neighbours[QTREE_WEST])
		reference->children[index]->neighbours[QTREE_WEST]->neighbours[QTREE_EAST] = reference->children[index];
	uvec2 newOffset = uvec2(offset.x * 2, offset.y * 2);
	if (index == QTREE_CHILD_NE || index == QTREE_CHILD_SE)
		newOffset.x++;
	if (index == QTREE_CHILD_SW || index == QTREE_CHILD_SE)
		newOffset.y++;
	return Iterator(reference->children[index], level + 1, newOffset);
}

template<typename NodeData>
typename void QuadTree<NodeData>::Iterator::Remove(int index, const NodeData& data = NodeData()) const
{
	if (index < 0 || index >= QTREE_CHILDREN_COUNT)
		throw invalid_argument("Invalid index. It must be from 0 to 3.");
	if (reference->children[index])
	{
		for (int i = 0; i < QTREE_CHILDREN_COUNT; i++)
			Child(index).Remove(i);
		delete reference->children[index];
		reference->children[index] = nullptr;
	}
}

template<typename NodeData>
typename QuadTree<NodeData>::Iterator QuadTree<NodeData>::Iterator::Parent() const
{
	return Iterator(reference->parent, level - 1, uvec2(offset.x / 2, offset.y / 2));
}
