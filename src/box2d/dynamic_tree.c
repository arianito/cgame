// SPDX-FileCopyrightText: 2023 Erin Catto
// SPDX-License-Identifier: MIT

#include "box2d/dynamic_tree.h"

#include "mem/mem.h"
#include "array.h"
#include "core.h"

#include "aabb.h"
#include "box2d/constants.h"

#include <float.h>
#include <string.h>

#define b2_treeStackSize 1024


// TODO_ERIN
// - try incrementally sorting internal nodes by height for better cache efficiency during depth first traversal.

static b2TreeNode b2_defaultTreeNode = {
	{{0.0f, 0.0f}, {0.0f, 0.0f}}, 0, {B2_NULL_INDEX}, B2_NULL_INDEX, B2_NULL_INDEX, -1, -2, false, {0, 0, 0, 0, 0, 0, 0, 0, 0}};

static inline bool b2IsLeaf(const b2TreeNode* node)
{
	return node->height == 0;
}

b2DynamicTree b2DynamicTree_Create(void)
{
	_Static_assert((sizeof(b2TreeNode) & 0xF) == 0, "tree node size not a multiple of 16");

	b2DynamicTree tree;
	tree.root = B2_NULL_INDEX;

	tree.nodeCapacity = 16;
	tree.nodeCount = 0;
	tree.nodes = (b2TreeNode*)xxmalloc(tree.nodeCapacity * sizeof(b2TreeNode));
	memset(tree.nodes, 0, tree.nodeCapacity * sizeof(b2TreeNode));

	// Build a linked list for the free list.
	for (int32_t i = 0; i < tree.nodeCapacity - 1; ++i)
	{
		tree.nodes[i].next = i + 1;
		tree.nodes[i].height = -1;
	}
	tree.nodes[tree.nodeCapacity - 1].next = B2_NULL_INDEX;
	tree.nodes[tree.nodeCapacity - 1].height = -1;
	tree.freeList = 0;

	tree.proxyCount = 0;

	tree.leafIndices = NULL;
	tree.leafBoxes = NULL;
	tree.leafCenters = NULL;
	tree.binIndices = NULL;
	tree.rebuildCapacity = 0;

	return tree;
}

void b2DynamicTree_Destroy(b2DynamicTree* tree)
{
	xxfree(tree->nodes, tree->nodeCapacity * sizeof(b2TreeNode));
	xxfree(tree->leafIndices, tree->rebuildCapacity * sizeof(int32_t));
	xxfree(tree->leafBoxes, tree->rebuildCapacity * sizeof(AABB));
	xxfree(tree->leafCenters, tree->rebuildCapacity * sizeof(Vec2));
	xxfree(tree->binIndices, tree->rebuildCapacity * sizeof(int32_t));

	memset(tree, 0, sizeof(b2DynamicTree));
}

void b2DynamicTree_Clone(b2DynamicTree* outTree, const b2DynamicTree* inTree)
{
	if (outTree->nodeCapacity < inTree->nodeCapacity)
	{
		xxfree(outTree->nodes, outTree->nodeCapacity * sizeof(b2TreeNode));
		outTree->nodeCapacity = inTree->nodeCapacity;
		outTree->nodes = (b2TreeNode*)xxmalloc(outTree->nodeCapacity * sizeof(b2TreeNode));
	}

	memcpy(outTree->nodes, inTree->nodes, inTree->nodeCapacity * sizeof(b2TreeNode));
	outTree->root = inTree->root;
	outTree->nodeCount = inTree->nodeCount;
	outTree->freeList = inTree->freeList;
	outTree->proxyCount = inTree->proxyCount;

	// Hook up free list.
	// TODO_ERIN make this optional?
	// TODO_ERIN perhaps find tail of existing free list and append
	int32_t inCapacity = inTree->nodeCapacity;
	int32_t outCapacity = outTree->nodeCapacity;
	if (outCapacity > inCapacity)
	{
		for (int32_t i = inCapacity; i < outCapacity - 1; ++i)
		{
			outTree->nodes[i].next = i + 1;
			outTree->nodes[i].height = -1;
		}
		outTree->nodes[outCapacity - 1].next = outTree->freeList;
		outTree->nodes[outCapacity - 1].height = -1;
		outTree->freeList = inCapacity;
	}
}

// Allocate a node from the pool. Grow the pool if necessary.
static int32_t b2AllocateNode(b2DynamicTree* tree)
{
	// Expand the node pool as needed.
	if (tree->freeList == B2_NULL_INDEX)
	{
		

		// The free list is empty. Rebuild a bigger pool.
		b2TreeNode* oldNodes = tree->nodes;
		int32_t oldCapcity = tree->nodeCapacity;
		tree->nodeCapacity += oldCapcity >> 1;
		tree->nodes = (b2TreeNode*)xxmalloc(tree->nodeCapacity * sizeof(b2TreeNode));
		memcpy(tree->nodes, oldNodes, tree->nodeCount * sizeof(b2TreeNode));
		xxfree(oldNodes, oldCapcity * sizeof(b2TreeNode));

		// Build a linked list for the free list. The parent
		// pointer becomes the "next" pointer.
		for (int32_t i = tree->nodeCount; i < tree->nodeCapacity - 1; ++i)
		{
			tree->nodes[i].next = i + 1;
			tree->nodes[i].height = -1;
		}
		tree->nodes[tree->nodeCapacity - 1].next = B2_NULL_INDEX;
		tree->nodes[tree->nodeCapacity - 1].height = -1;
		tree->freeList = tree->nodeCount;
	}

	// Peel a node off the free list.
	int32_t nodeIndex = tree->freeList;
	b2TreeNode* node = tree->nodes + nodeIndex;
	tree->freeList = node->next;
	*node = b2_defaultTreeNode;
	++tree->nodeCount;
	return nodeIndex;
}

// Return a node to the pool.
static void b2FreeNode(b2DynamicTree* tree, int32_t nodeId)
{
	
	
	tree->nodes[nodeId].next = tree->freeList;
	tree->nodes[nodeId].height = -1;
	tree->freeList = nodeId;
	--tree->nodeCount;
}

// Greedy algorithm for sibling selection using the SAH
// We have three nodes A-(B,C) and want to add a leaf D, there are three choices.
// 1: make a new parent for A and D : E-(A-(B,C), D)
// 2: associate D with B
//   a: B is a leaf : A-(E-(B,D), C)
//   b: B is an internal node: A-(B{D},C)
// 3: associate D with C
//   a: C is a leaf : A-(B, E-(C,D))
//   b: C is an internal node: A-(B, C{D})
// All of these have a clear cost except when B or C is an internal node. Hence we need to be greedy.

// The cost for cases 1, 2a, and 3a can be computed using the sibling cost formula.
// cost of sibling H = area(union(H, D)) + increased are of ancestors

