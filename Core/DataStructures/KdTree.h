#pragma once

#include <vector>
#include <unordered_set>
#include <queue>
#include <algorithm>
#include <execution>

#include "glm.hpp"
#include "gtx/fast_trigonometry.hpp"

#include "MortonCode.h"

#define ROOT_INDEX 1

namespace {

	template<typename T>
	T sqr(T t) { return t * t; }

	inline uint32_t clz(uint32_t code)
	{
		return __lzcnt(code);
	}

	inline int fast_log2(int x) {
		if (x < 1)
			return -1;
		return 8 * sizeof(int) - clz(static_cast<uint32_t>(x)) - 1;
	}

	int max_axis(glm::vec3 v) {
		if (v.x > v.y) {
			return v.x > v.z ? 0 : 2;
		}
		else {
			return v.y > v.z ? 1 : 2;
		}
	}

	template<class T>
	class NQueue {
		std::priority_queue<T> q;
		const size_t max_size;

	public:
		NQueue(const unsigned int in_size) : max_size(static_cast<unsigned int>(in_size)) {}

		void push(const T& in_item) {
			q.push(in_item);
			if (q.size() > max_size) {
				// remove worst element, if the size of the queue is heigher than max
				q.pop();
			}
		}

		unsigned int size() const {
			return static_cast<unsigned int>(q.size());
		}

		bool at_capacity() const {
			return q.size() == max_size;
		}

		const T& top() const {
			return q.top();
		}


		void to_vector(std::vector<T>& out_vec) {
			const size_t N = q.size();
			out_vec.resize(N);
			for (size_t i = 0; i < N; i++) {
				// fill vector from the back, to get the best first
				out_vec[N - 1 - i] = q.top();
				q.pop();
			}
		}


	};
}


template<class KeyT, class ValT>
struct KDTreeRecord {
	float dist;
	KeyT key;
	ValT val;
	KDTreeRecord(float in_dist, KeyT in_key, ValT in_val) : dist(in_dist), key(in_key), val(in_val) {}
	KDTreeRecord() : dist(0.0f), key(), val() {}
};

template<class KeyT, class ValT>
bool operator < (const KDTreeRecord<KeyT, ValT>& a, const KDTreeRecord<KeyT, ValT>& b) {
	return a.dist < b.dist;
}


template<class KeyT>
inline float SqrDist(const KeyT& k1, const KeyT& k2) {
	KeyT diff = k1 - k2;
	return dot(diff, diff);
}

template<>
inline float SqrDist(const glm::vec2& k1, const glm::vec2& k2) {
	glm::vec2 diff = k1 - k2;
	return glm::dot(diff, diff);
}

template<>
inline float SqrDist(const glm::vec3& k1, const glm::vec3& k2) {
	glm::vec3 diff = k1 - k2;
	return glm::dot(diff, diff);
}

template<>
inline float SqrDist(const glm::vec4& k1, const glm::vec4& k2) {
	glm::vec4 diff = k1 - k2;
	return glm::dot(diff, diff);
}

/// Uses the KeyT to map ValT in the KdTree. Example of use could be KeyT = Vec3f and ValT = LightSource*, where the Vec3f is used to map the lights by position
/// The KeyT class is assumed to be an extension of the ArithVec class, but can also consist of others, as long as the used methods are implemented.
template<class KeyT, class ValT, unsigned int K>
class KdTree
{
private:

	class KdNode {
	public:
		KeyT key;
		ValT val;
		short axis;

		KdNode() : axis(-1) {}
		KdNode(const KeyT& in_key, const ValT& in_val) : key(in_key), val(in_val), axis(-1) {}
		~KdNode() {}

		inline float Dist(const KeyT& other) const {
			return sqrt(SqrDist(key, other.key));
		}

	};

	class KeyAxisCompare {
	private:
		short axis;

	public:
		KeyAxisCompare() : axis(-1) {}
		KeyAxisCompare(short in_axis) : axis(in_axis) {}
		bool operator() (const KdNode& k0, const KdNode& k1) const {
			return k0.key[axis] < k1.key[axis];
		}

