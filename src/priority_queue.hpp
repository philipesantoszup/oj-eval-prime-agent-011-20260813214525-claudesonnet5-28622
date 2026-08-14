#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include <vector>
#include <utility>
#include "exceptions.hpp"

namespace sjtu {
/**
 * @brief a container like std::priority_queue which is a heap internal.
 * Implemented as a leftist heap to support O(log n) merge.
 * **Exception Safety**: The `Compare` operation might throw exceptions for certain data.
 * In such cases, any ongoing operation should be terminated, and the priority queue should be restored to its original state before the operation began.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct Node {
		T data;
		Node *left;
		Node *right;
		int dist;
		Node(const T &v) : data(v), left(nullptr), right(nullptr), dist(0) {}
		Node(const Node &o) : data(o.data), left(nullptr), right(nullptr), dist(o.dist) {}
	};

	Node *root;
	size_t sz;
	Compare cmp;

	// Merge two leftist-heap trees rooted at a and b.
	// Exception safety proof: the only place `cmp` is invoked in a frame is
	// *before* any recursive call in that frame; all mutations of node fields
	// happen strictly *after* the recursive call for that frame returns
	// successfully. Hence if `cmp` throws anywhere during the descent, no
	// node field in either input tree has been (or will be) mutated: the
	// exception propagates up through every pending frame without ever
	// reaching the post-recursion mutation statements. Therefore on throw,
	// both input trees are left completely intact.
	Node *mergeNodes(Node *a, Node *b) {
		if (!a) return b;
		if (!b) return a;
		if (cmp(a->data, b->data)) {
			std::swap(a, b);
		}
		Node *mergedRight = mergeNodes(a->right, b);
		a->right = mergedRight;
		if (!a->left || (a->right && a->left->dist < a->right->dist)) {
			std::swap(a->left, a->right);
		}
		a->dist = (a->right ? a->right->dist : -1) + 1;
		return a;
	}

	static void deleteTree(Node *node) {
		if (!node) return;
		std::vector<Node *> stk;
		stk.push_back(node);
		while (!stk.empty()) {
			Node *cur = stk.back();
			stk.pop_back();
			if (cur->left) stk.push_back(cur->left);
			if (cur->right) stk.push_back(cur->right);
			delete cur;
		}
	}

	static Node *cloneTree(Node *src) {
		if (!src) return nullptr;
		Node *newRoot = new Node(*src);
		std::vector<Node *> created;
		created.push_back(newRoot);
		std::vector<std::pair<Node *, Node *> > stk;
		stk.push_back(std::make_pair(src, newRoot));
		try {
			while (!stk.empty()) {
				std::pair<Node *, Node *> pr = stk.back();
				stk.pop_back();
				Node *s = pr.first;
				Node *d = pr.second;
				if (s->left) {
					Node *nl = new Node(*s->left);
					created.push_back(nl);
					d->left = nl;
					stk.push_back(std::make_pair(s->left, nl));
				}
				if (s->right) {
					Node *nr = new Node(*s->right);
					created.push_back(nr);
					d->right = nr;
					stk.push_back(std::make_pair(s->right, nr));
				}
			}
		} catch (...) {
			for (size_t i = 0; i < created.size(); ++i) delete created[i];
			throw;
		}
		return newRoot;
	}

public:
	/**
	 * @brief default constructor
	 */
	priority_queue() : root(nullptr), sz(0) {}

	/**
	 * @brief copy constructor
	 * @param other the priority_queue to be copied
	 */
	priority_queue(const priority_queue &other) : root(cloneTree(other.root)), sz(other.sz) {}

	/**
	 * @brief deconstructor
	 */
	~priority_queue() { deleteTree(root); }

	/**
	 * @brief Assignment operator
	 * @param other the priority_queue to be assigned from
	 * @return a reference to this priority_queue after assignment
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		Node *newRoot = cloneTree(other.root);
		deleteTree(root);
		root = newRoot;
		sz = other.sz;
		return *this;
	}

	/**
	 * @brief get the top element of the priority queue.
	 * @return a reference of the top element.
	 * @throws container_is_empty if empty() returns true
	 */
	const T & top() const {
		if (!root) throw container_is_empty();
		return root->data;
	}

	/**
	 * @brief push new element to the priority queue.
	 * @param e the element to be pushed
	 */
	void push(const T &e) {
		Node *newNode = new Node(e);
		try {
			root = mergeNodes(root, newNode);
		} catch (...) {
			delete newNode;
			throw runtime_error();
		}
		++sz;
	}

	/**
	 * @brief delete the top element from the priority queue.
	 * @throws container_is_empty if empty() returns true
	 */
	void pop() {
		if (!root) throw container_is_empty();
		Node *oldRoot = root;
		Node *newRoot;
		try {
			newRoot = mergeNodes(oldRoot->left, oldRoot->right);
		} catch (...) {
			throw runtime_error();
		}
		root = newRoot;
		oldRoot->left = oldRoot->right = nullptr;
		delete oldRoot;
		--sz;
	}

	/**
	 * @brief return the number of elements in the priority queue.
	 * @return the number of elements.
	 */
	size_t size() const { return sz; }

	/**
	 * @brief check if the container is empty.
	 * @return true if it is empty, false otherwise.
	 */
	bool empty() const { return sz == 0; }

	/**
	 * @brief merge another priority_queue into this one.
	 * The other priority_queue will be cleared after merging.
	 * The complexity is at most O(logn).
	 * @param other the priority_queue to be merged.
	 */
	void merge(priority_queue &other) {
		if (this == &other) return;
		Node *newRoot;
		try {
			newRoot = mergeNodes(root, other.root);
		} catch (...) {
			throw runtime_error();
		}
		root = newRoot;
		sz += other.sz;
		other.root = nullptr;
		other.sz = 0;
	}
};

}

#endif