// Suppose B (or C) is an internal node, then the lowest cost would be one of two cases:
// case1: D becomes a sibling of B
// case2: D becomes a descendant of B along with a new internal node of area(D).
static int32_t b2FindBestSibling(const b2DynamicTree* tree, AABB boxD)
{
	Vec2 centerD = aabb_center(boxD);
	float areaD = aabb_perimeter(boxD);

	const b2TreeNode* nodes = tree->nodes;
	int32_t rootIndex = tree->root;

	AABB rootBox = nodes[rootIndex].aabb;

	// Area of current node
	float areaBase = aabb_perimeter(rootBox);

	// Area of inflated node
	float directCost = aabb_perimeter(aabb_union(rootBox, boxD));
	float inheritedCost = 0.0f;

	int32_t bestSibling = rootIndex;
	float bestCost = directCost;

	// Descend the tree from root, following a single greedy path.
	int32_t index = rootIndex;
	while (nodes[index].height > 0)
	{
		int32_t child1 = nodes[index].child1;
		int32_t child2 = nodes[index].child2;

		// Cost of creating a new parent for this node and the new leaf
		float cost = directCost + inheritedCost;

		// Sometimes there are multiple identical costs within tolerance.
		// This breaks the ties using the centroid distance.
		if (cost < bestCost)
		{
			bestSibling = index;
			bestCost = cost;
		}

		// Inheritance cost seen by children
		inheritedCost += directCost - areaBase;

		bool leaf1 = nodes[child1].height == 0;
		bool leaf2 = nodes[child2].height == 0;

		// Cost of descending into child 1
		float lowerCost1 = FLT_MAX;
		AABB box1 = nodes[child1].aabb;
		float directCost1 = aabb_perimeter(aabb_union(box1, boxD));
		float area1 = 0.0f;
		if (leaf1)
		{
			// Child 1 is a leaf
			// Cost of creating new node and increasing area of node P
			float cost1 = directCost1 + inheritedCost;

			// Need this here due to while condition above
			if (cost1 < bestCost)
			{
				bestSibling = child1;
				bestCost = cost1;
			}
		}
		else
		{
			// Child 1 is an internal node
			area1 = aabb_perimeter(box1);

			// Lower bound cost of inserting under child 1.
			lowerCost1 = inheritedCost + directCost1 + minf(areaD - area1, 0.0f);
		}

		// Cost of descending into child 2
		float lowerCost2 = FLT_MAX;
		AABB box2 = nodes[child2].aabb;
		float directCost2 = aabb_perimeter(aabb_union(box2, boxD));
		float area2 = 0.0f;
		if (leaf2)
		{
			// Child 2 is a leaf
			// Cost of creating new node and increasing area of node P
			float cost2 = directCost2 + inheritedCost;

			// Need this here due to while condition above
			if (cost2 < bestCost)
			{
				bestSibling = child2;
				bestCost = cost2;
			}
		}
		else
		{
			// Child 2 is an internal node
			area2 = aabb_perimeter(box2);

			// Lower bound cost of inserting under child 2. This is not the cost
			// of child 2, it is the best we can hope for under child 2.
			lowerCost2 = inheritedCost + directCost2 + minf(areaD - area2, 0.0f);
		}

		if (leaf1 && leaf2)
		{
			break;
		}

		// Can the cost possibly be decreased?
		if (bestCost <= lowerCost1 && bestCost <= lowerCost2)
		{
			break;
		}

		if (lowerCost1 == lowerCost2 && leaf1 == false)
		{
			
			

			// No clear choice based on lower bound surface area. This can happen when both
			// children fully contain D. Fall back to node distance.
			Vec2 d1 = vec2_sub(aabb_center(box1), centerD);
			Vec2 d2 = vec2_sub(aabb_center(box2), centerD);
			lowerCost1 = vec2_sqr_length(d1);
			lowerCost2 = vec2_sqr_length(d2);
		}

		// Descend
		if (lowerCost1 < lowerCost2 && leaf1 == false)
		{
			index = child1;
			areaBase = area1;
			directCost = directCost1;
		}
		else
		{
			index = child2;
			areaBase = area2;
			directCost = directCost2;
		}

		
	}

	return bestSibling;
}

enum b2RotateType
{
	b2_rotateNone,
	b2_rotateBF,
	b2_rotateBG,
	b2_rotateCD,
	b2_rotateCE
};

