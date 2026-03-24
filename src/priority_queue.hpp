#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
    struct Node {
        T data;
        Node *left = nullptr;
        Node *right = nullptr;
        int npl = 0;  // null path length

        Node(const T &e) : data(e) {}
        Node(const Node &other)
            : data(other.data), npl(other.npl) {
            if (other.left) left = new Node(*other.left);
            if (other.right) right = new Node(*other.right);
        }
        ~Node() {
            // Don't delete children here - delete_tree handles it
        }
    };

    Node *root = nullptr;
    Compare cmp;

    // Recursively get the npl (null path length) of a node
    int get_npl(Node *node) const {
        if (!node) return 0;
        return node->npl;
    }

    // Recursively merge two leftist heaps
    Node* merge(Node *h1, Node *h2) {
        if (!h1) return h2;
        if (!h2) return h1;

        // Ensure h1 has the larger root (max-heap)
        if (cmp(h2->data, h1->data)) {
            std::swap(h1, h2);
        }

        // Merge h2 into h1
        h1->right = merge(h1->right, h2);

        // Update npl and possibly swap children to maintain leftist property
        if (get_npl(h1->left) < get_npl(h1->right)) {
            std::swap(h1->left, h1->right);
        }

        h1->npl = 1 + get_npl(h1->left);

        return h1;
    }

    // Deep copy a subtree
    Node* copy_tree(Node *node) {
        if (!node) return nullptr;
        Node *new_node = new Node(*node);
        new_node->left = copy_tree(node->left);
        new_node->right = copy_tree(node->right);
        return new_node;
    }

    // Deep delete a subtree
    void delete_tree(Node *node) {
        if (!node) return;
        delete_tree(node->left);
        delete_tree(node->right);
        delete node;
    }

public:
    priority_queue() : root(nullptr), cmp() {}

    priority_queue(const priority_queue &other) : cmp(other.cmp) {
        root = copy_tree(other.root);
    }

    ~priority_queue() {
        delete_tree(root);
    }

    priority_queue &operator=(const priority_queue &other) {
        if (this == &other) return *this;

        // Save current state for exception safety
        Node *old_root = root;
        Compare old_cmp = cmp;
        Node *new_root = nullptr;

        try {
            // Create new state
            new_root = copy_tree(other.root);
            Compare new_cmp = other.cmp;

            // Swap to new state
            root = new_root;
            cmp = new_cmp;

            // Delete old state
            delete_tree(old_root);
        } catch (...) {
            // Restore old state on exception
            if (new_root) delete_tree(new_root);
            root = old_root;
            cmp = old_cmp;
            throw;
        }

        return *this;
    }

    const T & top() const {
        if (empty()) {
            throw container_is_empty();
        }
        return root->data;
    }

    void push(const T &e) {
        // Create a single-node heap
        Node *new_heap = new Node(e);

        // Save current state for exception safety
        Node *old_root = root;

        try {
            root = merge(root, new_heap);
        } catch (...) {
            // Restore old state on exception
            delete new_heap;
            root = old_root;
            throw;
        }
    }

    void pop() {
        if (empty()) {
            throw container_is_empty();
        }

        // Save current state for exception safety
        Node *old_root = root;

        try {
            Node *left = root->left;
            Node *right = root->right;

            root = merge(left, right);

            // Delete old root
            delete old_root;
        } catch (...) {
            // Restore old state on exception
            root = old_root;
            throw;
        }
    }

    size_t size() const {
        // Count nodes in the tree
        std::function<size_t(Node*)> count_nodes = [&](Node *node) -> size_t {
            if (!node) return 0;
            return 1 + count_nodes(node->left) + count_nodes(node->right);
        };
        return count_nodes(root);
    }

    bool empty() const {
        return root == nullptr;
    }

    void merge(priority_queue &other) {
        // Save current state for exception safety
        Node *old_root = root;
        Node *other_root = other.root;

        try {
            root = merge(root, other.root);
            other.root = nullptr;
        } catch (...) {
            // Restore both heaps on exception
            root = old_root;
            other.root = other_root;
            throw;
        }
    }
};

}

#endif