		bool operator() (const KeyT& k0, const KeyT& k1) const {
			return k0[axis] < k1[axis];
		}
	};

	KeyAxisCompare axisCompare[K];

	bool isBuild = false;

	std::vector<KdNode> nodes;
	std::vector<KdNode> keys;

public:
	KdTree();
	~KdTree();

	bool Nearest(const KeyT& in_key, const ValT& in_val, float& in_out_dist, KeyT& out_key, ValT& out_val) const;
	bool Nearest(const unsigned int in_node, float& in_out_dist, KeyT& out_key, ValT& out_val) const;
	bool NNearest(const KeyT& in_key, const ValT& in_val, float& in_out_dist, std::vector<KDTreeRecord<KeyT, ValT>>& out_vector, const unsigned int N) const; // will ignore self
	unsigned int NearestExceptSelf(const KeyT& in_key, float& in_out_dist, KeyT& out_key, ValT& out_val, unsigned int in_self) const;
	unsigned int NearestExcept(const KeyT& in_key, float& in_out_dist, KeyT& out_key, ValT& out_val, const std::unordered_set<unsigned int>& in_except) const;

	void Reserve(const size_t size) { keys.reserve(size); }
	void Insert(const KeyT& in_key, const ValT& in_val);
	void ExcangeVal(const ValT& in_val, const unsigned int in_index);
	//void remove(const T*);

	void Build();

	/// Same functionallity as Build(), but is using Z-Order curve to sort primitives
	void BuildZOrder();

	//const T* Nearest(const T*) const;

	unsigned int Size() const;
	bool IsBuild() const;

	/// find a index of an element matching the key and value. if none is found zero is returned.
	unsigned int Find(const KeyT& in_key, const ValT& in_val);
	std::vector<std::pair<unsigned int, ValT>> GetNodes();

	//unsigned int depth() const;

private:

	void BuildRecurse(const unsigned int in_current, const unsigned int in_begin, const unsigned int in_end);
	/// will return the index of the nearest node in the tree within the distance 'dist', if present. 
	void NearestRecurse(const unsigned int in_node, const KeyT& in_key, const ValT& in_val, float& in_out_sqr_dist, unsigned int& out_node) const;
	void NNearestRecurse(const unsigned int in_node, const KeyT& in_key, const ValT& in_val, float& in_out_dist, NQueue<KDTreeRecord<KeyT, ValT>>& out_queue) const;
	unsigned int NearestRecurseExceptSelf(const unsigned int in_node, const KeyT& in_key, float& in_out_dist, unsigned int in_self) const;
	unsigned int NearestRecurseExcept(const unsigned int in_node, const KeyT& in_key, float& in_out_dist, const std::unordered_set<unsigned int>& in_except) const;
	int OptimalAxis(const unsigned int in_begin, const unsigned int in_end) const;

	unsigned int FindRecurse(const unsigned int in_node, const KeyT& in_key, const ValT& in_val) const;


};



template<class KeyT, class ValT, unsigned int K>
inline KdTree<KeyT, ValT, K>::KdTree()
{
	isBuild = false;
	nodes = std::vector<KdNode>(1);
	keys = std::vector<KdNode>(1);
	for (int i = 0; i < K; i++) {
		axisCompare[i] = KeyAxisCompare(i);
	}
}


template<class KeyT, class ValT, unsigned int K>
inline KdTree<KeyT, ValT, K>::~KdTree()
{
}

template<class KeyT, class ValT, unsigned int K>
inline bool KdTree<KeyT, ValT, K>::Nearest(const KeyT& in_key, const ValT& in_val, float& in_out_dist, KeyT& out_key, ValT& out_val) const
{
	// std::cout << "Nearest:" << std::endl;
	// check that the tree is in fact build since last insert
	assert(isBuild);

	if (nodes.size() > 1) {
		float sqr_dist = in_out_dist * in_out_dist;
		unsigned int n = 0;
		NearestRecurse(1, in_key, in_val, sqr_dist, n);
		if (n != 0) {
			//std::cout << "res = " << n << std::endl;
			out_key = nodes[n].key;
			out_val = nodes[n].val;
			in_out_dist = sqrt(sqr_dist);
			return true;
		}
	}
	return false;
}