// Perform a left or right rotation if node A is imbalanced.
// Returns the new root index.
static void b2RotateNodes(b2DynamicTree* tree, int32_t iA)
{
	

	b2TreeNode* nodes = tree->nodes;

	b2TreeNode* A = nodes + iA;
	if (A->height < 2)
	{
		return;
	}

	int32_t iB = A->child1;
	int32_t iC = A->child2;
	
	

	b2TreeNode* B = nodes + iB;
	b2TreeNode* C = nodes + iC;

	if (B->height == 0)
	{
		// B is a leaf and C is internal
		

		int32_t iF = C->child1;
		int32_t iG = C->child2;
		b2TreeNode* F = nodes + iF;
		b2TreeNode* G = nodes + iG;
		
		

		// Base cost
		float costBase = aabb_perimeter(C->aabb);

		// Cost of swapping B and F
		AABB aabbBG = aabb_union(B->aabb, G->aabb);
		float costBF = aabb_perimeter(aabbBG);

		// Cost of swapping B and G
		AABB aabbBF = aabb_union(B->aabb, F->aabb);
		float costBG = aabb_perimeter(aabbBF);

		if (costBase < costBF && costBase < costBG)
		{
			// Rotation does not improve cost
			return;
		}

		if (costBF < costBG)
		{
			// Swap B and F
			A->child1 = iF;
			C->child1 = iB;

			B->parent = iC;
			F->parent = iA;

			C->aabb = aabbBG;

			C->height = 1 + maxf(B->height, G->height);
			A->height = 1 + maxf(C->height, F->height);
			C->categoryBits = B->categoryBits | G->categoryBits;
			A->categoryBits = C->categoryBits | F->categoryBits;
			C->enlarged = B->enlarged || G->enlarged;
			A->enlarged = C->enlarged || F->enlarged;
		}
		else
		{
			// Swap B and G
			A->child1 = iG;
			C->child2 = iB;

			B->parent = iC;
			G->parent = iA;

			C->aabb = aabbBF;

			C->height = 1 + maxf(B->height, F->height);
			A->height = 1 + maxf(C->height, G->height);
			C->categoryBits = B->categoryBits | F->categoryBits;
			A->categoryBits = C->categoryBits | G->categoryBits;
			C->enlarged = B->enlarged || F->enlarged;
			A->enlarged = C->enlarged || G->enlarged;
		}
	}
	else if (C->height == 0)
	{
		// C is a leaf and B is internal
		

		int iD = B->child1;
		int iE = B->child2;
		b2TreeNode* D = nodes + iD;
		b2TreeNode* E = nodes + iE;
		
		

		// Base cost
		float costBase = aabb_perimeter(B->aabb);

		// Cost of swapping C and D
		AABB aabbCE = aabb_union(C->aabb, E->aabb);
		float costCD = aabb_perimeter(aabbCE);

		// Cost of swapping C and E
		AABB aabbCD = aabb_union(C->aabb, D->aabb);
		float costCE = aabb_perimeter(aabbCD);

		if (costBase < costCD && costBase < costCE)
		{
			// Rotation does not improve cost
			return;
		}

		if (costCD < costCE)
		{
			// Swap C and D
			A->child2 = iD;
			B->child1 = iC;

			C->parent = iB;
			D->parent = iA;

			B->aabb = aabbCE;

			B->height = 1 + maxf(C->height, E->height);
			A->height = 1 + maxf(B->height, D->height);
			B->categoryBits = C->categoryBits | E->categoryBits;
			A->categoryBits = B->categoryBits | D->categoryBits;
			B->enlarged = C->enlarged || E->enlarged;
			A->enlarged = B->enlarged || D->enlarged;
		}
		else
		{
			// Swap C and E
			A->child2 = iE;
			B->child2 = iC;

			C->parent = iB;
			E->parent = iA;

			B->aabb = aabbCD;
			B->height = 1 + maxf(C->height, D->height);
			A->height = 1 + maxf(B->height, E->height);
			B->categoryBits = C->categoryBits | D->categoryBits;
			A->categoryBits = B->categoryBits | E->categoryBits;
			B->enlarged = C->enlarged || D->enlarged;
			A->enlarged = B->enlarged || E->enlarged;
		}
	}
	else
	{
		int iD = B->child1;
		int iE = B->child2;
		int iF = C->child1;
		int iG = C->child2;

		b2TreeNode* D = nodes + iD;
		b2TreeNode* E = nodes + iE;
		b2TreeNode* F = nodes + iF;
		b2TreeNode* G = nodes + iG;

		
		
		
		

		// Base cost
		float areaB = aabb_perimeter(B->aabb);
		float areaC = aabb_perimeter(C->aabb);
		float costBase = areaB + areaC;
		enum b2RotateType bestRotation = b2_rotateNone;
		float bestCost = costBase;

		// Cost of swapping B and F
		AABB aabbBG = aabb_union(B->aabb, G->aabb);
		float costBF = areaB + aabb_perimeter(aabbBG);
		if (costBF < bestCost)
		{
			bestRotation = b2_rotateBF;
			bestCost = costBF;
		}

		// Cost of swapping B and G
		AABB aabbBF = aabb_union(B->aabb, F->aabb);
		float costBG = areaB + aabb_perimeter(aabbBF);
		if (costBG < bestCost)
		{
			bestRotation = b2_rotateBG;
			bestCost = costBG;
		}

		// Cost of swapping C and D
		AABB aabbCE = aabb_union(C->aabb, E->aabb);
		float costCD = areaC + aabb_perimeter(aabbCE);
		if (costCD < bestCost)
		{
			bestRotation = b2_rotateCD;
			bestCost = costCD;
		}

		// Cost of swapping C and E
		AABB aabbCD = aabb_union(C->aabb, D->aabb);
		float costCE = areaC + aabb_perimeter(aabbCD);
		if (costCE < bestCost)
		{
			bestRotation = b2_rotateCE;
			// bestCost = costCE;
		}

		switch (bestRotation)
		{
			case b2_rotateNone:
				break;

			case b2_rotateBF:
				A->child1 = iF;
				C->child1 = iB;

				B->parent = iC;
				F->parent = iA;

				C->aabb = aabbBG;
				C->height = 1 + maxf(B->height, G->height);
				A->height = 1 + maxf(C->height, F->height);
				C->categoryBits = B->categoryBits | G->categoryBits;
				A->categoryBits = C->categoryBits | F->categoryBits;
				C->enlarged = B->enlarged || G->enlarged;
				A->enlarged = C->enlarged || F->enlarged;
				break;

			case b2_rotateBG:
				A->child1 = iG;
				C->child2 = iB;

				B->parent = iC;
				G->parent = iA;

				C->aabb = aabbBF;
				C->height = 1 + maxf(B->height, F->height);
				A->height = 1 + maxf(C->height, G->height);
				C->categoryBits = B->categoryBits | F->categoryBits;
				A->categoryBits = C->categoryBits | G->categoryBits;
				C->enlarged = B->enlarged || F->enlarged;
				A->enlarged = C->enlarged || G->enlarged;
				break;

			case b2_rotateCD:
				A->child2 = iD;
				B->child1 = iC;

				C->parent = iB;
				D->parent = iA;

				B->aabb = aabbCE;
				B->height = 1 + maxf(C->height, E->height);
				A->height = 1 + maxf(B->height, D->height);
				B->categoryBits = C->categoryBits | E->categoryBits;
				A->categoryBits = B->categoryBits | D->categoryBits;
				B->enlarged = C->enlarged || E->enlarged;
				A->enlarged = B->enlarged || D->enlarged;
				break;

			case b2_rotateCE:
				A->child2 = iE;
				B->child2 = iC;

				C->parent = iB;
				E->parent = iA;

				B->aabb = aabbCD;
				B->height = 1 + maxf(C->height, D->height);
				A->height = 1 + maxf(B->height, E->height);
				B->categoryBits = C->categoryBits | D->categoryBits;
				A->categoryBits = B->categoryBits | E->categoryBits;
				B->enlarged = C->enlarged || D->enlarged;
				A->enlarged = B->enlarged || E->enlarged;
				break;

			default:
				
				break;
		}
	}
}

static void b2InsertLeaf(b2DynamicTree* tree, int32_t leaf, bool shouldRotate)
{
	if (tree->root == B2_NULL_INDEX)
	{
		tree->root = leaf;
		tree->nodes[tree->root].parent = B2_NULL_INDEX;
		return;
	}

	// Stage 1: find the best sibling for this node
	AABB leafAABB = tree->nodes[leaf].aabb;
	int32_t sibling = b2FindBestSibling(tree, leafAABB);

	// Stage 2: create a new parent for the leaf and sibling
	int32_t oldParent = tree->nodes[sibling].parent;
	int32_t newParent = b2AllocateNode(tree);

	// warning: node pointer can change after allocation
	b2TreeNode* nodes = tree->nodes;
	nodes[newParent].parent = oldParent;
	nodes[newParent].userData = -1;
	nodes[newParent].aabb = aabb_union(leafAABB, nodes[sibling].aabb);
	nodes[newParent].categoryBits = nodes[leaf].categoryBits | nodes[sibling].categoryBits;
	nodes[newParent].height = nodes[sibling].height + 1;

	if (oldParent != B2_NULL_INDEX)
	{
		// The sibling was not the root.
		if (nodes[oldParent].child1 == sibling)
		{
			nodes[oldParent].child1 = newParent;
		}
		else
		{
			nodes[oldParent].child2 = newParent;
		}

		nodes[newParent].child1 = sibling;
		nodes[newParent].child2 = leaf;
		nodes[sibling].parent = newParent;
		nodes[leaf].parent = newParent;
	}
	else
	{
		// The sibling was the root.
		nodes[newParent].child1 = sibling;
		nodes[newParent].child2 = leaf;
		nodes[sibling].parent = newParent;
		nodes[leaf].parent = newParent;
		tree->root = newParent;
	}

	// Stage 3: walk back up the tree fixing heights and AABBs
	int32_t index = nodes[leaf].parent;
	while (index != B2_NULL_INDEX)
	{
		int32_t child1 = nodes[index].child1;
		int32_t child2 = nodes[index].child2;

		
		

		nodes[index].aabb = aabb_union(nodes[child1].aabb, nodes[child2].aabb);
		nodes[index].categoryBits = nodes[child1].categoryBits | nodes[child2].categoryBits;
		nodes[index].height = 1 + maxf(nodes[child1].height, nodes[child2].height);
		nodes[index].enlarged = nodes[child1].enlarged || nodes[child2].enlarged;

		if (shouldRotate)
		{
			b2RotateNodes(tree, index);
		}

		index = nodes[index].parent;
	}
}

