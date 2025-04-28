#include <iostream>
#include <algorithm>
#include <memory>
#include <concepts>
#include <optional>
#include <string>
#include <vector>
#include <sstream>
#include <utility>
#include <iomanip>
#include <functional>

template <typename T>
concept Comparable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
    { a == b } -> std::convertible_to<bool>;
};

// TreePrinter class adapted from the Java implementation
template <typename T, typename NodeType>
class TreePrinter {
    struct TreeLine {
        std::string line;
        int leftOffset;
        int rightOffset;

        TreeLine(std::string l, int left, int right)
            : line(std::move(l)), leftOffset(left), rightOffset(right) {}
    };

    std::function<std::string(const NodeType*)> getLabel;
    std::function<const NodeType*(const NodeType*)> getLeft;
    std::function<const NodeType*(const NodeType*)> getRight;

    std::ostream& outStream;
    bool squareBranches = false;
    bool lrAgnostic = false;
    int hspace = 2;

    static std::string spaces(const int n) {
        return std::string(std::max(0, n), ' ');
    }

    static int minLeftOffset(const std::vector<TreeLine>& treeLines) {
        if (treeLines.empty()) return 0;
        int minOffset = treeLines[0].leftOffset;
        for (const auto& line : treeLines) {
            minOffset = std::min(minOffset, line.leftOffset);
        }
        return minOffset;
    }

    static int maxRightOffset(const std::vector<TreeLine>& treeLines) {
        if (treeLines.empty()) return 0;
        int maxOffset = treeLines[0].rightOffset;
        for (const auto& line : treeLines) {
            maxOffset = std::max(maxOffset, line.rightOffset);
        }
        return maxOffset;
    }

    void printTreeLines(const std::vector<TreeLine>& treeLines) {
        if (!treeLines.empty()) {
            int minLeft = minLeftOffset(treeLines);
            int maxRight = maxRightOffset(treeLines);
            for (const auto& treeLine : treeLines) {
                const int leftSpaces = -(minLeft - treeLine.leftOffset);
                const int rightSpaces = maxRight - treeLine.rightOffset;
                outStream << spaces(leftSpaces) << treeLine.line << spaces(rightSpaces) << std::endl;
            }
        }
    }

    std::vector<TreeLine> buildTreeLines(const NodeType* root) {
        if (!root) return {};

        std::string rootLabel = getLabel(root);
        std::vector<TreeLine> leftTreeLines = buildTreeLines(getLeft(root));
        std::vector<TreeLine> rightTreeLines = buildTreeLines(getRight(root));

        int leftCount = leftTreeLines.size();
        int rightCount = rightTreeLines.size();
        int minCount = std::min(leftCount, rightCount);
        int maxCount = std::max(leftCount, rightCount);

        // Find max root spacing
        int maxRootSpacing = 0;
        for (int i = 0; i < minCount; i++) {
            int spacing = leftTreeLines[i].rightOffset - rightTreeLines[i].leftOffset;
            maxRootSpacing = std::max(maxRootSpacing, spacing);
        }

        int rootSpacing = maxRootSpacing + hspace;
        if (rootSpacing % 2 == 0) rootSpacing++;

        std::vector<TreeLine> allTreeLines;

        // Add the root and branches
        allTreeLines.push_back(TreeLine(rootLabel, -(rootLabel.length() - 1) / 2, rootLabel.length() / 2));

        int leftTreeAdjust = 0;
        int rightTreeAdjust = 0;

        if (leftTreeLines.empty()) {
            if (!rightTreeLines.empty()) {
                // Right subtree only
                if (squareBranches) {
                    if (lrAgnostic) {
                        allTreeLines.push_back(TreeLine("|", 0, 0));
                    } else {
                        allTreeLines.push_back(TreeLine("+--+", 0, 3));
                        rightTreeAdjust = 3;
                    }
                } else {
                    allTreeLines.push_back(TreeLine("\\", 1, 1));
                    rightTreeAdjust = 2;
                }
            }
        } else if (rightTreeLines.empty()) {
            // Left subtree only
            if (squareBranches) {
                if (lrAgnostic) {
                    allTreeLines.push_back(TreeLine("|", 0, 0));
                } else {
                    allTreeLines.push_back(TreeLine("+--+", -3, 0));
                    leftTreeAdjust = -3;
                }
            } else {
                allTreeLines.push_back(TreeLine("/", -1, -1));
                leftTreeAdjust = -2;
            }
        } else {
            // Both left and right subtrees
            if (squareBranches) {
                int adjust = (rootSpacing / 2) + 1;
                auto horizontal = std::string(rootSpacing / 2, '-');
                std::string branch = "+" + horizontal + "+" + horizontal + "+";
                allTreeLines.push_back(TreeLine(branch, -adjust, adjust));
                rightTreeAdjust = adjust;
                leftTreeAdjust = -adjust;
            } else {
                if (rootSpacing == 1) {
                    allTreeLines.push_back(TreeLine("/ \\", -1, 1));
                    rightTreeAdjust = 2;
                    leftTreeAdjust = -2;
                } else {
                    for (int i = 1; i < rootSpacing; i += 2) {
                        std::string branches = "/" + spaces(i) + "\\";
                        allTreeLines.push_back(TreeLine(branches, -((i + 1) / 2), (i + 1) / 2));
                    }
                    rightTreeAdjust = (rootSpacing / 2) + 1;
                    leftTreeAdjust = -((rootSpacing / 2) + 1);
                }
            }
        }

        // Join lines of subtrees
        for (int i = 0; i < maxCount; i++) {
            if (i >= leftTreeLines.size()) {
                // Nothing remaining on left subtree
                TreeLine rightLine = rightTreeLines[i];
                rightLine.leftOffset += rightTreeAdjust;
                rightLine.rightOffset += rightTreeAdjust;
                allTreeLines.push_back(rightLine);
            } else if (i >= rightTreeLines.size()) {
                // Nothing remaining on right subtree
                TreeLine leftLine = leftTreeLines[i];
                leftLine.leftOffset += leftTreeAdjust;
                leftLine.rightOffset += leftTreeAdjust;
                allTreeLines.push_back(leftLine);
            } else {
                const TreeLine& leftLine = leftTreeLines[i];
                const TreeLine& rightLine = rightTreeLines[i];
                int adjustedRootSpacing = (rootSpacing == 1 ? (squareBranches ? 1 : 3) : rootSpacing);
                TreeLine combined(
                    leftLine.line + spaces(adjustedRootSpacing - leftLine.rightOffset + rightLine.leftOffset) + rightLine.line,
                    leftLine.leftOffset + leftTreeAdjust,
                    rightLine.rightOffset + rightTreeAdjust
                );
                allTreeLines.push_back(combined);
            }
        }

        return allTreeLines;
    }

public:
    TreePrinter(
        std::function<std::string(const NodeType*)> labelFn,
        std::function<const NodeType*(const NodeType*)> leftFn,
        std::function<const NodeType*(const NodeType*)> rightFn,
        std::ostream& os = std::cout
    ) : getLabel(std::move(labelFn)),
        getLeft(std::move(leftFn)),
        getRight(std::move(rightFn)),
        outStream(os) {}