template<class KeyT, class ValT, unsigned int K>
inline bool KdTree<KeyT, ValT, K>::Nearest(const unsigned int in_node, float& in_out_dist, KeyT& out_key, ValT& out_val) const
{
	assert(isBuild);

	const unsigned int size = static_cast<unsigned int>(nodes.size());
	if (size > 1 && in_node < size) {
		float sqr_dist = in_out_dist * in_out_dist;

		KeyT in_key = nodes[in_node].key;
		ValT in_val = nodes[in_node].val;

		{
			unsigned int idx = in_node * 2;
			if (idx < size) {
				sqr_dist = min(SqrDist(nodes[in_node].key, nodes[idx].key), sqr_dist);
			}
			if (idx + 1 < size) {
				sqr_dist = min(SqrDist(nodes[in_node].key, nodes[idx + 1].key), sqr_dist);
			}
		}

		unsigned int n = 0;
		NearestRecurse(1, in_key, in_val, sqr_dist, n);
		if (n != 0) {
			//std::cout << "res = " << n << std::endl;
			out_key = nodes[n].key;
			out_val = nodes[n].val;
			in_out_dist = sqrt(sqr_dist);
			return true;
		}
	}
	return false;
}

template<class KeyT, class ValT, unsigned int K>
inline bool KdTree<KeyT, ValT, K>::NNearest(const KeyT& in_key, const ValT& in_val, float& in_out_dist, std::vector<KDTreeRecord<KeyT, ValT>>& out_vector, const unsigned int N) const
{
	assert(isBuild);

	if (nodes.size() > 1) {
		// create queue for finding closest elements
		NQueue<KDTreeRecord<KeyT, ValT>> queue = NQueue<KDTreeRecord<KeyT, ValT>>(N);

		// recursively find the closeset elements and push them to the queue
		NNearestRecurse(1, in_key, in_val, in_out_dist, queue);
		// output the queue to the output vector
		queue.to_vector(out_vector);
	}
	// return true if anything was found
	return out_vector.size() > 0;
}


template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::NearestExceptSelf(const KeyT& in_key, float& in_out_dist, KeyT& out_key, ValT& out_val, unsigned int in_self) const
{
	// check that the tree is in fact build since last insert
	assert(isBuild);

	if (nodes.size() > 1) {
		// float sqr_dist = dist * dist;
		if (int n = NearestRecurseExceptSelf(1, in_key, in_out_dist, in_self)) {
			//std::cout << "res = " << n << std::endl;
			out_key = nodes[n].key;
			out_val = nodes[n].val;
			//dist = sqrt(dist);
			return n;
		}
	}
	return 0;
}

template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::NearestExcept(const KeyT& in_key, float& in_out_dist, KeyT& out_key, ValT& out_val, const std::unordered_set<unsigned int>& in_except) const
{
	//std::cout << "Nearest Except:";
	for (unsigned int i : in_except) {
		std::cout << " " << i;
	}
	std::cout << std::endl;
	// check that the tree is in fact build since last insert
	assert(isBuild);

	if (nodes.size() > 1) {
		// float sqr_dist = dist * dist;
		if (unsigned int n = NearestRecurseExcept(1, in_key, in_out_dist, in_except)) {
			out_key = nodes[n].key;
			out_val = nodes[n].val;
			//dist = sqrt(dist);
			return n;
		}
	}
	return 0;
}


template<class KeyT, class ValT, unsigned int K>
inline void KdTree<KeyT, ValT, K>::Insert(const KeyT& in_key, const ValT& in_val)
{
	if (isBuild) {
		isBuild = false;
		std::vector<KdNode> v(1);
		keys.swap(v);
	}
	keys.emplace_back(in_key, in_val);
}