static void b2RemoveLeaf(b2DynamicTree* tree, int32_t leaf)
{
	if (leaf == tree->root)
	{
		tree->root = B2_NULL_INDEX;
		return;
	}

	b2TreeNode* nodes = tree->nodes;

	int32_t parent = nodes[leaf].parent;
	int32_t grandParent = nodes[parent].parent;
	int32_t sibling;
	if (nodes[parent].child1 == leaf)
	{
		sibling = nodes[parent].child2;
	}
	else
	{
		sibling = nodes[parent].child1;
	}

	if (grandParent != B2_NULL_INDEX)
	{
		// Destroy parent and connect sibling to grandParent.
		if (nodes[grandParent].child1 == parent)
		{
			nodes[grandParent].child1 = sibling;
		}
		else
		{
			nodes[grandParent].child2 = sibling;
		}
		nodes[sibling].parent = grandParent;
		b2FreeNode(tree, parent);

		// Adjust ancestor bounds.
		int32_t index = grandParent;
		while (index != B2_NULL_INDEX)
		{
			b2TreeNode* node = nodes + index;
			b2TreeNode* child1 = nodes + node->child1;
			b2TreeNode* child2 = nodes + node->child2;

			// Fast union using SSE
			//__m128 aabb1 = _mm_load_ps(&child1->aabb.min.x);
			//__m128 aabb2 = _mm_load_ps(&child2->aabb.min.x);
			//__m128 lower = _mm_min_ps(aabb1, aabb2);
			//__m128 upper = _mm_max_ps(aabb1, aabb2);
			//__m128 aabb = _mm_shuffle_ps(lower, upper, _MM_SHUFFLE(3, 2, 1, 0));
			//_mm_store_ps(&node->aabb.min.x, aabb);

			node->aabb = aabb_union(child1->aabb, child2->aabb);
			node->categoryBits = child1->categoryBits | child2->categoryBits;
			node->height = 1 + maxf(child1->height, child2->height);

			index = node->parent;
		}
	}
	else
	{
		tree->root = sibling;
		tree->nodes[sibling].parent = B2_NULL_INDEX;
		b2FreeNode(tree, parent);
	}
}

// Create a proxy in the tree as a leaf node. We return the index of the node instead of a pointer so that we can grow
// the node pool.
int32_t b2DynamicTree_CreateProxy(b2DynamicTree* tree, AABB aabb, uint32_t categoryBits, int32_t userData)
{
	
	
	
	

	int32_t proxyId = b2AllocateNode(tree);
	b2TreeNode* node = tree->nodes + proxyId;

	node->aabb = aabb;
	node->userData = userData;
	node->categoryBits = categoryBits;
	node->height = 0;

	bool shouldRotate = true;
	b2InsertLeaf(tree, proxyId, shouldRotate);

	tree->proxyCount += 1;

	return proxyId;
}

void b2DynamicTree_DestroyProxy(b2DynamicTree* tree, int32_t proxyId)
{
	
	

	b2RemoveLeaf(tree, proxyId);
	b2FreeNode(tree, proxyId);

	
	tree->proxyCount -= 1;
}

int32_t b2DynamicTree_GetProxyCount(const b2DynamicTree* tree)
{
	return tree->proxyCount;
}

void b2DynamicTree_MoveProxy(b2DynamicTree* tree, int32_t proxyId, AABB aabb)
{
	
	
	
	
	

	b2RemoveLeaf(tree, proxyId);

	tree->nodes[proxyId].aabb = aabb;

	bool shouldRotate = false;
	b2InsertLeaf(tree, proxyId, shouldRotate);
}

void b2DynamicTree_EnlargeProxy(b2DynamicTree* tree, int32_t proxyId, AABB aabb)
{
	b2TreeNode* nodes = tree->nodes;

	
	
	
	
	

	// Caller must ensure this
	

	nodes[proxyId].aabb = aabb;

	int32_t parentIndex = nodes[proxyId].parent;
	while (parentIndex != B2_NULL_INDEX)
	{
		bool changed = aabb_enlarge(&nodes[parentIndex].aabb, aabb);
		nodes[parentIndex].enlarged = true;
		parentIndex = nodes[parentIndex].parent;

		if (changed == false)
		{
			break;
		}
	}

	while (parentIndex != B2_NULL_INDEX)
	{
		if (nodes[parentIndex].enlarged == true)
		{
			// early out because this ancestor was previously ascended and marked as enlarged
			break;
		}

		nodes[parentIndex].enlarged = true;
		parentIndex = nodes[parentIndex].parent;
	}
}

int32_t b2DynamicTree_GetHeight(const b2DynamicTree* tree)
{
	if (tree->root == B2_NULL_INDEX)
	{
		return 0;
	}

	return tree->nodes[tree->root].height;
}

float b2DynamicTree_GetAreaRatio(const b2DynamicTree* tree)
{
	if (tree->root == B2_NULL_INDEX)
	{
		return 0.0f;
	}

	const b2TreeNode* root = tree->nodes + tree->root;
	float rootArea = aabb_perimeter(root->aabb);

	float totalArea = 0.0f;
	for (int32_t i = 0; i < tree->nodeCapacity; ++i)
	{
		const b2TreeNode* node = tree->nodes + i;
		if (node->height < 0 || b2IsLeaf(node) || i == tree->root)
		{
			// Free node in pool
			continue;
		}

		totalArea += aabb_perimeter(node->aabb);
	}

	return totalArea / rootArea;
}

