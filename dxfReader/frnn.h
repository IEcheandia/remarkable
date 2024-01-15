#pragma once

#include <map>
#include <vector>
#include <cmath>
#include <cassert>
#include <cinttypes>

/**
Fixed radius nearest neighbor search helper.

About the problem: https://en.wikipedia.org/wiki/Fixed-radius_near_neighbors

In this implementation the "radius" is actually a rectangular "range" in x- and y-direction.
So a query for <i>x, y</i> will return at least the items in <i>(x - range, x + range) and
(y - range, y + range)</i>.
*/
template <class scalar_T, class val_T>
class Frnn2D
{
	class Index
	{
		std::size_t mOneBased = 0;
	public:
		Index() = default;
		explicit Index(size_t val) : mOneBased(val + 1) {}
		bool IsNull() const { return !mOneBased; }
		size_t Get() const { assert(!IsNull());  return mOneBased - 1; }
	};

	struct Item
	{
		val_T val;
		Index prev;
	};

	// This can be any unsigned integer type (it must be unsigned to be sure overflow is defined).
	// Smaller types lead to more collisions and decrease performance. 32bit should be a good practical choice.
	typedef std::uint32_t coord_t;

	typedef std::pair<coord_t, coord_t> Key;

	scalar_T const mRange;
	scalar_T const mInvRange;

	std::vector<Item> mItems;
	std::map<Key, Index> mMap;

public:

	/**
	Initializes an empty instance for a given range.

	@param range The range that applies to @ref QueryCandidates
	*/
	Frnn2D(scalar_T range) : mRange(range), mInvRange(1 / range)
	{}

	void Insert(scalar_T x, scalar_T y, val_T val)
	{
		Key key{
			static_cast<coord_t>(std::round(x * mInvRange)),
			static_cast<coord_t>(std::round(y * mInvRange))
		};

		Index & idx = mMap[key];
		mItems.push_back({ val, idx });
		idx = Index(mItems.size() - 1);
	}

	/**
	Queries all inserted items around the given position

	@param x,y Center of query position.
	@param f Will be called for all items in <i>(x - range, x + range) and (y - range, y + range)</i>.
	         May also be called for other closeby items that are slightly further away.
	*/
	template <class F>
	void QueryCandidates(scalar_T x, scalar_T y, F f) const
	{
		scalar_T const nx = x * mInvRange;
		scalar_T const ny = y * mInvRange;
		scalar_T const rx = std::round(nx);
		scalar_T const ry = std::round(ny);
		
		coord_t const xstart = static_cast<coord_t>(rx) - (nx < rx);
		coord_t const ystart = static_cast<coord_t>(ry) - (ny < ry);

		for (coord_t kx = xstart; kx != coord_t(xstart + 2u); ++kx)
		{
			for (coord_t ky = ystart; ky != coord_t(ystart + 2u); ++ky)
			{
				if (auto it = mMap.find(Key{ kx, ky }); it != mMap.end())
				{
					Item const * item = &mItems[it->second.Get()];
					f(item->val);

					while (!item->prev.IsNull())
					{
						item = &mItems[item->prev.Get()];
						f(item->val);
					}
				}
			}
		}
	}
};