template<class KeyT, class ValT, unsigned int K>
inline void KdTree<KeyT, ValT, K>::ExcangeVal(const ValT& in_val, const unsigned int in_index)
{
	if (in_index < nodes.size()) {
		nodes[in_index].val = in_val;
	}
	else {
		assert(false); // cannot excange a value at a index wich is not there
	}
}

template<class KeyT, class ValT, unsigned int K>
inline void KdTree<KeyT, ValT, K>::Build()
{
	nodes.resize(keys.size());
	BuildRecurse(1, 1, static_cast<unsigned int>(nodes.size()));
	std::vector<KdNode> v(1);
	keys.swap(v);
	isBuild = true;
}

template<class KeyT, class ValT, unsigned int K>
inline void KdTree<KeyT, ValT, K>::BuildZOrder()
{
	const size_t N = keys.size();

	AABB bbox = AABB();

	for (auto& k : keys) {
		bbox.add_point(k.key);
	}

	Vec3f diff = bbox.size();

	struct ZOrderNode {
		ZOrderIndex key;
		KdNode val;
		inline bool operator< (const ZOrderNode& other) {
			return key < other.key;
		}
	};

	std::vector<ZOrderNode> build_nodes = std::vector<ZOrderNode>();
	build_nodes.resize(N);

	for (int i = 0; i < N; i++) {
		build_nodes[i].key = ZOrderIndex((keys[i].key - bbox.p_min) / diff);
		build_nodes[i].val = keys[i];
	}

	//std::cout << "Sorting primitives" << std::endl;
	std::sort(std::execution::par_unseq, build_nodes.begin() + 1, build_nodes.end());

	for (int i = 0; i < N; i++) {
		//std::cout << build_nodes[i].val.key << std::endl;
		keys[i] = build_nodes[i].val;
	}

	std::cout << keys[1].key << std::endl;
	std::cout << keys[N - 1].key << std::endl;

	nodes.resize(N);

	//std::cout << "Building Hierachie" << std::endl;
	BuildRecurseZOrder(1, 1, N);
	std::vector<KdNode> v(1);
	keys.swap(v);
	isBuild = true;
}


template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::Size() const
{
	return static_cast<unsigned int>(nodes.size() - 1);
}


template<class KeyT, class ValT, unsigned int K>
inline bool KdTree<KeyT, ValT, K>::IsBuild() const
{
	return isBuild;
}

template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::Find(const KeyT& in_key, const ValT& in_val)
{
	if (isBuild) {
		std::cout << "Find: key: " << in_key << ", val: " << in_val << std::endl;
		if (unsigned int n = FindRecurse(ROOT_INDEX, in_key, in_val)) {
			return n;
		}
	}
	return 0;
}

template<class KeyT, class ValT, unsigned int K>
inline std::vector<std::pair<unsigned int, ValT>> KdTree<KeyT, ValT, K>::GetNodes()
{
	std::vector<std::pair<unsigned int, ValT>> res = std::vector<std::pair<unsigned int, ValT>>();

	if (isBuild) {
		res.resize(nodes.size() - 1);
		for (int i = 1; i < nodes.size(); i++) {
			res.push_back(std::make_pair(i, nodes[i].val));
		}
	}

	return res;
}

template<class KeyT, class ValT, unsigned int K>
inline void KdTree<KeyT, ValT, K>::BuildRecurse(const unsigned int in_current, const unsigned int in_begin, const unsigned int in_end)
{
	assert(in_begin <= in_end); // cannot use negative ranges
	const unsigned int N = in_end - in_begin;

	if (N == 1) {
		nodes[in_current] = keys[in_begin];
		nodes[in_current].axis = -1;
		return;
	}

	// find the best axis to sort split the elements on
	short axis = OptimalAxis(in_begin, in_end);
	const unsigned int M = 1 << fast_log2(N);
	const unsigned int R = N - (M - 1);
	unsigned int left_size = (M - 2) / 2;
	unsigned int right_size = (M - 2) / 2;
	if (R < M / 2) {
		left_size += R;
	}
	else {
		left_size += M / 2;
		right_size += R - M / 2;
	}

	// find median
	unsigned int median = in_begin + left_size;

	// sort the keys according to the axis
	KdNode* data = keys.data();
	std::nth_element(std::execution::par_unseq, data + in_begin, data + median, data + in_end, axisCompare[axis]);

	// insert node in the trace tree
	nodes[in_current] = keys[median];
	nodes[in_current].axis = axis;

	// recursively build left and right subtree
	if (left_size > 0)
		BuildRecurse(in_current * 2, in_begin, median);

	if (right_size > 0)
		BuildRecurse(in_current * 2 + 1, median + 1, in_end);
}

