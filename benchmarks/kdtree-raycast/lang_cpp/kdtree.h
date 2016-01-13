#pragma once

#include "bounding_box.h"
#include "ray.h"
#include "triangle.h"
#include "triangle_mesh.h"
#include "vector.h"
#include <cassert>
#include <cstdint>
#include <vector>

class KdTree {
  struct Node;

  KdTree(const KdTree&) = delete;
  KdTree& operator=(const KdTree&) = delete;

public:
  KdTree(std::vector<Node>&& nodes, std::vector<int32_t>&& triangleIndices,
         const TriangleMesh& mesh, const BoundingBox_f& meshBounds);
  KdTree(const std::string& kdtreeFileName, const TriangleMesh& mesh);
  void SaveToFile(const std::string& fileName) const;

  struct Intersection {
    double t = std::numeric_limits<double>::infinity();
    double epsilon = 0.0;
  };

  bool Intersect(const Ray& ray, Intersection& intersection) const;

  const TriangleMesh& GetMesh() const;
  const BoundingBox& GetMeshBounds() const;

private:
  void IntersectLeafTriangles(
      const Ray& ray, Node leaf,
      Triangle::Intersection& closestIntersection) const;

private:
  friend class KdTreeBuilder;

  enum { maxTraversalDepth = 64 };

  struct Node {
    uint32_t word0;
    uint32_t word1;

    enum : int32_t { maxNodesCount = 0x40000000 }; // max ~ 1 billion nodes
    enum : uint32_t { leafNodeFlags = 3 };

    void InitInteriorNode(int axis, int32_t aboveChild, float split)
    {
      // 0 - x axis, 1 - y axis, 2 - z axis
      assert(axis >= 0 && axis < 3);
      assert(aboveChild < maxNodesCount);

      word0 = axis | (uint32_t(aboveChild) << 2);
      word1 = *reinterpret_cast<uint32_t*>(&split);
    }

    void InitEmptyLeaf()
    {
      word0 = leafNodeFlags; // word0 == 3
      word1 = 0;             // not used for empty leaf, just set default value
    }

    void InitLeafWithSingleTriangle(int32_t triangleIndex)
    {
      word0 = leafNodeFlags | (1 << 2); // word0 == 7
      word1 = static_cast<uint32_t>(triangleIndex);
    }

    void InitLeafWithMultipleTriangles(int32_t numTriangles,
                                       int32_t triangleIndicesOffset)
    {
      assert(numTriangles > 1);
      // word0 == 11, 15, 19, ... (for numTriangles = 2, 3, 4, ...)
      word0 = leafNodeFlags | (numTriangles << 2);
      word1 = triangleIndicesOffset;
    }

    bool IsLeaf() const { return (word0 & leafNodeFlags) == leafNodeFlags; }

    bool IsInteriorNode() const { return !IsLeaf(); }

    int32_t GetTrianglesCount() const
    {
      assert(isLeaf());
      return static_cast<int32_t>(word0 >> 2);
    }

    int32_t GetIndex() const
    {
      assert(isLeaf());
      return static_cast<int32_t>(word1);
    }

    int GetSplitAxis() const
    {
      assert(isInteriorNode());
      return word0 & leafNodeFlags;
    }

    float GetSplitPosition() const
    {
      assert(isInteriorNode());
      return *reinterpret_cast<const float*>(&word1);
    }

    int32_t GetAboveChild() const
    {
      assert(isInteriorNode());
      return static_cast<int32_t>(word0 >> 2);
    }
  };

private:
  const std::vector<Node> _nodes;
  const std::vector<int32_t> _triangleIndices;

  const TriangleMesh& _mesh;
  const BoundingBox _meshBounds;
};