    void setSquareBranches(const bool value) { squareBranches = value; }
    void setLrAgnostic(const bool value) { lrAgnostic = value; }
    void setHspace(const int value) { hspace = value; }

    void printTree(const NodeType* root) {
        std::vector<TreeLine> treeLines = buildTreeLines(root);
        printTreeLines(treeLines);
    }
};

template <Comparable T>
class AVLTree {
    struct Node {
        T value;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        int height;

        explicit Node(T val) : value(std::move(val)), left(nullptr), right(nullptr), height(1) {}
    };

    std::unique_ptr<Node> root;

    // Get height of a node (nullptr returns 0)
    [[nodiscard]] static int height(const Node* node) {
        return node ? node->height : 0;
    }

    // Get balance factor of a node
    [[nodiscard]] int balanceFactor(const Node* node) const {
        return node ? height(node->left.get()) - height(node->right.get()) : 0;
    }

    // Update height of a node
    void updateHeight(Node* node) {
        if (node) {
            node->height = 1 + std::max(height(node->left.get()), height(node->right.get()));
        }
    }

    // Right rotation
    std::unique_ptr<Node> rotateRight(std::unique_ptr<Node> y) {
        auto x = std::move(y->left);
        y->left = std::move(x->right);

        updateHeight(y.get());
        x->right = std::move(y);
        updateHeight(x.get());

        return x;
    }

    // Left rotation
    std::unique_ptr<Node> rotateLeft(std::unique_ptr<Node> x) {
        auto y = std::move(x->right);
        x->right = std::move(y->left);

        updateHeight(x.get());
        y->left = std::move(x);
        updateHeight(y.get());

        return y;
    }

    // Balance the tree at a node
    std::unique_ptr<Node> balance(std::unique_ptr<Node> node) {
        if (!node) return nullptr;

        updateHeight(node.get());
        const int bf = balanceFactor(node.get());

        // Left-heavy
        if (bf > 1) {
            // Left-Right case
            if (balanceFactor(node->left.get()) < 0) {
                node->left = rotateLeft(std::move(node->left));
            }
            // Left-Left case
            return rotateRight(std::move(node));
        }

        // Right-heavy
        if (bf < -1) {
            // Right-Left case
            if (balanceFactor(node->right.get()) > 0) {
                node->right = rotateRight(std::move(node->right));
            }
            // Right-Right case
            return rotateLeft(std::move(node));
        }

        // Already balanced
        return std::move(node);
    }