// Compute the height of a sub-tree.
static int32_t b2ComputeHeight(const b2DynamicTree* tree, int32_t nodeId)
{
	
	b2TreeNode* node = tree->nodes + nodeId;

	if (b2IsLeaf(node))
	{
		return 0;
	}

	int32_t height1 = b2ComputeHeight(tree, node->child1);
	int32_t height2 = b2ComputeHeight(tree, node->child2);
	return 1 + maxf(height1, height2);
}

int32_t b2DynamicTree_ComputeHeight(const b2DynamicTree* tree)
{
	int32_t height = b2ComputeHeight(tree, tree->root);
	return height;
}


void b2DynamicTree_Validate(const b2DynamicTree* tree)
{
	B2_MAYBE_UNUSED(tree);
}

int32_t b2DynamicTree_GetMaxBalance(const b2DynamicTree* tree)
{
	int32_t maxBalance = 0;
	for (int32_t i = 0; i < tree->nodeCapacity; ++i)
	{
		const b2TreeNode* node = tree->nodes + i;
		if (node->height <= 1)
		{
			continue;
		}

		

		int32_t child1 = node->child1;
		int32_t child2 = node->child2;
		int32_t balance = absf(tree->nodes[child2].height - tree->nodes[child1].height);
		maxBalance = maxf(maxBalance, balance);
	}

	return maxBalance;
}

void b2DynamicTree_RebuildBottomUp(b2DynamicTree* tree)
{
	int32_t* nodes = (int32_t*)xxmalloc(tree->nodeCount * sizeof(int32_t));
	int32_t count = 0;

	// Build array of leaves. Free the rest.
	for (int32_t i = 0; i < tree->nodeCapacity; ++i)
	{
		if (tree->nodes[i].height < 0)
		{
			// free node in pool
			continue;
		}

		if (b2IsLeaf(tree->nodes + i))
		{
			tree->nodes[i].parent = B2_NULL_INDEX;
			nodes[count] = i;
			++count;
		}
		else
		{
			b2FreeNode(tree, i);
		}
	}

	while (count > 1)
	{
		float minCost = FLT_MAX;
		int32_t iMin = -1, jMin = -1;
		for (int32_t i = 0; i < count; ++i)
		{
			AABB aabbi = tree->nodes[nodes[i]].aabb;

			for (int32_t j = i + 1; j < count; ++j)
			{
				AABB aabbj = tree->nodes[nodes[j]].aabb;
				AABB b = aabb_union(aabbi, aabbj);
				float cost = aabb_perimeter(b);
				if (cost < minCost)
				{
					iMin = i;
					jMin = j;
					minCost = cost;
				}
			}
		}

		int32_t index1 = nodes[iMin];
		int32_t index2 = nodes[jMin];
		b2TreeNode* child1 = tree->nodes + index1;
		b2TreeNode* child2 = tree->nodes + index2;

		int32_t parentIndex = b2AllocateNode(tree);
		b2TreeNode* parent = tree->nodes + parentIndex;
		parent->child1 = index1;
		parent->child2 = index2;
		parent->aabb = aabb_union(child1->aabb, child2->aabb);
		parent->categoryBits = child1->categoryBits | child2->categoryBits;
		parent->height = 1 + maxf(child1->height, child2->height);
		parent->parent = B2_NULL_INDEX;

		child1->parent = parentIndex;
		child2->parent = parentIndex;

		nodes[jMin] = nodes[count - 1];
		nodes[iMin] = parentIndex;
		--count;
	}

	tree->root = nodes[0];
	xxfree(nodes, tree->nodeCount * sizeof(b2TreeNode));

	b2DynamicTree_Validate(tree);
}

void b2DynamicTree_ShiftOrigin(b2DynamicTree* tree, Vec2 newOrigin)
{
	// Build array of leaves. Free the rest.
	for (int32_t i = 0; i < tree->nodeCapacity; ++i)
	{
		b2TreeNode* n = tree->nodes + i;
		n->aabb.min.x -= newOrigin.x;
		n->aabb.min.y -= newOrigin.y;
		n->aabb.max.x -= newOrigin.x;
		n->aabb.max.y -= newOrigin.y;
	}
}

void b2DynamicTree_QueryFiltered(const b2DynamicTree* tree, AABB aabb, uint32_t maskBits, b2TreeQueryCallbackFcn* callback,
								 void* context)
{
	int32_t stack[b2_treeStackSize];
	int32_t stackCount = 0;
	stack[stackCount++] = tree->root;

	while (stackCount > 0)
	{
		int32_t nodeId = stack[--stackCount];
		if (nodeId == B2_NULL_INDEX)
		{
			continue;
		}

		const b2TreeNode* node = tree->nodes + nodeId;

		if (aabb_overlaps(node->aabb, aabb) && (node->categoryBits & maskBits) != 0)
		{
			if (b2IsLeaf(node))
			{
				// callback to user code with proxy id
				bool proceed = callback(nodeId, node->userData, context);
				if (proceed == false)
				{
					return;
				}
			}
			else
			{
				
				if (stackCount < b2_treeStackSize - 1)
				{
					stack[stackCount++] = node->child1;
					stack[stackCount++] = node->child2;
				}
			}
		}
	}
}

void b2DynamicTree_Query(const b2DynamicTree* tree, AABB aabb, b2TreeQueryCallbackFcn* callback, void* context)
{
	int32_t stack[b2_treeStackSize];
	int32_t stackCount = 0;
	stack[stackCount++] = tree->root;

	while (stackCount > 0)
	{
		int32_t nodeId = stack[--stackCount];
		if (nodeId == B2_NULL_INDEX)
		{
			continue;
		}

		const b2TreeNode* node = tree->nodes + nodeId;

		if (aabb_overlaps(node->aabb, aabb))
		{
			if (b2IsLeaf(node))
			{
				// callback to user code with proxy id
				bool proceed = callback(nodeId, node->userData, context);
				if (proceed == false)
				{
					return;
				}
			}
			else
			{
				
				if (stackCount < b2_treeStackSize - 1)
				{
					stack[stackCount++] = node->child1;
					stack[stackCount++] = node->child2;
				}
			}
		}
	}
}