template<class KeyT, class ValT, unsigned int K>
void KdTree<KeyT, ValT, K>::NearestRecurse(const unsigned int in_node, const KeyT& in_key, const ValT& in_val, float& in_out_sqr_dist, unsigned int& out_node) const
{
	unsigned int return_node = 0;
	float sqr_dist = SqrDist(nodes[in_node].key, in_key);
	//std::cout << "n=" << in_node << ", val: " << nodes[in_node].val << ", dist: " << dist << std::endl;

	if (sqr_dist < in_out_sqr_dist) {
		if (nodes[in_node].val != in_val) {
			in_out_sqr_dist = sqr_dist;
			out_node = in_node;
		}
	}

	int axis = nodes[in_node].axis;
	if (axis != -1) {
		float axis_sqr_dist = sqr(nodes[in_node].key[axis] - in_key[axis]);
		bool axis_dir = in_key[axis] < nodes[in_node].key[axis];

		if (axis_dir || axis_sqr_dist < in_out_sqr_dist) {

			unsigned int left_child = 2 * in_node;
			if (left_child < nodes.size())
				NearestRecurse(left_child, in_key, in_val, in_out_sqr_dist, out_node);
		}
		if (!axis_dir || axis_sqr_dist < in_out_sqr_dist) {
			unsigned int right_child = 2 * in_node + 1;
			if (right_child < nodes.size())
				NearestRecurse(right_child, in_key, in_val, in_out_sqr_dist, out_node);

		}
	}
}

template<class KeyT, class ValT, unsigned int K>
inline void KdTree<KeyT, ValT, K>::NNearestRecurse(const unsigned int in_node, const KeyT& in_key, const ValT& in_val, float& in_out_dist, NQueue<KDTreeRecord<KeyT, ValT>>& out_queue) const
{
	unsigned int return_node = 0;
	float dist = nodes[in_node].Dist(in_key);
	//std::cout << "n=" << in_node << ", val: " << nodes[in_node].val << ", dist: " << dist << std::endl;

	if (dist < in_out_dist) {
		if (nodes[in_node].val != in_val || dist > 0.0f) {
			out_queue.push(KDTreeRecord<KeyT, ValT>(dist, nodes[in_node].key, nodes[in_node].val));
			if (out_queue.at_capacity())
				in_out_dist = min(in_out_dist, out_queue.top().dist);
		}
	}

	//std::cout << "queue: " << out_queue.size() << ", dist: " << in_out_dist << ", current dist: " << dist << std::endl;

	int axis = nodes[in_node].axis;
	if (axis != -1) {
		float axis_dist = fabsf(nodes[in_node].key[axis] - in_key[axis]);
		bool axis_dir = in_key[axis] < nodes[in_node].key[axis];

		if (axis_dir || axis_dist < in_out_dist) {
			unsigned int left_child = 2 * in_node;
			if (left_child < nodes.size())
				NNearestRecurse(left_child, in_key, in_val, in_out_dist, out_queue);

		}
		if (!axis_dir || axis_dist < in_out_dist) {
			unsigned int right_child = 2 * in_node + 1;
			if (right_child < nodes.size())
				NNearestRecurse(right_child, in_key, in_val, in_out_dist, out_queue);
		}
	}
}