    // Insert a value into the subtree rooted at node
    std::unique_ptr<Node> insert(std::unique_ptr<Node> node, T value) {
        if (!node) {
            return std::make_unique<Node>(std::move(value));
        }

        if (value < node->value) {
            node->left = insert(std::move(node->left), value);
        } else if (value > node->value) {
            node->right = insert(std::move(node->right), value);
        } else {
            // Value already exists, do nothing
            return std::move(node);
        }

        return balance(std::move(node));
    }

    // Find the minimum value node in a subtree
    static Node* findMin(Node* node) {
        if (!node) return nullptr;
        while (node->left) {
            node = node->left.get();
        }
        return node;
    }

    // Remove a value from the subtree rooted at node
    std::unique_ptr<Node> remove(std::unique_ptr<Node> node, const T& value) {
        if (!node) return nullptr;

        if (value < node->value) {
            node->left = remove(std::move(node->left), value);
        } else if (value > node->value) {
            node->right = remove(std::move(node->right), value);
        } else {
            // Node with the value found

            // Case 1: Leaf node or node with only one child
            if (!node->left) {
                return std::move(node->right);
            }
            if (!node->right) {
                return std::move(node->left);
            }

            // Case 2: Node with two children
            // Find the inorder successor (minimum value in right subtree)
            Node* successor = findMin(node->right.get());
            node->value = successor->value;

            // Remove the successor
            node->right = remove(std::move(node->right), successor->value);
        }

        return balance(std::move(node));
    }

    // Search for a value in the subtree rooted at node
    const Node* search(const Node* node, const T& value) const {
        if (!node) return nullptr;

        if (value < node->value) {
            return search(node->left.get(), value);
        }
        if (value > node->value) {
            return search(node->right.get(), value);
        }
        return node; // Found
    }

    // Inorder traversal of the subtree rooted at node
    void inorderTraversal(const Node* node, auto& visitor) const {
        if (!node) return;

        inorderTraversal(node->left.get(), visitor);
        visitor(node->value);
        inorderTraversal(node->right.get(), visitor);
    }

public:
    AVLTree() : root(nullptr) {}

    // Insert a value into the tree
    void insert(T value) {
        root = insert(std::move(root), std::move(value));
    }

    // Remove a value from the tree
    void remove(const T& value) {
        root = remove(std::move(root), value);
    }

    // Check if the tree contains a value
    [[nodiscard]] bool contains(const T& value) const {
        return search(root.get(), value) != nullptr;
    }

    // Get a value if it exists
    [[nodiscard]] std::optional<T> get(const T& value) const {
        const Node* node = search(root.get(), value);
        return node ? std::optional<T>(node->value) : std::nullopt;
    }

    // Inorder traversal
    template <typename Visitor>
    void inorder(Visitor visitor) const {
        inorderTraversal(root.get(), visitor);
    }

    // Print the tree with improved visual format
    void print(std::ostream& os = std::cout) const {
        os << "Tree structure:" << std::endl;

        // Create a tree printer with appropriate functions for node access
        auto labelFn = [this](const Node* node) -> std::string {
            if (!node) return "";
            std::stringstream ss;
            ss << node->value << "[" << balanceFactor(node) << "]";
            return ss.str();
        };

        auto leftFn = [](const Node* node) -> const Node* {
            return node ? node->left.get() : nullptr;
        };

        auto rightFn = [](const Node* node) -> const Node* {
            return node ? node->right.get() : nullptr;
        };

        TreePrinter<T, Node> printer(labelFn, leftFn, rightFn, os);
        printer.setSquareBranches(true);  // Use ASCII box drawing characters
        printer.setHspace(3);  // Set horizontal spacing between nodes
        printer.printTree(root.get());

        // Print inorder traversal
        os << "\nInorder traversal: ";
        inorder([&os](const T& value) { os << value << " "; });
        os << std::endl;
    }
};

// Example usage
int main() {
    AVLTree<int> tree;

    // Insert some values
    tree.insert(10);
    tree.insert(20);
    tree.insert(30);  // This will cause rotations
    tree.insert(40);
    tree.insert(50);  // More rotations
    tree.insert(25);

    // Print the tree
    tree.print();

    // Test search
    std::cout << "Contains 30: " << (tree.contains(30) ? "Yes" : "No") << std::endl;
    std::cout << "Contains 35: " << (tree.contains(35) ? "Yes" : "No") << std::endl;

    // Test removal
    tree.remove(30);
    std::cout << "\nAfter removing 30:" << std::endl;
    tree.print();

    return 0;
}