void b2DynamicTree_RayCast(const b2DynamicTree* tree, const b2RayCastInput* input, uint32_t maskBits,
						   b2TreeRayCastCallbackFcn* callback, void* context)
{
	Vec2 p1 = input->origin;
	Vec2 d = input->translation;

	Vec2 r = vec2_norm(d);

	// v is perpendicular to the segment.
	Vec2 v = vec2_crossfv(1.0f, r);
	Vec2 abs_v = vec2_abs(v);

	// Separating axis for segment (Gino, p80).
	// |dot(v, p1 - c)| > dot(|v|, h)

	float maxFraction = input->maxFraction;

	Vec2 p2 = vec2_mul_add(p1, maxFraction, d);

	// Build a bounding box for the segment.
	AABB segmentAABB = {vec2_min(p1, p2), vec2_max(p1, p2)};

	int32_t stack[b2_treeStackSize];
	int32_t stackCount = 0;
	stack[stackCount++] = tree->root;

	b2RayCastInput subInput = *input;

	while (stackCount > 0)
	{
		int32_t nodeId = stack[--stackCount];
		if (nodeId == B2_NULL_INDEX)
		{
			continue;
		}

		const b2TreeNode* node = tree->nodes + nodeId;
		if (aabb_overlaps(node->aabb, segmentAABB) == false || (node->categoryBits & maskBits) == 0)
		{
			continue;
		}

		// Separating axis for segment (Gino, p80).
		// |dot(v, p1 - c)| > dot(|v|, h)
		// radius extension is added to the node in this case
		Vec2 c = aabb_center(node->aabb);
		Vec2 h = aabb_extents(node->aabb);
		float term1 = absf(vec2_dot(v, vec2_sub(p1, c)));
		float term2 = vec2_dot(abs_v, h);
		if (term2 < term1)
		{
			continue;
		}

		if (b2IsLeaf(node))
		{
			subInput.maxFraction = maxFraction;

			float value = callback(&subInput, nodeId, node->userData, context);

			if (value == 0.0f)
			{
				// The client has terminated the ray cast.
				return;
			}

			if (0.0f < value && value < maxFraction)
			{
				// Update segment bounding box.
				maxFraction = value;
				p2 = vec2_mul_add(p1, maxFraction, d);
				segmentAABB.min = vec2_min(p1, p2);
				segmentAABB.max = vec2_max(p1, p2);
			}
		}
		else
		{
			
			if (stackCount < b2_treeStackSize - 1)
			{
				// TODO_ERIN just put one node on the stack, continue on a child node
				// TODO_ERIN test ordering children by nearest to ray origin
				stack[stackCount++] = node->child1;
				stack[stackCount++] = node->child2;
			}
		}
	}
}

void b2DynamicTree_ShapeCast(const b2DynamicTree* tree, const b2ShapeCastInput* input, uint32_t maskBits,
							 b2TreeShapeCastCallbackFcn* callback, void* context)
{
	if (input->count == 0)
	{
		return;
	}

	AABB originAABB = {input->points[0], input->points[0]};
	for (int i = 1; i < input->count; ++i)
	{
		originAABB.min = vec2_min(originAABB.min, input->points[i]);
		originAABB.max = vec2_max(originAABB.max, input->points[i]);
	}

	Vec2 radius = {input->radius, input->radius};

	originAABB.min = vec2_sub(originAABB.min, radius);
	originAABB.max = vec2_add(originAABB.max, radius);

	Vec2 p1 = aabb_center(originAABB);
	Vec2 extension = aabb_extents(originAABB);

	// v is perpendicular to the segment.
	Vec2 r = input->translation;
	Vec2 v = vec2_crossfv(1.0f, r);
	Vec2 abs_v = vec2_abs(v);

	// Separating axis for segment (Gino, p80).
	// |dot(v, p1 - c)| > dot(|v|, h)

	float maxFraction = input->maxFraction;

	// Build total box for the shape cast
	Vec2 t = vec2_mulfv(maxFraction, input->translation);
	AABB totalAABB = {
		vec2_min(originAABB.min, vec2_add(originAABB.min, t)),
		vec2_max(originAABB.max, vec2_add(originAABB.max, t)),
	};

	b2ShapeCastInput subInput = *input;

	int32_t stack[b2_treeStackSize];
	int32_t stackCount = 0;
	stack[stackCount++] = tree->root;

	while (stackCount > 0)
	{
		int32_t nodeId = stack[--stackCount];
		if (nodeId == B2_NULL_INDEX)
		{
			continue;
		}

		const b2TreeNode* node = tree->nodes + nodeId;
		if (aabb_overlaps(node->aabb, totalAABB) == false || (node->categoryBits & maskBits) == 0)
		{
			continue;
		}

		// Separating axis for segment (Gino, p80).
		// |dot(v, p1 - c)| > dot(|v|, h)
		// radius extension is added to the node in this case
		Vec2 c = aabb_center(node->aabb);
		Vec2 h = vec2_add(aabb_extents(node->aabb), extension);
		float term1 = absf(vec2_dot(v, vec2_sub(p1, c)));
		float term2 = vec2_dot(abs_v, h);
		if (term2 < term1)
		{
			continue;
		}

		if (b2IsLeaf(node))
		{
			subInput.maxFraction = maxFraction;

			float value = callback(&subInput, nodeId, node->userData, context);

			if (value == 0.0f)
			{
				// The client has terminated the ray cast.
				return;
			}

			if (0.0f < value && value < maxFraction)
			{
				// Update segment bounding box.
				maxFraction = value;
				t = vec2_mulfv(maxFraction, input->translation);
				totalAABB.min = vec2_min(originAABB.min, vec2_add(originAABB.min, t));
				totalAABB.max = vec2_max(originAABB.max, vec2_add(originAABB.max, t));
			}
		}
		else
		{
			
			if (stackCount < b2_treeStackSize - 1)
			{
				// TODO_ERIN just put one node on the stack, continue on a child node
				// TODO_ERIN test ordering children by nearest to ray origin
				stack[stackCount++] = node->child1;
				stack[stackCount++] = node->child2;
			}
		}
	}
}

// Median split == 0, Surface area heurstic == 1
#define B2_TREE_HEURISTIC 0

#if B2_TREE_HEURISTIC == 0

// Median split heuristic
static int32_t b2PartitionMid(int32_t* indices, Vec2* centers, int32_t count)
{
	// Handle trivial case
	if (count <= 2)
	{
		return count / 2;
	}

	// TODO_ERIN SIMD?
	Vec2 lowerBound = centers[0];
	Vec2 upperBound = centers[0];

	for (int32_t i = 1; i < count; ++i)
	{
		lowerBound = vec2_min(lowerBound, centers[i]);
		upperBound = vec2_max(upperBound, centers[i]);
	}

	Vec2 d = vec2_sub(upperBound, lowerBound);
	Vec2 c = {0.5f * (lowerBound.x + upperBound.x), 0.5f * (lowerBound.y + upperBound.y)};

	// Partition longest axis using the Hoare partition scheme
	// https://en.wikipedia.org/wiki/Quicksort
	// https://nicholasvadivelu.com/2021/01/11/array-partition/
	int32_t i1 = 0, i2 = count;
	if (d.x > d.y)
	{
		float pivot = c.x;

		while (i1 < i2)
		{
			while (i1 < i2 && centers[i1].x < pivot)
			{
				i1 += 1;
			};

			while (i1 < i2 && centers[i2 - 1].x >= pivot)
			{
				i2 -= 1;
			};

			if (i1 < i2)
			{
				// Swap indices
				{
					int32_t temp = indices[i1];
					indices[i1] = indices[i2 - 1];
					indices[i2 - 1] = temp;
				}

				// Swap centers
				{
					Vec2 temp = centers[i1];
					centers[i1] = centers[i2 - 1];
					centers[i2 - 1] = temp;
				}

				i1 += 1;
				i2 -= 1;
			}
		}
	}
	else
	{
		float pivot = c.y;

		while (i1 < i2)
		{
			while (i1 < i2 && centers[i1].y < pivot)
			{
				i1 += 1;
			};

			while (i1 < i2 && centers[i2 - 1].y >= pivot)
			{
				i2 -= 1;
			};

			if (i1 < i2)
			{
				// Swap indices
				{
					int32_t temp = indices[i1];
					indices[i1] = indices[i2 - 1];
					indices[i2 - 1] = temp;
				}

				// Swap centers
				{
					Vec2 temp = centers[i1];
					centers[i1] = centers[i2 - 1];
					centers[i2 - 1] = temp;
				}

				i1 += 1;
				i2 -= 1;
			}
		}
	}
	

	if (i1 > 0 && i1 < count)
	{
		return i1;
	}
	else
	{
		return count / 2;
	}
}