template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::NearestRecurseExceptSelf(const unsigned int in_node, const KeyT& in_key, float& in_out_dist, unsigned int in_self) const
{
	unsigned int return_node = 0;
	float dist = nodes[in_node].Dist(in_key);
	//std::cout << "n=" << in_node << ", val: " << nodes[in_node].val << ", dist: " << dist << std::endl;

	if (dist < in_out_dist && in_node != in_self) {
		in_out_dist = dist;
		return_node = in_node;
	}

	int axis = nodes[in_node].axis;
	if (axis != -1) {
		float axis_dist = fabsf(nodes[in_node].key[axis] - in_key[axis]);
		bool axis_dir = in_key[axis] < nodes[in_node].key[axis];

		if (axis_dir || axis_dist < in_out_dist) {

			unsigned int left_child = 2 * in_node;
			if (left_child < nodes.size())
				if (unsigned int nleft = NearestRecurse(left_child, in_key, in_out_dist))
					return_node = nleft;

		}
		if (!axis_dir || axis_dist < in_out_dist) {
			unsigned int right_child = 2 * in_node + 1;
			if (right_child < nodes.size())
				if (unsigned int nright = NearestRecurse(right_child, in_key, in_out_dist))
					return_node = nright;
		}
	}
	return return_node;
}

template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::NearestRecurseExcept(const unsigned int in_node, const KeyT& in_key, float& in_out_dist, const std::unordered_set<unsigned int>& in_except) const
{
	unsigned int return_node = 0;
	if (in_except.find(in_node) == in_except.end()) {
		float dist = nodes[in_node].Dist(in_key);
		//std::cout << "n=" << in_node << ", val: " << nodes[in_node].val << ", dist: " << dist << std::endl;

		if (dist < in_out_dist) {
			in_out_dist = dist;
			return_node = in_node;
		}
	}

	int axis = nodes[in_node].axis;
	if (axis != -1) {
		float axis_dist = fabsf(nodes[in_node].key[axis] - in_key[axis]);
		bool axis_dir = in_key[axis] < nodes[in_node].key[axis];

		if (axis_dir || axis_dist < in_out_dist) {
			unsigned int left_child = 2 * in_node;
			if (left_child < nodes.size())
				if (unsigned int nleft = NearestRecurseExcept(left_child, in_key, in_out_dist, in_except))
					return_node = nleft;

		}
		if (!axis_dir || axis_dist < in_out_dist) {
			unsigned int right_child = 2 * in_node + 1;
			if (right_child < nodes.size())
				if (unsigned int nright = NearestRecurseExcept(right_child, in_key, in_out_dist, in_except))
					return_node = nright;
		}
	}
	return return_node;
}

template<class KeyT, class ValT, unsigned int K>
inline int KdTree<KeyT, ValT, K>::OptimalAxis(const unsigned int in_begin, const unsigned int in_end) const
{
	KeyT keymin = keys[in_begin].key;
	KeyT keymax = keys[in_begin].key;
	for (unsigned int i = in_begin + 1; i < in_end; i++) {
		keymin = min(keymin, keys[i].key);
		keymax = max(keymax, keys[i].key);
	}

	KeyT diff = keymax - keymin;
	return max_axis(diff);
}

template<class KeyT, class ValT, unsigned int K>
inline unsigned int KdTree<KeyT, ValT, K>::FindRecurse(const unsigned int in_node, const KeyT& in_key, const ValT& in_val) const
{
	const KeyT& node_key = nodes[in_node].key;
	std::cout << "node: " << in_node << ", key: " << node_key << ", val: " << nodes[in_node].val << std::endl;
	if (nodes[in_node].key == in_key && nodes[in_node].val == in_val) {
		return in_node;
	}

	int axis = nodes[in_node].axis;
	if (axis != -1) {
		if (in_key[axis] <= node_key[axis]) {
			unsigned int left_child = 2 * in_node;
			if (left_child < nodes.size())
				if (unsigned int nleft = FindRecurse(left_child, in_key, in_val))
					return nleft;
		}
		if (in_key[axis] >= node_key[axis]) {
			unsigned int right_child = 2 * in_node + 1;
			if (right_child < nodes.size())
				if (unsigned int nright = FindRecurse(right_child, in_key, in_val))
					return nright;
		}
	}
	return 0;
}


