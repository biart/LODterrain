#ifndef DENSE_QUAD_TREE_H
#define DENSE_QUAD_TREE_H

#include <memory>
#include "Common.h"
#include "Array2d.h"

#define QTREE_CHILDREN_COUNT 4
#define QTREE_NEIGHBOURS_COUNT 4

template<typename T>
class QuadTree
{
private:
    template <typename Ptr>
    struct TemplateIterator
    {
    public:
        friend class QuadTree;
        //Operators
        bool operator==(const TemplateIterator& op) const
        {
            return
                obj == op.obj &&
                level == op.level &&
                coord = op.coord;
        }
        T* operator->() const
        {
            if (!operator bool()) throw std::exception("Iterator is not dereferencable.");
            return obj->layers[level][coord].get();
        }
        operator bool() const
        {
            return
                (obj != nullptr) &&
                (level < obj->GetHeight()) && (level >= 0) &&
                (coord.x < obj->layers[level].GetSize().x) &&
                (coord.y < obj->layers[level].GetSize().y) &&
                (obj->layers[level][coord]);
        }

        //Get access to children, neighbours or parent
        uvec2 ChildOffset(int index) const
        {
            assert(index >= 0 || index < QTREE_CHILDREN_COUNT);
            uvec2 newOffset = uvec2(coord.x * 2, coord.y * 2);
            if (index == northWest || index == southWest)
                newOffset.x++;
            if (index == southWest || index == southEast)
                newOffset.y++;
            return newOffset;
        }
        TemplateIterator Child(int index) const
        {
            assert(index >= 0 || index < QTREE_CHILDREN_COUNT);
            return TemplateIterator(obj, level + 1, ChildOffset(index));
        }
        TemplateIterator Neighbour(int index) const
        {
            assert(index >= 0 || index < QTREE_NEIGHBOURS_COUNT);
            switch (index)
            {
            case north:
                return TemplateIterator(obj, level, coord - uvec2(1, 0));
            case east:
                return TemplateIterator(obj, level, coord + uvec2(0, 1));
            case south:
                return TemplateIterator(obj, level, coord + uvec2(1, 0));
            case west:
                return TemplateIterator(obj, level, coord - uvec2(0, 1));
            default:
                throw invalid_argument("Invalid index. It must be from 0 to 3.");
            }
        }
        TemplateIterator Parent() const
        {
            return Iterator(obj, level - 1, uvec2(coord.x / 2, coord.y / 2));
        }

        //Add or remove child
        TemplateIterator Add(int index, const T& data = T()) const
        {
            assert(index >= 0 || index < QTREE_CHILDREN_COUNT);
            if (!operator bool())
                throw std::exception("Iterator is not dereferencable.");

            if (level + 1 == obj->GetHeight()) obj->AddLayer();

            uvec2 newOffset = ChildOffset(index);
            obj->layers[level + 1][newOffset] = unique_ptr<T>(new T(data));
            return Iterator(obj, level + 1, newOffset);
        }
        void Remove() const
        {
            if (!operator bool())
                return;
            for (int i = 0; i < QTREE_CHILDREN_COUNT; i++)
                Child(i).Remove();
            delete obj->layers[level][coord].reset(nullptr);
        }

        //Get current node parameters
        int Level()     const { return level; }
        int LayerSize() const { return pow(2, level); }
        uvec2 Offset()  const { return coord; }
        vec2 OffsetFloat() const { return static_cast<vec2>(coord) / static_cast<float>(LayerSize()); }

    private:
        TemplateIterator(Ptr obj = nullptr, int level = 0, uvec2 coord = uvec2(0)) :
            obj(obj), level(level), coord(coord) {}
        int level;
        uvec2 coord;
        Ptr obj;
    };

public:
	// Constructor
	QuadTree(const T& initdata = T())
	{ 
		AddLayer();
		layers[0][uvec2(0)] = make_unique<T>(initdata);
	};
	// Destructor
	~QuadTree() { };

	// Get tree height
	int GetHeight() const { return layers.size(); }

    // Iterators
    typedef TemplateIterator<QuadTree*> Iterator;
    typedef TemplateIterator<const QuadTree*> ConstIterator;

    // Get heap
	Iterator Heap() { return Iterator(this); }

    // Iteration directions
    //   N  
    // W o E
    //   S
    static const int north = 3;
    static const int east = 2;
    static const int south = 1;
    static const int west = 0;
    // NW | NE
    // -------
    // SW | SE
    static const int northWest = 0;
    static const int northEast = 1;
    static const int southWest = 2;
    static const int southEast = 3;

private:
	QuadTree(const QuadTree<T>& a);
	QuadTree& operator=(const QuadTree<T>&);
	vector<Array2D<unique_ptr<T>>> layers;
	void AddLayer()
	{
		layers.push_back(Array2D<unique_ptr<T>>(
            uvec2(pow(2, layers.size())))
            );
	}
};

#endif // DENSE_QUAD_TREE_H