#else

#define B2_BIN_COUNT 8

typedef struct b2TreeBin
{
	AABB aabb;
	int32_t count;
} b2TreeBin;

typedef struct b2TreePlane
{
	AABB leftAABB;
	AABB rightAABB;
	int32_t leftCount;
	int32_t rightCount;
} b2TreePlane;

// "On Fast Construction of SAH-based Bounding Volume Hierarchies" by Ingo Wald
// Returns the left child count
static int32_t b2PartitionSAH(int32_t* indices, int32_t* binIndices, AABB* boxes, int32_t count)
{
	

	b2TreeBin bins[B2_BIN_COUNT];
	b2TreePlane planes[B2_BIN_COUNT - 1];

	Vec2 center = aabb_center(boxes[0]);
	AABB centroidAABB;
	centroidAABB.min = center;
	centroidAABB.max = center;

	for (int32_t i = 1; i < count; ++i)
	{
		center = aabb_center(boxes[i]);
		centroidAABB.min = vec2_min(centroidAABB.min, center);
		centroidAABB.max = vec2_max(centroidAABB.max, center);
	}

	Vec2 d = vec2_sub(centroidAABB.max, centroidAABB.min);

	// Find longest axis
	int32_t axisIndex;
	float invD;
	if (d.x > d.y)
	{
		axisIndex = 0;
		invD = d.x;
	}
	else
	{
		axisIndex = 1;
		invD = d.y;
	}

	invD = invD > 0.0f ? 1.0f / invD : 0.0f;

	// Initialize bin bounds and count
	for (int32_t i = 0; i < B2_BIN_COUNT; ++i)
	{
		bins[i].aabb.min = vec2(FLT_MAX, FLT_MAX);
		bins[i].aabb.max = vec2(-FLT_MAX, -FLT_MAX);
		bins[i].count = 0;
	}

	// Assign boxes to bins and compute bin boxes
	// TODO_ERIN optimize
	float binCount = B2_BIN_COUNT;
	float lowerBoundArray[2] = {centroidAABB.min.x, centroidAABB.min.y};
	float minC = lowerBoundArray[axisIndex];
	for (int32_t i = 0; i < count; ++i)
	{
		Vec2 c = aabb_center(boxes[i]);
		float cArray[2] = {c.x, c.y};
		int32_t binIndex = (int32_t)(binCount * (cArray[axisIndex] - minC) * invD);
		binIndex = clampf(binIndex, 0, B2_BIN_COUNT - 1);
		binIndices[i] = binIndex;
		bins[binIndex].count += 1;
		bins[binIndex].aabb = aabb_union(bins[binIndex].aabb, boxes[i]);
	}

	int32_t planeCount = B2_BIN_COUNT - 1;

	// Prepare all the left planes, candidates for left child
	planes[0].leftCount = bins[0].count;
	planes[0].leftAABB = bins[0].aabb;
	for (int32_t i = 1; i < planeCount; ++i)
	{
		planes[i].leftCount = planes[i - 1].leftCount + bins[i].count;
		planes[i].leftAABB = aabb_union(planes[i - 1].leftAABB, bins[i].aabb);
	}

	// Prepare all the right planes, candidates for right child
	planes[planeCount - 1].rightCount = bins[planeCount].count;
	planes[planeCount - 1].rightAABB = bins[planeCount].aabb;
	for (int32_t i = planeCount - 2; i >= 0; --i)
	{
		planes[i].rightCount = planes[i + 1].rightCount + bins[i + 1].count;
		planes[i].rightAABB = aabb_union(planes[i + 1].rightAABB, bins[i + 1].aabb);
	}

	// Find best split to minimize SAH
	float minCost = FLT_MAX;
	int32_t bestPlane = 0;
	for (int32_t i = 0; i < planeCount; ++i)
	{
		float leftArea = aabb_perimeter(planes[i].leftAABB);
		float rightArea = aabb_perimeter(planes[i].rightAABB);
		int32_t leftCount = planes[i].leftCount;
		int32_t rightCount = planes[i].rightCount;

		float cost = leftCount * leftArea + rightCount * rightArea;
		if (cost < minCost)
		{
			bestPlane = i;
			minCost = cost;
		}
	}

	// Partition node indices and boxes using the Hoare partition scheme
	// https://en.wikipedia.org/wiki/Quicksort
	// https://nicholasvadivelu.com/2021/01/11/array-partition/
	int32_t i1 = 0, i2 = count;
	while (i1 < i2)
	{
		while (i1 < i2 && binIndices[i1] < bestPlane)
		{
			i1 += 1;
		};

		while (i1 < i2 && binIndices[i2 - 1] >= bestPlane)
		{
			i2 -= 1;
		};

		if (i1 < i2)
		{
			// Swap indices
			{
				int32_t temp = indices[i1];
				indices[i1] = indices[i2 - 1];
				indices[i2 - 1] = temp;
			}

			// Swap boxes
			{
				AABB temp = boxes[i1];
				boxes[i1] = boxes[i2 - 1];
				boxes[i2 - 1] = temp;
			}

			i1 += 1;
			i2 -= 1;
		}
	}
	

	if (i1 > 0 && i1 < count)
	{
		return i1;
	}
	else
	{
		return count / 2;
	}
}

#endif

// Temporary data used to track the rebuild of a tree node
struct b2RebuildItem
{
	int32_t nodeIndex;
	int32_t childCount;

	// Leaf indices
	int32_t startIndex;
	int32_t splitIndex;
	int32_t endIndex;
};

// Returns root node index
static int32_t b2BuildTree(b2DynamicTree* tree, int32_t leafCount)
{
	b2TreeNode* nodes = tree->nodes;
	int32_t* leafIndices = tree->leafIndices;

	if (leafCount == 1)
	{
		nodes[leafIndices[0]].parent = B2_NULL_INDEX;
		return leafIndices[0];
	}

#if B2_TREE_HEURISTIC == 0
	Vec2* leafCenters = tree->leafCenters;
#else
	AABB* leafBoxes = tree->leafBoxes;
	int32_t* binIndices = tree->binIndices;
#endif

	struct b2RebuildItem stack[b2_treeStackSize];
	int32_t top = 0;

	stack[0].nodeIndex = b2AllocateNode(tree);
	stack[0].childCount = -1;
	stack[0].startIndex = 0;
	stack[0].endIndex = leafCount;
#if B2_TREE_HEURISTIC == 0
	stack[0].splitIndex = b2PartitionMid(leafIndices, leafCenters, leafCount);
#else
	stack[0].splitIndex = b2PartitionSAH(leafIndices, binIndices, leafBoxes, leafCount);
#endif

	while (true)
	{
		struct b2RebuildItem* item = stack + top;

		item->childCount += 1;

		if (item->childCount == 2)
		{
			// This internal node has both children established

			if (top == 0)
			{
				// all done
				break;
			}

			struct b2RebuildItem* parentItem = stack + (top - 1);
			b2TreeNode* parentNode = nodes + parentItem->nodeIndex;

			if (parentItem->childCount == 0)
			{
				
				parentNode->child1 = item->nodeIndex;
			}
			else
			{
				
				
				parentNode->child2 = item->nodeIndex;
			}

			b2TreeNode* node = nodes + item->nodeIndex;

			
			node->parent = parentItem->nodeIndex;

			
			
			b2TreeNode* child1 = nodes + node->child1;
			b2TreeNode* child2 = nodes + node->child2;

			node->aabb = aabb_union(child1->aabb, child2->aabb);
			node->height = 1 + maxf(child1->height, child2->height);
			node->categoryBits = child1->categoryBits | child2->categoryBits;

			// Pop stack
			top -= 1;
		}
		else
		{
			int32_t startIndex, endIndex;
			if (item->childCount == 0)
			{
				startIndex = item->startIndex;
				endIndex = item->splitIndex;
			}
			else
			{
				
				startIndex = item->splitIndex;
				endIndex = item->endIndex;
			}

			int32_t count = endIndex - startIndex;

			if (count == 1)
			{
				int32_t childIndex = leafIndices[startIndex];
				b2TreeNode* node = nodes + item->nodeIndex;

				if (item->childCount == 0)
				{
					
					node->child1 = childIndex;
				}
				else
				{
					
					
					node->child2 = childIndex;
				}

				b2TreeNode* childNode = nodes + childIndex;
				
				childNode->parent = item->nodeIndex;
			}
			else
			{
				
				

				top += 1;
				struct b2RebuildItem* newItem = stack + top;
				newItem->nodeIndex = b2AllocateNode(tree);
				newItem->childCount = -1;
				newItem->startIndex = startIndex;
				newItem->endIndex = endIndex;
#if B2_TREE_HEURISTIC == 0
				newItem->splitIndex = b2PartitionMid(leafIndices + startIndex, leafCenters + startIndex, count);
#else
				newItem->splitIndex =
					b2PartitionSAH(leafIndices + startIndex, binIndices + startIndex, leafBoxes + startIndex, count);
#endif
				newItem->splitIndex += startIndex;
			}
		}
	}

	b2TreeNode* rootNode = nodes + stack[0].nodeIndex;
	
	
	

	b2TreeNode* child1 = nodes + rootNode->child1;
	b2TreeNode* child2 = nodes + rootNode->child2;

	rootNode->aabb = aabb_union(child1->aabb, child2->aabb);
	rootNode->height = 1 + maxf(child1->height, child2->height);
	rootNode->categoryBits = child1->categoryBits | child2->categoryBits;

	return stack[0].nodeIndex;
}

// Not safe to access tree during this operation because it may grow
int32_t b2DynamicTree_Rebuild(b2DynamicTree* tree, bool fullBuild)
{
	int32_t proxyCount = tree->proxyCount;
	if (proxyCount == 0)
	{
		return 0;
	}

	// Ensure capacity for rebuild space
	if (proxyCount > tree->rebuildCapacity)
	{
		int32_t newCapacity = proxyCount + proxyCount / 2;

		xxfree(tree->leafIndices, tree->rebuildCapacity * sizeof(int32_t));
		tree->leafIndices = xxmalloc(newCapacity * sizeof(int32_t));

#if B2_TREE_HEURISTIC == 0
		xxfree(tree->leafCenters, tree->rebuildCapacity * sizeof(Vec2));
		tree->leafCenters = xxmalloc(newCapacity * sizeof(Vec2));
#else
		xxfree(tree->leafBoxes, tree->rebuildCapacity * sizeof(AABB));
		tree->leafBoxes = xxmalloc(newCapacity * sizeof(AABB));
		xxfree(tree->binIndices, tree->rebuildCapacity * sizeof(int32_t));
		tree->binIndices = xxmalloc(newCapacity * sizeof(int32_t));
#endif
		tree->rebuildCapacity = newCapacity;
	}

	int32_t leafCount = 0;
	int32_t stack[b2_treeStackSize];
	int32_t stackCount = 0;

	int32_t nodeIndex = tree->root;
	b2TreeNode* nodes = tree->nodes;
	b2TreeNode* node = nodes + nodeIndex;

	// These are the nodes that get sorted to rebuild the tree.
	// I'm using indices because the node pool may grow during the build.
	int32_t* leafIndices = tree->leafIndices;

#if B2_TREE_HEURISTIC == 0
	Vec2* leafCenters = tree->leafCenters;
#else
	AABB* leafBoxes = tree->leafBoxes;
#endif

	// Gather all proxy nodes that have grown and all internal nodes that haven't grown. Both are
	// considered leaves in the tree rebuild.
	// Free all internal nodes that have grown.
	// todo use a node growth metric instead of simply enlarged to reduce rebuild size and frequency
	// this should be weighed against b2_aabbMargin
	while (true)
	{
		if (node->height == 0 || (node->enlarged == false && fullBuild == false))
		{
			leafIndices[leafCount] = nodeIndex;
#if B2_TREE_HEURISTIC == 0
			leafCenters[leafCount] = aabb_center(node->aabb);
#else
			leafBoxes[leafCount] = node->aabb;
#endif
			leafCount += 1;

			// Detach
			node->parent = B2_NULL_INDEX;
		}
		else
		{
			int32_t doomedNodeIndex = nodeIndex;

			// Handle children
			nodeIndex = node->child1;

			
			if (stackCount < b2_treeStackSize)
			{
				stack[stackCount++] = node->child2;
			}

			node = nodes + nodeIndex;

			// Remove doomed node
			b2FreeNode(tree, doomedNodeIndex);

			continue;
		}

		if (stackCount == 0)
		{
			break;
		}

		nodeIndex = stack[--stackCount];
		node = nodes + nodeIndex;
	}


	

	tree->root = b2BuildTree(tree, leafCount);

	b2DynamicTree_Validate(tree);

	return leafCount;
}
